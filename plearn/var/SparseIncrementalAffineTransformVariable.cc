// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: SparseIncrementalAffineTransformVariable.cc 1442 2004-04-27 15:58:16Z morinf $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SparseIncrementalAffineTransformVariable.h"

namespace PLearn {
using namespace std;

template<class T>
void absargmax(const TMat<T>& mat, int& maxi, int& maxj)
{
  #ifdef BOUNDCHECK
  if(mat.length()==0 || mat.width()==0)
    PLERROR("IN void argmax(const TMat<T>& mat, int& maxi, iny& maxj) mat has 0 size");
  #endif
  T* m_i = mat.data();
  maxi=0;
  maxj=0;
  double maxval = m_i[0];
  for(int i=0; i<mat.length(); i++, m_i+=mat.mod())
    for(int j=0; j<mat.width(); j++)
      if(fabs(m_i[j])>maxval)
        {
          maxval = fabs(m_i[j]);
          maxi = i;
          maxj = j;
        }
}


PLEARN_IMPLEMENT_OBJECT(SparseIncrementalAffineTransformVariable,
                        "Affine transformation of a vector variable, with weights that are sparse and incrementally added.",
                        "NO HELP");

SparseIncrementalAffineTransformVariable::SparseIncrementalAffineTransformVariable(Variable* vec, Variable* transformation, real the_running_average_prop, real the_start_grad_prop)
    : inherited(vec, transformation, 
                (vec->size() == 1) ? transformation->width() : (vec->isRowVec() ? 1 : transformation->width()),
                (vec->size() == 1) ? 1 : (vec->isRowVec() ? transformation->width() : 1)),
      n_grad_samples(0), has_seen_input(0), n_weights(0), add_n_weights(0), start_grad_prop(the_start_grad_prop), running_average_prop(the_running_average_prop)
{
    build_();
}


void SparseIncrementalAffineTransformVariable::declareOptions(OptionList& ol)
{
  declareOption(ol, "start_grad_prop", &SparseIncrementalAffineTransformVariable::start_grad_prop, OptionBase::buildoption, 
                "Proportion of the average incoming gradient used to initialize the added weights\n");

  declareOption(ol, "add_n_weights", &SparseIncrementalAffineTransformVariable::add_n_weights, OptionBase::buildoption, 
                "Number of weights to add after next bprop\n");

   declareOption(ol, "positions", &SparseIncrementalAffineTransformVariable::positions, OptionBase::learntoption, 
                "Positions of non-zero weights\n");
   
   declareOption(ol, "sums", &SparseIncrementalAffineTransformVariable::sums, OptionBase::learntoption, 
                "Sums of the incoming gradient\n");
   
   declareOption(ol, "input_average", &SparseIncrementalAffineTransformVariable::input_average, OptionBase::learntoption, 
                "Average of the input\n");
   
   declareOption(ol, "n_grad_samples", &SparseIncrementalAffineTransformVariable::n_grad_samples, OptionBase::learntoption, 
                "Number of incoming gradient summed\n");
   
   declareOption(ol, "has_seen_input", &SparseIncrementalAffineTransformVariable::has_seen_input, OptionBase::learntoption, 
                "Indication that this variable has seen at least one input sample\n");
   
   declareOption(ol, "n_weights", &SparseIncrementalAffineTransformVariable::n_weights, OptionBase::learntoption, 
                "Number of weights in the affine transform\n");
   
  inherited::declareOptions(ol);
}

void
SparseIncrementalAffineTransformVariable::build()
{
    inherited::build();
    build_();
}

void
SparseIncrementalAffineTransformVariable::build_()
{
    // input1 is vec from constructor
    if (input1 && !input1->isVec())
      PLERROR("In SparseIncrementalAffineTransformVariable: expecting a vector Var (row or column) as first argument");
    if(input1->size() != input2->length()-1)
      PLERROR("In SparseIncrementalAffineTransformVariable: transformation matrix (%d+1) and input vector (%d) have incompatible lengths",input2->length()-1,input1->size());

    if(n_grad_samples == 0)
    {
      sums.resize(input2->length()-1,input2->width());
      sums.clear();
    }

    if(!has_seen_input)
    {
      input_average.resize(input2->length()-1);
      input_average.clear();
      positions.resize(input2->length(),input2->width());
      positions.clear();
      sc_input.resize(input1->size());
      sc_grad.resize(input2->width());
      sc_input_grad.resize(input2->length()-1,input2->width());

      // This may not be necessary ...
      for(int i=0; i< input1->size(); i++)
      {
        sc_input[i].forget();
        for(int j=0; j< input2->width(); j++)
        {
          if(i==0) sc_grad[j].forget();
          sc_input_grad(i,j).forget();
        }
      }
    }

    temp_grad.resize(input2->length()-1,input2->width());
    temp_grad.clear();
}

void SparseIncrementalAffineTransformVariable::recomputeSize(int& l, int& w) const
{ 
    if (input1 && input2) {
        l = input1->isRowVec() ? 1 : input2->width();
        w = input1->isColumnVec() ? 1 : input2->width(); 
    } else
        l = w = 0;
}


void SparseIncrementalAffineTransformVariable::fprop()
{
  if( n_weights >= (input2->matValue.length()-1)*input2->matValue.width())
  {
    value << input2->matValue.firstRow();
    Mat lintransform = input2->matValue.subMatRows(1,input2->length()-1);
    transposeProductAcc(value, lintransform, input1->value);
  }
  else
  {
    value.clear();
    /*
    if(has_seen_input)
      exponentialMovingAverageUpdate(input_average,input1->value,running_average_prop);
    else
    {
      input_average << input1->value;
      has_seen_input = true;
    }
    */

    value << input2->matValue.firstRow();
    Mat lintransform = input2->matValue.subMatRows(1,input2->length()-1);
    transposeProductAcc(value, lintransform, input1->value);

    /*
    for(int i=0; i<positions.length(); i++)
    {
      position_i = positions[i];
      value[i] = position_i.length() != 0 ? input2->matValue(0,i) : 0;
      for(int j=0; j<position_i.length(); j++)
      {
        value[i] += input2->matValue(position_i[j]+1,i) * input1->value[position_i[j]];
      }
    }
    */
  }
}


void SparseIncrementalAffineTransformVariable::bprop()
{
 
  if( n_weights >= (input2->matValue.length()-1)*input2->matValue.width())
  {
    Mat&  afftr = input2->matValue;
    int l = afftr.length();
    // Vec bias = afftr.firstRow();
    Mat lintr = afftr.subMatRows(1,l-1);

    Mat& afftr_g = input2->matGradient;
    Vec bias_g = afftr_g.firstRow();
    Mat lintr_g = afftr_g.subMatRows(1,l-1);

    bias_g += gradient;    
    if(!input1->dont_bprop_here)      
      productAcc(input1->gradient, lintr, gradient);
    externalProductAcc(lintr_g, input1->value, gradient);
  }
  else
  {
    // Update Stats Collector
    for(int i=0; i< input1->size(); i++)
    {
      sc_input[i].update(input1->value[i]);
      for(int j=0; j< input2->width(); j++)
      {
        if(i==0) sc_grad[j].update(gradient[j]);
        sc_input_grad(i,j).update(input1->value[i]*gradient[j]);
      }
    }
    
    // Update sums of gradient
    //externalProductAcc(sums, (input1->value-input_average)/input_stddev, gradient);
    n_grad_samples++;
    int l = input2->matValue.length();
        
    
      // Set the sums for already added weights to 0
      /*
      for(int i=0; i<positions.length(); i++)
      {
        position_i = positions[i];
        for(int j=0; j<position_i.length(); j++)
          sums(position_i[j],i) = 0;
      }
      */

      //sums *= positions.subMatRows(1,l-1);      

      if(add_n_weights > 0)
      {
        // Watch out! This is not compatible with the previous version!
        sums.clear();

        Mat positions_lin = positions.subMatRows(1,l-1); 
        real* sums_i = sums.data();
        real* positions_lin_i = positions_lin.data();
        for(int i=0; i<sums.length(); i++, sums_i+=sums.mod(),positions_lin_i+=positions_lin.mod())
          for(int j=0; j<sums.width(); j++)
          {
            //sums_i[j] *= 1-positions_lin_i[j];
            if(positions_lin_i[j] == 0)
            {
              sums_i[j] = safeflog(abs(sc_input_grad(i,j).mean() - sc_input[i].mean() * sc_grad[j].mean()))
                - safeflog( sc_input[i].stddev() * sc_grad[j].stddev());
            }
          }

        while(add_n_weights >0 && n_weights < (input2->matValue.length()-1)*input2->matValue.width())
        {
          add_n_weights--;
          n_weights++; 
          int maxi, maxj;
          absargmax(sums,maxi,maxj);
          input2->matValue(maxi+1,maxj) = start_grad_prop * sums(maxi,maxj)/n_grad_samples;
          //positions[maxj].push_back(maxi);
          if(positions(0,maxj) == 0)
            positions(0,maxj) = 1;
          positions(maxi+1,maxj) = 1;
          sums(maxi,maxj) = 0;
        }
        // Initialize gradient cumulator
        n_grad_samples=0;
        sums.clear();

        for(int i=0; i< input1->size(); i++)
        {
          sc_input[i].forget();
          for(int j=0; j< input2->width(); j++)
          {
            if(i==0) sc_grad[j].forget();
            sc_input_grad(i,j).forget();
          }
        }
      }
    // Do actual bprop
    /*
    for(int i=0; i<positions.length(); i++)
    {
    position_i = positions[i];
    input2->matGradient(0,i) += position_i.length() != 0 ? gradient[i] : 0;
    for(int j=0; j<position_i.length(); j++)
    {
    input2->matGradient(position_i[j]+1,i) += gradient[i] * input1->value[position_i[j]];
    if(!input1->dont_bprop_here) 
    input1->gradient[position_i[j]] += gradient[i] * input2->matValue(position_i[j]+1,i);
    }
    }
    */
    Mat&  afftr = input2->matValue;
    // Vec bias = afftr.firstRow();
    Mat lintr = afftr.subMatRows(1,l-1);

    Mat& afftr_g = input2->matGradient;
    Vec bias_g = afftr_g.firstRow();

    multiplyAcc(bias_g,gradient,positions.firstRow());    
    if(!input1->dont_bprop_here)      
      productAcc(input1->gradient, lintr, gradient);
    externalProduct(temp_grad, input1->value, gradient);
    temp_grad *= positions.subMatRows(1,l-1);
    afftr_g.subMatRows(1,l-1) += temp_grad;
  }
}


void SparseIncrementalAffineTransformVariable::symbolicBprop()
{
  PLERROR("SparseIncrementalAffineTransformVariable::symbolicBprop() not implemented");
}

void SparseIncrementalAffineTransformVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(positions, copies);
  deepCopyField(sums, copies);
  deepCopyField(input_average, copies);  
  //deepCopyField(position_i, copies);
  deepCopyField(temp_grad,copies);
  deepCopyField(sc_input,copies);
  deepCopyField(sc_grad,copies);
  deepCopyField(sc_input_grad,copies);
}

void SparseIncrementalAffineTransformVariable::reset()
{
  /*
  for(int i=0; i<positions.length(); i++)
  {
    positions[i].clear();
    positions[i].resize(0);
  }
  */
  positions.clear();
  sums.clear();
  n_grad_samples = 0;
  input_average.clear();
  has_seen_input = false;
  n_weights = 0;
  for(int i=0; i< input1->size(); i++)
  {
    sc_input[i].forget();
    for(int j=0; j< input2->width(); j++)
    {
      if(i==0) sc_grad[j].forget();
      sc_input_grad(i,j).forget();
    }
  }
}



} // end of namespace PLearn


