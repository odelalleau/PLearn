// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: stringutils.h,v 1.14 2003/08/13 08:13:16 plearner Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

// This file contains useful functions for string manipulation
// that are used in the PLearn Library


/*! \file PLearnLibrary/PLearnCore/stringutils.h */

#ifndef stringutils_INC
#define stringutils_INC

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include "TMat.h"

//!  to be replaced ultimately by the use of sstream, but including it currently produces a thousand warnings... [Pascal]
#include <strstream.h>

namespace PLearn <%
using namespace std;
  
  //!  converts anything to a string (same output as with cout << x)
  template<class T> string tostring(const T& x);

  //!  specialised version for char*
  inline string tostring(const char* s) { return string(s); }
  
  //!  aligns the given string in a cell having the given width
  string left(const string& s,   unsigned int width, char padding=' ');
  string right(const string& s,  unsigned int width, char padding=' ');
  string center(const string& s, unsigned int width, char padding=' ');

  // this function handle numbers with exponents (such as 10.2E09)
  // as well as Nans. String can have trailing whitespaces on both sides
  bool pl_isnumber(const string& s,double* dbl=NULL);

  //!  conversions from string to numerical types
  long tolong(const string& s, int base=10);  
  double todouble(const string& s);
  bool tobool(const string& s);
  inline int toint(const string& s, int base=10) { return int(tolong(s,base)); }
  inline float tofloat(const string& s) { return float(todouble(s)); }
#ifdef USEFLOAT
  inline float toreal(const string& s) { return tofloat(s); }
#endif //!<  FLOAT
#ifdef USEDOUBLE
  inline double toreal(const string& s) { return todouble(s); }
#endif //!<  DOUBLE
    
  //!  removes starting and ending blanks '\n','\r',' ','\t'
  string removeblanks(const string& s);

  //!  removes all blanks '\n','\r',' ','\t'
  string removeallblanks(const string& s);

  //!  removes any trailing '\n' and/or '\r'
  string removenewline(const string& s);

  //!  convert a string to all lowercase
  string lowerstring(const string& s);
  
  //!  convert a string to all uppercase
  string upperstring(const string& s);
    
  //!  returns the next line read from the stream,
  //!  after removing any trailing '\r' and/or '\n'
  string pgetline(istream& in=cin);

  //! returns true if s is a blank line (containing only space, tab, until end of line or a # comment-character is reached
  bool isBlank(const string& s);

  //! returns true if s is a blank paragraph (containing only space, tab, until end of **string**)
  bool isParagraphBlank(const string& s);


  //!  replaces all characters <= ' ' (i.e. newline, tab, space, etc...) by '_'
  string space_to_underscore(string str);

  //!  replaces all underscores by a single space character
  string underscore_to_space(string str);

  //! replaces all backslashes with slash
  string backslash_to_slash(string str);

//! replaces all occurences of searchstr in the text by replacestr
//! returns the number of matches that got replaced
int search_replace(string& text, const string& searchstr, const string& replacestr);

/*!     splits a string into a list of substrings (using any sequence of 
    the given delimiters as split point)
    if keepdelimiters is true the delimitersequences are appended to the list
    otherwise (the default) they are removed.
*/
  vector<string> split(const string& s, const string& delimiters=" \t\n\r", bool keepdelimiters=false);

/*!     Split the string on the first occurence of a delimiter and returns 
  what was left of the delimitor and what was right of it.
  If no delimitor character is found, the original string is returned 
  as left, and "" is returned in right
*/
void split_on_first(const string& s, const string& delimiters, string& left, string& right);

/*!     Split the string on the first occurence of a delimiter; return a pair with
    the two split parts.  If the splitting character is not found, the
    original string is returned in the first part of the pair, and "" is
    in the second part
*/
  pair<string,string> split_on_first(const string& s,
                                     const string& delimiters=" \t\n\r");
  
  //!  makes a single string from a vector of strings
  string join(const vector<string>& s, const string& separator=" ");
  
  //!  returns the list of names, but with a prepended prefix and an appended postfix
  vector<string> addprepostfix(const string& prefix, const vector<string>& names, const string& postfix);

  //!  returns the list of names, but with a prepended prefix
  inline vector<string> addprefix(const string& prefix, const vector<string>& names)
  { return addprepostfix(prefix, names, ""); }

  //!  returns the list of names, but with an appended postfix
  inline vector<string> addpostfix(const vector<string>& names, const string& postfix)
  { return addprepostfix("", names, postfix); }

//! Returns a string with the prefix prepended and the postfix appended 
//! to each *line* of the text string.
string addprepostfix(const string& prefix, const string& text, const string& postfix);

//! Returns a string with the prefix prepended
//! to each *line* of the text string.
inline string addprefix(const string& prefix, const string& text)
{ return addprepostfix(prefix, text, ""); }

//! Returns a string with the postfix appended 
//! to each *line* of the text string.
inline string addpostfix(const string& text, const string& postfix)
{ return addprepostfix("", text, postfix); }

//!  makes a C++ style vector of strings from a C style vectr of strings
//!  Note: this may be useful in conjunction with get_option.
vector<string> stringvector(int argc, char** argv);

/*!     The command_line is made of pairs of the form
       option value
    Look for an option in the command_line, and return
    the corresponding value if it is found, or return
    default_value otherwise. Note: this may be useful
    with command_line = stringvector(argc,argv);
*/
  string get_option(const vector<string> &command_line, 
                    const string& option, const string& default_value);

/*!     also useful to find "turn-on" options in a command line, i.e. of the form
       option
    this function just returns true if the option is found
    in the command_line. Note: this may be useful
    with command_line = stringvector(argc,argv);
*/
  bool find(const vector<string> &command_line, const string& option);

  //!  return index of element in v, or -1 if not found
  int findpos(const vector<string> &v, string element);

  //!  return vector with all instances of element removed
vector<string> remove(const vector<string> &v, string element);

  //!  ** File path manipulation functions ** 

  //!  Returns everything after the last '/' (if there's no '/' returns filepath)
  string extract_filename(const string& filepath);

  //!  Returns everything before the last '/' including the '/' (if there's no '/' it returns "./")  
  string extract_directory(const string& filepath);

  //!  Returns everything after the last '.' of the filename (i.e. excluding the directory paths
  //!  if any is present), including the '.' (if there's no '.' in the filename it returns "")
  string extract_extension(const string& filepath);

  //!  Return the filename withoug the extension (i.e. removing the last
  //'.' and everything after it
  string remove_extension(const string& filename);

  //!  removes any trailing '/' from the path
  string remove_trailing_slash(const string& path);    

  //! appends a trailing slash to path if there isn't already one
  string append_slash(const string& path);

  //!  Returns everything before the last '.' of the filename, excluding the '.' (if there's no '.' in the filename it returns the whole filename) 
  string extract_filename_without_extension(const string& filepath);

  //! In a multiline text, removes everything starting at commentstart pattern until the end of line
  void remove_comments(string& text, const string& commentstart="#");

  //! Returns a vector of string containing only non-empty lines, as you guessed it
  vector<string> getNonBlankLines(const string & in);

  //!  take a filename containing the name of a file per line, and return
  //!  theses names as a string* of length nb_files
  string* data_filename_2_filenames(const string& filename, int& nb_files);

  //!  formatted printing of vector<string> prints strings separated by a ", "
  ostream& operator<<(ostream& out, const vector<string>& vs);

  //!  ------------------------------------------------------------------


/*! ******************
    * Implementation *
    ******************
*/
    
  template<class T> string tostring(const T& x)
    {
      /* It would be nice to some day be able to use just this... when #include <sstream> works properly... 
      ostringstream out;
      out << x;
      return out.str();
      */

      ostrstream out;
      out << setprecision(8) << x;
      char* buf = out.str();
      int n = out.pcount();
      string s(buf,n);
      out.freeze(false); // return ownership to the stream, so that it may free it...
      return s;
    }



%> // end of namespace PLearn

#endif




