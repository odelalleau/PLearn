
// -*- C++ -*-

// AutoRunCommand.cc
//
// Copyright (C) 2003  Pascal Vincent 
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

/*! \file AutoRunCommand.cc */

#include "AutoRunCommand.h"
#include "RunCommand.h"
#include <plearn/math/TVec.h>

#include <plearn/io/fileutils.h>

// norman: added check for win32
#ifndef WIN32
#include <unistd.h>
#endif

namespace PLearn {
using namespace std;

//! This allows to register the 'AutoRunCommand' command in the command registry
PLearnCommandRegistry AutoRunCommand::reg_(new AutoRunCommand);

//! The actual implementation of the 'AutoRunCommand' command 
void AutoRunCommand::run(const vector<string>& args)
{
    string scriptname = args[0];
    int nargs = (int)args.size();

    vector<string> runargs(1);
    runargs[0] = scriptname;

    TVec<int> times(nargs);

    for(;;)
    {
        try 
        {
            RunCommand rc;
            cerr << ">>> Running script " << scriptname << endl; 
            rc.run(runargs);
        }
        catch(const PLearnError& e)
        {
            cerr << "FATAL ERROR running script " << scriptname << "\n"
                 << e.message() << endl;
        }
      
        sleep(1);

        // Store times
        for(int k=0; k<nargs; k++)
            times[k] = mtime(args[k]);

        bool up_to_date = true;
        while(up_to_date)
        {
            sleep(1);
            for(int k=0; k<nargs; k++)
                if(times[k] < mtime(args[k]))
                {
                    cerr << " File " << args[k] << " has changed." << endl;
                    up_to_date = false;
                }
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
