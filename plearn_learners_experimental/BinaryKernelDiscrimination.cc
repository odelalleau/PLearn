// -*- C++ -*-

// BinaryKernelDiscrimination.cc
//
// Copyright (C) 2006 Pierre-Jean L Heureux 
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Pierre-Jean L Heureux

/*! \file BinaryKernelDiscrimination.cc */


#include "BinaryKernelDiscrimination.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BinaryKernelDiscrimination,
    "Kernel suitable for a binary vector input",
    "Based on Harper,et al. J.Chem.Inf.Comput.Sci. 2001,41,p.1295. \nHELP"
    );

//////////////////
// BinaryKernelDiscrimination //
//////////////////
BinaryKernelDiscrimination::BinaryKernelDiscrimination() :
/* ### Initialize all fields to their default value here */
    K(1),
    ExpOutput(false)
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

////////////////////
// declareOptions //
////////////////////
void BinaryKernelDiscrimination::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    declareOption(ol, "lambda", &BinaryKernelDiscrimination::lambda, OptionBase::buildoption,
                  "The base of the kernel");
    declareOption(ol, "K", &BinaryKernelDiscrimination::K, OptionBase::buildoption,
                  "a constant");
    declareOption(ol, "ExpOutput", &BinaryKernelDiscrimination::ExpOutput, OptionBase::buildoption,
                  "Flag to 1 if you do not want the log of the output. The exponential form will be outputted instead");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void BinaryKernelDiscrimination::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void BinaryKernelDiscrimination::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

//////////////
// evaluate //
//////////////
real BinaryKernelDiscrimination::evaluate(const Vec& x1, const Vec& x2) const {
    // ### Evaluate the kernel on a pair of points.
    int i;
    if (x1.length() != x2.length()){PLERROR("Vector not of same length");}
    int N= x1.length();
    
    int dij = 0;
    for (i=0; i < x1.length();i++)
    {
        if (x1[i] != x2[i]){dij++;};
    }
    if (ExpOutput)
        return pow((pow(lambda,N-dij))*(pow(1-lambda,dij)),K/N);
    else
        return safeflog(pow((pow(lambda,N-dij))*(pow(1-lambda,dij)),K/N));
}

/* ### This method will very often be overridden.
//////////////////
// evaluate_i_j //
//////////////////
real BinaryKernelDiscrimination::evaluate_i_j(int i, int j) const {
// ### Evaluate the kernel on a pair of training points.
}
*/

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void BinaryKernelDiscrimination::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("BinaryKernelDiscrimination::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/* ### This method will be overridden if computations need to be done,
   ### or to forward the call to another object.
   ### In this case, be careful that it may be called BEFORE the build_()
   ### method has been called, if the 'specify_dataset' option is used.
////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void BinaryKernelDiscrimination::setDataForKernelMatrix(VMat the_data) {
}
*/

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
