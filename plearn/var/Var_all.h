// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org


/* *******************************************************      
   * $Id: Var_all.h,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef Var_all_INC
#define Var_all_INC

#include "Var.h"
#include "Variable.h"
#include "SourceVariable.h"
#include "UnaryVariable.h"
#include "BinaryVariable.h"
#include "NaryVariable.h"

#include "AbsVariable.h"
#include "AffineTransformVariable.h"
#include "AffineTransformWeightPenalty.h"
#include "ArgmaxVariable.h"
#include "ArgminOfVariable.h"
#include "ArgminVariable.h"
#include "ClassificationLossVariable.h"
#include "ColumnIndexVariable.h"
#include "ColumnSumVariable.h"
#include "ConcatColumnsVariable.h"
#include "ConcatOfVariable.h"
#include "ConcatRowsVariable.h"
#include "ConvolveVariable.h"
#include "CrossEntropyVariable.h"
#include "CutAboveThresholdVariable.h"
#include "CutBelowThresholdVariable.h"
#include "DeterminantVariable.h"
#include "DivVariable.h"
#include "DotProductVariable.h"
#include "DuplicateColumnVariable.h"
#include "DuplicateRowVariable.h"
#include "DuplicateScalarVariable.h"
#include "ElementAtPositionVariable.h"
#include "EqualConstantVariable.h"
#include "EqualScalarVariable.h"
#include "EqualVariable.h"
#include "ErfVariable.h"
#include "ExpVariable.h"
#include "ExtendedVariable.h"
#include "Func.h"
#include "IfThenElseVariable.h"
#include "IndexAtPositionVariable.h"
#include "InterValuesVariable.h"
#include "InvertElementsVariable.h"
#include "IsAboveThresholdVariable.h"
#include "IsLargerVariable.h"
#include "IsSmallerVariable.h"
#include "LeftPseudoInverseVariable.h"
#include "LogAddVariable.h"
#include "LogSoftmaxVariable.h"
#include "LogSumVariable.h"
#include "LogVariable.h"
#include "MatRowVariable.h"
#include "MatrixAffineTransformFeedbackVariable.h"
#include "MatrixAffineTransformVariable.h"
#include "MatrixElementsVariable.h"
#include "MatrixInverseVariable.h"
#include "MatrixOneHotSquaredLoss.h"
#include "MatrixSoftmaxLossVariable.h"
#include "MatrixSoftmaxVariable.h"
#include "MatrixSumOfVariable.h"
#include "Max2Variable.h"
#include "MaxVariable.h"
#include "MinVariable.h"
#include "MiniBatchClassificationLossVariable.h"
#include "MinusColumnVariable.h"
#include "MinusRowVariable.h"
#include "MinusScalarVariable.h"
#include "MinusTransposedColumnVariable.h"
#include "MinusVariable.h"
#include "MulticlassLossVariable.h"
#include "NegateElementsVariable.h"
#include "OneHotSquaredLoss.h"
#include "OneHotVariable.h"
#include "PLogPVariable.h"
#include "PlusColumnVariable.h"
#include "PlusConstantVariable.h"
#include "PlusRowVariable.h"
#include "PlusScalarVariable.h"
#include "PlusVariable.h"
#include "PowVariable.h"
#include "PowVariableVariable.h"
#include "ProductTransposeVariable.h"
#include "ProductVariable.h"
#include "ReshapeVariable.h"
#include "RightPseudoInverseVariable.h"
#include "RowAtPositionVariable.h"
#include "RowSumVariable.h"
#include "SigmoidVariable.h"
#include "SignVariable.h"
#include "SoftmaxLossVariable.h"
#include "SoftmaxVariable.h"
#include "SoftplusVariable.h"
#include "SquareRootVariable.h"
#include "SquareVariable.h"
#include "SubMatTransposeVariable.h"
#include "SubMatVariable.h"
#include "SubsampleVariable.h"
#include "SumOfVariable.h"
#include "SumSquareVariable.h"
#include "SumVariable.h"
#include "TanhVariable.h"
#include "TimesColumnVariable.h"
#include "TimesConstantVariable.h"
#include "TimesRowVariable.h"
#include "TimesScalarVariable.h"
#include "TimesVariable.h"
#include "TransposeProductVariable.h"
#include "UnequalConstantVariable.h"
#include "VarArrayElementVariable.h"
#include "VarColumnsVariable.h"
#include "VarElementVariable.h"
#include "VarMeasurer.h"
#include "VarRowVariable.h"
#include "VarRowsVariable.h"
#include "VecElementVariable.h"
#include "WeightedSumSquareVariable.h"

#include "Var_utils.h"

#endif

