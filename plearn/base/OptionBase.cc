// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin

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
   * $Id: OptionBase.cc,v 1.4 2005/02/23 21:07:44 chapados Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

#include "OptionBase.h"
#include "Object.h"

namespace PLearn {
using namespace std;

const OptionBase::flag_t OptionBase::buildoption = 1;       
const OptionBase::flag_t OptionBase::learntoption = 1<<1;
const OptionBase::flag_t OptionBase::tuningoption = 1<<2;
const OptionBase::flag_t OptionBase::nosave = 1<<4; 


bool OptionBase::shouldBeSkipped() const
{
  return (flags() & (buildoption | learntoption | tuningoption)) == 0;
}

string OptionBase::writeIntoString(const Object* o) const
{
  ostrstream out_;
  PStream out(&out_);
  write(o, out);
  char* buf = out_.str();
  int n = out_.pcount();
  string s(buf,n);
  out_.freeze(false); // return ownership to the stream, so that it may free it...
  return s;
}

void OptionBase::readIntoIndex(Object*, PStream&, const string&)
{
  PLERROR("OptionBase::readIntoIndex: indexed reads are not supported for option '%s' "
          "of type '%s'", optionname().c_str(), optiontype().c_str());
}

void OptionBase::writeAtIndex(const Object*, PStream&, const string&) const
{
  PLERROR("OptionBase::writeAtIndex: indexed writes are not supported for option '%s' "
          "of type '%s'", optionname().c_str(), optiontype().c_str());
}

} // end of namespace PLearn
