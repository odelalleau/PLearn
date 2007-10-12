// -*- C++ -*-

// MultiSampleVariable.cc
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

/*! \file MultiSampleVariable.cc */


#include "MultiSampleVariable.h"

namespace PLearn {
using namespace std;

/** MultiSampleVariable **/

PLEARN_IMPLEMENT_OBJECT(
    MultiSampleVariable,
    "Different max variables done on separate groups of the input",
    "This variables samples" 
    "\non subvectors of the input, which lengths are defined by the field groupsize"
    "\n"
    );


//! Constructor

MultiSampleVariable::MultiSampleVariable(Variable* input, int groupsize)
    : inherited(input, input->length(), input->width()),
      groupsize(groupsize),
      random_gen(NULL)
{
    build_();
}

void MultiSampleVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length();
        w = input->width() ;
    } else
        l = w = 0;
}

// ### computes value from input's value
void MultiSampleVariable::fprop()
{
    int k;
    Mat inputValue = input->matValue;

    Vec inputValue_n;
    Vec value_n;

    for(int n=0; n<inputValue.length(); n++)
    {
        k=0;
        inputValue_n = inputValue(n);
        value_n = matValue(n);

        //we set all values to 0. before sampling "ones"
        for (int i=0; i<value_n.length(); i++)
            value_n[i]=0.;
        
        while ( k < this->width() )
        {            
            sample_range(inputValue_n, value_n, k, groupsize);         
            k+=groupsize;
        }
    }
}

// ### computes input's gradient from gradient
void MultiSampleVariable::bprop()
{}    
// ### You can implement these methods:
// void MultiSampleVariable::bbprop() {}
// void MultiSampleVariable::symbolicBprop() {}
// void MultiSampleVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void MultiSampleVariable::build()
{
    inherited::build();
    build_();
}

void MultiSampleVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    //deepCopyField(groupsizes, copies);
    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);   
}

void MultiSampleVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    //declareOption(ol, "groupsizes", &MultiSampleVariable::groupsizes,
    //              OptionBase::buildoption,
    //              "this tells how to \"divide\" our diffrents inputs\nex: groupsizes = [1,2,3] says we divide our output like this :\n[x1],[x2,x3],[x4,x5,x6] and apply a maximum algorithm on each group separately");

    declareOption(ol, "groupsize", &MultiSampleVariable::groupsize,
                  OptionBase::buildoption,
                  "shortcut if you want all groupsizes to be equals, for example if you set the value of this option to be 3, it will make groupsizes = [3,3,...,3]");   

    declareOption(ol, "random_gen", &MultiSampleVariable::random_gen,
                  OptionBase::buildoption,
                  "random generator");
            
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void MultiSampleVariable::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
    
    if (groupsize <= 0)
        PLERROR("Groupsize(s) not specified or invalid in MultiSampleVariable");    
    if (input->width() % groupsize != 0)
        PLERROR("Invalid groupsize in MultiSampleVariable (%i does not divide %i)", groupsize, input->width());

    
    if(random_gen == NULL)
        random_gen = new PRandom();
    
}


////////////////
// some utils //
////////////////

void MultiSampleVariable::sample_range(Vec &x, Vec &y, int start, int length)
{
    if(length != 1)
    {
        y[start+random_gen->multinomial_sample(x.subVec(start,length))] = 1;
    }
    else // if groupsize == 1
    {
        Vec temp(2);
        temp[0] = 1.-x[start];
        temp[1] = temp[0];
        y[start] = random_gen->multinomial_sample(temp);
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
