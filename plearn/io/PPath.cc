// -*- C++ -*-

// PPath.cc
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
 * $Id$ 
 ******************************************************* */

// Authors: Christian Dorion

/*! \file PPath.cc */

// #define PL_LOG_MODULE_NAME "PPath"

#include <ctype.h>
#include <nspr/prenv.h>

#include "PPath.h"
#include "PStream.h"
#include "openFile.h"
#include "pl_log.h"
#include "fileutils.h"
#include <plearn/base/stringutils.h>    //!< For the split() method.

///////////////////////  
// DOS SETTINGS
#if defined(WIN32) && !defined(__CYGWIN__)

#include <direct.h>
#define SYS_GETCWD     _getcwd
  
///////////////////////  
// POSIX SETTINGS
#else

#include <unistd.h>
#define SYS_GETCWD     ::getcwd

#endif

#define PPATH_SLASH '/'  // The canonical slash.

namespace PLearn {
using namespace std;

////////////////////////////////////////////////////////
//  Stringutils.h         //////////////////////////////
bool startsWith(const string& str, const char& c) 
{
    if (str.empty())
        return false;
    return str[0] == c;
}

bool endsWith  (const string& str, const char& c) 
{
    if (str.empty())
        return false;
    return str[str.length()-1] == c;
}

bool startsWith(const string& str, const string& s) 
{
    if ( s.length() > str.length() )
        return false;
    return str.substr(0, s.length()) == s;
}

bool endsWith  (const string& str, const string& s) 
{
    if ( s.length() > str.length() )
        return false;
    return str.substr(str.length()-s.length()) == s;
}
////////////////////////////////////////////////////////


///////////////////////  
// DOS SETTINGS
#if defined(WIN32)
string PPath::forbidden_chars()
{
    return string();
}

const string& PPath::_slash()
{
    static string s = "\\"; 
    return s;
}

char          PPath::_slash_char()    { return '\\'; }  
  
///////////////////////  
// POSIX SETTINGS
#else
// Even if the posix standard allows backslashes in file paths,
// PLearn users should never use those since PLearn aims at full and
// easy portability.
//
// Other chars may be forbidden soon.
string PPath::forbidden_chars()
{
    return "\\";
}

const string& PPath::_slash()
{
    static string s = "/"; 
    return s; 
}

char   PPath::_slash_char()    { return '/';  }

#endif  

  
//////////////////////////////////////////////  
// In the scope of the PLearn namespace 
PStream& operator<<(PStream& out, const PPath& path)
{
    switch (out.outmode) {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
    {
        out << path.c_str();
        break;
    }
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
    {
        out << path.canonical();
        break;
    }
    default:
        PLERROR("This PStream mode is not supported for PPath");
    }
    return out;
}

PStream& operator>>(PStream& in, PPath& path)
{
    string spath;
    in >> spath;
    switch (in.inmode) {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
    case PStream::plearn_ascii:
    case PStream::plearn_binary:
    {
        path = PPath(spath);
        break;
    }
    default:
        PLERROR("In operator>> - This PStream mode is not supported for PPath");
    }
    return in;
}

//////////////////////////////////////////////  
// Static PPath methods

//////////
// home //
//////////
PPath PPath::home()
{
    // Supply a default value so PLearn does not crash.
    // when $HOME isn't defined.
#ifdef WIN32
#define PL_DEFAULT_HOME PPath("C:\\")
#else
#define PL_DEFAULT_HOME PPath("/")
#endif
    return PPath::getenv("HOME", PL_DEFAULT_HOME);
}

////////////
// getcwd //
////////////
PPath PPath::getcwd()
{
    // TODO Use a NSPR function when there is one:
    //      https://bugzilla.mozilla.org/show_bug.cgi?id=280953
    char buf[2000];
    if (!SYS_GETCWD(buf, 2000))
        // Error while reading the current directory. One should probably use a
        // larger buffer, but it is even easier to crash.
        PLERROR("In PPath::getcwd - Could not obtain the current working "
                "directory, a larger buffer may be necessary");
    return PPath(buf);
}

////////////
// getenv //
////////////
PPath PPath::getenv(const string& var, const PPath& default_)
{
    char* env_var = PR_GetEnv(var.c_str());
    if ( env_var )
        return PPath( env_var );
    return default_;
}

////////////////////////////
// metaprotocolToMetapath //
////////////////////////////

// Static map that stores the binding metaprotocol <-> metapath.
// It is embedded within a function to prevent potential compiler issues during
// static initialization (e.g. under Windows with gcc 3.4.4).
map<string, PPath>& metaprotocol_to_metapath() {
    static map<string, PPath> metaprotocol_to_metapath;
    return metaprotocol_to_metapath;
}

const map<string, PPath>& PPath::metaprotocolToMetapath()
{
    static  bool                mappings;
  
    if ( !mappings )
    {
        // Avoiding infinite loop.
        mappings = true;

        PPath   plearn_configs   = PPath::getenv( "PLEARN_CONFIGS",
                                                  PPath::home() / ".plearn" );    

        PPath   config_file_path = plearn_configs / "ppath.config";

        if (isfile(config_file_path))
        {
            PStream ppath_config = openFile(config_file_path,
                                            PStream::plearn_ascii);

            string  next_metaprotocol;
            PPath   next_metapath;    
            while (ppath_config.good()) {
                ppath_config >> next_metaprotocol >> next_metapath;
                if (next_metaprotocol.empty()){
                    if (ppath_config.good())
                        PLERROR("In PPath::metaprotocolToMetapath - Error while parsing PPath config file (%s): read "
                                "a blank line before reaching the end of the file",
                                config_file_path.errorDisplay().c_str());
                    else
                        // Nothing left to read.
                        break;
                }
                // Make sure we managed to read the metapath associated with the metaprotocol.
                if (next_metapath.empty())
                    PLERROR("In PPath::metaprotocolToMetapath - Error in PPath config file (%s): could not read the "
                            "path associated with '%s'",
                            config_file_path.errorDisplay().c_str(), next_metaprotocol.c_str());
        
                // For the sake of simplicity, we do not allow a metapath to end with
                // a slash unless it is a root directory.
                next_metapath.removeTrailingSlash();
        
                if (!addMetaprotocolBinding(next_metaprotocol, next_metapath))
                    PLWARNING("In PPath::metaprotocolToMetapath - Metaprotocol"
                              " '%s' is being redefined, please check your "
                              "ppath.config (%s)",
                              next_metaprotocol.c_str(),
                              config_file_path.errorDisplay().c_str());
            }       
        }
        else
        {
            if (PR_GetEnv("HOME"))
            {
                // Default PPath settings. Defined only if the HOME environment
                // variable exists.
                metaprotocol_to_metapath()["HOME"] = "${HOME}";
                metaprotocol_to_metapath()["PLEARNDIR"] = "HOME:PLearn";
                metaprotocol_to_metapath()["PLEARN_LIBDIR"] = "PLEARNDIR:external_libs";
            }
        }
    }

    return metaprotocol_to_metapath();
}

////////////////////////////
// addMetaprotocolBinding //
////////////////////////////
bool PPath::addMetaprotocolBinding(const string& metaprotocol,
                                   const PPath& metapath,
                                   bool  force)
{
    const map<string, PPath>& bindings = metaprotocolToMetapath();
    bool already_here = bindings.find(metaprotocol) != bindings.end();
    if (!already_here || force)
        metaprotocol_to_metapath()[metaprotocol] = metapath;
    return !already_here;
}


// This method MUST NOT return a path since it would lead to an infinite
// loop of constructors.
string PPath::expandEnvVariables(const string& path)
{
    string expanded     = path;
    size_t begvar       = expanded.find( "${" );
    size_t endvar       = expanded.find(  "}" );
  
    while ( begvar != npos && endvar != npos  )
    {
        size_t start       = begvar+2;
        size_t len         = endvar - start;
        string envvar      = expanded.substr(start, len);
        PPath  envpath     = PPath::getenv(envvar);

        if ( envpath == "" )
            PLERROR( "Unknown environment variable %s in %s.",
                     envvar.c_str(), path.c_str() );

        expanded.replace(begvar, len+3, envpath);

        // Look for other environment variables
        begvar = expanded.find( "${" );
        endvar = expanded.find(  "}" );
    }

    // This method MUST NOT return a path since it would lead to an infinite
    // loop of ctors.
    return expanded;
}


//////////////////////////
// setCanonicalInErrors //
//////////////////////////
bool PPath::canonical_in_errors = false;

void PPath::setCanonicalInErrors(bool canonical)
{
    PPath::canonical_in_errors = canonical;
}

#if defined(__CYGWIN__)

extern "C" void cygwin_conv_to_win32_path(const char *path,
                                          char *win32_path);
#endif

//////////////////////////////////////////////  
// PPath methods

PPath::PPath(const char* path)
    : _protocol("")
{
    // MODULE_LOG << "PPath(const char* path = " << path << ")" << endl;
    operator=( PPath(string(path)) );
}

// The canonical path always contains '/' delimiters to seperate
// subdirectories. Under windows, the internal representation will
// however keep the '\' version. Under Unix, the following simply copy the
// path value in the current instance.
PPath::PPath(const string& path_)
    : _protocol("")
{
    // MODULE_LOG << "PPath(const string& path_ = " << path_ << ")" << endl;
    
    // Empty path.
    if ( path_.empty() ) 
        return;
    const string* the_path = &path_;
#if defined(__CYGWIN__) || defined(_MINGW_)
#ifdef __CYGWIN__
    static char buf[3000];
#endif
    string new_path;
    // This is a hack to try to get the right DOS path from Cygwin.
    // Because Cygwin has its own translation rules, not necessarily compatible
    // with the PPath ones, we ask it to translate the path iff it starts with
    // a UNIX '/' character. TODO We will need a better home-made function
    // to translate paths safely.
    if (startsWith(*the_path, '/')) {
#if defined(__CYGWIN__)
        cygwin_conv_to_win32_path(the_path->c_str(), buf);
        new_path = string(buf);
#elif defined(_MINGW_)
        // We need to convert the path by ourselves.
        if (!startsWith(*the_path, "/cygdrive/")) {
            PLWARNING("Path '%s' is expected to start with '/cygdrive/'",
                    the_path->c_str());
            new_path = *the_path;
        } else {
            // Remove '/cygdrive'.
            new_path = the_path->substr(9);
            // Copy drive letter from second to first position.
            new_path[0] = new_path[1];
            // Add ':' after drive letter.
            new_path[1] = ':';
            // Replace '/' by '\'.
            for (string::size_type i = 0; i < new_path.size(); i++)
                if (new_path[i] == '/')
                    new_path[i] = '\\';
        }
#endif
        the_path = &new_path;
    }
#endif

    // The path_ argument may contain environment variables that must be
    // expanded prior to any other processing.
    string internal =  expandEnvVariables( *the_path );
  
    // The PPath internal string is set here
    string::operator=   ( internal );
    resolveSlashChars   ( );
    expandMetaprotocols ( );
    resolveDots         ( );    
    parseProtocol       ( );
    // pout << "Creating PPath from '" << *the_path << "' --> '" << string(*this)
    //      << "'" << endl;
}

///////////////////////
// resolveSlashChars //
///////////////////////
void PPath::resolveSlashChars( )
{
    size_t plen = length();      
    string resolved;
    resolved.reserve( plen );
  
    bool last_is_slash = false;
    for ( size_t ch = 0; ch < plen; ch++ )
    {
        char char_ch = operator[](ch);
        if ( forbidden_chars().find(char_ch) != npos )
            PLERROR( "PPath '%s' cannot contain character '%c' (or any of \"%s\").",
                     c_str(), char_ch, forbidden_chars().c_str() );
    
        // Convert the canonic representation '/' to the appropriate
        // representation given the system (see slash_char instanciation in
        // PPath.cc). Multiple slash chars are removed.
        if ( char_ch == PPATH_SLASH || char_ch == _slash_char() )
        {
            if( !last_is_slash ) { // Prevents duplicated slash characters.
                resolved += _slash_char();
                last_is_slash = true;
            }
        }
        else {
            resolved += char_ch;
            last_is_slash = false;
        }
    }

    string::operator=(resolved);
}

/////////////////////////
// expandMetaprotocols //
/////////////////////////
void PPath::expandMetaprotocols()
{
    size_t endmeta = find(':');
    if ( endmeta != npos )    
    {
        string meta = substr(0, endmeta);
        map<string, PPath>::const_iterator it = metaprotocolToMetapath().find(meta);

        PPath metapath;
        if ( it != metaprotocolToMetapath().end() )
            metapath = it->second;
        else
            metapath = getenv(meta);

        if ( !metapath.isEmpty() )
        {      
            string after_colon = endmeta == length()-1 ? "" : substr(endmeta+1);
            *this = metapath / after_colon;
        }
    }
}

///////////////////////
// resolveSingleDots //
///////////////////////
void PPath::resolveSingleDots() {
    static string ds, sd;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        ds   = "."    + _slash();  // Posix: "./"    DOS: ".\" 
        sd   = _slash() + ".";     // Posix: "/."    DOS: "\."
    }
    // Examples with single dots.
    // ./foo      -> foo
    // ./         -> ./
    // .          -> .
    // /.         -> /
    // /./        -> /
    // /./foo     -> /foo
    // foo/.      -> foo
    // foo/./     -> foo/
    // foo/./bar  -> foo/bar
    // foo/.bar   -> foo/.bar

    // First deal with "/.":
    // - when it is followed by a slash, remove it
    // - when it ends the path, remove the dot, and remove the slash unless
    //   the resulting directory is a root directory
    size_t pos_sd = find(sd);
    size_t next;
    while (pos_sd != npos) {
        if (pos_sd + 2 < size()) {
            if (operator[](pos_sd + 2) == _slash_char()) {
                // It is followed by a slash.
                replace(pos_sd, 2, ""); // Remove '/.'
                next = pos_sd;
            }
            else
                next = pos_sd + 2;      // We ignore this one (e.g. "/.plearn").
        } else {
            // It ends the path.
            resize(size() - 1);       // Remove '.'
            if (!isRoot())
                resize(size() - 1);     // Remove '/'
            next = size();            // We reached the end.
        }
        pos_sd = find(sd, next);
    }

    // Now deals with "./". Because we have removed the "/." before, we cannot
    // have "/./" nor "./.". Thus the only case we have to consider is when it
    // starts the path (and this can happen only once), in which case we remove
    // it iff there is something that follows.
    size_t pos_ds = find(ds);
    if (pos_ds == 0 && pos_ds + 2 < size())
        replace(0, 2, "");
}

///////////////////////
// resolveDoubleDots //
///////////////////////
void PPath::resolveDoubleDots() {
    static string sdd;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        sdd  = _slash() + "..";    // Posix: "/.."   DOS: "\.." 
    }
    // Examples with double dots.
    // "/.."         -> PLERROR
    // "/../foo"     -> PLERROR
    // "../foo"      -> "../foo"
    // "foo/.."      -> "."
    // "foo/../"     -> "./"
    // "/foo/.."     -> "/"
    // "/foo/../"    -> "/"
    // "foo/../bar"  -> "./bar" (call resolveSingleDots() after)
    // "/foo/../bar" -> "/bar"
    // "/..foo"      -> "/..foo"
    // "foo../"      -> "foo../"
    // "../../../foo"-> "../../../foo"
    // "foo/../../.."-> "foo/../../.."

    // We only care about "/.." when it is followed by a slash or it ends the path.
    // The path made of the substring until the '/' must not be a root directory.
    size_t pos_sdd = find(sdd);
    size_t next;
    while (pos_sdd != npos) {
        if (pos_sdd + 3 < size() && operator[](pos_sdd + 3) != _slash_char()) {
            // Something like "/..foo", that we ignore.
            next = pos_sdd + 4;
        } else {
            // Look for the slash just before.
            size_t pos_previous_slash = pos_sdd == 0 ? npos
                : rfind(_slash_char(), pos_sdd - 1);
            if (pos_previous_slash == npos) {
                // We need to make sure we are not trying to go up on a root directory.
                if (PPath(substr(0, pos_sdd + 1)).isRoot())
                    PLERROR("In PPath::resolveDots - '%s' is invalid",
                            errorDisplay().c_str());
                if (   (pos_sdd == 2 && substr(0,2) == "..")
                       || (pos_sdd == 1 && operator[](0) == '.'))
                    // We are in the case "../.." or "./.."
                    next = pos_sdd + 3;
                else {
                    // We are in the case "foo/.."
                    replace(0, pos_sdd + 3, ".");
                    next = 1;
                }
            } else {
                // There was a slash: "/xxx/..". If "xxx" == "..", we do nothing,
                // otherwise we get rid of it.
                if (substr(pos_previous_slash+1, 2) == "..")
                    next = pos_sdd + 3;
                else {
                    // We must be careful with the special case "/foo/..", where we
                    // need to ensure we keep a final slash.
                    if (   pos_sdd + 3 == size()    // Ends with "/.."
                           && (   pos_previous_slash == 0
                                  || rfind(_slash_char(), pos_previous_slash-1) == npos) // "xxx/foo/.."
                           && PPath(substr(0, pos_previous_slash+1)).isRoot())  // "xxx/" is root
                        replace(pos_previous_slash + 1, pos_sdd-pos_previous_slash+2, "");
                    else
                        replace(pos_previous_slash, pos_sdd-pos_previous_slash+3, "");
                    next = pos_previous_slash;
                }
            }
        }
        pos_sdd = find(sdd, next);
    }
    // At this point, we may have introduced an extra single dot ("./xxx").
    resolveSingleDots();
}

/////////////////
// resolveDots //
/////////////////
void PPath::resolveDots()
{
    // First remove temporarily the protocol.
    if (!_protocol.empty())
        replace(0, _protocol.size() + 1, "");

    resolveSingleDots();
    resolveDoubleDots();

    // Put back the protocol.
    if (!_protocol.empty())
        insert(0, _protocol + ":");
}

///////////////////
// parseProtocol //
///////////////////
void PPath::parseProtocol()
{
    size_t endpr = find(':');
  
    // No specified protocol  
    if ( endpr == npos )
    {
        // Even if the default protocol is considered to be the file protocol,
        // the _protocol member keeps the exact protocol value in the
        // string. The protocol() method, however, returns FILE_PROTOCOL if the
        // protocol was not specified.
        _protocol = "";
    }

    // The substring preceeding the ':' delimiter COULD be the protocol.  
    else
    {
        _protocol = substr(0, endpr);

        if ( _protocol == FILE_PROTOCOL ||
             _protocol == HTTP_PROTOCOL ||
             _protocol == FTP_PROTOCOL  )
        {
            // Make sure we do not define a protocol for a relative path.
            PPath check_filepath_validity = removeProtocol();
            if ( !check_filepath_validity.isEmpty() &&
                 !check_filepath_validity.isAbsPath() )
                PLERROR("A PPath should not specify a protocol "
                        "for a relative path (in %s).", c_str());
        }

        // Nothing prevents a file from containing a ':' char. Under dos, for
        // instance, a letter preceeding ':' represents the drive. Hence, if
        // we do not recognize the protocol, we assume the ':' was part of
        // the file name.
        else
            _protocol = "";
    }
}

//////////////
// absolute //
//////////////
PPath PPath::absolute(bool add_protocol) const
{
    if (!add_protocol && protocol() != FILE_PROTOCOL)
        PLERROR("In PPath::absolute - The absolute() method is only meant for "
                "the FILE_PROTOCOL protocol when 'add_protocol' is false");

    PPath abspath;

    // An empty path remains empty when converted to absolute.
    if ( isEmpty() || isAbsPath() )
        abspath = PPath( *this );

    // This is necessarily a file protocol (because other protocols require
    // an absolute path).
    // ===> we concatenate the current working directory of the process.
    else
    {
        PLASSERT( _protocol.empty() );
        abspath = PPath::getcwd() / *this;
    }

    // Remove useless trailing slash.
    abspath.removeTrailingSlash();
    // Add / remove protocol if required. Note that we cannot add a protocol to
    // an empty PPath (as an empty PPath is considered relative).
    if (add_protocol && !abspath.isEmpty())
        abspath = abspath.addProtocol();
    else
        // There can be a protocol in abspath only if there is one in *this.
        if (!_protocol.empty())
            abspath = abspath.removeProtocol();

    return abspath;
}

///////////////
// canonical //
///////////////
string PPath::canonical() const
{
    // An empty path does not need to be modified.
    if (isEmpty())
        return *this;

    // We have to replace any special path by its canonic equivalent.
    // Note that the protocol is kept. This means in particular that
    // if ppath = "/foo/bar" and the metaprotocol FOO maps to "file:/foo",
    // then the canonical form of ppath will still be "/foo/bar", and not
    // "FOO:bar" (we may want to change this behavior in the future).

    string canonic_path = *this;
    EXTREME_LOG << plhead("canonic_path: "+canonic_path) << endl;

    map<string, PPath>::const_iterator it  = metaprotocolToMetapath().begin();
    map<string, PPath>::const_iterator end = metaprotocolToMetapath().end();

    string metaprotocol;
    string metapath;      // Used to store the longest metapath found so far.
    while ( it != end )
    {      
        const string& candidate = it->second;
        if ( candidate.length() < metapath.length() )
        {
            // The candidate is shorter, we are not interested.
            EXTREME_LOG << "Shorter:\n\t"
                        << it->first << " -> " << candidate.c_str() << endl;
            ++it;
            continue;
        }
        if ( !startsWith(canonic_path, candidate) ) {
            // No match.
            EXTREME_LOG << "No match:\n\t"
                        << it->first << " -> " << candidate.c_str() << endl;
            ++it;
            continue;
        }

        size_t endpath = candidate.length();

        // The current candidate is only a subtring of the canonic path.
        // Ex:
        //    /home/dorionc/hey
        // in
        //    /home/dorionc/heyYou. 
        // Note that if the canonic path is a root directory, it may end
        // with a slash, in which case this cannot happen.
        if ( endpath != canonic_path.length()     &&
             canonic_path[endpath] != _slash_char() &&
             !endsWith(candidate, _slash_char()) )
        {
            EXTREME_LOG << "Substring:\n\t" 
                        << it->first << " -> " << it->second.c_str() << endl;
            ++it;
            continue;
        }

        // The current candidate is indeed a subpath of canonic_path.
        metaprotocol = it->first;
        metapath     = candidate;
        EXTREME_LOG << "Kept:\n\t" 
                    << it->first << " -> " << candidate.c_str() << endl;
        ++it; // We iterate to find the longest matching candidate.
    }

    // If any metapath was found, it must be replaced by its metaprotocol
    // equivalent.
    if ( metaprotocol.length() > 0 ) {
        canonic_path.replace( 0, metapath.length(), metaprotocol+':' );
        // Remove the slash just after the ':' if there is something following.
        size_t after_colon = metaprotocol.size() + 1;
        if (canonic_path.size() > after_colon + 1 &&
            canonic_path[after_colon] == _slash_char())
            canonic_path.erase(after_colon, 1);
    }

    // If necessary, convert slash characters to the canonical slash.
    if (_slash_char() != PPATH_SLASH) {
        size_t slash_pos = 0;
        while ((slash_pos = canonic_path.find(_slash_char(), slash_pos)) != npos)
            canonic_path[slash_pos] = PPATH_SLASH;
    }

    return canonic_path;
}

//////////////////
// errorDisplay //
//////////////////
string PPath::errorDisplay() const
{
    if (PPath::canonical_in_errors)
        return this->canonical();
    else
        return this->absolute();
}

/////////////////
// addProtocol //
/////////////////
PPath PPath::addProtocol()  const
{
    if ( _protocol.empty()) {
        if (!isAbsPath())
            PLERROR("In PPath::addProtocol - A protocol can only be added to an "
                    "absolute path, and '%s' is relative", errorDisplay().c_str());
        return ( PPath(string(FILE_PROTOCOL) + ':' + string(*this)) );
    }
    return PPath( *this );
}

////////////////////
// removeProtocol //
////////////////////
PPath PPath::removeProtocol()  const
{
    if ( _protocol.length()==0 )
        return PPath(*this);
    PPath no_protocol;
    // Avoid a call to the PPath constructor from a string.
    no_protocol.assign(substr(_protocol.length()+1));
    return no_protocol;
}

///////////////
// operator/ //
///////////////
PPath PPath::operator/(const PPath& other) const
{
    return ( PPath(*this) /= other );
}

////////////////
// operator/= //
////////////////
PPath& PPath::operator/=(const PPath& other)
{
    // MODULE_LOG << this->c_str() << " /= " << other << endl;
    
    if (other.isAbsPath())
        PLERROR("In PPath::operator/= - The concatenated path (%s) cannot be absolute",
                other.c_str());
    // Add a slash if needed.
    // Note that 'other' cannot start with a slash, otherwise it would be an
    // absolute directory.
    if ( !isEmpty  ()                  &&
         !endsWith (*this, _slash_char()) )
        string::operator+=(_slash_char());
    string::operator+=(other);

    resolveDots       ( );
    return *this;
}

////////////////
// operator== //
////////////////
bool PPath::operator== (const string& other) const
{
    // MODULE_LOG << this->c_str() << " == " << other << " (string)"<< endl;    
    if ( other.empty() )
        return isEmpty();
    if ( isEmpty() )
        return false; // since 'other' is not
    return operator==( PPath(other) );
}

bool PPath::operator==(const PPath& other) const
{
    // MODULE_LOG << this->c_str() << " == " << other << " (PPath)"<< endl;
    
    // If they are stricly equal there is no need to go further.
    // Otherwise they must point to the same absolute file or directory.
    // Note that the absolute() method already removes the trailing slash.
    return (   !strcmp(c_str(), other.c_str())
               || !strcmp(absolute(true).c_str(), other.absolute(true).c_str()));
}

////////
// up //
////////
PPath PPath::up() const
{
    if (isEmpty() || isRoot())
        PLERROR("In PPath::up - Cannot go up on directory '%s'",
                errorDisplay().c_str());
    return *this / "..";
}

/////////////
// dirname //
/////////////
PPath PPath::dirname() const
{
    if (isEmpty() || isRoot())
        return PPath(*this);
    size_t slash_pos = rfind(_slash_char());
    if ( slash_pos == npos ){
        if (_protocol.empty())
            return ".";  
        else
            return _protocol + ":.";
    }
    PPath result = substr(0, slash_pos + 1);
    // Remove trailing slash if it is not a root directory.
    result.removeTrailingSlash();
    return result;
}

//////////////
// basename //
//////////////
PPath PPath::basename() const
{
    size_t slash_pos = rfind(_slash_char());
    if ( slash_pos == npos )
        return PPath(*this);  
    return substr(slash_pos+1);
}

//////////////
// hostname //
//////////////
string PPath::hostname() const
{
    if (!isAbsPath())
        PLERROR("In PPath::hostname - Can only be used with an absolute path");
    if (isRoot())
        PLERROR("In PPath::hostname - The path cannot be a root directory");
    int i = 0;
    PPath paths[2];
    paths[0] = *this;
    paths[1] = paths[0].up();
    while (!paths[1 - i].isRoot()) {
        i = 1 - i;
        paths[1 - i] = paths[i].up();
    }
    paths[i].removeTrailingSlash();
    return paths[i].basename();
}

///////////////
// extension //
///////////////
string PPath::extension(bool with_dot) const
{
    PPath base = basename();
    size_t begext = base.rfind('.');
    if ( begext == npos            || // Not found.
         begext == base.length()-1 )  // Filename ending with a dot.
        return "";
    return with_dot ? base.substr(begext) : base.substr(begext+1);
}

//////////////////
// no_extension //
//////////////////
PPath PPath::no_extension() const
{
    size_t ext_size = extension().size();
    string copy(*this);
    if (ext_size > 0)
        copy.resize(size() - ext_size - 1);  // Remove the extension and the dot.
    return copy;
}

// // Returns a ppath shorter than 256 character and exempt of any of the
// // following chars: "*?'\"${}[]@ ,()"  --- replaced by underscores. 
// PPath PPath::makeFileNameValid(const PPath& path) const
// {
//   PPath  valid    = path;  
//   PPath  dirname  = path.dirname();
//   PPath  filename = path.basename();
//   
//   if ( filename.length() > 256 )
//   {
//     string ext             = filename.extension();
// 
//     int    kept_length     = 256-ext.length()-12;
//     string filename_kept   = filename.substr( 0, kept_length );    
// 
//     int    rest_length     = filename.length() - kept_length - ext.length();
//     string rest            = filename.substr( kept_length, rest_length );
// 
//     int j= 0;
//     do
//     {
//       unsigned int n= j++;
//       for(unsigned int i= 0; i < rest_length; ++i)
//       {
//         int m=0;
//         switch(i%4)
//         {
//         case 3: m= 1; break;
//         case 2: m= 256; break;
//         case 1: m= 65536; break;
//         case 0: m= 256*65536; break;
//         }
//         n+= m*(unsigned char)rest[i];
//       }
//       
// 
//       valid = dirname / filename_noext + "-" + tostring(n) + ext;
//     }
//     while( pathexists( valid ) );
// 
//     PLWARNING("makeFilenameValid: Filename '%s' changed to '%s'.", 
//               path.c_str(), (dirname + filename_wo_ext + ext).c_str());
//     valid = dirname + filename_wo_ext + ext;
//   }
//   
//   // replace illegal characters
//   char illegal[]="*?'\"${}[]@ ,()";
//   for(int i=0;i<(signed)valid.size();i++)
//     for(int j=0;j<15;j++)
//       if(valid[i]==illegal[j])
//         valid[i]='_';
//   return valid;
// }


#if defined(WIN32)
PPath PPath::drive() const
{
    if ( find(':') == 1 && isalpha( c_str()[0] ) )
        return PPath(substr(0, 2));
    return PPath("");
}

bool PPath::isabs() const
{
    // Note that a Win32 path is considered absolute if starting with a '\'
    // character: this is so that the path resulting from removing the protocol
    // in a ftp path for instance is still considered as absolute.
    return !drive().isEmpty() || isHttpPath() || isFtpPath() ||
           _protocol == FILE_PROTOCOL || startsWith(c_str(), _slash_char());
}

#else
PPath PPath::drive() const
{
    return PPath("");
}

bool PPath::isabs() const
{
    return startsWith(c_str(), _slash_char()) || isHttpPath() || isFtpPath() || _protocol == FILE_PROTOCOL;
}
// TODO What about file:foo/bar ?

#endif

////////////
// isRoot //
////////////
bool PPath::isRoot() const
{
    if (!isAbsPath())
        return false;
    PPath no_prot = removeProtocol();
    string drv = no_prot.drive();
    return string(no_prot) == drv + _slash();
}


////////////////////////
// parseUrlParameters //
////////////////////////
void PPath::parseUrlParameters(PPath& base_path, map<string, string>& parameters) const
{
    size_t pos = rfind('?');
    if (pos == string::npos) {
        base_path = *this;
        return;
    }
    size_t check = rfind('?', pos - 1);
    if (check != string::npos)
        PLERROR("In PPath::parseUrlParameters - There can be only one '?' in a PPath");
    base_path = substr(0, pos);
    string rest = substr(pos + 1);
    vector<string> pairs = PLearn::split(rest, '&');
    string equal = "=";
    string name, value;
    vector<string>::const_iterator it = pairs.begin();
    for (; it != pairs.end(); it++) {
        PLearn::split_on_first(*it, equal, name, value);
        if (!name.empty()) {
            if (value.empty())
                PLERROR("In PPath::parseUrlParameters - The parameter %s has no value",
                        name.c_str());
            parameters[name] = value;
        } else if (!value.empty())
            PLERROR("In PPath::parseUrlParameters - The value %s has no parameter name",
                    value.c_str());
    }
}

/////////////////////////
// removeTrailingSlash //
/////////////////////////
void PPath::removeTrailingSlash() {
    if (isEmpty() || (*this)[length() - 1] != _slash_char() || isRoot())
        return;
    resize(length() - 1);
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
