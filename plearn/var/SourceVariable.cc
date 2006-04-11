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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "SourceVariable.h"

namespace PLearn {
using namespace std;

/** SourceVariable **/

PLEARN_IMPLEMENT_OBJECT(
    SourceVariable,
    "Simple variable that views a source Mat data.",
    ""
);

SourceVariable::SourceVariable(int thelength, int thewidth)
    : inherited(thelength,thewidth)
{}

SourceVariable::SourceVariable(const Vec& v, bool vertical)
    : inherited(vertical ?v.toMat(v.length(),1) :v.toMat(1,v.length()))
{}

SourceVariable::SourceVariable(const Mat& m)
    : inherited(m)
{}

void SourceVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Variable::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(rows_to_update, copies);
}

void SourceVariable::fprop() {} // No input: nothing to fprop
void SourceVariable::bprop() {} // No input: nothing to bprop
void SourceVariable::bbprop() {} // No input: nothing to bbprop
void SourceVariable::rfprop() {} // No input: nothing to rfprop
void SourceVariable::symbolicBprop() {} // No input: nothing to bprop

VarArray SourceVariable::sources() 
{ 
    if (!marked)
    {
        setMark();
        return Var(this);
    }
    return VarArray(0,0);
}

VarArray SourceVariable::random_sources() 
{ 
    if (!marked)
        setMark();
    return VarArray(0,0);
}

VarArray SourceVariable::ancestors() 
{ 
    if (marked)
        return VarArray(0,0);
    setMark();
    return Var(this);
}

void SourceVariable::unmarkAncestors()
{ 
    if (marked)
        clearMark();
}

VarArray SourceVariable::parents()
{ return VarArray(0,0); }

bool SourceVariable::markPath()
{ return marked; }

void SourceVariable::buildPath(VarArray& proppath)
{
    if(marked)
    {
        //cout<<"add:"<<this->getName()<<endl;
        proppath.append(Var(this));
        clearMark();
    }
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
