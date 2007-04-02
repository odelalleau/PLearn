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
 * $Id: ObjectOptionVariable.h 4493 2005-11-13 03:48:04Z morinf $
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef ObjectOptionVariable_INC
#define ObjectOptionVariable_INC

#include "VarArray.h"

namespace PLearn {
using namespace std;

/**
 *  Variable which wraps an option of an object.
 *
 *  This variable serves to mirror its contents onto an option of a PLearn
 *  Object.  The option can refer to either an integer, a real value, a vector
 *  or a matrix within the object: the option will discover its size
 *  automatically.  Furthermore, upon performing an fprop on the variable, this
 *  translates into a changeOption on the object.
 */
class ObjectOptionVariable : public Variable
{
    typedef Variable inherited;

public:
    //#####  Public Build Options  ############################################

    //! Pointer to the object we are wrapping an option of.  This can also be
    //! a 'root' object in the case of a complex option such as 'A.B.C'.
    PP<Object> m_root;

    //! Name of the option we are wrapping within the object
    string m_option_name;

    /**
     *  Optional initial value that should be set into the option.  This is a
     *  string that is set with 'changeOption'.  Note that this exact initial
     *  value is initially set into the option, regardless of the setting for
     *  'log_variable'.
     */
    string m_initial_value;

public:
    //!  Default constructor for persistence
    ObjectOptionVariable();
    ObjectOptionVariable(PP<Object> root, const string& option_name,
                         const string& initial_value="");

    
    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(ObjectOptionVariable);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    
    //#####  'Variable' Interface  ############################################

    virtual void setParents(const VarArray& parents);
    // { PLERROR("In Variable::setParents  trying to set parents of a ObjectOptionVariable..."); }

    virtual bool markPath();
    virtual void buildPath(VarArray& proppath);
  
    virtual void fprop();
    virtual void bprop();
    virtual void bbprop();
    virtual void symbolicBprop();
    virtual void rfprop();
    virtual VarArray sources();
    virtual VarArray random_sources();
    virtual VarArray ancestors();
    virtual void unmarkAncestors();
    virtual VarArray parents();
    bool isConstant() { return true; }

    void printInfo(bool print_gradient) { 
        pout << getName() << "[" << (void*)this << "] " << *this << " = " << value;
        if (print_gradient) pout << " gradient=" << gradient;
        pout << endl; 
    }

    // All the 'update' methods are overridden to be followed by an fprop(),
    // whose purpose is to reflect the contents of the updated Variable value
    // within the mirrorred object-option.
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

    //! Pointer to the final enclosing object of the option
    Object* m_final_object;
    
    //! Encodes the type of the option in order to perform appropriate cast
    //! upon fpropping
    enum OptionType {
        OptionTypeUnknown,
        OptionTypeInt,
        OptionTypeReal,
        OptionTypeVec,
        OptionTypeMat
    } m_option_type;

    //! Bound semi-typed pointer to the object the option stands for
    union {
        int*  m_option_int;
        real* m_option_real;
        Vec*  m_option_vec;
        Mat*  m_option_mat;
    };

    //! Index into a vector if the option contains []
    int m_index;
};


DECLARE_OBJECT_PTR(ObjectOptionVariable);

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
