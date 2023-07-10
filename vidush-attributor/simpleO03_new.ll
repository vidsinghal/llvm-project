; ModuleID = 'simpleO03.bc'
source_filename = "simple.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Foo = type { ptr, i32 }

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local { ptr, i32 } @foo(i32 noundef %val) #0 {
entry:
  %retval = alloca %struct.Foo, align 8
  %val.addr = alloca i32, align 4
  store i32 %val, ptr %val.addr, align 4
  %field1 = getelementptr inbounds %struct.Foo, ptr %retval, i32 0, i32 1
  store i32 10, ptr %field1, align 8
  store ptr %val.addr, ptr %retval, align 8
  %0 = load { ptr, i32 }, ptr %retval, align 8
  ret { ptr, i32 } %0
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %ff = alloca %struct.Foo, align 8
  %0 = getelementptr inbounds { ptr, i32 }, ptr %ff, i32 0, i32 1
  ret i32 0
}

attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none) uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.0 (git@github.com:llvm/llvm-project.git e18e17aa470ef804eb305c27e836cd54c5d7d82e)"}
