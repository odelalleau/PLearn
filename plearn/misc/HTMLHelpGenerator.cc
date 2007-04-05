// -*- C++ -*-

// HTMLHelpGenerator.cc
//
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

// Authors: Xavier Saint-Mleux

/*! \file HTMLHelpGenerator.cc */


#include "HTMLHelpGenerator.h"
#include <plearn/io/openFile.h>
#include <plearn/io/fileutils.h>
#include <plearn/base/stringutils.h>
#include <plearn/base/HelpSystem.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    HTMLHelpGenerator,
    "ONE LINE USER DESCRIPTION",
    "MULTI LINE\nHELP FOR USERS"
    );

HTMLHelpGenerator::HTMLHelpGenerator(const PPath& output_dir_, 
                                     const PPath& resources_dir_)
    :output_dir(output_dir_), resources_dir(resources_dir_)
{}

void HTMLHelpGenerator::run()
{
    openFile(output_dir/"index.html", PStream::raw_ascii, "w") 
        << HelpSystem::helpIndexHTML();
    helpCommands();
    helpFunctions();
    helpClasses();
}

void HTMLHelpGenerator::helpCommands()
{
    openFile(output_dir/"commands_index.html", PStream::raw_ascii, "w") 
        << HelpSystem::helpCommandsHTML();
    vector<string> commands= HelpSystem::listCommands();
    for(vector<string>::iterator it= commands.begin();
        it != commands.end(); ++it)
        openFile(output_dir/"command_"+(*it)+".html", PStream::raw_ascii, "w") 
            << HelpSystem::helpOnCommandHTML(*it);
}

void HTMLHelpGenerator::helpFunctions()
{
    openFile(output_dir/"functions_index.html", PStream::raw_ascii, "w") 
        << HelpSystem::helpFunctionsHTML();
}

void HTMLHelpGenerator::helpClasses()
{
    openFile(output_dir/"classes_index.html", PStream::raw_ascii, "w")
        << HelpSystem::helpClassesHTML();
    vector<string> commands= HelpSystem::listClasses();
    for(vector<string>::iterator it= commands.begin();
        it != commands.end(); ++it)
        openFile(output_dir/"class_"+(*it)+".html", PStream::raw_ascii, "w") 
            << HelpSystem::helpOnClassHTML(*it);
}

void HTMLHelpGenerator::build()
{
    inherited::build();
    build_();
}

void HTMLHelpGenerator::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

void HTMLHelpGenerator::declareOptions(OptionList& ol)
{
    declareOption(ol, "output_dir", &HTMLHelpGenerator::output_dir,
                  OptionBase::buildoption,
                  "Directory where the .html files should be generated");

    declareOption(ol, "resources_dir", &HTMLHelpGenerator::resources_dir,
                  OptionBase::buildoption,
                  "Directory where the HTML help resources should be found.");

    inherited::declareOptions(ol);
}

void HTMLHelpGenerator::build_()
{
    if(output_dir != "" && !pathexists(output_dir))
        force_mkdir(output_dir);
    HelpSystem::setResourcesPathHTML(resources_dir);
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
