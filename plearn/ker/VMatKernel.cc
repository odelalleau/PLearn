// -*- C++ -*-

// VMatKernel.cc
//
// Copyright (C) 2005 Benoit Cromp 
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
 * $Id$ 
 ******************************************************* */

// Authors: Benoit Cromp

/*! \file VMatKernel.cc */


#include "VMatKernel.h"

namespace PLearn {
using namespace std;

//////////////////
// VMatKernel //
//////////////////
VMatKernel::VMatKernel() 
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(VMatKernel,
                        "Kernel that is given its Gram matrix.",
                        "This kernel can only be applied on examples that are integers, and that\n"
                        "correspond to indices in the matrix.\n"
    );

////////////////////
// declareOptions //
////////////////////
void VMatKernel::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &VMatKernel::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(ol,"source",&VMatKernel::source,OptionBase::buildoption,
                  "Gram matrix");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void VMatKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void VMatKernel::build_()
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
real VMatKernel::evaluate(const Vec& x1, const Vec& x2) const {
    PLASSERT( source );
    PLASSERT( x1.size()==1 && x2.size()==1 );
    return source->get(int(x1[0]),int(x2[0]));
}

void VMatKernel::computeGramMatrix(Mat K) const
{
    PLASSERT( source );
    K << source->toMat();
}


//////////////////
// evaluate_i_j //
//////////////////
real VMatKernel::evaluate_i_j(int i, int j) const {
    PLASSERT( source );
    return source->get(i,j);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VMatKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("VMatKernel::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/* ### This method will be overridden if computations need to be done,
   ### or to forward the call to another object.
   ### In this case, be careful that it may be called BEFORE the build_()
   ### method has been called, if the 'specify_dataset' option is used.
////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void VMatKernel::setDataForKernelMatrix(VMat the_data) {
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
