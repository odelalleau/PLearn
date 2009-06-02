// -*- C++ -*-

// LinearCombinationOfScalarVariables.h
//
// Copyright (C) 2009 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file LinearCombinationOfScalarVariables.h */


#ifndef LinearCombinationOfScalarVariables_INC
#define LinearCombinationOfScalarVariables_INC

#include <plearn/var/NaryVariable.h>

namespace PLearn {
using namespace std;

/*! * LinearCombinationOfScalarVariables * */

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class LinearCombinationOfScalarVariables : public NaryVariable
{
    typedef NaryVariable inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    //! The coeficients for the linear combination
    Vec coefs;
    real constant_term;
    bool appendinputs;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor, usually does nothing
    LinearCombinationOfScalarVariables();

    //! Constructor initializing from input variables
    // NaryVariable constructor (inherited) takes a VarArray as
    // argument.  You can either construct from a VarArray (if the
    // number of parent Var is not fixed, for instance), or construct
    // a VarArray from Var by operator &: input1 & input2 &
    // input3. You can also do both, uncomment what you prefer.

    // ### Make sure the implementation in the .cc calls inherited constructor
    // ### and initializes all fields with reasonable default values.
    // LinearCombinationOfScalarVariables(const VarArray& vararray);
    // LinearCombinationOfScalarVariables(Var input1, Var input2, Var input3, ...);

    // ### If your class has parameters, you probably want a constructor that
    // ### initializes them
    LinearCombinationOfScalarVariables(const VarArray& vararray, Vec the_coefs);

    // ### If your parent variables are a meaning and you want to be able to
    // ### access them easily, you can add functions like:
    // Var& First() { return varray[0]; }
    // Var& Second() { return varray[1]; }
    // ...

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
    PLEARN_DECLARE_OBJECT(LinearCombinationOfScalarVariables);

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

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(LinearCombinationOfScalarVariables);

// ### Put here a convenient method for building your variable from other
// ### existing ones, or a VarArray.
// ### e.g., if your class is TotoVariable, with two parameters foo_type foo
// ### and bar_type bar, you could write, depending on your constructor:
// inline Var toto(Var v1, Var v2, Var v3,
//                 foo_type foo=default_foo, bar_type bar=default_bar)
// { return new TotoVariable(v1, v2, v3, foo, bar); }
// ### or:
// inline Var toto(Var v1, Var v2, v3
//                 foo_type foo=default_foo, bar_type bar=default_bar)
// { return new TotoVariable(v1 & v2 & v3, foo, bar); }
// ### or:
// inline Var toto(const VarArray& varray, foo_type foo=default_foo,
//                 bar_type bar=default_bar)
// { return new TotoVariable( varray, foo, bar); }

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
