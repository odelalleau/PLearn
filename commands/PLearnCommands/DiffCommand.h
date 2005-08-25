// -*- C++ -*-

// DiffCommand.h
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
 * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file DiffCommand.h */


#ifndef DiffCommand_INC
#define DiffCommand_INC

#include <commands/PLearnCommands/PLearnCommand.h>
#include <commands/PLearnCommands/PLearnCommandRegistry.h>
#include <plearn/io/PStream.h>
#include <plearn/base/PP.h>

namespace PLearn {

class Object;

class DiffCommand: public PLearnCommand
{

public:

    DiffCommand();                    
    virtual void run(const std::vector<std::string>& args);

protected:

    static PLearnCommandRegistry reg_;

public:

//  static int diff(PP<Object> refer, PP<Object> other, vector<string>& diffs);
    /*
      const string& refer_name,
      const string& other_name,
      PStream& out = pout, PStream& err = err);
    */

    /*
      static bool diff(PP<Object> refer, PP<Object> other, PP<OptionBase> opt,
      vector<string>& diffs);

      static bool diff(const string& refer, const string& other, const string& name,
      const string& type, TMat<string>& diffs);

      static void newDiff(TMat<string>& diffs,
      const string& name, const string& val1, const string& val2);
    */
};

  
} // end of namespace PLearn

#endif


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
