// -*- C++ -*-

// PLearnServer.cc
//
// Copyright (C) 2005 Pascal Vincent 
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
   * $Id: PLearnServer.cc,v 1.4 2005/01/14 19:40:50 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file PLearnServer.cc */


#include "PLearnServer.h"
#include <plearn/base/plerror.h>

namespace PLearn {
using namespace std;

// Put function implementations here.

  PLearnServer::PLearnServer(const PStream& input_output)
    :io(input_output)
  {
    
  }

  void PLearnServer::callFunction(const string& name, int nargs)
  {
    if(name=="cd")
      {
        string path;
        io >> path;
        chdir(path);
        prepareToSendResults(io,0);
      }
    else
      PLERROR("In PLearnServer::callFunction Invalid function name %s",name.c_str());

    io << endl;
  }


  void PLearnServer::printHelp()
  {
    io.write(
             "Summary of commands:\n"
             "  ?                                 # print this help message \n"
             "  F functionname nargs arg1 ...     # calls a supported function.\n"
             "  N objid object_specification      # creates new object.\n"
             "  M objid methodname nargs arg1 ... # calls method on object objid. Returns: R <nreturn> ret1 ... \n"
             "  D objid                   # deletes object objid. Returns: R 0 \n"
             "  Z                         # delete all objects. Returns: R 0 \n"
             "  Q                         # Quit. Returns nothing. \n"
             "\n"
             "Except for ? and Q, all commands upon success return: \n"
             "  R n_retutn_args arg1 ... \n"
             "If an error or exception occurs, the following is returned: \n"
             "  E \"errormsg\" \n"
             "\n"
             "Summary of currently supported functions:\n"
             "  F cd 1 \"path\" \n"
             "\n"
             );
    io << endl;
  }

  void PLearnServer::run()
  {
    int obj_id;
    Object* obj;
    ObjMap::iterator found;
    string method_name;
    int n_args; // number of input arguments to the method call
    string dirpath;

    for(;;)
      {
        io.skipBlanksAndComments();
        int command = io.get();

        try 
          {
            switch(command)
              {
              case '?':
                printHelp();
                break;

              case 'F': // call function 
                io >> method_name >> n_args;
                callFunction(method_name, n_args);
                io << endl;
                break;

              case 'N': // new
                obj = 0;
                io >> obj_id >> obj;           // Read new object
                objmap[obj_id] = obj;
                io.write("R 0");
                io << endl;  
                break;
            
              case 'D': // delete
                io >> obj_id;
                if(objmap.erase(obj_id)==0)
                  PLERROR("Calling delete of a non-existing object");
                io.write("R 0");
                io << endl;
                break;

              case 'M': // method call
                io >> obj_id;
                found = objmap.find(obj_id);
                if(found == objmap.end()) // unexistant obj_id
                  PLERROR("Calling a method on a non-existing object");
                else 
                  {
                    io >> method_name >> n_args;
                    found->second->call(method_name, n_args, io);
                    io << endl;
                  }
                break;

              case 'Z': // delete all objects
                objmap.clear();
                io.write("R 0");
                io << endl;
                break;

              case 'Q': // quit
              case EOF:
                cerr << "Quitting" << endl;
                return;

              default:
                PLERROR("Invalid PLearnServer command char: %c",command);
              }
          }
        catch(const PLearnError& e)
          {
            io.write("E ");
            io << e.message() << endl;
          }
        catch (...) 
          {
            io.write("E ");
            io << "Unknown exception" << endl;
          }
      }
  }

} // end of namespace PLearn

