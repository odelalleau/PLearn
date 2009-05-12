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

#include "NaryVariable.h"

namespace PLearn {
using namespace std;

/** NaryVariable **/

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(NaryVariable,
    "A Variable that may take more than two other Variables as input.",
    ""
);

//////////////////
// NaryVariable //
//////////////////
NaryVariable::NaryVariable(const VarArray& the_varray, int thelength,
                           int thewidth, bool call_build_):
    inherited(thelength, thewidth, call_build_),
    varray(the_varray)
{
    if (call_build_)
        build_();
}

////////////////////
// declareOptions //
////////////////////
void NaryVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "varray", &NaryVariable::varray, OptionBase::buildoption, 
                  "The array of parent variables that this one depends on.");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void NaryVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void NaryVariable::build_()
{
    // Nothing to do here.
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NaryVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(varray, copies);
}

void NaryVariable::setParents(const VarArray& parents)
{
    varray = parents;
    sizeprop();
}

bool NaryVariable::markPath()
{
    if(!marked)
    {
        for(int i=0; i<varray.size(); i++)
            if (!varray[i].isNull())
                marked |= varray[i]->markPath();
    }
    return marked;
}


void NaryVariable::buildPath(VarArray& proppath)
{
    if(marked)
    {
        for(int i=0; i<varray.size(); i++)
            if (!varray[i].isNull())
                varray[i]->buildPath(proppath);
        proppath &= Var(this);
        clearMark();
    }
}


VarArray NaryVariable::sources()
{
    VarArray a(0,0);
    if (!marked)
    {
        marked = true;
        for(int i=0; i<varray.size(); i++)
            if (!varray[i].isNull())
                a &= varray[i]->sources();
        //if (a.size()==0)
        //    a &= Var(this);
    }
    return a;
}


VarArray NaryVariable::random_sources()
{
    VarArray a(0,0);
    if (!marked)
    {
        marked = true;
        for(int i=0; i<varray.size(); i++)
            if (!varray[i].isNull())
                a &= varray[i]->random_sources();
    }
    return a;
}


VarArray NaryVariable::ancestors() 
{ 
    VarArray a(0,0);
    if (marked)
        return a;
    marked = true;
    for(int i=0; i<varray.size(); i++)
        if (!varray[i].isNull())
            a &= varray[i]->ancestors();
    a &= Var(this);
    return a;
}


void NaryVariable::unmarkAncestors()
{
    if (marked)
    {
        marked = false;
        for(int i=0; i<varray.size(); i++)
            if (!varray[i].isNull())
                varray[i]->unmarkAncestors();
    }
}


VarArray NaryVariable::parents()
{
    VarArray unmarked_parents;
    for(int i=0; i<varray.size(); i++)
        if (!varray[i].isNull() && !varray[i]->marked)
            unmarked_parents.append(varray[i]);
    return unmarked_parents;
}


void NaryVariable::resizeRValue()
{
    inherited::resizeRValue();
    for (int i=0; i<varray.size(); i++)
        if (!varray[i]->rvaluedata) varray[i]->resizeRValue();
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
