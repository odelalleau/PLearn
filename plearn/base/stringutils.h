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
 * $Id$
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */

// This file contains useful functions for string manipulation
// that are used in the PLearn Library


/*! \file PLearn/plearn/base/stringutils.h */

#ifndef stringutils_INC
#define stringutils_INC

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
// Removed inclusion of tostring because it causes circular includes; please
// include tostring directly in your code now (rather than assume it gets
// included through stringutils
// #include <plearn/base/tostring.h>
#include <plearn/base/lexical_cast.h>

namespace PLearn {
using namespace std;

// THIS slash CHAR HAS TO GO AWAY!!!!
#if defined(_MINGW_)
#define slash "\\"
#define slash_char '\\'
#else
#define slash "/"
#define slash_char '/'
#endif

//!  aligns the given string in a cell having the given width
string left(const string& s,   size_t width, char padding=' ');
string right(const string& s,  size_t width, char padding=' ');
string center(const string& s, size_t width, char padding=' ');
    
//!  removes starting and ending blanks '\n','\r',' ','\t'
string removeblanks(const string& s);

//!  removes all blanks '\n','\r',' ','\t'
string removeallblanks(const string& s);

//!  removes any trailing '\n' and/or '\r'
string removenewline(const string& s);

//!  remove exactly one pair of matching leading and trailing '\'' and '"';
//!  if there is none, return the string unmodified
string removequotes(const string& s);

//! Quote the provided string 's'
string quote_string(const string& s);

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

//! From a string s = "base_string::arg1=val1::arg2=val2::arg3=val3", fill
//! 'base' with 'base_string', and add to 'params' mappings argX -> valX.
//! Note that 'params' is not cleared.
//! One can use another delimiting string through the 'delimiter' argument.
//! If the optional 'added' map is given, it will be cleared, then filled
//! with the added parameters (useful is one wants to keep track of the
//! parameters that have been added to 'params').
//! If the optional 'backup' map is given, it will be cleared, then filled
//! with those parameters in 'params' which have been erased by the parameters
//! given in the string 's'.
void parseBaseAndParameters(const string& s, string& base,
                            map<string, string>& params,
                            map<string, string>* added =  0,
                            map<string, string>* backup = 0,
                            const string& delimiter = "::");

//!  replaces all characters <= ' ' (i.e. newline, tab, space, etc...) by '_'
string space_to_underscore(string str);

//!  replaces all underscores by a single space character
string underscore_to_space(string str);

//! replaces all backslashes with slash
string backslash_to_slash(string str);

//! Return true iff string 's' begins with string 'beginning'.
inline bool string_begins_with(const string& s, const string& beginning)
{
    string::size_type n = beginning.size();
    return (s.size() >= n && beginning == s.substr(0, n));
}

//! Return true iff string 's' ends with string 'end'.
inline bool string_ends_with(const string& s, const string& end)
{
    string::size_type n = end.size();
    string::size_type m = s.size();
    return (m >= n  &&  end == s.substr(m-n, n) );
}
  
//! replaces all occurences of searchstr in the text by replacestr
//! returns the number of matches that got replaced
int search_replace(string& text, const string& searchstr, const string& replacestr);

//! Split a string along occurences of the substring 'delimiter'.
vector<string> split_from_string(const string& s, const string& delimiter);

//! splits a string along occurences of the delimiters.
vector<string> split(const string& s, char delimiter);

/*!     splits a string into a list of substrings (using any sequence of 
  the given delimiters as split point)
  if keepdelimiters is true the delimitersequences are appended to the list
  otherwise (the default) they are removed.
*/
vector<string> split(const string& s, const string& delimiters=" \t\n\r", bool keepdelimiters=false);

/*! Split a string at deliminer while allowing a delimiter to be quoted so that it is not considered to be as a delimiter.
  The double_quote are only considered at the boundary of the field.
  The function should execute in O(n+k) where n is the number of character in s and k is the number of field in k.
  The delimiter should not be the same as double_quote.
  @param s the string to split
  @param delimiter the caractere that separate the fields of s.
  @double_quote a string that will surround a field if it containt delimiter caractere that should not consider generate a new field.
  @todo optimize...
*/
vector<string> split_quoted_delimiter(const string& s, char delimiter, string double_quote);

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


/**
 *  "Universal compare".  If x and y "look like" numbers (according to
 *  'todouble'), compare them as numbers: return <0 if x<y, ==0 if x==y and >0
 *  if x>y.  Equality checking for numbers is performed with
 *  pl_math.cc::is_equal function.  Otherwise, return x.compare(y) [[string
 *  comparison]]
 */
int universal_compare(const string& x, const string& y);


//!  ** File path manipulation functions ** 

/////////////////////////////////////////////////////////////////////////////////////////
// TO REMOVE
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
// END OF TO REMOVE
/////////////////////////////////////////////////////////////////////////////////////////

//! In a multiline text, removes everything starting at commentstart pattern until the end of line
void remove_comments(string& text, const string& commentstart="#");

//! Returns a vector of string containing only non-empty lines, as you guessed it
vector<string> getNonBlankLines(const string & in);

//!  take a filename containing the name of a file per line, and return
//!  theses names as a string* of length nb_files
string* data_filename_2_filenames(const string& filename, int& nb_files);

//!  formatted printing of vector<string> prints strings separated by a ", "
ostream& operator<<(ostream& out, const vector<string>& vs);

//! Formatted printing of a pair<U,V> as U:V
template <class U, class V>
ostream& operator<<(ostream& out, const pair<U,V>& p)
{
    return out << p.first << ':' << p.second;
}
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
