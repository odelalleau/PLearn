// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2008 University of Montreal
//

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
 * $Id: TMat_maths.cc 8235 2007-11-07 21:32:01Z nouiz $
 * AUTHORS: Olivier Delalleau
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file PLearn/plearn/math/TMat_maths.cc */

#include <plearn/base/RemoteDeclareMethod.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {

BEGIN_DECLARE_REMOTE_FUNCTIONS

    declareFunction("solveLinearSystemByCholesky",
                    &remote_solveLinearSystemByCholesky,
        (BodyDoc("Solve a linear regression problem using Cholesky "
                 "decomposition."),
         ArgDoc("XtX", 
             "Result of X'X, where X is the input data matrix, with samples "
             "as rows. A constant input can be added to compute a bias term."
             "Weight decay can be added on the diagonal terms (that do not "
             "correspond to the constant input when a bias is computed)."),
         ArgDoc("XtY", 
             "Result of X'Y, where Y is the target data matrix, with samples "
             " as rows."),
         RetDoc ("The weights W of the linear regression, s.t. XW ~= Y")));

END_DECLARE_REMOTE_FUNCTIONS

////////////////////////////////////////
// remote_solveLinearSystemByCholesky //
////////////////////////////////////////
Mat remote_solveLinearSystemByCholesky(const Mat& A, const Mat& B)
{
    Mat weights(A.length(), B.width());
    solveLinearSystemByCholesky(A, B, weights);
    return weights;
}
 
} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
