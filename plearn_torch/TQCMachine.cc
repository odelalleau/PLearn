// -*- C++ -*-

// TQCMachine.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file TQCMachine.cc */


#include "TQCMachine.h"

namespace PLearn {
using namespace std;

TQCMachine::TQCMachine() 
: qc_machine(0),
#ifdef USEDOUBLE
  epsilon_bound(1e-12),
#else
  epsilon_bound(1e-4),
#endif
  n_alpha(0)
{
}

PLEARN_IMPLEMENT_OBJECT(TQCMachine,
    "Interface between PLearn and a Torch QCMachine object",
    ""
);

void TQCMachine::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "epsilon_bound", &TQCMachine::epsilon_bound, OptionBase::buildoption,
      "Bound accuracy.");

  declareOption(ol, "n_alpha", &TQCMachine::n_alpha, OptionBase::learntoption,
      "Number of parameters.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void TQCMachine::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

// ### Nothing to add here, simply calls build_
void TQCMachine::build()
{
  inherited::build();
  build_();
}

void TQCMachine::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TQCMachine::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//////////////////////
// updateFromPLearn //
//////////////////////
void TQCMachine::updateFromPLearn(Torch::Object* ptr) {
  if (ptr)
    qc_machine = (Torch::QCMachine*) ptr;
  else
    PLERROR("In TQCMachine::updateFromPLearn - Torch::QCMachine is an abstract class "
            "and cannot be instantiated");
  FROM_P_BASIC(epsilon_bound, epsilon_bound, qc_machine, bound_eps);
  FROM_P_BASIC(n_alpha,       n_alpha,       qc_machine, n_alpha  );

  inherited::updateFromPLearn(qc_machine);
  // NB: not updating Cup, Cdown, alpha, grad, y, cache.
}


/////////////////////
// updateFromTorch //
/////////////////////
void TQCMachine::updateFromTorch() {
  FROM_T_BASIC(epsilon_bound, epsilon_bound, qc_machine, bound_eps);
  FROM_T_BASIC(n_alpha,       n_alpha,       qc_machine, n_alpha  );
  inherited::updateFromTorch();
}

} // end of namespace PLearn
