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
   * $Id: SampleVariable.cc,v 1.4 2004/07/21 16:30:54 chrish42 Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


#include "SampleVariable.h"
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

/*** SourceSampleVariable ***/

  string SourceSampleVariable::classname() const
  { return "SourceSampleVariable"; }

VarArray SourceSampleVariable::random_sources() 
{ 
  if (marked)
    return VarArray(0,0);
  marked = true;
  return Var(this); 
}

/*** UnarySampleVariable ***/

  string UnarySampleVariable::classname() const
  { return "UnarySampleVariable"; }

VarArray UnarySampleVariable::random_sources() 
{ 
  if (marked)
    return VarArray(0,0);
  marked = true;
  return input->random_sources() & Var(this); 
}

/*** BinarySampleVariable ***/

string BinarySampleVariable::classname() const
{ return "BinarySampleVariable"; }

VarArray BinarySampleVariable::random_sources() 
{ 
  if (marked)
    return VarArray(0,0);
  marked = true;
  return input1->random_sources() & input2->random_sources() & Var(this); 
}

/*** UniformSampleVariable ***/

string UniformSampleVariable::classname() const
{ return "UniformSampleVariable"; }

UniformSampleVariable::UniformSampleVariable( int length, int width,
                                             real minvalue, 
                                             real maxvalue)
    :SourceSampleVariable(length,width),
     min_value(minvalue),max_value(maxvalue)
{
  sprintf(name,"U[%f,%f]",min_value,max_value);
}

UniformSampleVariable* UniformSampleVariable::deepCopy(map<const void*, void*>& copies) const
{
  map<const void*, void*>::iterator it = copies.find(this);
  if (it!=copies.end()) // a copy already exists, so return it
    return (UniformSampleVariable*)it->second;
  
  // Otherwise call the copy constructor to obtain a SHALLOW copy
  UniformSampleVariable* deep_copy = new UniformSampleVariable(*this); 
  // Put the copy in the map
  copies[this] = deep_copy;
  // Transform the shallow copy into a deep copy
  deep_copy->makeDeepCopyFromShallowCopy(copies);
  // return the completed deep_copy
  return deep_copy;
}

void UniformSampleVariable::fprop()
{
  for (int k=0;k<nelems();k++)
    valuedata[k] = bounded_uniform(min_value,max_value);

}

/*** MultinomialSampleVariable ***/

string MultinomialSampleVariable::classname() const
{ return "MultinomialSampleVariable"; }

MultinomialSampleVariable::MultinomialSampleVariable(Variable* probabilities, 
                                                     int length, int width)
    :UnarySampleVariable(probabilities, length, width)
{
  sprintf(name,"Multinomial[%dx%d]",length,width);
}

MultinomialSampleVariable* MultinomialSampleVariable::deepCopy(map<const void*, void*>& copies) const
{
  map<const void*, void*>::iterator it = copies.find(this);
  if (it!=copies.end()) // a copy already exists, so return it
    return (MultinomialSampleVariable*)it->second;
  
  // Otherwise call the copy constructor to obtain a SHALLOW copy
  MultinomialSampleVariable* deep_copy = new MultinomialSampleVariable(*this); 
  // Put the copy in the map
  copies[this] = deep_copy;
  // Transform the shallow copy into a deep copy
  deep_copy->makeDeepCopyFromShallowCopy(copies);
  // return the completed deep_copy
  return deep_copy;
}

void MultinomialSampleVariable::fprop()
{
  for (int k=0;k<nelems();k++)
    valuedata[k] = multinomial_sample(input->value);

}

/*** DiagonalNormalSampleVariable ***/

string DiagonalNormalSampleVariable::classname() const
{ return "DiagonalNormalSampleVariable"; }

DiagonalNormalSampleVariable::DiagonalNormalSampleVariable
(Variable* mu, Variable* sigma)
  :BinarySampleVariable(mu, sigma, mu->length(), mu->width()) 
{
  if (!sigma->isScalar() && (mu->length()!=sigma->length() || mu->width()!=sigma->width()) )
    PLERROR("DiagonalNormalSampleVariable: mu(%d,%d) incompatible with sigma(%d,%d)",
          mu->length(),mu->width(),sigma->length(),sigma->width());
}

DiagonalNormalSampleVariable* DiagonalNormalSampleVariable::deepCopy(map<const void*, void*>& copies) const
{
  map<const void*, void*>::iterator it = copies.find(this);
  if (it!=copies.end()) // a copy already exists, so return it
    return (DiagonalNormalSampleVariable*)it->second;
  
  // Otherwise call the copy constructor to obtain a SHALLOW copy
  DiagonalNormalSampleVariable* deep_copy = new DiagonalNormalSampleVariable(*this); 
  // Put the copy in the map
  copies[this] = deep_copy;
  // Transform the shallow copy into a deep copy
  deep_copy->makeDeepCopyFromShallowCopy(copies);
  // return the completed deep_copy
  return deep_copy;
}

void DiagonalNormalSampleVariable::fprop()
{
  if (input2->isScalar())
    {
      real sigma = input2->valuedata[0];
      for (int k=0;k<length();k++)
        valuedata[k] = gaussian_mu_sigma(input1->valuedata[k],
                                         sigma);
    }
  else
    for (int k=0;k<length();k++)
      valuedata[k] = gaussian_mu_sigma(input1->valuedata[k],
                                       input2->valuedata[k]);
}


} // end of namespace PLearn

