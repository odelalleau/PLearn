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
   * $Id: PLearnServer.cc,v 1.1 2005/01/06 02:09:38 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file PLearnServer.cc */


#include "PLearnServer.h"
#include <plearn/base/plerror.h>

namespace PLearn {
using namespace std;

// Put function implementations here.

  PLearnServer::PLearnServer(const PStream& input_output)
    :io(input_output), last_obj_id(0)
  {
    
  }

  void PLearnServer::run()
  {
    unsigned int obj_id;
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
              case '>': // cd "dirpath"
                io >> dirpath;
                chdir(dirpath);
                io.write("R 0");
                io << endl;
                break;

              case 'N': // new
                obj = 0;
                cerr << "before reading" << endl;
                io >> obj;           // Read new object
                cerr << "after reading" << endl;
                objmap[++last_obj_id] = obj;
                io.write("R 1 ");
                io << last_obj_id << endl;   // Send back the object id
                break;
            
              case 'D': // delete
                io >> obj_id;
                if(objmap.erase(obj_id)==0)
                  PLERROR("Calling delete of an inexistant object");
                io.write("R 0");
                io << endl;
                break;

              case 'M': // method call
                io >> obj_id;
                found = objmap.find(obj_id);
                if(found == objmap.end()) // unexistant obj_id
                  PLERROR("Calling a method on an inexistant object");
                else 
                  {
                    io >> method_name >> n_args;
                    found->second->call(method_name, n_args, io);
                    io << endl;
                  }
                break;

              case 'Z': // delete all objects
                objmap.clear();
                last_obj_id = 0;
                io.write("R 0");
                io << endl;
                break;

              case 'Q': // quit
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

