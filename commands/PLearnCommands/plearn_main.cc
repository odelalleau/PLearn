// -*- C++ -*-

// plearn_main.cc
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
   * $Id: plearn_main.cc,v 1.8 2003/02/28 22:47:56 plearner Exp $
   ******************************************************* */

#include "plearn_main.h"
#include "PLearnCommandRegistry.h"
#include "stringutils.h"
#include "random.h"
#include "PLMPI.h"

// Things to get help on
#include "getDataSet.h"
#include "Learner.h"
#include "Splitter.h"
#include "VMatrix.h"
#include "Optimizer.h"
#include "Kernel.h"
#include "Variable.h"

namespace PLearn <%
using namespace std;


int plearn_main(int argc, char** argv)
{
  PLMPI::init(&argc, &argv);

  seed();

  string programname = argv[0];
  if(argc<=1)
    {
      cerr << "Type '" << programname << " help' for help" << endl;
      return 0;
    }
  else 
    {
      string command = argv[1];
      if(command=="help")
        {
          if(argc==2)
            {
              cerr << "Available commands are: " << endl;
              PLearnCommandRegistry::print_command_summary(cerr);
              cerr << endl;
              cerr << "Type 'plearn help xxx' to get detailed help on command xxx \n\n" 
                "You may also run plearn with the name of a plearn script file as argument\n"
                "A plearn script file should have a name ending in .plearn\n"
                "It can use macro variable definitions and expansion. Macro commands start by a $\n"
                "ex: $DEFINE{toto=[1,2,3,4]}  ${toto}  $INCLUDE{otherfile.pscript} \n"
                "Macro variable definitions can also be provided on the command line in the form \n"
                "varname=varvalue with each such pair separated by a blank, thus\n"
                "allowing for scripts with arguments\n\n"
                "A plearn script must contain at least one runnable PLearn object\n"
                "Typical runnable PLearn objects are 'Experiment' and 'ComparisonExperiment'\n\n"
                "You can type 'plearn help xxx' to get a description and the list of build options\n"
                "for any instantiable PLearn object xxx \n\n"
                "In addition you can get a list of all instantiable subclasses of the following \n"
                "base classes yyy by typing 'plearn help yyy':\n"
                "   Learner, Splitter, VMatrix, Optimizer, Kernel, Variable \n\n"
                "Finally 'plearn help datasets' will print some help on datasets\n" << endl;
            }
          else
            {
              string aboutwhat = argv[2];
              if(PLearnCommandRegistry::is_registered(aboutwhat))
                PLearnCommandRegistry::help(aboutwhat, cout);
              else if(aboutwhat=="datasets")
                cout << getDataSetHelp();
              else if(aboutwhat=="Learner")
                displayRegisteredSubClassesOf<Learner>("Learner", cout);
              else if(aboutwhat=="Optimizer")
                displayRegisteredSubClassesOf<Optimizer>("Optimizer", cout);
              else if(aboutwhat=="Kernel")
                displayRegisteredSubClassesOf<Kernel>("Kernel", cout);
              else if(aboutwhat=="Splitter")
                displayRegisteredSubClassesOf<Splitter>("Splitter", cout);
              else if(aboutwhat=="VMatrix")
                displayRegisteredSubClassesOf<VMatrix>("VMatrix", cout);
              else if(aboutwhat=="Variable")
                displayRegisteredSubClassesOf<Variable>("Variable", cout);
              else
                displayObjectHelp(cout, aboutwhat);
            }
        }
      else if(PLearnCommandRegistry::is_registered(command))
        {
          vector<string> args = stringvector(argc-2, argv+2);
          PLearnCommandRegistry::run(command, args);
        }
      else // we suppose it's a filename of a .psave or .pexp file containing Objects to be run
        {
          if(!file_exists(command))
            {
              cerr << "** ERROR: " << command << " appears to be neither a valid plearn command type, nor an existing filename" << endl;
              cerr << "Type plearn with no argument to see the help." << endl;
              exit(0);
            }

          map<string, string> vars;
          // populate vars with the arguments passed on the command line
          for(int i=2; i<argc; i++)
            {
              string option = argv[i];
              pair<string,string> name_val = split_on_first(option, "=");
              vars[name_val.first] = name_val.second;
            }
          PStream pout(&cout);
          pout << vars << endl;
      
          string script = readFileAndMacroProcess(command, vars);
          PIStringStream in(script);

          while(in)
            {
              PP<Object> o = readObject(in);
              o->run();
              in.skipBlanksAndCommentsAndSeparators();
              // cerr << bool(in) << endl;
              // cerr << in.peek() << endl;
            }
        }


    }


  PLMPI::finalize();
  return 0;
}


%> // end of namespace PLearn
