; ModuleID = 'simple.c'
source_filename = "simple.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Foo = type { i32, i8, ptr }

; Function Attrs: nounwind optsize uwtable
define dso_local { i64, ptr } @foo(i32 noundef %val) #0 {
entry:
  %retval = alloca %struct.Foo, align 8
  %val.addr = alloca i32, align 4
  store i32 %val, ptr %val.addr, align 4, !tbaa !5
  %field1 = getelementptr inbounds %struct.Foo, ptr %retval, i32 0, i32 0
  store i32 10, ptr %field1, align 8, !tbaa !9
  %field3 = getelementptr inbounds %struct.Foo, ptr %retval, i32 0, i32 2
  store ptr %val.addr, ptr %field3, align 8, !tbaa !12
  %0 = load { i64, ptr }, ptr %retval, align 8
  ret { i64, ptr } %0
}

; Function Attrs: nounwind optsize uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca i32, align 4
  %ff = alloca %struct.Foo, align 8
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %a) #2
  store i32 20, ptr %a, align 4, !tbaa !5
  call void @llvm.lifetime.start.p0(i64 16, ptr %ff) #2
  %0 = load i32, ptr %a, align 4, !tbaa !5
  %call = call { i64, ptr } @foo(i32 noundef %0) #3
  %1 = getelementptr inbounds { i64, ptr }, ptr %ff, i32 0, i32 0
  %2 = extractvalue { i64, ptr } %call, 0
  store i64 %2, ptr %1, align 8
  %3 = getelementptr inbounds { i64, ptr }, ptr %ff, i32 0, i32 1
  %4 = extractvalue { i64, ptr } %call, 1
  store ptr %4, ptr %3, align 8
  call void @llvm.lifetime.end.p0(i64 16, ptr %ff) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %a) #2
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind optsize uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nounwind }
attributes #3 = { optsize }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 17.0.0 (git@github.com:llvm/llvm-project.git e18e17aa470ef804eb305c27e836cd54c5d7d82e)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !6, i64 0}
!10 = !{!"Foo", !6, i64 0, !7, i64 4, !11, i64 8}
!11 = !{!"any pointer", !7, i64 0}
!12 = !{!10, !11, i64 8}
