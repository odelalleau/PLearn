// -*- C++ -*-

// VVMatrix.h
// Copyright (C) 2002 Pascal Vincent and Julien Keable
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
   * $Id: VVMatrix.h,v 1.1 2002/10/03 07:35:28 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef VVMatrix_INC
#define VVMatrix_INC

#include "VMat.h"

namespace PLearn <%
using namespace std;

//! this class is a wrapper for a .vmat VMatrix.
class VVMatrix: public VMatrix
{
  typedef VMatrix inherited;
  
protected:
  string the_filename;
  string code;
  VMat the_mat;

  //! loads a .vmat
  static VMat createPreproVMat(const string & filename);
  static void generateVMatIndex(VMat source, const string& meta_data_dir,
                                const string & filename, time_t date_of_code,const string & in, 
                                unsigned int idx_prefilter,unsigned int cidx_prefilter,
                                unsigned int idx_postfilter,unsigned int cidx_postfilter,
                                unsigned int idx_process,unsigned int cidx_process,
                                unsigned int idx_shuffle,unsigned int cidx_shuffle,
                                unsigned int idx_join,unsigned int cidx_join);
  static void processJoinSection(const vector<string> & code, VMat & tmpsource);
  static vector<vector<string> > extractSourceMatrix(const string & str,const string& filename);
  static void generateFilterIndexFile(VMat source, const string & code, const string& ivfname);
  
public:
  virtual void build();
  void build_();
  
  const string & getCode(){return code;}

  //! get "real" date of .vmat according to its dependencies
  static time_t getDateOfVMat(const string& filename);
  
  //! returns the source VMat after filtering it with 'code' . ivfname is the name of the index file and
  //! date_of_code is the code last modification date. If existant, index file is used as is, provided 
  //! its date is > the latest date of all files (code + all its dependencies)
  static VMat buildFilteredVMatFromVPL(VMat source, const string & code, const string& ivfname, time_t date_of_code);
  
  //! tells if the .vmat is precomputed and if that precomputed data is more recent than the .vmat file
  bool isPrecomputedAndUpToDate();

  string getPrecomputedDataName();

  VVMatrix(const string& filename_):the_filename(filename_){build_();}
  VVMatrix(){};
  
  // ****************************************************
  // POSSIBLE "cache" IMPROVEMENT.. need to check it out with pascal
  // would it be a good idea to systematically wrap "the_mat" with a RowBufferedVMatrix  ?

  virtual string getValString(int col, real val) const;
  virtual real getStringVal(int col, const string & str) const;
  virtual string getString(int row,int col) const;

  virtual real get(int i, int j) const {return the_mat->get(i,j);}
  virtual void getSubRow(int i, int j, Vec v) const {the_mat->getSubRow(i,j,v);}
  static void declareOptions(OptionList & ol);
  DECLARE_NAME_AND_DEEPCOPY(VVMatrix);
};

DECLARE_OBJECT_PTR(VVMatrix);

%> // end of namespace PLearn


#endif // VVMatrix_INC
