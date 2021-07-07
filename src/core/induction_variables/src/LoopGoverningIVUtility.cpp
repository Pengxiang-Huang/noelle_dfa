/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "IVStepperUtility.hpp"

namespace llvm::noelle{

LoopGoverningIVUtility::LoopGoverningIVUtility (LoopGoverningIVAttribution &attribution)
  : attribution{attribution}
  , conditionValueOrderedDerivation{}
  , flipOperandsToUseNonStrictPredicate{false}
  , flipBrSuccessorsToUseNonStrictPredicate{false} 
{

  /*
   * Fetch the IV
   */
  auto IV = attribution.getInductionVariable();

  condition = attribution.getHeaderCmpInst();
  // TODO: Refer to whichever intermediate value is used in the comparison (known on attribution)
  this->doesOriginalCmpInstHaveIVAsLeftOperand = condition->getOperand(0) == attribution.getIntermediateValueUsedInCompare();

  auto conditionValueDerivationSet = attribution.getConditionValueDerivation();
  for (auto &I : *condition->getParent()) {
    if (conditionValueDerivationSet.find(&I) == conditionValueDerivationSet.end()) continue;
    conditionValueOrderedDerivation.push_back(&I);
  }

  assert(IV.getSingleComputedStepValue() && isa<ConstantInt>(IV.getSingleComputedStepValue()));
  bool isStepValuePositive = cast<ConstantInt>(IV.getSingleComputedStepValue())->getValue().isStrictlyPositive();
  bool conditionExitsOnTrue = attribution.getHeaderBrInst()->getSuccessor(0) == attribution.getExitBlockFromHeader();
  // errs() << "Exit predicate before exit check: " << condition->getPredicate() << "\n";
  auto exitPredicate = conditionExitsOnTrue ? condition->getPredicate() : condition->getInversePredicate();
  // errs() << "Exit predicate before operand check: " << exitPredicate << "\n";
  exitPredicate = doesOriginalCmpInstHaveIVAsLeftOperand ? exitPredicate : CmpInst::getSwappedPredicate(exitPredicate);
  // errs() << "Exit predicate after: " << exitPredicate << "\n";
  this->flipOperandsToUseNonStrictPredicate = !doesOriginalCmpInstHaveIVAsLeftOperand;
  this->flipBrSuccessorsToUseNonStrictPredicate = !conditionExitsOnTrue;
  // errs() << "Flips: " << flipOperandsToUseNonStrictPredicate << " " << flipBrSuccessorsToUseNonStrictPredicate << "\n";

  // condition->print(errs() << "Condition (exits on true: " << conditionExitsOnTrue << "): "); errs() << "\n";

  switch (exitPredicate) {
    case CmpInst::Predicate::ICMP_NE:
      // This predicate is non-strict and will result in either 0 or 1 iteration(s)
      this->nonStrictPredicate = exitPredicate;
      break;
    case CmpInst::Predicate::ICMP_EQ:
      // This predicate is strict and needs to be extended to LTE/GTE to catch jumping past the exiting value
      this->nonStrictPredicate = isStepValuePositive
        ? CmpInst::Predicate::ICMP_UGE : CmpInst::Predicate::ICMP_ULE;
      break;
    case CmpInst::Predicate::ICMP_SLE:
    case CmpInst::Predicate::ICMP_SLT:
    case CmpInst::Predicate::ICMP_ULT:
    case CmpInst::Predicate::ICMP_ULE:
      // This predicate is non-strict. We simply assert that the step value has the expected sign

      // HACK: while it is technically correct to increment with a less than exit condition, yielding 0 or 1 iteration,
      // it would break under assumptions that further recurrences of the IV can be checked on this condition
      // Our parallelization schemes make that assumption, hence the assert here
      assert(!isStepValuePositive && "IV step value is not compatible with exit condition!");
      this->nonStrictPredicate = exitPredicate;
      break;
    case CmpInst::Predicate::ICMP_UGT:
    case CmpInst::Predicate::ICMP_UGE:
    case CmpInst::Predicate::ICMP_SGT:
    case CmpInst::Predicate::ICMP_SGE:
      // This predicate is non-strict. We simply assert that the step value has the expected sign

      // HACK: while it is technically correct to decrement with a greater than exit condition, yielding 0 or 1 iteration,
      // it would break under assumptions that further recurrences of the IV can be checked on this condition
      // Our parallelization schemes make that assumption, hence the assert here
      assert(isStepValuePositive && "IV step value is not compatible with exit condition!");
      this->nonStrictPredicate = exitPredicate;
      break;
  }

  return ;
}

void LoopGoverningIVUtility::updateConditionAndBranchToCatchIteratingPastExitValue(
  CmpInst *cmpToUpdate,
  BranchInst *branchInst,
  BasicBlock *exitBlock
  ) {

  if (flipOperandsToUseNonStrictPredicate) {
    auto opL = cmpToUpdate->getOperand(0);
    auto opR = cmpToUpdate->getOperand(1);
    cmpToUpdate->setOperand(0, opR);
    cmpToUpdate->setOperand(1, opL);
  }
  cmpToUpdate->setPredicate(nonStrictPredicate);

  if (flipBrSuccessorsToUseNonStrictPredicate) {
    branchInst->swapSuccessors();
  }

  return ;
}

void LoopGoverningIVUtility::cloneConditionalCheckFor(
  Value *recurrenceOfIV,
  Value *clonedCompareValue,
  BasicBlock *continueBlock,
  BasicBlock *exitBlock,
  IRBuilder<> &cloneBuilder
  ) {

  /*
   * Create the comparison instruction.
   */
  auto cmpInst = cloneBuilder.CreateICmp(nonStrictPredicate, recurrenceOfIV, clonedCompareValue);

  /*
   * Add the conditional branch
   */
  cloneBuilder.CreateCondBr(cmpInst, exitBlock, continueBlock);

  return ;
}

std::vector<Instruction *> & LoopGoverningIVUtility::getConditionValueDerivation (void) {
  return conditionValueOrderedDerivation;
}

}
