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
 * $Id$
 * AUTHORS: Pascal Vincent & Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */

#include "OptionBase.h"
#include "Object.h"

namespace PLearn {
using namespace std;

const OptionBase::flag_t OptionBase::buildoption      = 1;       
const OptionBase::flag_t OptionBase::learntoption     = 1 << 1;
const OptionBase::flag_t OptionBase::tuningoption     = 1 << 2;
const OptionBase::flag_t OptionBase::nosave           = 1 << 3; 
const OptionBase::flag_t OptionBase::nonparentable    = 1 << 4;
const OptionBase::flag_t OptionBase::nontraversable   = 1 << 5;


OptionBase::OptionBase(const string& optionname, flag_t flags,
                       const string& optiontype, const string& defaultval, 
                       const string& description)
    : optionname_(optionname), flags_(flags), 
      optiontype_(optiontype), defaultval_(defaultval),
      description_(description)
{
    if (optionname.size() > 0 && optionname[0] == '_' )
        PLERROR("OptionBase::OptionBase: options should not start with an underscore: '%s'",
                optionname.c_str());
}


bool OptionBase::shouldBeSkipped() const
{
    return (flags() & (buildoption | learntoption | tuningoption)) == 0;
}


string OptionBase::writeIntoString(const Object* o) const
{
    string s;
    PStream out = openString(s, PStream::plearn_ascii, "w");
    write(o, out);
    out.flush(); // May not be necessary ?
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


vector<string> OptionBase::flagStrings() const
{
    flag_t curflags = flags();
    vector<string> fs;

    static bool initialized = false;
    static map<flag_t, string> flag_map;
    if (! initialized) {
        flag_map[buildoption   ] = "buildoption";
        flag_map[learntoption  ] = "learntoption";
        flag_map[tuningoption  ] = "tuningoption";
        flag_map[nosave        ] = "nosave";
        flag_map[nonparentable ] = "nonparentable";
        flag_map[nontraversable] = "nontraversable";
        initialized = true;
    }

    for (map<flag_t, string>::const_iterator it = flag_map.begin(),
             end = flag_map.end() ; it != end ; ++it)
    {
        // As we process each option, turn it off in temporary copy of flags to
        // detect unprocessed flags
        if (curflags & it->first) {
            fs.push_back(it->second);
            curflags &= ~it->first;
        }
    }

    if (curflags)
        PLERROR("OptionBase::flagStrings: unprocessed flags in option '%s' (%s);\n"
                "cannot interpret remaining bits %d", optionname().c_str(),
                optiontype().c_str(), curflags);
    
    return fs;
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
