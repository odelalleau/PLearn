// -*- C++ -*-

// ForwardVMatrix.cc
// Copyright (C) 2002 Pascal Vincent
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
 * $Id: ForwardVMatrix.cc,v 1.3 2003/03/19 23:15:23 jkeable Exp $
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#include "ForwardVMatrix.h"
#include "Kernel.h"
#include "Func.h"

namespace PLearn <%
using namespace std;

IMPLEMENT_NAME_AND_DEEPCOPY(ForwardVMatrix);

ForwardVMatrix::ForwardVMatrix()
{}

void ForwardVMatrix::setVMat(VMat the_vm)
{
  if(the_vm)
    {
      vm = the_vm;
      length_ = vm->length();
      width_ = vm->width();
      writable = vm->isWritable();
      setMtime(vm->getMtime());
      setMetaDataDir(vm->getMetaDataDir());
      setAlias(vm->getAlias());
      //  field_stats = vm->field_stats;
    }
  else
    {
      vm = VMat();
      length_ = 0;
      width_ = 0;
    }
}

string ForwardVMatrix::getValString(int col, real val) const
{ return vm->getValString(col, val); }
  
  
real ForwardVMatrix::getStringVal(int col, const string& str) const
{ return vm->getStringVal(col, str); }

  
string ForwardVMatrix::getString(int row,int col) const
{ return vm->getString(row,col); }

map<string,real> getStringMapping(int col) const;
{
  return vm->getStringMapping(col); 
}

void ForwardVMatrix::computeStats()
{ vm->computeStats(); }

  
void ForwardVMatrix::save(const string& filename) const
{ vm->save(filename); }

void ForwardVMatrix::savePMAT(const string& pmatfile) const
{ vm->savePMAT(pmatfile); }
void ForwardVMatrix::saveDMAT(const string& dmatdir) const
{ vm->saveDMAT(dmatdir); }
void ForwardVMatrix::saveAMAT(const string& amatfile) const
{ vm->saveAMAT(amatfile); }

  
real ForwardVMatrix::get(int i, int j) const
{ return vm->get(i, j); }

  
void ForwardVMatrix::put(int i, int j, real value)
{ vm->put(i, j, value); }
  
void ForwardVMatrix::getSubRow(int i, int j, Vec v) const
{ vm->getSubRow(i, j, v); }

void ForwardVMatrix::putSubRow(int i, int j, Vec v)
{ vm->putSubRow(i, j, v); }
      
void ForwardVMatrix::appendRow(Vec v)
{ vm->appendRow(v); }

void ForwardVMatrix::getRow(int i, Vec v) const
{ vm->getRow(i, v); }
void ForwardVMatrix::putRow(int i, Vec v)
{ vm->putRow(i, v); }
void ForwardVMatrix::fill(real value)
{ vm->fill(value); }
void ForwardVMatrix::getMat(int i, int j, Mat m) const
{ vm->getMat(i, j, m); }
void ForwardVMatrix::putMat(int i, int j, Mat m)
{ vm->putMat(i, j, m); }

  
void ForwardVMatrix::getColumn(int i, Vec v) const
{ vm->getColumn(i, v); }

Mat ForwardVMatrix::toMat() const
{ return vm->toMat(); }

  
  
void ForwardVMatrix::compacify()
{ vm->compacify(); }

  
void ForwardVMatrix::reset_dimensions() 
{ 
  vm->reset_dimensions();
  length_ = vm->length();
  width_ = vm->width();
}

VMat ForwardVMatrix::subMat(int i, int j, int l, int w)
{ return vm->subMat(i,j,l,w); }

real ForwardVMatrix::dot(int i1, int i2, int inputsize) const
{ return vm->dot(i1, i2, inputsize); }

  
real ForwardVMatrix::dot(int i, const Vec& v) const
{ return vm->dot(i,  v); }

  
  
void ForwardVMatrix::getRow(int i, VarArray& inputs) const
{ vm->getRow(i, inputs); }

void ForwardVMatrix::oldwrite(ostream& out) const
{ vm->oldwrite(out); }
void ForwardVMatrix::oldread(istream& in)
{ vm->oldread(in); }

void ForwardVMatrix::evaluateKernel(Ker ker, int v1_startcol, int v1_ncols, 
                                 const Vec& v2, const Vec& result, int startrow, int nrows) const
{ vm->evaluateKernel(ker, v1_startcol, v1_ncols, 
                     v2,  result, startrow, nrows); }

  
real ForwardVMatrix::evaluateKernelSum(Ker ker, int v1_startcol, int v1_ncols, 
                                    const Vec& v2, int startrow, int nrows, int ignore_this_row) const
{ return vm->evaluateKernelSum(ker, v1_startcol, v1_ncols, 
                               v2, startrow, nrows, ignore_this_row); }

  
  
real ForwardVMatrix::evaluateKernelWeightedTargetSum(Ker ker, int v1_startcol, int v1_ncols, const Vec& v2, 
                                                  int t_startcol, int t_ncols, Vec& targetsum, int startrow, int nrows, int ignore_this_row) const
{ return vm->evaluateKernelWeightedTargetSum(ker, v1_startcol, v1_ncols,  v2, 
                                             t_startcol, t_ncols, targetsum, startrow, nrows, ignore_this_row); }
  
   
/*!     This will return the Top N kernel evaluated values (between vmat (sub)rows and v2) and their associated row_index.
  Result is returned as a vector of length N of pairs (kernel_value,row_index)
  Results are sorted with largest kernel value first
*/
TVec< pair<real,int> > ForwardVMatrix::evaluateKernelTopN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                       const Vec& v2, int startrow, int nrows, int ignore_this_row) const
{ return vm->evaluateKernelTopN(N, ker, v1_startcol, v1_ncols, 
                                v2, startrow, nrows, ignore_this_row); }

  
  
TVec< pair<real,int> > ForwardVMatrix::evaluateKernelBottomN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                          const Vec& v2, int startrow, int nrows, int ignore_this_row) const
{ return vm->evaluateKernelBottomN(N, ker, v1_startcol, v1_ncols, 
                                   v2, startrow, nrows, ignore_this_row); }
void ForwardVMatrix::accumulateXtY(int X_startcol, int X_ncols, int Y_startcol, int Y_ncols, 
                                Mat& result, int startrow, int nrows, int ignore_this_row) const
{ vm->accumulateXtY(X_startcol, X_ncols, Y_startcol, Y_ncols, 
                    result, startrow, nrows, ignore_this_row); }

void ForwardVMatrix::accumulateXtX(int X_startcol, int X_ncols, 
                                Mat& result, int startrow, int nrows, int ignore_this_row) const
{ vm->accumulateXtX(X_startcol, X_ncols, 
                    result, startrow, nrows, ignore_this_row); }

  
void ForwardVMatrix::evaluateSumOfFprop(Func f, Vec& output_result, int nsamples)
{ vm->evaluateSumOfFprop(f, output_result, nsamples); }

void ForwardVMatrix::evaluateSumOfFbprop(Func f, Vec& output_result, Vec& output_gradient, int nsamples)
{ vm->evaluateSumOfFbprop(f, output_result, output_gradient, nsamples); }
 
void ForwardVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  VMatrix::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(vm, copies);
}

%> 


