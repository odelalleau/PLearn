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

#include "UnaryVariable.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    UnaryVariable,
    "Variable that has a single parent.",
    ""
);

///////////////////
// UnaryVariable //
///////////////////
UnaryVariable::UnaryVariable(Variable* v, int thelength, int thewidth,
                             bool call_build_):
    Variable(thelength, thewidth, call_build_),
    input(v) 
{
    if (call_build_)
        build_();
}

///////////
// build //
///////////
void UnaryVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void UnaryVariable::build_()
{
    // Nothing to do here.
}

void UnaryVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "input", &UnaryVariable::input, OptionBase::buildoption, 
                  "The parent variable that this one depends on\n");

    inherited::declareOptions(ol);
}

#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif

void UnaryVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    //deepCopyField(input, copies);
    varDeepCopyField(input, copies); // a cause d'une bug du compilateur
}

bool UnaryVariable::markPath()
{
    if(!marked)
        marked = input->markPath();
    return marked;
}


void UnaryVariable::buildPath(VarArray& proppath)
{
    if(marked)
    {
        input->buildPath(proppath);
        //cout<<"add :"<<this->getName()<<endl;
        proppath.append(Var(this));
        clearMark();
    }
}

void UnaryVariable::checkContiguity() const
{
    if(matValue.isNotContiguous() 
       || input->matValue.isNotContiguous()
       || matGradient.isNotContiguous() 
       || input->matGradient.isNotContiguous())
        PLERROR("operation not currently implemented for non-contiguous matrix data");
}

VarArray UnaryVariable::sources() 
{ 
    if (marked)
        return VarArray(0,0);
    marked = true;
    return input->sources(); 
}

VarArray UnaryVariable::random_sources() 
{ 
    if (marked)
        return VarArray(0,0);
    marked = true;
    return input->random_sources(); 
}


VarArray UnaryVariable::ancestors() 
{ 
    if (marked)
        return VarArray(0,0);
    marked = true;
    return input->ancestors() & (VarArray)Var(this);
}


void UnaryVariable::unmarkAncestors()
{ 
    if (marked)
    {
        marked = false;
        input->unmarkAncestors();
    }
}


VarArray UnaryVariable::parents() 
{ 
    if (input->marked)
        return VarArray(0,0);
    else
        return input;
}


void UnaryVariable::resizeRValue()
{
    inherited::resizeRValue();
    if (!input->rvaluedata) input->resizeRValue();
}

//////////////
// setInput //
//////////////
void UnaryVariable::setInput(Var the_input)
{
    this->input = the_input;
    build();
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
