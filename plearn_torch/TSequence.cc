// -*- C++ -*-

// TSequence.cc
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

/*! \file TSequence.cc */


#include "TSequence.h"

namespace PLearn {
using namespace std;

///////////////
// TSequence //
///////////////
TSequence::TSequence() 
: sequence(0),
  n_frames(0)
{}

TSequence::TSequence(Torch::Sequence* seq)
: sequence(seq),
  n_frames(0)
{
  updateFromTorch();
//  build(); // TODO Will most likely cause troubles...
}

PLEARN_IMPLEMENT_OBJECT(TSequence,
    "Interface between PLearn and a Torch Sequence object",
    ""
);

void TSequence::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "frames", &TSequence::frames, OptionBase::buildoption,
      "The data itself.");

  declareOption(ol, "n_frames", &TSequence::n_frames, OptionBase::buildoption,
      "Number of visible frames.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void TSequence::build_()
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
void TSequence::build()
{
  inherited::build();
  build_();
}

void TSequence::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TSequence::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//////////////////////
// updateFromPLearn //
//////////////////////
void TSequence::updateFromPLearn(Torch::Object* ptr) {
  int n_real_frames = frames.length();  // Number of frames.
  int frame_size = frames.width();      // Size of each frame.
  if (frame_size == 0)
    return;
  if (ptr)
    sequence = (Torch::Sequence*) ptr;
  else {
    if (sequence)
      allocator->free(sequence);
    sequence = new(allocator) Torch::Sequence(n_real_frames, frame_size);
  }

  FROM_P_BASIC(n_frames, n_frames, sequence, n_frames);
  
  // TODO Find a cleaner way to do this.
  if (options["frames"]) {
    if (sequence->frame_size != frame_size)
      PLERROR("In TSequence::updateFromPLearn - Cannot modify the frame size");
    sequence->resize(n_real_frames);
    for (int i = 0; i < n_real_frames; i++) {
      real* dest   = sequence->frames[i];
      real* source = frames[i];
      for (int j = 0; j < frame_size; j++)
        *(dest++) = *(source++);
    }
  }

  inherited::updateFromPLearn(sequence);
}

/////////////////////
// updateFromTorch //
/////////////////////
void TSequence::updateFromTorch() {
  FROM_T_BASIC(n_frames, n_frames, sequence, n_frames);

  frames.resize(sequence->n_real_frames, sequence->frame_size);
  for (int i = 0; i < frames.length(); i++) {
    real* source = sequence->frames[i];
    real* dest   = frames[i];
    for (int j = 0; j < frames.width(); j++)
      *(dest++) = *(source++);
  }
  inherited::updateFromTorch();
}

} // end of namespace PLearn
