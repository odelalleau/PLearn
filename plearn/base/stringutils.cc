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

#include "stringutils.h"
#include "general.h"

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
    string::size_type j;
    for(i=0; i<s.length(); i++)
        if(s[i]!=' ' && s[i]!='\t' && s[i]!='\n' && s[i]!='\r')
            break;
  
    if(i==s.length())
        return string("");
    else
        start = i;

    for(j=s.length(); j>=1; j--)
        if(s[j-1]!=' ' && s[j-1]!='\t' && s[j-1]!='\n' && s[j-1]!='\r')
            break;
    end = size_t(j - 1);
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
    string::size_type pos = s.length();
    while(pos>=1 && (s[pos - 1]=='\r' || s[pos - 1]=='\n'))
        pos--;
    return s.substr(0,pos);
}

string removequotes(const string& s)
{
    const int n = int(s.size());
    if (n >= 2) {
        if ((s[0] == '"'  && s[n-1] == '"') ||
            (s[0] == '\'' && s[n-1] == '\''))
            return s.substr(1,n-2);
    }
    return s;
}

string quote_string(const string& s)
{
    string quoted(s);
        
    // Escape the existing quotes
    string::size_type pos = quoted.find("\"");
    while ( pos != quoted.npos )
    {
        quoted.insert(pos, "\\");
        pos = quoted.find("\"", pos+2); // +2 since the inserted char...
    }

    // Quote the string
    quoted.insert(0, "\"");
    quoted.insert(quoted.size(), "\"");
    return quoted;
}

string remove_trailing_slash(const string& s)
{
    string::size_type pos = s.length();
    while(pos >= 1 && s[pos - 1]==slash_char)
        pos--;
    return s.substr(0,pos);
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
    string::size_type l = s.length();
    for(unsigned int i=0; i<l; i++)
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
    string::size_type l = s.length();
    bool in_comment=false;
    for(unsigned int i=0; i<l; i++)
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
    string::size_type l = s.length();
    unsigned int beg = 0;
    unsigned int end = 0;
  
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

// One version where we allow to quote a delimiter so that it is not considered as a delimiter.
// TODO: optimize...
// the double_quote are only considered if at the boundary of deliminer.
// should execute in O(n+k) where n is the number of character in s and k is the number of field in k.
vector<string> split_quoted_delimiter(const string& s, char delimiter, string double_quote){
    if(double_quote.length()==1)
        PLASSERT(delimiter!=double_quote[0]);
    vector<string> ret = split(s, delimiter);
    vector<string> ret2;
    int delim_size=double_quote.size();
    for(uint i=0; i<ret.size();i++){
        bool bw=string_begins_with(ret[i],double_quote);
        bool ew=string_ends_with(ret[i],double_quote);
        if(bw && ew){
            ret2.push_back(ret[i].substr(delim_size,
                                         ret[i].size()-delim_size)); 
        }else if(bw){
            string tmp=ret[i].substr(delim_size);
            tmp+=delimiter;
            for(uint j=i+1;j<ret.size();j++){
                if(string_ends_with(ret[j],double_quote)){
                    tmp+=ret[j].substr(0,ret[j].size()-delim_size);
                    ret2.push_back(tmp);
                    i=j;
                    break;
                }
                tmp+=ret[j];
                tmp+=delimiter;
            }
        }else
            ret2.push_back(ret[i]);
    }
    return ret2;
    
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
        if (v[i]==element) return int(i);
    return -1;
}


int universal_compare(const string& x, const string& y)
{
    // Try numerical comparison
    double x_double = todouble(x);
    double y_double = todouble(y);
    if (! (is_missing(x_double) || is_missing(y_double))) {
        if (is_equal(x_double, y_double))
            return 0;
        else
            return int(x_double - y_double);
    }

    // Fall back to string comparison
    return x.compare(y);
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
    vector<string>::size_type n = command_line.size();
    for (unsigned int i=0;i<n;i++)
        if (command_line[i]==option && i+1<n) return command_line[i+1];
    return default_value;
}

bool find(const vector<string> &command_line, const string& option)
{
    vector<string>::size_type n = command_line.size();
    for (unsigned int i=0;i<n;i++)
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

///////////////////////
// split_from_string //
///////////////////////
vector<string> split_from_string(const string& s, const string& delimiter)
{
    vector<string> res;
    string::size_type pos_of_delim;
    string::size_type pos_of_start = 0;
    string::size_type size_of_delim = delimiter.size();
    do {
        pos_of_delim = s.find(delimiter, pos_of_start);
        if (pos_of_delim == string::npos)
            res.push_back(s.substr(pos_of_start));
        else {
            res.push_back(s.substr(pos_of_start, pos_of_delim - pos_of_start));
            pos_of_start = pos_of_delim + size_of_delim;
        }
    } while (pos_of_delim != string::npos);
    return res;
}

////////////////////////////
// parseBaseAndParameters //
////////////////////////////
void parseBaseAndParameters(const string& s, string& base,
                            map<string, string>& params,
                            map<string, string>* added,
                            map<string, string>* backup,
                            const string& delimiter)
{
    vector<string> splits = split_from_string(s, delimiter);
    base = splits[0];
    string name, value;
    map<string, string>::const_iterator it;
    if (backup)
        backup->clear();
    if (added)
        added->clear();
    for (vector<string>::size_type i = 1; i < splits.size(); i++) {
        const string& str = splits[i];
        if (str.find('=') == string::npos)
            PLERROR("In parseBaseAndParameters - Could not find the '=' character when parsing "
                    "parameter '%s'", str.c_str());
        split_on_first(str, "=", name, value);
        if (name.empty())
            PLERROR("In parseBaseAndParameters - Read an empty parameter name in string '%s'"
                    , s.c_str());
        if (backup && (it = params.find(name)) != params.end())
            (*backup)[name] = it->second;
        params[name] = value;
        if (added)
            (*added)[name] = value;
    }
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
