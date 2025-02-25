; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=riscv32 -global-isel -verify-machineinstrs < %s \
; RUN:   | FileCheck %s --check-prefix=RV32I
; RUN: llc -mtriple=riscv32 -global-isel -mattr=+zbb -verify-machineinstrs < %s \
; RUN:   | FileCheck %s --check-prefix=RV32ZBB
; RUN: llc -mtriple=riscv64 -global-isel -verify-machineinstrs < %s \
; RUN:   | FileCheck %s --check-prefix=RV64I
; RUN: llc -mtriple=riscv64 -global-isel -mattr=+zbb -verify-machineinstrs < %s \
; RUN:   | FileCheck %s --check-prefix=RV64ZBB

declare i8 @llvm.abs.i8(i8, i1 immarg)
declare i16 @llvm.abs.i16(i16, i1 immarg)
declare i32 @llvm.abs.i32(i32, i1 immarg)
declare i64 @llvm.abs.i64(i64, i1 immarg)

define i8 @abs8(i8 %x) {
; RV32I-LABEL: abs8:
; RV32I:       # %bb.0:
; RV32I-NEXT:    slli a1, a0, 24
; RV32I-NEXT:    srai a1, a1, 31
; RV32I-NEXT:    add a0, a0, a1
; RV32I-NEXT:    xor a0, a0, a1
; RV32I-NEXT:    ret
;
; RV32ZBB-LABEL: abs8:
; RV32ZBB:       # %bb.0:
; RV32ZBB-NEXT:    sext.b a0, a0
; RV32ZBB-NEXT:    neg a1, a0
; RV32ZBB-NEXT:    max a0, a0, a1
; RV32ZBB-NEXT:    ret
;
; RV64I-LABEL: abs8:
; RV64I:       # %bb.0:
; RV64I-NEXT:    slli a1, a0, 24
; RV64I-NEXT:    sraiw a1, a1, 31
; RV64I-NEXT:    addw a0, a0, a1
; RV64I-NEXT:    xor a0, a0, a1
; RV64I-NEXT:    ret
;
; RV64ZBB-LABEL: abs8:
; RV64ZBB:       # %bb.0:
; RV64ZBB-NEXT:    sext.b a0, a0
; RV64ZBB-NEXT:    neg a1, a0
; RV64ZBB-NEXT:    max a0, a0, a1
; RV64ZBB-NEXT:    ret
  %abs = tail call i8 @llvm.abs.i8(i8 %x, i1 true)
  ret i8 %abs
}

define i16 @abs16(i16 %x) {
; RV32I-LABEL: abs16:
; RV32I:       # %bb.0:
; RV32I-NEXT:    slli a1, a0, 16
; RV32I-NEXT:    srai a1, a1, 31
; RV32I-NEXT:    add a0, a0, a1
; RV32I-NEXT:    xor a0, a0, a1
; RV32I-NEXT:    ret
;
; RV32ZBB-LABEL: abs16:
; RV32ZBB:       # %bb.0:
; RV32ZBB-NEXT:    sext.h a0, a0
; RV32ZBB-NEXT:    neg a1, a0
; RV32ZBB-NEXT:    max a0, a0, a1
; RV32ZBB-NEXT:    ret
;
; RV64I-LABEL: abs16:
; RV64I:       # %bb.0:
; RV64I-NEXT:    slli a1, a0, 16
; RV64I-NEXT:    sraiw a1, a1, 31
; RV64I-NEXT:    addw a0, a0, a1
; RV64I-NEXT:    xor a0, a0, a1
; RV64I-NEXT:    ret
;
; RV64ZBB-LABEL: abs16:
; RV64ZBB:       # %bb.0:
; RV64ZBB-NEXT:    sext.h a0, a0
; RV64ZBB-NEXT:    neg a1, a0
; RV64ZBB-NEXT:    max a0, a0, a1
; RV64ZBB-NEXT:    ret
  %abs = tail call i16 @llvm.abs.i16(i16 %x, i1 true)
  ret i16 %abs
}

define i32 @abs32(i32 %x) {
; RV32I-LABEL: abs32:
; RV32I:       # %bb.0:
; RV32I-NEXT:    srai a1, a0, 31
; RV32I-NEXT:    add a0, a0, a1
; RV32I-NEXT:    xor a0, a0, a1
; RV32I-NEXT:    ret
;
; RV32ZBB-LABEL: abs32:
; RV32ZBB:       # %bb.0:
; RV32ZBB-NEXT:    neg a1, a0
; RV32ZBB-NEXT:    max a0, a0, a1
; RV32ZBB-NEXT:    ret
;
; RV64I-LABEL: abs32:
; RV64I:       # %bb.0:
; RV64I-NEXT:    sraiw a1, a0, 31
; RV64I-NEXT:    addw a0, a0, a1
; RV64I-NEXT:    xor a0, a0, a1
; RV64I-NEXT:    ret
;
; RV64ZBB-LABEL: abs32:
; RV64ZBB:       # %bb.0:
; RV64ZBB-NEXT:    sext.w a0, a0
; RV64ZBB-NEXT:    neg a1, a0
; RV64ZBB-NEXT:    max a0, a0, a1
; RV64ZBB-NEXT:    ret
  %abs = tail call i32 @llvm.abs.i32(i32 %x, i1 true)
  ret i32 %abs
}

define i64 @abs64(i64 %x) {
; RV32I-LABEL: abs64:
; RV32I:       # %bb.0:
; RV32I-NEXT:    srai a2, a1, 31
; RV32I-NEXT:    add a0, a0, a2
; RV32I-NEXT:    sltu a3, a0, a2
; RV32I-NEXT:    add a1, a1, a2
; RV32I-NEXT:    add a1, a1, a3
; RV32I-NEXT:    xor a0, a0, a2
; RV32I-NEXT:    xor a1, a1, a2
; RV32I-NEXT:    ret
;
; RV32ZBB-LABEL: abs64:
; RV32ZBB:       # %bb.0:
; RV32ZBB-NEXT:    srai a2, a1, 31
; RV32ZBB-NEXT:    add a0, a0, a2
; RV32ZBB-NEXT:    sltu a3, a0, a2
; RV32ZBB-NEXT:    add a1, a1, a2
; RV32ZBB-NEXT:    add a1, a1, a3
; RV32ZBB-NEXT:    xor a0, a0, a2
; RV32ZBB-NEXT:    xor a1, a1, a2
; RV32ZBB-NEXT:    ret
;
; RV64I-LABEL: abs64:
; RV64I:       # %bb.0:
; RV64I-NEXT:    srai a1, a0, 63
; RV64I-NEXT:    add a0, a0, a1
; RV64I-NEXT:    xor a0, a0, a1
; RV64I-NEXT:    ret
;
; RV64ZBB-LABEL: abs64:
; RV64ZBB:       # %bb.0:
; RV64ZBB-NEXT:    neg a1, a0
; RV64ZBB-NEXT:    max a0, a0, a1
; RV64ZBB-NEXT:    ret
  %abs = tail call i64 @llvm.abs.i64(i64 %x, i1 true)
  ret i64 %abs
}
