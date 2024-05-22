//===-- ShuffleBB.cpp - Reorder Basic Blocks --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/ShuffleBB.h"

using namespace llvm;

int getNewIndex(std::vector<int> &IntVector);

Function::iterator ShuffleBasicBlocksPass::getOriginalAtIndex(int index) {
  int startIndex = 0;
  for (Function::iterator block : BasicBlocks) {
    if (startIndex == index) {
      return block;
    } else {
      startIndex++;
    }
  }
}

PreservedAnalyses ShuffleBasicBlocksPass::run(Function &F,
                                              FunctionAnalysisManager &AM) {

  
  srand(time(NULL));
  //preserve original basic blocks order. 
  int i = 0;
  for (Function::iterator start = F.begin(); start != F.end(); start++){
    if (i > 0)
      BasicBlocks.push_back(start);
    
    errs() << start->getNameOrAsOperand() << "\n";
    i++;
  }
  
  errs() << "\n";
  generateRandomBasicBlockPermutation(F);
  errs() << "\n";

  for (Function::iterator start = F.begin(); start != F.end(); start++){
    errs() << start->getNameOrAsOperand() << "\n";
  }

  BasicBlocks.clear();
  errs() << "\n";

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

void ShuffleBasicBlocksPass::generateRandomBasicBlockPermutation(Function &F) {
  
  int numBasicBlocks = F.size() - 1;
  std::vector<int> IntVector(numBasicBlocks);
  
  for (int i = 0; i < numBasicBlocks; i++) {
    IntVector[i] = i;
  }
  
  //always insert new blocks from the tail of the list. 
  Function::iterator InsertPoint = F.end();
  while (!IntVector.empty()) {
    int NewIndex = getNewIndex(IntVector);
    errs() << NewIndex << "\n";

    Function::iterator RemovedBasicBlock = getOriginalAtIndex(NewIndex); 
    F.splice(InsertPoint, &F, RemovedBasicBlock);
  }
}