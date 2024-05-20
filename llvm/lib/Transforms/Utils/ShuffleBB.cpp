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
//Function::iterator getOriginalAtIndex(int index);
//int decrementIndices(int currIndex, std::vector<int> &vec);

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

// int decrementIndices(int currIndex, std::vector<int> &vec){

//   for (int &index : vec){
//     if (index >= currIndex){
//       index = index - 1;
//     }
//   }

// }


// auto returnIteratorAtIndex(int currIndex, Function &F){

//   int startIndex = 0;
//   for (Function::iterator start = F.begin(), end = F.end(); start != end; start++){
//     if (startIndex == currIndex){
//       return start;
//     }
//     else{
//       startIndex++;
//     }
//   }
// }

PreservedAnalyses ShuffleBasicBlocksPass::run(Function &F,
                                              FunctionAnalysisManager &AM) {

  
  
  // int numBB = F.size();
  // auto FromIt = returnIteratorAtIndex(1, F);
  // auto ToIt = returnIteratorAtIndex()
  // splice(ToIt, F, FromIt);

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

  //for (const BasicBlock &BB : F){
  //  BasicBlocks.push_back(&BB);
  //}

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
  

  //auto SecondBlock = F.end();
  //F.splice(F.end(), &F, F.end());

  //std::copy(BasicBlocks.begin(), BasicBlocks.end(), std::ostream_iterator<const BasicBlock*>(std::cout, " "));
  //std::cout << '\n';

  //std::shuffle(BasicBlocks.begin(), BasicBlocks.end(), g);

  //std::copy(BasicBlocks.begin(), BasicBlocks.end(), std::ostream_iterator<const BasicBlock*>(std::cout, " "));
  //std::cout << '\n';

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
  
  Function::iterator InsertPoint = F.begin();
  //we don't want to insert before the entry block. 
  InsertPoint++;
  while (!IntVector.empty()) {
    int NewIndex = getNewIndex(IntVector);
    errs() << NewIndex << "\n";

    Function::iterator RemovedBasicBlock = getOriginalAtIndex(NewIndex); 
    F.splice(InsertPoint, &F, RemovedBasicBlock);
    InsertPoint = RemovedBasicBlock;
  }
}