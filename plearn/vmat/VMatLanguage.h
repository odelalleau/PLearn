// -*- C++ -*-

// PLearn ("A C++ Machine Learning Library")
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
   * $Id: VMatLanguage.h,v 1.8 2003/10/29 16:55:49 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef VMatLanguage_INC
#define VMatLanguage_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"
#include "RealMapping.h"


namespace PLearn <%
using namespace std;

/* The VMatLanguage object contains a VPL bytecode program that can be applied to a row of a VMat.
   
   The VPL Language is described in /u/jkeable/visu/help/vpl.html. 

   By the way, the define statements in the VPL language can be recursive. Cool!

*/

  class VMatLanguage: public Object
  {
  private:
    VMat vmsource;
    TVec<int> program; 
    TVec<RealMapping> mappings;
    mutable Vec pstack;
    mutable Vec myvec;

    // maps opcodes strings to opcodes numbers
    static map<string, int> opcodes;
    
    //! builds the opcodes map if it does not already exist
    static void build_opcodes_map();

    // generates bytecode from a preprocessed text sourcecode
    void generateCode(const string& processed_sourcecode);
    void generateCode(istream& processed_sourcecode);

    // this function takes raw VPL code and returns the preprocessed sourcecode 
    // along with the defines and the fieldnames it generated
    void preprocess(istream& in, map<string, string>& defines, string& processed_sourcecode,
                           vector<string>& fieldnames);
    
public:
    VMatLanguage():vmsource(Mat()) { build_opcodes_map(); }
    VMatLanguage(VMat vmsrc):vmsource(vmsrc) { build_opcodes_map(); }
    typedef Object inherited;
    PLEARN_DECLARE_OBJECT(VMatLanguage);

    //! Executes the program on the srcvec, copy resulting stack to result
    //! rowindex is only there for instruction 'rowindex' that pushes it on the stack
    void run(const Vec& srcvec, const Vec& result, int rowindex=-1) const;

    //! Gets the row with the given rowindex from the vmsource VMat
    //! and applies program to it.
    void run(int rowindex, const Vec& result) const;

    inline void setSource(VMat the_source) 
    { 
      vmsource = the_source;
      // program must be compiled with the right source in place...
      program.resize(0);
    }

    // from the outside, use the next 3 high-level functions
    /////////////////////////////////////////////////////////

    //! takes a string, filename, or istream and generate the bytecode from it
    //! On exit, fieldnames will contain fieldnames of the resulting matrix
    //! The source vmat must be set before compiling a VPL program. 
    void compileStream(istream &in, vector<string>& fieldnames);
    void compileString(const string & code, vector<string>& fieldnames);
    void compileFile(const string & filename, vector<string>& fieldnames);
    
    int pstackSize() const {return pstack.size();}

    // set this to true when debugging. Will dump the preprocessed code when you call compilexxxx
    static bool output_preproc;
  };

/*
  PreprocessingVMatrix : a VMatLanguage derivated product, a la sauce VMat.
  Construct a PreprocessingVMatrix by specifying the source VMat and a string containing the VPL code to process each row
*/

  class PreprocessingVMatrix: public RowBufferedVMatrix
  {
    PLEARN_DECLARE_OBJECT(PreprocessingVMatrix);

  protected:
    VMat source;
    VMatLanguage program;
    Vec sourcevec;
    vector<string> fieldnames;
    

  public:
    PreprocessingVMatrix(){}
    PreprocessingVMatrix(VMat the_source, const string& program_string)
      :source(the_source),program(the_source)
    {
      program.compileString(program_string,fieldnames);
  
      fieldinfos.resize(fieldnames.size());
      for(unsigned int j=0; j<fieldnames.size(); j++)
        fieldinfos[j] = VMField(fieldnames[j]);

      sourcevec.resize(source->width());
      width_ = fieldnames.size();
      length_ = source.length();
      
    }

    virtual void getRow(int i, Vec v) const;

  };


  time_t getDateOfCode(const string& codefile);

%> // end of namespace PLearn

#endif


