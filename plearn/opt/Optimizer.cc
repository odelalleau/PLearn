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
   * $Id: Optimizer.cc,v 1.8 2003/05/12 20:50:52 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

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
    mean_grad.resize(params.nelems());
    same_sign.resize(params.nelems());
    same_sign.clear();
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

real max_grad = 0;
real min_grad = 0;

//////////////////////////
// collectGradientStats //
//////////////////////////
void Optimizer::collectGradientStats(Vec gradient) {
  /*static VecStatsCollector grad;
  static VecStatsCollector grad_abs;
  static VecStatsCollector abs_grad;
  static VMat gradm;
  grad.update(gradient);

  if (gradm) {
    gradm = new AsciiVMatrix("gradm.amat", params.nelems());
  }
*/  

  mean_grad += gradient;
  if ((stage+1) % nstages_per_epoch == 0) {
    // One epoch has just been completed
 /*   gradm.appendRow(grad.mean()); */
    
    string filename = "gradstats.data";
    ofstream* gradstats = new ofstream(filename.c_str(),ios::out|ios::app);
    ostream& out = *gradstats;
    filename = "graddistrib.data";
    ofstream* gradstatsd = new ofstream(filename.c_str(),ios::out|ios::app);
    ostream& outd = *gradstatsd;

    if(out.bad())
      PLERROR("In Optimizer::collectGradientStats could not open file %s for appending",filename.c_str());
#if __GNUC__ < 3
    if(out.tellp() == 0)
#else
    if(out.tellp() == streampos(0))
#endif
      out << "#: Epoch  Mean_Means  Var_Means  Std_Means  N_outliers" << endl;
    
    mean_grad /= nstages_per_epoch;
    for (int i=0; i<mean_grad.length(); i++) {
      if (mean_grad[i] * same_sign[i] > 0) {
        // two consecutive updates in the same direction
        if (mean_grad[i] < 0) {
          same_sign[i]--;
        } else {
          same_sign[i]++;
        }
      } else {
        // two consecutive updates in different directions
        if (mean_grad[i] > 0) {
          same_sign[i] = +1;
        } else {
          same_sign[i] = -1;
        }
      }
    }
    real m = mean(mean_grad);
    real v = variance(mean_grad, m);
    if (max_grad == min_grad) {
      // this means it is the first epoch
      max_grad = sqrt(v)/2;
      min_grad = -max_grad;
      outd << "  " << min_grad << "  " << max_grad << endl;
    }
    int n = 20;
    Vec distrib(n);
    Vec mean_abs_grad(mean_grad.length());
    for (int i=0; i<mean_grad.length(); i++) {
      mean_abs_grad[i] = abs(mean_grad[i]);
    }
    int noutliers;
    computeRepartition(mean_grad, n, min_grad, max_grad, distrib, noutliers);
    out << "  " <<
           (stage + 1) / nstages_per_epoch << "  " <<
           m << "  " <<
           v << "  " <<
           sqrt(v) << "  " <<
           noutliers << " ";
    outd << "  ";
//            (stage + 1) / nstages_per_epoch << "  ";
    for (int i=0; i<n; i++) {
      outd << distrib[i] << "  ";
    }
    out << endl;
    outd << endl;
    computeRepartition(mean_abs_grad, n, 0, max_grad, distrib, noutliers);
    for (int i=0; i<n; i++) {
      outd << distrib[i] << "  ";
    }
    outd << endl;
    for (int i=0; i<mean_grad.length(); i++) {
      outd << abs(same_sign[i]) << "  ";
    }
    outd << endl;

    // Distribution of the number of consecutive iterations
    outd << "  ";
    int nepochs = (stage+1) / nstages_per_epoch;
    Vec consec_distrib(nepochs+1);
    Vec consec_distrib_grads(nepochs+1);
    consec_distrib.clear();
    consec_distrib_grads.clear();
    for (int i=0; i<same_sign.length(); i++) {
      int ind = int(abs(same_sign[i]));
      consec_distrib[ind]++;
      consec_distrib_grads[ind] += abs(mean_grad[i]);
    }
    for (int i=1; i<=nepochs; i++) {
      consec_distrib[i] += consec_distrib[i-1];
      outd << consec_distrib[i] << " ";
    }
    outd << endl << " ";
    int smooth = 10;
    Vec consec_grad_distrib(nepochs+1);
    consec_grad_distrib.clear();
    for (int i=1; i<=nepochs; i++) {
      int nb = 0;
      for (int j=i-smooth; j<=i+smooth; j++) {
        if (j>=1 && j<=nepochs) {
          nb += int(consec_distrib[j]);
          consec_grad_distrib[i] += consec_distrib_grads[j];
        }
      }
      if (nb > 0) {
        consec_grad_distrib[i] /= real(nb);
      }
      outd << consec_grad_distrib[i] * 1000000<< " ";
    }
    outd << endl;

    mean_grad.clear();
    gradstats->close();
    gradstatsd->close();
    free(gradstats);
    free(gradstatsd);
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
