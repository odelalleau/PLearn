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
   * $Id: PPath.cc,v 1.2 2005/01/18 17:09:10 dorionc Exp $ 
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

namespace PLearn {
using namespace std;

//TMP!!!
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
  if ( str.length() == 0 )
    return false;
  return str.substr(0,1)[0] == c;
}

bool endsWith  (const string& str, const char& c) 
{
  if ( str.length() == 0 )
    return false;
  return str.substr(str.length()-1, str.length())[0] == c;
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
  return str.substr(str.length()-s.length(), str.length()) == s;
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
  string canonic = path.canonical(); 
  out << canonic;
  return out;
}
  
PStream& operator>>(PStream& in, PPath& path)
{
  string spath;
  in >> spath;
  path = PPath(spath).absolute();
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
    PStream ppath_config     = openFile( config_file_path.c_str(), PStream::plearn_ascii );

    string          next_metaprotocol;
    ppath_config >> next_metaprotocol;

    PPath           next_metapath;    
    ppath_config >> next_metapath;

    while ( next_metaprotocol != "" && next_metapath != "" )
    {
      // Keep no trailing slash!!!
      if ( endsWith(next_metapath, '/') )
        next_metapath.replace( next_metapath.length()-1,
                               next_metapath.length(), "");        

      metaprotocol_to_metapath[ next_metaprotocol  ]  = next_metapath;

//!<       cerr << left("next_metaprotocol: ", 20) << next_metaprotocol << endl
//!<            << left("next_metapath: ", 20)     << next_metapath     << endl << endl;
      
      ppath_config    >>    next_metaprotocol;
      ppath_config    >>    next_metapath; 
    }       
  }
}

// This method MUST NOT return a path since it would lead to an infinite
// loop of ctors.
string PPath::expandEnvVariables(const string& path)
{
  string expanded     = path;
  unsigned int begvar = expanded.find( "${" );
  unsigned int endvar = expanded.find(  "}" );
  
  while ( begvar != npos && endvar != npos  )
  {
    unsigned int start = begvar+2;
    unsigned int len   = endvar - start;
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
  if ( path_ == "" ) 
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
    unsigned int forbidden = forbidden_chars.find( internal[ch] );    
    if ( forbidden != npos )
      PLERROR( "PPath %s can not contain %c chars (or any of \"%s\").",
               internal.c_str(), internal[ch], forbidden_chars.c_str() );
    
    // Convert the canonic representation '/' by the appropriate
    // representation given the system (see slash_char instanciation in
    // PPath.cc). Multiple slash chars are removed.
    if ( internal[ch] == '/' )
    {
      if( last != '/' )
        resolved += _slash_char;
    }

#if defined(WIN32)
#error \ == /: overload operator==    
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

void PPath::expandMetaprotocols()
{
  ensureMappings();

  unsigned int endmeta = find(':');
  if ( endmeta != npos )    
  {
    string meta = substr(0, endmeta);

    map<string, PPath>::const_iterator it =\
      metaprotocol_to_metapath.find(meta);

    if ( it != metaprotocol_to_metapath.end() )
    {      
      string after_colon = endmeta == length()-1 ? "" : substr(endmeta+1, length());
      *this = it->second / after_colon;
    }
  }

  EXTREME_LOG << left("c_str(): ", 20) << c_str() << endl;
}

void PPath::resolveDots()
{
  string ds   = "."    + _slash;  // Posix: "./"    DOS: ".\" 
  string sds  = _slash + ds;      // Posix: "/./"   DOS: "\.\"

  string dds  = ".."   + _slash;  // Posix: "../"    DOS: "..\"
  string sdds = _slash + dds;     // Posix: "/../"   DOS: "\..\" 
  
  string       prefix;
  while ( substr(0, 3) == dds )
  {
    prefix  += dds;
    replace(0, 3, "");
  }
  
  unsigned int begddots = find( sdds );
  
  // The double dots are considered only if these are within the path
  // ("/../"). If not, these are left for a later (and better)
  // interpretation since, for instance, a file could well be name
  // "..filename". Also not that we allow PPath("../filename") so that the
  // statement PPath("foo/bar") / PPath("../filename") works and leads to
  // PPath("foo/filename").
  while ( begddots != npos )
  {
    // Note here that the first slash of "/../" is neglected by the rfind
    // so that we actually go up in the path
    unsigned int upend = rfind(_slash, begddots-1);
    string replace_by  = _slash;

    if ( upend == npos )
    {
      upend      = 0;
      replace_by = "";
    }
    
    int replace_len = begddots - upend + 4;
    replace(upend, replace_len, replace_by);
    while ( substr(0, 3) == dds )
    {
      prefix += dds;
      replace(0, 3, "");
    }

    begddots = find( sdds );    
  }

  // Put the prefix back in the string
  if ( prefix != "" )
    insert(0, prefix);

  // Unlike for "../", we want to remove the useless "./" in "./filename".
  else while ( substr(0, 2) == ds )
    replace(0, 2, "");

  unsigned int begdot = find( sds );
  while ( begdot != npos )
  {
    replace(begdot, 3, _slash);
    while ( substr(0, 2) == ds )
      replace(0, 2, "");

    begdot = find( sds );
  }
}

void PPath::parseProtocol()
{
  unsigned int endpr = find(':');
  
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
    }

    // Nothing prevents a file from containing a ':' char. Under dos, for
    // instance, a letter preceeding ':' represents the drive. Hence, if
    // we do not recognize the protocol, we assume the ':' was part of
    // the file name.
    else if ( _protocol != HTTP_PROTOCOL &&
              _protocol !=  FTP_PROTOCOL  )
      _protocol = "";
  }

  EXTREME_LOG << left("_protocol: ", 20) << _protocol << endl;
}

PPath PPath::absolute() const
{
  PPath abspath;

  // Http and ftp urls are considered absolute.
  if ( isabs() )
    abspath = PPath( *this );

  // File protocol (unspecified); relative file path.
  else
  {
    // No specified protocol and !isabs()
    //    ==> relative path (current working directory of the process)
    assert( _protocol == "" );
    abspath = PPath::getcwd() / *this;
  }
  
  return abspath;
}

PPath PPath::canonical() const
{
  PPath canonic_path;

  // http && ftp path are already considered as canonical
  if ( isHttpPath() || isFtpPath() )
      canonic_path = PPath( *this );

  // isFilePath()
  //  We have to replace any special path by its canonic equivalent.
  else
  {
    // Remove the file protocol if any.
    canonic_path = removeProtocol().absolute();
    DBG_LOG << plhead("canonic_path: "+canonic_path) << endl;
    
    map<string, PPath>::const_iterator it  = metaprotocol_to_metapath.begin();
    map<string, PPath>::const_iterator end = metaprotocol_to_metapath.end();

    string metaprotocol;
    string metapath;
    while ( it != end )
    {      
      unsigned int begpath = canonic_path.find(it->second);

      // The path does not start with the current candidate or is shorter
      // than the previous metapath found.
      if ( begpath != 0 || it->second.length() < metapath.length() )
      {
        DBG_LOG << "Invalid or shorter:\n\t"
                 << it->first << " -> " << it->second.c_str() << endl;
        ++it;
        continue;
      }
      
      unsigned int endpath = it->second.length();

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

      ++it;
    }

    // If any metapath was found, it must be replaced by its metaprotocol
    // equivalent.
    if ( metaprotocol.length() > 0 )
    {
      if ( canonic_path.length() == metapath.length() )
        canonic_path.replace( 0, metapath.length(), metaprotocol );

      // Replace the '/' by a ':' (metapath never contain trailingSlash --
      // see ensureMappings)
      else 
        canonic_path.replace( 0, metapath.length()+1, metaprotocol+':' );
    }
  }

  // Remove the trailing slash if any
  int canonic_len = canonic_path.length();
  if ( canonic_len > 0 && canonic_path[ canonic_len-1 ] == _slash_char )
    canonic_path.replace(canonic_len-1, canonic_len, "");
  
  return canonic_path;
}

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
  return PPath( substr(_protocol.length()+1, length()) );
}

// The resolveSlashChars and resolveDots were called on other upon construction
//!< PPath PPath::operator+(const PPath& other) const
//!< {
//!<   return ( PPath(*this) += other );
//!< }

//!< PPath& PPath::operator+=(const PPath& other)
//!< {
//!<   string::operator+=(other);
//!<   resolveSlashChars ( );
//!<   resolveDots       ( );
//!<   return *this;
//!< }

PPath PPath::operator/(const PPath& other) const
{
  return ( PPath(*this) /= other );
}

PPath& PPath::operator/=(const PPath& other)
{
  if ( !endsWith  (c_str(),       _slash_char)  &&
       !startsWith(other.c_str(), _slash_char)  )
    string::operator+=(_slash);
  string::operator+=(other);

  // The resolveSlashChars and resolveDots were called on other upon construction
  // resolveSlashChars ( );
  // resolveDots       ( );
  return *this;
}

bool PPath::operator== (const string& other) const
{
  // Or operator==( PPath::getcwd() ) ?
  if ( other == "" )
    return strcmp(c_str(), "") == 0; 
  return operator==( PPath(other) );
}

bool PPath::operator==(const PPath& other) const
{
  // If raw_equal is true, there is no need to call canonical()
  bool raw_equal = strcmp(c_str(), other.c_str());
  return raw_equal || ( strcmp(canonical().c_str(), other.canonical().c_str()) == 0 );
}

PPath PPath::up() const
{
  if ( extension() != "" )
    return dirname().up();

  // The following rfind neglects a possible 'last position' slash char
  unsigned int slash_pos = rfind(_slash_char, length()-2);
  if ( slash_pos == npos )
    PLERROR("Can't go up on this path.");

  return substr(0, slash_pos);  
}

PPath PPath::dirname() const
{
  unsigned int slash_pos = rfind(_slash_char);
  if ( slash_pos == npos )
    return "";  
  return substr(0, slash_pos);
}

PPath PPath::basename() const
{
  unsigned int slash_pos = rfind(_slash_char);
  if ( slash_pos == npos )
    return PPath(*this);  
  return substr(slash_pos+1, length());
}

string PPath::extension() const
{
  unsigned int begext = find('.');
  if ( begext == npos ||
       begext == length()-1 )
    return "";
  return substr(begext+1, length());
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
  if ( find(':') == 2 && isalpha(*this[0]) )
    return PPath(substr(0, 2));
  return PPath("");
}

bool PPath::isabs() const
{
  return drive() != "" || isHttpPath() || isFtpPath() || _protocol == FILE_PROTOCOL;
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
#endif


} // end of namespace PLearn
