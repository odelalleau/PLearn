// -*- C++ -*-

// PPath.h
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
   * $Id: PPath.h,v 1.5 2005/01/28 00:24:22 dorionc Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent, Christian Dorion, Nicolas Chapados

/*! \file PPath.h */


#ifndef PPath_INC
#define PPath_INC

// Put includes here
#include <string>
#include <mozilla/nspr/prenv.h>
#include <plearn/base/TypeTraits.h>
#include <plearn/io/PStream.h>

#define  FILE_PROTOCOL    "file"
#define  HTTP_PROTOCOL    "http"
#define   FTP_PROTOCOL    "ftp"

namespace PLearn {
using namespace std;

// Forward declaration
class PStream;  
  
/*! This class is meant to manage paths homogeneously under any OS and to ease path manipulations.

This class inherits from string (www.sgi.com/tech/stl/basic_string.html),
hence PPath instances behave pretty much like strings and offer all public
string method you may like to use to inspect paths (find, replace, substr,
...). The main difference is that PPath offers methods and redefine
operators so that path manipulations are intuitive and that concerns
regarding OS differences may be forgotten. The PPath instances may also
represent http or ftp file path.

Absolute and canonic path
==========================

The two major methods are absolute() and canonical(). The first one,
absolute(), returns an absolute path that is in a suitable form for opening
under the current OS. The second one, canonical(), returns the
serialization format that we adopted for paths in PLearn. This canonical
form looks like:

  METAPROTOCOL:foo/bar

where METAPROCOL is a kind of define which value is parsed out of the PPath
config file ( found in ${PLEARN_CONFIGS}/ppath.config or in
${HOME}/.plearn/ppath.config if the PLEARN_CONFIG environment variable is
not defined). The rest of the path is relative to the expanded value of the
metaprocol. For instance,

  HOME:foo/bar

maps to /home/dorionc/foo/bar for me, while it could map to
/u/bengioy/foo/bar for Yoshua and to R:\\foo\bar for some Windows
user. Note that the canonical form of a path ALWAYS USES slash chars ('/')
while the absolute() representation uses the appropriate of slash ('/') or
backslash ('\'). Hence, you should never care for windows' ugly '\' and always use
slash char '/' (this pretty much deprecates the usage of stringutils' slash
slash_char global variables):

  // Under DOS the following is true.
  PPath("C:/foo/bar") == "C:\\foo\bar" 

Also note that . and .. directories are elegantly resolved by PPath so that
/home/dorionc/./foo/bar and /home/dorionc/foo/bar resolve to the same
representation (HOME:foo/bar).

  Examples:
      // All the following asserts are true.
      PPath('./foo/bar')                                     == 'foo/bar'                             

      PPath('foo/./bar')                                     == 'foo/bar'                             

      PPath('foo/../bar')                                    == 'bar'                                

      PPath('./foo/bar/../bar/../../foo/./bar')              == 'foo/bar'      

      PPath('././foo/bar') / PPath('../bar/../../foo/./bar') == 'foo/bar'

Finally, relative paths like "foo/bar" are considered to be relative to the
current working directory of the process.

What more?
===========

Among other useful methods defined by PPath, the operator/(const PPath&) is
probably the most useful. It's usage will get your code rid of the
stringutils' inelegant 'append_slash'

  Examples:  
      // All the following asserts are true.
      PPath('')                      == ''                                             
      
      PPath('foo/bar') / ''          == 'foo/bar/'                         
      
      PPath('foo/bar') / 'file.cc'   == 'foo/bar/file.cc'           
      
      PPath('foo/bar/') / 'file.cc'  == 'foo/bar/file.cc'          

      // And note that given those...
      PPath foo = "foo", bar = "bar/";

      // ... both the following prints "foo/bar/toto".
      // One must admit the second looks nicer ;)
      cout << append_slash( append_slash( append_slash(foo)+bar ) ) + "toto" << endl;
      cout << foo / bar / "toto" << endl;
      
      
      
Finally, note that PPath also provide some useful static methods like
PPath::home, PPath::getenv (with a default value!) and PPath::getcwd.
*/
class PPath: public string
{
public:
  static PPath home  ();
  static PPath getcwd();
  static PPath getenv(const string& var, const PPath& default_="")  ;

protected:

  /***********************
   *  protected methods  *
   **********************/

  //! Builds the static metaprotocol_to_metapath map once.
  static void ensureMappings();

  /*!  Within PPath(const string& path_) ctor, the path_ argument may
    contain environment variables that must be expanded prior to any other
    processing. This method MUST NOT return a path since it would lead to
    an infinite loop of ctors.
  */
  static string expandEnvVariables(const string& path);

  void resolveSlashChars   ( );
  
  void resolveDots         ( );
  
  /*!
    Used in operator= (and therefore in the ctor) to transform
    recognized metaprotocol contained in the path, if any.
  */
  void expandMetaprotocols ( );

  /*!
    Used in operator= (and therefore in the ctor) to assign the _protocol
    member.
  */
  void parseProtocol       ( );

  /***********************
   *  protected members  *
   **********************/

  /*!
    Even if the default protocol is considered to be the file protocol,
    the _protocol member keeps the exact protocol value in the
    string. The protocol() method, however, returns FILE_PROTOCOL if the
    protocol was not specified.
  */
  string _protocol;

  /*! OS dependant list of forbidden chars.
   *
   * POSIX SETTINGS: "\\"
   *  Even if the posix standard allows backslashes in file paths,
   *  PLearn users should never use those since PLearn aims at full and
   *  easy portability.
   *
   *  Other chars may be forbidden soon.
   *
   * DOS SETTINGS: ""
   *  None for now; coming soon.
   */
  static  string              forbidden_chars;
  static  map<string, PPath>  metaprotocol_to_metapath;

public:
  PPath(const string &path_="");

  // Shorthand
  PPath(const char   path[]) { operator=( PPath(string(path)) ); }

  //! Returns an absolute path in the form appropriate for the OS
  PPath absolute() const;   

  //! Returns a PPath in the canonical form
  PPath canonical() const;
  
  /*!
    Even if the default protocol is considered to be the file protocol,
    the _protocol member keeps the exact protocol value in the
    string. The protocol() method, however, returns FILE_PROTOCOL if the
    protocol was not specified.
  */
  string protocol      ()  const { return _protocol   == "" ? FILE_PROTOCOL : _protocol;   }
  PPath  addProtocol   ()  const ; 
  PPath  removeProtocol()  const ; 
  bool   isAbsPath     ()  const { return isabs();   } 
  bool   isFilePath    ()  const { return  protocol() == FILE_PROTOCOL; }
  bool   isHttpPath    ()  const { return _protocol   == HTTP_PROTOCOL; }
  bool   isFtpPath     ()  const { return _protocol   ==  FTP_PROTOCOL; }  


  PPath  operator+  (const char*    other) const { return operator+ (PPath(other)); }
  PPath& operator+= (const char*    other)       { return operator+=(PPath(other)); }
  PPath  operator+  (const string&  other) const { return operator+ (PPath(other)); }
  PPath& operator+= (const string&  other)       { return operator+=(PPath(other)); }
  PPath  operator+  (const PPath&   other) const;
  PPath& operator+= (const PPath&   other);  
  
  static string _slash;
  static char   _slash_char;  
  PPath  operator/  (const char*    other) const { return operator/ (PPath(other)); }
  PPath& operator/= (const char*    other)       { return operator/=(PPath(other)); }
  PPath  operator/  (const PPath&   other) const;
  PPath& operator/= (const PPath&   other);

  bool   operator== (const char*   other) const  { return operator==(string(other)); }
  bool   operator== (const string& other) const;
  bool   operator== (const PPath&  other) const;
    
  /*!
    If extension() != "", returns up() on the dirname().
    
    If this path last char is a slash, returns the subpath prior to the
    prior to last slash char.

    If this path does not end with a slash but contains one, returns the
    subpath prior to the last slash.

    In other cases, throws a PLERROR.

    PPath::up vs PPath::dirname

        PPath("foo.cc").up()                 // PLERROR
        PPath("foo.cc").dirname()            // ""

        PPath("foo/bar").up()                // "foo"
        PPath("foo/bar").dirname()           // "foo"

        PPath("foo/bar/").up()               // "foo"
        PPath("foo/bar/").dirname()          // "foo/bar"

        PPath("foo/bar/hi.cc").up()          // "foo"
        PPath("foo/bar/hi.cc").dirname()     // "foo/bar"        
  */
  PPath     up        () const;

  /*!
    Returns the directory component of a pathname    

    That is, if this path contains a slash, returns a path without the
    substring that follows the last slash of this path. Otherwise,
    returns "".

    It is always true that

        path.dirname() / path.basename() == path
    
    PPath::up vs PPath::dirname

        PPath("foo.cc").up()                 // PLERROR
        PPath("foo.cc").dirname()            // ""

        PPath("foo/bar").up()                // "foo"
        PPath("foo/bar").dirname()           // "foo"

        PPath("foo/bar/").up()               // "foo"
        PPath("foo/bar/").dirname()          // "foo/bar"

        PPath("foo/bar/hi.cc").up()          // "foo"
        PPath("foo/bar/hi.cc").dirname()     // "foo/bar"        
   */
  PPath     dirname   () const;

  /*!
    Returns the final component of a pathname.

    It is always true that

        path.dirname() / path.basename() == path
  */
  PPath     basename  () const;

  /*!
    If this path contains a dot, return the substring that comes after
    the last dot. Otherwise, it returns "".
  */
  string    extension () const;

  /*!  If this path contains a dot, return a path made from the substring
    that comes before the last dot. Otherwise, it returns a copy of this
    path.
  */
  PPath     no_extension () const;

//   /*! Migrated from fileutils.{h,cc}
// 
//   Returns a ppath shorter than 256 character and exempt of any of the
//   following chars: "*?'\"${}[]@ ,()"  --- replaced by underscores. 
//    */
//   PPath     makeFileNameValid(const PPath& path) const;

  
//!<   bool startsWith(const char&   c) const;
//!<   bool endsWith  (const char&   c) const;  
//!<   bool startsWith(const string& s) const;
//!<   bool endsWith  (const string& s) const;


  /*************************************************************************
   * Dos/Posix dependent methods
   ************************************************************************/
  
  
  /**
     Returns the drive specification of a path (a drive letter followed by a
     colon) under dos. Under posix, always returns "".
  */
  PPath drive() const;

  /**
     Returns whether a path is absolute.

     Trivial in Posix, harder on the Mac or MS-DOS.  For DOS it is absolute
     if it starts with a slash or backslash (current volume), or if a
     pathname after the volume letter and colon starts with a slash or
     backslash.
  */
  bool isabs() const;  
};

DECLARE_TYPE_TRAITS(PPath);
  
PStream& operator<<(PStream& out, const PPath& path);
PStream& operator>>(PStream& in, PPath& path);
  
} // end of namespace PLearn

#endif
