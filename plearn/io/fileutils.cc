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
   * $Id: fileutils.cc,v 1.60 2005/02/01 23:41:38 plearner Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

// Win32 specific declarations
#ifdef WIN32
#include <direct.h>
#define chdir _chdir
#include <Windows.h>

#else

#include <unistd.h>

#endif // WIN32


#include <strstream>

#include "fileutils.h"
#include <plearn/base/stringutils.h>
#include <plearn/base/plerror.h>
#include <plearn/math/pl_math.h>    //!< For 'real'.
#include "PStream.h"
#include "PPath.h"

#include <plearn/base/PrUtils.h>
#include <mozilla/nspr/prio.h>
#include <mozilla/nspr/prtime.h>
#include <mozilla/nspr/prerror.h>


namespace PLearn {
using namespace std;

  string getcwd()
  {
    char buf[2000];
    // norman: added specific functions for win32
    //         (cannot do the define because getcwd is also
    //         the name of this function!)
#ifdef WIN32
    _getcwd(buf, 2000);
    // norman: replace the ugly windows backslash with the forward slash
    for (int i = 0; buf[i] != '\0' && i < 2000; ++i) {
      if (buf[i] == '\\')
        buf[i] = '/';
    }
#else
    ::getcwd(buf, 2000);
#endif
    return string(buf);
  }

  int chdir(const string& path) 
  { 
    int status = ::chdir(path.c_str()); 
    if (status!=0)
      PLERROR("Could not chdir to %s.",path.c_str());
    return status;
  }
  
  string abspath(const string& path)
  {    
    string cwd = getcwd();
    string result;
    if (isdir(path))
      {
        chdir(path);
        result = append_slash(getcwd());
        chdir(cwd);
      }
    else
      {
        string dirname = extract_directory(path);
        string filename = extract_filename(path);
        chdir(dirname);
        result = append_slash(getcwd()) + filename;
        chdir(cwd);
      }
    return result;
  }

  bool pathexists(const string& path)
  {
    PRFileInfo fi;

    if (PR_GetFileInfo(path.c_str(), &fi) != PR_SUCCESS)
      return false;
    else
      return fi.type == PR_FILE_FILE || fi.type == PR_FILE_DIRECTORY;
  }

  bool isdir(const string& path)
  {
    PRFileInfo fi;

    if (PR_GetFileInfo(path.c_str(), &fi) != PR_SUCCESS)
      return false;
    else
      return fi.type == PR_FILE_DIRECTORY;
  }

  bool isfile(const string& path)
  {
    PRFileInfo fi;

    if (PR_GetFileInfo(path.c_str(), &fi) != PR_SUCCESS)
      return false;
    else
      return fi.type == PR_FILE_FILE;
  }

  PRTime mtime(const string& path)
  {
    PRFileInfo fi;

    if (PR_GetFileInfo(path.c_str(), &fi) != PR_SUCCESS)
      return 0;
    else
      return fi.modifyTime;
  }

  // Returns a list of all entries in the given directory (omitting entries
  // "." and "..") If the directory cannot be opened an error is issued. 
  // The returned entries are not full paths.
  vector<string> lsdir(const string& dirpath)
  {
    vector<string> list;

    PRDir* d = PR_OpenDir(dirpath.c_str());
    if (!d)
        PLERROR("In lsdir: could not open directory %s",dirpath.c_str());

    PRDirEntry* dirent = PR_ReadDir(d, PR_SKIP_BOTH);
    while (dirent) {
      list.push_back(dirent->name);
      dirent = PR_ReadDir(d, PR_SKIP_BOTH);
    }

    PRErrorCode e = PR_GetError();
    if (e != PR_NO_MORE_FILES_ERROR
#if 1 // Workaround for NSPR bug
        && e != PR_FILE_NOT_FOUND_ERROR
        && e != PR_NOT_DIRECTORY_ERROR
#endif
       )
      PLERROR("In lsdir: error while listing directory: %s.",
              getPrErrorString().c_str());

    if (PR_CloseDir(d) != PR_SUCCESS)
      PLERROR("In lsdir: error while closing directory: %s.",
              getPrErrorString().c_str());

    return list;
  }

  // Same as lsdir, except dirpath is prepended to the entries' names.
  vector<string> lsdir_fullpath(const string& dirpath)
  {
    return addprefix(remove_trailing_slash(dirpath)+slash, lsdir(dirpath));
  }


  // Forces directory creation if it doesn't already exist. 
  // (also creates any missing directory along its path)
  // Return value indicates success (true) or failure (false).
  // If the directory already exists, true is returned.
  bool force_mkdir(const string& dirname)
  {
    string path = remove_trailing_slash(dirname);
    if(isdir(path))
      return true;

    string pathpart;
    for (size_t pos = 1; pos != string::npos;) {
      // keep ++pos here!
      ++pos;
      pos = path.find(slash, pos);
      if( pos != string::npos )
        pathpart = path.substr(0, pos);
      else
        pathpart = path;
      
      if(!isdir(pathpart)) {
        if (PR_MkDir(pathpart.c_str(), 0775) != PR_SUCCESS)
          return false;
      }
    }
    
    return true;
  }


  void force_mkdir_for_file(const string& filepath)
  {
    string dirpath = extract_directory(filepath);
    if (!force_mkdir(dirpath))
      PLERROR("force_mkdir(%s) failed",dirpath.c_str());
  }

  // Forces removal of directory and all its subdirectories
  // Return value indicates success (true) or failure (false)
  // If the directory does not exist, false is returned.
  bool force_rmdir(const string& dirname)
  {
    if (!isdir(dirname))
      return false;
    
    const vector<string> entries = lsdir_fullpath(dirname);
    for (vector<string>::const_iterator it = entries.begin();
         it != entries.end(); ++it) {
      if (isdir(*it)) {
        if (!force_rmdir(*it))
          return false;
      }
      else {
        if (PR_Delete(it->c_str()) != PR_SUCCESS)
          return false;
      }
    }

    return PR_RmDir(dirname.c_str()) == PR_SUCCESS;
  }

  
  long filesize(const string& filename)
  {
    PRFileInfo64 inf;

    if (PR_GetFileInfo64(filename.c_str(), &inf) != PR_SUCCESS)
      PLERROR("In filesize: error getting file info for %s: %s.",
              filename.c_str(), getPrErrorString().c_str());

    return inf.size;
  }

  
  string loadFileAsString(const string& filepath)
  {
    long l = filesize(filepath);
    char* buf = new char[l];    
    ifstream in(filepath.c_str());
    if (in.bad()) {
      delete[] buf;
      PLERROR("Cannot load file %s", filepath.c_str());
    }
    in.read(buf, l);
    string text(buf, l);
    delete[] buf;
    return text;
  }

  
  void saveStringInFile(const string& filepath, const string& text)
  {
    force_mkdir_for_file(filepath);
    ofstream out(filepath.c_str());
    if(!out)
      PLERROR("Couldn't open file %s for writing", filepath.c_str());
    out << text;
  }


  void cp(const string& srcpath, const string& destpath)
  {
    string command = "\\cp -R " + srcpath + " " + destpath;
    system(command.c_str());
  }

  void rm(const string& file)
  {
#ifdef WIN32
    // For the moment works ONLY with files!!!
    if ( !DeleteFile(file.c_str()) )
    {
      DWORD errorCode = GetLastError(); 
      LPVOID lpMsgBuf;
      FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, errorCode,
        MAKELANGID(LANG_NEUTRAL,
        SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf, 0,
         NULL );

      // Comment because it works only with files..
      //PLERROR("Cannot delete file %s. %s", file.c_str(), lpMsgBuf);
      LocalFree( lpMsgBuf );
    }
#else
    string command = "\\rm -rf " + file;
    system(command.c_str());
#endif
  }

  void mv(const string& file)
  {
    string command = "\\mv " + file;
    system(command.c_str());
  }

  void mvforce(const string& file)
  {
    string command = "\\mv -f " + file;
    system(command.c_str());
  }


//! Reads while the characters read exactly match those in s
//! Will throw a PLERROR exception as soon as it doesn't match
void readWhileMatches(istream& in, const string& s){
  int i = 0;
  int c;
  c = in.get();
  int n = (int)s.length();
  while(c!=EOF)
    {
      if(s[i]!=c)
        {
          in.unget();  // match failed, unget that last character
          PLERROR("In readWhileMatches. Failure while matching %s: "
                  "at position %d expected a '%c', but read a '%c'",s.c_str(),i,s[i],c);
        }
      ++i;
      if(i==n) // passed through the whole string 
        return;
      c = in.get();
    }
  PLERROR("In readWhileMatches, met EOF while matching %s", s.c_str());
}

//! Reads everything until '\n' (also consumes the '\n')
void skipRestOfLine(istream& in)
{
  int c=in.get();
  while(c!='\n' && c!=EOF)
    c=in.get();
}

void skipBlanksAndComments(istream& in)
{
  int c = in.get();
  while(c!=EOF)
    {
      if(!isspace(c))
        {
          if(c=='#')
            skipRestOfLine(in);
          else
            break;
        }
      c = in.get();
    }
  in.unget();
}

void getNextNonBlankLine(istream& in, string& line)
{
  for(;;)
    {
      getline(in,line);
      if(in.eof())
      {
        line="";
        return;
      }
      int l = (int)line.length();
      for(int i=0; i<l; i++)
        {
          char c = line[i];
          if(!isspace(c) && c!='#')
            return;            
        }
    }
}

int countNonBlankLinesOfFile(const string& filename)
{
  ifstream in(filename.c_str());
  if(!in)
    PLERROR("Could not open file %s for reading",filename.c_str());
  int count = 0;
  int c = in.get();
  while(c!=EOF)
    {
      while(c=='\n' || c==' ' || c=='\t' || c=='\r')
        c = in.get();
      if(c!='\n' && c!='#' && c!=EOF) // We've found a non-blank, non-comment char.
        ++count;
      while(c!='\n' && c!=EOF) // read until end of line
        c = in.get();
      c = in.get();
    }
  return count;  
}

int smartReadUntilNext(istream& in, string stoppingsymbols, string& characters_read, bool ignore_brackets)
{
  PStream pin(&in);
  return pin.smartReadUntilNext(stoppingsymbols, characters_read, ignore_brackets);
}
  
//!  Makes use of mkstemp(...) to create a new file.
string newFilename(const string directory, const string prefix, bool is_directory)
{
#if defined(_MINGW_) || defined(WIN32)
    PLERROR("This call is not yet implemented for this platform");
    return "";
#else
  const string tmpdirname = remove_trailing_slash(directory);
  const int length = tmpdirname.length() + 1 + prefix.length() + 6 + 1;
  char* tmpfilename = new char[length];
  if (tmpdirname=="") //!<  save in current dir
    sprintf(tmpfilename,"%sXXXXXX",prefix.c_str());
  else
    sprintf(tmpfilename,"%s/%sXXXXXX",tmpdirname.c_str(),prefix.c_str());
  int fd = mkstemp(tmpfilename);
  if (fd == -1)
    PLERROR("In newFilename - Could not create temporary file");
  // Close the file descriptor, since we are not using it.
  close(fd);
  if (is_directory) {
      // Defeats the purpose of mktemp, but who cares?
      std::remove(tmpfilename);
      PR_MkDir(tmpfilename, 0666);
  }
  if(!tmpfilename)
    PLERROR("In newFilename : could not make a new temporary filename");
  return tmpfilename;
#endif
}

string makeFileNameValid(const string& path)
{
  string dirname  = extract_directory(path);
  string filename = extract_filename_without_extension(path);
  string ext      = extract_extension(path);
  string ret      = path;
  if(filename.length() + ext.length() > 256)
    {
      int j= 0;
      string rest= filename.substr(256-ext.length()-12);
      do
	{
	  unsigned int n= j++;
	  for(unsigned int i= 0; i < rest.length(); ++i)
	    {
	      int m=0;
	      switch(i%4)
		{
		case 3: m= 1; break;
		case 2: m= 256; break;
		case 1: m= 65536; break;
		case 0: m= 256*65536; break;
		}
	    n+= m*(unsigned char)rest[i];
	    }
	  filename.resize(256-ext.length()-12);
	  filename+= "-" + tostring(n);
	} while(pathexists(dirname + filename + ext));
      PLWARNING("makeFileNameValid: Filename '%s' changed to '%s'.", 
		path.c_str(), (dirname + filename + ext).c_str());
      ret= dirname + filename + ext;
    }
  
  // replace illegal characters
  char illegal[]="*?'\"${}[]@ ,()";
  for(int i=0;i<(signed)ret.size();i++)
    for(int j=0;j<15;j++)
      if(ret[i]==illegal[j])
        ret[i]='_';
  return ret;
}

void touch(const string& file)
{
  string command = "touch "+file;
  system(command.c_str());
} 

string makeExplicitPath(const string& filename)
{
  string fn = removeblanks(filename);
  if(fn!="" && fn[0]!=slash_char && fn[0]!='.')
  {
    string dot = ".";
    return dot+slash+fn;
  }
  return fn;
}
  

void addFileAndDateVariables(const string& filepath, map<string, string>& variables)
{
  // Define new local variables
  variables["HOME"]        = PPath::getenv("HOME");
  
  const PPath fpath        = PPath(filepath).absolute();
  variables["FILEPATH"]    = fpath;
  variables["DIRPATH"]     = fpath.dirname();

  const PPath basename     = fpath.basename();
  variables["FILENAME"]    = basename;
  variables["FILEBASE"]    = basename.no_extension();
  variables["FILEEXT"]     = fpath.extension();

  
  // dorionc: This should soon be deprecated...
  // PyPLearn should provide date(), time() and date_time() functions so
  // that these could be removed
  char* date_time_env = getenv("PLEARN_DATE_TIME");

  if ( date_time_env 
       && string(date_time_env) == "NO" )
  {
    variables["DATETIME"]  = "";
    variables["DATE"]      = "";
    variables["TIME"]      = "";    
  }
  else
  {
    // Compute DATE, TIME, and DATETIME variables
    time_t curtime = time(NULL);
    struct tm *broken_down_time = localtime(&curtime);
    const int SIZE = 100;
    char time_buffer[SIZE];
    strftime(time_buffer,SIZE,"%Y%m%d:%H%M%S",broken_down_time);
    variables["DATETIME"] = time_buffer;
    strftime(time_buffer,SIZE,"%Y%m%d",broken_down_time);
    variables["DATE"] = time_buffer;
    strftime(time_buffer,SIZE,"%H%M%S",broken_down_time);
    variables["TIME"] = time_buffer;
  }
}

string readFileAndMacroProcess(const string& filepath, map<string, string>& variables)
{
  // Save old variables (to allow recursive calls)
  const char* OldVariables[] = {
    "FILEPATH", "DIRPATH", "FILENAME", "FILEBASE", "FILEEXT", "DATE", "TIME", "DATETIME"
  };
  const int num_old = sizeof(OldVariables) / sizeof(OldVariables[0]);
  map<string,string> old_vars;
  for (int i=0; i<num_old; ++i)
    old_vars[OldVariables[i]] = variables[OldVariables[i]];

  // Add the new file and date variables
  addFileAndDateVariables(filepath, variables);

  // Perform actual parsing and macro processing...
  const PPath fpath = PPath(filepath).absolute();  
  ifstream in(fpath.c_str());
  if (!in)
    PLERROR("In readFileAndMacroProcess, could not open file %s for reading", fpath.c_str());
  string text = readAndMacroProcess(in, variables);

  // Restore previous variables
  for (int i=0; i<num_old; ++i)
    variables[OldVariables[i]] = old_vars[OldVariables[i]];

  return text;
}

string readAndMacroProcess(istream& in, map<string, string>& variables)
{
  string text; // the processed text to return
  bool inside_a_quoted_string=false; // inside a quoted string we don't skip characters following a #
  int c=EOF,last_c=EOF;
  while(in)
    {
      last_c = c;
      c = in.get();
      if (last_c!='\\' && c=='"') // we find either the beginning or end of a quoted string
        inside_a_quoted_string = !inside_a_quoted_string; // flip status

      if(!inside_a_quoted_string && c=='#')  // It's a comment: skip rest of line
        {
          while(c!=EOF && c!='\n' && c!='\r')
            c = in.get();
        }

      if(c==EOF)
        break;
      else if(c!='$')
        text += c;
      else  // We have a $ macro command
        {
          int c = in.peek();
          switch(c)
            {
            case '{':  // expand a defined variable ${varname}
              {
                string varname; // name of a variable
                in.get(); // skip '{'
                smartReadUntilNext(in, "}", varname, true);
                // Maybe there are macros to process to obtain the real name of the variable.
                istrstream varname_stream(varname.c_str());
                varname = readAndMacroProcess(varname_stream, variables);
                varname = removeblanks(varname);
                map<string, string>::iterator it = variables.find(varname);
                if(it==variables.end())
                  PLERROR("Macro variable ${%s} undefined", varname.c_str());
                istrstream varin(it->second.c_str());
                text += readAndMacroProcess(varin, variables);
              }
              break;

            case 'C': // it's a CHAR{expression}
              {
                string expr;
                readWhileMatches(in, "CHAR");
                bool syntax_ok = true;
                int c = in.get();
                if(c == '{')
                  smartReadUntilNext(in, "}", expr, true);
                else
                  syntax_ok = false;
                if (!syntax_ok)
                  PLERROR("$CHAR syntax is: $CHAR{expr}");
                istrstream expr_stream(expr.c_str());
                char ch = (char) toint(readAndMacroProcess(expr_stream, variables));
                text += ch;
              }
              break;

            case 'D':
              {
                int next = in.get();
                next = in.peek();   // Next character.
                switch(next) {

                  case 'E':   // it's a DEFINE{varname}{expr}
                    {
                      string varname; // name of a variable
                      string vardef; // definition of a variable
                      readWhileMatches(in, "EFINE{");
                      getline(in,varname, '}');
                      varname = removeblanks(varname);
                      skipBlanksAndComments(in);
                      if(in.get()!='{')
                        PLERROR("Bad syntax in .plearn DEFINE macro: correct syntax is $DEFINE{name}{definition}");
                      smartReadUntilNext(in, "}", vardef, true);
                      variables[varname] = vardef;
                    }
                    break;

                  case 'I': // it's a DIVIDE{expr1}{expr2}
                    {
                      string expr1, expr2;
                      readWhileMatches(in, "IVIDE");
                      bool syntax_ok = true;
                      int c = in.get();
                      if (syntax_ok) {
                        if(c == '{')
                          smartReadUntilNext(in, "}", expr1, true);
                        else
                          syntax_ok = false;
                      }
                      if (syntax_ok) {
                        c = in.get();
                        if(c == '{')
                          smartReadUntilNext(in, "}", expr2, true);
                        else
                          syntax_ok = false;
                      }
                      if (!syntax_ok)
                        PLERROR("$DIVIDE syntax is: $DIVIDE{expr1}{expr2}");
                      istrstream expr1_stream(expr1.c_str());
                      istrstream expr2_stream(expr2.c_str());
                      string expr1_eval = readAndMacroProcess(expr1_stream, variables);
                      string expr2_eval = readAndMacroProcess(expr2_stream, variables);
                      real e1, e2;
                      if (!pl_isnumber(expr1_eval, &e1) || !pl_isnumber(expr2_eval, &e2)) {
                        PLERROR("In $DIVIDE{expr1}{expr2}, either 'expr1' or 'expr2' is not a number");
                      }
                      text += tostring(e1 / e2);
                    }
                    break;

                }
                break;
              }

            case 'E':
              {
                int next = in.get();
                next = in.peek();   // Next character.
                switch(next) {

                  case 'C': // it's an ECHO{expr}
                    {
                      string expr;
                      readWhileMatches(in, "CHO");
                      bool syntax_ok = true;
                      int c = in.get();
                      if(c == '{')
                        smartReadUntilNext(in, "}", expr, true);
                      else
                        syntax_ok = false;
                      if (!syntax_ok)
                        PLERROR("$ECHO syntax is: $ECHO{expr}");
                      istrstream expr_stream(expr.c_str());
                      cout << readAndMacroProcess(expr_stream, variables) << endl;
                    }
                    break;

                  case 'V': // it's an EVALUATE{varname}
                    {
                      string expr;
                      readWhileMatches(in, "VALUATE");
                      bool syntax_ok = true;
                      int c = in.get();
                      if(c == '{')
                        smartReadUntilNext(in, "}", expr, true);
                      else
                        syntax_ok = false;
                      if (!syntax_ok)
                        PLERROR("$EVALUATE syntax is: $EVALUATE{varname}");
                      istrstream expr_stream(expr.c_str());
                      string varname = readAndMacroProcess(expr_stream, variables);
                      string to_evaluate = variables[varname];
                      istrstream to_evaluate_stream(to_evaluate.c_str());
                      string evaluated = readAndMacroProcess(to_evaluate_stream, variables);
                      variables[varname] = evaluated;
                    }
                    break;
                }
                break;
              }

            case 'G': // it's a GETENV{expression}
              {
                string expr;
                readWhileMatches(in, "GETENV");
                bool syntax_ok = true;
                int c = in.get();
                if(c == '{')
                  smartReadUntilNext(in, "}", expr, true);
                else
                  syntax_ok = false;
                if (!syntax_ok)
                  PLERROR("$GETENV syntax is: $GETENV{expr}");
                istrstream expr_stream(expr.c_str());
                string var_name = readAndMacroProcess(expr_stream, variables);
                char* var = getenv(var_name.c_str());
                if (!var)
                  PLERROR("In readAndMacroProcess - The environment variable %s is not defined", var_name.c_str());
                text += string(var);
              }
              break;

             case 'I':
              {
                int next = in.get();
                next = in.peek();   // Next character.
                switch(next) {

                  case 'F': // it's an IF{cond}{expr_cond_true}{expr_cond_false}
                    {
                      string cond, expr_cond_true, expr_cond_false, expr_evaluated;
                      readWhileMatches(in, "F");
                      bool syntax_ok = true;
                      int c = in.get();
                      if(c == '{')
                        smartReadUntilNext(in, "}", cond, true);
                      else
                        syntax_ok = false;
                      if (syntax_ok) {
                        c = in.get();
                        if(c == '{')
                          smartReadUntilNext(in, "}", expr_cond_true, true);
                        else
                          syntax_ok = false;
                      }
                      if (syntax_ok) {
                        c = in.get();
                        if(c == '{')
                          smartReadUntilNext(in, "}", expr_cond_false, true);
                        else
                          syntax_ok = false;
                      }
                      if (!syntax_ok)
                        PLERROR("$IF syntax is: $IF{cond}{expr_cond_true}{expr_cond_false}");

                      istrstream cond_stream(cond.c_str());
                      string evaluate_cond = readAndMacroProcess(cond_stream, variables);
                      if (evaluate_cond == "1" ) {
                        expr_evaluated = expr_cond_true;
                      } else if (evaluate_cond == "0") {
                        expr_evaluated = expr_cond_false;
                      } else {
                        PLERROR("$IF condition should be 0 or 1, but is %s", evaluate_cond.c_str());
                      }
                      istrstream expr_stream(expr_evaluated.c_str());
                      text += readAndMacroProcess(expr_stream, variables);
                    }
                    break;

                  case 'N':
                    {
                      int next = in.get();
                      next = in.peek();   // Next character.
                      switch(next) {

                        case 'C': // it's an INCLUDE{filepath}
                          {
                            string includefilepath; // path of the file in a $INCLUDE{...} directive
                            readWhileMatches(in, "CLUDE");
                            int c = in.get();
                            if(c=='<')
                              smartReadUntilNext(in, ">", includefilepath, true);
                            else if(c=='{')
                              smartReadUntilNext(in, "}", includefilepath, true);
                            else
                              PLERROR("$INCLUDE must be followed immediately by a { or <");
                            istringstream pathin(includefilepath);
                            includefilepath = readAndMacroProcess(pathin,variables);
                            includefilepath = removeblanks(includefilepath);
                            string dirname = extract_directory(includefilepath);
                            string filename = extract_filename(includefilepath);
                            string olddir = getcwd();
                            chdir(dirname);
                            text += readFileAndMacroProcess(filename, variables);
                            chdir(olddir);
                          }
                          break;

                        case 'T': // it's an INT{val}
                          {
                            string expr;
                            readWhileMatches(in, "T");
                            bool syntax_ok = true;
                            int c = in.get();
                            if(c == '{')
                              smartReadUntilNext(in, "}", expr, true);
                            else
                              syntax_ok = false;
                            if (!syntax_ok)
                              PLERROR("$INT syntax is: $INT{expr}");
                            istrstream expr_stream(expr.c_str());
                            string expr_eval = readAndMacroProcess(expr_stream, variables);
                            real e;
                            if (!pl_isnumber(expr_eval, &e)) {
                              PLERROR("In $INT{expr}, 'expr' is not a number");
                            }
                            text += tostring(int(e));
                          }
                      }
                    }
                    break;

                  case 'S':
                    {

                      int next = in.get();
                      next = in.peek();   // Next character.
                      switch(next) {

                        case 'D': // it's an ISDEFINED{expr}
                          {
                            string expr;
                            readWhileMatches(in, "DEFINED");
                            bool syntax_ok = true;
                            int c = in.get();
                            if(c == '{')
                              smartReadUntilNext(in, "}", expr, true);
                            else
                              syntax_ok = false;
                            if (!syntax_ok)
                              PLERROR("$ISDEFINED syntax is: $ISDEFINED{expr}");
                            istrstream expr_stream(expr.c_str());
                            string expr_eval = readAndMacroProcess(expr_stream, variables);
                            map<string, string>::iterator it = variables.find(expr_eval);
                            if(it==variables.end()) {
                              // The variable is not defined.
                              text += "0";
                            } else {
                              text += "1";
                            }
                          }
                          break;

                        case 'E': // it's an ISEQUAL{expr1}{expr2}
                          {
                            string expr1, expr2;
                            readWhileMatches(in, "EQUAL");
                            bool syntax_ok = true;
                            int c = in.get();
                            if(c == '{')
                              smartReadUntilNext(in, "}", expr1, true);
                            else
                              syntax_ok = false;
                            if (syntax_ok) {
                              c = in.get();
                              if(c == '{')
                                smartReadUntilNext(in, "}", expr2, true);
                              else
                                syntax_ok = false;
                            }
                            if (!syntax_ok)
                              PLERROR("$ISEQUAL syntax is: $ISEQUAL{expr1}{expr2}");
                            istrstream expr1_stream(expr1.c_str());
                            istrstream expr2_stream(expr2.c_str());
                            string expr1_eval = readAndMacroProcess(expr1_stream, variables);
                            string expr2_eval = readAndMacroProcess(expr2_stream, variables);
                            if (expr1_eval == expr2_eval) {
                              text += "1";
                            } else {
                              text += "0";
                            }
                          }
                          break;

                        case 'H': // it's an ISHIGHER{expr1}{expr2}
                          {
                            string expr1, expr2;
                            readWhileMatches(in, "HIGHER");
                            bool syntax_ok = true;
                            int c = in.get();
                            if(c == '{')
                              smartReadUntilNext(in, "}", expr1, true);
                            else
                              syntax_ok = false;
                            if (syntax_ok) {
                              c = in.get();
                              if(c == '{')
                                smartReadUntilNext(in, "}", expr2, true);
                              else
                                syntax_ok = false;
                            }
                            if (!syntax_ok)
                              PLERROR("$ISHIGHER syntax is: $ISHIGHER{expr1}{expr2}");
                            istrstream expr1_stream(expr1.c_str());
                            istrstream expr2_stream(expr2.c_str());
                            string expr1_eval = readAndMacroProcess(expr1_stream, variables);
                            string expr2_eval = readAndMacroProcess(expr2_stream, variables);
                            real e1, e2;
                            if (!pl_isnumber(expr1_eval, &e1) || !pl_isnumber(expr2_eval, &e2)) {
                              PLERROR("In $ISHIGHER{expr1}{expr2}, either 'expr1' or 'expr2' is not a number");
                            }
                            if (e1 > e2) {
                              text += "1";
                            } else {
                              text += "0";
                            }
                          }
                          break;
                      }
                    }
                    break;
                }
              }
              break;

            case 'M': // it's a MINUS{expr1}{expr2}
              {
                string expr1, expr2;
                readWhileMatches(in, "MINUS");
                bool syntax_ok = true;
                int c = in.get();
                if (syntax_ok) {
                  if(c == '{')
                    smartReadUntilNext(in, "}", expr1,true);
                  else
                    syntax_ok = false;
                }
                if (syntax_ok) {
                  c = in.get();
                  if(c == '{')
                    smartReadUntilNext(in, "}", expr2,true);
                  else
                    syntax_ok = false;
                }
                if (!syntax_ok)
                  PLERROR("$MINUS syntax is: $MINUS{expr1}{expr2}");
                istrstream expr1_stream(expr1.c_str());
                istrstream expr2_stream(expr2.c_str());
                string expr1_eval = readAndMacroProcess(expr1_stream, variables);
                string expr2_eval = readAndMacroProcess(expr2_stream, variables);
                real e1, e2;
                if (!pl_isnumber(expr1_eval, &e1) || !pl_isnumber(expr2_eval, &e2)) {
                  PLERROR("In $MINUS{expr1}{expr2}, either 'expr1' or 'expr2' is not a number");
                }
                text += tostring(e1 - e2);
              }
              break;

            case 'O': // it's an OR{expr1}{expr2}
              {
                string expr1, expr2;
                readWhileMatches(in, "OR");
                bool syntax_ok = true;
                int c = in.get();
                if (syntax_ok) {
                  if(c == '{')
                    smartReadUntilNext(in, "}", expr1,true);
                  else
                    syntax_ok = false;
                }
                if (syntax_ok) {
                  c = in.get();
                  if(c == '{')
                    smartReadUntilNext(in, "}", expr2,true);
                  else
                    syntax_ok = false;
                }
                if (!syntax_ok)
                  PLERROR("$OR syntax is: $OR{expr1}{expr2}");
                istrstream expr1_stream(expr1.c_str());
                istrstream expr2_stream(expr2.c_str());
                string expr1_eval = readAndMacroProcess(expr1_stream, variables);
                string expr2_eval = readAndMacroProcess(expr2_stream, variables);
                real e1, e2;
                if (!pl_isnumber(expr1_eval, &e1) || !pl_isnumber(expr2_eval, &e2)) {
                  PLERROR("In $OR{expr1}{expr2}, either 'expr1' or 'expr2' is not a number");
                }
                int i1 = toint(expr1_eval);
                int i2 = toint(expr2_eval);
                bool is_true = i1 || i2;
                text += tostring(is_true);
              }
              break;

            case 'P': // it's a PLUS{expr1}{expr2}
              {
                string expr1, expr2;
                readWhileMatches(in, "PLUS");
                bool syntax_ok = true;
                int c = in.get();
                if (syntax_ok) {
                  if(c == '{')
                    smartReadUntilNext(in, "}", expr1,true);
                  else
                    syntax_ok = false;
                }
                if (syntax_ok) {
                  c = in.get();
                  if(c == '{')
                    smartReadUntilNext(in, "}", expr2,true);
                  else
                    syntax_ok = false;
                }
                if (!syntax_ok)
                  PLERROR("$PLUS syntax is: $PLUS{expr1}{expr2}");
                istrstream expr1_stream(expr1.c_str());
                istrstream expr2_stream(expr2.c_str());
                string expr1_eval = readAndMacroProcess(expr1_stream, variables);
                string expr2_eval = readAndMacroProcess(expr2_stream, variables);
                real e1, e2;
                if (!pl_isnumber(expr1_eval, &e1) || !pl_isnumber(expr2_eval, &e2)) {
                  PLERROR("In $PLUS{expr1}{expr2}, either 'expr1' or 'expr2' is not a number");
                }
                text += tostring(e1 + e2);
              }
              break;

            case 'S': // it's a SWITCH{expr}{cond1}{val1}{cond2}{val2}...{valdef}
              {
                string expr, valdef;
                vector<string> comp;
                vector<string> val;
                readWhileMatches(in, "SWITCH");
                bool syntax_ok = true;
                // First read 'expr'.
                int c = in.get();
                if (syntax_ok) {
                  if(c == '{')
                    smartReadUntilNext(in, "}", expr, true);
                  else
                    syntax_ok = false;
                }
                // Read the pairs {compx}{valx}, then {valdef}
                bool done_parsing = false;
                while (syntax_ok && !done_parsing) {
                  c = getAfterSkipBlanksAndComments(in);
                  string tmp_comp, tmp_val;
                  if(c == '{')
                    smartReadUntilNext(in, "}", tmp_comp, true);
                  else
                    syntax_ok = false;
                  if (syntax_ok) {
                    c = peekAfterSkipBlanksAndComments(in);
                    if(c == '{') {
                      c = getAfterSkipBlanksAndComments(in);
                      smartReadUntilNext(in, "}", tmp_val, true);
                    }
                    else {
                      // We must have read 'valdef' just before.
                      valdef = tmp_comp;
                      done_parsing = true;
                    }
                  }
                  if (!done_parsing) {
                    comp.push_back(tmp_comp);
                    val.push_back(tmp_val);
                  }
                }
                if (!syntax_ok)
                  PLERROR("$SWITCH syntax is: $SWITCH{expr}{comp1}{val1}{comp2}{val2}...{valdef}");
                istrstream expr_stream(expr.c_str());
                string expr_eval =  readAndMacroProcess(expr_stream, variables);
                bool not_done = true;
                for (unsigned int i = 0; i < comp.size() && not_done; i++) {
                  istrstream comp_stream(comp[i].c_str());
                  string comp_eval = readAndMacroProcess(comp_stream, variables);
                  if (expr_eval == comp_eval) {
                    not_done = false;
                    istrstream val_stream(val[i].c_str());
                    text += readAndMacroProcess(val_stream, variables);
                  }
                }
                if (not_done) {
                  // Default value needed.
                  istrstream val_stream(valdef.c_str());
                  text += readAndMacroProcess(val_stream, variables);
                }
              }
              break;

            case 'T': // it's a TIMES{expr1}{expr2}
              {
                string expr1, expr2;
                readWhileMatches(in, "TIMES");
                bool syntax_ok = true;
                int c = in.get();
                if (syntax_ok) {
                  if(c == '{')
                    smartReadUntilNext(in, "}", expr1, true);
                  else
                    syntax_ok = false;
                }
                if (syntax_ok) {
                  c = in.get();
                  if(c == '{')
                    smartReadUntilNext(in, "}", expr2, true);
                  else
                    syntax_ok = false;
                }
                if (!syntax_ok)
                  PLERROR("$TIMES syntax is: $TIMES{expr1}{expr2}");
                istrstream expr1_stream(expr1.c_str());
                istrstream expr2_stream(expr2.c_str());
                string expr1_eval = readAndMacroProcess(expr1_stream, variables);
                string expr2_eval = readAndMacroProcess(expr2_stream, variables);
                real e1, e2;
                if (!pl_isnumber(expr1_eval, &e1) || !pl_isnumber(expr2_eval, &e2)) {
                  PLERROR("In $TIMES{expr1}{expr2}, either 'expr1' or 'expr2' is not a number");
                }
                text += tostring(e1 * e2);
              }
              break;

            case 'U': // it's an UNDEFINE{varname}
              {
                string expr;
                readWhileMatches(in, "UNDEFINE");
                bool syntax_ok = true;
                int c = in.get();
                if(c == '{')
                  smartReadUntilNext(in, "}", expr, true);
                else
                  syntax_ok = false;
                if (!syntax_ok)
                  PLERROR("$UNDEFINE syntax is: $UNDEFINE{expr}");
                istrstream expr_stream(expr.c_str());
                string varname = readAndMacroProcess(expr_stream, variables);
                while (variables.count(varname) > 0) {
                  // This loop is probably not necessary, but just in case...
                  variables.erase(varname);
                }
              }
              break;

            default:
              PLERROR("In readAndMacroProcess: only supported macro commands are \n"
                      "${varname}, $CHAR, $DEFINE, $DIVIDE, $ECHO, $EVALUATE, $GETENV, $IF, $INCLUDE, $INT, $ISDEFINED, $ISEQUAL, $ISHIGHER, $MINUS, $PLUS, $OR, $SWITCH, $TIMES, $UNDEFINE."
                      "But I read $%c !!",c);
            }
        }
    }

  return text;
}

#ifdef WIN32
#undef chdir
#endif

} // end of namespace PLearn
