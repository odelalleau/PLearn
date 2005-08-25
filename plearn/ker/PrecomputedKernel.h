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

#ifndef PrecomputedKernel_INC
#define PrecomputedKernel_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;



//!  A kernel that precomputes the kernel matrix as soon as setDataForKernelMatrix is called.
class PrecomputedKernel: public Kernel
{
    typedef Kernel inherited;
  
protected:
    Ker ker; //!<  the real underlying kernel
    TVec<Vec> precomputedK; //!<  the precomputed kernel matrix

    /* *******************
       protected options *
    **********************/

private:
    void build_();
  
public:
    PrecomputedKernel() //: precomputedK(0) 
    {}

    PrecomputedKernel(Ker the_ker): 
        ker(the_ker) //, precomputedK(0) 
    {}

    virtual void build();
    //virtual ~PrecomputedKernel();

    PLEARN_DECLARE_OBJECT(PrecomputedKernel);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //!  This method precomputes and stores all kernel values 
    virtual void setDataForKernelMatrix(VMat the_data);

    virtual void addDataForKernelMatrix(const Vec& newRow)
    { PLERROR("PrecomputedKernel does not manage size varying data vmat. Use SequentialKernel instead."); }
  
    //!  simply forwards to underlying kernel  
    virtual real evaluate(const Vec& x1, const Vec& x2) const; //!<  returns K(x1,x2) 
    virtual real evaluate_i_j(int i, int j) const; //!<  returns evaluate(data(i),data(j))
    virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const; //!<  returns evaluate(data(i),x)
    virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const; //!<  returns evaluate(x,data(i))

protected:
    static void declareOptions(OptionList &ol);
};

DECLARE_OBJECT_PTR(PrecomputedKernel);

} // end of namespace PLearn

#endif


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
