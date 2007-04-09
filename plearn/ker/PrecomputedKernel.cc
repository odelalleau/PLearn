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

#include "PrecomputedKernel.h"

namespace PLearn {
using namespace std;



// ** PrecomputedKernel **

PLEARN_IMPLEMENT_OBJECT(PrecomputedKernel, "ONE LINE DESCR", "NO HELP");

// PrecomputedKernel::~PrecomputedKernel()
// {
//   if(precomputedK)
//     delete[] precomputedK;
// }

void PrecomputedKernel::build_()
{}


void PrecomputedKernel::build()
{
    inherited::build();
    build_();
}



void PrecomputedKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(ker, copies);
    deepCopyField(precomputedK, copies);
}


// Old
// void PrecomputedKernel::setDataForKernelMatrix(VMat the_data)
// { 
//   Kernel::setDataForKernelMatrix(the_data);
//   ker->setDataForKernelMatrix(the_data);
  
//   if(precomputedK)
//     delete[] precomputedK;
//   int l = data.length();
//   precomputedK = new float(l*l);
//   float* Kdata = precomputedK;
//   for(int i=0; i<l; i++)
//     {
//       cerr << "Precomputing Kernel Matrix Row " << i << " of " << l << " ..." << endl;
//       for(int j=0; j<l; j++)
//         Kdata[j] = (float)ker->evaluate_i_j(i,j);
//       Kdata += l;
//     }
// }

/*
  Given that the matrix is symetric, we
  reduce the computation from n^2 to (n^2)/2 + n/2 calls to evaluate_i_j
*/
void PrecomputedKernel::setDataForKernelMatrix(VMat the_data)
{   
    Kernel::setDataForKernelMatrix(the_data);
    ker->setDataForKernelMatrix(the_data);
  
    int len = data.length();
    precomputedK.resize(len); //TVec of lines!!!
    for(int i=0; i < len; i++)
    {
        precomputedK[i].resize(len);
    
        for(int j=0; j < len; j++)
        {
            if(is_symmetric && j<i)
                precomputedK[i][j] = precomputedK[j][i];
            else
                precomputedK[i][j] = ker->evaluate_i_j(i,j);
        }
    }
}


real PrecomputedKernel::evaluate(const Vec& x1, const Vec& x2) const
{ return ker->evaluate(x1,x2); }


real PrecomputedKernel::evaluate_i_j(int i, int j) const
{ 
#ifdef BOUNDCHECK
    if(precomputedK.isNull())
        PLERROR("In PrecomputedKernel::evaluate_i_j data must first be set with setDataForKernelMatrix");
    else if(i<0 || j<0 || i>=data.length() || j>=data.length())
        PLERROR("In PrecomputedKernel::evaluate_i_j i (%d) and j (%d) must be between 0 and data.length() (%d)",
                i, j, data.length());
#endif
    return precomputedK[i][j];//[i*data.length()+j];
}


real PrecomputedKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const 
{ return ker->evaluate_i_x(i,x,squared_norm_of_x); }


real PrecomputedKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
{ return ker->evaluate_x_i(x,i,squared_norm_of_x); }

void PrecomputedKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "ker", &PrecomputedKernel::ker, OptionBase::buildoption,
                  "The underlying kernel.");    
    inherited::declareOptions(ol);
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
