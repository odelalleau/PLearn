// -*- C++ -*-

// ReadAndWriteCommand.cc
// 
// Copyright (C) 2002 Pascal Vincent
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

/*! \file ReadAndWriteCommand.cc */
#include "ReadAndWriteCommand.h"
#include <plearn/base/Object.h>
#include <plearn/base/stringutils.h>      //!< For extract_extension.
#include <plearn/io/fileutils.h>        //!< For readFileAndMacroProcess.
#include <plearn/io/load_and_save.h>
#include <plearn/io/openFile.h>
#include <plearn/io/openString.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'ReadAndWriteCommand' command in the command registry
PLearnCommandRegistry ReadAndWriteCommand::reg_(new ReadAndWriteCommand);

ReadAndWriteCommand::ReadAndWriteCommand():
    PLearnCommand("read_and_write",
                
                  "Used to check (debug) the serialization system",
                
                  "read_and_write <sourcefile> <destfile> [--updaet] [modification string] ...\n"
                  "Reads an Object (in PLearn serialization format) from the <sourcefile> and writes it to the <destfile>\n"
                  "If the sourcefile ends with a .psave file, then it will not be subjected to macro preprosessing \n"
                  "Otherwise (ex: .plearn .vmat) it will. \n"
                  "If their is modification string in format option=value, the modification will be made to the object before saving\n"
                  "The --update option make that we generate the file only if we can calculate the modification time of the sourcefile and it is older then the destfile."
        )
{}

//! The actual implementation of the 'ReadAndWriteCommand' command 
void ReadAndWriteCommand::run(const vector<string>& args)
{
    if(args.size()<2)
        PLERROR("read_and_write takes 2 or more arguments: <sourcefile> <destfile> [--update] [modification string] ...");
    string source = args[0];
    string dest = args[1];

    string ext = extract_extension(source);
    PP<Object> o;
    time_t date_src=0;

    //read the file
    if(ext==".psave") // may be binay. Don't macro-process
    {
        PLearn::load(source,o);
        date_src=mtime(source);
    }
    else
    {
        map<string, string> vars;
        string script = readFileAndMacroProcess(source, vars, date_src);
        PStream in = openString(script,PStream::plearn_ascii);
        o = readObject(in);
    }
    int idx_start=2;
    if(args.size()>2 && args[2]=="--update"){
        PLCHECK(date_src>0);
        idx_start++;
        time_t date_dst=mtime(dest);
        if((date_dst>date_src) && (date_src>0)){
            pout << "The file is up to date. We don't regenerate it."<<endl;
            return;
        }
    }

    //modif the object
    string left;
    string right;
    for(uint i=idx_start; i<args.size();i++){
        split_on_first(args[i], "=", left, right);
        o->setOption(left, right);
    }

    //write the file
    PStream out = openFile(dest,PStream::plearn_ascii,"w");
    if(!out)
        PLERROR("Could not open file %s for writing",dest.c_str());
    out << *o;
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
