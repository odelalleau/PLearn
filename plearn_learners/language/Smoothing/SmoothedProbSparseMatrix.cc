// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Christopher Kermorvant
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

/*! \file SmoothedProbSparseMatrix.cc */

#include "SmoothedProbSparseMatrix.h"

namespace PLearn {

SmoothedProbSparseMatrix::SmoothedProbSparseMatrix(int n_rows, int n_cols, string name, int mode, bool double_access)
  :ProbSparseMatrix(n_rows, n_cols, name, mode, double_access)
{
  smoothingMethod = 0;
}

// Normalize with Laplace smoothing : 
// Warning : resulting matrices are _NOT_ normalized as such !!!
// if P(x|y)=0 in the matrix, it must be set to 
//     1/(getHeight+sumCol(y)) in  COLUMN_WISE mode
//     1/(getWidth+sumRow(y)) in COLUMN_WISE
void SmoothedProbSparseMatrix::normalizeCondLaplace(ProbSparseMatrix& nXY, bool clear_nXY)
{
  int nXY_height = nXY.getHeight();
  int nXY_width = nXY.getWidth();
  smoothingMethod = 1;
  if (mode == ROW_WISE && (nXY.getMode() == ROW_WISE || nXY.isDoubleAccessible()))
  {
    clear();
    normalizationSum.resize(nXY_height);
    for (int i = 0; i < nXY_height; i++)
    {
      // Laplace smoothing normalization
      real sum_row_i = nXY.sumRow(i)+nXY_width;
      // Store normalization sum
      normalizationSum[i] =  sum_row_i;
      map<int, real>& row_i = nXY.getRow(i);
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        real nij = it->second;
	// Laplace smoothing
	if (nij > 0.0){
	  real pij = (nij +1)/ sum_row_i;
	  set(i, j, pij);
	}
      }
    }
    if (clear_nXY)
      nXY.clear();
  } else if (mode == COLUMN_WISE && (nXY.getMode() == COLUMN_WISE || nXY.isDoubleAccessible())){
    clear();
    normalizationSum.resize(nXY_width);
    for (int j = 0; j < nXY_width; j++){
      // Laplace smoothing normalization
      real sum_col_j = nXY.sumCol(j)+nXY_height;
      // Store normalization sum
      normalizationSum[j] =  sum_col_j;
      map<int, real>& col_j = nXY.getCol(j);
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
        int i = it->first;
        real nij = it->second;
	// Laplace smoothing
        if (nij > 0.0){
	  set(i, j, (nij+1) / sum_col_j);
	}
      }
    }
    if (clear_nXY)
      nXY.clear();
  } else{
    PLERROR("pXY and nXY accessibility modes must match");
  }
}

void SmoothedProbSparseMatrix::normalizeCondBackoff(ProbSparseMatrix& nXY, real disc, Vec& bDist, bool clear_nXY,bool shadow)
{
  // disc is the percent of minial value to discount : discval = minvalue*disc
  // In case of integer counts, the discounted value is 1*disc = disc
 
  int i,j;
  real nij,pij;
  real minval,discval;
  int nXY_height = nXY.getHeight();
  int nXY_width = nXY.getWidth();
  // Shadowing or non shadowing smoothing
  if(shadow){
    smoothingMethod = 2;
  }else{
    smoothingMethod = 3;
  }


  
  
  // Copy Backoff Distribution
  backoffDist.resize(bDist.size());
  backoffDist << bDist;
  if (mode == ROW_WISE && (nXY.getMode() == ROW_WISE || nXY.isDoubleAccessible())){
    clear();
    if (backoffDist.size()!=nXY_width)PLERROR("Wrong dimension for backoffDistribution");
    normalizationSum.resize(nXY_height);normalizationSum.clear();
    backoffNormalization.resize(nXY_height);backoffNormalization.clear();
    discountedMass.resize(nXY_height);discountedMass.clear();
    for (int i = 0; i < nXY_height; i++){
      // normalization
      real sum_row_i = nXY.sumRow(i);
      if (sum_row_i==0){
      // if there is no count in this column : uniform distribution	
	normalizationSum[j]=nXY_width;
      }else{
	// Store normalization sum
	normalizationSum[i] =  sum_row_i;
      }
      backoffNormalization[i]= 1.0;
      map<int, real>& row_i = nXY.getRow(i);
      // compute minial value
      minval=FLT_MAX;
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
	minval=	it->second<minval?it->second:minval;
      }
      discval = minval*disc;
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
        j = it->first;
        nij = it->second;
	if(nij>discval){
	  discountedMass[i]+=discval;
	  // Discount
	  pij =  (nij -discval)/ sum_row_i;
	  if (pij<0) PLERROR("modified count < 0 in Backoff  Smoothing SmoothedProbSparseMatrix %s",nXY.getName().c_str());
	  // update backoff normalization factor
	  backoffNormalization[i]-= backoffDist[j];
	}else{
	   pij =  nij/ sum_row_i;
	}
	// Set modified count
	set(i, j,pij);
	
      }
      if(discountedMass[i]==0)PLERROR("Discounted mass is null but count are not null in %s line %d",nXY.getName().c_str(),i);
    }
    
    if (clear_nXY)nXY.clear();
  } else if (mode == COLUMN_WISE && (nXY.getMode() == COLUMN_WISE || nXY.isDoubleAccessible())){
    clear();
    normalizationSum.resize(nXY_width);normalizationSum.clear();
    backoffNormalization.resize(nXY_width);backoffNormalization.clear();
    discountedMass.resize(nXY_width);discountedMass.clear();
    for ( j = 0; j < nXY_width; j++){
      // normalization
      real sum_col_j = nXY.sumCol(j);
      if (sum_col_j==0){
	// if there is no count in this column : uniform distribution
	normalizationSum[j]=nXY_height;
	continue;
      }else{
      // Store normalization sum
      normalizationSum[j] =  sum_col_j;
      }
      
      backoffNormalization[j]= 1.0;
      map<int, real>&  col_j = nXY.getCol(j);
      // compute minimal value
      minval=FLT_MAX;
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
	minval=	(it->second<minval && it->second!=0) ?it->second:minval;
      }
      discval = minval*disc;
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
	i = it->first;
	nij = it->second;
	if(nij>discval){
	  discountedMass[j]+=discval;
	  // Discount
	  pij = (nij -discval)/ sum_col_j;
	  if (pij<0) PLERROR("modified count < 0 in Backoff  Smoothing SmoothedProbSparseMatrix %s : i=%d j=%d p=%f",nXY.getName().c_str(),i,j,pij);
	  // update backoff normalization factor
	  backoffNormalization[j]-=backoffDist[i];
	}else{
	  pij = nij / sum_col_j;
	}
	if(pij<=0 || pij>1) PLERROR("Invalide smoothed probability %f in %s",pij,nXY.getName().c_str());
	set(i, j, pij);
      }
      if(discountedMass[j]==0){
	PLERROR("Discounted mass is null but count are not null in %s col %d",nXY.getName().c_str(),j);
      }
    }
    
    if (clear_nXY)nXY.clear();
  }else{
    PLERROR("pXY and nXY accessibility modes must match");
  }
}

real SmoothedProbSparseMatrix::get(int i, int j)
{
#ifdef BOUNDCHECK      
  if (i < 0 || i >= height || j < 0 || j >= width)
    PLERROR("SmoothedProbSparseMatrix.get : out-of-bound access to (%d, %d), dims = (%d, %d)", i, j, height, width);
#endif

  // If the matrix is not yet smoothed
  if(smoothingMethod==0){
    return ProbSparseMatrix::get(i,j);
  }
  if (mode == ROW_WISE){
    map<int, real>& row_i = rows[i];
    map<int, real>::iterator it = row_i.find(j);
    if (it == row_i.end()){
      // if no data in this column : uniform distribution
      if (discountedMass[i]==0)return 1/normalizationSum[i];
      // Laplace smoothing
      if(smoothingMethod==1)return 1/normalizationSum[i];
      // Backoff smoothing
      if(smoothingMethod==2)return discountedMass[i]*backoffDist[j]/(normalizationSum[i]* backoffNormalization[i]);
      // Non-shadowing backoff
      if(smoothingMethod==3)return discountedMass[i]*backoffDist[j]/(normalizationSum[i]);
    }else{
      // Non-shadowing backoff
      if(smoothingMethod==3)return it->second+discountedMass[i]*backoffDist[j]/(normalizationSum[i]);
      return it->second;
    }
  } else{
    map<int, real>& col_j = cols[j];
    map<int, real>::iterator it = col_j.find(i);
    if (it == col_j.end()){
      // if no data in this column : uniform distribution
      if (discountedMass[j]==0)return 1/normalizationSum[j];
      // Laplace smoothing
      if(smoothingMethod==1)return 1/normalizationSum[j];
      // Backoff smoothing
      if(smoothingMethod==2)return discountedMass[j]*backoffDist[i]/(normalizationSum[j]*backoffNormalization[j]);
      // Non-shadowing backoff
      if(smoothingMethod==3)return discountedMass[j]*backoffDist[i]/(normalizationSum[j]);
    }else{
      // Non-shadowing backoff
      if(smoothingMethod==3)return it->second+discountedMass[j]*backoffDist[i]/(normalizationSum[j]);
      return it->second;
    }
  }
  return;
}


bool SmoothedProbSparseMatrix::checkCondProbIntegrity()
{
  real sum = 0.0;
  real backsum;
  int null_size;
  if (normalizationSum.size()==0)return false;
  //cout << " CheckCondIntegrity : mode " <<smoothingMethod; 
  if (mode == ROW_WISE){
    for (int i = 0; i < height; i++){
      map<int, real>& row_i = rows[i];
      
      if(smoothingMethod==1)sum = (width-row_i.size())/normalizationSum[i];
      else if(smoothingMethod==2||smoothingMethod==3)sum = discountedMass[i]/normalizationSum[i];
       backsum=0;
      
    
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
	sum += it->second;
	if (smoothingMethod==2)backsum +=backoffDist[it->first];
      }
      if (smoothingMethod==2){
	if (fabs(1.0 - backsum - backoffNormalization[i]) > 1e-4 ){
	  cout << " Inconsistent backoff normalization  " << i << " : "<<backoffNormalization[i]<< " "<< backsum;
	  return false;
	}	
      }
      if (fabs(sum - 1.0) > 1e-4 && (sum > 0.0))
	cout << " Inconsistent  line " << i << " sum = "<< sum;
	return false;
    }
    return true;
  }else{
    for (int j = 0; j < width; j++){
      map<int, real>& col_j = cols[j];
      if(smoothingMethod==1)sum = (height-col_j.size())/normalizationSum[j];
      else if(smoothingMethod==2 || smoothingMethod==3)sum = discountedMass[j]/normalizationSum[j];
      
      backsum=0;
      
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
	sum += it->second;
	if (smoothingMethod==2)backsum +=backoffDist[it->first];
      }
      if (smoothingMethod==2){
	if (fabs(1.0 - backsum - backoffNormalization[j]) > 1e-4 ){
	  cout << " Inconsistent backoff normalization  " << j << " : "<<backoffNormalization[j]<< " "<< backsum;
	  return false;
	}	
      }
      if(fabs(sum - 1.0) > 1e-4 && (sum > 0.0)){
	cout << " Inconsistent  column " << j << " sum = "<< sum;
        return false;
      }
    }
    return true;
  }
}

void  SmoothedProbSparseMatrix::write(PStream& out) const
{
  ProbSparseMatrix::write(out);
  string class_name = getClassName();
  switch(out.outmode)
    {
    case PStream::raw_ascii :
    case PStream::pretty_ascii :
      PLERROR("raw/pretty_ascii write not implemented in %s", class_name.c_str());
      break;        
    case PStream::raw_binary :
      PLERROR("raw_binary write not implemented in %s", class_name.c_str());
      break;        
    case PStream::plearn_binary :
    case PStream::plearn_ascii :
      out.write(class_name + "(");
      out << smoothingMethod;
      out << normalizationSum;
      out << backoffDist;
      out << backoffNormalization;
      out << discountedMass;
      out.write(")\n");
      break;
    default:
      PLERROR("unknown outmode in %s::write(PStream& out)", class_name.c_str());
      break;
    }
}

void  SmoothedProbSparseMatrix::read(PStream& in)
{
  ProbSparseMatrix::read(in);
  string class_name = getClassName();
  switch (in.inmode)
    {
    case PStream::raw_ascii :
      PLERROR("raw_ascii read not implemented in %s", class_name.c_str());
      break;
    case PStream::raw_binary :
      PLERROR("raw_binary read not implemented in %s", class_name.c_str());
      break;
    case PStream::plearn_ascii :
    case PStream::plearn_binary :
      {
	in.skipBlanksAndCommentsAndSeparators();
	string word(class_name.size() + 1, ' ');
	for (unsigned int i = 0; i < class_name.size() + 1; i++)
	  in.get(word[i]);
	if (word != class_name + "(")
	  PLERROR("in %s::(PStream& in), '%s' is not a proper header", class_name.c_str(), word.c_str());
	in.skipBlanksAndCommentsAndSeparators();
	in >> smoothingMethod;
	in.skipBlanksAndCommentsAndSeparators();
	in >> normalizationSum;
	in.skipBlanksAndCommentsAndSeparators();
	in >> backoffDist;
	in.skipBlanksAndCommentsAndSeparators();
	in >> backoffNormalization;
	in.skipBlanksAndCommentsAndSeparators();
	in >> discountedMass;
	in.skipBlanksAndCommentsAndSeparators();
	int c = in.get();
	if(c != ')')
	  PLERROR("in %s::(PStream& in), expected a closing parenthesis, found '%c'", class_name.c_str(), c);
      }
    break;
    default:
      PLERROR("unknown inmode in %s::write(PStream& out)", class_name.c_str());
      break;
    }
}

void ComplementedProbSparseMatrix::complement(ProbSparseMatrix& nXY, bool clear_nXY)
{
  int nXY_height = nXY.getHeight();
  int nXY_width = nXY.getWidth();

  rowSum.resize(nXY_height);
  columnSum.resize(nXY_width);
  if (mode == ROW_WISE && (nXY.getMode() == ROW_WISE || nXY.isDoubleAccessible())){
    for (int i = 0; i < nXY_height; i++){
      map<int, real>& row_i = nXY.getRow(i);
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
	rowSum[i]+=it->second;
	columnSum[it->first]+=it->second;
	grandSum+=it->second;
      }
    }
    
    if (clear_nXY)
      nXY.clear();
  } else if (mode == COLUMN_WISE && (nXY.getMode() == COLUMN_WISE || nXY.isDoubleAccessible())){
    
    for (int j = 0; j < nXY_width; j++){
      map<int, real>& col_j = nXY.getCol(j);
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
	rowSum[it->first]+=it->second;
	columnSum[j]+=it->second;
	grandSum+=it->second;
      }
    }
    if (clear_nXY)
      nXY.clear();
  } else{
    PLERROR("pXY and nXY accessibility modes must match");
  }
}

real ComplementedProbSparseMatrix::get(int i, int j)
{
#ifdef BOUNDCHECK      
  if (i < 0 || i >= height || j < 0 || j >= width)
    PLERROR("SmoothedProbSparseMatrix.get : out-of-bound access to (%d, %d), dims = (%d, %d)", i, j, height, width);
#endif
  if (mode == ROW_WISE){
    map<int, real>& row_i = rows[i];
    map<int, real>::iterator it = row_i.find(j);
    if (it == row_i.end()){
      return rowSum[j]/(grandSum-columnSum[i]);
    }else{
      return (rowSum[j]-it->second)/(grandSum-columnSum[i]);
    }
  } else{
    map<int, real>& col_j = cols[j];
    map<int, real>::iterator it = col_j.find(i);
    if (it == col_j.end()){
      return columnSum[j]/(grandSum-rowSum[i]);
    }else{

      return (columnSum[j]-it->second)/(grandSum-rowSum[i]);
    }
  }
}

bool ComplementedProbSparseMatrix::checkCondProbIntegrity()
{
  real sum = 0.0;

  int null_size;
  // TO BE DONE

  if (mode == ROW_WISE){
    for (int i = 0; i < height; i++){
      map<int, real>& row_i = rows[i];
      
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
	
      }
      
      if (fabs(sum - 1.0) > 1e-4 && (sum > 0.0))
	cout << " Inconsistent  line " << i << " sum = "<< sum;
	return false;
    }
    return true;
  }else{
    for (int j = 0; j < width; j++){
      map<int, real>& col_j = cols[j];
      
      for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
	
      }
      
      if(fabs(sum - 1.0) > 1e-4 && (sum > 0.0)){
	cout << " Inconsistent  column " << j << " sum = "<< sum;
        return false;
      }
    }
    return true;
  }
}


}
