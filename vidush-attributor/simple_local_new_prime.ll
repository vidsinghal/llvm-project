; ModuleID = 'simple_local.bc'
source_filename = "simple_local.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Foo = type { i32, i32, i8 }

@.str = private unnamed_addr constant [15 x i8] c"Field 1 is %d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(i32 noundef %val) #0 {
entry:
  %val.addr = alloca i32, align 4
  %f = alloca %struct.Foo, align 4
  %a = alloca i32, align 4
  store i32 %val, ptr %val.addr, align 4
  store i32 10, ptr %f, align 4
  %0 = load i32, ptr %f, align 4
  store i32 %0, ptr %a, align 4
  %1 = load i32, ptr %a, align 4
  %add = add nsw i32 %1, 1
  store i32 %add, ptr %a, align 4
  %2 = load i32, ptr %a, align 4
  store i32 %2, ptr %f, align 4
  %3 = load i32, ptr %f, align 4
  %add4 = add nsw i32 %3, 1
  store i32 %add4, ptr %f, align 4
  %4 = load i32, ptr %f, align 4
  %add6 = add nsw i32 %4, %val
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %add6)
  ret void
}

declare i32 @printf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind uwtable
define dso_local noundef i32 @main() #0 {
entry:
  call void @foo(i32 noundef 10) #2
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.0 (git@github.com:vidsinghal/llvm-project.git f60cfcd26e7beb71439bf63cb35e3e18eebd9cd2)"}
