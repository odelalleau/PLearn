// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2001 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: Popen.cc,v 1.3 2002/09/17 01:27:34 zouave Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include <sys/types.h>
#if !defined(_MSC_VER) && !defined(_MINGW_)
#include <sys/wait.h>
#endif
#include <unistd.h>
#include "stringutils.h"
#include "Popen.h"
#include <errno.h>

namespace PLearn <%
using namespace std;

#ifdef _MINGW_
void Popen::launch(const string& command)
{ PLERROR("Popen - Not implemented for this platform"); }

void Popen::launch(const string& command, const vector<string>& commandoptions)
{ PLERROR("Popen - Not implemented for this platform"); }

int Popen::wait()
{ PLERROR("Popen - Not implemented for this platform"); return 0; }

Popen::~Popen()
{}

vector<string> execute(const string& command)
{ vector<string> results; PLERROR("Popen - Not implemented for this platform"); return results; }

#else

  void Popen::launch(const string& command)
  {
    /*vector<string> args = split(command);
    vector<string> commandoptions(args.begin()+1, args.end());
    launch(args[0], commandoptions);*/
    if (verbose)
      cout << "Popen launches:" << endl << command << endl;
    int tocommand[2];
    int fromcommand[2];

    pipe(tocommand);
    pipe(fromcommand);
    pid = fork();
    if(pid==0) // in child process 
      {
        close(tocommand[1]);
        close(fromcommand[0]);
        dup2(tocommand[0],0);
        dup2(fromcommand[1],1);
	if (verbose)
	{
	  ofstream dbg("dbg1");
	  dbg << command << endl;
	  dbg.close();
	}
        execl("/bin/sh", "sh", "-c", command.c_str(), 0);
        ofstream dbg2("dbg2");
        dbg2 << "exec failed: " << errno << " : "  << strerror(errno)
             << endl;
        dbg2.close();
      }
    else // in parent
      {
        process_alive=true;
        close(tocommand[0]);
        close(fromcommand[1]);
        fdout = tocommand[1];
        fdin = fromcommand[0];
        in.attach(fdin);
        out.attach(fdout);
      }    
  }

  void Popen::launch(const string& command, const vector<string>& commandoptions)
  {
    int tocommand[2];
    int fromcommand[2];
    pipe(tocommand);
    pipe(fromcommand);
    pid = fork();
    if(pid==0) // in child process 
      {
        close(tocommand[1]);
        close(fromcommand[0]);
        dup2(tocommand[0],0);
        dup2(fromcommand[1],1);
        char **argv = new char*[commandoptions.size()+2];
        argv[0] = (char*)command.c_str();
        //ofstream out("dbg.out"); //commented out by xsm (??? what is this line for ???)
        for(unsigned int i=0; i<commandoptions.size(); i++)
          argv[i+1] = (char*)commandoptions[i].c_str();
        argv[commandoptions.size()+1] = 0;
        execvp(command.c_str(),argv);
      }
    else // in parent
      { 
        process_alive=true;
        close(tocommand[0]);
        close(fromcommand[1]);
        fdout = tocommand[1];
        fdin = fromcommand[0];
        in.attach(fdin);
        out.attach(fdout);
      }    
  }

  int Popen::wait()
  {
    int status;
    if(process_alive)
        waitpid(pid, &status, 0);
    process_alive=false;
    // the following will return 1 IF THE PROGRAM RETURNED NORMALLY
    return WIFEXITED(status);
  }
    
  Popen::~Popen()
  {
    // cerr << "Entering Popen destructor" << endl;
    close(fdout);
    close(fdin);
    // cerr << "Popen in waitpid()...." << endl;
    if(process_alive)
        waitpid(pid, 0, 0);
    // cerr << "Popen destructor completed." << endl;
  }

  vector<string> execute(const string& command)
  {
    Popen p(command);
    vector<string> result;
    while(p.in)
    {
      string line = pgetline(p.in.rawin());
      //cout << line << endl;
      result.push_back(line);
    }
    return result;
  }

#endif
%> // end of namespace PLearn



