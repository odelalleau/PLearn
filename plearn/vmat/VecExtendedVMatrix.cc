// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
   * $Id: VecExtendedVMatrix.cc,v 1.4 2004/04/05 23:12:21 morinf Exp $
   ******************************************************* */

#include "VecExtendedVMatrix.h"
#include "TMat.h"

namespace PLearn {
using namespace std;


/** VecExtendedVMatrix **/

PLEARN_IMPLEMENT_OBJECT(VecExtendedVMatrix, "ONE LINE DESC", "NO HELP");

VecExtendedVMatrix::VecExtendedVMatrix()
{
}

VecExtendedVMatrix::VecExtendedVMatrix(VMat underlying, Vec extend_data)
  : inherited(underlying.length(), underlying.width() +
              extend_data.length()),
    underlying_(underlying), extend_data_(extend_data)
{
  build();
}

void
VecExtendedVMatrix::build()
{
    inherited::build();
    build_();
}

void
VecExtendedVMatrix::build_()
{
    if (underlying_)
        fieldinfos = underlying_->getFieldInfos();
}

void
VecExtendedVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "underlying_", &VecExtendedVMatrix::underlying_, OptionBase::buildoption, "");
    declareOption(ol, "extend_data_", &VecExtendedVMatrix::extend_data_, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void VecExtendedVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In VecExtendedVMatrix::getRow OUT OF BOUNDS");
  if(v.length() != width())
    PLERROR("In VecExtendedVMatrix::getRow v.length() must be equal to the VMat's width");
#endif

  Vec subv = v.subVec(0,underlying_->width());
  underlying_->getRow(i,subv);
  copy(extend_data_.begin(), extend_data_.end(),
       v.begin() + underlying_->width());
}

} // end of namespace PLearn
