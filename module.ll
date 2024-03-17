; ModuleID = 'test_module'
source_filename = "sample.muon"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-darwin24.5.0"

@muon.handle_list = local_unnamed_addr global ptr poison
@muon.number = local_unnamed_addr global i64 poison
@muon.result = local_unnamed_addr global i64 poison

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(write, argmem: none, inaccessiblemem: none)
define void @initialize() local_unnamed_addr #0 {
  store ptr @handle_list, ptr @muon.handle_list, align 8
  store i64 1234, ptr @muon.number, align 8
  store i64 1234, ptr @muon.result, align 8
  ret void
}

declare i64 @handle_list({ i64, ptr })

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(write, argmem: none, inaccessiblemem: none) }
