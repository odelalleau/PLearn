// -*- C++ -*-

// ConstrainedSourceVariable.h
//
// Copyright (C) 2008 Pascal Vincent
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

/*! \file ConstrainedSourceVariable.h */


#ifndef ConstrainedSourceVariable_INC
#define ConstrainedSourceVariable_INC

#include <plearn/var/SourceVariable.h>

namespace PLearn {
using namespace std;

/**
 * SourceVariable that after each update, modifies values as needed to satisfy simple constraints.
 *
 * The currently supported constraint is rows having norm 1.
 * i.e. after each update rows are divided by their norm.
 */
class ConstrainedSourceVariable : public SourceVariable
{
    typedef SourceVariable inherited;

public:
    //#####  Public Build Options  ############################################

    int constraint_mode;

public:
    //!  Default constructor for persistence
    ConstrainedSourceVariable()
        :SourceVariable(), 
         constraint_mode(2)
    {}

    ConstrainedSourceVariable(int thelength, int thewidth, int the_constraint_mode=2, bool call_build_ = true)
        :SourceVariable(thelength, thewidth, call_build_), 
         constraint_mode(the_constraint_mode)
    {}

    ConstrainedSourceVariable(const Vec& v, bool vertical=true, int the_constraint_mode=2, bool call_build_ = true)
        :SourceVariable(v, vertical, call_build_), 
         constraint_mode(the_constraint_mode)
    {}

    ConstrainedSourceVariable(const Mat& m, int the_constraint_mode=2, bool call_build_ = true)
        :SourceVariable(m, call_build_), 
         constraint_mode(the_constraint_mode)
    {}

    // The method that will be called after each update to satisfy the constraints.
    virtual void satisfyConstraints();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(ConstrainedSourceVariable);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    
    //#####  'Variable' Interface  ############################################


    // All the 'update' methods are overridden to be followed by a call to satisfyConstraints(),
    // whose purpose is to "correct" the values after an update so that they fulfill the constraints
    virtual bool update(real step_size, Vec direction_vec, real coeff = 1.0, real b = 0.0);
    virtual bool update(Vec step_sizes, Vec direction_vec, real coeff = 1.0, real b = 0.0);
    virtual bool update(real step_size, bool clear=false);
    virtual bool update(Vec new_value);
    virtual void updateAndClear();
    virtual void updateWithWeightDecay(real step_size, real weight_decay,
                                       bool L1, bool clear=true);
    
    
protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //! This does the actual building.
    void build_();
};


DECLARE_OBJECT_PTR(ConstrainedSourceVariable);

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
