// -*- C++ -*-

// PLS.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: PLS.cc,v 1.4 2004/03/05 20:07:28 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file PLS.cc */

#include "PLS.h"
#include "ShiftAndRescaleVMatrix.h"
#include "SubVMatrix.h"
#include "VMat_maths.h"

namespace PLearn {
using namespace std;

PLS::PLS() 
: m(-1),
  p(-1),
  k(1),
  precision(1e-6),
  output_the_score(0),
  output_the_target(1)
{}

PLEARN_IMPLEMENT_OBJECT(PLS,
  "Partial Least Squares Regression (PLSR).",
  "You can use this learner to perform regression, and / or dimensionality\n"
  "reduction.\n"
  "PLS regression assumes the target Y and the data X are linked through:\n"
  " Y = T.Q' + E\n"
  " X = T.P' + F\n"
  "The underlying coefficients T (the 'scores') and the loading matrices\n"
  "Q and P are seeked. It is then possible to compute the prediction y for\n"
  "a new input x, as well as its score vector t (its representation in\n"
  "lower-dimensional coordinates).\n"
  "The code implements a NIPALS-PLS-like algorithm, which is a so-called\n"
  "'kernel' algorithm (faster than more classical implementations).\n"
  "The algorithm, inspired from 'Factor Analysis in Chemistry' and above all\n"
  "www.statsoftinc.com/textbook/stpls.html, is the following:\n"
  " (1) Let X (n x p) = the centered and normalized input data\n"
  "     Let Y (n x m) = the centered and normalized target data\n"
  "     Let k be the number of components extracted\n"
  " (2) Initialize A_0 = X'Y, M_0 = X'X, C_0 = Identity(p), and h = 0\n"
  " (3) q_h = largest eigenvector of B_h = A_h' A_h, found by the NIPALS method:\n"
  "       (a) q_h = a (normalized) randomn column of B_h\n"
  "       (b) q_h = B_h q_h\n"
  "       (c) normalize q_h\n"
  "       (d) if q_h has changed by more than 'precision', go to (b)\n"
  " (4) w_h = C_h A_h q_h, normalize w_h and store it in a column of W (p x k)\n"
  " (5) p_h = M_h w_h, c_h = w_h' p_h, p_h = p_h / c_h and store it in a column\n"
  "     of P (p x c)\n"
  " (6) q_h = A_h' w_h / c_h, and store it in a column of Q (m x k)\n"
  " (7) A_h+1 = A_h - c_h p_h q_h'\n"
  "     M_h+1 = M_h - c_h p_h p_h',\n"
  "     C_h+1 = C_h - w_h p_h\n"
  " (8) h = h+1, and if h < k, go to (3)\n"
  "\n"
  "The result is then given by:\n"
  " - Y = X B, with B (p x m) = W Q'\n"
  " - T = X W, where T is the score (reduced coordinates)\n"
  "\n"
  "You can choose to have the score (T) and / or the target (Y) in the output\n"
  "of the learner (default is target only, i.e. regression)."
);

////////////////////
// declareOptions //
////////////////////
void PLS::declareOptions(OptionList& ol)
{
  declareOption(ol, "B", &PLS::B, OptionBase::learntoption,
      "The regression matrix in Y = X.B + E.");

  declareOption(ol, "k", &PLS::k, OptionBase::buildoption,
      "The number of components (factors) computed.");

  declareOption(ol, "m", &PLS::m, OptionBase::learntoption,
      "Used to store the target size.");

  declareOption(ol, "mean_input", &PLS::mean_input, OptionBase::learntoption,
      "The mean of the input data X.");

  declareOption(ol, "mean_target", &PLS::mean_target, OptionBase::learntoption,
      "The mean of the target data Y.");

  declareOption(ol, "output_the_score", &PLS::output_the_score, OptionBase::buildoption,
      "If set to 1, then the score (the low-dimensional representation of the input)\n"
      "will be included in the output (before the target).");

  declareOption(ol, "output_the_target", &PLS::output_the_target, OptionBase::buildoption,
      "If set to 1, then (the prediction of) the target will be included in the\n"
      "output (after the score).");

  declareOption(ol, "p", &PLS::p, OptionBase::learntoption,
      "Used to store the input size.");

  declareOption(ol, "precision", &PLS::precision, OptionBase::buildoption,
      "The precision to which we compute the eigenvectors.");

  declareOption(ol, "stddev_input", &PLS::stddev_input, OptionBase::learntoption,
      "The standard deviation of the input data X.");

  declareOption(ol, "stddev_target", &PLS::stddev_target, OptionBase::learntoption,
      "The standard deviation of the target data Y.");

  declareOption(ol, "W", &PLS::W, OptionBase::learntoption,
      "The regression matrix in T = X.W.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void PLS::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void PLS::build_()
{
  if (train_set) {
    this->m = train_set->targetsize();
    this->p = train_set->inputsize();
    mean_input.resize(p);
    stddev_input.resize(p);
    mean_target.resize(m);
    stddev_target.resize(m);
  }
  if (!output_the_score && !output_the_target) {
    // Weird, we don't want any output ??
    PLWARNING("In PLS::build_ - There will be no output");
  }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void PLS::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  // No cost computed.
}                                

///////////////////
// computeOutput //
///////////////////
void PLS::computeOutput(const Vec& input, Vec& output) const
{
  static Vec input_copy;
  // Compute the output from the input
  int nout = outputsize();
  output.resize(nout);
  // First normalize the input.
  input_copy.resize(this->p);
  input_copy << input;
  input_copy -= mean_input;
  input_copy /= stddev_input;
  int target_start = 0;
  if (output_the_score) {
    transposeProduct(output.subVec(0, this->k), W, input_copy);
    target_start = this->k;
  }
  if (output_the_target) {
    Vec target = output.subVec(target_start, this->m);
    transposeProduct(target, B, input_copy);
    target *= stddev_target;
    target += mean_target;
  }
}    

////////////
// forget //
////////////
void PLS::forget()
{
  stage = 0;
  B.resize(0,0);
  W.resize(0,0);
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> PLS::getTestCostNames() const
{
  // No cost computed.
  TVec<string> t;
  return t;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> PLS::getTrainCostNames() const
{
  // No cost computed.
  TVec<string> t;
  return t;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PLS::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("PLS::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

///////////////////////
// NIPALSEigenvector //
///////////////////////
void PLS::NIPALSEigenvector(const Mat& m, Vec& v, real precision) {
  int n = v.length();
  Vec w(n);
  v << m.column(0);
  normalize(v, 2.0);
  bool ok = false;
  while (!ok) {
    w << v;
    product(v, m, w);
    normalize(v, 2.0);
    ok = true;
    for (int i = 0; i < n && ok; i++) {
      if (fabs(v[i] - w[i]) > precision) {
        ok = false;
      }
    }
  }
}
    
////////////////
// outputsize //
////////////////
int PLS::outputsize() const
{
  int os = 0;
  if (output_the_score) {
    os += this->k;
  }
  if (output_the_target) {
    os += this->m;
  }
  return os;
}

///////////
// train //
///////////
void PLS::train()
{
  if (stage == 1) {
    // Already trained.
    if (verbosity >= 1) {
      cout << "Skipping PLS training" << endl;
    }
    return;
  }
  if (verbosity >= 1) {
    cout << "PLS training started" << endl;
  }

  // Construct the centered and normalized training set, for the input
  // as well as the target part.
  if (verbosity >= 2) {
    cout << " Normalization of the data" << endl;
  }
  VMat input_part = new SubVMatrix(
      train_set, 0, 0, train_set->length(), train_set->inputsize());
  VMat target_part = new SubVMatrix(
      train_set, 0, train_set->inputsize(), train_set->length(), train_set->targetsize());
  PP<ShiftAndRescaleVMatrix> X_vmat = new ShiftAndRescaleVMatrix();
  X_vmat->vm = input_part;
  X_vmat->build();
  mean_input << X_vmat->shift;
  stddev_input << X_vmat->scale;
  negateElements(mean_input);
  invertElements(stddev_input);
  PP<ShiftAndRescaleVMatrix> Y_vmat = new ShiftAndRescaleVMatrix();
  Y_vmat->vm = target_part;
  Y_vmat->build();
  mean_target << Y_vmat->shift;
  stddev_target << Y_vmat->scale;
  negateElements(mean_target);
  invertElements(stddev_target);

  // Initialize the various coefficients.
  if (verbosity >= 2) {
    cout << " Initialization of the coefficients" << endl;
  }
  int c = this->k;
  Vec ph(p);
  Mat P(p, c);
  Vec qh(m);
  Mat Q(m, c);
  Vec wh(p);
  Vec tmp(p);
  W.resize(p, c);
  real ch;
  VMat X_vmatrix = static_cast<ShiftAndRescaleVMatrix*>(X_vmat);
  VMat Y_vmatrix = static_cast<ShiftAndRescaleVMatrix*>(Y_vmat);
  Mat Ah = transposeProduct(X_vmatrix, Y_vmatrix);
  Mat Mh = transposeProduct(X_vmatrix, X_vmatrix);
  Mat Ch(p,p);    // Initialized to Identity(p).
  Mat Ah_t_Ah;
  Mat update_Ah(p,m);
  Mat update_Mh(p,p);
  Mat update_Ch(p,p);
  for (int i = 0; i < p; i++) {
    for (int j = i+1; j < p; j++) {
      Ch(i,j) = Ch(j,i) = 0;
    }
    Ch(i,i) = 1;
  }

  // Iterate k times to find the k first factors.
  ProgressBar* pb = 0;
  if(report_progress) {
    pb = new ProgressBar("Computing the components", k);
  }
  for (int h = 0; h < this->k; h++) {
    Ah_t_Ah = transposeProduct(Ah,Ah);
    if (m == 1) {
      // No need to compute the eigenvector.
      qh[0] = 1;
    } else {
      NIPALSEigenvector(Ah_t_Ah, qh, precision);
    }
    product(tmp, Ah, qh);
    product(wh, Ch, tmp);
    normalize(wh, 2.0);
    W.column(h) << wh;
    product(ph, Mh, wh);
    ch = dot(wh, ph);
    ph /= ch;
    P.column(h) << ph;
    transposeProduct(qh, Ah, wh);
    qh /= ch;
    Q.column(h) << qh;
    Mat ph_mat(p, 1, ph);
    Mat qh_mat(m, 1, qh);
    Mat wh_mat(p, 1, wh);
    update_Ah = productTranspose(ph_mat, qh_mat);
    update_Ah *= ch;
    Ah -= update_Ah;
    update_Mh = productTranspose(ph_mat, ph_mat);
    update_Mh *= ch;
    Mh -= update_Mh;
    update_Ch = productTranspose(wh_mat, ph_mat);
    Ch -= update_Ch;
    if (pb) pb->update(h + 1);
  }
  B.resize(p,m);
  productTranspose(B, W, Q);
  if (pb) delete pb;
  if (verbosity >= 1) {
    cout << "PLS training ended" << endl;
  }
  stage = 1;
}

} // end of namespace PLearn
