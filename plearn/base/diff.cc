// -*- C++ -*-

// diff.cc
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

/*! \file diff.cc */


#include "diff.h"
#include <plearn/base/Object.h>
#include <plearn/base/PLearnDiff.h>

namespace PLearn {
using namespace std;

//////////
// diff //
//////////
/*
  int diff(const string& refer, const string& other, const OptionBase* opt, PLearnDiff* diffs)
  {
  pout << "Calling basic diff with Option< ObjectType, " << opt->optiontype() << " >" << endl;
  PLASSERT( diffs );
  return diffs->diff(refer, other, opt->optionname());
  }
*/

//////////
// diff //
//////////
int diff(PP<Object> refer, PP<Object> other, PLearnDiff* diffs)
{
    bool delete_diffs = false;
    if (!diffs) {
        diffs = new PLearnDiff();
        delete_diffs = true;
    }
    PLASSERT(diffs);
    // Check objects are of the same class.
    string refer_class = refer ? refer->classname() : "null";
    string other_class = other ? other->classname() : "null";
    int n_diffs = diffs->diff(refer_class, other_class, "classname");
    if (n_diffs > 0)
        return n_diffs; // We cannot compare two objects from different classes.
    else if (!other && !refer)
        return 0; // Both objects are null pointers.
    PLASSERT( other && refer );
    OptionList& options = refer->getOptionList();
    for (OptionList::const_iterator it = options.begin(); it != options.end(); it++) {
        // pout << "Comparing " << (*it)->optionname() << endl;
        string option = (*it)->optionname();
        string refer_opt = refer->getOption(option);
        string other_opt = other->getOption(option);
        n_diffs += (*it)->diff(refer_opt, other_opt, diffs);
    }
    if (delete_diffs)
        delete diffs;
    return n_diffs;
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
