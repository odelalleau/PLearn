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
   * $Id: MatIO.h,v 1.4 2003/07/31 05:23:56 chapados Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef MatIO_INC
#define MatIO_INC

#include "TMat.h"
#include "fileutils.h"
#include <stdlib.h>                          //!< for strtod

namespace PLearn <%
using namespace std;


//! convenience construction from string
//! allows to write things such as Mat m = newMat(2,2, "1 2   3 4")
inline Mat makeMat(int length, int width, const string& values)
{ Mat m(length,width); m << values; return m; } 

inline Vec makeVec(int length, const string& values)
{ Vec v(length); v << values; return v; }

//! Tries to guess the format... (quite dumb right now)
//! This is currently what the old constructor with string argument did
void loadMat(const string& filename, TMat<float>& mat);
void loadMat(const string& filename, TMat<double>& mat);
void loadVec(const string& filename, TVec<float>& vec);
void loadVec(const string& filename, TVec<double>& vec);

//! Old native PLearn binary format (.pmat)
void savePVec(const string& filename, const TVec<float>& vec);
void savePVec(const string& filename, const TVec<double>& vec);
void loadPVec(const string& filename, TVec<float>& vec);
void loadPVec(const string& filename, TVec<double>& vec);
void savePMat(const string& filename, const TMat<float>& mat);
void savePMat(const string& filename, const TMat<double>& mat);
void loadPMat(const string& filename, TMat<float>& mat);
void loadPMat(const string& filename, TMat<double>& mat);


//! WARNING: use only for float, double, and int types. Other type are not guaranteed to work

//! intelligent functions that will load a file in almost all ascii formats that ever existed in this lab
template<class T> void loadAscii(const string& filename, TMat<T>& mat, TVec<string>& fieldnames);
template<class T> void loadAscii(const string& filename, TMat<T>& mat);
template<class T> void saveAscii(const string& filename, const TMat<T>& mat, 
                                 const TVec<string>& fieldnames = TVec<string>() );
//! first number in file is length
template<class T> void saveAscii(const string& filename, const TVec<T>& vec);
template<class T> void loadAscii(const string& filename, TVec<T>& vec);

//! Format readable by gnuplot
void loadGnuplot(const string& filename, Mat& mat);
void saveGnuplot(const string& filename, const Vec& vec);
void saveGnuplot(const string& filename, const Mat& mat);

//!  Reads and writes an ascii file without the size header (assuming that the size(length() and width()) is set)
void loadAsciiWithoutSize(const string& filename, const Vec& vec);
void saveAsciiWithoutSize(const string& filename, const Vec& vec);
void loadAsciiWithoutSize(const string& filename, const Mat& mat);
void saveAsciiWithoutSize(const string& filename, const Mat& mat);

//!  SN Format
Mat loadSNMat(const string& filename);
Vec loadSNVec(const string& filename);
void saveSNMat(const string& filename, const Mat& mat);
void saveSNVec(const string& filename, const Vec& vec);

//!  Native AD format
Mat loadADMat(const string& filename);
Vec loadADVec(const string& filename);

/*!     UCI machine-learning-database format
      Format used for most of the UCI machine-learning-database.
      The missing value is represented with the '?' character
      in the source file, and with the MISSING_VALUE in the Mat.
      If some symbols are detected then integer codes are assigned to
      them (by sorting them for each symbolic column in lexicographic
      order). The *to_symbols table has one element per column,
      each of which is a table of strings. The number of strings
      (i.e., symbols) for each column is given in the table *to_n_symbols. 
*/
Mat loadUCIMLDB(const string& filename, char ****to_symbols=0, int **to_n_symbols=0);

/*!     STATLOG machine-learning-database-format
      Format used for most of the STATLOG machine-learning-database.
      The missing value is represented with the '?' character
      in the source file, and with the MISSING_VALUE in the Mat.
      If some symbols are detected then integer codes are assigned to
      them (by sorting them for each symbolic column in lexicographic
      order). The *to_symbols table has one element per column,
      each of which is a table of strings. The number of strings
      (i.e., symbols) for each column is given in the table *to_n_symbols. 
*/
Mat loadSTATLOG(const string& filename, char ****to_symbols=0, int **to_n_symbols=0);

/*!     read a file in JPEG format (read the RGB components).
    this will be resized to a (npixels x 3) matrix, where
    the (R,G,B) pixels are ordered by rows of the original image.
    To figure the actual image dimensions, the row size is also
    returned (so the number of columns is length()/row_size).
    An optional subsampling factor can be given (1,2,4 or 8) 
    The R,G,B components always range from 0 to 255.
*/
void loadJPEGrgb(const string& jpeg_filename, Mat& rgbmat, int& row_size, int scale = 1);


// intelligent function that will load a file in almost all ascii formats that ever existed in this lab
template<class T>
void loadAscii(const string& filename, TMat<T>& mat, TVec<string>& fieldnames)
{
  ifstream in(filename.c_str());
  if(!in)
    PLERROR("Could not open file %s for reading", filename.c_str());

  int length = -1;
  int width = -1;
  bool could_be_old_amat=true; // true while there is still a chance that this be an "old" amat format (length and width in first row with no starting ##)
  
  in >> ws;
  string line;

  while(in.peek()=='#')
  {
    getline(in, line);
    could_be_old_amat = false;
 
    unsigned int pos=line.find(":");
    if(pos!=string::npos)
    {
      string sub=line.substr(0,pos);
      if(sub=="#size") // we've found the dimension specification line
      {
        string siz=removeblanks((line.substr(pos)).substr(1));
        vector<string> dim = split(siz," ");
        if(dim.size()!=2)  PLERROR("I need exactly 2 dimensions for matrix");
        length = toint(dim[0]);
        width = toint(dim[1]);
      }
      else if(sub=="#") // we've found the fieldnames specification line
      {
        string fnl=line.substr(pos).substr(1);
        fieldnames = split(fnl," ");
        width=fieldnames.size();
      }              
    }
    in >> ws;
  }

  if(length==-1)  // still looking for size info...
  {
    string line;
    getNextNonBlankLine(in,line);
    if(line=="")
    {
      width=length=0;
      in.seekg(0);
      in.clear();
      could_be_old_amat=false; 
    }
    int nfields1 = split(line).size();
    getNextNonBlankLine(in,line);
    if(line=="")
    {
      length=1;
      width=nfields1;
      in.seekg(0);
      in.clear();
      could_be_old_amat=false; 
    }
    int nfields2 = split(line).size();
    int guesslength = countNonBlankLinesOfFile(filename);    
    real a = -1, b = -1;
    if(could_be_old_amat && nfields1==2) // could be an old .amat with first 2 numbers being length width
    {
      in.seekg(0);
      in.clear();
      in >> a >> b;
      if(guesslength == int(a)+1 && real(int(a))==a && real(int(b))==b && a>0 && b>0 && int(b)==nfields2) // it's clearly an old .amat
      {
        length = int(a);
        width = int(b);
      }
    }

    if(length==-1) // still don't know size info...
    {
      if(nfields1==nfields2) // looks like a plain ascii file
      {
        length=guesslength;
        if(width!=-1 && width!=nfields1)
        {
          PLWARNING("In loadAscii:  Number of fieldnames and width mismatch in file %s.  "
              "Replacing fieldnames by 'Field-0', 'Field-1', ...", filename.c_str());
          fieldnames.resize(nfields1);
          for(int i= 0; i < nfields1; ++i)
            fieldnames[i]= string("Field-") + tostring(i);
        }
        width = nfields1;
        in.seekg(0); 
        in.clear();
      }
      else if (real(int(a))==a && real(int(b))==b && a>0 && b>0 && int(b)==nfields2)
      {
        length = int(a);
        width = int(b);
      }
    }
  }

  if(length==-1)
    PLERROR("In loadAscii: trying to load but couldn't determine file format automatically for %s",filename.c_str());

  // We are now more careful about the possibility of the stream being in a
  // bad state.
  mat.resize(length,width);
  string inp_element;
  for(int i=0; i<length; i++)
  {
    T* mat_i = mat[i];
    skipBlanksAndComments(in);
    for(int j=0; j<width; j++) {
      // C99 strtod handles NAN's and INF's.
      if (in) {
        in >> inp_element;
        mat_i[j] = strtod(inp_element.c_str(), 0);
      }
      if (!in) {
        in.clear();
        mat_i[j] = MISSING_VALUE;
      }
    }
  }
}

template<class T>
void loadAscii(const string& filename, TVec<T>& vec)
{
  ifstream in(filename.c_str());
  if(!in)
    PLERROR("In loadAscii could not open file %s for reading",filename.c_str());
 
  int size = -1;
  in >> size;
  if (size<0 || size>1e10)
    PLERROR("In Vec::loadAscii the file is probably not in the right format: size=%d", size);
  vec.resize(size);
  typename TVec<T>::iterator it = vec.begin();
  typename TVec<T>::iterator itend = vec.end();

  // We are now more careful about the possibility of the stream being in a
  // bad state
  string inp_element;
  for(; it!=itend; ++it) {
    // C99 strtod handles NAN's and INF's.
    if (in) {
      in >> inp_element;
      *it = strtod(inp_element.c_str(), 0);
    }
    if (!in) {
      in.clear();
      *it = MISSING_VALUE;
    }
  }
}

template<class T> 
void saveAscii(const string& filename, const TMat<T>& mat, const TVec<string>& fieldnames)
{
  ofstream out(filename.c_str());
  if (!out)
   PLERROR("In saveAscii could not open file %s for writing",filename.c_str());

  out << "#size: "<< mat.length() << ' ' << mat.width() << endl;
  out.precision(15);
  if(fieldnames.size()>0)
  {
    out << "#: ";
    for(int k=0; k<fieldnames.size(); k++)
      //there must not be any space in a field name...
      out << space_to_underscore(fieldnames[k]) << ' ';
    out << endl;
  }

  for(int i=0; i<mat.length(); i++) 
  {
    const T* row_i = mat[i];
    for(int j=0; j<mat.width(); j++)
      out << row_i[j] << ' ';
    out << '\n';
  }
}
  
template<class T> 
void saveAscii(const string& filename, const TVec<T>& vec)
{
  ofstream out(filename.c_str());
  if (!out)
    PLERROR("In saveAscii: could not open file %s for writing",filename.c_str());

  out << vec.length() << endl;
  out.precision(15);

  typename TVec<T>::iterator it = vec.begin();
  typename TVec<T>::iterator itend = vec.end();
  for(; it!=itend; ++it)
    out << *it << ' ';
  out << endl;
}

template<class T>
void loadAscii(const string& filename, TMat<T>& mat)
{
  TVec<std::string> fn;
  loadAscii(filename,mat,fn);
}

%> // end of namespace PLearn


#endif
