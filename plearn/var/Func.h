// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * AUTHORS: Pascal Vincent & Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef FUNCTION_INC
#define FUNCTION_INC

#include <plearn/base/Object.h>
#include <plearn/base/general.h>
#include <plearn/base/PP.h>
#include <plearn/math/TMat.h>
#include <plearn/vmat/VMat.h>
#include "Variable.h"
#include "VarArray.h"

namespace PLearn {
using namespace std;


class Function;
class Func: public PP<Function>
{
public:
    Func();

    Func(Function* f);
 
    Func(const VarArray& the_inputs, const VarArray& the_outputs);

    Func(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs);

    Vec operator()(const Vec& input) const;

    real operator()(const Vec& input1, const Vec& input2) const;

/*!     builds a whole new Var graph modeled after the current one
  but starting from new_inputs (instead of inputs) 
  the resulting new_outputs var array is returned by the call
*/
    VarArray operator()(const VarArray& new_inputs) const;
};

class Function: public Object
{
public:

    // Build options:
    mutable VarArray inputs;
    mutable VarArray parameters;  //!< nonInputSources
    mutable VarArray outputs;

    // Other variables
    int inputsize;
    int outputsize;
    mutable VarArray fproppath;
    VarArray bproppath;
    VarArray parentspath; //!<  path on which to do a fprop to update the values of all the non-input direct parents on the fproppath (this will be called by method recomputeParents() )

    Func df; //!<  remembers the symbolic derivative

private: 
    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();

protected: 
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);


public:
    Function();
    Function(const VarArray& the_inputs, const VarArray& the_outputs);
    Function(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs);
    //void bprop(VarArray& parameters_to_optimize);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods
    //  If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
    typedef Object inherited;
    PLEARN_DECLARE_OBJECT(Function);

  
    // **************************
    // **** Function methods ****
    // **************************

    void fprop(const Vec& in, const Vec& out) const;
    void fprop(const Array<Vec>& in, const Array<Vec>& out) const;

    void sizefprop(const Vec& in, const Vec& out) const;
    void sizefprop(const Array<Vec>& in, const Array<Vec>& out) const;

/*!   when put_gradient_on_first_element_only, a gradient of 1 is put
  in only the first element of the output gradient this is a hack
  that is useful for having a SumOfVariable computing several
  costs in parallel, but backpropagating only through the first
*/
    void fbprop(const Vec& in, const Vec& out, const Vec& input_gradient, const Vec& output_gradient);
    void fbprop(const Array<Vec>& in, const Array<Vec>& out, 
                const Array<Vec>& input_gradient, const Array<Vec>& output_gradient);
    
    void sizefbprop(const Vec& in, const Vec& out, const Vec& input_gradient, const Vec& output_gradient);
    void sizefbprop(const Array<Vec>& in, const Array<Vec>& out, 
                const Array<Vec>& input_gradient, const Array<Vec>& output_gradient);
    
    //!  given input, compute output, gradient (=doutput/dinput) and hessian (=d^2output/dinput^2)
    void fbbprop(const Vec& in, const Vec& out, const Vec& gradient, const Mat& hessian);
    //!  same thing but accumulate output, gradient and hessian
    void fbbpropAcc(const Vec& in, const Vec& out, const Vec& gradient, const Mat& hessian);
  
    void rfprop(const Vec& in, const Vec& out, const Vec& input_rvalue, const Vec& output_rvalue, bool do_fprop=true);
  
    void recomputeParents(); //!<  recomputes the value of all the vars that influence the output but do not depend on the declared inputs (shared parameters for instance...)

/*!     Returns a Func that will compute the derivative of this function's output 
  (expected to be a scalar) relative to the given input (of length inputsize)
  The computed derivative has (logically) also a length of inputsize. 
  (This call uses symbolic gradient computation)
*/
    Func differentiate();

    Vec operator()(const Vec& input) const;
    real operator()(const Vec& input1, const Vec& input2) const;

/*!       builds a whole new Var graph modeled after the current one but
  starting from new_inputs (instead of inputs) the resulting
  new_outputs var array is returned by the call All variables on the
  direct path from inputs to outputs are cloned but the parents of the
  cloned variables are the same as the originals (parents of the
  variables in the path are shared).
*/

    VarArray operator()(const VarArray& new_inputs) const;

    //!  take the values given in the in Vec
    void verifyGradient(const Vec& in, real step=0.01, int which_component=0);

    void verifyHessian(const Vec& in, real step=0.01);

    //!  take the values randomly between minval and maxval
    void verifyGradient(real minval, real maxval, real step=0.01,  int which_component=0);

    //!  take the current values of the inputs variables
    void verifyGradient(real step=0.01, int which_component=0);  

/*!     Checks that the gradient computed by a bprop on the function 
  and the gradient computed by a fprop on the symbolic derivative
  of the function give the same results
*/
    void verifySymbolicGradient(const Vec& in);

    // This is developed to make sure that the code of rfprop is correct.
    // The value of H * r computed by rfprop or Hessian should be the same.
    void verifyrfprop(const Vec& in, real step=0.01);
};

DECLARE_OBJECT_PTR(Function);
DECLARE_OBJECT_PP(Func, Function);

Func operator/(Func f, real value);

template <> void deepCopyField(Func& field, CopiesMap& copies);

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
