//===-- ShuffleBB.cpp - Reorder Basic Blocks --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/ShuffleBB.h"

using namespace llvm;

void generateRandomBasicBlockPermutation(
    int numBasicBlocks, Function::BasicBlockListType &TempBasicBlocksList,
    Function::BasicBlockListType &BasicBlocks);

int getNewIndex(std::vector<int> &IntVector);
auto removeAtIndex(int index, Function::BasicBlockListType &BBList);
int decrementIndices(int currIndex, std::vector<int> &vec);

auto removeAtIndex(int index, Function::BasicBlockListType &BBList) {
  int startIndex = 0;
  for (Function::iterator start = BBList.begin(), end = BBList.end();
       start != end; start++) {
    if (startIndex == index) {
      return BBList.remove(start);
    } else {
      startIndex++;
    }
  }
}

int decrementIndices(int currIndex, std::vector<int> &vec){

  for (int &index : vec){
    if (index >= currIndex){
      index = index - 1;
    }
  }

}

PreservedAnalyses ShuffleBasicBlocksPass::run(Function &F,
                                              FunctionAnalysisManager &AM) {

  srand(time(NULL));

  Function::BasicBlockListType &BasicBlocks = F.getBasicBlocks();

  int NumBasicBlocks = BasicBlocks.size() - 1;
  Function::BasicBlockListType TempBasicBlocksList;

  //F.splice();
  
  // int i=0; 
  // for (BasicBlock &B : BasicBlocks){
    
  //   //Cannot change order of the Entry Basic Block.
    
  //   if (i != 0)
  //     BasicBlock *RemovedBlock = BasicBlocks.remove(B);
  //   //TempBasicBlocksList.push_front(RemovedBlock);

  //   i++;

  // }

  //reverse the order of the remaining blocks 
  //for (BasicBlock &B : TempBasicBlocksList){

  //  BasicBlock *RemovedBlock = TempBasicBlocksList.remove(B);
  //  BasicBlocks.push_back(RemovedBlock);

  //}

  //generateRandomBasicBlockPermutation(NumBasicBlocks, TempBasicBlocksList,
  //                                    BasicBlocks);

  return PreservedAnalyses::none();
}

int getNewIndex(std::vector<int> &IntVector) {

  int n = IntVector.size();
  int randomIndex = rand() % n;
  int numAtIndex = IntVector[randomIndex];
  std::swap(IntVector[randomIndex], IntVector[n - 1]);
  IntVector.pop_back();
  return numAtIndex;
}

void generateRandomBasicBlockPermutation(
    int numBasicBlocks, Function::BasicBlockListType &TempBasicBlocksList,
    Function::BasicBlockListType &BasicBlocks) {

  std::vector<int> IntVector(numBasicBlocks);

  for (int i = 0; i < numBasicBlocks; i++) {
    IntVector[i] = i;
  }
  
  while (!IntVector.empty()) {
    int NewIndex = getNewIndex(IntVector);
    errs() << NewIndex << "\n";
    BasicBlocks.push_back(
        removeAtIndex(NewIndex, TempBasicBlocksList));
    decrementIndices(NewIndex, IntVector);
  }
}