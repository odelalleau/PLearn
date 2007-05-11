// -*- C++ -*-

// MultiMaxVariable.h
//
// Copyright (C) 2007 Simon Lemieux, Pascal Vincent
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

// Authors: Simon Lemieux, Pascal Vincent

/*! \file MultiMaxVariable.h */


#ifndef MultiMaxVariable_INC
#define MultiMaxVariable_INC

#include <plearn/var/UnaryVariable.h>

namespace PLearn {
using namespace std;

/*! * MultiMaxVariable * */

/**
 * This variables computes a max functions (softmax, log-softmax, hardmax, etc., determined by the field computation_type) 
 * on subvectors of the input, which lenght is defined by the field groupsizes.
 * 
 * ex :
 * if groupsizes = [1,2,3], and computation_type = 'S' (for softmax), and the input vector [1,2,3,4,5,6],
 * the result will be [softmax([1]), softmax([2,3]), softmax([4,5,6])]
 *
 *  note : in that example matValue.width() of the variable must be 1+2+3=6
 * 
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class MultiMaxVariable : public UnaryVariable
{
    typedef UnaryVariable inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    //! group sizes, ex: [2,3,4] = ([x1,x2],[x3,x4,x5],[x6,x7,x8,x9])
//TODO : peut-etre rendre ça private et faire un get set qtion de checker si
// les sizes ont du sens... ou sinon valider qqpart ailleurs que la somme des
// groupsize's donne bien le width de la variable
    TVec<int> groupsizes;
    char computation_type;
    int groupsize;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor, usually does nothing
    MultiMaxVariable()
        :computation_type('S'),
         groupsize(-1)
    {}


    // ### If your class has parameters, you probably want a constructor that
    // ### initializes them
    MultiMaxVariable(Variable* input, TVec<int> groupsizes, char computation_type='S');
    MultiMaxVariable(Variable* input, int groupsizes, char computation_type='S');

    // Your other public member functions go here

    //#####  PLearn::Variable methods #########################################
    // (PLEASE IMPLEMENT IN .cc)
    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();

    // ### These ones are not always implemented
    // virtual void bbprop();
    // virtual void symbolicBprop();
    // virtual void rfprop();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(MultiMaxVariable);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

    //somes utils
    static void softmax_range(Vec &x, Vec &y, int start, int length);
    static void logSoftmax_range(Vec &x, Vec &y, int start, int length);
    static void hardMax_range(Vec &x, Vec &y, int start, int length, bool value = false);

    static void bpropSoftMax(Vec &gradientInput, Vec &gradient, Vec &variableValue, int start, int length);
    static void bpropLogSoftMax(Vec &gradientInput, Vec &gradient, Vec &variableValue, int start, int length);
    static void bpropHardMaxValue(Vec &gradientInput, Vec &gradient, Vec &variableValue, int start, int length);
    

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MultiMaxVariable);

// ### Put here a convenient method for building your variable.
// ### e.g., if your class is TotoVariable, with two parameters foo_type foo
// ### and bar_type bar, you could write:
inline Var MultiMax(Var v, TVec<int> groupsizes, char computation_type)
{ return new MultiMaxVariable(v, groupsizes, computation_type); }

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
