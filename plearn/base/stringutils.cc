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
   * $Id: stringutils.cc,v 1.24 2005/01/18 20:28:59 chrish42 Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

#include "stringutils.h"
#include "general.h"

#if USING_MPI
#include <plearn/sys/PLMPI.h>
#endif //USING_MPI

namespace PLearn {
using namespace std;


string left(const string& s, size_t width, char padding)
  { 
    if(s.length()>width)
      return s;
    else
      return s+string(width-s.length(),padding);
  }

string right(const string& s, size_t width, char padding)
  {
    if(s.length()>width)
      return s;
    else
      return string(width-s.length(),padding)+s;
  }

string center(const string& s, size_t width, char padding)
  {
    if(s.length()>width)
      return s;
    else
      return string((width-s.length())/2,padding) + s +
	  string((width-s.length()+1)/2,padding);
  }

// this function handle numbers with exponents (such as 10.2E09)
// as well as Nans. String can have trailing whitespaces on both sides
bool pl_isnumber(const string& str, double* dbl)
{
  double d;
  string s=removeblanks(str);
  char* l;
  d = strtod(s.c_str(),&l);
  if(s=="")d=MISSING_VALUE;
  if(dbl!=NULL)*dbl=d;
  return ((unsigned char)(l-s.c_str())==s.length());
}

// norman: there is no strtof in .NET
#ifndef WIN32
bool pl_isnumber(const string& str, float* dbl) {
  float d;
  string s=removeblanks(str);
  char* l;
  d = strtof(s.c_str(),&l);
  if(s=="")d=MISSING_VALUE;
  if(dbl!=NULL)*dbl=d;
  return ((unsigned char)(l-s.c_str())==s.length());
}
#endif // WIN32

long tolong(const string& s, int base)
  {
    const char* nptr = s.c_str();
    char* endptr;
    long result = strtol(nptr,&endptr,base);
    if(endptr==nptr) { // no character to be read
      string err = string("in toint string is not an int: ") + s;
      PLERROR(err.c_str());
    }
    return result;
  }
 
bool tobool(const string& s)
{
  if (s=="true" || s=="1") return true;
  if (s=="false" || s=="0") return false;
  PLERROR("tobool: can't convert string %s into a boolean",s.c_str());
  return false;
}

double todouble(const string& s)
  {
    const char* nptr = s.c_str();
    char* endptr;
    double result = strtod(nptr,&endptr);
    if(endptr==nptr) // no character to be read
      result = MISSING_VALUE;
    return result;
  }

  string extract_filename(const string& filepath)
  {
    size_t p = filepath.rfind(slash);
    if (p != string::npos)
      return filepath.substr(p+1,filepath.length()-(p+1));
    else
      return filepath;
  }

  string extract_directory(const string& filepath)
  {
    size_t p = filepath.rfind(slash);
    if (p != string::npos)
      return filepath.substr(0,p+1);
    else
    {
      string dot = ".";
      return dot+slash;
    }
  }
  
  string extract_extension(const string& filepath)
  {
    string filename = extract_filename(filepath);
    size_t p = filename.rfind(".");
    if (p != string::npos)
      return filename.substr(p,filename.length()-p);
    else
      return "";
  }

  string extract_filename_without_extension(const string& filepath)
  {
    string filename = extract_filename(filepath);
    size_t p = filename.rfind(".");
    if (p != string::npos)
      return filename.substr(0,p);
    else
      return filename;
  }

  string remove_extension(const string& filename)
  {
    size_t p = filename.rfind(".");
    if (p != string::npos)
      return filename.substr(0,p);
    else
      return filename;
  }

string* data_filename_2_filenames(const string& filename, int& nb_files)
{
  ifstream in(filename.c_str());
  if (!in)
    PLERROR("In data_filename_2_filenames: couldn't open file %s",
      filename.c_str());
 
  const int buffersize = 100;
  string* filenames = new string[buffersize];
  nb_files = 0;
  string fname;
  while (getline(in, fname, '\n'))
    filenames[nb_files++] = fname;
 
  return filenames;
}

string removeblanks(const string& s)
{
  size_t start=0;
  size_t end=0;
  size_t i;
  int j;
  for(i=0; i<s.length(); i++)
    if(s[i]!=' ' && s[i]!='\t' && s[i]!='\n' && s[i]!='\r')
      break;
  
  if(i==s.length())
    return string("");
  else
    start = i;

  for(j=s.length()-1; j>=0; j--)
    if(s[j]!=' ' && s[j]!='\t' && s[j]!='\n' && s[j]!='\r')
      break;
  end = size_t(j);
  return s.substr(start,end-start+1);
}

string removeallblanks(const string& s)
{
  string res;
  size_t l = s.length();
  for(size_t i=0; i<l; i++)
    {
      char c = s[i];
      if(c!=' ' && c!='\t' && c!='\n' && c!='\r')
        res += c;
    }
  return res;
}

string removenewline(const string& s)
{
  int pos = s.length()-1;
  while(pos>=0 && (s[pos]=='\r' || s[pos]=='\n'))
    pos--;
  return s.substr(0,pos+1);
}

string remove_trailing_slash(const string& s)
{
  int pos = s.length()-1;
  while(pos >= 0 && s[pos]==slash_char)
    pos--;
  return s.substr(0,pos+1);
}

string append_slash(const string& path)
{
  size_t l = path.length();
  if(l>0 && path[l-1]!=slash_char)
    return path+slash;
  else
    return path;
}

string lowerstring(const string& ss)
{
    string s(ss);
    string::iterator it = s.begin(), end = s.end();

    // for some reason toupper and tolower from ctype.h seem to create
    // problems when compiling in optimized mode, so we do this
    for (; it != end; ++it)
    {
      if(*it>='A' && *it<='Z')
        *it += 'a'-'A';
    }
    return s;
}

string upperstring(const string& ss)
{
    string s(ss);
    string::iterator it = s.begin(), end = s.end();

    // for some reason toupper and tolower from ctype.h seem to create
    // problems when compiling in optimized mode, so we do this
    for (; it != end; ++it)
    {
      if(*it>='a' && *it<='z')
        *it -= 'a'-'A';
    }
    return s;
}

string pgetline(istream& in)
{
  string line;
  getline(in,line);
  return removenewline(line);
}

bool isBlank(const string& s)
{
  int l = (int)s.length();
  for(int i=0; i<l; i++)
    {
      char c = s[i];
      if(c=='#' || c=='\n' || c=='\r')
        return true;
      else if(c!=' ' && c!='\t')
        return false;
    }
  return true; // empty line
}


bool isParagraphBlank(const string& s)
{
  int l = (int)s.length();
  bool in_comment=false;
  for(int i=0; i<l; i++)
    {
      char c = s[i];
      if(c=='#')
        in_comment=true;
      else if(c=='\n' || c=='\r')
        in_comment=false;
      else if(c!=' ' && c!='\t' && !in_comment)
        return false;
    }
  return true; // empty line
}

string space_to_underscore(string str)
{
  for(size_t i=0; i<str.size(); i++)
    {
      if(str[i]<=' ')
        str[i] = '_';
    }
  return str;
}

string underscore_to_space(string str)
{
  for(size_t i=0; i<str.size(); i++)
    {
      if(str[i]=='_')
        str[i] = ' ';
    }
  return str;
}

string backslash_to_slash(string str)
{
  for(size_t i=0; i<str.size(); i++)
    {
      if(str[i]=='\\')
        str[i] = '/';
    }
  return str;
}


int search_replace(string& text, const string& searchstr, const string& replacestr)
{
  int n = 0;
  size_t startpos = text.find(searchstr, 0);
  while(startpos!=string::npos)
    {
      text.replace(startpos, searchstr.length(), replacestr);
      ++n;
      startpos = text.find(searchstr, startpos+replacestr.length());
    }
  return n;
}


vector<string> split(const string& s, char delimiter)
{
  vector<string> res;
  int l = (int)s.length();
  int beg = 0;
  int end = 0;
  
  while(end<=l)
    {
      while(end<l && s[end]!=delimiter)
        ++end;
      res.push_back(s.substr(beg,end-beg));
      ++end;
      beg = end;
    }

  return res;
}

vector<string> split(const string& s, const string& delimiters, bool keep_delimiters)
{
  vector<string> result;

  size_t startpos = 0;
  size_t endpos = 0;

  for(;;)
    {
      startpos = endpos;
      endpos = s.find_first_not_of(delimiters,startpos);
      if(endpos==string::npos)
        {
          if(keep_delimiters)
            result.push_back(s.substr(startpos));
          break;
        }
      if(keep_delimiters && endpos>startpos)
        result.push_back(s.substr(startpos,endpos-startpos));

      startpos = endpos;
      endpos = s.find_first_of(delimiters,startpos);
      if(endpos==string::npos)
        {
          result.push_back(s.substr(startpos));
          break;
        }
      result.push_back(s.substr(startpos,endpos-startpos));
    }

  return result;
}


void split_on_first(const string& s,
                    const string& delimiters, string& left, string& right)
{
  size_t pos = s.find_first_of(delimiters);
  if (pos != string::npos)
    {
      left = s.substr(0,pos);
      right = s.substr(pos+1);
    }
  else
    {
      left = s;
      right = "";
    }
}

pair<string,string> split_on_first(const string& s,
                                   const string& delimiters)
{
  string left, right;
  split_on_first(s, delimiters, left, right);
  return make_pair(left,right);
}


void remove_comments(string& text, const string& commentstart)
{
  size_t startpos=0;
  size_t endpos=0;
  while(endpos!=string::npos)
    {
      startpos = text.find(commentstart,startpos);
      if(startpos==string::npos)
        break;
      endpos = text.find_first_of("\n\r",startpos);
      text.erase(startpos, endpos-startpos);
    }
}


string join(const vector<string>& s, const string& separator)
{
  string result;
  vector<string>::const_iterator it = s.begin();
  if(it!=s.end())
    {
      for(;;)
        {
          result += *it;
          ++it;
          if(it==s.end())
            break;
          result += separator;
        }
    }
  return result;
}

vector<string> remove(const vector<string> &v, string element)
{
  vector<string> res;
  for (size_t i=0;i<v.size();i++)
    if (v[i]!=element) res.push_back(v[i]);
  return res;
}

int findpos(const vector<string> &v, string element)
{
  for (size_t i=0;i<v.size();i++)
    if (v[i]==element) return (int)i;
  return -1;
}

vector<string> addprepostfix(const string& prefix, const vector<string>& names, const string& postfix)
{
  vector<string> newnames(names.size());
  vector<string>::const_iterator it = names.begin();
  vector<string>::iterator newit = newnames.begin();
  while(it!=names.end())
    {
      *newit = prefix + *it + postfix;
      ++it;
      ++newit;
    }
  return newnames;
}

string addprepostfix(const string& prefix, const string& text, const string& postfix)
{
  size_t startpos = 0;
  size_t endpos = 0;
  string txt = removenewline(text);
  string res;
  while(endpos!=string::npos)
    {
      endpos = txt.find_first_of("\n",startpos);
      if(endpos!=string::npos)
        res += prefix + txt.substr(startpos, endpos-startpos) + postfix + "\n";
      else
        res += prefix + txt.substr(startpos) + postfix + "\n";
      startpos = endpos + 1;
    }
  return res;
}

vector<string> stringvector(int argc, char** argv)
{
  if(argc>0)
    {
      vector<string> result(argc);
      for(int i=0; i<argc; i++)
        result[i] = string(argv[i]);
      return result;
    }
  else
    return vector<string>();
}

string get_option(const vector<string> &command_line, 
                  const string& option, const string& default_value)
{
  int n=(int)command_line.size();
  for (int i=0;i<n;i++)
    if (command_line[i]==option && i+1<n) return command_line[i+1];
  return default_value;
}

bool find(const vector<string> &command_line, const string& option)
{
  int n=(int)command_line.size();
  for (int i=0;i<n;i++)
    if (command_line[i]==option) return true;
  return false;
}

vector<string> getNonBlankLines(const string & in)
{
  vector<string> lines;
  vector<string> nblines;

  char sep[3]={10,13,0};
  lines= split(in,sep);
  for(size_t i=0;i<lines.size();i++)
    if(!isBlank(lines[i]))
      nblines.push_back(lines[i]);
  return nblines;
}


ostream& operator<<(ostream& out, const vector<string>& vs)
{
  vector<string>::const_iterator it = vs.begin();
  if(it!=vs.end())
    {
      out << *it;
      ++it;
    }
  while(it!=vs.end())
    {
      out << ", " << *it;
      ++it;
    }
  return out;
}

string tostring(const double& x)
{
  ostringstream out;
  int ix = (int)x;
  if (ix == x)
    out << ix;
  else {
    out.precision(12);
    out << x;
  }
  return out.str();
}

string tostring(const float& x)
{
  ostringstream out;
  int ix = (int)x;
  if (ix == x)
    out << ix;
  else {
    out.precision(8);
    out << x;
  }
  return out.str();
}


} // end of namespace PLearn




