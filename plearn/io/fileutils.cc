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

// Win32 specific declarations.
#if defined(WIN32) && !defined(__CYGWIN__) && !defined(_MINGW_)
#include <direct.h> // Needed for declaration of _chdir.
#define SYSTEM_CHDIR _chdir
#else
#define SYSTEM_CHDIR chdir
#include <unistd.h>
#endif // WIN32

#include "fileutils.h"
#include "openFile.h"
#include "openString.h"
#include "PStream.h"
#include "PPath.h"
#include <plearn/base/tostring.h> 
#include <plearn/base/stringutils.h> //!< For 'slash' // TODO Can we get rid of this ?
#include <plearn/base/plerror.h>
#include <plearn/math/pl_math.h>    //!< For 'real'.

#include <plearn/base/PrUtils.h>
#include <nspr/prio.h>
#include <nspr/prtime.h>
#include <nspr/prerror.h>
#include <nspr/prlong.h>
#include <nspr/prenv.h>

namespace PLearn {
using namespace std;

// TODO Use NSPR everywhere !?
// TODO-PPath: there are a few things to fix to make it fully PPath-compliant.
// TODO-PStream: this file is PStream-compliant


/**
 * Workaround for the fact that on Windows the PR_GetFileInfo function
 * does a wildcard expansion of the filename before returning the file stats,
 * which slows down (for example) lookups of the modification times for a
 * number of files by a big factor on Windows.
 *
 * Use this function instead of PR_GetFileInfo{,64} in fileutils.cc for
 * functions that deal with files only. (This function doesn't work portably 
 * with directories.)
 */
static PRStatus PR_GetFileInfo64_NoWildcards(const char *fn, 
                                             PRFileInfo64 *info)
{
    PRFileDesc* f = PR_Open(fn, PR_RDONLY, 0);
    if (!f)
        return PR_FAILURE;

    PRStatus status = PR_GetOpenFileInfo64(f, info);
    PR_Close(f);
    return status;
}
  
///////////
// chdir //
///////////
int chdir(const PPath& path) 
{ 
    int status = ::SYSTEM_CHDIR(path.absolute().c_str()); 
    if (status!=0)
        PLERROR("Could not chdir to %s.",path.absolute().c_str());
    return status;
}

////////////////
// pathexists //
////////////////
bool pathexists(const PPath& path)
{
    PRFileInfo64 fi;

    if (PR_GetFileInfo64(path.absolute().c_str(), &fi) != PR_SUCCESS)
        return false;
    else
        return fi.type == PR_FILE_FILE || fi.type == PR_FILE_DIRECTORY;
}

///////////
// isdir //
///////////
bool isdir(const PPath& path)
{
    PRFileInfo64 fi;

    if (PR_GetFileInfo64(path.absolute().c_str(), &fi) != PR_SUCCESS)
        return false;
    else
        return fi.type == PR_FILE_DIRECTORY;
}

////////////
// isfile //
////////////
bool isfile(const PPath& path)
{
    PRFileInfo64 fi;

    if (PR_GetFileInfo64(path.absolute().c_str(), &fi) != PR_SUCCESS)
        return false;
    else
        return fi.type == PR_FILE_FILE;
}

////////////
// isfile //
////////////
bool isemptyFile(const PPath& path)
{
    PRFileInfo64 fi;

    if (PR_GetFileInfo64(path.absolute().c_str(), &fi) != PR_SUCCESS)
        return false;
    else
        return (fi.type == PR_FILE_FILE) && (fi.size == 0);
}

///////////
// mtime //
///////////
time_t mtime(const PPath& path)
{
    PRFileInfo64 fi;

    if (PR_GetFileInfo64_NoWildcards(path.absolute().c_str(), &fi) != PR_SUCCESS)
        return 0;
    else {
        // The NSPR PRTime is number of microseconds since the epoch, while
        // time_t is the number of seconds since the (same) epoch.
        // Translate from the former to the later by dividing by 1e6, using
        // NSPR long long manipulation macros to be extra safe.
        PRInt64 time_t_compatible_value;
        PRInt64 one_million = LL_INIT(0, 1000000);
        LL_DIV(time_t_compatible_value, fi.modifyTime, one_million);
        return (time_t)time_t_compatible_value;
    }
}

///////////
// lsdir //
///////////
vector<string> lsdir(const PPath& dirpath)
{
    vector<string> list;

    // Since NSPR functions do not reset the current error id when nothing goes
    // wrong, we do it manually by setting it to the 'PR_MAX_ERROR' value,
    // which is a placeholder for the last available error in NSPR (thus it is
    // not a true error code by itself).
    // This will avoid a crash triggered by an earlier error.
    PR_SetError(PR_MAX_ERROR, 0);

    PRDir* d = PR_OpenDir(dirpath.absolute().c_str());
    if (!d)
        PLERROR("In lsdir: could not open directory %s",dirpath.absolute().c_str());

    PRDirEntry* dirent = PR_ReadDir(d, PR_SKIP_BOTH);
    while (dirent) {
        list.push_back(dirent->name);
        dirent = PR_ReadDir(d, PR_SKIP_BOTH);
    }

    PRErrorCode e = PR_GetError();
    // The error code 'PR_NO_MORE_FILES_ERROR' can be found due to the call to
    // 'PR_ReadDir', that sets this error when reaching the end of the
    // directory.
    if (e != PR_MAX_ERROR && e != PR_NO_MORE_FILES_ERROR)
        PLERROR("In lsdir: error while listing directory: %s.",
                getPrErrorString().c_str());

    if (PR_CloseDir(d) != PR_SUCCESS)
        PLERROR("In lsdir: error while closing directory: %s.",
                getPrErrorString().c_str());

    return list;
}

////////////////////
// lsdir_fullpath //
////////////////////
vector<PPath> lsdir_fullpath(const PPath& dirpath)
{
    // TODO Somewhat a copy of addprefix, not really elegant. Do better ?
    vector<string> without_path = lsdir(dirpath);
    vector<PPath> with_path(without_path.size());
    PPath prefix = dirpath;
    vector<string>::const_iterator it = without_path.begin();
    vector<PPath>::iterator newit = with_path.begin();
    while (it != without_path.end()) {
        *newit = prefix / *it;
        ++it;
        ++newit;
    }
    return with_path;
}


bool mkdir_lowlevel(const PPath& dirname)
{
    return PR_MkDir(dirname.c_str(), 0777) == PR_SUCCESS;
}
    

/////////////////
// force_mkdir //
/////////////////
// If you can't spot a race condition in the previous version of this function
// (look in the version control history), please don't change the logic used
// here.
bool force_mkdir(const PPath& dirname)
{
    if (dirname.isEmpty())
        PLERROR("In force_mkdir - Parameter 'dirname' is empty");
    
    vector<PPath> paths;
    PPath path = dirname.absolute();
    while (!path.isRoot()) {
        paths.push_back(path);
        path = path.up();
    }

    for (int i = int(paths.size()) - 1; i >= 0; i--)
        mkdir_lowlevel(paths[i].absolute());

    return isdir(dirname);
}

//////////////////////////
// force_mkdir_for_file //
//////////////////////////
void force_mkdir_for_file(const PPath& filepath)
{
    PPath dirpath = filepath.dirname();
    if (!force_mkdir(dirpath))
        PLERROR("force_mkdir(%s) failed",dirpath.absolute().c_str());
}

/////////////////
// force_rmdir //
/////////////////
bool force_rmdir(const PPath& dirname)
{
    if (!isdir(dirname))
        return false;

    const vector<PPath> entries = lsdir_fullpath(dirname);
    for (vector<PPath>::const_iterator it = entries.begin();
         it != entries.end(); ++it) {
        if (isdir(*it)) {
            if (!force_rmdir(*it))
                return false;
        }
        else {
            if (PR_Delete(it->absolute().c_str()) != PR_SUCCESS)
                return false;
        }
    }

    return PR_RmDir(dirname.absolute().c_str()) == PR_SUCCESS;
}

//////////////
// filesize //
//////////////
PRUint64 filesize64(const PPath& filename)
{
    PRFileInfo64 inf;
    if (PR_GetFileInfo64_NoWildcards(filename.absolute().c_str(), &inf) != PR_SUCCESS)
        PLERROR("In filesize: error getting file info for %s: %s.",
                filename.absolute().c_str(), getPrErrorString().c_str());
    return inf.size;
}


//////////////////////
// loadFileAsString //
//////////////////////
string loadFileAsString(const PPath& filepath)
{
    PStream in = openFile(filepath, PStream::raw_ascii, "r");
    long n = filesize(filepath);

    string result;
    in.read(result, streamsize(n));
    return result;
}

//////////////////////
// saveStringInFile //
//////////////////////
void saveStringInFile(const PPath& filepath, const string& text)
{
    force_mkdir_for_file(filepath);
    PStream out = openFile(filepath, PStream::raw_ascii, "w");
    out << text;
}

////////
// cp //
////////
void cp(const PPath& srcpath, const PPath& destpath)
{
    // TODO Cross-platform version ?
    string command = "\\cp -R " + srcpath.absolute() + " " + destpath.absolute();
    system(command.c_str());
}

////////
// rm //
////////
bool rm(const PPath& file, bool fail_on_error_if_exist)
{
    // New cross-platform version.
    PRStatus ret = PR_Delete(file.absolute().c_str());
    if(fail_on_error_if_exist && ret != PR_SUCCESS && pathexists(file))
        PLERROR("Can't delete file %s",file.c_str());
    return ret == PR_SUCCESS;
    /*
    // TODO Better cross-platform version ?
#ifdef WIN32
    // For the moment works ONLY with files!!!
    if ( !DeleteFile(file.absolute().c_str()) )
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
    string command = "\\rm -rf " + file.absolute();
    system(command.c_str());
#endif
*/
}

////////
// mv //
////////
PRStatus mv(const PPath& source, const PPath& destination, bool fail_on_error)
{
    PRStatus ret=PR_Rename(source.absolute().c_str(),destination.absolute().c_str());
    if(ret!=PR_SUCCESS && fail_on_error)
        PLERROR("In mv(%s,%s) - the move failed!",source.c_str(),destination.c_str());

    return ret;
}

/////////////
// mvforce //
/////////////
PRStatus mvforce(const PPath& source, const PPath& destination, bool fail_on_error)
{
    if(PR_Access(destination.c_str(), PR_ACCESS_EXISTS)==PR_SUCCESS){
        if(PR_Delete(destination.c_str())!=PR_SUCCESS){
             if(fail_on_error)
                 PLERROR("In mvforce(%s,%s) - we failed to delete the destination!",source.c_str(),destination.c_str());
             else
                 return PR_FAILURE;
        }
    }
     return mv(source,destination);
}


//////////////////////
// readWhileMatches //
//////////////////////
void readWhileMatches(PStream& in, const string& s){
    string::size_type i = 0;
    int c;
    c = in.get();
    string::size_type n = s.length();
    while(c!=EOF)
    {
        if(s[i]!=c)
        {
            in.unget(); // Match failed, unget that last character.
            PLERROR("In readWhileMatches. Failure while matching %s: "
                    "at position %ld expected a '%c', but read a '%c'",s.c_str(),long(i),s[i],c);
        }
        ++i;
        if(i==n) // passed through the whole string 
            return;
        c = in.get();
    }
    PLERROR("In readWhileMatches, met EOF while matching %s", s.c_str());
}

////////////////////
// skipRestOfLine //
////////////////////
void skipRestOfLine(PStream& in)
{
    int c = in.get();
    while (c!='\n' && c!=EOF)
        c = in.get();
}

///////////////////////////
// skipBlanksAndComments //
///////////////////////////
void skipBlanksAndComments(PStream& in)
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

/////////////////////////
// getNextNonBlankLine //
/////////////////////////
void getNextNonBlankLine(PStream& in, string& line)
{
    while (in.good()) {
        in.getline(line);
        size_t l = line.size();
        bool ok = false;
        size_t i = 0;
        while (i < l) {
            char& c = line[i];
            if (!isspace(c)) {
                if (c == '#') {
                    if (!ok)
                        // The first non-blank character is a comment.
                        break;
                    else {
                        // We get rid of the comments.
                        line.resize(i);
                        return;
                    }
                } else {
                    // We got a non-blank, non-comment character.
                    ok = true;
                    i++;
                }
            } else
                // Read a blank character.
                i++;
        }
        if (ok)
            // We read a non-blank line with no comment.
            return;
    }
    // Could not find a non-blank line.
    line = "";
}

//////////////////////////////
// countNonBlankLinesOfFile //
//////////////////////////////
int countNonBlankLinesOfFile(const PPath& filename)
{
    PStream in = openFile(filename, PStream::raw_ascii, "r");
    int count = 0;
    int c = in.get();
    while(c!=EOF)
    {
        while(c=='\n' || c==' ' || c=='\t' || c=='\r')
            c = in.get();
        if(c!='\n' && c!='#' && c!=EOF) // We've found a non-blank, non-comment char.
            ++count;
        while(c!='\n' && c!=EOF) // Read until end of line.
            c = in.get();
        c = in.get();
    }
    return count;  
}

/////////////////
// newFilename //
/////////////////
PPath newFilename(const PPath& directory, const string& prefix, bool is_directory)
{
#if defined(_MINGW_) || (defined(WIN32) && !defined(__CYGWIN__))
    //PLERROR("This call is not yet implemented for this platform");
    char* tmpfilename = tempnam(directory.absolute().c_str(), prefix.c_str());
#else
    // TODO Could probably make a better implementation.
    const string tmpdirname = remove_trailing_slash(directory.absolute());
    const int length = int(tmpdirname.length() + 1 + prefix.length() + 6 + 1);
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
#endif
    if(!tmpfilename)
        PLERROR("In newFilename - Could not obtain temporary file name");
    if (is_directory) {
        // Defeats the purpose of creating a temporary file, but who cares?
        PLearn::rm(tmpfilename);
        PR_MkDir(tmpfilename, 0777);
    }
    return tmpfilename;
}


///////////////////////
// makeFileNameValid //
///////////////////////
PPath makeFileNameValid(const PPath& path)
{
    PPath dirname       = path.dirname();
    PPath filename_full = path.basename();
    PPath filename      = filename_full.no_extension();
    string ext          = filename_full.extension(true);
    PPath ret           = path;
    if(filename.length() + ext.length() > 256)
    {
        // We make a shorter name by encoding the rest into a few numbers.
        int j = 0;
        string rest = filename.substr(256-ext.length()-12);
        do
        {
            unsigned int n = j++;
            for(size_t i = 0; i < rest.length(); ++i)
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
        } while(pathexists(dirname / (filename + ext)));
        PLWARNING("makeFileNameValid: Filename '%s' changed to '%s'.", 
                  path.absolute().c_str(), (dirname / (filename + ext)).c_str());
        ret = (dirname / (filename + ext));
    }

    // Replace illegal characters.
    string illegal = "*?'\"${}[]@ ,()";
    for(size_t i=0;i<ret.size();i++)
        if (illegal.find(ret[i]) != string::npos)
            ret[i]='_';
    return ret;
}

///////////
// touch //
///////////
void touch(const PPath& file)
{
    string command = "touch "+ file.absolute();
    system(command.c_str());
} 

/////////////////////////////
// addFileAndDateVariables //
/////////////////////////////
void addFileAndDateVariables(const PPath& filepath, map<string, string>& variables, const time_t& latest)
{
    // Define new local variables
    variables["HOME"]        = PPath::getenv("HOME");
  
    const PPath fpath        = filepath.absolute();
    variables["FILEPATH"]    = fpath;
    variables["DIRPATH"]     = fpath.dirname();

    const PPath basename     = fpath.basename();
    variables["FILENAME"]    = basename;
    variables["FILEBASE"]    = basename.no_extension();
    variables["FILEEXT"]     = fpath.extension();
  
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
    variables["MTIME"] = tostring(latest);
}

/////////////////////////////
// readFileAndMacroProcess //
/////////////////////////////
string readFileAndMacroProcess(const PPath& filepath, map<string, string>& variables,
                               time_t& latest, bool change_dir)
{
    // pout << "Processing file: " << filepath.absolute() << endl;
    // Save old variables (to allow recursive calls)
    const char* OldVariables[] = {
        "FILEPATH", "DIRPATH", "FILENAME", "FILEBASE", "FILEEXT", "DATE", "TIME", "DATETIME"
    };
    const int num_old = sizeof(OldVariables) / sizeof(OldVariables[0]);
    map<string,string> old_vars;
    for (int i=0; i<num_old; ++i)
        old_vars[OldVariables[i]] = variables[OldVariables[i]];
    PPath file(filepath); // Default: file = filepath.

    map<string, string>* added = 0;
    map<string, string>* backup = 0;
    if (!isfile(file)) {
        // Parse 'file' for potential additional arguments.
        added  = new map<string, string>();
        backup = new map<string, string>();
        parseBaseAndParameters(file.absolute(), file, variables, added, backup);
    }

    // Possibly change directory.
    PPath old_dir;
    if (change_dir) {
        old_dir = PPath::getcwd();
        chdir(file.dirname());
        file = file.basename();
    }

    latest=max(latest,mtime(file.absolute()));

    // Add the new file and date variables
    addFileAndDateVariables(file, variables, latest);

    // Perform actual parsing and macro processing...
    PStream in = openFile(file, PStream::plearn_ascii, "r");
    string text;
    try
    { 
        text = readAndMacroProcess(in, variables, latest);
    }
    catch(const PLearnError& e)
    {
        PLERROR("while parsing file %s we got an error: \n%s",
                filepath.c_str(),e.message().c_str());
    }

    // Restore previous variables
    if (added)
        for (map<string, string>::const_iterator it = added->begin();
             it != added->end(); it++)
            variables.erase(it->first);
    if (backup)
        for (map<string, string>::const_iterator it = backup->begin();
             it != backup->end(); it++)
            variables[it->first] = it->second;
    for (int i=0; i<num_old; ++i)
        variables[OldVariables[i]] = old_vars[OldVariables[i]];

    // Restore previous directory.
    if (change_dir)
        chdir(old_dir);

    // Free memory.
    if (added)  delete added;
    if (backup) delete backup;

    return text;
}

/////////////////////////
// readAndMacroProcess //
/////////////////////////
string readAndMacroProcess(PStream& in, map<string, string>& variables, 
                           time_t& latest, bool skip_comments)
{
    string text; // the processed text to return
    bool inside_a_quoted_string=false; // inside a quoted string we don't skip characters following a #
    int c=EOF, last_c=EOF;
    while(in.good())
    {
        last_c = c;
        c = in.get();
        if (last_c!='\\' && c=='"') // we find either the beginning or end of a quoted string
            inside_a_quoted_string = !inside_a_quoted_string; // flip status

        if(!inside_a_quoted_string && c=='#' && skip_comments)
            // It's a comment: skip rest of line
            while(c!=EOF && c!='\n' && c!='\r')
                c = in.get();

        if(c==EOF)
            break;
        else if(c!='$')
            text += c;
        else  // We have a $ macro command
        {
            c = in.peek();
            switch(c)
            {
            case '{':  // expand a defined variable ${varname}
            {
                string varname; // name of a variable
                in.get(); // skip '{'
                in.smartReadUntilNext("}", varname, true);
                // Maybe there are macros to process to obtain the real name of the variable.
                PStream varname_stream = openString(varname, PStream::raw_ascii);
                varname = readAndMacroProcess(varname_stream, variables, latest);
                varname = removeblanks(varname);
                map<string, string>::const_iterator it = variables.find(varname);
                if(it==variables.end())
                    PLERROR("Macro variable ${%s} undefined", varname.c_str());
                PStream varin = openString(it->second, PStream::raw_ascii);
                text += readAndMacroProcess(varin, variables, latest);
            }
            break;

            case 'C': // it's a CHAR{expression}
            {
                string expr;
                readWhileMatches(in, "CHAR");
                bool syntax_ok = true;
                c = in.get();
                if(c == '{')
                    in.smartReadUntilNext("}", expr, true);
                else
                    syntax_ok = false;
                if (!syntax_ok)
                    PLERROR("$CHAR syntax is: $CHAR{expr}");
                PStream expr_stream = openString(expr, PStream::raw_ascii);
                char ch = (char) toint(readAndMacroProcess(expr_stream, variables, latest));
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
                    in.getline(varname, '}');
                    varname = removeblanks(varname);
                    skipBlanksAndComments(in);
                    if(in.get()!='{')
                        PLERROR("Bad syntax in .plearn DEFINE macro: correct syntax is $DEFINE{name}{definition}");
                    in.smartReadUntilNext("}", vardef, true);
                    map<string, string>::const_iterator it = variables.find(varname);
                    if (it == variables.end())
                        variables[varname] = vardef;
                    else
                        PLERROR("Variable %s is already defined, you need to first $UNDEFINE it "
                                "if you want to assign it a new value", varname.c_str());
                }
                break;

                case 'I': // it's a DIVIDE{expr1}{expr2}
                {
                    string expr1, expr2;
                    readWhileMatches(in, "IVIDE");
                    bool syntax_ok = true;
                    c = in.get();
                    if (syntax_ok) {
                        if(c == '{')
                            in.smartReadUntilNext("}", expr1, true);
                        else
                            syntax_ok = false;
                    }
                    if (syntax_ok) {
                        c = in.get();
                        if(c == '{')
                            in.smartReadUntilNext("}", expr2, true);
                        else
                            syntax_ok = false;
                    }
                    if (!syntax_ok)
                        PLERROR("$DIVIDE syntax is: $DIVIDE{expr1}{expr2}");
                    PStream expr1_stream = openString(expr1, PStream::raw_ascii);
                    PStream expr2_stream = openString(expr2, PStream::raw_ascii);
                    string expr1_eval = readAndMacroProcess(expr1_stream, variables, latest);
                    string expr2_eval = readAndMacroProcess(expr2_stream, variables, latest);
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
                    c = in.get();
                    if(c == '{')
                        in.smartReadUntilNext("}", expr, true);
                    else
                        syntax_ok = false;
                    if (!syntax_ok)
                        PLERROR("$ECHO syntax is: $ECHO{expr}");
                    PStream expr_stream = openString(expr, PStream::raw_ascii);
                    pout << readAndMacroProcess(expr_stream, variables, latest) << endl;
                }
                break;

                case 'V': // it's an EVALUATE{varname}
                {
                    string expr;
                    readWhileMatches(in, "VALUATE");
                    bool syntax_ok = true;
                    c = in.get();
                    if(c == '{')
                        in.smartReadUntilNext("}", expr, true);
                    else
                        syntax_ok = false;
                    if (!syntax_ok)
                        PLERROR("$EVALUATE syntax is: $EVALUATE{varname}");
                    PStream expr_stream = openString(expr, PStream::raw_ascii);
                    string varname = readAndMacroProcess(expr_stream, variables, latest);
                    string to_evaluate = variables[varname];
                    PStream to_evaluate_stream = openString(to_evaluate, PStream::raw_ascii);
                    string evaluated = readAndMacroProcess(to_evaluate_stream, variables, latest);
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
                c = in.get();
                if(c == '{')
                    in.smartReadUntilNext("}", expr, true);
                else
                    syntax_ok = false;
                if (!syntax_ok)
                    PLERROR("$GETENV syntax is: $GETENV{expr}");
                PStream expr_stream = openString(expr, PStream::raw_ascii);
                string var_name = readAndMacroProcess(expr_stream, variables, latest);
                const char* var = PR_GetEnv(var_name.c_str());

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
                    c = in.get();
                    if(c == '{')
                        in.smartReadUntilNext("}", cond, true);
                    else
                        syntax_ok = false;
                    if (syntax_ok) {
                        c = in.get();
                        if(c == '{')
                            in.smartReadUntilNext("}", expr_cond_true, true);
                        else
                            syntax_ok = false;
                    }
                    if (syntax_ok) {
                        c = in.get();
                        if(c == '{')
                            in.smartReadUntilNext("}", expr_cond_false, true);
                        else
                            syntax_ok = false;
                    }
                    if (!syntax_ok)
                        PLERROR("$IF syntax is: $IF{cond}{expr_cond_true}{expr_cond_false}");

                    PStream cond_stream = openString(cond, PStream::raw_ascii);
                    string evaluate_cond = readAndMacroProcess(cond_stream, variables, latest);
                    if (evaluate_cond == "1" ) {
                        expr_evaluated = expr_cond_true;
                    } else if (evaluate_cond == "0") {
                        expr_evaluated = expr_cond_false;
                    } else {
                        PLERROR("$IF condition should be 0 or 1, but is %s", evaluate_cond.c_str());
                    }
                    PStream expr_stream = openString(expr_evaluated, PStream::raw_ascii);
                    text += readAndMacroProcess(expr_stream, variables, latest);
                }
                break;

                case 'N':
                {
                    next = in.get();
                    next = in.peek();   // Next character.
                    switch(next) {

                    case 'C': // it's an INCLUDE{filepath}
                    {
                        string raw_includefilepath; // The raw path read from the script.
                        readWhileMatches(in, "CLUDE");
                        c = in.get();
                        if(c=='<')
                            in.smartReadUntilNext(">", raw_includefilepath, true);
                        else if(c=='{')
                            in.smartReadUntilNext("}", raw_includefilepath, true);
                        else
                            PLERROR("$INCLUDE must be followed immediately by a { or <");
                        PStream pathin = openString(raw_includefilepath, PStream::raw_ascii);
                        raw_includefilepath = readAndMacroProcess(pathin, variables, latest);
                        raw_includefilepath = removeblanks(raw_includefilepath);
                        PPath p = PPath(raw_includefilepath);
                        // Read file with appropriate variable definitions.
                        time_t new_latest = 0;
                        text += readFileAndMacroProcess
                            (p, variables, new_latest);
                        latest=max(latest,new_latest);
                        string s=tostring(latest);
                        variables["MTIME"]=s;
                    }
                    break;

                    case 'T': // it's an INT{val}
                    {
                        string expr;
                        readWhileMatches(in, "T");
                        bool syntax_ok = true;
                        c = in.get();
                        if(c == '{')
                            in.smartReadUntilNext("}", expr, true);
                        else
                            syntax_ok = false;
                        if (!syntax_ok)
                            PLERROR("$INT syntax is: $INT{expr}");
                        PStream expr_stream = openString(expr, PStream::raw_ascii);
                        string expr_eval = readAndMacroProcess(expr_stream, variables, latest);
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

                    next = in.get();
                    next = in.peek();   // Next character.
                    switch(next) {

                    case 'D': // it's an ISDEFINED{expr}
                    {
                        string expr;
                        readWhileMatches(in, "DEFINED");
                        bool syntax_ok = true;
                        c = in.get();
                        if(c == '{')
                            in.smartReadUntilNext("}", expr, true);
                        else
                            syntax_ok = false;
                        if (!syntax_ok)
                            PLERROR("$ISDEFINED syntax is: $ISDEFINED{expr}");
                        PStream expr_stream = openString(expr, PStream::raw_ascii);
                        string expr_eval = readAndMacroProcess(expr_stream, variables, latest);
                        map<string, string>::const_iterator it = variables.find(expr_eval);
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
                        c = in.get();
                        if(c == '{')
                            in.smartReadUntilNext( "}", expr1, true);
                        else
                            syntax_ok = false;
                        if (syntax_ok) {
                            c = in.get();
                            if(c == '{')
                                in.smartReadUntilNext("}", expr2, true);
                            else
                                syntax_ok = false;
                        }
                        if (!syntax_ok)
                            PLERROR("$ISEQUAL syntax is: $ISEQUAL{expr1}{expr2}");
                        PStream expr1_stream = openString(expr1, PStream::raw_ascii);
                        PStream expr2_stream = openString(expr2, PStream::raw_ascii);
                        string expr1_eval = readAndMacroProcess(expr1_stream, variables, latest);
                        string expr2_eval = readAndMacroProcess(expr2_stream, variables, latest);
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
                        c = in.get();
                        if(c == '{')
                            in.smartReadUntilNext("}", expr1, true);
                        else
                            syntax_ok = false;
                        if (syntax_ok) {
                            c = in.get();
                            if(c == '{')
                                in.smartReadUntilNext("}", expr2, true);
                            else
                                syntax_ok = false;
                        }
                        if (!syntax_ok)
                            PLERROR("$ISHIGHER syntax is: $ISHIGHER{expr1}{expr2}");
                        PStream expr1_stream = openString(expr1, PStream::raw_ascii);
                        PStream expr2_stream = openString(expr2, PStream::raw_ascii);
                        string expr1_eval = readAndMacroProcess(expr1_stream, variables, latest);
                        string expr2_eval = readAndMacroProcess(expr2_stream, variables, latest);
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
                c = in.get();
                if (syntax_ok) {
                    if(c == '{')
                        in.smartReadUntilNext("}", expr1,true);
                    else
                        syntax_ok = false;
                }
                if (syntax_ok) {
                    c = in.get();
                    if(c == '{')
                        in.smartReadUntilNext("}", expr2,true);
                    else
                        syntax_ok = false;
                }
                if (!syntax_ok)
                    PLERROR("$MINUS syntax is: $MINUS{expr1}{expr2}");
                PStream expr1_stream = openString(expr1, PStream::raw_ascii);
                PStream expr2_stream = openString(expr2, PStream::raw_ascii);
                string expr1_eval = readAndMacroProcess(expr1_stream, variables, latest);
                string expr2_eval = readAndMacroProcess(expr2_stream, variables, latest);
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
                c = in.get();
                if (syntax_ok) {
                    if(c == '{')
                        in.smartReadUntilNext("}", expr1,true);
                    else
                        syntax_ok = false;
                }
                if (syntax_ok) {
                    c = in.get();
                    if(c == '{')
                        in.smartReadUntilNext("}", expr2,true);
                    else
                        syntax_ok = false;
                }
                if (!syntax_ok)
                    PLERROR("$OR syntax is: $OR{expr1}{expr2}");
                PStream expr1_stream = openString(expr1, PStream::raw_ascii);
                PStream expr2_stream = openString(expr2, PStream::raw_ascii);
                string expr1_eval = readAndMacroProcess(expr1_stream, variables, latest);
                string expr2_eval = readAndMacroProcess(expr2_stream, variables, latest);
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
                c = in.get();
                if (syntax_ok) {
                    if(c == '{')
                        in.smartReadUntilNext("}", expr1,true);
                    else
                        syntax_ok = false;
                }
                if (syntax_ok) {
                    c = in.get();
                    if(c == '{')
                        in.smartReadUntilNext("}", expr2,true);
                    else
                        syntax_ok = false;
                }
                if (!syntax_ok)
                    PLERROR("$PLUS syntax is: $PLUS{expr1}{expr2}");
                PStream expr1_stream = openString(expr1, PStream::raw_ascii);
                PStream expr2_stream = openString(expr2, PStream::raw_ascii);
                string expr1_eval = readAndMacroProcess(expr1_stream, variables, latest);
                string expr2_eval = readAndMacroProcess(expr2_stream, variables, latest);
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
                c = in.get();
                if (syntax_ok) {
                    if(c == '{')
                        in.smartReadUntilNext("}", expr, true);
                    else
                        syntax_ok = false;
                }
                // Read the pairs {compx}{valx}, then {valdef}
                bool done_parsing = false;
                while (syntax_ok && !done_parsing) {
                    c = getAfterSkipBlanksAndComments(in);
                    string tmp_comp, tmp_val;
                    if(c == '{')
                        in.smartReadUntilNext("}", tmp_comp, true);
                    else
                        syntax_ok = false;
                    if (syntax_ok) {
                        c = peekAfterSkipBlanksAndComments(in);
                        if(c == '{') {
                            c = getAfterSkipBlanksAndComments(in);
                            in.smartReadUntilNext("}", tmp_val, true);
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
                PStream expr_stream = openString(expr, PStream::raw_ascii);
                string expr_eval =  readAndMacroProcess(expr_stream, variables, latest);
                bool not_done = true;
                for (size_t i = 0; i < comp.size() && not_done; i++) {
                    PStream comp_stream = openString(comp[i], PStream::raw_ascii);
                    string comp_eval = readAndMacroProcess(comp_stream, variables, latest);
                    if (expr_eval == comp_eval) {
                        not_done = false;
                        PStream val_stream = openString(val[i], PStream::raw_ascii);
                        text += readAndMacroProcess(val_stream, variables, latest);
                    }
                }
                if (not_done) {
                    // Default value needed.
                    PStream val_stream = openString(valdef, PStream::raw_ascii);
                    text += readAndMacroProcess(val_stream, variables, latest);
                }
            }
            break;

            case 'T': // it's a TIMES{expr1}{expr2}
            {
                string expr1, expr2;
                readWhileMatches(in, "TIMES");
                bool syntax_ok = true;
                c = in.get();
                if (syntax_ok) {
                    if(c == '{')
                        in.smartReadUntilNext("}", expr1, true);
                    else
                        syntax_ok = false;
                }
                if (syntax_ok) {
                    c = in.get();
                    if(c == '{')
                        in.smartReadUntilNext("}", expr2, true);
                    else
                        syntax_ok = false;
                }
                if (!syntax_ok)
                    PLERROR("$TIMES syntax is: $TIMES{expr1}{expr2}");
                PStream expr1_stream = openString(expr1, PStream::raw_ascii);
                PStream expr2_stream = openString(expr2, PStream::raw_ascii);
                string expr1_eval = readAndMacroProcess(expr1_stream, variables, latest);
                string expr2_eval = readAndMacroProcess(expr2_stream, variables, latest);
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
                c = in.get();
                if(c == '{')
                    in.smartReadUntilNext("}", expr, true);
                else
                    syntax_ok = false;
                if (!syntax_ok)
                    PLERROR("$UNDEFINE syntax is: $UNDEFINE{expr}");
                PStream expr_stream = openString(expr, PStream::raw_ascii);
                string varname = readAndMacroProcess(expr_stream, variables, latest);
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
            c = ' '; // Make sure we do not believe it is a quoted string.
        }
    }

    return text;
}

static map<string, unsigned int> count_refs_to_file;

////////////////////////
// addReferenceToFile //
////////////////////////
void addReferenceToFile(const PPath& file)
{
    if (file.isEmpty())
        return;
    string s = file.canonical();
    if (count_refs_to_file.find(s) != count_refs_to_file.end())
        count_refs_to_file[s]++;
    else
        count_refs_to_file[s] = 1;
}

///////////////////////
// noReferenceToFile //
///////////////////////
bool noReferenceToFile(const PPath& file)
{
    return nReferencesToFile(file) == 0;
}

///////////////////////
// nReferencesToFile //
///////////////////////
unsigned int nReferencesToFile(const PPath& file)
{
    if (file.isEmpty())
        return 0;
    string s = file.canonical();
    if (count_refs_to_file.find(s) != count_refs_to_file.end())
        return count_refs_to_file[s];
    else
        return 0;
}

///////////////////////////
// removeReferenceToFile //
///////////////////////////
void removeReferenceToFile(const PPath& file)
{
    if (file.isEmpty())
        return;
    string s = file.canonical();
    if (count_refs_to_file.find(s) != count_refs_to_file.end())
        if (count_refs_to_file[s] == 0)
            PLERROR("In removeReferenceToFile - Trying to decrease the counter"
                    " of references to file '%s', but it is already zero",
                    file.absolute().c_str());
        else
            count_refs_to_file[s]--;
    else
        PLERROR("In removeReferenceToFile - Trying to decrease the counter of "
                "references to file '%s', but it is not in the counter map",
                file.absolute().c_str());
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
