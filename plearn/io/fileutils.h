// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent and Yoshua Bengio
// Copyright (C) 1999-2005 University of Montreal
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
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */

// This file contains useful functions for file manipulation
// that are used in the PLearn Library


/*! \file PLearn/plearn/io/fileutils.h */

#ifndef fileutils_INC
#define fileutils_INC


//#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "PPath.h"
#include "PStream.h"
#include <nspr/prlong.h> //< for PRUint64

namespace PLearn {
using namespace std;
  
//! Change current directory.
int chdir(const PPath& path);

//! Returns true if the given path points to an existing regular file or directory.
bool pathexists(const PPath& path);

//! Returns true if the given path is an existing directory (or a symbolic link pointing to a directory).
bool isdir(const PPath& path);

//! Returns true if the given path is an existing regular file (or a symbolic link pointing to a file).
bool isfile(const PPath& path);

//! Returns true if the given path is an existing regular file (or a symbolic link pointing to a file) and the size of the file is 0.
bool isemptyFile(const PPath& path);

//! Returns the time of last modification of file (or 0 if file does not exist).
time_t mtime(const PPath& path);

/*! Returns a list of all entries in the given directory (omitting entries "." and "..")
  If the directory cannot be opened an error is issued.
  The returned entries are not full paths.
*/
vector<string> lsdir(const PPath& dirpath);

//! Same as lsdir, except the returned entries are full paths.
vector<PPath> lsdir_fullpath(const PPath& dirpath);

/** Low-level cross-platform mkdir function, with the normal mkdir semantics. 
 * Returns false if the directory could not be created (including because it
 * already exists), and does not create intermediate directories along the way. 
 *
 * Contrast with the API of force_mkdir, which cannot be used in situations where
 * race conditions matter, because of its "return true if the directory already
 * exists" semantics.
 */
bool mkdir_lowlevel(const PPath& dirname);

/*! Forces directory creation if it does not already exist. 
  (also creates any missing directory along its path).
  Return value indicates success (true) or failure (false).
  If the directory already exists, true is returned.
*/
bool force_mkdir(const PPath& dirname);
  
//! Extracts the directory part of the filepath and calls force_mkdir.
//! Calls PLERROR in case of failure.
void force_mkdir_for_file(const PPath& filepath);

/*! Forces removal of directory and all its content.
  Return value indicates success (true) or failure (false).
  If the directory does not exist, false is returned.
*/
bool force_rmdir(const PPath& dirname);

//! Returns the length of a file, measured in bytes, 
//! as a 64bit unsigned integer type defined by NSPR
PRUint64 filesize64(const PPath& filename);

//! Returns the length of a file, measured in bytes, as a long
inline long filesize(const PPath& filename)
{ return long(filesize64(filename)); }

//! Calls system with cp -R to recursively copy source to destination.
void cp(const PPath& srcpath, const PPath& destpath);

//! Remove a file (return 'true' if removed sucessfully).
bool rm(const PPath& file, bool fail_on_error_if_exist = false);

//! Calls system mv command to move the given source file to destination.
//! It fail if file exist. Use mvforce to force the overwrite existing file.
PRStatus mv(const PPath& source, const PPath& dest, bool fail_on_error = true);

//! Same as mv, but will not prompt before overwriting.
PRStatus mvforce(const PPath& source, const PPath& dest, bool fail_on_error = true);

//! Trivial unix touch.
void touch(const PPath& file);

//! Reads while the characters read exactly match those in s.
//! Will throw a PLERROR exception as soon as it doesn't match.
void readWhileMatches(PStream& in, const string& s);

//! Skips everything until '\n' (also consumes the '\n').
void skipRestOfLine(PStream& in);

//! Will skip all blanks (white space, newline and #-style comments).
//! Next character read will be first "non-blank".
void skipBlanksAndComments(PStream& in);

//! Fills 'line' with the next non blank line (#-style comments are
//! considered blank, and automatically stripped out of 'line').
void getNextNonBlankLine(PStream& in, string& line);

//! Will return the number of non-blank lines of file.
//! #-style comments are considered blank.
int countNonBlankLinesOfFile(const PPath& filename);

//! Peeks the first char after removal of blanks.
inline int peekAfterSkipBlanks(PStream& in) {
    int c;
    do {
        c = in.get();
    } while (c != EOF && isspace(c));
    in.unget();
    return c;
}

//! Peeks the first char after removal of blanks and comments.
inline int peekAfterSkipBlanksAndComments(PStream& in)
{ skipBlanksAndComments(in); return in.peek(); }

//! Gets the first char after removal of blanks.
inline int getAfterSkipBlanks(PStream& in) {
    int c;
    do {
        c = in.get();
    } while (c != EOF && isspace(c));
    return c;
}

//! Gets the first char after removal of blanks and comments.
inline int getAfterSkipBlanksAndComments(PStream& in)
{ skipBlanksAndComments(in); return in.get(); }

//! Returns a temporary file (or directory) name suitable
//! for a unique (one time) use. If provided, 'prefix' will
//! be give the first characters of the file (or directory).
PPath newFilename(const PPath& directory = "/tmp/", const string& prefix="", bool is_directory=false);

//! Return a valid filename from a potentially invalid one.
PPath makeFileNameValid(const PPath& filename);

//! Returns the whole content of the file as a string.
string loadFileAsString(const PPath& filepath);

//! Writes the raw string into the given file.
//! Intermediate directories in filepath are created if necessary.
void saveStringInFile(const PPath& filepath, const string& text);

//! Will return the text, macro processed, with each instance of ${varname}
//! in the text that corresponds to a key in the given map replaced by its
//! associated value. 
//! Also every $DEFINE{varname=... } in the text will add a new varname
//! entry in the map (the DEFINE macro will be discarded).
//! Also every $INCLUDE{filepath} will be replaced in place by the text of
//! the file it includes and set latest, to max(latest,mtime(filepath))
string readAndMacroProcess(PStream& in, map<string, string>& variables, 
                           time_t& latest, bool skip_comments= true);

/*! Given a filename, generates the standard PLearn variables FILEPATH,
  DIRPATH, FILENAME, FILEBASE, FILEEXT, DATE, TIME and DATETIME and
  adds them to the map of variables passed as an argument.
*/
void addFileAndDateVariables(const PPath& filepath,
                             map<string, string>& variables,
                             const time_t& latest);
    
/*! Same as readAndMacroProcess, but takes a filename instead of a string.
  The following variables are automatically set from the filepath: FILEPATH DIRPATH FILENAME FILEBASE FILEEXT 
  Ex: if the absolute path to filepath is /home/me/foo.plearn
  Then we'll get:
  FILEPATH = "/home/me/foo.plearn"
  DIRPATH  = "/home/me"
  FILENAME = "foo.plearn"
  FILEBASE = "foo"
  FILEEXT  = ".plearn"
  Variables for the date and time (DATE, TIME, DATETIME) are also defined.

  If 'change_dir' is set to true, the program will move to the directory
  containing 'filepath' before reading the file, and will move back to the
  current directory before exiting the function.

  The path 'filepath' may contain local variables in the form:
  dir/file::var1=x1::var2=x2 ...
  These variables will be used when parsing the file, but will not be saved
  in the 'variables' map.
*/
string readFileAndMacroProcess(const PPath& filepath,
                               map<string, string>& variables,
                               time_t& latest, bool change_dir = false);

inline string readFileAndMacroProcess(const PPath& filepath,
                               map<string, string>& variables,
                               bool change_dir = false)
{
    time_t latest = 0;
    return readFileAndMacroProcess(filepath, variables, latest, change_dir);
}

inline string readFileAndMacroProcess(const PPath& filepath, time_t& latest)
{
    map<string, string> variables;
    return readFileAndMacroProcess(filepath, variables, latest);
}

inline string readFileAndMacroProcess(const PPath& filepath)
{
    map<string, string> variables;
    time_t latest = 0;
    return readFileAndMacroProcess(filepath, variables, latest);
}

//! Increase by one the number of references to a file.
//! This references system is used in order to track which files are currently
//! being used, e.g. to be able to delete a temporary file when it is not used
//! anymore (see TemporaryFileVMatrix and TemporaryDiskVMatrix).
//! Note that in this function (and related functions below), an empty PPath
//! will be ignored (and considered to have no reference to it).
void addReferenceToFile(const PPath& file);

//! Decrease by one the number of references to a file.
void removeReferenceToFile(const PPath& file);

//! Return the number of references to a file.
unsigned int nReferencesToFile(const PPath& file);

//! Return 'true' iff there is no reference to a file.
bool noReferenceToFile(const PPath& file);
  
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
