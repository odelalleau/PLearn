// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
   * $Id: DiskVMatrix.cc,v 1.3 2003/04/29 21:33:43 plearner Exp $
   ******************************************************* */

#include "DiskVMatrix.h"
#include "VecCompressor.h"

namespace PLearn <%
using namespace std;



/** DiskVMatrix **/

DiskVMatrix::DiskVMatrix(const string& the_dirname, bool readwrite)
  : readwritemode(readwrite),freshnewfile(false),
    dirname(remove_trailing_slash(the_dirname))
{
  build_();
}

DiskVMatrix::DiskVMatrix(const string& the_dirname, int the_width, bool write_double_as_float)  
  : RowBufferedVMatrix(0,the_width),readwritemode(true), 
  freshnewfile(true),dirname(remove_trailing_slash(the_dirname))
{
  build_();
}

void DiskVMatrix::writeRow(ostream& out, const Vec& v)
{ VecCompressor::writeCompressedVec(out,v); }
 
void DiskVMatrix::readRow(istream& in, const Vec& v)
{ VecCompressor::readCompressedVec(in, v); }

void DiskVMatrix::build()
{
  inherited::build();
  build_();
}

void DiskVMatrix::build_()
{
  if(!freshnewfile)
  {
    if(!isdir(dirname))
      PLERROR("In DiskVMatrix constructor, directory %s could not be found",dirname.c_str());
    setMetaDataDir(dirname + ".metadata"); 
    setMtime(mtime(append_slash(dirname)+"indexfile"));
    ios::openmode omode;
    if(readwritemode)
      omode = ios::in | ios::out | ios::binary;
    else // read-only
      omode = ios::in | ios::binary;

    string indexfname = dirname+"/indexfile";
    indexf = new fstream();
    indexf->open(indexfname.c_str(), omode);
    if(!*indexf)
      PLERROR("In DiskVMatrix constructor, could not open file %s in specified mode", indexfname.c_str());
  
    int header;
    indexf->read((char*)&header,sizeof(int));
    indexf->read((char*)&length_,sizeof(int));
    indexf->read((char*)&width_,sizeof(int));

    int k=0;
    string fname = dirname+"/"+tostring(k)+".data";
    while(isfile(fname))
    {
      fstream* f = new fstream();
      f->open(fname.c_str(), omode);
      if(!(*f))
        PLERROR("In DiskVMatrix constructor, could not open file %s in specified mode", fname.c_str());
      dataf.append(f);
      fname = dirname+"/"+tostring(++k)+".data";
    }
    // Stuff related to RowBufferedVMatrix, for consistency
    current_row_index = -1;
    current_row.resize(width_);
    other_row_index = -1;
    other_row.resize(width_);

    //resize the string mappings
    map_sr = TVec<map<string,real> >(width_);
    map_rs = TVec<map<real,string> >(width_);

    getFieldInfos();
  }
  else
  {
    if(isdir(dirname))
      PLERROR("In DiskVMatrix constructor (with specified width), directory %s already exists",dirname.c_str());
    setMetaDataDir(dirname + ".metadata");
    setMtime(mtime(append_slash(dirname)+"indexfile"));

    //ios::openmode omode;
    if(isfile(dirname)) // patch for running mkstemp (TmpFilenames)
      unlink(dirname.c_str());
    if(!force_mkdir(dirname)) // force directory creation 
      PLERROR("In DiskVMatrix constructor (with specified width), could not create directory %s  Error was: %s",dirname.c_str(), strerror(errno));

    string indexfname = dirname + "/indexfile";
    indexf = new fstream();
    indexf->open(indexfname.c_str(),ios::in | ios::out | ios::trunc | ios::binary);

    int header = 123408; 
    indexf->write((char*)&header,sizeof(int));
    indexf->write((char*)&length_,sizeof(int));
    indexf->write((char*)&width_,sizeof(int));
  
    string fname = dirname + "/0.data";
    // These two line don't work (core dump!) with our actual libraries (sigh!)
    fstream* f = new fstream();
    f->open(fname.c_str(), ios::in | ios::out | ios::trunc | ios::binary);
    dataf.append(f);
  }
  freshnewfile=false;
}

void DiskVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "dirname", &DiskVMatrix::dirname, OptionBase::buildoption, "Directory name of the.dmat");
  inherited::declareOptions(ol);
}

void DiskVMatrix::getRow(int i, Vec v) const
{ 
#ifdef BOUNDCHECK
  if(i<0 || i>length())
    PLERROR("In DiskVMatrix::getRow, bad row number %d",i);
  if(v.length() != width())
    PLERROR("In DiskVMatrix::getRow, length of v (%d) does not match matrix width (%d)",v.length(),width());
#endif

  unsigned char filenum;
  unsigned int position;
  indexf->seekg(3*sizeof(int) + i*(sizeof(unsigned char)+sizeof(unsigned int)));
  indexf->get((char&)filenum);
  indexf->read((char*)&position,sizeof(unsigned int));
  fstream* f = dataf[int(filenum)];
  f->seekg(position);
  binread_compressed(*f,v.data(),v.length());
}

void DiskVMatrix::putRow(int i, Vec v)
{ 
#ifdef BOUNDCHECK
  if(i<0 || i>length())
    PLERROR("In DiskVMatrix::putRow, bad row number %d",i);
  if(v.length() != width())
    PLERROR("In DiskVMatrix::putRow, length of v (%d) does not match matrix width (%d)",v.length(),width());
#endif

  unsigned char filenum;
  unsigned int position;
  indexf->seekg(3*sizeof(int) + i*(sizeof(unsigned char)+sizeof(unsigned int)));
  indexf->get((char&)filenum);
  indexf->read((char*)&position,sizeof(unsigned int));
  fstream* f = dataf[int(filenum)];
  f->seekp(position);
  binwrite_compressed(*f,v.data(), v.length());
}

void DiskVMatrix::appendRow(Vec v)
{
  if(!readwritemode)
    PLERROR("In DiskVMatrix::appendRow cannot append row in read only mode, set readwrite parameter to true when calling the constructor");
  if(v.length() != width())
    PLERROR("In DiskVMatrix::appendRow, length of v (%d) does not match matrix width (%d)",v.length(),width());

  int filenum = dataf.size()-1;
  fstream* f = dataf[filenum];
  f->seekp(0,ios::end);
  unsigned int position = f->tellp();
  if(position>500000000L)
    {
      filenum++;
      string filename = dirname + "/" + tostring(filenum) + ".data";
      f = new fstream();
      f->open(filename.c_str(), ios::in | ios::out);
      dataf.append(f);
      position = 0;
    }
  binwrite_compressed(*f,v.data(),v.length());
  indexf->seekp(0,ios::end);
  indexf->put((unsigned char)filenum);
  indexf->write((char*)&position,sizeof(unsigned int));
  length_++;
  indexf->seekp(sizeof(int),ios::beg);
  indexf->write((char*)&length_,sizeof(int));
  //  indexf.flush();
}

void DiskVMatrix::flush()
{
  int filenum = dataf.size()-1;
  fstream* f = dataf[filenum];
  f->flush();
  indexf->flush();
}

DiskVMatrix::~DiskVMatrix()
{
  for(int i=0; i<dataf.size(); i++)
    delete dataf[i];
  delete indexf;
}

IMPLEMENT_NAME_AND_DEEPCOPY(DiskVMatrix);


%> // end of namespcae PLearn
