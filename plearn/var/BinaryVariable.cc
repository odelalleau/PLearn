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

#include "BinaryVariable.h"

namespace PLearn {

using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    BinaryVariable,
    "Variable that takes two other variables as input.",
    ""
);

////////////////////
// BinaryVariable //
////////////////////
BinaryVariable::BinaryVariable(Variable* v1, Variable* v2, int thelength,
                               int thewidth, bool call_build_):
    inherited(thelength, thewidth, call_build_), 
    input1(v1),
    input2(v2) 
{
    if (input1)
        input1->disallowPartialUpdates();
    if (input2)
        input2->disallowPartialUpdates();
    if (call_build_)
        build_();
}

////////////////////
// declareOptions //
////////////////////
void BinaryVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "input1", &BinaryVariable::input1, OptionBase::buildoption, 
                  "The first parent variable that this one depends on\n");

    declareOption(ol, "input2", &BinaryVariable::input2, OptionBase::buildoption, 
                  "The second parent variable that this one depends on\n");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void BinaryVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void BinaryVariable::build_() {}

////////////////
// setParents //
////////////////
void BinaryVariable::setParents(const VarArray& parents)
{
    if(parents.length() != 2)
        PLERROR("In BinaryVariable::setParents  VarArray length must be 2;"
                " you are trying to set %d parents for this BinaryVariable...", parents.length());

    input1= parents[0];
    input2= parents[1];
    // int dummy_l, dummy_w;
    //recomputeSize(dummy_l, dummy_w);
    sizeprop();
}

#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void BinaryVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    varDeepCopyField(input1, copies);
    varDeepCopyField(input2, copies);
}

//////////////
// markPath //
//////////////
bool BinaryVariable::markPath()
{
    if(!marked)
        marked = (input1 ? input1->markPath() : false) |
                 (input2 ? input2->markPath() : false);
    return marked;
}

///////////////
// buildPath //
///////////////
void BinaryVariable::buildPath(VarArray& proppath)
{
    if(marked)
    {
        if (input1)
            input1->buildPath(proppath);
        if (input2)
            input2->buildPath(proppath);
        proppath &= Var(this);
        //cout<<"proppath="<<this->getName()<<endl;
        clearMark();
    }
}

/////////////
// sources //
/////////////
VarArray BinaryVariable::sources() 
{ 
    if (marked)
        return VarArray(0,0);
    marked = true;
    if (!input1)
        return input2->sources();
    if (!input2)
        return input1->sources();
    return input1->sources() & input2->sources();
}

////////////////////
// random_sources //
////////////////////
VarArray BinaryVariable::random_sources() 
{ 
    if (marked)
        return VarArray(0,0);
    marked = true;
    return input1->random_sources() & input2->random_sources(); 
}

///////////////
// ancestors //
///////////////
VarArray BinaryVariable::ancestors() 
{ 
    if (marked)
        return VarArray(0,0);
    marked = true;
    return (input1 ? input1->ancestors() : VarArray(0, 0)) &
           (input2 ? input2->ancestors() : VarArray(0, 0)) & Var(this);
}

/////////////////////
// unmarkAncestors //
/////////////////////
void BinaryVariable::unmarkAncestors()
{ 
    if (marked)
    {
        marked = false;
        if (input1)
            input1->unmarkAncestors();
        if (input2)
            input2->unmarkAncestors();
    }
}

/////////////
// parents //
/////////////
VarArray BinaryVariable::parents() 
{ 
    VarArray unmarked_parents;
    if (input1 && !input1->marked)
        unmarked_parents.append(input1);
    if (input2 && !input2->marked)
        unmarked_parents.append(input2);
    return unmarked_parents;
}

//////////////////
// resizeRValue //
//////////////////
void BinaryVariable::resizeRValue()
{
    inherited::resizeRValue();
    if (input1 && !input1->rvaluedata)
        input1->resizeRValue();
    if (input2 && !input2->rvaluedata)
        input2->resizeRValue();
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
