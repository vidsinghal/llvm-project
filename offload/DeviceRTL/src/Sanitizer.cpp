//===------ Sanitizer.cpp - Track allocation for sanitizer checks ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "DeviceTypes.h"
#include "DeviceUtils.h"
#include "Interface.h"
#include "LibC.h"
#include "Mapping.h"
#include "Shared/Environment.h"
#include "Synchronization.h"
//#include "Shared/Debug.h"

using namespace ompx;
using namespace utils;

#pragma omp begin declare target device_type(nohost)

#include "Shared/Sanitizer.h"

struct AllocationInfoLocalTy {
  _AS_PTR(void, AllocationKind::LOCAL) Start;
  uint64_t Length;
  uint32_t Tag;
};
struct AllocationInfoGlobalTy {
  _AS_PTR(void, AllocationKind::GLOBAL) Start;
  uint64_t Length;
  uint32_t Tag;
};

template <AllocationKind AK> struct AllocationInfoTy {};
template <> struct AllocationInfoTy<AllocationKind::GLOBAL> {
  using ASVoidPtrTy = AllocationInfoGlobalTy;
};
template <> struct AllocationInfoTy<AllocationKind::LOCAL> {
  using ASVoidPtrTy = AllocationInfoLocalTy;
};

template <>
AllocationPtrTy<AllocationKind::LOCAL>
AllocationPtrTy<AllocationKind::LOCAL>::get(_AS_PTR(void, AllocationKind::LOCAL)
                                                P) {
  TypePunUnion TPU;
  TPU.P = (void *)P;
  return TPU.AP;
}

template <>
AllocationPtrTy<AllocationKind::LOCAL>::operator _AS_PTR(
    void, AllocationKind::LOCAL)() const {
  TypePunUnion TPU;
  TPU.AP = *this;
  return TPU.AddrP;
}

template <AllocationKind AK> struct AllocationTracker {
  static_assert(sizeof(AllocationTy<AK>) == sizeof(_AS_PTR(void, AK)) * 2,
                "AllocationTy should not exceed two pointers");
  //  static_assert(sizeof(AllocationPtrTy<AK>) * 8 ==
  //                    SanitizerConfig<AK>::ADDR_SPACE_PTR_SIZE,
  //                "AllocationTy pointers should be pointer sized");

  [[clang::disable_sanitizer_instrumentation]] static
      typename AllocationInfoTy<AK>::ASVoidPtrTy
      getAllocationInfo(_AS_PTR(void, AK) P) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    uint32_t AllocationId = AP.AllocationId;
    if (OMP_UNLIKELY(AllocationId >= SanitizerConfig<AK>::SLOTS))
      return {P, 0, (uint32_t)-1};
    auto &A = getAllocation<AK>(AP, /*AccessId=*/0, /*PC=*/0);
    return {A.Start, A.Length, (uint32_t)A.Tag};
  }

  [[clang::disable_sanitizer_instrumentation]] static _AS_PTR(void, AK)
      create(_AS_PTR(void, AK) Start, uint64_t Length, int64_t AllocationId,
             int64_t Slot, int64_t SourceId, uint64_t PC) {
    if constexpr (SanitizerConfig<AK>::OFFSET_BITS < 64)
      if (OMP_UNLIKELY(Length >= (1UL << (SanitizerConfig<AK>::OFFSET_BITS))))
        __sanitizer_trap_info_ptr->exceedsAllocationLength<AK>(
            Start, Length, AllocationId, Slot, SourceId, /*PC=*/0);

    // Reserve the 0 element for the null pointer in global space.
    auto &AllocArr = getAllocationArray<AK>();
    auto &Cnt = AllocArr.Cnt;
    if constexpr (AK == AllocationKind::LOCAL)
      Slot = ++Cnt;
    if (Slot == -1)
      Slot = ++Cnt;

    uint64_t NumSlots = SanitizerConfig<AK>::SLOTS;
    if (OMP_UNLIKELY(Slot >= NumSlots))
      __sanitizer_trap_info_ptr->exceedsAllocationSlots<AK>(
          Start, Length, AllocationId, Slot, SourceId, /*PC=*/0);

    auto &A = AllocArr.Arr[Slot];

    A.Start = Start;
    A.Length = Length;
    A.Id = AllocationId;

    AllocationPtrTy<AK> AP;
    AP.Offset = 0;
    if constexpr (SanitizerConfig<AK>::useTags()) {
      AP.AllocationTag = ++A.Tag;
    }
    AP.AllocationId = Slot;
    AP.Magic = SanitizerConfig<AK>::MAGIC;
    AP.Kind = (uint64_t)AK;
    return AP;
  }

  [[clang::disable_sanitizer_instrumentation]] static void
  remove(_AS_PTR(void, AK) P, int64_t SourceId) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    uint64_t AllocationId = AP.AllocationId;
    auto &AllocArr = getAllocationArray<AK>();
    auto &A = AllocArr.Arr[AllocationId];
    A.Length = 0;

    auto &Cnt = AllocArr.Cnt;
    if constexpr (AK == AllocationKind::LOCAL) {
      if (Cnt == AllocationId)
        --Cnt;
    }
  }

  [[clang::disable_sanitizer_instrumentation]] static void remove_n(int32_t N) {
    static_assert(AK == AllocationKind::LOCAL, "");
    auto &AllocArr = getAllocationArray<AK>();
    auto &Cnt = AllocArr.Cnt;
    for (int32_t I = 0; I < N; ++I) {
      auto &A = AllocArr.Arr[Cnt--];
      A.Length = 0;
    }
  }

  [[clang::disable_sanitizer_instrumentation]] static _AS_PTR(void, AK)
      advance(_AS_PTR(void, AK) P, uint64_t Offset, int64_t SourceId) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    AP.Offset += Offset;
    return AP;
  }

  [[clang::disable_sanitizer_instrumentation]] static _AS_PTR(void, AK)
      checkWithBase(_AS_PTR(void, AK) P, _AS_PTR(void, AK) Start,
                    int64_t Length, uint32_t Tag, int64_t Size,
                    int64_t AccessId, int64_t SourceId, uint64_t PC) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    if constexpr (AK == AllocationKind::LOCAL)
      if (Length == 0)
        Length = getAllocation<AK>(AP, AccessId, PC).Length;
    if constexpr (AK == AllocationKind::GLOBAL)
      if (AP.Magic != SanitizerConfig<AllocationKind::GLOBAL>::MAGIC)
        __sanitizer_trap_info_ptr->garbagePointer<AK>(AP, (void *)P, SourceId,
                                                      PC);
    int64_t Offset = AP.Offset;
    if (OMP_UNLIKELY(
            Offset > Length - Size ||
            (SanitizerConfig<AK>::useTags() && Tag != AP.AllocationTag))) {
      __sanitizer_trap_info_ptr->accessError<AK>(AP, Size, AccessId, SourceId,
                                                 PC);
    }
    return utils::advancePtr(Start, Offset);
  }

  [[clang::disable_sanitizer_instrumentation]] static void
  checkWithBaseVoid(_AS_PTR(void, AK) P, _AS_PTR(void, AK) Start,
                    int64_t Length, uint32_t Tag, int64_t Size,
                    int64_t AccessId, int64_t SourceId, uint64_t PC) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    if constexpr (AK == AllocationKind::LOCAL)
      if (Length == 0)
        Length = getAllocation<AK>(AP, AccessId, PC).Length;
    if constexpr (AK == AllocationKind::GLOBAL)
      if (AP.Magic != SanitizerConfig<AllocationKind::GLOBAL>::MAGIC)
        __sanitizer_trap_info_ptr->garbagePointer<AK>(AP, (void *)P, SourceId,
                                                      PC);
    int64_t Offset = AP.Offset;
    if (OMP_UNLIKELY(
            Offset > Length - Size ||
            (SanitizerConfig<AK>::useTags() && Tag != AP.AllocationTag))) {
      __sanitizer_trap_info_ptr->accessError<AK>(AP, Size, AccessId, SourceId,
                                                 PC);
    }
  }

  [[clang::disable_sanitizer_instrumentation]] static void
  checkRangeWithBase(_AS_PTR(void, AK) SCEVMax, _AS_PTR(void, AK) SCEVMin,
                     _AS_PTR(void, AK) StartAddress, int64_t AllocationLength,
                     uint32_t Tag, int64_t AccessTypeSize, int64_t AccessId,
                     int64_t SourceId, uint64_t PC) {

    AllocationPtrTy<AK> APSCEVMax = AllocationPtrTy<AK>::get(SCEVMax);
    AllocationPtrTy<AK> APSCEVMin = AllocationPtrTy<AK>::get(SCEVMin);
    if constexpr (AK == AllocationKind::LOCAL)
      if (AllocationLength == 0)
        AllocationLength = getAllocation<AK>(APSCEVMax, AccessId, PC).Length;

    if constexpr (AK == AllocationKind::GLOBAL) {
      if (APSCEVMax.Magic != SanitizerConfig<AllocationKind::GLOBAL>::MAGIC)
        __sanitizer_trap_info_ptr->garbagePointer<AK>(
            APSCEVMax, (void *)SCEVMax, SourceId, PC);

      if (APSCEVMin.Magic != SanitizerConfig<AllocationKind::GLOBAL>::MAGIC)
        __sanitizer_trap_info_ptr->garbagePointer<AK>(
            APSCEVMin, (void *)SCEVMin, SourceId, PC);
    }

    // check upper bound
    int64_t MaxOffset = APSCEVMax.Offset;
    if (OMP_UNLIKELY(MaxOffset > AllocationLength - AccessTypeSize ||
                     (SanitizerConfig<AK>::useTags() &&
                      Tag != APSCEVMax.AllocationTag))) {
      __sanitizer_trap_info_ptr->accessError<AK>(APSCEVMax, AccessTypeSize,
                                                 AccessId, SourceId, PC);
    }

    // check lower bound
    auto &AllocationOfMinOffset = getAllocation<AK>(APSCEVMin, AccessId, PC);
    if (OMP_UNLIKELY(AllocationOfMinOffset.Start != StartAddress ||
                     (SanitizerConfig<AK>::useTags() &&
                      Tag != APSCEVMin.AllocationTag))) {
      __sanitizer_trap_info_ptr->accessError<AK>(APSCEVMin, AccessTypeSize,
                                                 AccessId, SourceId, PC);
    }
  }

  [[clang::disable_sanitizer_instrumentation]] static void
  checkRange(_AS_PTR(void, AK) SCEVMax, _AS_PTR(void, AK) SCEVMin,
             int64_t AccessTypeSize, int64_t AccessId, int64_t SourceId,
             uint64_t PC) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(SCEVMax);
    auto &Alloc = getAllocation<AK>(AP, AccessId, PC);
    return checkRangeWithBase(SCEVMax, SCEVMin, Alloc.Start, Alloc.Length,
                              Alloc.Tag, AccessTypeSize, AccessId, SourceId,
                              PC);
  }

  [[clang::disable_sanitizer_instrumentation]] static _AS_PTR(void, AK)
      check(_AS_PTR(void, AK) P, int64_t Size, int64_t AccessId,
            int64_t SourceId, uint64_t PC) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    auto &Alloc = getAllocation<AK>(AP, AccessId, PC);
    return checkWithBase(P, Alloc.Start, Alloc.Length, Alloc.Tag, Size,
                         AccessId, SourceId, PC);
  }

  [[clang::disable_sanitizer_instrumentation]] static void
  checkVoid(_AS_PTR(void, AK) P, int64_t Size, int64_t AccessId,
            int64_t SourceId, uint64_t PC) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    auto &Alloc = getAllocation<AK>(AP, AccessId, PC);
    return checkWithBaseVoid(P, Alloc.Start, Alloc.Length, Alloc.Tag, Size,
                             AccessId, SourceId, PC);
  }

  [[clang::disable_sanitizer_instrumentation]] static _AS_PTR(void, AK)
      unpack(_AS_PTR(void, AK) P, int64_t SourceId, uint64_t PC) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    auto &A = getAllocation<AK>(AP, SourceId, PC);
    uint64_t Offset = AP.Offset;
    _AS_PTR(void, AK) Ptr = utils::advancePtr(A.Start, Offset);
    return Ptr;
  }

  [[clang::disable_sanitizer_instrumentation]] static void
  lifetimeStart(_AS_PTR(void, AK) P, uint64_t Length) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    auto &A = getAllocation<AK>(AP, /*AccessId=*/0, /*PC=*/0);
    // TODO: Check length
    A.Length = Length;
  }

  [[clang::disable_sanitizer_instrumentation]] static void
  lifetimeEnd(_AS_PTR(void, AK) P, uint64_t Length) {
    AllocationPtrTy<AK> AP = AllocationPtrTy<AK>::get(P);
    auto &A = getAllocation<AK>(AP, /*AccessId=*/0, /*PC=*/0);
    // TODO: Check length
    A.Length = 0;
  }

  [[clang::disable_sanitizer_instrumentation]] static void leakCheck() {
    static_assert(AK == AllocationKind::GLOBAL, "");
    auto &AllocArr = getAllocationArray<AK>();
    for (uint64_t Slot = 0; Slot < SanitizerConfig<AK>::SLOTS; ++Slot) {
      auto &A = AllocArr.Arr[Slot];
      if (OMP_UNLIKELY(A.Length))
        __sanitizer_trap_info_ptr->memoryLeak<AK>(A, Slot);
    }
  }
};

template <AllocationKind AK>
AllocationArrayTy<AK>
    Allocations<AK>::Arr[SanitizerConfig<AK>::NUM_ALLOCATION_ARRAYS];

static void checkForMagic(bool IsGlobal, void *P, int64_t SourceId,
                          uint64_t PC) {
  if (IsGlobal) {
    auto AP = AllocationPtrTy<AllocationKind::GLOBAL>::get(P);
    if (AP.Magic != SanitizerConfig<AllocationKind::GLOBAL>::MAGIC)
      __sanitizer_trap_info_ptr->garbagePointer<AllocationKind::GLOBAL>(
          AP, P, SourceId, PC);
  } else {
    auto AP = AllocationPtrTy<AllocationKind::LOCAL>::get(P);
    if (AP.Magic != SanitizerConfig<AllocationKind::LOCAL>::MAGIC)
      __sanitizer_trap_info_ptr->garbagePointer<AllocationKind::LOCAL>(
          AP, P, SourceId, PC);
  }
}

extern "C" {

#define REAL_PTR_IS_LOCAL(PTR) (isThreadLocalMemPtr(PTR))
#define IS_GLOBAL(PTR) ((uintptr_t)PTR & (1UL << 63))

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::LOCAL)
    ompx_new_local(_AS_PTR(void, AllocationKind::LOCAL) Start, uint64_t Length,
                   int64_t AllocationId, int64_t SourceId, uint64_t PC) {
  return AllocationTracker<AllocationKind::LOCAL>::create(
      Start, Length, AllocationId, 0, SourceId, PC);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::GLOBAL)
    ompx_new_global(_AS_PTR(void, AllocationKind::GLOBAL) Start,
                    uint64_t Length, int64_t AllocationId, int64_t SourceId,
                    uint64_t PC) {
  return AllocationTracker<AllocationKind::GLOBAL>::create(
      Start, Length, AllocationId, -1, SourceId, PC);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
__sanitizer_register_host(_AS_PTR(void, AllocationKind::GLOBAL) Start,
                          uint64_t Length, uint64_t Slot, int64_t SourceId) {
  AllocationTracker<AllocationKind::GLOBAL>::create(Start, Length, Slot, Slot,
                                                    SourceId, /*PC=*/0);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void *
ompx_new(void *Start, uint64_t Length, int64_t AllocationId, int64_t SourceId,
         uint64_t PC) {
  if (REAL_PTR_IS_LOCAL(Start))
    return (void *)ompx_new_local((_AS_PTR(void, AllocationKind::LOCAL))Start,
                                  Length, AllocationId, SourceId, PC);
  return (void *)ompx_new_global((_AS_PTR(void, AllocationKind::GLOBAL))Start,
                                 Length, AllocationId, SourceId, PC);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_free_local_n(int32_t N) {
  return AllocationTracker<AllocationKind::LOCAL>::remove_n(N);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
__sanitizer_unregister_host(_AS_PTR(void, AllocationKind::GLOBAL) P) {
  AllocationTracker<AllocationKind::GLOBAL>::remove(P, /*SourceId=*/0);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_free_local(_AS_PTR(void, AllocationKind::LOCAL) P, int64_t SourceId) {
  return AllocationTracker<AllocationKind::LOCAL>::remove(P, SourceId);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_free_global(_AS_PTR(void, AllocationKind::GLOBAL) P, int64_t SourceId) {
  return AllocationTracker<AllocationKind::GLOBAL>::remove(P, SourceId);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_free(void *P, int64_t SourceId, uint64_t PC) {
  bool IsGlobal = IS_GLOBAL(P);
  checkForMagic(IsGlobal, P, SourceId, PC);
  if (IsGlobal)
    return ompx_free_global((_AS_PTR(void, AllocationKind::GLOBAL))P, SourceId);
  return ompx_free_local((_AS_PTR(void, AllocationKind::LOCAL))P, SourceId);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::LOCAL)
    ompx_gep_local(_AS_PTR(void, AllocationKind::LOCAL) P, uint64_t Offset,
                   int64_t SourceId) {
  return AllocationTracker<AllocationKind::LOCAL>::advance(P, Offset, SourceId);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::GLOBAL)
    ompx_gep_global(_AS_PTR(void, AllocationKind::GLOBAL) P, uint64_t Offset,
                    int64_t SourceId) {
  return AllocationTracker<AllocationKind::GLOBAL>::advance(P, Offset,
                                                            SourceId);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void *
ompx_gep(void *P, uint64_t Offset, int64_t SourceId) {
  bool IsGlobal = IS_GLOBAL(P);
  checkForMagic(IsGlobal, P, SourceId, /*PC=*/0);
  if (IsGlobal)
    return (void *)ompx_gep_global((_AS_PTR(void, AllocationKind::GLOBAL))P,
                                   Offset, SourceId);
  return (void *)ompx_gep_local((_AS_PTR(void, AllocationKind::LOCAL))P, Offset,
                                SourceId);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::LOCAL)
    ompx_check_local(_AS_PTR(void, AllocationKind::LOCAL) P, uint64_t Size,
                     uint64_t AccessId, int64_t SourceId, uint64_t PC) {
  return AllocationTracker<AllocationKind::LOCAL>::check(P, Size, AccessId,
                                                         SourceId, PC);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::GLOBAL)
    ompx_check_global(_AS_PTR(void, AllocationKind::GLOBAL) P, uint64_t Size,
                      uint64_t AccessId, int64_t SourceId, uint64_t PC) {
  return AllocationTracker<AllocationKind::GLOBAL>::check(P, Size, AccessId,
                                                          SourceId, PC);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void *
ompx_check(void *P, uint64_t Size, uint64_t AccessId, int64_t SourceId,
           uint64_t PC) {
  bool IsGlobal = IS_GLOBAL(P);
  checkForMagic(IsGlobal, P, SourceId, PC);
  if (IsGlobal)
    return (void *)ompx_check_global((_AS_PTR(void, AllocationKind::GLOBAL))P,
                                     Size, AccessId, SourceId, PC);
  return (void *)ompx_check_local((_AS_PTR(void, AllocationKind::LOCAL))P, Size,
                                  AccessId, SourceId, PC);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::LOCAL)
    ompx_check_with_base_local(_AS_PTR(void, AllocationKind::LOCAL) P,
                               _AS_PTR(void, AllocationKind::LOCAL) Start,
                               uint64_t Length, uint32_t Tag, uint64_t Size,
                               uint64_t AccessId, int64_t SourceId,
                               uint64_t PC) {
  return AllocationTracker<AllocationKind::LOCAL>::checkWithBase(
      P, Start, Length, Tag, Size, AccessId, SourceId, PC);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::GLOBAL)
    ompx_check_with_base_global(_AS_PTR(void, AllocationKind::GLOBAL) P,
                                _AS_PTR(void, AllocationKind::GLOBAL) Start,
                                uint64_t Length, uint32_t Tag, uint64_t Size,
                                uint64_t AccessId, int64_t SourceId,
                                uint64_t PC) {
  return AllocationTracker<AllocationKind::GLOBAL>::checkWithBase(
      P, Start, Length, Tag, Size, AccessId, SourceId, PC);
}

// Void functions for sanitizing a pointer from base offset and without it
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_void_local(_AS_PTR(void, AllocationKind::LOCAL) P, uint64_t Size,
                      uint64_t AccessId, int64_t SourceId, uint64_t PC) {
  return AllocationTracker<AllocationKind::LOCAL>::checkVoid(P, Size, AccessId,
                                                             SourceId, PC);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_void_global(_AS_PTR(void, AllocationKind::GLOBAL) P, uint64_t Size,
                       uint64_t AccessId, int64_t SourceId, uint64_t PC) {
  return AllocationTracker<AllocationKind::GLOBAL>::checkVoid(P, Size, AccessId,
                                                              SourceId, PC);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_void(void *P, uint64_t Size, uint64_t AccessId, int64_t SourceId,
                uint64_t PC) {
  bool IsGlobal = IS_GLOBAL(P);
  checkForMagic(IsGlobal, P, SourceId, PC);
  if (IsGlobal)
    return ompx_check_void_global((_AS_PTR(void, AllocationKind::GLOBAL))P,
                                  Size, AccessId, SourceId, PC);
  return ompx_check_void_local((_AS_PTR(void, AllocationKind::LOCAL))P, Size,
                               AccessId, SourceId, PC);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_with_base_void_local(_AS_PTR(void, AllocationKind::LOCAL) P,
                                _AS_PTR(void, AllocationKind::LOCAL) Start,
                                uint64_t Length, uint32_t Tag, uint64_t Size,
                                uint64_t AccessId, int64_t SourceId,
                                uint64_t PC) {
  return AllocationTracker<AllocationKind::LOCAL>::checkWithBaseVoid(
      P, Start, Length, Tag, Size, AccessId, SourceId, PC);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_with_base_void_global(_AS_PTR(void, AllocationKind::GLOBAL) P,
                                 _AS_PTR(void, AllocationKind::GLOBAL) Start,
                                 uint64_t Length, uint32_t Tag, uint64_t Size,
                                 uint64_t AccessId, int64_t SourceId,
                                 uint64_t PC) {
  return AllocationTracker<AllocationKind::GLOBAL>::checkWithBaseVoid(
      P, Start, Length, Tag, Size, AccessId, SourceId, PC);
}
// End of void functions.

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_range_with_base_global(_AS_PTR(void, AllocationKind::GLOBAL) SCEVMax,
                                  _AS_PTR(void, AllocationKind::GLOBAL) SCEVMin,
                                  _AS_PTR(void, AllocationKind::GLOBAL)
                                      StartAddress,
                                  int64_t AllocationLength, uint32_t Tag,
                                  int64_t AccessTypeSize, int64_t AccessId,
                                  int64_t SourceId, uint64_t PC) {

  return AllocationTracker<AllocationKind::GLOBAL>::checkRangeWithBase(
      SCEVMax, SCEVMin, StartAddress, AllocationLength, Tag, AccessTypeSize,
      AccessId, SourceId, PC);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_range_with_base_local(_AS_PTR(void, AllocationKind::LOCAL) SCEVMax,
                                 _AS_PTR(void, AllocationKind::LOCAL) SCEVMin,
                                 _AS_PTR(void, AllocationKind::LOCAL)
                                     StartAddress,
                                 int64_t AllocationLength, uint32_t Tag,
                                 int64_t AccessTypeSize, int64_t AccessId,
                                 int64_t SourceId, uint64_t PC) {

  return AllocationTracker<AllocationKind::LOCAL>::checkRangeWithBase(
      SCEVMax, SCEVMin, StartAddress, AllocationLength, Tag, AccessTypeSize,
      AccessId, SourceId, PC);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_range_local(_AS_PTR(void, AllocationKind::LOCAL) SCEVMax,
                       _AS_PTR(void, AllocationKind::LOCAL) SCEVMin,
                       int64_t AccessTypeSize, int64_t AccessId,
                       int64_t SourceId, uint64_t PC) {
  return AllocationTracker<AllocationKind::LOCAL>::checkRange(
      SCEVMax, SCEVMin, AccessTypeSize, AccessId, SourceId, PC);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_range_global(_AS_PTR(void, AllocationKind::GLOBAL) SCEVMax,
                        _AS_PTR(void, AllocationKind::GLOBAL) SCEVMin,
                        int64_t AccessTypeSize, int64_t AccessId,
                        int64_t SourceId, uint64_t PC) {
  return AllocationTracker<AllocationKind::GLOBAL>::checkRange(
      SCEVMax, SCEVMin, AccessTypeSize, AccessId, SourceId, PC);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_check_range(void *SCEVMax, void *SCEVMin, int64_t AccessTypeSize,
                 int64_t AccessId, int64_t SourceId, uint64_t PC) {
  bool IsGlobalMax = IS_GLOBAL(SCEVMax);
  bool IsGlobalMin = IS_GLOBAL(SCEVMin);
  checkForMagic(IsGlobalMax, SCEVMax, SourceId, PC);
  checkForMagic(IsGlobalMin, SCEVMin, SourceId, PC);
  if (IsGlobalMax && IsGlobalMin)
    return ompx_check_range_global(
        (_AS_PTR(void, AllocationKind::GLOBAL))SCEVMax,
        (_AS_PTR(void, AllocationKind::GLOBAL))SCEVMin, AccessTypeSize,
        AccessId, SourceId, PC);

  return ompx_check_range_local((_AS_PTR(void, AllocationKind::LOCAL))SCEVMax,
                                (_AS_PTR(void, AllocationKind::LOCAL))SCEVMin,
                                AccessTypeSize, AccessId, SourceId, PC);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::noinline,
  gnu::used, gnu::retain]] char**
ompx_check_with_base_global_vec(char ** Pointers, 
                                char ** Starts,
                                uint64_t *Lengths, uint32_t *Tags,
                                uint64_t *Sizes, uint64_t *AccessIds,
                                int64_t *SourceIds, uint64_t PC,
                                uint64_t ArraySize) {

  //for (int Index = 0; Index < ArraySize; Index++) {
    void* P =  *Pointers;//Pointers[Index];
    void* Start = *Starts; //Starts[Index];
    uint64_t Length = Lengths[0];
    uint32_t Tag = Tags[0];
    uint64_t Size = Sizes[0];
    uint64_t AccessId = AccessIds[0];
    uint64_t SourceId = SourceIds[0];

    //_AS_PTR(void, AllocationKind::GLOBAL)
    //Ptr = AllocationTracker<AllocationKind::GLOBAL>::checkWithBase(
    //   P, Start, Length, Tag, Size, AccessId, SourceId, PC);

    void* Ptr = ompx_check(
                   P, Size, AccessId, SourceId, PC);
    *Pointers = (char*) Ptr;
  //}

  return Pointers;
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::LOCAL)
    ompx_unpack_local(_AS_PTR(void, AllocationKind::LOCAL) P,
                      int64_t SourceId) {
  return AllocationTracker<AllocationKind::LOCAL>::unpack(P, SourceId,
                                                          /*PC=*/0);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] _AS_PTR(void, AllocationKind::GLOBAL)
    ompx_unpack_global(_AS_PTR(void, AllocationKind::GLOBAL) P,
                       int64_t SourceId) {
  return AllocationTracker<AllocationKind::GLOBAL>::unpack(P, SourceId,
                                                           /*PC=*/0);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void *
ompx_unpack(void *P, int64_t SourceId) {
  bool IsGlobal = IS_GLOBAL(P);
  checkForMagic(IsGlobal, P, SourceId, /*PC=*/0);
  if (IsGlobal)
    return (void *)ompx_unpack_global((_AS_PTR(void, AllocationKind::GLOBAL))P,
                                      SourceId);
  return (void *)ompx_unpack_local((_AS_PTR(void, AllocationKind::LOCAL))P,
                                   SourceId);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_lifetime_start(_AS_PTR(void, AllocationKind::LOCAL) P, uint64_t Length) {
  AllocationTracker<AllocationKind::LOCAL>::lifetimeStart(P, Length);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_lifetime_end(_AS_PTR(void, AllocationKind::LOCAL) P, uint64_t Length) {
  AllocationTracker<AllocationKind::LOCAL>::lifetimeEnd(P, Length);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] struct AllocationInfoLocalTy
ompx_get_allocation_info_local(_AS_PTR(void, AllocationKind::LOCAL) P) {
  return AllocationTracker<AllocationKind::LOCAL>::getAllocationInfo(P);
}
[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] struct AllocationInfoGlobalTy
ompx_get_allocation_info_global(_AS_PTR(void, AllocationKind::GLOBAL) P) {
  return AllocationTracker<AllocationKind::GLOBAL>::getAllocationInfo(P);
}

[[clang::disable_sanitizer_instrumentation, gnu::flatten, gnu::always_inline,
  gnu::used, gnu::retain]] void
ompx_leak_check() {
  AllocationTracker<AllocationKind::GLOBAL>::leakCheck();
}

[[gnu::weak, gnu::noinline, gnu::used, gnu::retain]] int64_t
__san_get_location_value() {
  return -1;
}
}

#pragma omp end declare target
