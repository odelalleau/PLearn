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
   * $Id: Optimizer.cc,v 1.5 2003/05/03 05:02:18 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "Optimizer.h"
#include "TMat_maths.h"
// #include "DisplayUtils.h"

namespace PLearn <%
using namespace std;


Optimizer::Optimizer(int n_updates, const string& file_name, int every_iterations)
  :nupdates(n_updates), filename(file_name), every(every_iterations)
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
                "    number of iterations to reach on the next ""optimizeN"" call\n");

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
