// -*- C++ -*-

// Plide.h
//
// Copyright (C) 2006 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file Plide.h */


#ifndef Plide_INC
#define Plide_INC

// Python includes must come first
#include <plearn/python/PythonCodeSnippet.h>

// From PLearn
#include <commands/PLearnCommands/PLearnCommand.h>
#include <commands/PLearnCommands/PLearnCommandRegistry.h>
#include <commands/PLearnCommands/HTMLHelpCommand.h>

namespace PLearn {

/**
 *  Command to start the PLearn Integrated Development Environment (PLIDE)
 *
 *  Upon running, this command starts up a graphical user interface to
 *  interface more easily with PLearn.
 */
class Plide : public PLearnCommand
{
    typedef PLearnCommand inherited;

public:
    Plide();
    virtual void run(const std::vector<std::string>& args);

public:
    //#####  Injected: Utilities  #############################################

    //! PLearn version string
    PythonObjectWrapper versionString(const TVec<PythonObjectWrapper>& args) const;

    //! List of classes registered for serialization (does not return abstract
    //! classes since the user may not instantiate them)
    PythonObjectWrapper getAllClassnames(const TVec<PythonObjectWrapper>& args) const;

    /**
     *  Establish the directory in which elements for HTML help generation
     *  should be found.  This comes from Python since it's relative to the
     *  location of the Plide Python code.
     */
    PythonObjectWrapper helpResourcesPath(const TVec<PythonObjectWrapper>& args);

    //! Return the help index as a string
    PythonObjectWrapper helpIndex(const TVec<PythonObjectWrapper>& args) const;
    
    //! Generate list of registered commands
    PythonObjectWrapper helpCommands(const TVec<PythonObjectWrapper>& args) const;
    
    //! Generate list of registered classes
    PythonObjectWrapper helpClasses(const TVec<PythonObjectWrapper>& args) const;

    //! Generate documentation for a given command
    PythonObjectWrapper helpOnCommand(const TVec<PythonObjectWrapper>& args) const;
    
    //! Generate documentation for a given class
    PythonObjectWrapper helpOnClass(const TVec<PythonObjectWrapper>& args) const;

    //! Generate the short (one-line) documentation for a given class, as well
    //! as the list of all BUILD options it supports.  This is returned as a pair
    //! (doc, options_list).  Return None if no such class exists.
    PythonObjectWrapper precisOnClass(const TVec<PythonObjectWrapper>& args) const;

    //! Logging control.  This function should be called with two arguments:
    //! 1) Desired verbosity level (integer)
    //! 2) List of module names to activate in named logging (list of strings)
    PythonObjectWrapper loggingControl(const TVec<PythonObjectWrapper>& args) const;
    

    //#####  Work Executors  ##################################################

    //! Execute .pyplearn script (given as a character string) using the
    //! specified directory as cwd
    void executePyPLearn(const string& script_code, const string& root_dir) const;
    
public:
    //! Make available the puthon snippet to supporting classes (e.g. progress
    //! bars that display in the GUI environment)
    PP<PythonCodeSnippet> m_python;

    //! HTML generator for the Help system
    //! (points to the systemwide command instance)
    HTMLHelpCommand* m_help_command;
    
    //! Configuration object for the HTML Help system
    PP<HTMLHelpConfig> m_help_config;

protected:
    static PLearnCommandRegistry reg_;
};


} // end of namespace PLearn

#endif


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
