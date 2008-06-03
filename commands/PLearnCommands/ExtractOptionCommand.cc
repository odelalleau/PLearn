// -*- C++ -*-

// ExtractOptionCommand.cc
//
// Copyright (C) 2008 Pascal Vincent
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

/*! \file ExtractOptionCommand.cc */


#include "ExtractOptionCommand.h"
#include <plearn/io/openFile.h>
#include <plearn/io/openString.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'ExtractOptionCommand' command in the command registry
PLearnCommandRegistry ExtractOptionCommand::reg_(new ExtractOptionCommand);

ExtractOptionCommand::ExtractOptionCommand()
    : PLearnCommand(
        "extract_option",
        "Extracts an option from a saved plearn object (.psave) and saves it to its own file.",
        "   extract_option <objfile.psave> <optionname> <optionfile.psave>\n"
        "OR extract_option <objfile.psave> <optionname> <optionfile.pmat> <transpose> \n"
        "The first form will output the option serialized in plearn_ascii format\n"
        "The second form is available only for Mat or Vec options, and will save it as a .pmat file.\n"
        "For Mat, if transpose is 0 then the matrix won't be transposed. If it's 1 it will be transposed.\n"
        "For Vec, if transpose is 0 then it will be saved as a row matrix. If it's 1, i will be saved as a column matrix.\n"
        )
{}

//! The actual implementation of the 'ExtractOptionCommand' command
void ExtractOptionCommand::run(const vector<string>& args)
{
    if(args.size()==3)
    {
        string objfile = args[0];
        string optionname = args[1];
        string optionfile = args[2];
        PP<Object> obj = loadObject(objfile);
        PStream out = openFile(optionfile, PStream::plearn_ascii, "w");
        obj->writeOptionVal(out, optionname);
        out = 0;
        perr << "Option " << optionname << " has been written to file " << optionfile << endl;
    }
    else if(args.size()==4)
    {
        string objfile = args[0];
        string optionname = args[1];
        string optionfile = args[2];
        int do_transpose = toint(args[3]);
        PP<Object> obj = loadObject(objfile);
        string optionval = obj->getOption(optionname);
        bool ismat = false;
        Mat m;
        try
        {
            PStream in = openString(optionval, PStream::plearn_ascii);
            in >> m;
            perr << "Extracted a " << m.length() << " x " << m.width() << " matrix" << endl;
            ismat = true;
        }
        catch(const PLearnError& e)
        { ismat = false; }

        if(!ismat)
        {
            Vec v;
            PStream in = openString(optionval, PStream::plearn_ascii);
            in >> v;
            perr << "Extracted a vector of length " << v.length() << endl;
            m = v.toMat(1,v.length());
        }

        if(do_transpose==1)
            m = transpose(m);
        FileVMatrix vmat(optionfile, m.length(), m.width());
        vmat.putMat(0, 0, m);
        vmat.flush();
        perr << "Option " << optionname << " has been written to file " << optionfile << endl;
    }
    else
        PLERROR("Wrong number of argumens, please consult help");    
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
