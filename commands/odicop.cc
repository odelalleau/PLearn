/***************************************************************************
 *   Copyright (C) 2004 by Norman Casagrande                               *
 *   casagran@iro.umontreal.ca                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <dirent.h>
#include <unistd.h>

using namespace std;

bool isdir(const string& path);
void makedir(string& dir);
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
  cout << spc << "Examining dir " << sourcedir << endl;    
  spc += "  ";

  DIR* d = opendir(sourcedir.c_str());
  if(!d)
  {
      cerr << "Could not open directory " << sourcedir.c_str() << endl;
      return 1;    
  }
  struct dirent* dent;
  while( (dent = readdir(d)) != 0)
  {
     
      string s = dent->d_name;

      if(s=="." || s=="..")
        continue;

      if (!isdir(sourcedir + s))
        continue;

      // ignore CVS dirs
      if (s.rfind("CVS",0) != string::npos)
        continue;
        
      newSourceDir = sourcedir + s + "/";
      newDestDir = destdir + s + "/";
      makedir(newDestDir);

      if(s.rfind("OBJ",0) != string::npos)
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
  closedir(d);

  return 0;
}

void copyAndLinkObjs(string& sourceOBJdir, string& original_sourcedir, string& destOBJdir)
{
  // copy all the object files
  ostringstream mvCommand(ostringstream::out);
  mvCommand << "cp -R " << sourceOBJdir << "*" << " " << destOBJdir;
  system(mvCommand.str().c_str());

  // delete the old OBJ directory  
  ostringstream rmCommand(ostringstream::out);
  rmCommand << "rm -R " << sourceOBJdir;
  system(rmCommand.str().c_str());
  
  // make the symbolic link of the OBJ directory
  ostringstream lnCommand(ostringstream::out); 
  lnCommand << "ln -s " << destOBJdir << " " << original_sourcedir;
  system(lnCommand.str().c_str());

}


void makedir(string& dir)
{
  // directory already exists!!
  if (isdir(dir))
    return;

  // make the symbolic link of the OBJ directory
  ostringstream mkdirCommand(ostringstream::out); 
  mkdirCommand << "mkdir " << dir;
  system(mkdirCommand.str().c_str());
}

bool isdir(const string& path)
{
  DIR *dfp = NULL;
  dfp = opendir(path.c_str());
  if (dfp != NULL)
    return true;
  else
    return false;
}

