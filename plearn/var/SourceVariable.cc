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
    : inherited(thelength,thewidth),
      build_length(thelength),
      build_width(thewidth)
{}

SourceVariable::SourceVariable(const Vec& v, bool vertical)
    : inherited(vertical ?v.toMat(v.length(),1) :v.toMat(1,v.length()))
{
    build_length = length();
    build_width = width();
}

SourceVariable::SourceVariable(const Mat& m)
    : inherited(m),
      build_length(m.length()),
      build_width(m.width())
{}

void SourceVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "build_length", &SourceVariable::build_length, OptionBase::buildoption, 
                  "The initial length for the variable");

    declareOption(ol, "build_width", &SourceVariable::build_width, OptionBase::buildoption, 
                  "The initial width for the variable");

    declareOption(ol, "random_type", &SourceVariable::random_type, OptionBase::buildoption, 
                  "Type of random generation to use : 'F' = fill with random_a, 'U' = uniform, 'N' = normal");

    declareOption(ol, "random_a", &SourceVariable::random_a, OptionBase::buildoption, 
                  "A first parameter for random generation");

    declareOption(ol, "random_b", &SourceVariable::random_b, OptionBase::buildoption, 
                  "A second parameter for random generation");

    declareOption(ol, "random_clear_first_row", &SourceVariable::random_clear_first_row, OptionBase::buildoption, 
                  "Indicates if we assign 0 to the elements of the first row when doing random generation");

    inherited::declareOptions(ol);
}


////////////
// build_ //
////////////
void SourceVariable::build_()
{ 
    if(build_length>0 && build_width>0)
        resize(build_length, build_width);
}

///////////
// build //
///////////
void SourceVariable::build()
{
    inherited::build();
    build_();
}


void SourceVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
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


void SourceVariable::randomInitialize(PP<PRandom> random_gen)
{
    Mat values = matValue;
    if(random_clear_first_row)
        values = values.subMatRows(1, values.length()-1);

    switch (random_type)
    {
    case 'F':
        values.fill(random_a);
        break;
    case 'U':
        random_gen->fill_random_uniform(values, random_a, random_b);
        break;
    case 'N':
        random_gen->fill_random_normal(values, random_a, random_b);
        break;
    default :
        PLERROR("Invalid random_type in SourceVariable");
        break;
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
