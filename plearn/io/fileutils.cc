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
   * $Id: fileutils.cc,v 1.22 2004/02/27 21:14:49 nova77 Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#if !defined(_MSC_VER) && !defined(_MINGW_)
#include <sys/wait.h>
#endif

// norman: added win32 specific declarations
#ifdef WIN32
#include <direct.h>
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)

#define PL_MAX_FILE_SIZE 1000
#define chdir _chdir
#define stat _stat
#include <Windows.h>

#else
#include <dirent.h>
#include <unistd.h>

#endif // WIN32

#include <sstream>
#include "fileutils.h"
#include "stringutils.h"
#include "plerror.h"
#include "PStream.h"

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
    for (int i = 0; buf[i] != 0 && i < 2000; ++i) {
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
    PLERROR("Could not chdir to %s\n",path.c_str());
  return status;
}

  string abspath(const string& path)
  {    
    string cwd = getcwd();
    string result;
    if(isdir(path))
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
   struct stat s;
    int status = stat(path.c_str(),&s);
    if(status!=0)
      return false;
    return S_ISDIR(s.st_mode) | S_ISREG(s.st_mode);
  }

  bool isdir(const string& path)
  {
#if defined(_MINGW_) || defined(WIN32) 
    if (path[path.size()-1] == ':') // c: or C: or r: or R: or...
      return true;
#endif

    struct stat s;
    int status = stat(path.c_str(),&s);
    if(status!=0)
      return false;
    return S_ISDIR(s.st_mode);
  }

  bool isfile(const string& path)
  {
    struct stat s;
    int status = stat(path.c_str(),&s);
    if(status!=0)
      return false;
    return S_ISREG(s.st_mode);
  }

  time_t mtime(const string& path)
  {
    struct stat s;
    int status = stat(path.c_str(),&s);
    if(status!=0)
      return 0;
    else
      return s.st_mtime;
  }

  // Returns a list of all entries in the given directory (omitting entries "." and "..")
  // If the direcotry cannot be opened an error is issued.
  // The returned entries are not full paths.
  vector<string> lsdir(const string& dirpath)
  {
    vector<string> list;

    // norman: added check
#ifdef WIN32

    // norman: Experimental version of directory listing for WIN32

    WIN32_FIND_DATA fileData; 
    HANDLE hSearch; 
    bool fFinished = false; 
    char oldpath[PL_MAX_FILE_SIZE];

    GetCurrentDirectory(FILENAME_MAX, oldpath);

    if (! SetCurrentDirectory(dirpath.c_str()) )
    {
      SetCurrentDirectory(oldpath);
      PLERROR("In lsdir: could not open directory %s",dirpath.c_str());
    }

    hSearch = FindFirstFile("*", &fileData); 
    if (hSearch == INVALID_HANDLE_VALUE) 
    {
      SetCurrentDirectory(oldpath);
      PLERROR("In lsdir: could not open directory %s. Invalid Handle Value.",dirpath.c_str());
    }

    while (!fFinished) 
    { 
      string s = fileData.cFileName;
      if(s!="." && s!="..")
        list.push_back(s);

      if (!FindNextFile(hSearch, &fileData)) 
      {
        if (GetLastError() == ERROR_NO_MORE_FILES) 
        { 
          fFinished = true; 
        } 
        else 
        { 
          printf("Couldn't find next file."); 
          // strange problem! :)
        } 
      }
    }

     SetCurrentDirectory(oldpath);

#else

    DIR* d = opendir(dirpath.c_str());
    if(!d)
        PLERROR("In lsdir: could not open directory %s",dirpath.c_str());
    struct dirent* dent;
    while( (dent = readdir(d)) != 0)
      {
        string s = dent->d_name;
        if(s!="." && s!="..")
          list.push_back(s);
      }
    closedir(d);
#endif
    return list;
  }


  // Same as lsdir, except dirpath is prepended to the entries' names.
  vector<string> lsdir_fullpath(const string& dirpath)
  { return addprefix(remove_trailing_slash(dirpath)+slash,lsdir(dirpath)); }


  // Forces directory creation if it doesn't already exist. 
  // (also creates any missing directory along its path)
  // Return value indicates success (true) or failure (false).
  // If the directory already exists, true is returned.
  bool force_mkdir(const string& dirname)
  {
    string path = remove_trailing_slash(dirname);
    if(isdir(path))
      return true;

    int pos = 0;
#if !defined(_MINGW_) && !defined(WIN32)
    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
#endif
    string pathpart;
    do
      {
        pos++;
        pos = path.find(slash,pos);
        if(pos>=0)
          pathpart = path.substr(0,pos);
        else
          pathpart = path;
        if(!isdir(pathpart))
          {
#if !defined(_MINGW_) && !defined(WIN32)
            if(mkdir(pathpart.c_str(),mode)!=0)
#else
            if(mkdir(pathpart.c_str())!=0)
#endif
              return false;
          }
      } while(pos>=0);
    return true;
  }

void force_mkdir_for_file(const string& filepath)
{
  string dirpath = extract_directory(filepath);
  if(!force_mkdir(dirpath))
    PLERROR("force_mkdir(%s) failed",dirpath.c_str());
}

  // Forces removal of directory and all its content
  // Return value indicates success (true) or failure (false)
  // If the directory does not exist, false is returned.
  bool force_rmdir(const string& dirname)
  {
    system("pwd");
    if(!isdir(dirname))
      return false;
    vector<string> entries = lsdir_fullpath(dirname);
    vector<string>::const_iterator it = entries.begin();
    while(it!=entries.end())
      {
        if(isdir(*it))
          {
            if(!force_rmdir(*it))
              return false;
          }
        else
          {
            if(unlink(it->c_str())!=0)
              return false;
          }
        ++it;
      }
    if(rmdir(dirname.c_str())!=0)
      return false;
    return true;

  }

  long filesize(const string& filename)
  {
    FILE *f = fopen(filename.c_str(),"r");
    long fsize;
    if (!f)
      PLERROR("In filesize(const string& filename): cannot open file %s.",filename.c_str());
    fseek(f,0,SEEK_END);
    fsize = ftell(f);
    fclose(f);
    return fsize;
  }

  string loadFileAsString(const string& filepath)
  {
    long l = filesize(filepath);
    char* buf = new char[l];    
    ifstream in(filepath.c_str());
    if(in.bad())
      PLERROR("Cannot load file %s",filepath.c_str());
    in.read(buf, l);
    string text(buf,l);
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
    string command = "\\rm -rf " + file;
    system(command.c_str());
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
  int n = s.length();
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

// reads everything until '\n' (also consumes the '\n')
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
      int l = line.length();
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

int smartReadUntilNext(istream& in, string stoppingsymbols, string& characters_read)
{
  PStream pin(&in);
  return pin.smartReadUntilNext(stoppingsymbols, characters_read);
}
  
//!  Makes use of mkstemp(...) to create a new file.
string newFilename(const string directory, const string prefix, bool is_directory)
{
#if defined(_MINGW_) || defined(WIN32)
    PLERROR("This call is not yet implemented for this platform");
    return "";
#else
  string tmpdirname = remove_trailing_slash(directory);
  int length = tmpdirname.length()+1+prefix.length()+6+1;
  char* tmpfilename = new char[length];
  if (tmpdirname=="") //!<  save in current dir
    sprintf(tmpfilename,"%sXXXXXX",prefix.c_str());
  else
    sprintf(tmpfilename,"%s/%sXXXXXX",tmpdirname.c_str(),prefix.c_str());
  mkstemp(tmpfilename);
  if (is_directory) {
      // Defeats the purpose of mktemp, but who cares?
      std::remove(tmpfilename);
      // see <sys/stat.h>, mode= 0666
      mode_t mode = (S_IRUSR|S_IWUSR|S_IXUSR) | S_IRWXG | S_IRWXO;
      mkdir(tmpfilename, mode);
  }
  if(!tmpfilename)
    PLERROR("In newFilename : could not make a new temporary filename");
  return tmpfilename;
#endif
}

string makeFileNameValid(const string& path)
{
  string dirname = extract_directory(path);
  string filename = extract_filename_without_extension(path);
  string ext= extract_extension(path);
  string ret=path;
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
  
  // Define new local variables
  string fpath = abspath(filepath);
  variables["FILEPATH"] = fpath;
  variables["DIRPATH"] = remove_trailing_slash(extract_directory(fpath));
  variables["FILENAME"] = extract_filename(fpath);
  variables["FILEBASE"] = remove_extension(extract_filename(fpath));
  variables["FILEEXT"] = extract_extension(fpath);

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

  // Perform actual parsing and macro processing...
  ifstream in(fpath.c_str());
  if(!in)
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

  while(in)
    {
      int c = in.get();
      if(c=='#')  // It's a comment: skip rest of line
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
                getline(in,varname,'}');
                varname = removeblanks(varname);
                map<string, string>::iterator it = variables.find(varname);
                if(it==variables.end())
                  PLERROR("Macro variable ${%s} undefined", varname.c_str());
                istrstream varin(it->second.c_str());
                text += readAndMacroProcess(varin, variables);
              }
              break;

            case 'I': // it's an INCLUDE{filepath}
              {
                string includefilepath; // path of the file in a $INCLUDE{...} directive
                readWhileMatches(in, "INCLUDE");
                int c = in.get();
                if(c=='<')
                  smartReadUntilNext(in, ">", includefilepath);
                else if(c=='{')
                  smartReadUntilNext(in, "}", includefilepath);
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

            case 'D': // it's a DEFINE{ varname }{ ... }
              {
                string varname; // name of a variable
                string vardef; // definition of a variable
                readWhileMatches(in, "DEFINE{");
                getline(in,varname, '}');
                varname = removeblanks(varname);
                skipBlanksAndComments(in);
                if(in.get()!='{')
                  PLERROR("Bad syntax in .plearn DEFINE macro: correct syntax is $DEFINE{name}{definition}");
                smartReadUntilNext(in, "}", vardef);
                variables[varname] = vardef;
              }
              break;

            default:
              PLERROR("In readAndMacroProcess: only supported macro commands are \n"
                      "$DEFINE{varname=...} $INCLUDE{filepath} and ${varname} for macro expansion."
                      "But I read $%c !!",c);
            }
        }
    }

  // cerr << "MACRO PROCESSEd TEXT: \n" << text << endl;

  return text;
}

#ifdef WIN32
#undef S_ISDIR
#undef S_ISREG
#undef PL_MAX_FILE_SIZE
#undef chdir
#undef stat
#endif

} // end of namespace PLearn

// int main()
/*
{
  using namespace PLearn;
  
  map<string, string> variables;
  string text =  readFileAndMacroProcess("essai.plearn", variables);
  
  cout << "TEXT:" << endl;
  cout << text << endl;

  cout << "\nVARIABLES:" << endl;

  PStream pout(&cout);
  pout << variables << endl;

  return 0;
}
*/

