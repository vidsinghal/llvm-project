; ModuleID = 'use_first_bytes.bc'
source_filename = "use_first_bytes.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Foo = type { i32, i32, i8 }

@.str = private unnamed_addr constant [15 x i8] c"field a is %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [15 x i8] c"val is now %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [27 x i8] c"field a is now %d in main\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local noundef nonnull align 4 dereferenceable(12) ptr @foo(ptr nocapture nofree noundef %val) #0 {
entry:
  %val.addr = alloca ptr, align 8
  %f = alloca i32, align 4
  store ptr %val, ptr %val.addr, align 8
  %0 = load i32, ptr %val, align 4
  %mul = mul nsw i32 2, %0
  store i32 %mul, ptr %f, align 4
  %1 = load i32, ptr %val, align 4
  %mul1 = mul nsw i32 %1, 10
  store i32 %mul1, ptr %val, align 4
  %2 = load i32, ptr %f, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %2)
  ret ptr %f
}

declare i32 @printf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %argc, ptr nocapture nofree noundef readonly %argv) #0 {
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca ptr, align 8
  %val = alloca i32, align 4
  %f = alloca ptr, align 8
  store ptr %argv, ptr %argv.addr, align 8
  %arrayidx = getelementptr inbounds ptr, ptr %argv, i64 1
  %0 = load ptr, ptr %arrayidx, align 8
  %call = call i32 @atoi(ptr noundef %0) #3
  store i32 %call, ptr %val, align 4
  %call1 = call align 4 ptr @foo(ptr noalias nocapture nofree noundef nonnull align 4 dereferenceable(4) %val) #4
  store ptr %call1, ptr %f, align 8
  %1 = load i32, ptr %val, align 4
  %2 = load i32, ptr %call1, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, ptr %call1, align 4
  %3 = load i32, ptr %val, align 4
  %call2 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %3)
  %4 = load i32, ptr %call1, align 4
  %call4 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i32 noundef %4)
  ret i32 0
}

; Function Attrs: nounwind willreturn memory(read)
declare i32 @atoi(ptr noundef) #2

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind willreturn memory(read) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind willreturn memory(read) }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.0 (git@github.com:vidsinghal/llvm-project.git 820be30ad96591de2d7e651b3ec9cc0253ca6344)"}
