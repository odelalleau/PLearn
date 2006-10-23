// -*- C++ -*-

// FileDictionary.cc
//
// Copyright (C) 2004 Hugo Larochelle, Christopher Kermorvant
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

// Authors: Hugo Larochelle, Christopher Kermorvant

/*! \file FileDictionary.cc */


#include "FileDictionary.h"

namespace PLearn {
using namespace std;
  
FileDictionary::FileDictionary():inherited(){}

FileDictionary::FileDictionary(string file_name, bool up_mode)
{
    setUpdateMode(up_mode);
    file_name_dict=file_name;
    build_();
}

PLEARN_IMPLEMENT_OBJECT(FileDictionary,
                        "Dictionary instantiated from a file",
                        "This class simply permits the instantiation of a Dictionary from a file that contains a list of symbols.\n"
                        "Each line of the file should be a symbol to be inserted in the dictionary.\n"
                        "Blanks are removed at the beginning and end of every line.\n");

void FileDictionary::declareOptions(OptionList& ol)
{
    declareOption(ol, "file_name_dict", 
                  &FileDictionary::file_name_dict, 
                  OptionBase::buildoption, 
                  "File name for the dictionary");
    inherited::declareOptions(ol);
}

void FileDictionary::build_()
{
    //initial building
    if(size() == 0) // Verifying if dictionary is new
    {
        // save update mode for later
        int saved_up_mode=update_mode;
        // set the dictionary in update mode to insert the words
        update_mode =  UPDATE;
        string line;
        ifstream ifs(file_name_dict.c_str());
        if (!ifs) PLERROR("Cannot open file %s", file_name_dict.c_str());
        while(!ifs.eof()){
            getline(ifs, line, '\n');
            if(line == "") continue;
            getId(removeblanks(line));
        }
        ifs.close();
        // restore update mode;
        update_mode=saved_up_mode;
    }
}

// ### Nothing to add here, simply calls build_
void FileDictionary::build()
{
    inherited::build();
    build_();
}

void FileDictionary::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
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
