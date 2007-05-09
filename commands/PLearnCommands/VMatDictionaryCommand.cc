// -*- C++ -*-

// VMatDictionaryCommand.cc
//
// Copyright (C) 2006 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file VMatDictionaryCommand.cc */


#include "VMatDictionaryCommand.h"
#include <plearn/vmat/DictionaryVMatrix.h>
#include <plearn/db/getDataSet.h>
#include <plearn/io/load_and_save.h>
#include <plearn/vmat/VMat.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'VMatDictionaryCommand' command in the command registry
PLearnCommandRegistry VMatDictionaryCommand::reg_(new VMatDictionaryCommand);

VMatDictionaryCommand::VMatDictionaryCommand()
    : PLearnCommand(
        "vmat_dictionary",
        "Create Dictionary objects for each field of a vmat",
        "Will create <dataset>.field#.dict, where # is the\n"
        "field (column) number, starting at 0. Those files contain the plearn\n"
        "scripts of the Dictionary objets for each field.\n"
        )
{}

//! The actual implementation of the 'VMatDictionaryCommand' command
void VMatDictionaryCommand::run(const vector<string>& args)
{
    PPath indexf= "";
    string vmat_file = args[0];
    string ext = extract_extension(vmat_file);
    VMat vmat;
    if(ext == ".txt")
    {
        DictionaryVMatrix* dvmat = new DictionaryVMatrix();
        TVec<PPath> file_names(1);
        file_names[0] = vmat_file;
        dvmat->file_names = file_names;
        dvmat->delimiters = "";
        dvmat->build();            
        vmat = dvmat;
    }
    else
        vmat = getDataSet(vmat_file);

    for(int i=0; i<vmat->width(); i++)
    {
        if(vmat->getDictionary(i))
        {
            string dico_name = vmat_file + ".col" + tostring(i) + ".dict";
            save(dico_name,*(vmat->getDictionary(i)));
        }
    }
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
