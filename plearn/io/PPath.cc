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
   * $Id: PPath.cc,v 1.12 2005/02/16 15:12:24 tihocan Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file PPath.cc */


#include "PPath.h"
#include <ctype.h>
#include <plearn/io/PStream.h>
#include <plearn/io/openFile.h>
#include <plearn/io/pl_log.h>

///////////////////////  
// DOS SETTINGS
#if defined(WIN32)

#include <direct.h>
#define SYS_GETCWD     _getcwd
  
///////////////////////  
// POSIX SETTINGS
#else

#include <unistd.h>
#define SYS_GETCWD     ::getcwd

#endif

#define PPATH_SLASH '/'  // The canonical slash.

// TODO Use NSPR for getcwd ?

namespace PLearn {
using namespace std;

//TMP!!! TODO Remove ?
static string left(const string& s, size_t width, char padding=' ')
{ 
  if(s.length()>width)
    return s;
  else
    return s+string(width-s.length(),padding);
}  

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
string PPath::forbidden_chars = "";

string PPath::_slash          = "\\";
char   PPath::_slash_char     = '\\';  
  
///////////////////////  
// POSIX SETTINGS
#else
// Even if the posix standard allows backslashes in file paths,
// PLearn users should never use those since PLearn aims at full and
// easy portability.
//
// Other chars may be forbidden soon.
string PPath::forbidden_chars = "\\";

string PPath::_slash      = "/";
char   PPath::_slash_char = '/';

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
      {
        out << path.canonical();
        break;
      }
    default:
      PLERROR("In operator<< - This PStream mode is not supported for PPath");
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
PPath PPath::home()
{
  return PPath( PR_GetEnv("HOME") );
}

PPath PPath::getcwd()
{
  char buf[2000];
  SYS_GETCWD(buf, 2000);
  return PPath(buf);
}

PPath PPath::getenv(const string& var, const PPath& default_)
{
  char* env_var = PR_GetEnv(var.c_str());
  if ( env_var )
    return PPath( env_var );
  return default_;
}

map<string, PPath>  PPath::metaprotocol_to_metapath;
void PPath::ensureMappings()
{
  static bool mappings;
  if ( !mappings )
  {
    // Avoiding infinite loop.
    mappings = true;

    PPath   plearn_configs   = PPath::getenv( "PLEARN_CONFIGS",
                                               PPath::home() / ".plearn" );    

    PPath   config_file_path = plearn_configs / "ppath.config";
    PStream ppath_config     = openFile(config_file_path, PStream::plearn_ascii );

    string  next_metaprotocol;
    PPath   next_metapath;    
    while (ppath_config) {
      ppath_config >> next_metaprotocol >> next_metapath;
      // Keep no trailing slash, because it makes it easier to manipulate
      // and recognize a metapath.
      // Only canonical '/' are allowed in a metapath.
      // TODO Problem if we define a metapath to be '/' ?
      if ( endsWith(next_metapath, PPATH_SLASH) )
        next_metapath.replace( next_metapath.length()-1,
                               next_metapath.length(), "");        

      metaprotocol_to_metapath[ next_metaprotocol  ]  = next_metapath;

//!<       cerr << left("next_metaprotocol: ", 20) << next_metaprotocol << endl
//!<            << left("next_metapath: ", 20)     << next_metapath     << endl << endl;
    }       
  }
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

//////////////////////////////////////////////  
// PPath methods

// The canonical path always contains '/' delimiters to seperate
// subdirectories. Under windows, the internal representation will
// however keep the '\' version. Under Unix, the following simply copy the
// path value in the current instance.
PPath::PPath(const string& path_)
{
  // Empty path.
  if ( path_.empty() ) 
    return;
    
  // The path_ argument may contain environment variables that must be
  // expanded prior to any other processing.
  string internal =  expandEnvVariables(  path_   );
  
  // The PPath internal string is set here
  string::operator=   ( internal );
  resolveSlashChars   ( );
  expandMetaprotocols ( );
  resolveDots         ( );    
  parseProtocol       ( );
}

// Ensure a valid internal representation
void PPath::resolveSlashChars( )
{
  const string internal = c_str();
    
  int plen = internal.length();      
  string resolved;
  resolved.reserve( plen );
  
  char last = ' ';  // Could have been anything but '/' or '\'
  for ( int ch=0; ch < plen; ch++ )
  {
    size_t forbidden = forbidden_chars.find( internal[ch] );    
    if ( forbidden != npos )
      PLERROR( "PPath %s can not contain %c chars (or any of \"%s\").",
               internal.c_str(), internal[ch], forbidden_chars.c_str() );
    
    // Convert the canonic representation '/' by the appropriate
    // representation given the system (see slash_char instanciation in
    // PPath.cc). Multiple slash chars are removed.
    if ( internal[ch] == PPATH_SLASH )
    {
      if( last != PPATH_SLASH )
        resolved += _slash_char;
    }

    // TODO Why not doing this systematically ? We could have a _canonical_slash_char too
    // and do something like if ( == _slash_char || canonical_slash_char).
    // TODO Do ze_car = internal[ch]
#if defined(WIN32)
    // Under DOS, the previous if statement manages canonical '/' (that are
    // forbidden in dos file paths). This if statement prevents multiple
    // backslashes.
    else if ( internal[ch] == _slash_char )
    {
      if( last != _slash_char )
        resolved += _slash_char;
    }
#endif
    
    else
      resolved += internal[ch];

    // The last char is kept to help remove doubled _slash_chars.
    last = internal[ch];
  }

  string::operator=(resolved);
}

// TODO What about file:HOME:PLearn ?

void PPath::expandMetaprotocols()
{
  ensureMappings();

  size_t endmeta = find(':');
  if ( endmeta != npos )    
  {
    string meta = substr(0, endmeta);

    map<string, PPath>::const_iterator it = metaprotocol_to_metapath.find(meta);

    if ( it != metaprotocol_to_metapath.end() )
    {      
      string after_colon = endmeta == length()-1 ? "" : substr(endmeta+1);
      *this = it->second / after_colon;
    }
  }

#ifdef __INTEL_COMPILER
#pragma warning(disable:279)  // Get rid of compiler warning.
#endif
  EXTREME_LOG << left("c_str(): ", 20) << c_str() << endl;
#ifdef __INTEL_COMPILER
#pragma warning(default:279)
#endif

}

///////////////////////
// resolveSingleDots //
///////////////////////
void PPath::resolveSingleDots() {
  static string ds, sd;
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    ds   = "."    + _slash;  // Posix: "./"    DOS: ".\" 
    sd   = _slash + ".";     // Posix: "/."    DOS: "\."
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
      if (operator[](pos_sd + 2) == _slash_char) {
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
    sdd  = _slash + "..";    // Posix: "/.."   DOS: "\.." 
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
    if (pos_sdd + 3 < size() && operator[](pos_sdd + 3) != _slash_char) {
      // Something like "/..foo", that we ignore.
      next = pos_sdd + 4;
    } else {
      // Look for the slash just before.
      size_t pos_previous_slash = pos_sdd == 0 ? npos
                                               : rfind(_slash_char, pos_sdd - 1);
      if (pos_previous_slash == npos) {
        // We need to make sure we are not trying to go up on a root directory.
        if (PPath(substr(0, pos_sdd + 1)).isRoot())
          PLERROR("In PPath::resolveDots - '%s' is invalid", c_str());
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
                  || rfind(_slash_char, pos_previous_slash-1) == npos) // "xxx/foo/.."
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

    if ( _protocol == FILE_PROTOCOL )
    {
      PPath check_filepath_validity = removeProtocol();
      if ( check_filepath_validity != "" &&
          !check_filepath_validity.isAbsPath() )
        PLERROR("A PPath should not specify the 'file:' protocol "
                "for a relative path (in %s).", c_str());
      // TODO Why ?
    }

    // Nothing prevents a file from containing a ':' char. Under dos, for
    // instance, a letter preceeding ':' represents the drive. Hence, if
    // we do not recognize the protocol, we assume the ':' was part of
    // the file name.
    else if ( _protocol != HTTP_PROTOCOL &&
              _protocol !=  FTP_PROTOCOL  )
      _protocol = "";
  }

#ifdef __INTEL_COMPILER
#pragma warning(disable:279)  // Get rid of compiler warning.
#endif
  EXTREME_LOG << left("_protocol: ", 20) << _protocol << endl;
#ifdef __INTEL_COMPILER
#pragma warning(default:279)
#endif

}

PPath PPath::absolute() const
{
  PPath abspath;

  // Http and ftp urls are considered absolute.
  // An empty path remains empty when converted to absolute.
  if ( isabs() || isEmpty() )
    abspath = PPath( *this );

  // File protocol (unspecified); relative file path.
  else
  {
    // No specified protocol and !isabs()
    //    ==> relative path (current working directory of the process)
    assert( _protocol == "" );
    abspath = PPath::getcwd() / *this;
  }
  abspath.removeTrailingSlash();
  
  return abspath;
}

// TODO Make sure we remove the file: protocol in absolute()

string PPath::canonical() const
{
  // An empty path does not need to be modified.
  // Http && ftp path are already considered as canonical. // TODO See if ok...
  if (isEmpty() || isHttpPath() || isFtpPath())
    return *this;

  // isFilePath()
  // We have to replace any special path by its canonic equivalent.

  // Remove the file protocol if any.
  string canonic_path = removeProtocol();
#ifdef __INTEL_COMPILER
#pragma warning(disable:279)  // Get rid of compiler warning.
#endif
  DBG_LOG << plhead("canonic_path: "+canonic_path) << endl;

  map<string, PPath>::const_iterator it  = metaprotocol_to_metapath.begin();
  map<string, PPath>::const_iterator end = metaprotocol_to_metapath.end();

  string metaprotocol;
  string metapath;
  while ( it != end )
  {      
    size_t begpath = canonic_path.find(it->second);
    // TODO Be more efficient.

    // The path does not start with the current candidate or is shorter
    // than the previous metapath found.
    if ( begpath != 0 || it->second.length() < metapath.length() )
    {
      DBG_LOG << "Invalid or shorter:\n\t"
        << it->first << " -> " << it->second.c_str() << endl;
      ++it;
      continue;
    }

    size_t endpath = it->second.length();

    // The current candidate is only a subtring of the canonic path.
    // Ex:
    //    /home/dorionc/hey
    // dans
    //    /home/dorionc/heyYou. 
    if ( endpath != canonic_path.length()     &&
        canonic_path[endpath] != _slash_char  )
    {
      DBG_LOG << "Substring:\n\t" 
        << it->first << " -> " << it->second.c_str() << endl;
      ++it;
      continue;
    }

    // The current candidate is indeed a subpath of canonic_path.
    metaprotocol = it->first;
    metapath     = it->second;
    DBG_LOG << "Kept:\n\t" 
      << it->first << " -> " << it->second.c_str() << endl;
#ifdef __INTEL_COMPILER
#pragma warning(default:279)
#endif
    ++it; // TODO Why do it more than once ? What if more than one metaprotocol ?
    // Probably to find the longest one.
    // But is it really the longest one ?
  }

  // If any metapath was found, it must be replaced by its metaprotocol
  // equivalent.
  if ( metaprotocol.length() > 0 ) {
    canonic_path.replace( 0, metapath.length(), metaprotocol+':' );
    // Remove the slash just after the ':' if there is something following.
    if (canonic_path.size() > metaprotocol.size() + 2)
      canonic_path.erase(metaprotocol.size() + 1, 1);
  }

  return canonic_path;
}

// TODO Check this if I mess with protocols.

PPath PPath::addProtocol()  const
{
  if ( _protocol == "" )
    return ( PPath("file:") += *this );
  return PPath( *this );
}

PPath PPath::removeProtocol()  const
{
  if ( _protocol == "" )
    return PPath(*this);
  return PPath( substr(_protocol.length()+1) );
}

// TODO Check operators.

// The resolveSlashChars and resolveDots were called on other upon construction
PPath PPath::operator+(const PPath& other) const
{
  return PPath( string(*this) + string(other) );
}

PPath& PPath::operator+=(const PPath& other)
{
  string::operator+=(other);
  resolveDots       ( );
  return *this;
}

PPath PPath::operator/(const PPath& other) const
{
  return ( PPath(*this) /= other );
}

PPath& PPath::operator/=(const PPath& other)
{
  // Add a slash if needed.
  // TODO The other path should not be absolute ?!
  if ( !isEmpty()                              &&
       !endsWith  (c_str(),       _slash_char) &&
       !startsWith(other.c_str(), _slash_char)  )
    string::operator+=(_slash);
  string::operator+=(other);

  resolveDots       ( );
  return *this;
}

////////////////
// operator== //
////////////////
bool PPath::operator== (const string& other) const
{
  if ( other.empty() )
    return isEmpty();
  return operator==( PPath(other) );
}

bool PPath::operator==(const PPath& other) const
{
  // If they are stricly equal there is no need to go further.
  // Otherwise they must point to the same absolute file or directory.
  // Note that the absolute() method already removes the trailing slash.
  return (   !strcmp(c_str(), other.c_str())
          || !strcmp(absolute().c_str(), other.absolute().c_str()));
}

////////
// up //
////////
PPath PPath::up() const
{
  if (isEmpty() || isRoot())
    PLERROR("In PPath::up - Cannot go up on directory '%s'",
             absolute().c_str());
  return *this / "..";
}

/////////////
// dirname //
/////////////
PPath PPath::dirname() const
{
  if (isEmpty() || isRoot())
    return PPath(*this);
  size_t slash_pos = rfind(_slash_char);
  if ( slash_pos == npos )
    if (_protocol.empty())
      return ".";  
    else
      return _protocol + ":.";
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
  size_t slash_pos = rfind(_slash_char);
  if ( slash_pos == npos )
    return PPath(*this);  
  return substr(slash_pos+1);
}

///////////////
// extension //
///////////////
string PPath::extension() const
{
  // TODO Better search from the end ?
  size_t begext = find('.');
  if ( begext == npos ||
       begext == length()-1 )
    return "";
  return substr(begext+1);
}

PPath PPath::no_extension () const
{
  // TODO Better search from the end ?
  size_t begext = find('.');
  if ( begext == npos ||
       begext == length()-1 )
    return *this;
  return substr(0, begext);
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
  return !drive().isEmpty() || isHttpPath() || isFtpPath() || _protocol == FILE_PROTOCOL;
}

#else
PPath PPath::drive() const
{
  return PPath("");
}

bool PPath::isabs() const
{
  return startsWith(c_str(), _slash_char) || isHttpPath() || isFtpPath() || _protocol == FILE_PROTOCOL;
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
  PPath drv = no_prot.drive();
  return string(no_prot) == string(drv) + _slash;
}

/////////////////////////
// removeTrailingSlash //
/////////////////////////
void PPath::removeTrailingSlash() {
  if (isEmpty() || (*this)[length() - 1] != _slash_char || isRoot())
    return;
  resize(length() - 1);
}

} // end of namespace PLearn

