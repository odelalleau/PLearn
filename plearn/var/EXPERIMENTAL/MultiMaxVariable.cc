// -*- C++ -*-

// MultiMaxVariable.cc
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

/*! \file MultiMaxVariable.cc */


#include "MultiMaxVariable.h"

namespace PLearn {
using namespace std;

/** MultiMaxVariable **/

PLEARN_IMPLEMENT_OBJECT(
    MultiMaxVariable,
    "Different max variables done on separate groups of the input",
"This variables computes a max functions (softmax, log-softmax, hardmax, etc., determined by the field computation_type)" 
"\non subvectors of the input, which lengths are defined by the field groupsizes (or groupsize if all the groups will have the same size)." 
"\n"
"\nex :"
"\nif groupsizes = [1,2,3], and computation_type = 'S' (for softmax), and the input vector [1,2,3,4,5,6],"
"\nthe result will be [softmax([1]), softmax([2,3]), softmax([4,5,6])]"
"\n"
"\nnote : in that example matValue.width() of the variable must be 1+2+3=6" );


//! Constructor
MultiMaxVariable::MultiMaxVariable(Variable* input, TVec<int> groupsizes, char computation_type)
    : inherited(input, input->length(), input->width()),
      groupsizes(groupsizes),
      computation_type(computation_type)
{
    build_();
}


MultiMaxVariable::MultiMaxVariable(Variable* input, int groupsize, char computation_type)
    : inherited(input, input->length(), input->width()),
      computation_type(computation_type),
      groupsize(groupsize)
{
    build_();
}

void MultiMaxVariable::recomputeSize(int& l, int& w) const
{
        if (input) {
            l = input->length();
            w = input->width() ;
        } else
            l = w = 0;
}

// ### computes value from input's value
void MultiMaxVariable::fprop()
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
        
        for(int i=0; i<groupsizes.length(); i++)
        {
            switch(computation_type)
            {
//softmax
            case 'S':
//          softmax(v.subVec(k,k+groupsizes[i]), value.subVec(k,k+groupsizes[i]));
                softmax_range(inputValue_n, value_n, k, groupsizes[i]);                   
                break;
//log_softmax
            case 'L':
//          log_softmax(v.subVec(k,k+groupsizes[i]), value.subVec(k,k+groupsizes[i]));
                logSoftmax_range(inputValue_n, value_n, k, groupsizes[i]);
                break;
//hardmax_value
            case 'H':
                hardMax_range(inputValue_n, value_n, k, groupsizes[i], true);
                break;
//hardmax
            case 'h':
                hardMax_range(inputValue_n, value_n, k, groupsizes[i], false);
                break;
//random_softmax_value
            case 'R':
                softmax_range(inputValue_n, value_n, k, groupsizes[i]);
                //TODO : RANDOM
                PLERROR("computation_type 'R' not fully implemented yet");
                break;
//random_softmax
            case 'r':
                softmax_range(inputValue_n, value_n, k, groupsizes[i]);
                //TODO : RANDOM
                PLERROR("computation_type 'r' not fully implemented yet");
                break;
            default :
                PLERROR("invalid computation_type in MultiMaxVariable");
            }
            k+=groupsizes[i];
        }
    }
}

// ### computes input's gradient from gradient
void MultiMaxVariable::bprop()
{
    int k;
    Mat inputGradient = input->matGradient;
    int l = inputGradient.length();
    Vec inputGradient_n;
    Vec value_n;
    Vec gradient_n;
    for(int n=0; n<l; n++)
    {
        inputGradient_n = inputGradient(n);
        value_n = matValue(n);
        gradient_n = matGradient(n);
        k=0;
        for(int i=0; i<groupsizes.length(); i++)
        {
            switch(computation_type)
            {
//softmax
            case 'S':
                bpropSoftMax(inputGradient_n, gradient_n, value_n, k, groupsizes[i]);
                break;
//log_softmax
            case 'L':
                //ici aussi j'ai tout copié, en changeant seulement les "bornes" de la sommation
                bpropLogSoftMax(inputGradient_n, gradient_n, value_n, k, groupsizes[i]); 
                break;
//hardmax_value
            case 'H':
                bpropHardMaxValue(inputGradient_n, gradient_n, value_n, k, groupsizes[i]);
                break;
//hardmax
            case 'h':
                PLERROR("computation_type 'h' not implemented yet");
                break;
//random_softmax_value
            case 'R':
                PLERROR("computation_type 'R' not implemented yet");
                break;
//random_softmax
            case 'r':
                PLERROR("computation_type 'r' not implemented yet");
                break;
            default :
                PLERROR("unable to bprop because of invalid computation_type");
            }
            k+=groupsizes[i];
        }
    }
}    
// ### You can implement these methods:
// void MultiMaxVariable::bbprop() {}
// void MultiMaxVariable::symbolicBprop() {}
// void MultiMaxVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void MultiMaxVariable::build()
{
    inherited::build();
    build_();
}

void MultiMaxVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    deepCopyField(groupsizes, copies);
    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);   
}

void MultiMaxVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    declareOption(ol, "groupsizes", &MultiMaxVariable::groupsizes,
                  OptionBase::buildoption,
                  "this tells how to \"divide\" our diffrents inputs\nex: groupsizes = [1,2,3] says we divide our output like this :\n[x1],[x2,x3],[x4,x5,x6] and apply a maximum algorithm on each group separately");

    declareOption(ol, "groupsize", &MultiMaxVariable::groupsize,
                  OptionBase::buildoption,
                  "shortcut if you want all groupsizes to be equals, for example if you set the value of this option to be 3, it will make groupsizes = [3,3,...,3]");

    declareOption(ol, "computation_type", &MultiMaxVariable::computation_type,
                  OptionBase::buildoption,
                  "specifies what maximum algorithm should be used on our groups\n\'S\' = Softmax\n\'L\' = Log(Softmax)\n\'H\' = Hardmax*value\n\'h\' = hardmax\n\'R\' = random_Softmax*value\n\'r\' = random_Softmax");
            
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void MultiMaxVariable::build_()
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
    
    if (input.isNotNull() ) // otherwise postpone building until we have an input!
    {
        if (groupsizes.length() <= 0)
        {
            if (groupsize <= 0)
                PLERROR("Groupsize(s) not specified or invalid in MultiMaxVariable");    
            if (input->width() % groupsize != 0)
                PLERROR("Invalid groupsize in MultiMaxVariable");

            TVec<int> vec(input->width()/groupsize, groupsize);
            groupsizes = vec;
        }
        else
        {
            int sum = 0;
            for(int i=0; i<groupsizes.length(); i++)
                sum += groupsizes[i];       
            if(sum != input->width())
                PLERROR("Invalid groupsizes in MultiMaxVariable");    
        }
    }
}


////////////////
// some utils //
////////////////

void MultiMaxVariable::softmax_range(Vec &x, Vec &y, int start, int length)
{
    real somme=0;
    for(int i=start; i<start+length; i++)    
        somme +=safeexp(x[i]);
    if (somme == 0) PLERROR("trying to divide by 0 in softmax");

    for(int i=start; i<start+length; i++)
        y[i] = safeexp(x[i])/somme;
}

void MultiMaxVariable::logSoftmax_range(Vec &x, Vec &y, int start, int length)
{
    real somme=0;
    for(int i=start; i<start+length; i++)    
        somme += safeexp(x[i]);
    
    for(int i=start; i<start+length; i++)
        y[i] = x[i] - safelog(somme);
}

void MultiMaxVariable::hardMax_range(Vec &x, Vec &y, int start, int length, bool value)
{
    int indMax=start;
    for(int i=start+1; i<start+length; i++)
        if(x[i] > x[indMax])
            indMax = i;

    for(int i=start; i<start+length; i++)
        y[i] = 0;

    if(value)
        y[indMax] = x[indMax];
    else
        y[indMax]=1;
}


void MultiMaxVariable::bpropSoftMax(Vec &gradientInput, Vec &gradient, Vec &variableValue, int start, int length)
{
    //on parcout le gradient de notre vecteur
    for(int i=start; i<start+length; i++)
    {
        //et on rajoute un petit qqch pour chacun du gradient de l'input
        for(int j=start; j<start+length; j++)
        {
            //note ici jai juste copié ce quil y avait avant dans le bprob de softmaxVariable...
            if(i==j)
                gradientInput[i] += gradient[j]*variableValue[i]*(1.-variableValue[i]);
            else
                gradientInput[i] -= gradient[j]*variableValue[i]*variableValue[j];
        }
    }           
}


//ici j'ai juste adapté le LogSoftMax qui existait déjà
void MultiMaxVariable::bpropLogSoftMax(Vec &gradientInput, Vec &gradient, Vec &variableValue, int start, int length)
{
    real sum=0.;
    for (int i = start; i < start+length; i++)
        sum += gradient[i];

    for (int i = start; i < start+length; ++i)
        gradientInput[i] += gradient[i] - sum * safeexp(variableValue[i]);    
}

void MultiMaxVariable::bpropHardMaxValue(Vec& gradientInput, Vec& gradient, Vec& variableValue, int start, int length)
{
    real sum=0.;
    for(int i=start; i<start+length; i++)
        sum += gradient[i];

    for(int i=start; i<start+length; i++)
        if(variableValue[i] != 0)
            gradientInput[i] += sum;
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
