// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1999-2002 Christian Jauvin
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

/*! \file ProbSparseMatrix.cc */

#include "ProbSparseMatrix.h"

namespace PLearn {

ProbSparseMatrix::ProbSparseMatrix(int n_rows, int n_cols, string name, int mode, bool double_access) : DoubleAccessSparseMatrix<real>(n_rows, n_cols, name, mode, double_access)
{
}

void ProbSparseMatrix::incr(int i, int j, real inc, bool warning)
{
    if (inc <= 0.0 && warning)
        PLWARNING("incrementing value by: %g", inc);
    DoubleAccessSparseMatrix<real>::incr(i, j, inc);
}

void ProbSparseMatrix::set(int i, int j, real value, bool warning)
{
    if (value <= 0.0 && warning)
        PLWARNING("setting value: %g", value);
    DoubleAccessSparseMatrix<real>::set(i, j, value);
}

bool ProbSparseMatrix::checkCondProbIntegrity()
{
    real sum = 0.0;
    if (mode == ROW_WISE)
    {
        for (int i = 0; i < height; i++)
        {
            map<int, real>& row_i = rows[i];
            sum = 0.0;
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
                sum += it->second;
            if (fabs(sum - 1.0) > 1e-4 && (sum > 0.0))
                return false;
        }
        return true;
    } else
    {
        for (int j = 0; j < width; j++)
        {
            map<int, real>& col_j = cols[j];
            sum = 0.0;
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
                sum += it->second;
            if (fabs(sum - 1.0) > 1e-4 && (sum > 0.0))
                return false;
        }
        return true;
    }
}

bool ProbSparseMatrix::checkJointProbIntegrity()
{
    return (fabs(sumOfElements() - 1.0) > 1e-4);
}

void ProbSparseMatrix::normalizeCond(ProbSparseMatrix& nXY, bool clear_nXY)
{
    if (mode == ROW_WISE && (nXY.getMode() == ROW_WISE || nXY.isDoubleAccessible()))
    {
        clear();
        int nXY_height = nXY.getHeight();
        for (int i = 0; i < nXY_height; i++)
        {
            real sum_row_i = nXY.sumRow(i);
            map<int, real>& row_i = nXY.getRow(i);
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
            {
                int j = it->first;
                real nij = it->second;
                real pij = nij / sum_row_i;
                if (pij > 0.0)
                    set(i, j, pij);
            }
        }
        if (clear_nXY)
            nXY.clear();
    } else if (mode == COLUMN_WISE && (nXY.getMode() == COLUMN_WISE || nXY.isDoubleAccessible()))
    {
        clear();
        int nXY_width = nXY.getWidth();
        for (int j = 0; j < nXY_width; j++)
        {
            real sum_col_j = nXY.sumCol(j);
            map<int, real>& col_j = nXY.getCol(j);
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
            {
                int i = it->first;
                real nij = it->second;
                real pij = nij / sum_col_j;
                if (pij > 0.0)
                    set(i, j, pij);
            }
        }
        if (clear_nXY)
            nXY.clear();
    } else
    {
        PLERROR("pXY and nXY accessibility modes must match");
    }
}

void ProbSparseMatrix::normalizeCond()
{
    if (mode == ROW_WISE)
    {
        for (int i = 0; i < height; i++)
        {
            real sum_row_i = sumRow(i);
            map<int, real>& row_i = rows[i];
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
            {
                int j = it->first;
                real nij = it->second;
                real pij = nij / sum_row_i;
                if (pij > 0.0)
                    set(i, j, pij);
            }
        }
    } else
    {
        for (int j = 0; j < width; j++)
        {
            real sum_col_j = sumCol(j);
            map<int, real>& col_j = cols[j];
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
            {
                int i = it->first;
                real nij = it->second;
                real pij = nij / sum_col_j;
                if (pij > 0.0)
                    set(i, j, pij);
            }
        }
    }
}

//! Normalize the matrix nXY as a joint probability matrix 
//! /f$ \sum_i \sum_j x_{ij} =1 /f$
void ProbSparseMatrix::normalizeJoint(ProbSparseMatrix& nXY, bool clear_nXY)
{
    clear();
    real sum_nXY = nXY.sumOfElements();
    if (nXY.getMode() == ROW_WISE)
    {
        int nXY_height = nXY.getHeight();
        for (int i = 0; i < nXY_height; i++)
        {
            map<int, real>& row_i = nXY.getRow(i);
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
            {
                int j = it->first;
                real nij = it->second;
                real pij = nij / sum_nXY;
                if (pij > 0.0)
                    set(i, j, pij);
            }
        }
    } else if (nXY.getMode() == COLUMN_WISE)
    {
        int nXY_width = nXY.getWidth();
        for (int j = 0; j < nXY_width; j++)
        {
            map<int, real>& col_j = nXY.getCol(j);
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
            {
                int i = it->first;
                real nij = it->second;
                real pij = nij / sum_nXY;
                if (pij > 0.0)
                    set(i, j, pij);
            }
        }
    }
    if (clear_nXY)
        nXY.clear();
}

void ProbSparseMatrix::normalizeJoint()
{
    if (mode == ROW_WISE)
    {
        for (int i = 0; i < height; i++)
        {
            map<int, real>& row_i = rows[i];
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
            {
                int j = it->first;
                real nij = it->second;
                real pij = nij / sumOfElements();
                if (pij > 0.0)
                    set(i, j, pij);
            }
        }
    } else
    {
        for (int j = 0; j < width; j++)
        {
            map<int, real>& col_j = cols[j];
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it)
            {
                int i = it->first;
                real nij = it->second;
                real pij = nij / sumOfElements();
                if (pij > 0.0)
                    set(i, j, pij);
            }
        }
    }
}

//! euclidian distance between two sparse matrices (of same dimensions)
real ProbSparseMatrix::euclidianDistance( ProbSparseMatrix &p)
{
    real  distance=0;
    real diff;
    if(p.getHeight()!=height || p.getWidth() != width) PLERROR("euclidianDistance: matrix dimensions do not match ");
    if (mode == ROW_WISE){
        // go thru the first matrix
        for (int i = 0; i < height; i++){
            map<int, real>& row_i = rows[i];
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
                diff = it->second-p.get(i, it->first);
                distance +=sqrt(diff*diff);
            }
        }
        // go thru the second one
        for (int i = 0; i < p.getHeight(); i++){
            map<int, real>& row_i = p.getRow(i);
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
                // if the value exists in the first matrix, it has already been included in the distance
                if(exists(i,it->first))continue;
                // no value in the first matrix
                diff = p.get(i,it->first);
                distance +=sqrt(diff*diff);
            }
        }
    }else{
        // go thru the first matrix
        for (int j = 0; j < width; j++){
            map<int, real>& col_j = cols[j];
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
                diff = it->second-p.get(it->first,j);
                distance +=sqrt(diff*diff);
            }
        }
        // go thru the second one
        for (int j = 0; j < width; j++){
            map<int, real>& col_j = p.getCol(j);
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
                // if the value exists in the first matrix, it has already been included in the distance
                if(exists(it->first,j))continue;
                // no value in the first matrix
                diff = p.get(it->first,j);
                distance +=sqrt(diff*diff);
            }
        }
    }
    return(distance);
}



void ProbSparseMatrix::iterativeProportionalFittingStep( ProbSparseMatrix& p,Vec& lineMarginal, Vec& colMarginal)
    // one step of proportional iterative fitting on the matrix with lineMarginal and colMarginal 
{
    real newVal;
    real sum_row_i;
    real sum_col_j;
  
    if(p.getHeight()!=lineMarginal.size() || p.getWidth() != colMarginal.size()) PLERROR("iterativeProportionalFittingStep: matrix dimension does not match marginal vectors dimensions");
    if(p.getHeight()!=height || p.getWidth() != width) PLERROR("iterativeProportionalFittingStep: new matrix dimension does not match old matrix dimensions");
    if(p.mode!=mode) PLERROR("iterativeProportionalFittingStep: Matrices access mode must match");
    if (mode == ROW_WISE){
        Vec sum_col(width);
        // First pass
        for (int i = 0; i < height; i++){
            map<int, real>& row_i = p.getRow(i);
            sum_row_i = p.sumRow(i);
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
                if(sum_row_i==0)PLERROR("iterativeProportionalFittingStep: line %d is empty",i);
                newVal= it->second*lineMarginal[i]/sum_row_i;
                // store sum of column for next step
                sum_col[it->first]+=newVal;
                set(i,it->first,newVal);
            }
        }
        // Second Pass
        for (int i = 0; i < height; i++){
            // we use the values set in the matrix at the previous stage
            map<int, real>& row_i = rows[i];
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
                if(sum_col[it->first]==0)PLERROR("iterativeProportionalFittingStep: column %d is empty",i);
                newVal= it->second*colMarginal[it->first]/sum_col[it->first];
                set(i,it->first,newVal);
            }
        }
   
    }else{
        Vec sum_row(height);
        for (int j = 0; j < width; j++){
            map<int, real>& col_j = p.getCol(j);
            sum_col_j = p.sumCol(j);
            if( col_j.begin()!= col_j.end())cout << " " << colMarginal[j]/sum_col_j<< ":";
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
                if(sum_col_j==0){
                    PLWARNING("iterativeProportionalFittingStep: column %d is empty",j);
                    continue;
                }
                newVal= it->second*colMarginal[j]/sum_col_j;
                cout << " " <<it->second<<"="<<newVal;
                sum_row[it->first]+=newVal;
                set(it->first,j,newVal);
            }
            if( col_j.begin()!= col_j.end())cout << endl;
        }
        cout << endl;
        // Second Pass
        for (int j = 0; j < width; j++){
            // we use the values set in the matrix at the previous stage
            map<int, real>& col_j = cols[j];
            //      cout << " " << lineMarginal[it->first]/sum_row[it->first]<< ":";
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
                if(sum_row[it->first]==0){
                    PLWARNING("iterativeProportionalFittingStep: line %d is empty",it->first);
                    continue;
                }
                newVal= it->second*lineMarginal[it->first]/sum_row[it->first];
                cout << " " <<it->second<<"="<<newVal;
                set(it->first,j,newVal);
            }
            cout << endl;
        }
    }
}

void ProbSparseMatrix::add( ProbSparseMatrix& p, ProbSparseMatrix& q)
{
    real val;
    if(p.getHeight()!=q.getHeight() || p.getWidth() != q.getWidth()) PLERROR("euclidianDistance: matrix dimensions do not match ");
    if (mode == ROW_WISE){
        // go thru the first matrix
        for (int i = 0; i < p.getHeight(); i++){
            map<int, real>& row_i = p.getRow(i);
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
                val = it->second+q.get(i,it->first);
                set(i,it->first,val);
            }
        }
        // go thru the second one
        for (int i = 0; i < q.getHeight(); i++){
            map<int, real>& row_i = q.getRow(i);
            for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it){
                // if the value exists in the first matrix, it has already been seen
                if(!p.exists(i,it->first)){
                    // no value in the first matrix
                    set(i,it->first,it->second);
                }
            }
        }
    }else{
        // go thru the first matrix
        for (int j = 0; j < p.getWidth(); j++){
            map<int, real>& col_j = p.getCol(j);
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
                val = it->second+q.get(it->first,j);
                set(it->first,j,val);
            }
        }
        // go thru the second one
        for (int j = 0; j < q.getWidth(); j++){
            map<int, real>& col_j = q.getCol(j);
            for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
                // if the value exists in the first matrix, it has already been seen
                if(!p.exists(it->first,j)){
                    // no value in the first matrix
                    set(it->first,j,it->second);
                }
            }
        }
    }

}

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
