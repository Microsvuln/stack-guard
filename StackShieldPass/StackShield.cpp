#include "StackShield.hpp"
#include "Unsafe.hpp"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include <set>

using namespace llvm;

char StackShield::ID = 0;

bool StackShield::doInitialization(Module &) {
  dependencyGraph = new DependencyGraph;
  firstAllocaInst.clear();
  vulnerableNodes.clear();
  return false;
}

bool StackShield::doFinalization(Module &) {
  std::set<Node *> sources;

  /**
   * Reordering variables
   */
  for (auto node: vulnerableNodes) {
    std::set<Node *> vulnerableSources = node->getSources();
    sources.insert(vulnerableSources.begin(), vulnerableSources.end());
  }

  for (auto node: sources) {
    AllocaInst *allocaInst = node->getAllocaInst();
    Function *func = node->getFunction();
    if (allocaInst == firstAllocaInst[func]) {
      continue;
    }
    // Moving this allocation instruction from LLVM IR to the start
    allocaInst->moveBefore(firstAllocaInst[func]);
    firstAllocaInst[func] = allocaInst;
  }

  dependencyGraph->clear();
  delete dependencyGraph;

  return true;
}

bool StackShield::runOnFunction(Function &F) {

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I!=E; I++) {
    if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(&*I)) {
      // It is allocation instruction - a local variable on stack
      dependencyGraph->get(&F, allocaInst, allocaInst);
      if (firstAllocaInst.count(&F) == 0) {
        firstAllocaInst[&F] = allocaInst;
      }
    }

    if (CallInst *callInst = dyn_cast<CallInst>(&*I)) {
      // Instruction is a function call instruction
      Function *func = callInst->getCalledFunction();
      if (func) {
        // Check for sink
        if (unsafe_functions.find(func->getName().str()) !=
          unsafe_functions.end()) {
          Value *vulnerableVar = callInst->getArgOperand(0);
          Node *vulnerableNode = dependencyGraph->get(&F, vulnerableVar);
          vulnerableNodes.push_back(vulnerableNode);
        } else {
          int i = 0;
          for (auto it = func->arg_begin(); it != func->arg_end(); it++, i++) {
            Value *src = callInst->getArgOperand(i);
            Value *dest = it;
            Node *srcNode = dependencyGraph->get(&F, src);
            Node *destNode = dependencyGraph->get(func, dest);
            srcNode->addDependent(destNode);
          }
        }
      }
    }

    if (GetElementPtrInst *gepInst = dyn_cast<GetElementPtrInst>(&*I)) {
      Value *src = gepInst->getPointerOperand();
      Value *dest = gepInst;
      Node *srcNode = dependencyGraph->get(&F, src);
      Node *destNode = dependencyGraph->get(&F, dest);
      srcNode->addDependent(destNode);
    }

    if (StoreInst *storeInst = dyn_cast<StoreInst>(&*I)) {
      // Instruction is a store instruction
      // Need to update dependencies, if any
      Value *src = storeInst->getValueOperand();
      Value *dest = storeInst->getPointerOperand();
      Node *srcNode = dependencyGraph->get(&F, src);
      Node *destNode = dependencyGraph->get(&F, dest);
      srcNode->addDependent(destNode);
    }

    if (LoadInst *loadInst = dyn_cast<LoadInst>(&*I)) {
      Value *src = loadInst->getPointerOperand();
      Value *dest = loadInst;
      Node *srcNode = dependencyGraph->get(&F, src);
      Node *destNode = dependencyGraph->get(&F, dest);
      srcNode->addDependent(destNode);
    }
  }

  return false;
}

/**
 * Registring our pass
 */
static RegisterPass<StackShield> X("stack-shield", "Stack Shield Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

static void registerStackShieldPass(const PassManagerBuilder &,
                                    PassManagerBase &PM) {
  PM.add(new StackShield());
}

static RegisterStandardPasses
    RegisterMBAPass(PassManagerBuilder::EP_EarlyAsPossible,
                    registerStackShieldPass);