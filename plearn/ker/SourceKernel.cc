// -*- C++ -*-

// SourceKernel.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: SourceKernel.cc,v 1.2 2004/05/07 19:07:32 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file SourceKernel.cc */

#include "SourceKernel.h"

namespace PLearn {
using namespace std;

//////////////////
// SourceKernel //
//////////////////
SourceKernel::SourceKernel() 
/* ### Initialize all fields to their default value here */
{}

PLEARN_IMPLEMENT_OBJECT(SourceKernel,
    "A kernel built upon an underlying source kernel",
    "The default behavior of a SourceKernel is to forward all calls to the underlying\n"
    "kernel. However, subclasses will probably want to override the methods to perform\n"
    "more complex operations."
);

////////////////////
// declareOptions //
////////////////////
void SourceKernel::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "source_kernel", &SourceKernel::source_kernel, OptionBase::buildoption,
      "The underlying kernel.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void SourceKernel::build()
{
  // ### Nothing to add here, simply calls build_
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void SourceKernel::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
  this->is_symmetric = source_kernel->is_symmetric;
  this->data_inputsize = source_kernel->dataInputsize();
  this->n_examples = source_kernel->nExamples();
  if (specify_dataset) {
    // Forward the specified dataset to the underlying kernel, if it is not done already.
    if (specify_dataset != source_kernel->specify_dataset) {
      source_kernel->specify_dataset = specify_dataset;
      source_kernel->build();
    }
  }
}

////////////////////////////
// addDataForKernelMatrix //
////////////////////////////
void SourceKernel::addDataForKernelMatrix(const Vec& newRow) {
  // By default, this kernel and its source_kernel share the same data.
  // Therefore, we must be careful not to append 'newRow' twice. This is
  // why we do not call inherited::addDataForKernelMatrix().
  source_kernel->addDataForKernelMatrix(newRow);
}

///////////////////////
// computeGramMatrix //
///////////////////////
void SourceKernel::computeGramMatrix(Mat K) const {
  source_kernel->computeGramMatrix(K);
}

//////////////
// evaluate //
//////////////
real SourceKernel::evaluate(const Vec& x1, const Vec& x2) const {
  // ### Evaluate the kernel on a pair of points.
  return source_kernel->evaluate(x1, x2);
}

//////////////////
// evaluate_i_j //
//////////////////
real SourceKernel::evaluate_i_j(int i, int j) const {
  // ### Evaluate the kernel on a pair of training points.
  return source_kernel->evaluate_i_j(i,j);
}

//////////////////
// evaluate_i_x //
//////////////////
real SourceKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const {
  return source_kernel->evaluate_i_x(i, x, squared_norm_of_x);
}
 
//////////////////
// evaluate_x_i //
//////////////////
real SourceKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const {
  return source_kernel->evaluate_x_i(x, i, squared_norm_of_x);
}
 
///////////////////
// getParameters //
///////////////////
Vec SourceKernel::getParameters() const {
  return source_kernel->getParameters();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SourceKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  deepCopyField(source_kernel, copies);
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void SourceKernel::setDataForKernelMatrix(VMat the_data) {
  inherited::setDataForKernelMatrix(the_data);
  source_kernel->setDataForKernelMatrix(the_data);
}

///////////////////
// setParameters //
///////////////////
void SourceKernel::setParameters(Vec paramvec) {
  source_kernel->setParameters(paramvec);
}

} // end of namespace PLearn

