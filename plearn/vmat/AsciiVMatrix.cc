// -*- C++ -*-

// AsciiVMatrix.cc
// 
// Copyright (C) 2003 Rejean Ducharme
// ...
// Copyright (C) 2003 Rejean Ducharme
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
   * $Id: AsciiVMatrix.cc,v 1.1 2003/02/18 16:30:28 ducharme Exp $ 
   ******************************************************* */

/*! \file AsciiVMatrix.cc */
#include "AsciiVMatrix.h"

namespace PLearn <%
using namespace std;


IMPLEMENT_NAME_AND_DEEPCOPY(AsciiVMatrix);

AsciiVMatrix::AsciiVMatrix(const string& fname, bool readwrite)
  :inherited(), filename(fname), readwritemode(readwrite), newfile(false)
{
  build();
}

AsciiVMatrix::AsciiVMatrix(const string& fname, int the_width, const vector<string>& the_comments)
  :inherited(0,the_width), filename(fname), comments(the_comments),
   readwritemode(true), newfile(true)
{
  build();
}

void AsciiVMatrix::build()
{
  inherited::build();
  build_();
}

void AsciiVMatrix::build_()
{
  //setMtime(mtime(filename));

  if (newfile)
  {
    if (isfile(filename))
      PLERROR("In AsciiVMatrix constructor: filename %s already exists",filename.c_str());
    file = new fstream();
    file->open(filename.c_str(), ios::in | ios::out | ios::trunc);
  if (!file->is_open())
    PLERROR("In AsciiVMatrix constructor: could not open file %s for reading",filename.c_str());

    *file << "#size: ";
    vmatlength_pos = file->tellp();
    *file << "0         " << width() << endl;
    for (vector<string>::iterator it = comments.begin(); it != comments.end(); ++it)
    {
      *file << "# " << *it << endl;
    }

    string field_infos = filename+".fieldnames";
    if (isfile(field_infos))
      loadFieldInfos(field_infos);
  }
  else  // open old file
  {
    if (!isfile(filename))
      PLERROR("In AsciiVMatrix constructor (with specified width), filename %s does not exists",filename.c_str());
    file = new fstream();
    file->open(filename.c_str(), ios::in | ios::out);
    if (!file->is_open())
      PLERROR("In AsciiVMatrix constructor, could not open file %s for reading",filename.c_str());

    // read the matrix in old or new format
    int length = -1;
    int width = -1;
    bool could_be_old_amat=true; // true while there is still a chance that this be an "old" amat format (length and width in first row with no starting #size:) 
    *file >> ws;
    string line;  
    while (file->peek()=='#')
    {
      getline(*file, line);
      could_be_old_amat = false;  
      unsigned int pos=line.find(":");
      if (pos!=string::npos)
      {
        string sub=line.substr(0,pos);
        if (sub=="#size") // we've found the dimension specification line
        {
          string siz=removeblanks((line.substr(pos)).substr(1));
          vector<string> dim = split(siz," ");
          if (dim.size()!=2)  PLERROR("I need exactly 2 dimensions for matrix");
          length = toint(dim[0]);
          width = toint(dim[1]);
        }
        else if (sub=="#") // we've found the fieldnames specification line
        {
          string fnl=line.substr(pos).substr(1);
          TVec<string> fieldnames = split(fnl," ");
          width=fieldnames.size();
        }
      }
      *file >> ws;
    }

    if (length==-1)  // still looking for size info...
    {
      string line;
      getNextNonBlankLine(*file,line);
      if (line=="")
      {
        width=length=0;
        file->seekg(0);
        file->clear();
        could_be_old_amat=false;
      }
      int nfields1 = split(line).size();
      getNextNonBlankLine(*file,line);
      if (line=="")
      {
        length=1;
        width=nfields1;
        file->seekg(0);
        file->clear();
        could_be_old_amat=false;
      }
      int nfields2 = split(line).size();
      int guesslength = countNonBlankLinesOfFile(filename);
      real a, b;
      if (could_be_old_amat && nfields1==2) // could be an old .amat with first 2 numbers being length width
      {
        file->seekg(0);
        file->clear();
        *file >> a >> b;
        if (guesslength == int(a)+1 && real(int(a))==a && real(int(b))==b && a>0 && b>0 && int(b)==nfields2) // it's clearly an old .amat
        {
          length = int(a);
          width = int(b);
        }
      }

      if (length==-1) // still don't know size info...
      {
        if (nfields1==nfields2) // looks like a plain ascii file
        {
          length=guesslength;
          if (width!=-1 && width!=nfields1)
          {
            PLWARNING("In AsciiVMatrix:  Number of fieldnames and width mismatch in file %s.", filename.c_str());
            //fieldnames.resize(nfields1);
            //for (int i= 0; i < nfields1; ++i)
            //  fieldnames[i]= string("Field-") + tostring(i);
          }
          width = nfields1;
          file->seekg(0);
          file->clear();
        }
        else if (real(int(a))==a && real(int(b))==b && a>0 && b>0 && int(b)==nfields2)
        {
          length = int(a);
          width = int(b);
        }
      }
    }
 
    if (length==-1)
      PLERROR("In AsciiVMatrix: trying to load but couldn't determine file format automatically for %s",filename.c_str());

    length_ = length;
    width_ = width;

    // build the vector of position of the begining of the lines
    pos_rows.clear();
    while (!file->eof())
    {
      pos_rows.push_back(file->tellg());
      cout << "build : pos = " << pos_rows.back() << endl;
      string line;
      getline(*file, line);
      *file >> ws;
    }
    if (pos_rows.size() != (unsigned int)length)
      PLERROR("In AsciiVMatrix: the matrix has not the rigth size");
  }
}

void AsciiVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>length())
    PLERROR("In AsciiVMatrix::getRow, bad row number %d",i);
  if(v.length() != width())
    PLERROR("In AsciiVMatrix::getRow, length of v (%d) does not match matrix width (%d)",v.length(),width());
#endif

  file->seekg(pos_rows[i]);
  cout << "getRow : pos = " << pos_rows[i] << endl;
  cout << "v avant = " << v << endl;
  for (int j=0; j<width(); j++)
    *file >> v[j];
  cout << "v apres = " << v << endl;
}

void AsciiVMatrix::appendRow(Vec v)
{
  // write the Vec at the end of the file
  file->seekp(0,ios::end);
  pos_rows.push_back(file->tellg());
  cout << "appendRow : pos = " << pos_rows.back() << endl;
  for (int i=0; i<v.length(); i++)
    *file << v[i] << " ";
  *file << endl;

  // update the length
  length_++;
  file->seekp(vmatlength_pos);
  *file << length();
}

void AsciiVMatrix::put(int i, int j, real value)
{ PLERROR("In AsciiVMatrix::put not accepted."); }
void AsciiVMatrix::putSubRow(int i, int j, Vec v)
{ PLERROR("In AsciiVMatrix::putSubRow not accepted."); }
void AsciiVMatrix::putRow(int i, Vec v)
{ PLERROR("In AsciiVMatrix::putRow not accepted."); }

void AsciiVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "filename", &AsciiVMatrix::filename, OptionBase::buildoption, "Filename of the matrix");
  inherited::declareOptions(ol);
}

string AsciiVMatrix::help() const
{
  // ### Provide some useful description of what the class is ...
  return 
    "AsciiVMatrix implements a ..."
    + optionHelp();
}

AsciiVMatrix::~AsciiVMatrix()
{
  if (file->is_open()) file->close();
  delete file;
}

%> // end of namespcae PLearn

