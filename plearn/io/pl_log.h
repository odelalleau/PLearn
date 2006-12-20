// -*- C++ -*-

// pl_log.h
//
// Copyright (C) 2004-2006 Nicolas Chapados 
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

// Authors: Nicolas Chapados, Christian Dorion

/**
 *  @file pl_log.h
 */


#ifndef pl_log_INC
#define pl_log_INC

// From C++ stdlib
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <string>

// From Plearn
#include "PStream.h"

namespace PLearn {

#ifndef PL_LOG_MAXVERBOSITY
#define PL_LOG_MAXVERBOSITY 1000
#endif

#ifndef PL_LOG_MINVERBOSITY
#define PL_LOG_MINVERBOSITY 1
#endif

#ifndef PL_LOG_VERBOSITY
#define PL_LOG_VERBOSITY 499                 // block EXTREME by default
#endif

enum VerbosityLevel {
    VLEVEL_MAND     = 0,    // Mandatory
    VLEVEL_IMP      = 1,    // Important
    VLEVEL_NORMAL   = 5,    // Normal
    VLEVEL_DBG      = 10,   // Debug Info
    VLEVEL_EXTREME  = 500   // Extreme Verbosity
}; 


//#####  LogPlugin  ###########################################################

/**
 *  Provides several back-ends for displaying the log messages
 *
 *  Similarly to ProgressBarPlugin, this class allows several types of
 *  back-ends to render log messages.  The purpose of this plugin is to return
 *  a reference to a PStream that's ready for carrying out logging operations
 *  on the specified "name" and given verbosity.
 */
class PL_LogPlugin : public PPointable
{
public:
    virtual ~PL_LogPlugin();

    virtual PStream& getStream(PStream::mode_t outmode, const string& module_name,
                               int requested_verbosity) = 0;
};


/**
 *  Default implementation of PL_LogPlugin :: outputs to specified PStream
 *  (perr by default)
 */
class PL_LogPluginPStream : public PL_LogPlugin
{
public:
    PL_LogPluginPStream(PStream pstream)
        : m_pstream(pstream)
    { }
    
    virtual PStream& getStream(PStream::mode_t outmode, const string& module_name,
                               int requested_verbosity);

protected:
    PStream m_pstream;                       //!< Actual stream to use
};


class LogInterceptorPStreamBuf;              //!< Forward declare

/**
 *  Interceptive version PL_LogPlugin.  This plugin intercepts the requests
 *  made to specific log-names, and appends them to a list of log entries that
 *  is kept separately for each intercepted log-name.  Requests that are made
 *  to other log-names are passed through to a 'chained plugin', for further
 *  processing.
 */
class PL_LogPluginInterceptor : public PL_LogPlugin
{
    typedef PL_LogPlugin inherited;
    friend class LogInterceptorPStreamBuf;
    typedef map< string, deque<string> > LogMap;

public:
    PL_LogPluginInterceptor(const set<string>& intercept_lognames,
                            PL_LogPlugin* previous);

    virtual PStream& getStream(PStream::mode_t outmode, const string& module_name,
                               int requested_verbosity);

    //! Return the log entries accumulated so far under the given logname
    const deque<string>& logEntries(const string& logname) const;

    //! Erase all log entries associated with given logname.  If logname is
    //! empty, ALL entries are cleared, regardless of logname
    void clearLog(const string& logname);

protected:
    //! Append a new log entry under given logname
    void appendLog(const string& logname, const string& logentry);
    
protected:
    //! List of intercepted lognames
    set<string> m_intercept_lognames;

    //! Previous plugin to process log entries that are not intercepted
    PL_LogPlugin* m_previous;

    //! Current log entries for each logname
    mutable LogMap m_log_entries;

    //! Streambuf that's actually tasked with accumulating strings
    PP<LogInterceptorPStreamBuf> m_streambuf;

    //! PStream object that's returned by a call to getStream
    PStream m_pstream;
};


//#####  Logging Class Per Se  ################################################

class PL_Log
{
public:
    //! Constructor
    //! (Use default destructor, copy constructor, etc.)
    PL_Log(PP<PL_LogPlugin> plugin);
  
    //! Set the actual runtime verbosity.  This is a verbosity threshold;
    //! any "requested_verbosity" less than or equal to this verbosity is
    //! displayed by PL_LOG.
    void verbosity(int v)                     { runtime_verbosity = v; }

    //! Return the current runtime verbosity
    int verbosity() const                     { return runtime_verbosity; }

    //! Changes the output_stream outmode
    void outmode(PStream::mode_t outmode)     { m_outmode = outmode; }

    /**
     *  Underlying logging function.  If "requested_verbosity" is less than
     *  or equal to verbosity, then output_stream is returned; otherwise
     *  null_stream is returned.
     */
    PStream& logger(int requested_verbosity);

    /**
     *  Underlying named logging function.  Use this in conjunction with the
     *  MODULE_LOG define.  If logging is enabled for the specified module,
     *  then return output_stream; otherwise return null_stream.
     */
    PStream& namedLogger(const string& module_name,
                         int requested_verbosity);

    /**
     *  Named logging support.  Enable logging for the specified list
     *  of modules.  If the special "__ALL__" keyword is used, then all
     *  named logging is enabled.  If the special "__NONE__" keyword is
     *  used, then no named logging is enabled.  (Default = "__NONE__")
     */
    void enableNamedLogging(const vector<string>& module_names);

    //! Shortcut for enableNamedLogging: first split the module names by comma
    //! and call enableNamedLogging -- convenient for in-code logging enabling
    void enableNamedLogging(const string& comma_separated_module_names);

    //! Return the list of modules for which named logging is enabled
    vector<string> namedLogging() const;

    //! Return true if logging is enabled for the given module
    bool loggingEnabled(const string& module_name) const;
    
    //! Return number of times logger() has been called
    long loggerCount() const                   { return logger_count; }
  
    //! Return the current plugin
    PP<PL_LogPlugin> getCurrentPlugin()
    {
        return m_plugin;
    }

    //! Set a new plugin
    void setPlugin(PP<PL_LogPlugin> plugin)
    {
        m_plugin = plugin;
    }
    
    //! Return system-wide PL_Log
    static PL_Log& instance();

    //! Support stuff for heading manipulator
    struct Heading {
        Heading(string h_) : h(h_) {}
        string h;
    };

    /**
     * Parses a string to see whether or not it names a VerbosityLevel. If it
     * doesn't, tries the cast to an int.
     */
    static VerbosityLevel vlevelFromString(const string& v);
  
protected:
    int runtime_verbosity;
    PP<PL_LogPlugin> m_plugin;      //!< Used to obtain the actual logging stream
    PStream null_stream;            //!< Used when there is nothing to output
    PStream::mode_t m_outmode;      //!< Formatting instructions for logging
    long logger_count;              //!< Number of times logger() has been called
    set<string> enabled_modules;    //!< Modules for which logging is enabled.

    enum {
        AllModules = 0,           //!< Named logging enabled for all modules
        NoModules = 1,            //!< Named logging enabled for no modules
        SomeModules = 2           //!< Named logging enabled for some modules
    } named_logging_kind;
};


//#####  Main interface to the logging system  ################################

#define PL_LOG(v) if (v <= PL_LOG_VERBOSITY) PL_Log::instance().logger(v)
#define MAND_LOG      PL_LOG(VLEVEL_MAND)
#define IMP_LOG       PL_LOG(VLEVEL_IMP) 
#define NORMAL_LOG    PL_LOG(VLEVEL_NORMAL)
#define DBG_LOG       PL_LOG(VLEVEL_DBG)   
#define EXTREME_LOG   PL_LOG(VLEVEL_EXTREME)


//! An explicitly named log with a given level.  For use within templates.
#define PL_LEVEL_NAMED_LOG(name,level)                  \
    if (level <= PL_LOG_VERBOSITY)                      \
        PL_Log::instance().namedLogger(name, level)

//! Define various leveled named logs
#define MAND_NAMED_LOG(name)      PL_LEVEL_NAMED_LOG(name, VLEVEL_MAND)
#define IMP_NAMED_LOG(name)       PL_LEVEL_NAMED_LOG(name, VLEVEL_IMP) 
#define NAMED_LOG(name)           PL_LEVEL_NAMED_LOG(name, VLEVEL_NORMAL)
#define DBG_NAMED_LOG(name)       PL_LEVEL_NAMED_LOG(name, VLEVEL_DBG)   
#define EXTREME_NAMED_LOG(name)   PL_LEVEL_NAMED_LOG(name, VLEVEL_EXTREME)


/**
 *  If the "PL_LOG_MODULE_NAME" variable is defined before pl_log.h is
 *  included, then *_MODULE_LOG are defined to provide module-specified logging
 *  for that module.  From an implementation standpoint, we define a static
 *  string variable that holds the module name, and that variable is used.
 *
 *  The equivalent of MAND_, IMP_, NORMAL_, DBG_, AND EXTREME_ are defined for
 *  module logs as well, although NORMAL_MODULE_LOG is simply abbreviated
 *  MODULE_LOG.
 *
 *  NOTE: to avoid conflicts and strange behavior, you should only use this
 *  define within a .cc file (outside of templates).  For templates, you have
 *  to rely on NAMED_LOG for now.
 */
#ifdef PL_LOG_MODULE_NAME
# ifndef MODULE_LOG

   namespace {
       string pl_log_module_name_string(PL_LOG_MODULE_NAME);
   }

#  define MAND_MODULE_LOG    PL_LEVEL_NAMED_LOG(pl_log_module_name_string, VLEVEL_MAND)
#  define IMP_MODULE_LOG     PL_LEVEL_NAMED_LOG(pl_log_module_name_string, VLEVEL_IMP)
#  define MODULE_LOG         PL_LEVEL_NAMED_LOG(pl_log_module_name_string, VLEVEL_NORMAL)
#  define DBG_MODULE_LOG     PL_LEVEL_NAMED_LOG(pl_log_module_name_string, VLEVEL_DBG)
#  define EXTREME_MODULE_LOG PL_LEVEL_NAMED_LOG(pl_log_module_name_string, VLEVEL_EXTREME)

#endif // MODULE_LOG
#endif // PL_LOG_MODULE_NAME


//#####  Heading Support  #####################################################

//! Manipulator that displays a separator with the Logger count
PStream& plsep(PStream&);

//! Manipulator that displays a nice heading
inline PL_Log::Heading plhead(string s)
{
    return PL_Log::Heading(s);
}

//! Actually draw the heading
PStream& operator<<(PStream&, PL_Log::Heading);

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
