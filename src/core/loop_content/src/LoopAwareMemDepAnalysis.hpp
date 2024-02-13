/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef NOELLE_SRC_CORE_LOOP_CONTENT_LOOPAWAREMEMDEPANALYSIS_H_
#define NOELLE_SRC_CORE_LOOP_CONTENT_LOOPAWAREMEMDEPANALYSIS_H_

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/LoopIterationSpaceAnalysis.hpp"
#include "noelle/core/LoopForest.hpp"
#include "noelle/core/LDGGenerator.hpp"

namespace arcana::noelle {

// Perform loop-aware memory dependence analysis to refine the loop PDG
void refinePDGWithLoopAwareMemDepAnalysis(LDGGenerator &ldgAnalysis,
                                          PDG *loopDG,
                                          Loop *l,
                                          LoopStructure *loopStructure,
                                          LoopTree *loops,
                                          LoopIterationSpaceAnalysis *LIDS);

// Refine the loop PDG with SCAF
void refinePDGWithSCAF(PDG *loopDG, Loop *l);

void refinePDGWithLIDS(PDG *loopDG,
                       LoopStructure *loopStructure,
                       LoopTree *loops,
                       LoopIterationSpaceAnalysis *LIDS);

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LOOP_CONTENT_LOOPAWAREMEMDEPANALYSIS_H_
