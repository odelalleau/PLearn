// -*- C++ -*-

// odicop.cc
// Copyright (C) 2004 Norman Casagrande (casagran@iro.umontreal.ca)
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

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

bool isdir(const string& path);
bool islink(const string& path);
void makedir(const string& dir);
int goAndCreateDir(string sourcedir, string destdir, string spc);
void copyAndLinkObjs(string& sourceOBJdir, string& original_sourcedir, string& destOBJdir);

int main(int argc, char *argv[])
{
  
    if (argc < 3)
    {
        cout << "odicop <sourcedir> <destdir>" << endl;
        return 0;
    }
  
    string sourcedir = argv[1];
    string destdir = argv[2];
  
    if (sourcedir[sourcedir.length()-1] != '/')
        sourcedir += '/';
  
    if (destdir[destdir.length()-1] != '/')
        destdir += '/';
  
    goAndCreateDir(sourcedir, destdir, "");
  
    return EXIT_SUCCESS;
}


// recursive function
int goAndCreateDir(string sourcedir, string destdir, string spc)
{
    string newSourceDir;
    string newDestDir;
    string command;
    cout << spc << "Examining dir " << sourcedir << endl;    
    spc += "  ";

    DIR* d = opendir(sourcedir.c_str());
    if(!d)
    {
        cerr << "Could not open directory " << sourcedir.c_str() << endl;
        return 1;    
    }
    struct dirent* dent;

    bool hasCode = false;
    bool hasOBJS = false;
		
    while( (dent = readdir(d)) != 0)
    {
     
        string s = dent->d_name;

        if(s=="." || s=="..")
            continue;

  	if (s.find("OBJS") != string::npos)
        {
            if (islink(sourcedir + s))
            {
                command = "rm " + sourcedir + s;
                system(command.c_str());	
            }
            else
                hasOBJS = true;
        }

        if (!isdir(sourcedir + s))
        {
            if (s.rfind(".cc") != string::npos)
                hasCode = true;

            continue;
        }

        // ignore CVS dirs
        if (s.find("CVS") != string::npos)
            continue;
        
        newSourceDir = sourcedir + s + "/";
        newDestDir = destdir + s + "/";
        makedir(newDestDir);

        if(s.find("OBJS") != string::npos)
        {
            // OBJ dir found!      
            cout << spc << "-> Copying and creating link..";
            cout.flush();
            copyAndLinkObjs(newSourceDir, sourcedir, newDestDir);
            cout << "done!" << endl;
        }
        else
        {      
            // normal dir
            goAndCreateDir(newSourceDir, newDestDir, spc);
        }
     
    }

    if (hasCode && !hasOBJS)
    {
        cout << spc << "-> Creating OBJS dir and linking it..";
  
        // checks if directory already exists in the destination
		 
        if (!isdir(destdir + "OBJS"))
            makedir(destdir + "OBJS");
			 
        command = "ln -s " + destdir + "OBJS " + sourcedir;
        system(command.c_str());
   
        cout << "done!" << endl;
    }

	
    closedir(d);

    return 0;
}

void copyAndLinkObjs(string& sourceOBJdir, string& original_sourcedir, string& destOBJdir)
{
    string command;

    // copy all the object files
    command = "cp -R " + sourceOBJdir + "*" + " " + destOBJdir;
    system(command.c_str());

    // delete the old OBJ directory  
    command = "rm -R " + sourceOBJdir;
    system(command.c_str());
  
    // make the symbolic link of the OBJ directory
    command = "ln -s " + destOBJdir + " " + original_sourcedir;
    system(command.c_str());

}


void makedir(const string& dir)
{
    // directory already exists!!
    if (isdir(dir))
        return;

    // make the symbolic link of the OBJS directory
    string mkdirCommand = "mkdir " + dir;
    system(mkdirCommand.c_str());
}

bool isdir(const string& path)
{
    struct stat statusinfo;
    int status;

    status = lstat(path.c_str(), &statusinfo);
		
    if (status != 0)
        return false;
	
    if (S_ISDIR(statusinfo.st_mode))
        return true;
    else
        return false;
}

bool islink(const string& path)
{
    struct stat statusinfo;
    int status;

    status = lstat(path.c_str(), &statusinfo);
		
    if (status != 0)
        return false;
	
    if (S_ISLNK(statusinfo.st_mode))
        return true;
    else
        return false;

}


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
