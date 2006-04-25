// -*- C++ -*-

// pl_log.cc
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

/*! \file pl_log.cc */

// From C++ stdlib
#include <map>
#include <string>
#include <algorithm>
#include <iterator>

// From FinLearn
#include <plearn/base/stringutils.h>
#include "pl_log.h"

namespace PLearn {
using namespace std;


//#####  Default Plugin Implementation  #######################################

PL_LogPlugin::~PL_LogPlugin()
{ }

PStream& PL_LogPluginPStream::getStream(
    PStream::mode_t outmode, const string& module_name, int requested_verbosity)
{
    m_pstream.setOutMode(outmode);
    if (! module_name.empty())
        m_pstream << '[' << module_name << "] ";
    return m_pstream;
}


//#####  PL_Log Implementation  ###############################################

PL_Log::PL_Log(PP<PL_LogPlugin> plugin)
    : runtime_verbosity(VLEVEL_NORMAL),
      m_plugin(plugin),
      null_stream(get_pnull()),
      m_outmode(PStream::pretty_ascii),
      logger_count(0),
      named_logging_kind(NoModules)
{ }


/**
 *  Return a logger object given verbosity only
 */
PStream& PL_Log::logger(int requested_verbosity)
{
    static const string module_name;         // Empty string
    assert( m_plugin );
    logger_count++;
    if (requested_verbosity <= runtime_verbosity)
        return m_plugin->getStream(m_outmode, module_name, requested_verbosity);
    else
        return null_stream;
}


/**
 *  Return a logger if logging is enabled for the specified module name
 */
PStream& PL_Log::namedLogger(const string& module_name, int requested_verbosity)
{
    logger_count++;
    if (requested_verbosity <= runtime_verbosity &&
        (named_logging_kind == AllModules ||
         (named_logging_kind == SomeModules &&
          enabled_modules.find(module_name) != enabled_modules.end())))
    {
        return m_plugin->getStream(m_outmode, module_name, requested_verbosity);
    }

    return null_stream;
}


/**
 *  Enable named logging for the specified modules.  Watch if one of the
 *  elements is one of the special keywords __ALL__ or __NONE__, and handle
 *  them accordingly.
 */
void PL_Log::enableNamedLogging(const vector<string>& module_names)
{
    enabled_modules.clear();
    named_logging_kind = SomeModules;
    for (int i=0, n=module_names.size() ; i<n ; ++i) {
        if (module_names[i] == "__ALL__") {
            named_logging_kind = AllModules;
            break;
        }
        else if (module_names[i] == "__NONE__") {
            named_logging_kind = NoModules;
            break;
        }
        else
            enabled_modules.insert(module_names[i]);
    }
}


/**
 *  Shortcut for enableNamedLogging: first split the module names by comma
 *  and call enableNamedLogging -- convenient for in-code logging enabling
 */
void PL_Log::enableNamedLogging(const string& comma_separated_module_names)
{
    vector<string> module_names = split(comma_separated_module_names,
                                        ", \t\n\r");
    enableNamedLogging(module_names);
}

    
/**
 *  Return the list of modules for which named logging is enabled
 */
vector<string> PL_Log::namedLogging() const
{
    vector<string> r;
    if (named_logging_kind == AllModules)
        r.push_back("__ALL__");
    else if (named_logging_kind == NoModules)
        r.push_back("__NONE__");
    else
        copy(enabled_modules.begin(), enabled_modules.end(), back_inserter(r));

    return r;
}
            

PL_Log& PL_Log::instance()    
{
    static PP<PL_LogPlugin> global_plugin = new PL_LogPluginPStream(get_perr());
    static PL_Log global_logger(global_plugin);
    return global_logger;
}


/**
 * Parses a string to see whether or not it names a VerbosityLevel. If it
 * doesn't, tries the cast to an int.
 */
VerbosityLevel PL_Log::vlevelFromString(const string& v)
{
    static map<string, VerbosityLevel> _vlevels;
    if ( _vlevels.size() == 0 )
    {
        _vlevels["VLEVEL_MAND"]    = VLEVEL_MAND;
        _vlevels["VLEVEL_IMP"]     = VLEVEL_IMP;
        _vlevels["VLEVEL_NORMAL"]  = VLEVEL_NORMAL ;
        _vlevels["VLEVEL_DBG"]     = VLEVEL_DBG    ;
        _vlevels["VLEVEL_EXTREME"] = VLEVEL_EXTREME;
    }
  
    map<string, VerbosityLevel>::iterator it = _vlevels.find(v);
    if ( it != _vlevels.end() )
        return it->second;
    return (VerbosityLevel)toint(v);
}


PStream& plsep(PStream& p)
{
    return p << plhead("");
}


PStream& operator<<(PStream& p, PL_Log::Heading heading)
{
    // The loggerCount is likely to change in test even if nothing more is
    // printed... PyTest dislikes. 
    //   string msg = "#####  " + tostring(PL_Log::instance().loggerCount())
    //     + (heading.h.size() > 0? (": "+heading.h) : string("")) + "  ";
    string msg = "#####  " + (heading.h.size() > 0? (heading.h + "  ") : string(""));
    string::size_type rest_length = msg.size() > 70 ? 5 : 75 - msg.size();
    string rest(rest_length,'#');
    return p << endl << (msg + rest) << endl;
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
