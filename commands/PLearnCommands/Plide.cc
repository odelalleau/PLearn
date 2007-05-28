// -*- C++ -*-

// Plide.cc
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

/*! \file Plide.cc */

#define PL_LOG_MODULE_NAME "Plide"

// Python includes must come first
#include <plearn/python/PythonCodeSnippet.h>

// From C++ stdlib
#include <map>
#include <sstream>

// From boost
#include <boost/smart_ptr.hpp>

// From PLearn
#include "Plide.h"
#include "plearn_main.h"
#include "PLearnCommandRegistry.h"
#include <plearn/base/TypeFactory.h>
#include <plearn/base/ProgressBar.h>
#include <plearn/io/fileutils.h>             // chdir
#include <plearn/io/openString.h>
#include <plearn/io/StringPStreamBuf.h>
#include <plearn/io/pl_log.h>
#include <plearn/io/PyPLearnScript.h>

namespace PLearn {
using namespace std;

//#####  Progress Bar that's mapped to Plide Window  ##########################

class PlideProgressPlugin : public ProgressBarPlugin
{
public:
    PlideProgressPlugin(PythonCodeSnippet* python)
        : m_python(python)
    {
        PLASSERT( m_python );
    }

    //! Destructor releases all pending progress bars from GUI
    virtual ~PlideProgressPlugin();

    //! Try to allocate a GUI element for progress bar
    virtual void addProgressBar(ProgressBar* pb);

    //! Release the allocated GUI element for progress bar
    virtual void killProgressBar(ProgressBar* pb);

    //! Update GUI element if there's one associated with progress bar
    virtual void update(ProgressBar* pb, unsigned long newpos);

protected:
    //! Python environment for execution
    PythonCodeSnippet* m_python;
    
    //! Map from the progress bar to a GUI id for transmitting the update.
    //! The id is -1 if there is no associated GUI element.
    map<ProgressBar*, int> m_progress_ids;
};

PlideProgressPlugin::~PlideProgressPlugin()
{
    PLASSERT( m_python );
    for (map<ProgressBar*, int>::const_iterator it = m_progress_ids.begin(),
             end = m_progress_ids.end() ; it != end ; ++it)
    {
        // Don't call killProgressBar since iterators might be invalidated
        m_python->invoke("ReleaseProgressBar", int(it->second));
    }
    m_progress_ids.clear();
}

void PlideProgressPlugin::addProgressBar(ProgressBar* pb)
{
    PLASSERT( pb && m_python );
    if (m_progress_ids.find(pb) == m_progress_ids.end()) {
        m_progress_ids[pb] =
            m_python->invoke("AllocateProgressBar", pb->title).as<int>();
    }
}

void PlideProgressPlugin::killProgressBar(ProgressBar* pb)
{
    PLASSERT( pb && m_python );
    map<ProgressBar*, int>::iterator found = m_progress_ids.find(pb);
    if (found != m_progress_ids.end()) {
        m_python->invoke("ReleaseProgressBar", int(found->second));
        m_progress_ids.erase(found);
    }
}

void PlideProgressPlugin::update(ProgressBar* pb, unsigned long newpos)
{
    PLASSERT( pb && m_python );
    map<ProgressBar*, int>::const_iterator found = m_progress_ids.find(pb);
    if (found != m_progress_ids.end()) {
        int progress_id = found->second;
        double fraction = double(newpos) / double(pb->maxpos);
        m_python->invoke("ProgressUpdate", progress_id, fraction);
    }
}


//#####  PlideLogPStreamBuf  ##################################################

/**
 *  This class sends stuff to the PlideLog when it's flushed.
 */
class PlideLogPStreamBuf : public StringPStreamBuf
{
    typedef StringPStreamBuf inherited;

public:
    PlideLogPStreamBuf(string* s, PythonCodeSnippet* python)
        : inherited(s, "w"),
          m_python(python),
          m_requested_verbosity(-1)
    { }

    //! Establish current-mode module-name and verbosity
    void outputParameters(const string& module_name, int requested_verbosity);

    //! Flush actually invokes the Python function LogAppend if the
    //! string is not empty, and it empties it.
    virtual void flush();

protected:
    PythonCodeSnippet* m_python;
    string m_module_name;
    int m_requested_verbosity;
};

void PlideLogPStreamBuf::outputParameters(const string& module_name, int requested_verbosity)
{
    m_module_name = module_name;
    m_requested_verbosity = requested_verbosity;
}

void PlideLogPStreamBuf::flush()
{
    PLASSERT( st && m_python );
    inherited::flush();
    if (! st->empty()) {
        m_python->invoke("LogAppend", m_module_name, m_requested_verbosity, *st);
        st->clear();
    }
}


/**
 *  This plugin connects the logging mechanism to PlideLogPStreamBuf
 */
class PlideLogPlugin : public PL_LogPlugin
{
public:
    PlideLogPlugin(PythonCodeSnippet* python)
        : m_string(new std::string),
          m_streambuf(new PlideLogPStreamBuf(m_string.get(), python)),
          m_pstream(m_streambuf)
    { }

    virtual PStream& getStream(PStream::mode_t outmode, const string& module_name,
                               int requested_verbosity);

protected:
    boost::scoped_ptr<std::string> m_string;
    PP<PlideLogPStreamBuf>         m_streambuf;
    PStream                        m_pstream;
};

PStream& PlideLogPlugin::getStream(PStream::mode_t outmode, const string& module_name,
                                   int requested_verbosity)
{
    PLASSERT( m_streambuf );
    m_pstream.setOutMode(outmode);
    m_streambuf->outputParameters(module_name, requested_verbosity);
    m_streambuf->flush();
    return m_pstream;
}



//#####  Implementation of Command  ###########################################

//! This allows to register the 'Plide' command in the command registry
PLearnCommandRegistry Plide::reg_(new Plide);

Plide::Plide()
    : PLearnCommand(
        "plide",
        "Starts up the PLearn Integrated Development Environment (PLIDE)",
        "Upon running, this command starts up a graphical user interface to\n"
        "interface more easily with PLearn.\n"
        )
{ }

const char* plide_code = "from plearn.plide.plide import *\n" ;

//! The actual implementation of the 'Plide' command
void Plide::run(const vector<string>& args)
{
    m_python = new PythonCodeSnippet(plide_code);
    m_python->build();

    // A bunch of injections
    m_python->inject("versionString",     this, &Plide::versionString);
    m_python->inject("getAllClassnames",  this, &Plide::getAllClassnames);
    m_python->inject("helpResourcesPath", this, &Plide::helpResourcesPath);
    m_python->inject("helpIndex",         this, &Plide::helpIndex);
    m_python->inject("helpCommands",      this, &Plide::helpCommands);
    m_python->inject("helpClasses",       this, &Plide::helpClasses);
    m_python->inject("helpOnCommand",     this, &Plide::helpOnCommand);
    m_python->inject("helpOnClass",       this, &Plide::helpOnClass);
    m_python->inject("setOptionLevel",    this, &Plide::setOptionLevel);
    m_python->inject("toggleOptionFlag",  this, &Plide::toggleOptionFlag);
    m_python->inject("precisOnClass",     this, &Plide::precisOnClass);
    m_python->inject("loggingControl",    this, &Plide::loggingControl);

    // Start the thing!
    m_python->invoke("StartPlide", args);

    // Link progress bar stuff to GUI elements
    PP<ProgressBarPlugin> previous_progress_plugin = ProgressBar::getCurrentPlugin();
    PP<ProgressBarPlugin> pp_ppp = new PlideProgressPlugin(m_python);
    ProgressBar::setPlugin(pp_ppp);

    // Link logging stuff to GUI elements; change stream mode to pretty ascii
    PL_Log::instance().outmode(PStream::pretty_ascii);
    PP<PL_LogPlugin> previous_log_plugin = PL_Log::instance().getCurrentPlugin();
    PP<PL_LogPlugin> log_plugin = new PlideLogPlugin(m_python);
    PL_Log::instance().setPlugin(log_plugin);
    
    // Main loop: get work, dispatch, start again
    while (true) {
        TVec<string> work = m_python->invoke("GetWork").as< TVec<string> >();

        if (work.size() != 4)
            PLERROR("%sExpected to receive a work vector with 4 elements; got %d",
                    __FUNCTION__, work.size());

        const string& request_id  = work[0];
        const string& script_code = work[1];
        const string& root_dir    = work[2];
        const string& script_kind = work[3];
        MODULE_LOG << "Got work request " << request_id
                   << " in directory "    << root_dir
                   << " with script (subset):\n"
                   << script_code.substr(0,100)
                   << endl;

        if (script_code == "Quit()")
            break;

        string result_code;
        string result_details;
        try {
            if (script_kind == "pyplearn")
                executePyPLearn(script_code, root_dir);
            else
                PLERROR("%sUnknown/unsupported script kind '%s'",
                        __FUNCTION__, script_kind.c_str());
        }
        catch (const PLearnError& e) {
            result_code = "PLearnError";
            result_details = e.message();
        }
        catch (const std::exception& e) {
            result_code = "std::exception";
            result_details = e.what();
        }
        catch (...) 
        {
            result_code = "UnknownError";
            result_details =
                "uncaught unknown exception "
                "(ex: out-of-memory when allocating a matrix)";
        }

        m_python->invoke("PostWorkResults", request_id, result_code, result_details);
    }

    // Finally, quit the GTK loop
    m_python->invoke("QuitPlide");

    // Bring back previous plugin before shutting down Python
    PL_Log::instance().setPlugin(previous_log_plugin);
    ProgressBar::setPlugin(previous_progress_plugin);
}


//#####  Injected: Utilities  #################################################

PythonObjectWrapper Plide::versionString(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 0)
        PLERROR("%sExpecting 0 argument; got %d", __FUNCTION__, args.size());
    return version_string();
}

PythonObjectWrapper Plide::getAllClassnames(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 0)
        PLERROR("%sExpecting 0 argument; got %d", __FUNCTION__, args.size());

    const TypeMap& tm = TypeFactory::instance().getTypeMap();
    TVec<string> all_classes;
    all_classes.resize(0, tm.size());
    for (TypeMap::const_iterator it = tm.begin(), end = tm.end()
             ; it != end ; ++it)
    {
        // Skip abstract classes
        if (it->second.constructor)
            all_classes.push_back(it->second.type_name);
    }
    return all_classes;
}

PythonObjectWrapper Plide::helpResourcesPath(const TVec<PythonObjectWrapper>& args)
{
// DEPRECATED: see HelpSystem
/*
    if (args.size() != 1)
        PLERROR("%sExpecting 1 argument; got %d", __FUNCTION__, args.size());

    PPath base_path = args[0].as<string>();
    m_help_config   = new HTMLHelpConfig;
    m_help_config->html_index_document  = "";
    m_help_config->html_prolog_document = base_path / "help_prolog.html";
    m_help_config->html_epilog_document = base_path / "help_epilog.html";
    
    m_help_command = dynamic_cast<HTMLHelpCommand*>(
        PLearnCommandRegistry::commands()["htmlhelp"]);
    if (! m_help_command)
        PLERROR("%sThe PLearn command 'HTMLHelpCommand' must be linked into "
                "the executable in order to use the Plide help system.", __FUNCTION__);
*/
    return PythonObjectWrapper();
}

PythonObjectWrapper Plide::helpIndex(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 0)
        PLERROR("%sExpecting 0 argument; got %d", __FUNCTION__, args.size());

    PLASSERT( m_help_config && m_help_command );
    ostringstream os;
    m_help_command->helpIndex(os, m_help_config);
    return os.str();
}

PythonObjectWrapper Plide::helpCommands(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 0)
        PLERROR("%sExpecting 0 argument; got %d", __FUNCTION__, args.size());

    PLASSERT( m_help_config && m_help_command );
    ostringstream os;
    m_help_command->helpCommands(os, m_help_config);
    return os.str();
}

PythonObjectWrapper Plide::helpClasses(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 0)
        PLERROR("%sExpecting 0 argument; got %d", __FUNCTION__, args.size());

    PLASSERT( m_help_config && m_help_command );
    ostringstream os;
    m_help_command->helpClasses(os, m_help_config);
    return os.str();
}

PythonObjectWrapper Plide::helpOnCommand(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 1)
        PLERROR("%sExpecting 1 argument; got %d", __FUNCTION__, args.size());

    PLASSERT( m_help_config && m_help_command );
    ostringstream os;
    m_help_command->helpOnCommand(args[0].as<string>(), os, m_help_config);
    return os.str();
}

PythonObjectWrapper Plide::helpOnClass(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 1)
        PLERROR("%sExpecting 1 argument; got %d", __FUNCTION__, args.size());

    PLASSERT( m_help_config && m_help_command );
    ostringstream os;
    m_help_command->helpOnClass(args[0].as<string>(), os, m_help_config);
    return os.str();
}

PythonObjectWrapper Plide::setOptionLevel(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 1)
        PLERROR("%sExpecting 1 argument; got %d", __FUNCTION__, args.size());
    OptionBase::OptionLevel desired_level = args[0].as<OptionBase::OptionLevel>();
    OptionBase::setCurrentOptionLevel(desired_level);
    return PythonObjectWrapper();
}

PythonObjectWrapper Plide::toggleOptionFlag(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 1)
        PLERROR("%sExpecting 1 argument; got %d", __FUNCTION__, args.size());
    OptionBase::flag_t flag_to_toggle = args[0].as<OptionBase::flag_t>();
    OptionBase::setCurrentFlags(OptionBase::getCurrentFlags() ^ flag_to_toggle);
    return PythonObjectWrapper();
}


PythonObjectWrapper Plide::precisOnClass(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 1)
        PLERROR("%sExpecting 1 argument; got %d", __FUNCTION__, args.size());

    string classname = args[0].as<string>();
    if (TypeFactory::instance().isRegistered(classname)) {
        const TypeMapEntry& tme = TypeFactory::instance().getTypeMapEntry(classname);
        OptionList& options = (*tme.getoptionlist_method)();
        TVec<string> build_options;
        build_options.resize(0, options.size());

        // Determine buildoptions
        for (OptionList::iterator it=options.begin(), end=options.end()
                 ; it != end ; ++it)
        {
            int flags = (*it)->flags();
            if (flags & OptionBase::buildoption)
                build_options.push_back((*it)->optionname());
        }
        return make_pair(tme.one_line_descr, build_options);
    }
    else
        return PythonObjectWrapper();        // None
}


PythonObjectWrapper Plide::loggingControl(const TVec<PythonObjectWrapper>& args) const
{
    if (args.size() != 2)
        PLERROR("%sExpecting 2 arguments; got %d", __FUNCTION__, args.size());

    int desired_verbosity = args[0].as<int>();
    vector<string> module_names = args[1].as< vector<string> >();
    PL_Log::instance().verbosity(desired_verbosity);
    PL_Log::instance().enableNamedLogging(module_names);
    return PythonObjectWrapper();
}


//#####  Work Executors  ######################################################

void Plide::executePyPLearn(const string& script_code, const string& root_dir) const
{
    PLearn::chdir(PPath(root_dir));
    PStream pyplearn_in = openString(script_code, PStream::plearn_ascii);
    PP<PyPLearnScript> pyplearn_script = new PyPLearnScript;
    pyplearn_in >> pyplearn_script;
    PLASSERT( pyplearn_script );

    string pyplearn_script_string = pyplearn_script->getScript();
    PStream plearn_in = openString(pyplearn_script_string,
                                   PStream::plearn_ascii);
    while (plearn_in) {
        PP<Object> o = readObject(plearn_in);
        o->run();
        plearn_in.skipBlanksAndCommentsAndSeparators();
    }

    // Save out experiment.plearn and metainfos.txt
    pyplearn_script->close();
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
