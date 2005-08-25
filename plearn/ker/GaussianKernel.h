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

#ifndef GaussianKernel_INC
#define GaussianKernel_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;



//!  returns exp(-norm_2(x1-x2)^2/sigma^2)
class GaussianKernel: public Kernel
{

private:

    typedef Kernel inherited;
		
public:

    //! Build options below.
    bool scale_by_sigma;
    real sigma;

protected:

    real minus_one_over_sigmasquare;  //!< -1 / sigma^2
    real sigmasquare_over_two;        //!<  sigma^2 / 2

    Vec squarednorms; //!<  squarednorms of the rows of the data VMat (data is a member of Kernel)
	 
public:

    //! Default constructor.
    GaussianKernel();
  
    //! Convenient constructor.
    GaussianKernel(real the_sigma);

    PLEARN_DECLARE_OBJECT(GaussianKernel);

    virtual void build();

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //!  This method precomputes the squared norm for all the data to later speed up evaluate methods
    virtual void setDataForKernelMatrix(VMat the_data);
  
    //! This method appends the newRow squared norm to the squarednorms Vec field
    virtual void addDataForKernelMatrix(const Vec& newRow);

    virtual real evaluate(const Vec& x1, const Vec& x2) const; //!<  returns K(x1,x2) 
    virtual real evaluate_i_j(int i, int j) const; //!<  returns evaluate(data(i),data(j))
    virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const; //!<  returns evaluate(data(i),x)
    virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const; //!<  returns evaluate(x,data(i))

    virtual void setParameters(Vec paramvec);

protected:
    static void declareOptions(OptionList& ol);  

    //! Return true if estimating the squared difference of two points x1 and x2
    //! from their squared norms sqn_1 and sqn_2 (and their dot product) might
    //! lead to unsafe numerical approximations.
    //! Specifically, it will return true when
    //! ||x1||^2 > 1e6 and | ||x2||^2 / ||x1||^2 - 1 | < 0.01
    //! i.e. when the two points have a large similar norm.
    inline bool isUnsafe(real sqn_1, real sqn_2) const;

private:

    inline real evaluateFromSquaredNormOfDifference(real sqnorm_of_diff) const;

    inline real evaluateFromDotAndSquaredNorm(real sqnorm_x1, real dot_x1_x2, real sqnorm_x2) const;

    void build_();
};

DECLARE_OBJECT_PTR(GaussianKernel);

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
