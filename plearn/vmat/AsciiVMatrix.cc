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
   * $Id: AsciiVMatrix.cc,v 1.4 2003/02/21 21:26:37 ducharme Exp $ 
   ******************************************************* */

/*! \file AsciiVMatrix.cc */
#include "AsciiVMatrix.h"

namespace PLearn <%
using namespace std;


IMPLEMENT_NAME_AND_DEEPCOPY(AsciiVMatrix);

AsciiVMatrix::AsciiVMatrix(const string& fname, bool readwrite)
  :inherited(), filename(fname), readwritemode(readwrite), newfile(false),
   rewrite_length(true)
{
  build();
}

AsciiVMatrix::AsciiVMatrix(const string& fname, int the_width, const vector<string>& the_comments)
  :inherited(0,the_width), filename(fname), comments(the_comments),
   readwritemode(true), newfile(true), rewrite_length(true)
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
    file->open(filename.c_str(), fstream::in | fstream::out | fstream::trunc);
  if (!file->is_open())
    PLERROR("In AsciiVMatrix constructor: could not open file %s for reading",filename.c_str());

    *file << "#size: ";
    vmatlength_pos = file->tellp();
    length_max = 9999999;  // = 10 000 000 - 1
    *file << "0 " << width() << "      " << endl;
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
    file->open(filename.c_str(), fstream::in | fstream::out);
    if (!file->is_open())
      PLERROR("In AsciiVMatrix constructor, could not open file %s for reading",filename.c_str());

    // read the matrix in old or new format
    int length = -1;
    int width = -1;
    bool could_be_old_amat = true;
    file->seekg(0,fstream::beg);
    *file >> ws;
    string line;  
    while (file->peek()=='#')
    {
      streampos old_pos = file->tellg();
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

          // we set vmatlength_pos
          streampos current_pos = file->tellg();
          file->seekg(old_pos);
          char c = file->get();
          while (c != ':')
          {
            c = file->get();
          }
          c = file->get();
          vmatlength_pos = file->tellp();
          file->seekg(current_pos);

          // we set length_max
          int width_ndigits = (int)log(real(width)) + 1;
          string remain = removenewline(line.substr(pos+1));
          int length_ndigits = remain.length() - width_ndigits - 1;
          length_max = (int)pow(10.0,real(length_ndigits)) - 1;
        }
      }
      *file >> ws;
    }

    if (length==-1)  // still looking for size info...
    {
      string line;
      getNextNonBlankLine(*file,line);
      int nfields1 = split(line).size();
      getNextNonBlankLine(*file,line);
      if (line=="") // only one line, no length nor width info
      {
        length=1;
        width=nfields1;
        rewrite_length = false;
        could_be_old_amat = false;
      }
      int nfields2 = split(line).size();
      int guesslength = countNonBlankLinesOfFile(filename);
      real a, b;
      if (could_be_old_amat && nfields1==2) // could be an old .amat with first 2 numbers being length width
      {
        file->seekg(0,fstream::beg);
        file->clear();
        vmatlength_pos = file->tellp();
        *file >> a >> b;
        if (guesslength == int(a)+1 && real(int(a))==a && real(int(b))==b && a>0 && b>0 && int(b)==nfields2) // it's clearly an old .amat
        {
          length = int(a);
          width = int(b);

          file->seekg(vmatlength_pos);
          getline(*file, line);
          int width_ndigits = (int)log(real(width)) + 1;
          int max_length_ndigits = line.length() - width_ndigits - 1;
          length_max = (int)pow(10.0,real(max_length_ndigits)) - 1;
        }
      }

      if (length==-1) // still don't know size info...
      {
        if (nfields1==nfields2) // looks like a plain ascii file
        {
          rewrite_length = false;
          length=guesslength;
          if (width!=-1 && width!=nfields1)
          {
            PLWARNING("In AsciiVMatrix:  Number of fieldnames and width mismatch in file %s.", filename.c_str());
          }
          width = nfields1;
        }
        else
          PLERROR("In AsciiVMatrix: trying to load but couldn't determine file format automatically for %s",filename.c_str());
      }
    }
 
    length_ = length;
    width_ = width;

    // build the vector of position of the begining of the lines
    file->seekg(0,fstream::beg);
    file->clear();
    if (could_be_old_amat && rewrite_length)
    {
      string line;
      getline(*file, line);
    }
    pos_rows.clear();
    while (!file->eof())
    {
      *file >> ws;
      if (file->peek()!='#' && file->peek()!=EOF) pos_rows.push_back(file->tellg());
      string line;
      getline(*file, line);
    }
    file->clear();
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
  for (int j=0; j<width(); j++)
    *file >> v[j];
}

void AsciiVMatrix::appendRow(Vec v)
{
  if (length() == length_max)
    PLERROR("AsciiVMatrix::appendRow aborted: the matrix has reach its maximum length.");

  if (readwritemode)
  {
    // write the Vec at the end of the file
    file->seekp(0,fstream::end);
    pos_rows.push_back(file->tellg());
    for (int i=0; i<v.length(); i++)
      *file << v[i] << " ";
    *file << endl;

    // update the length
    length_++;
    if (rewrite_length)
    {
      file->seekp(vmatlength_pos);
      *file << length() << " " << width();
    }
  }
  else
    PLERROR("AsciiVMatrix::appendRow aborted: the vmat has been open in read only format.");
}

void AsciiVMatrix::put(int i, int j, real value)
{ PLERROR("In AsciiVMatrix::put not permitted."); }
void AsciiVMatrix::putSubRow(int i, int j, Vec v)
{ PLERROR("In AsciiVMatrix::putSubRow not permitted."); }
void AsciiVMatrix::putRow(int i, Vec v)
{ PLERROR("In AsciiVMatrix::putRow not permitted."); }

void AsciiVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "filename", &AsciiVMatrix::filename, OptionBase::buildoption, "Filename of the matrix");
  inherited::declareOptions(ol);
}

string AsciiVMatrix::help() const
{
  // ### Provide some useful description of what the class is ...
  return 
    "AsciiVMatrix implements a file in ascii format"
    + optionHelp();
}

AsciiVMatrix::~AsciiVMatrix()
{
  if (file->is_open()) file->close();
  delete file;
}

%> // end of namespace PLearn

