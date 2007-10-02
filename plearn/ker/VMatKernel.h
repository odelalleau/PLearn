// -*- C++ -*-

// VMatKernel.h
//
// Copyright (C) 2005 Benoit Cromp 
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
 * $Id$ 
 ******************************************************* */

// Authors: Benoit Cromp

/*! \file VMatKernel.h */


#ifndef VMatKernel_INC
#define VMatKernel_INC

#include "Kernel.h"
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;

class VMatKernel: public Kernel
{

private:

    typedef Kernel inherited;
  
protected:

    // *********************
    // * Protected options *
    // *********************

    // *************************
    // * Public learnt options *
    // *************************

    Vec train_indices;
    
public:

    // ************************
    // * Public build options *
    // ************************ 

    VMat source;


    // ### declare public option fields (such as build options) here
    // ...

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    // Make sure the implementation in the .cc initializes all fields to
    // reasonable default values.
    VMatKernel();

    // ******************
    // * Kernel methods *
    // ******************

private: 

    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();

protected: 
  
    //! Declares this class' options.
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods.
    // If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS.
    PLEARN_DECLARE_OBJECT(VMatKernel);

    // ************************
    // **** Kernel methods ****
    // ************************

    //! Compute K(x1,x2).
    virtual real evaluate(const Vec& x1, const Vec& x2) const;
    virtual real evaluate(real x1, real x2) const;
    virtual real evaluate(int x1, int x2) const;

    //! Overridden methods
    virtual void setDataForKernelMatrix(VMat the_data);
    virtual void addDataForKernelMatrix(const Vec& newRow);
    virtual real evaluate_i_j(int i, int j) const; //! Compute K(xi,xj) on training samples
    virtual real evaluate_i_x(int i, const Vec& x, real squared_norm_of_x=-1) const;
    virtual real evaluate_x_i(const Vec& x, int i, real squared_norm_of_x=-1) const;
    virtual void computeGramMatrix(Mat K) const;   //! Overridden for more efficiency
    
    //! Maybe later someone wants to implement a special behaviours with these functions
    // virtual void setParameters(Vec paramvec);
    // virtual Vec getParameters() const;
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(VMatKernel);
  
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
