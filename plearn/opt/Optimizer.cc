// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: Optimizer.cc,v 1.10 2003/05/13 15:51:05 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "AsciiVMatrix.h" // TODO Same as below ?
#include "fileutils.h"  // TODO Remove when no more gradient stats ?
#include "Optimizer.h"
#include "TMat_maths.h"
// #include "DisplayUtils.h"

namespace PLearn <%
using namespace std;


Optimizer::Optimizer(int n_updates, const string& file_name, int every_iterations)
  :nupdates(n_updates), nstages(0), filename(file_name), every(every_iterations)
{}
Optimizer::Optimizer(VarArray the_params, Var the_cost, int n_updates,
                     const string& file_name, int every_iterations)
  :params(the_params),cost(the_cost), nupdates(n_updates), 
  filename(file_name), every(every_iterations)
{}
Optimizer::Optimizer(VarArray the_params, Var the_cost, VarArray the_update_for_measure,
                     int n_updates, const string& file_name, 
                     int every_iterations)
  :params(the_params), cost(the_cost), nupdates(n_updates),
   update_for_measure(the_update_for_measure),
   filename(file_name), every(every_iterations)
{}

void Optimizer::build()
{
  inherited::build();
  build_();
}

void Optimizer::build_()
{
  if (update_for_measure.length()==0) // normal propagation path
  {
    // JS - dans la premiere version, ces deux lignes etaient seulement la pour le second constructeur (dans init())
    early_stop=false;
    early_stop_i=0;
    proppath = propagationPath(params, cost);
  }
  else
    proppath = propagationPath(params, update_for_measure&(VarArray)cost);
  VarArray path_from_all_sources_to_direct_parents = propagationPathToParentsOfPath(params, cost);
  path_from_all_sources_to_direct_parents.fprop();
  int n = params.nelems();
  if (n > 0) {
    temp_grad.resize(params.nelems());
/*    same_sign.resize(params.nelems());
    same_sign.clear();*/
  }
  stage = 0;
  if (nstages_per_epoch <= 0) { // probably not correctly initialized by the learner
    nstages_per_epoch = 1;
  }
}

void Optimizer::declareOptions(OptionList& ol)
{
  declareOption(ol, "n_updates", &Optimizer::nupdates, OptionBase::buildoption, 
                "    maximum number of parameter-updates to be performed by the optimizer\n");

  declareOption(ol, "every_iterations", &Optimizer::every, OptionBase::buildoption, 
                "    call measure() method every that many updates \n");

  declareOption(ol, "filename", &Optimizer::filename, OptionBase::buildoption, 
                "    call measure <every> <nupdates> iterations saving the results in the <filename>. \n");

  declareOption(ol, "nstages", &Optimizer::nstages, OptionBase::buildoption, 
                "    number of iterations to perform on the next ""optimizeN"" call\n");

  inherited::declareOptions(ol);
}

void Optimizer::oldwrite(ostream& out) const
{
  writeHeader(out, "Optimizer", 0);
  writeField(out, "n_updates", nupdates);
  writeField(out, "every_iterations", every);
  writeFooter(out, "Optimizer");
}

void Optimizer::oldread(istream& in)
{
  int ver = readHeader(in, "Optimizer");
  if(ver!=0)
    PLERROR("In Optimizer::read version number %d not supported",ver);
  readField(in, "n_updates", nupdates);
  readField(in, "every_iterations", every);
  readFooter(in, "Optimizer");
}

void Optimizer::setToOptimize(VarArray the_params, Var the_cost)
{
//  if(the_cost->length()!=1)
//    PLERROR("IN Optimizer::setToOptimize, cost must be a scalar variable (length 1)");
  params = the_params;//displayVarGraph(params, true, 333, "p1", false);
  cost = the_cost;//displayVarGraph(cost[0], true, 333, "c1", false);
  proppath = propagationPath(params,cost);//displayVarGraph(proppath, true, 333, "x1", false);
  VarArray path_from_all_sources_to_direct_parents = propagationPathToParentsOfPath(params, cost);
  path_from_all_sources_to_direct_parents.fprop();//displayVarGraph(path_from_all_sources_to_direct_parents, true, 333, "x1", false);
}

void Optimizer::setVarArrayOption(const string& optionname, VarArray value)
{
  if (optionname=="params") setToOptimize(value, cost);
  else if (optionname=="update_for_measure") update_for_measure = value;
  else PLERROR("In Optimizer::setVarArrayOption(const string& optionname, VarArray value): option not recognized (%s).",optionname.c_str());
}

void Optimizer::setVarOption(const string& optionname, Var value)
{
  if (optionname=="cost") setToOptimize(params, value);
  else PLERROR("In Optimizer::setVarOption(const string& optionname, VarArray value): option not recognized (%s).",optionname.c_str());
}

void Optimizer::setVMatOption(const string& optionname, VMat value)
{
  PLERROR("In Optimizer::setVMatOption(const string& optionname, VarArray value): option not recognized (%s).",optionname.c_str());
}


IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(Optimizer);

void Optimizer::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(cost, copies);
  deepCopyField(params, copies);
  //deepCopyField(measurers, copies);
  build();
}

void Optimizer::addMeasurer(Measurer& measurer)
{ 
  measurers.appendIfNotThereAlready(&measurer); 
}

bool Optimizer::measure(int t, const Vec& costs)
{
  bool stop=false;
  for(int i=0; i<measurers.size(); i++)
    stop = stop || measurers[i]->measure(t,costs);
  return stop;
}

void Optimizer::verifyGradient(real minval, real maxval, real step)
{
  Func f(params,cost);
  f->verifyGradient(minval, maxval, step);
}

void Optimizer::verifyGradient(real step)
{
  Func f(params,cost);
  Vec p(params.nelems());
  params >> p;
  f->verifyGradient(p, step);
}

Optimizer::~Optimizer()
{}

////////////////////////
// computeRepartition //
////////////////////////
void Optimizer::computeRepartition(
    Vec v, int n, real mini, real maxi, 
    Vec res, int& noutliers) {
  res.clear();
  noutliers = 0;
  for (int i=0; i<v.length(); i++) {
    real k = (v[i] - mini) / (maxi - mini);
    int j = int(k*n);
    if (j >= n) {
      noutliers++;
      j = n-1;
    }
    if (j < 0) {
      noutliers++;
      j = 0;
    }
    res[j]++;
  }
  for (int i = 0; i<n; i++) {
    res[i] /= v.length();
  }
}

//////////////////////////
// collectGradientStats //
//////////////////////////
void Optimizer::collectGradientStats(Vec gradient) {
  static VecStatsCollector grad;
  static VecStatsCollector abs_grad;
  static VecStatsCollector sign_grad;
  static VecStatsCollector change_sign_grad;
  static VMat grad_mean;
  static VMat abs_grad_mean;
  static VMat grad_var;
  static VMat abs_grad_var;
  static VMat sign_grad_mean;
  static VMat sign_grad_var;
  static VMat sign_mean_grad;
  static VMat tmp_mat;
  static VMat change_sign_grad_mean;
  static bool first_time = true;

  // Store gradient
  grad.update(gradient);

  // Store abs(gradient)
  for (int i=0; i<gradient.length(); i++) {
    temp_grad[i] = abs(gradient[i]);
  }
  abs_grad.update(temp_grad);
  
  // Store sign(gradient)
  for (int i=0; i<gradient.length(); i++) {
    if (gradient[i] > 0) {
      temp_grad[i] = 1;
    } else {
      temp_grad[i] = -1;
    }
  }
  sign_grad.update(temp_grad);

  if ((stage+1) % nstages_per_epoch == 0) {
    // One epoch has just been completed
    if (first_time) {
      // This means it is the first epoch
      first_time = false;
      grad_mean = new AsciiVMatrix("grad_mean.amat", params.nelems());
      grad_var = new AsciiVMatrix("grad_var.amat", params.nelems());
      abs_grad_mean = new AsciiVMatrix("abs_grad_mean.amat", params.nelems());
      abs_grad_var = new AsciiVMatrix("abs_grad_var.amat", params.nelems());
      sign_grad_mean = new AsciiVMatrix("sign_grad_mean.amat", params.nelems());
      sign_grad_var = new AsciiVMatrix("sign_grad_var.amat", params.nelems());
      sign_mean_grad = new AsciiVMatrix("sign_mean_grad.amat", params.nelems());
      change_sign_grad_mean = new AsciiVMatrix("change_sign_grad_mean.amat", params.nelems());
    }

    temp_grad << grad.getMean();

    grad_mean->appendRow(temp_grad); 
    grad_var->appendRow(grad.getVariance());
    abs_grad_mean->appendRow(abs_grad.getMean());
    abs_grad_var->appendRow(abs_grad.getVariance());
    sign_grad_mean->appendRow(sign_grad.getMean());
    sign_grad_var->appendRow(sign_grad.getVariance());
    
    // Store sign(mean(grad))
    for (int i=0; i<gradient.length(); i++) {
      if (temp_grad[i] > 0) {
        temp_grad[i] = 1;
      } else
        temp_grad[i] = -1;
    }
    sign_mean_grad->appendRow(temp_grad);
    
    // Compare two consecutive updates
    tmp_mat = sign_mean_grad.row(sign_mean_grad.length()-2);
    for (int i=0; i<gradient.length(); i++) {
      if (i==0 || tmp_mat(0,i) == temp_grad[i]) {
        // Same direction
        temp_grad[i] = 0;
      } else {
        // Opposite direction
        temp_grad[i] = 1;
      }
    }
    change_sign_grad.update(temp_grad);
    change_sign_grad_mean->appendRow(change_sign_grad.getMean());

    grad.forget();
    abs_grad.forget();
    sign_grad.forget();
  }
}

/////////////////////
// computeGradient //
/////////////////////
void Optimizer::computeGradient(
    Optimizer* opt,
    const Vec& gradient) {
  // Clear all what's left from previous computations
  opt->proppath.clearGradient();
  opt->params.clearGradient();
  opt->cost->gradient[0] = 1;
  opt->proppath.fbprop();
  opt->params.copyGradientTo(gradient);
}

/////////////////////////////
// computeOppositeGradient //
/////////////////////////////
void Optimizer::computeOppositeGradient(
    Optimizer* opt,
    const Vec& gradient) {
  // Clear all what's left from previous computations
  opt->proppath.clearGradient();
  opt->params.clearGradient();
  // We want the opposite of the gradient, thus the -1
  opt->cost->gradient[0] = -1;
  opt->proppath.fbprop();
  opt->params.copyGradientTo(gradient);
}
  
%> // end of namespace PLearn
