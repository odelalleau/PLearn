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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "ExpVariable.h"
#include "LogAddVariable.h"
#include <plearn/math/pl_math.h>   //!< For logadd()
#include <plearn/math/TMat_maths.h>  //!< For apply()
#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
        LogAddVariable,
        "Stable computation of log(exp(input1) + exp(input2)).",
        "This Variable may be used:\n"
        "   - with two inputs of the same sizes, to compute an element-wize\n"
        "     logadd over both input matrices, or\n"
        "   - with one matrix input (input1) and one scalar input (input2)\n"
        "     to compute a vector logadd (i.e. log(exp(a1) + ... + exp(an)))\n"
        "     over the first 'n' rows (or columns, depending on the option\n"
        "     'vector_logadd'), where 'n' is an integer value provided by\n"
        "     the input2 Variable. If input2 is not provided, then the\n"
        "     logadd is performed over all rows/columns of input1.\n"
        "Note that in order to use the second mechanism, one must change the\n"
        "value of the 'vector_logadd' option, to remove any ambiguity, e.g\n"
        "in the case of two scalar inputs."
);

////////////////////
// LogAddVariable //
////////////////////
LogAddVariable::LogAddVariable(Variable* input1, Variable* input2,
                               const string& vl,
                               bool call_build_):
    inherited(input1, input2,
              vl == "none" || vl == "per_row" ? input1->length()
                                              : 1,
              vl == "none" || vl == "per_column" ? input1->width()
                                                 : 1,
              call_build_),
    vector_logadd(vl),
    vector_logadd_id(0)
{
    if (call_build_)
        build_();
}

////////////////////
// declareOptions //
////////////////////
void LogAddVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "vector_logadd", &LogAddVariable::vector_logadd,
                  OptionBase::buildoption, 
        "Must be one of:\n"
        "   - 'none'      : element-wize logadd over the two input matrices\n"
        "   - 'per_column': vector logadd on each column of input1, using\n"
        "                   the first 'n' rows as given by input2\n"
        "   - 'per_row'   : vector logadd on each row of input1, using the\n"
        "                   first 'n' columns as given by input2.");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void LogAddVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void LogAddVariable::build_()
{
    // Transform the string 'vector_logadd' into an integer for faster
    // computations.
    if (vector_logadd == "none")
        vector_logadd_id = 0;
    else if (vector_logadd == "per_row")
        vector_logadd_id = 1;
    else if (vector_logadd == "per_column")
        vector_logadd_id = -1;
    else
        PLERROR("In LogAddVariable::build_ - Invalid value for "
                "'vector_logadd': %s", vector_logadd.c_str());
    
    if (!vector_logadd_id && input1 && input2) {
        if (input1->length() != input2->length() ||
            input1->width() != input2->width())
            PLERROR("In LogAddVariable::build_ - input1 and input2 must "
                    "have the same size");
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LogAddVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(work,     copies);
    deepCopyField(work_ptr, copies);
}
///////////////////
// recomputeSize //
///////////////////
void LogAddVariable::recomputeSize(int& l, int& w) const
{
    if (input1) {
        l = vector_logadd_id >= 0 ? input1->length()
                                  : 1;
        w = vector_logadd_id <= 0 ? input1->width()
                                  : 1;
    } else
        l = w = 0;
}

///////////
// fprop //
///////////
void LogAddVariable::fprop()
{
    if (!vector_logadd_id) {
        // Ugly hack to make it compile with ICC.
#ifdef __INTEL_COMPILER
        PLearn::apply(input1->value, input2->value, value, logadd_for_icc);
#else
        PLearn::apply(input1->value, input2->value, value, logadd);
#endif
    } else if (vector_logadd_id > 0) {
        int n = input2 ? int(round(input2->value[0]))
                       : width();
        for (int i = 0; i < length(); i++) {
            work_ptr = input1->matValue(i);
            if (input2)
                work_ptr = work_ptr.subVec(0, n);
            value[i] = logadd(work_ptr);
        }
    } else {
        int n = input2 ? int(round(input2->value[0]))
                       : length();
        work.resize(n);
        for (int i = 0; i < width(); i++) {
            if (input2)
                work << input1->matValue.subMat(0, i, n, 1);
            else
                work << input1->matValue.column(i);
            value[i] = logadd(work);
        }
    }
}

///////////
// bprop //
///////////
void LogAddVariable::bprop()
{
    if (!vector_logadd_id) {
        // TODO Note that these computations are not efficient at all.
        Vec grad1(nelems());
        grad1 = input1->value - value;
        apply(grad1, grad1, safeexp);
        input1->gradient += grad1%gradient;

        Vec grad2(nelems());
        grad2 = input2->value - value;
        apply(grad2, grad2, safeexp);
        input2->gradient += grad2%gradient;
    } else if (vector_logadd_id > 0) {
        int n = input2 ? int(round(input2->value[0]))
                       : width();
        work.resize(n);
        for (int i = 0; i < length(); i++) {
            work << input1->matValue.subMat(i, 0, 1, n);
            work -= value[i];
            apply(work, work, safeexp);
            multiplyAcc(input1->matGradient.subMat(i, 0, 1, n).toVec(),
                        work, gradient[i]);
        }
    } else {
        int n = input2 ? int(round(input2->value[0]))
                       : length();
        work.resize(n);
        for (int i = 0; i < width(); i++) {
            work << input1->matValue.subMat(0, i, n, 1);
            work -= value[i];
            apply(work, work, safeexp);
            work *= gradient[i];
            input1->matGradient.subMat(0, i, n, 1) += work;
        }
    }
}

///////////////////
// symbolicBprop //
///////////////////
void LogAddVariable::symbolicBprop()
{
    if (!vector_logadd_id) {
        input1->accg(g * (exp(input1)/(exp(input1)+exp(input2))));
        input2->accg(g * (exp(input2)/(exp(input1)+exp(input2))));
    } else {
        PLERROR("In LogAddVariable::symbolicBprop - Not implemented");
    }
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
