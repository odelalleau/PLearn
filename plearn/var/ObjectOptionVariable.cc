// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal

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
 * $Id: ObjectOptionVariable.cc 5354 2006-04-11 18:04:57Z tihocan $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "ObjectOptionVariable.h"
#include <plearn/base/TypeTraits.h>
#include <plearn/base/OptionBase.h>
#include <plearn/base/lexical_cast.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

/** ObjectOptionVariable **/

PLEARN_IMPLEMENT_OBJECT(
    ObjectOptionVariable,
    "Variable which wraps an option of an object.",
    "The option can refer to either an integer, a real value, a vector or a\n"
    "matrix within the object: the option will discover its size automatically.\n"
    "Furthermore, upon performing an fprop on the variable, this translates into\n"
    "a changeOption on the object.\n");

ObjectOptionVariable::ObjectOptionVariable()
    : m_log_variable(false),
      m_final_object(0),
      m_option_type(OptionTypeUnknown),
      m_option_int(0),
      m_index(-1)
{ }

ObjectOptionVariable::ObjectOptionVariable(PP<Object> root, const string& option_name,
                                           const string& initial_value, bool log_variable)
    : m_root(root),
      m_option_name(option_name),
      m_initial_value(initial_value),
      m_log_variable(log_variable),
      m_final_object(0),
      m_option_type(OptionTypeUnknown),
      m_option_int(0),
      m_index(-1)
{
    if (m_root)
        build();
}


void ObjectOptionVariable::declareOptions(OptionList& ol)
{
    inherited::declareOptions(ol);
}


void ObjectOptionVariable::build_()
{ 
    PLASSERT( m_root );
    PLASSERT( ! m_option_name.empty() );
    if (! m_initial_value.empty())
        m_root->changeOption(m_option_name, m_initial_value);

    // Infer the option type from the option, set related member variables, and
    // initialize the base-class variable size
    OptionList::iterator option_iter;
    string option_index;
    if (m_root->parseOptionName(m_option_name, m_final_object,
                                option_iter,   option_index))
    {
        PLASSERT( m_final_object );
        void* bound_option = (*option_iter)->getAsVoidPtr(m_final_object);
        string option_type = (*option_iter)->optiontype();
        if (! option_index.empty())
            m_index = lexical_cast<int>(option_index);
        else
            m_index = -1;

        // Set the appropriate pointer depending on the actual option type.  If
        // an index is specified, don't immediately dereference it. Wait until
        // very last minute since the actual memory location the vector points
        // to may change with vector resizes/reallocations.
        if (option_type == TypeTraits<int>::name()) {
            m_option_type = OptionTypeInt;
            m_option_int  = static_cast<int*>(bound_option);
        }
        else if (option_type == TypeTraits<real>::name()) {
            m_option_type = OptionTypeReal;
            m_option_real = static_cast<real*>(bound_option);
        }
        else if (option_type == TypeTraits<Vec>::name()) {
            m_option_type = OptionTypeVec;
            m_option_vec  = static_cast<Vec*>(bound_option);
        }
        else if (option_type == TypeTraits<Mat>::name()) {
            m_option_type = OptionTypeMat;
            m_option_mat  = static_cast<Mat*>(bound_option);
        }
        else
            PLERROR("ObjectOptionVariable::build: unhandled type \"%s\" the option "
                    "\"%s\"; supported types are: { %s, %s, %s, %s }",
                    option_type.c_str(),
                    m_option_name.c_str(),
                    TypeTraits<int>::name() .c_str(),
                    TypeTraits<real>::name().c_str(),
                    TypeTraits<Vec>::name() .c_str(),
                    TypeTraits<Mat>::name() .c_str());
    }

    // Now properly set the size and initial contents of the variable
    switch (m_option_type) {
    case OptionTypeInt:
        PLASSERT( m_option_int );
        inherited::resize(1,1);
        value[0] = *m_option_int;
        break;

    case OptionTypeReal:
        PLASSERT( m_option_real );
        inherited::resize(1,1);
        value[0] = *m_option_real;
        break;

    case OptionTypeVec:
        // Assume row vector
        PLASSERT( m_option_vec );
        inherited::resize(1, m_option_vec->size());
        value << *m_option_vec;
        break;
        
    case OptionTypeMat:
        PLASSERT( m_option_mat );
        inherited::resize(m_option_mat->length(), m_option_mat->width());
        matValue << *m_option_mat;
        break;

    default:
        PLASSERT( false );
    }

    if (m_log_variable)
        compute_log(value, value);
}

void ObjectOptionVariable::build()
{
    inherited::build();
    build_();
}



void ObjectOptionVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(m_root, copies);
    build();
}

void ObjectOptionVariable::fprop()
{
    // Depending on the type of the option, set the proper contents of the option.
    switch (m_option_type) {
    case OptionTypeInt:
        PLASSERT( m_option_int );
        if (m_log_variable)
            *m_option_int = int(exp(value[0]));
        else
            *m_option_int = int(value[0]);
        break;

    case OptionTypeReal:
        PLASSERT( m_option_real );
        if (m_log_variable)
            *m_option_real = exp(value[0]);
        else
            *m_option_real = value[0];
        break;

    case OptionTypeVec:
        PLASSERT( m_option_vec );
        if (m_log_variable)
            exp(value, *m_option_vec);
        else
            *m_option_vec << value;
        break;

    case OptionTypeMat:
        PLASSERT( m_option_mat );
        *m_option_mat << matValue;
        if (m_log_variable) {
            Vec option_vec = m_option_mat->toVec();
            exp(option_vec, option_vec);
        }
        break;

    default:
        PLASSERT( false );
    }

    // Rebuild the object to reflect the change; we should find a smart way to
    // optimize this away in the future
    PLASSERT( m_final_object );
    m_final_object->build();
}

void ObjectOptionVariable::setParents(const VarArray& parents)
{
    PLERROR("ObjectOptionVariable::setParents: this variable has no parents.");
}

void ObjectOptionVariable::bprop() {} // No input: nothing to bprop
void ObjectOptionVariable::bbprop() {} // No input: nothing to bbprop
void ObjectOptionVariable::rfprop() {} // No input: nothing to rfprop
void ObjectOptionVariable::symbolicBprop() {} // No input: nothing to bprop

VarArray ObjectOptionVariable::sources() 
{ 
    if (!marked)
    {
        setMark();
        return Var(this);
    }
    return VarArray(0,0);
}

VarArray ObjectOptionVariable::random_sources() 
{ 
    if (!marked)
        setMark();
    return VarArray(0,0);
}

VarArray ObjectOptionVariable::ancestors() 
{ 
    if (marked)
        return VarArray(0,0);
    setMark();
    return Var(this);
}

void ObjectOptionVariable::unmarkAncestors()
{ 
    if (marked)
        clearMark();
}

VarArray ObjectOptionVariable::parents()
{
    return VarArray(0,0);
}

bool ObjectOptionVariable::markPath()
{
    return marked;
}

void ObjectOptionVariable::buildPath(VarArray& proppath)
{
    if(marked)
    {
        proppath.append(Var(this));
        clearMark();
    }
}


//#####  update*  #############################################################

bool ObjectOptionVariable::update(real step_size, Vec direction_vec, real coeff, real b)
{
    bool ret = inherited::update(step_size, direction_vec, coeff, b);
    fprop();
    return ret;
}

bool ObjectOptionVariable::update(Vec step_sizes, Vec direction_vec, real coeff, real b)
{
    bool ret = inherited::update(step_sizes, direction_vec, coeff, b);
    fprop();
    return ret;
}

bool ObjectOptionVariable::update(real step_size, bool clear)
{
    bool ret = inherited::update(step_size, clear);
    fprop();
    return ret;
}

bool ObjectOptionVariable::update(Vec new_value)
{
    bool ret = inherited::update(new_value);
    fprop();
    return ret;
}

void ObjectOptionVariable::updateAndClear()
{
    inherited::updateAndClear();
    fprop();
}

void ObjectOptionVariable::updateWithWeightDecay(real step_size, real weight_decay,
                                                 bool L1, bool clear)
{
    inherited::updateWithWeightDecay(step_size, weight_decay, L1, clear);
    fprop();
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
