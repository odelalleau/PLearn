// -*- C++ -*-

// PPath.h
//
// Copyright (C) 2005 Pascal Vincent 
// Copyright (C) 2005 Olivier Delalleau
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

// Authors: Pascal Vincent, Christian Dorion, Nicolas Chapados

/*! \file PPath.h */


#ifndef PPath_INC
#define PPath_INC

// Put includes here
#include <map>
#include <string>
#include <plearn/base/TypeTraits.h>   //!< For DECLARE_TYPE_TRAITS macro.

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

where METAPROTOCOL is a kind of define whose value is parsed out of the PPath
config file (found in ${PLEARN_CONFIGS}/ppath.config or in
${HOME}/.plearn/ppath.config if the PLEARN_CONFIG environment variable is
not defined). The rest of the path is relative to the expanded value of the
metaprocol. For instance,

HOME:foo/bar

maps to /home/dorionc/foo/bar for me, while it could map to
/u/bengioy/foo/bar for Yoshua and to R:\foo\bar for some Windows
user. Note that the canonical form of a path ALWAYS USES slash chars ('/')
while the absolute() representation uses the appropriate slash ('/') or
backslash ('\'). Hence, you should never care for windows' ugly '\' and always use
slash char '/' (this pretty much deprecates the usage of stringutils' slash
slash_char global variables):

// Under DOS the following is true.
PPath("C:/foo/bar") == "C:\foo\bar"

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

Note that here the '==' operator returns true iff the two paths point to
the same ressource, in particular the final slash is ignored (except for a
root directory).
Finally, relative paths like "foo/bar" are considered to be relative to the
current working directory of the process.

Protocols
=========

In the examples above, paths are assumed to target a file (or directory) on disk.
This can be made explicit by adding the protocol "file:", e.g. "file:/foo/bar".
The protocols available are:
file:  <- on disk
ftp:   <- on a ftp server
http:  <- on a http server
If a protocol is used, the path MUST be absolute.
Also, a protocol cannot be used with a metaprotocol: "file:HOME:foo" is invalid,
but you could define HOME to be "file:/home/login" and use "HOME:foo".

What more?
===========

Among other useful methods defined by PPath, the operator/(const PPath&) is
probably the most useful. Its usage will get your code rid of the stringutils'
inelegant 'append_slash'.

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

Moreover, a PPath can be split into two parts: its 'dirname' and its 'basename'
(accessible respectively through the dirname() and basename() methods).
The semantic of this distinction is that a PPath points to a ressource,
'basename', which can be found in the 'dirname' directory.
Thus 'basename' is the part of the PPath following the last '/' (or its
platform-dependent counterpart). For instance "foo/bar/file" has a 'dirname'
part equal to "foo/bar" and a 'basename' part equal to "file".
See help on these methods for more details and examples.
      
Finally, note that PPath also provide some useful static methods like
PPath::home, PPath::getenv (with a default value!) and PPath::getcwd.
*/

class PPath: public string
{
public:

    static PPath home  ();
    static PPath getcwd();
    static PPath getenv(const string& var, const PPath& default_="")  ;
    
    /*!
     *  Add a new metaprotocol-to-metapath binding.
     *  Return 'true' iff the given metaprotocol was not already
     *  binded.
     *  If 'force' is set to true, the binding will be made even
     *  if the given metaprotocol was already binded. Otherwise,
     *  the existing metaprotocol will be preserved.
     */
    static bool addMetaprotocolBinding(const string& metaprotocol,
                                       const PPath& metapath,
                                       bool  force = true);

    /*!
     *  Decide whether or not to display canonical paths in error
     *  messages. The default behavior is to display absolute
     *  paths, but canonical paths might sometimes be preferred
     *  (e.g. in testing, for cross-platform compatibility, or for
     *  debug purpose).
     */
    static void setCanonicalInErrors(bool canonical);

protected:

    /***********************
     *  protected methods  *
     **********************/

    /*! OS dependent list of forbidden chars.
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
    static string forbidden_chars();

    /*!
      Builds a static metaprotocol-to-metapath map once and returns it at
      each call.
    */
    static  const map<string, PPath>&  metaprotocolToMetapath();

    /*!
      Within PPath(const string& path_) ctor, the path_ argument may
      contain environment variables that must be expanded prior to any other
      processing. This method MUST NOT return a path since it would lead to
      an infinite loop of ctors.
    */
    static string expandEnvVariables(const string& path);

    /*!
      Replace any slash character (canonical '/' or '_slash_char') by the
      '_slash_char' character, removing duplicated slashes.
    */
    void resolveSlashChars   ( );

    //! Remove extra dots from the path.
    void resolveDots         ( );

    //! Remove only extra single dots from the path. Assume there is no explicit
    //! protocol in the path.
    void resolveSingleDots   ( );

    //! Remove only extra double dots from the path (assume there are no extra
    //! single dots, i.e. in general that resolveSingleDots() was called first).
    //! Also assume there is no explicit protocol in the path.
    void resolveDoubleDots   ( );

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

public:

    //! Constructs a PPath from a string (which can be a serialized PPath).
    PPath(const string &path_="");

    // Shorthand.
    PPath(const char* path);

    //! Returns an absolute path in the form appropriate for the OS.
    //! The returned path never ends with a slash unless it is a root directory.
    //!  - if 'add_protocol' is false, then only the FILE_PROTOCOL is allowed
    //!    and the protocol will be systematically removed,
    //!  - if 'add_protocol' is true, then all protocols are allowed and the
    //!    protocol will be systematically added.
    PPath absolute(bool add_protocol = false) const;

    //! Returns a PPath in the canonical (serialized form).
    //! It is a string because it needs to be converted to a system-dependent
    //! version before it can be used directly as a PPath.
    string canonical() const;

    //! Return the string that should be displayed in an error
    //! message. It will be either the absolute or canonical
    //! string, depending on the value of the global PPath
    //! boolean 'canonical_in_errors' (the default being to
    //! display the absolute path).
    string errorDisplay() const;

    /*!
      Even if the default protocol is considered to be the file protocol,
      the _protocol member keeps the exact protocol value in the
      string. This is to be able to quickly know whether there is a protocol
      in the actual string or not.
      The protocol() method, however, returns FILE_PROTOCOL if the
      protocol was not specified.
    */
    string protocol      ()  const { return _protocol.empty() ? FILE_PROTOCOL : _protocol;   }
    //! Return either a copy of this PPath if it has an explicit protocol,
    //! or a copy with an explicit FILE_PROTOCOL (the default) otherwise.
    PPath  addProtocol   ()  const ; 
    //! Return a copy of this PPath without its explicit protocol.
    PPath  removeProtocol()  const ; 
    //! Remove the current trailing slash if there is one and it is not a
    //! root directory.
    void removeTrailingSlash();

    //! Parse a PPath url-like parameters. For instance, if the PPath is
    //! protocol:/path?param1=value1&param2=value2&param3=value3
    //! then after calling this method, 'base_path' will be equal to
    //! protocol:/path, and the 'parameters' map will be such that
    //! parameters["paramX"] = "valueX".
    //! Note that existing mappings in 'parameters' are not removed (unless
    //! they are overwritten by the PPath parameters).
    void parseUrlParameters(PPath& base_path, map<string, string>& parameters) const;

    //! Return true iff this is an absolute path.
    bool   isAbsPath     ()  const { return isabs();   } 
    bool   isFilePath    ()  const { return  protocol() == FILE_PROTOCOL; }
    bool   isHttpPath    ()  const { return _protocol   == HTTP_PROTOCOL; }
    bool   isFtpPath     ()  const { return _protocol   ==  FTP_PROTOCOL; }  

    //! Return true iff this is an empty path, i.e. it is equal to "" after its
    //! protocol has been removed.
    bool   isEmpty       ()  const { return removeProtocol().empty(); }
    //! Return true iff this is an (absolute) root directory.
    bool   isRoot        ()  const;

    static const string& _slash();       //!< System-dependent slash string.
    static       char   _slash_char();  //!< System-dependent slash character.

    //! Path concatenation. Note there is no need for an
    //! operator/(const string& other)
    //! because a string will be automatically converted to a PPath.
    PPath  operator/  (const char*    other) const { return operator/ (PPath(other)); }
    PPath& operator/= (const char*    other)       { return operator/=(PPath(other)); }
    PPath  operator/  (const PPath&   other) const;
    PPath& operator/= (const PPath&   other);

    //! The operator '==' returns true iff the two paths represent the same file or
    //! directory. The final slash is systematically ignored. For instance:
    //!   "/foo/bar"    == "/foo/bar/"
    //!   "/foo/bar/.." == "/foo"
    //!   "bar"         == "/foo/bar" if the current working directory is "/foo"
    //! This is done by comparing this->absolute() to other.absolute().
    bool   operator== (const char*   other) const  { return operator==(string(other)); }
    bool   operator== (const string& other) const;
    bool   operator== (const PPath&  other) const;

    //! The operator '!=' is defined as the contrary of '=='.
    inline bool operator!= (const char*    other) const { return !(operator==(string(other))); }
    inline bool operator!= (const string&  other) const { return !(operator==(other));         }
    inline bool operator!= (const PPath&   other) const { return !(operator==(other));         }

    /*!
      Return a PPath pointing to the parent directory of the PPath, assuming the
      PPath represents a directory: you should never call this method on a PPath
      pointing to a file, but instead use file_path.dirname().up().
      The final '/' in the PPath is ignored.

      If there is no parent directory, throws a PLERROR.

      PPath::up examples:

      PPath("/").up()                      // PLERROR
      PPath("").up()                       // PLERROR
      PPath("/foo").up()                   // "/"
      PPath("foo").up()                    // "."
      PPath(".").up()                      // ".."
      PPath("foo/bar").up()                // "foo"
      PPath("foo/bar/").up()               // "foo"
      PPath("foo/bar/hi.cc").up()          // "foo/bar", but never do this
    */
    PPath up() const;

    /*!
      Returns a PPath that points to the directory component of the PPath.
      Contrary to the up() method, the final slash is important, and the PPath
      may point either to a file or to a directory ressource.
      The returned PPath will never end with a slash, unless it is a root directory.

      It is always true that

      path.dirname() / path.basename() == path

      PPath::dirname examples:

      PPath("").dirname()                  // ""
      PPath("foo.cc").dirname()            // "."
      PPath(".").dirname()                 // "."
      PPath("./").dirname()                // "."
      PPath("/").dirname()                 // "/"
      PPath("/foo.cc").dirname()           // "/"
      PPath("foo/bar").dirname()           // "foo"
      PPath("foo/bar/").dirname()          // "foo/bar"
      PPath("foo/bar/hi.cc").dirname()     // "foo/bar"
    */
    PPath dirname() const;

    /*!
      Returns the final component of a pathname (which will be "" if it ends
      with a slash). The basename never contains a slash.

      It is always true that

      path.dirname() / path.basename() == path

      PPath::basename examples:

      PPath("").basename()                 // ""
      PPath("foo.cc").basename()           // "foo.cc"
      PPath("/").basename()                // ""
      PPath(".").basename()                // "."
      PPath("./").basename()               // ""
      PPath("foo/bar").basename()          // "bar"
      PPath("foo/bar/").basename()         // ""
      PPath("foo/bar/hi.cc").basename()    // "hi.cc"
    */
    PPath basename  () const;

    /*!
      Return the hostname of a PPath representing an url.
      Although this method is meant to be called on a PPath with
      a HTTP or FTP protocol, it may also be used with other
      protocols.

      PPath::hostname example:

      PPath("http://foo.com/bar/hi").hostname() // "foo.com"
    */
    string hostname() const;

    /*!
      Return the extension of basename() (or an empty string if it has no
      extension). The dot may or may not be included, depending on the value
      of the 'with_dot' parameter.
    */
    string extension (bool with_dot = false) const;

    /*!
      Return a copy of the path, but without its extension (as computed by the
      extension() method).
    */
    PPath no_extension () const;


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

protected:

    /**
       Returns whether a path is absolute.

       Trivial in Posix, harder on the Mac or MS-DOS. For DOS it is absolute
       if it starts with a slash or backslash (current volume), or if a
       pathname after the volume letter and colon starts with a slash or
       backslash.

       It is protected because one should use isAbsPath().
    */
    bool isabs() const;  

private:

    //! Whether or not to display the canonical path in
    //! errorDisplay(). Default value is 'false', and it can be
    //! modified through the setCanonicalInErrors(..) function.
    static bool canonical_in_errors;

};

DECLARE_TYPE_TRAITS(PPath);
  
//! Serialization and output of a PPath.
PStream& operator<<(PStream& out, const PPath& path);
PStream& operator>>(PStream& in, PPath& path);
  
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
