// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: RandomVar.cc,v 1.6 2004/07/06 15:53:16 tihocan Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


/** RandomVar **/
#include "AbsVariable.h"
#include "ConcatColumnsVariable.h"
#include "ConcatRowsVariable.h"
#include "DeterminantVariable.h"
#include "EqualVariable.h"
#include "ExpVariable.h"
#include "ExtendedVariable.h"
#include "LeftPseudoInverseVariable.h"
#include "LogSumVariable.h"
#include "LogVariable.h"
#include "MatrixSumOfVariable.h"
#include "PowVariable.h"
#include "ProductVariable.h"
#include "RightPseudoInverseVariable.h"
#include "SoftmaxVariable.h"
#include "SumOfVariable.h"
#include "SumVariable.h"

#include "general.h"
#include "RandomVar.h"
//#include "NaryVariable.h"
//#include "ConjugateGradientOptimizer.h" // Not in the PLearn CVS repository.
#include "plapack.h"
#include "Var_operators.h"
//#include <cmath>

namespace PLearn {
using namespace std;

RandomVar::RandomVar() 
  :PP<RandomVariable>(new NonRandomVariable(1)) {}
RandomVar::RandomVar(RandomVariable* v) 
  :PP<RandomVariable>(v) {}
RandomVar::RandomVar(const RandomVar& other) 
  :PP<RandomVariable>(other) {}
RandomVar::RandomVar(int length, int width)
  :PP<RandomVariable>(new NonRandomVariable(length,width)) {}

RandomVar::RandomVar(const Vec& vec) 
  :PP<RandomVariable>(new NonRandomVariable(Var(vec))) {}
RandomVar::RandomVar(const Mat& mat) 
  :PP<RandomVariable>(new NonRandomVariable(Var(mat))) {}
RandomVar::RandomVar(const Var& v) 
  :PP<RandomVariable>(new NonRandomVariable(v)) {}

RandomVar::RandomVar(const RVArray& rvars) 
  :PP<RandomVariable>(new JointRandomVariable(rvars)) {}

RandomVar RandomVar::operator[](RandomVar index)
{ return new RandomElementOfRandomVariable((*this),index); }

void RandomVar::operator=(const RVArray& vars) 
{ *this = RandomVar(vars); }

void RandomVar::operator=(real f)
{
  if (!(*this)->isNonRandom())
    PLERROR("RandomVar: can't assign values to a truly random RV");
  (*this)->value->value.fill(f);
}

void RandomVar::operator=(const Vec& v)
{
  if (!(*this)->isNonRandom())
    PLERROR("RandomVar: can't assign a values to a truly random RV");
  (*this)->value->value << v;
}

void RandomVar::operator=(const Mat& m)
{ 
  if (!(*this)->isNonRandom())
    PLERROR("RandomVar: can't assign a Var to a truly random RV");
  Var v(m);
  (*this)->value = v;
}

void RandomVar::operator=(const Var& v)
{
  if (!(*this)->isNonRandom())
    PLERROR("RandomVar: can't assign a Var to a truly random RV");
  (*this)->value = v;
}

RVInstance RandomVar::operator==(const Var& v) const
{
  return RVInstance(*this,v);
}

// make an array of RV's
RVArray RandomVar::operator&(const RandomVar& v) const
{
  return RVArray(*this,v,10);
}

ConditionalExpression RandomVar::operator|(RVArray a) const
{
  RVInstanceArray rvia(a.size());
  for (int i=0;i<a.size();i++)
    {
      rvia[i].V = a[i];
      rvia[i].v = a[i]->value;
    }
  return ConditionalExpression(RVInstance(*this,Var((*this)->length())),rvia);
}
  
ConditionalExpression RandomVar::operator|(RVInstanceArray rhs) const
{
  return ConditionalExpression(RVInstance(*this,Var((*this)->length())),rhs);
}

#if 0

RandomVar RandomVar::operator[](RandomVar index)
{ return matRandomVarElement(*this,index); }

Vec RandomVar::operator[](int i)
{ return matRandomVarElement(*this,i); }

RandomVar RandomVar::operator()(RandomVar i, RandomVar j)
{
  return 
    new RandomElementOfRandomVariable(*this,i*((real)((*this)->value->matValue.width()))+j);
}

real& RandomVar::operator()(int i, int j)
{
  return new RandomVarElement(*this,i*((real)((*this)->value->matValue.width()))+j);
}

#endif



/** RandomVariable **/

int RandomVariable::rv_counter = 0;

RandomVariable::RandomVariable(int thelength, int thewidth)
  :rv_number(rv_counter++), value(thelength,thewidth),marked(false), 
   EMmark(false), pmark(false), learn_the_parameters(0)
{
}

RandomVariable::RandomVariable(const Vec& the_value)
  :rv_number(rv_counter++), value(the_value),marked(false), EMmark(false)
  , pmark(false), learn_the_parameters(0)
{
}

RandomVariable::RandomVariable(const Mat& the_value)
  :rv_number(rv_counter++), value(the_value),marked(false), EMmark(false),
  pmark(false), learn_the_parameters(0)
{
}

RandomVariable::RandomVariable(const Var& the_value)
  :rv_number(rv_counter++), value(the_value),marked(false), EMmark(false),
  pmark(false), learn_the_parameters(0)
{
}

RandomVariable::RandomVariable(const RVArray& the_parents, int thelength)
  :rv_number(rv_counter++),
   parents(the_parents), value(thelength), marked(false), EMmark(false),
   pmark(false), learn_the_parameters(new bool[the_parents.size()])
{
}

RandomVariable::RandomVariable(const RVArray& the_parents, int thelength, 
                               int thewidth)
  : rv_number(rv_counter++),
    parents(the_parents), value(thelength,thewidth),marked(false), 
   EMmark(false), pmark(false),
   learn_the_parameters(new bool[the_parents.size()])
{
}


RandomVar RandomVariable::subVec(int start, int length) { 
  return new SubVecRandomVariable(this,start,length);
}

real RandomVariable::EM(const RVArray& parameters_to_learn,
    VarArray& prop_path, VarArray& observedVars, VMat distr, int n_samples, 
    int max_n_iterations, real relative_improvement_threshold,
    bool accept_worsening_likelihood)
{
  real avgnegloglik = 0;
  real previous_nll=FLT_MAX, nll_change;
  bool EMfinished= !(max_n_iterations>0);
  int n_epochs=0;

  EMTrainingInitialize(parameters_to_learn);
  clearEMmarks();
  while (!EMfinished) {
    avgnegloglik=epoch(prop_path, observedVars, distr,n_samples);
    cout << "EM epoch NLL = " << avgnegloglik << endl;
    nll_change = (previous_nll - avgnegloglik)/fabs(previous_nll);
    if (nll_change < -1e-4  &&  !accept_worsening_likelihood)
      printf("%s %s from %f to %f\n", "RandomVariable::EM",
                "An EM epoch yielded worse negative log-likelihood,",
                previous_nll, avgnegloglik);
    n_epochs++;
    EMfinished = canStopEM() &&
      ((n_epochs >= max_n_iterations) ||
       (fabs(nll_change) <= relative_improvement_threshold) ||
       (!accept_worsening_likelihood &&
        nll_change  <= relative_improvement_threshold));
    previous_nll=avgnegloglik;
  }
  return avgnegloglik;
}

real RandomVariable::epoch(VarArray& prop_path, 
                            VarArray& observed_vars, 
                            const VMat& distr, int n_samples,
                            bool do_EM_learning)
{
  real avg_cost = 0;
  if (do_EM_learning) 
    {
      EMEpochInitialize();
      clearEMmarks();
    }
  for (int i=0;i<n_samples;i++)
    {
      Vec sam(distr->width());
      distr->getRow(i,sam);
      observed_vars << sam;
      prop_path.fprop(); // computes logP in last element of prop_path
      Var logp = prop_path.last();

#if 0
      // debugging
      cout << "at example i=" << i << endl;
      VarArray sources = logp->sources();
      logp->unmarkAncestors();
      sources.printInfo();
      prop_path.printInfo();
#endif
        
      avg_cost -= logp->value[0];
      if (do_EM_learning)
        // last = LHS observed value
        EMBprop(observed_vars.last()->value,1.0);
    }

  if (do_EM_learning) 
    {
      EMUpdate();
      clearEMmarks();
    }
  return avg_cost / n_samples;
}

void RandomVariable::unmarkAncestors() 
{ 
  if (pmark)
    {
      marked=false; 
      pmark=false;
      for (int i=0;i<parents.size();i++)
        parents[i]->unmarkAncestors();
    }
}

void RandomVariable::clearEMmarks()
{ 
  if (EMmark)
    {
      EMmark=false; 
      for (int i=0;i<parents.size();i++)
        parents[i]->clearEMmarks();
    }
}

void RandomVariable::setKnownValues()
{
  if (!pmark && !marked)
  {
    pmark=true;
    bool all_parents_marked=true;
    for (int i=0;i<parents.size();i++)
    {
      parents[i]->setKnownValues();
      all_parents_marked &= parents[i]->isMarked();
    }
    setValueFromParentsValue();
    if (all_parents_marked)
      marked=true;
  }
}

void RandomVariable::EMUpdate()
{
  if (EMmark) return;
  EMmark=true;
  for (int i=0;i<parents.size();i++)
    if (!parents[i]->isConstant()) parents[i]->EMUpdate();
}

bool RandomVariable::canStopEM()
{
  // propagate to parents
  bool can=true;
  for (int i=0;i<parents.size() && !can;i++)
    can = parents[i]->canStopEM();
  return can;
}

void RandomVariable::
EMTrainingInitialize(const RVArray& parameters_to_learn)
{
  if (EMmark) return;
  EMmark=true;
  int n_can_learn=0;
  int n_random=0;
  for (int i=0;i<parents.size();i++)
    {
      if (parameters_to_learn.contains(parents[i]))
        {
          if (!parents[i]->isConstant())
            PLERROR("Trying to learn a parameter that is not constant!");
          learn_the_parameters[i] = true;
          n_can_learn++;
        }
      else learn_the_parameters[i] = false;
      if (!parents[i]->isNonRandom())
        n_random++;
    }
  if (n_can_learn>0 && n_random>0)
    PLERROR("RandomVariable: can't learn some parameter if others are random");

  for (int i=0;i<parents.size();i++)
    parents[i]->EMTrainingInitialize(parameters_to_learn);
}

void RandomVariable::EMEpochInitialize()
{
  if (EMmark) return;
  EMmark=true;
  for (int i=0;i<parents.size();i++)
    parents[i]->EMEpochInitialize();
}

Var RandomVariable::P(const Var& obs, const RVInstanceArray& RHS)
{ return exp(logP(obs,RHS,0)); }


RandomVariable::~RandomVariable() 
{ if (learn_the_parameters) delete learn_the_parameters; }

Var RandomVariable::ElogP(const Var& obs, RVArray& parameters_to_learn, 
    const RVInstanceArray& RHS)
{
  PLERROR("ElogP not implemented for this class (%s)",classname());
  return Var(0);
}

/** global functions **/

RandomVar operator*(RandomVar a, RandomVar b)
{
  if (a->isScalar() || b->isScalar()); // scalar times something
  else if (a->isVec() && b->isVec()) // vec times vec
  {
    if (a->length()*a->width() != b->length()*b->width())
      PLERROR("In RandomVar operator*(RandomVar a, RandomVar b) cannot do a dot product between 2 vecs with different sizes");
  }
  else if (a->isRowVec()) // rowvec times mat
  {
    if (a->length() != b->width())
      PLERROR("In RandomVar operator*(RandomVar a, RandomVar b) in case rowvec times mat: a->length() != b->width()");
  }
  else if (b->isRowVec()) // mat times rowvec
  {
    if (b->length() != a->width())
      PLERROR("In RandomVar operator*(RandomVar a, RandomVar b) in case mat times rowvec: b->length() != a->width()");
  }
  else
    PLERROR("In RandomVar operator*(RandomVar a, RandomVar b) This case is not handled (but maybe it should be...)");

  return new ProductRandomVariable(a,b);
}

RandomVar operator+(RandomVar a, RandomVar b)
{
  return new PlusRandomVariable(a,b);
}

RandomVar operator-(RandomVar a, RandomVar b)
{
  return new MinusRandomVariable(a,b);
}

RandomVar operator/(RandomVar a, RandomVar b)
{
  return new ElementWiseDivisionRandomVariable(a,b);
}

// exponential function applied element-by-element
RandomVar exp(RandomVar x) { return new ExpRandomVariable(x); }

// natural logarithm function applied element-by-element
RandomVar log(RandomVar x) { return new LogRandomVariable(x); }

RandomVar extend(RandomVar v, real extension_value, int n_extend)
{ return new ExtendedRandomVariable(v,extension_value,n_extend); }

RandomVar hconcat(const RVArray& a)
{ return new ConcatColumnsRandomVariable(a); }


real EM(ConditionalExpression conditional_expression,
    RVArray parameters_to_learn,
    VMat distr, int n_samples, int max_n_iterations, 
    real relative_improvement_threshold,
    bool accept_worsening_likelihood,
    bool compute_final_train_NLL)
{
  // assign the value fields of the RV's to those provided by user
  RandomVar& LHS=conditional_expression.LHS.V;
  Var& lhs_observation=conditional_expression.LHS.v;
  VarArray prop_input_vars;
  ///////////////////////////////////////////////////////////
  // NOTE NOTE NOTE:
  //
  // THE ORDER OF THE VALUES IN THE DISTRIBUTION MUST BE:
  // (1) conditioning variables (RHS), (2) output variables (LHS)
  ///////////////////////////////////////////////////////////
  VarArray distr_observed_vars = 
    conditional_expression.RHS.instances() & (VarArray)lhs_observation;
  // note that we don't use LHS->value to put the LHS_observation
  // in case some distribution need to compare the observation
  // with a function of their parents that is put in their value field.
  Var logp = logP(conditional_expression,false);
  VarArray prop_path = 
    propagationPath(distr_observed_vars & parameters_to_learn.values(), logp);
  // do the actual EM training with multiple epochs
  real train_NLL =
    LHS->EM(parameters_to_learn,prop_path,distr_observed_vars, distr, 
            n_samples, max_n_iterations, relative_improvement_threshold,
            accept_worsening_likelihood);
  if (compute_final_train_NLL)
    train_NLL = LHS->epoch(prop_path,distr_observed_vars,distr,
                           n_samples,false);
  LHS->unmarkAncestors();
  return train_NLL;
}

real EM(ConditionalExpression conditional_expression,
    RVArray parameters_to_learn,
    VMat distr, int n_samples, int max_n_iterations, 
    real relative_improvement_threshold,
    bool compute_final_train_NLL)
{
  // assign the value fields of the RV's to those provided by user
  RandomVar& LHS=conditional_expression.LHS.V;
  Var& lhs_observation=conditional_expression.LHS.v;
  VarArray prop_input_vars;
  ///////////////////////////////////////////////////////////
  // NOTE NOTE NOTE:
  //
  // THE ORDER OF THE VALUES IN THE DISTRIBUTION MUST BE:
  // (1) conditioning variables (RHS), (2) output variables (LHS)
  ///////////////////////////////////////////////////////////
  VarArray distr_observed_vars = 
    conditional_expression.RHS.instances() & (VarArray)lhs_observation;
  // note that we don't use LHS->value to put the LHS_observation
  // in case some distribution need to compare the observation
  // with a function of their parents that is put in their value field.
  RVInstanceArray params_to_learn(parameters_to_learn.size());
  for (int i=0;i<parameters_to_learn.size();i++)
    {
      params_to_learn[i].V=parameters_to_learn[i];
      // params_to_learn[i].v will hold "new" params in EM epoch
      params_to_learn[i].v=Var(parameters_to_learn[i]->length());
      // initialize "new" params with current value
      params_to_learn[i].v->value << params_to_learn[i].V->value->value;
    }
  Var elogp = ElogP(conditional_expression,params_to_learn,false);
  VarArray new_params = params_to_learn.instances();
  VarArray current_params = parameters_to_learn.values();
  VarArray prop_path = 
    propagationPath(distr_observed_vars & current_params & new_params, elogp);
  // do the actual EM training with N epochs, N=#free parameters
  // (this is because, assuming that maximizing the auxiliary function
  // is solvable analytically, i.e. it is quadratic in the parameters,
  // then N iterations of conjugate gradiends should suffice. With
  // numerical errors, we can tolerate a bit more...
  int n_free_params = new_params.nelems();
  int max_n_Q_iterations = 1 + (int)(n_free_params*1.5);
  Vec params(n_free_params);
  // again, assuming solvable max Q, a specialized but faster CG is enough
  Var totalElogP = meanOf(elogp,distr_observed_vars,
                          distr,n_samples,new_params);
  PLERROR("In EM (RandomVar.cc), code using ConjugateGradientOptimizer is now commented out");
  max_n_Q_iterations = max_n_Q_iterations; // TODO Remove this (just to make the compiler happy).
  /* REMOVED because not in the PLearn CVS repository.
     
  ConjugateGradientOptimizer opt(new_params, totalElogP,
                                 0.001,0.001,max_n_Q_iterations);
                                 */

  // the outer loop is over EM iterations
  real avgnegloglik = 0;
  real previous_nll=FLT_MAX, nll_change;
  bool EMfinished= !(max_n_iterations>0);
  int n_epochs=0;

  while (!EMfinished) {
    // the inner loop is over "new params" optimization of totalElogP 
  PLERROR("In EM (RandomVar.cc), code using ConjugateGradientOptimizer is now commented out");
//    opt.optimize(); COMMENTED (same as above)
    avgnegloglik = - totalElogP->value[0];
    cout << "EM epoch -Q = " << avgnegloglik << endl;
    nll_change = (previous_nll - avgnegloglik)/fabs(previous_nll);
    if (nll_change < -1e-4)
      printf("%s %s from %f to %f\n", "RandomVariable::EM",
                "An EM epoch yielded worse negative log-likelihood,",
                previous_nll, avgnegloglik);
    n_epochs++;
    EMfinished = 
      ((n_epochs >= max_n_iterations) ||
       (fabs(nll_change) <= relative_improvement_threshold) ||
       nll_change  <= relative_improvement_threshold);
    previous_nll=avgnegloglik;

    // copy the "new params" to the "current params"
    new_params >> params;
    current_params << params;
  }

  if (compute_final_train_NLL)
    {
      Var logp = logP(conditional_expression,false);
      Var totalLogP = meanOf(logp,distr_observed_vars,
                             distr,n_samples,current_params);
      totalLogP->fprop();
      avgnegloglik = - totalLogP->value[0];
    }

  LHS->unmarkAncestors();
  return avgnegloglik;
}

Var P(ConditionalExpression conditional_expression, 
      bool clearMarksUponReturn) 
{
  RandomVar& LHS = conditional_expression.LHS.V;
  RVInstanceArray& RHS = conditional_expression.RHS;
  // traverse the tree of ancestors of this node
  // and mark nodes which are deterministic descendents of RHS
  // and of non-random variables
  // while setting their "value" field to this Var function of them.
  LHS->markRHSandSetKnownValues(RHS);

  Var p = LHS->P(conditional_expression.LHS.v,RHS);

  if (clearMarksUponReturn)
    // put the network back in its original state
    LHS->unmarkAncestors();

  // make sure that all the paths which do not
  // depend on x, y, and the tunable_parameters are correctly computed
  p->fprop_from_all_sources();
  return p;
}

Var logP(ConditionalExpression conditional_expression, bool clearMarksUponReturn,
    RVInstanceArray* parameters_to_learn) 
{
  RandomVar& LHS = conditional_expression.LHS.V;
  RVInstanceArray& RHS = conditional_expression.RHS;
  // traverse the tree of ancestors of this node
  // and mark nodes which are deterministic descendents of RHS
  // and of non-random variables
  // while setting their "value" field to this Var function of them.
  LHS->markRHSandSetKnownValues(RHS);

  Var logp = LHS->logP(conditional_expression.LHS.v,RHS,parameters_to_learn);

  if (clearMarksUponReturn)
    {
      // put the network back in its original state
      LHS->unmarkAncestors();
      for (int i=0;i<RHS.size();i++) RHS[i].V->unmark();
    }

  // make sure that all the paths which do not
  // depend on x, y, and the tunable_parameters are correctly computed
  logp->fprop_from_all_sources();
  return logp;
}

Var ElogP(ConditionalExpression conditional_expression, 
    RVInstanceArray& parameters_to_learn,
    bool clearMarksUponReturn)
{ 
  return logP(conditional_expression,clearMarksUponReturn,&parameters_to_learn);
}


// integrate the RV over the given hiddenRV
// and return the resulting new RandomVariable. This 
// may be difficult to do in general...
RandomVar marginalize(const RandomVar& RV, const RandomVar& hiddenRV)
{ 
  PLERROR("marginalize not implemented yet..."); 
  return RandomVar();
}

// Sample an instance from the given conditional expression,
// of the form (LHS|RHS) where LHS is a RandomVar and
// RHS is a RVInstanceArray, e.g. (X==x && Z==z && W==w).
// THIS IS A VERY INEFFICIENT IMPLEMENTATION IF TO
// BE CALLED MANY TIMES.
Vec sample(ConditionalExpression conditional_expression)
{
  Var instance = Sample(conditional_expression);
  instance->fprop_from_all_sources();
  return instance->value;
}

// Sample N instances from the given conditional expression,
// of the form (LHS|RHS) where LHS is a RandomVar and
// RHS is a RVInstanceArray, e.g. (X==x && Z==z && W==w).
// Put the N instances in the rows of the given Nxd matrix.
// THIS ALSO SHOWS HOW TO REPEATEDLY SAMPLE IN AN EFFICIENT
// MANNER (rather than call "Vec sample(ConditionalExpression)").
void sample(ConditionalExpression conditional_expression,Mat& samples)
{
  if (samples.length()==0) return;
  Var instance = Sample(conditional_expression);
  instance->fprop_from_all_sources();
  samples(0) << instance->value;
  if (samples.length()>0)
    {
      VarArray path;
      instance->random_sources().setMark(); // mark the random sources
      instance->markPath(); // mark successors of the random sources
      instance->buildPath(path); // extract path from the random sources to instance
                                 // and clear marks
      for (int i=1;i<samples.length();i++)
        {
          path.fprop();
          samples(i) << instance->value;
        }
    }
}

// Return a Var which depends functionally on the RHS instances
// and the value of other RandomVars which are non-random and
// influence the LHS.
Var Sample(ConditionalExpression conditional_expression)
{
  RVInstanceArray& RHS = conditional_expression.RHS;
  RandomVar& LHS = conditional_expression.LHS.V;
  LHS->markRHSandSetKnownValues(RHS);
  LHS->unmarkAncestors();
  return LHS->value;
}

// multivariate d-dimensional diagonal normal with NON-RANDOM and CONSTANT 
// parameters (default means = 0, default standard deviations = 1)
RandomVar normal(real mean, real standard_dev, int d,
                 real minimum_standard_deviation)
{
  RandomVar means(d); 
  means->value->value.fill(mean);
  RandomVar logvar(d);
  real variance = standard_dev*standard_dev-
    minimum_standard_deviation*minimum_standard_deviation;
  if (variance<=0)
    PLERROR("normal: variance should be positive");
  logvar->value->value.fill((real)log((double)variance));
  return new DiagonalNormalRandomVariable(means,logvar,
                                          minimum_standard_deviation);
}

// diagonal normal with general parameters
// given by the provided RandomVar's
RandomVar normal(RandomVar mean, RandomVar log_variance,
                 real minimum_standard_deviation)
{
  return new DiagonalNormalRandomVariable(mean,log_variance,
                                          minimum_standard_deviation);
}

RandomVar mixture(RVArray components, RandomVar log_weights)
{
  return new MixtureRandomVariable(components,log_weights);
}

RandomVar multinomial(RandomVar log_probabilities)
{
  return new MultinomialRandomVariable(log_probabilities);
}

/** ConditionalExpression **/

ConditionalExpression::
ConditionalExpression(RVInstance lhs, RVInstanceArray rhs) 
  :LHS(lhs), RHS(rhs) {}

ConditionalExpression::
ConditionalExpression(RVInstance lhs)
  :LHS(lhs), RHS() {}

ConditionalExpression::
ConditionalExpression(RandomVar lhs)
  :LHS(lhs,Var(lhs->length())), RHS() {}

// build from multiple LHS RVInstances: make one RVInstance
// from the joint of the RVs and the vconcat of the instances.
ConditionalExpression::ConditionalExpression(RVInstanceArray lhs)
  :LHS(lhs.random_variables(),vconcat(lhs.instances())), RHS() {}

/** RVInstance **/

RVInstance::RVInstance(const RandomVar& VV, const Var& vv) :V(VV), v(vv) 
{
  if (VV->length()!=vv->length())
    PLERROR("Associating a RandomVar of length %d to a Var of length %d",
          VV->length(),vv->length());
}

RVInstance::RVInstance() {}

RVInstanceArray RVInstance::operator&&(RVInstance rvi)
{
  return RVInstanceArray(*this,rvi);
}

ConditionalExpression RVInstance::operator|(RVInstanceArray a)
{
  return ConditionalExpression(*this,a);
}

// swap the v with the V->value
void RVInstance::swap_v_and_Vvalue()
{ Var tmp = v; v = V->value; V->value = tmp; }


/** RVInstanceArray **/

RVInstanceArray::RVInstanceArray()
  : Array<RVInstance>(0,0)
{}

RVInstanceArray::RVInstanceArray(int n,int n_extra)
  : Array<RVInstance>(n,n_extra)
{}

RVInstanceArray::RVInstanceArray(const Array<RVInstance>& va)
  : Array<RVInstance>(va) {} 

RVInstanceArray::RVInstanceArray(const RVInstance& v, int n_extra)
  : Array<RVInstance>(1,n_extra)
{ (*this)[0] = v; }

RVInstanceArray::RVInstanceArray(const RVInstance& v1, const RVInstance& v2, int n_extra)
  : Array<RVInstance>(2,n_extra)
{ 
  (*this)[0] = v1; 
  (*this)[1] = v2; 
}

RVInstanceArray::RVInstanceArray(const RVInstance& v1, const RVInstance& v2, 
                                 const RVInstance& v3, int n_extra)
: Array<RVInstance>(3,n_extra)
{ 
  (*this)[0] = v1; 
  (*this)[1] = v2; 
  (*this)[2] = v3; 
}

int RVInstanceArray::length() const {
  int l=0;
  for (int i=0;i<size();i++)
    l += (*this)[i].V->length();
  return l;
}

VarArray RVInstanceArray::values() const {
  VarArray vals(size());
  for (int i=0;i<size();i++)
    vals[i]=(*this)[i].V->value;
  return vals;
}

VarArray RVInstanceArray::instances() const {
  VarArray vals(size());
  for (int i=0;i<size();i++)
    vals[i]=(*this)[i].v;
  return vals;
}

RVArray RVInstanceArray::random_variables() const {
  RVArray vars(size());
  for (int i=0;i<size();i++)
    vars[i]=(*this)[i].V;
  return vars;
}

RVInstanceArray RVInstanceArray::operator&&(RVInstance rvi)
{
 return PLearn::operator&(*this,(RVInstanceArray)rvi);
 //return this->operator&((RVInstanceArray)rvi);
}

ConditionalExpression RVInstanceArray::operator|(RVInstanceArray RHS)
{ 
  return ConditionalExpression(RVInstance(random_variables(),
                                          vconcat(instances())),RHS);
}

int RVInstanceArray::compareRVnumbers(const RVInstance* rvi1, 
                                      const RVInstance* rvi2)
{
  return rvi1->V->rv_number - rvi2->V->rv_number;
}

// sorts in-place the elements by V->rv_number (topological order of 
// the graphical model) (in the order: ancestors -> descendants)
void RVInstanceArray::sort()
{
  RVInstance* array = data();
  qsort(array,size(),sizeof(RVInstance),(compare_function)compareRVnumbers);
}

/** RVArray **/

RVArray::RVArray()
  : Array<RandomVar>(0,0)
{}

RVArray::RVArray(int n,int n_extra)
  : Array<RandomVar>(n,n_extra)
{}

RVArray::RVArray(const Array<RandomVar>& va)
  : Array<RandomVar>(va) {} 

RVArray::RVArray(const RandomVar& v, int n_extra)
  : Array<RandomVar>(1,n_extra)
{ (*this)[0] = v; }

RVArray::RVArray(const RandomVar& v1, const RandomVar& v2, int n_extra)
  : Array<RandomVar>(2,n_extra)
{ 
  (*this)[0] = v1; 
  (*this)[1] = v2; 
}

RVArray::RVArray(const RandomVar& v1, const RandomVar& v2, const RandomVar& v3,
                 int n_extra)
: Array<RandomVar>(3,n_extra)
{ 
  (*this)[0] = v1; 
  (*this)[1] = v2; 
  (*this)[2] = v3; 
}

int RVArray::length() const
{
  int l=0;
  for (int i=0;i<size();i++)
    l += (*this)[i]->length();
  return l;
}

VarArray RVArray::values() const
{
  VarArray vals(size());
  for (int i=0;i<size();i++)
    vals[i]=(*this)[i]->value;
  return vals;
}

RandomVar RVArray::operator[](RandomVar index) 
{ return new RVArrayRandomElementRandomVariable(*this, index); }

int RVArray::compareRVnumbers(const RandomVar* v1, const RandomVar* v2)
{
  return (*v1)->rv_number - (*v2)->rv_number;
}

// sorts in-place the elements by rv_number (topological order of 
// the graphical model) (in the order: ancestors -> descendants)
void RVArray::sort()
{
  RandomVar* array = data();
  qsort(array,size(),sizeof(RandomVar),(compare_function)compareRVnumbers);
}

/** StochasticRandomVariable **/

StochasticRandomVariable::StochasticRandomVariable(int length)
  :RandomVariable(length) {}

StochasticRandomVariable::StochasticRandomVariable(const RVArray& parameters,
                                                   int length)
  :RandomVariable(parameters,length)
{}

StochasticRandomVariable::StochasticRandomVariable(const RVArray& parameters,
                                                   int length, int width)
  :RandomVariable(parameters,length,width)
{
}

void StochasticRandomVariable::setKnownValues()
{
  if (!marked && !pmark)
    {
      pmark=true;
      // a StochasticRandomVariable cannot be non-random
      // unless it is a "dirac" distribution (i.e., isNonRandom()==true).
      for (int i=0;i<parents.size();i++)
        parents[i]->setKnownValues();
      setValueFromParentsValue();
      if (isNonRandom()) marked=true;
    }
}

/** FunctionalRandomVariable **/

// these are only used by NonRandomVariable
FunctionalRandomVariable::FunctionalRandomVariable(int thelength)
  :RandomVariable(thelength) {}
FunctionalRandomVariable::FunctionalRandomVariable(int thelength, int thewidth)
  :RandomVariable(thelength,thewidth) {}
FunctionalRandomVariable::FunctionalRandomVariable(const Vec& the_value)
  :RandomVariable(the_value) {}
FunctionalRandomVariable::FunctionalRandomVariable(const Mat& the_value)
  :RandomVariable(the_value) {}
FunctionalRandomVariable::FunctionalRandomVariable(const Var& the_value)
  :RandomVariable(the_value) {}

// only used by the other sub-classes
FunctionalRandomVariable::FunctionalRandomVariable(const RVArray& the_parents,
                                                   int length)
  :RandomVariable(the_parents,length) {}

FunctionalRandomVariable::FunctionalRandomVariable(const RVArray& the_parents,
                                                   int length, int width)
  :RandomVariable(the_parents,length,width) {}

Var FunctionalRandomVariable::logP(const Var& obs, const RVInstanceArray& RHS,
    RVInstanceArray* parameters_to_learn)
{
  // gather the unobserved parents
  int np=parents.size();
  RVInstanceArray unobserved_parents(0,np);
  for (int i=0;i<np;i++)
    if (!(parents[i]->isMarked() || parents[i]->isNonRandom()))
      unobserved_parents &= RVInstance(parents[i],Var(parents[i]->length()));
  // simplest case first
  int nup = unobserved_parents.size();
  if (nup==0)
  {
    if (isDiscrete())
      return isequal(value,obs);
    // else
    return isequal(value,obs)*FLT_MAX;
  }
  // else
  Var *JacobianCorrection=0;
  if (invertible(obs,unobserved_parents,&JacobianCorrection))
  {
    Var logp(1);
    // sort the unobserved parents in topological order of graph
    unobserved_parents.sort();
    bool first = true;
    RVInstanceArray RHS(0,RHS.size()+unobserved_parents.size());
    RVInstanceArray xRHS(RHS);
    for (int i=0;i<nup;i++)
    {
      // note these are still symbolic computations to build Var logp
      if (first)
        logp = unobserved_parents[i].V->logP(unobserved_parents[i].v,xRHS,
            parameters_to_learn);
      else
        logp = logp + 
          unobserved_parents[i].V->logP(unobserved_parents[i].v,xRHS,
              parameters_to_learn);
      first = false;
      // add the visited parents to the RHS, e.g. to compute
      //   P(P1=p1,P2=p2,P3=p3) = P(P1=p1)*P(P2=p2|P1=p1)*P(P3=p3|P2=p2,P1=p1)
      //
      xRHS &= unobserved_parents[i];
    }
    if (JacobianCorrection)
      return logp + *JacobianCorrection;
    return logp;
  }
  // else
  return 
    PLearn::logP(ConditionalExpression
        (RVInstance(marginalize(this, unobserved_parents.random_variables()), obs),
         RHS),true,parameters_to_learn);
}

bool FunctionalRandomVariable::invertible(const Var& obs, 
    RVInstanceArray& unobserved_parents, Var** JacobianCorrection)
{
  PLERROR("FunctionalRandomVariable::invertible() should not be called\n"
        "Either the sub-class should re-implement logP() or re-define\n"
        "invertible() appropriately.");
  return false;
}

// note that stochastic RV's are never non-random,
// but a fonctional RV is non-random if it has
// no parents (i.e., it is a NonRandomVariable) or if all its
// parents are non-random. Note also that the marked field
// sometimes means conditionally non-random during calls
// to functions such as logP.
bool FunctionalRandomVariable::isNonRandom()
{ 
  bool non_random=true;
  for (int i=0;i<parents.size() && non_random;i++)
    non_random = parents[i]->isNonRandom();
  return non_random;
}

bool FunctionalRandomVariable::isDiscrete()
{
  bool all_discrete = true;
  for (int i=0;i<parents.size() && all_discrete;i++)
    all_discrete = parents[i]->isDiscrete();
  return all_discrete;
}


/** NonRandomVariable **/

NonRandomVariable::NonRandomVariable(int thelength)
  :FunctionalRandomVariable(thelength) {}

NonRandomVariable::NonRandomVariable(int thelength, int thewidth)
  :FunctionalRandomVariable(thelength,thewidth) {}

NonRandomVariable::NonRandomVariable(const Var& v)
  :FunctionalRandomVariable(v) {}

/** JointRandomVariable **/

JointRandomVariable::JointRandomVariable(const RVArray& variables)
  :FunctionalRandomVariable(variables,variables.length()) 
{ 
  if (variables.size()==0)
    PLERROR("JointRandomVariables(RVArray) expects an array with >0 elements");
}

void JointRandomVariable::setValueFromParentsValue() 
{
  if (marked) return;
  VarArray values(parents.size());
  for (int i=0;i<parents.size();i++)
    values[i]=parents[i]->value;
  value = vconcat(values);
}

bool JointRandomVariable::invertible(const Var& obs, 
    RVInstanceArray& unobserved_parents, Var** JacobianCorrection)
{
  int p=0;
  int j=0;
  int nun=unobserved_parents.size();
  for (int i=0;i<parents.size();i++)
    {
      if (j==nun)
        PLERROR("JointRandomVariable::invertible ==> logic error");
      int l = parents[i]->length();
      if (unobserved_parents[j].V==parents[i])
        unobserved_parents[j++].v = obs->subVec(p,l);
      p+=l;
    }
  return true;
}

void JointRandomVariable::EMBprop(const Vec obs, real posterior)
{
  int p=0;
  for (int i=0;i<parents.size();i++)
    {
      int l = parents[i]->length();
      // watch for redundant computation!
      parents[i]->EMBprop(obs.subVec(p,l),posterior);
      p+=l;
    }
}

/** RandomElementOfRandomVariable **/

RandomElementOfRandomVariable::RandomElementOfRandomVariable(const RandomVar& v, 
                                               const RandomVar& index)
  :FunctionalRandomVariable(v&index,1)
{ 
  if (index->length()!=1)
    PLERROR("RandomElementOfRandomVariables expects an index RandomVar of length 1");
}

void RandomElementOfRandomVariable::setValueFromParentsValue() 
{
  if (marked) return;
  value = v()->value[index()->value];
}

bool RandomElementOfRandomVariable::
invertible(const Var& obs, 
           RVInstanceArray& unobserved_parents,
           Var** JacobianCorrection)
{
  // IT IS POSSIBLE TO COMPUTE A LOGP WITHOUT INTEGRATING
  // IF v() IS OBSERVED
  if (v()==unobserved_parents[0].V && unobserved_parents.size()==1)
    PLERROR("RandomElementOFRandomVariable could compute logP but not implemented");
  return false;
}

void RandomElementOfRandomVariable::EMBprop(const Vec obs, real posterior)
{
}

/** RVArrayRandomElementRandomVariable **/

RVArrayRandomElementRandomVariable::
RVArrayRandomElementRandomVariable(const RVArray& table, const RandomVar& index)
  :FunctionalRandomVariable((RVArray)(table&index),table[0]->length())
{ 
  int l=table[0]->length();
  for (int i=1;i<table.size();i++)
    if (table[i]->length()!=l)
      PLERROR("RVArrayRandomElementRandomVariables expect all the elements of table\n"
            " to have the same length (%-th has length %d while 0-th has length %d",
            i,table[i]->length(),l);
  if (index->length()!=1)
    PLERROR("RVArrayRandomElementRandomVariables expect an index RandomVar of length 1");
}

void RVArrayRandomElementRandomVariable::setValueFromParentsValue() 
{
  if (marked) return;
  int n = parents.size()-1;
  VarArray parents_values(n);
  for (int i=0;i<n;i++)
    parents_values[i]=parents[i]->value;
  value = parents_values[index()->value];
}

Var RVArrayRandomElementRandomVariable::
logP(const Var& obs, const RVInstanceArray& RHS,
     RVInstanceArray* parameters_to_learn)
{
  int n = parents.size()-1;
  if (index()->isMarked() || index()->isNonRandom())
    // special case where index is observed, just pass to selected parent
    {
      VarArray parents_logp(n);
      for (int i=0;i<n;i++)
        parents_logp[i]=parents[i]->logP(obs,RHS,parameters_to_learn);
      return parents_logp[index()->value];
    }
  // otherwise, build a Mixture, with log_weights = logP(index()==i)
  VarArray log_weights(n);
  RVArray components(n);
  for (int i=0;i<n;i++)
    {
      Var indx(1);
      indx = (real)i;
      log_weights[i] = index()->logP(indx,RHS,parameters_to_learn);
      components[i] = parents[i];
    }
  RandomVar mixt = mixture(components,RandomVar(vconcat(log_weights)));
  return mixt->logP(obs,RHS,parameters_to_learn);
}

void RVArrayRandomElementRandomVariable::EMBprop(const Vec obs, real posterior)
{
}


/** NegRandomVariable **/

NegRandomVariable::NegRandomVariable(RandomVariable* input)
  :FunctionalRandomVariable(input->length()) {}

void NegRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = -parents[0]->value;
}

bool NegRandomVariable::invertible(const Var& obs, 
                                      RVInstanceArray& unobserved_parents,
                                      Var** JacobianCorrection)
{
  unobserved_parents[0].v = -obs;
  return true;
}

void NegRandomVariable::EMBprop(const Vec obs, real posterior)
{
  if (!parents[0]->isConstant())
    parents[0]->EMBprop(-obs,posterior);
}

/** ExpRandomVariable **/

ExpRandomVariable::ExpRandomVariable(RandomVar& input)
  :FunctionalRandomVariable(input,input->length()) {}

void ExpRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = exp(parents[0]->value);
}

bool ExpRandomVariable::invertible(const Var& obs, 
                                      RVInstanceArray& unobserved_parents,
                                      Var** JacobianCorrection)
{
  unobserved_parents[0].v = log(obs);
  return true;
}

void ExpRandomVariable::EMBprop(const Vec obs, real posterior)
{
  if (!parents[0]->isConstant())
    parents[0]->EMBprop(log(obs),posterior);
}

/** LogRandomVariable **/

LogRandomVariable::LogRandomVariable(RandomVar& input)
  :FunctionalRandomVariable(input,input->length()) {}

void LogRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = log(parents[0]->value);
}

bool LogRandomVariable::invertible(const Var& obs, 
                                      RVInstanceArray& unobserved_parents,
                                      Var** JacobianCorrection)
{
  unobserved_parents[0].v = exp(obs);
  return true;
}

void LogRandomVariable::EMBprop(const Vec obs, real posterior)
{
  if (!parents[0]->isConstant())
    parents[0]->EMBprop(exp(obs),posterior);
}

/** RandomVariable Plus **/

PlusRandomVariable::PlusRandomVariable(RandomVar input1, RandomVar input2)
  : FunctionalRandomVariable(input1 & input2, 
                             MAX(input1->length(),input2->length())),
    parent_to_learn(parents[0]), other_parent(parents[0]),
    numerator(value->length()), difference(value->length())
{
  if(input1->length() != input2->length() &&
     input1->length() !=1 && input2->length()!=1)
    PLERROR("PlusRandomVariable(RandomVariable* in1, RandomVariable* in2) in1 and"
          "in2 must have the same length or one of them must be of length 1");
}
  
void PlusRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = X0()->value + X1()->value;
}

bool PlusRandomVariable::invertible(const Var& obs, 
                                       RVInstanceArray& unobserved_parents,
                                       Var** JacobianCorrection)
{
  if (unobserved_parents.size()==2)
    return false; // can't invert if two parents are unobserved
  if (unobserved_parents[0].V == X0())
    unobserved_parents[0].v = obs - X1()->value;
  else
    unobserved_parents[0].v = obs - X0()->value;
  return true;
    
}

void PlusRandomVariable::
EMTrainingInitialize(const RVArray& parameters_to_learn)
{
  RandomVariable::EMTrainingInitialize(parameters_to_learn);
  if (learn_X0() && learn_X1())
    PLERROR("PlusRandomVariable: can't learn both X0 and X1");
  if (learn_X0() || learn_X1())
    {
      learn_something=true;
      if (learn_X0())
        {
          parent_to_learn = X0();
          other_parent = X1();
        }
      else
        {
          parent_to_learn = X1();
          other_parent = X0();
        }
    }
}

void PlusRandomVariable::EMEpochInitialize()
{
  if (EMmark) return;
  RandomVariable::EMEpochInitialize();
  if (learn_something)
    {
      numerator.clear();
      denom = 0.;
    }
}

void PlusRandomVariable::EMBprop(const Vec obs, real posterior)
{
  if (learn_something)
    {
      // numerator += posterior * (obs - other_parent->value->value);
      substract(obs,other_parent->value->value,difference);
      multiplyAcc(numerator, difference,posterior);
      denom += posterior;
      if (!other_parent->isConstant())
        {
          // propagate to other parent
          substract(obs,parent_to_learn->value->value,difference);
          other_parent->EMBprop(difference,posterior);
        }
    }
  else
    {
      if (!X1()->isConstant())
        {
          substract(obs,X0()->value->value,difference);
          X1()->EMBprop(difference,posterior);
        }
      if (!X0()->isConstant())
        {
          substract(obs,X1()->value->value,difference);
          X0()->EMBprop(difference,posterior);
        }
    }
}

void PlusRandomVariable::EMUpdate()
{
  if (EMmark) return;
  EMmark=true;
  if (learn_something && denom>0)
    // new value = numerator / denom
    multiply(numerator,real(1.0/denom),parent_to_learn->value->value);
  if (!learn_X0() && !X0()->isConstant())
    X0()->EMUpdate();
  if (!learn_X1() && !X1()->isConstant())
    X1()->EMUpdate();
}

/** RandomVariable Minus **/

MinusRandomVariable::MinusRandomVariable(RandomVar input1, RandomVar input2)
  : FunctionalRandomVariable(input1 & input2, 
                             MAX(input1->length(),input2->length())),
    parent_to_learn(parents[0]), other_parent(parents[0]),
    numerator(value->length()), difference(value->length())
{
  if(input1->length() != input2->length() &&
     input1->length() !=1 && input2->length()!=1)
    PLERROR("MinusRandomVariable(RandomVariable* in1, RandomVariable* in2) in1 and"
          "in2 must have the same length or one of them must be of length 1");
}
  
void MinusRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = X0()->value - X1()->value;
}

bool MinusRandomVariable::invertible(const Var& obs, 
                                       RVInstanceArray& unobserved_parents,
                                       Var** JacobianCorrection)
{
  if (unobserved_parents.size()==2)
    return false; // can't invert if two parents are unobserved
  if (unobserved_parents[0].V == X0())
    unobserved_parents[0].v = obs + X1()->value;
  else
    unobserved_parents[0].v = X0()->value - obs;
  return true;
    
}

void MinusRandomVariable::
EMTrainingInitialize(const RVArray& parameters_to_learn)
{
  RandomVariable::EMTrainingInitialize(parameters_to_learn);
  if (learn_X0() && learn_X1())
    PLERROR("MinusRandomVariable: can't learn both X0 and X1");
  if (learn_X0() || learn_X1())
    {
      learn_something=true;
      if (learn_X0())
        {
          parent_to_learn = X0();
          other_parent = X1();
        }
      else
        {
          parent_to_learn = X1();
          other_parent = X0();
        }
    }
}

void MinusRandomVariable::EMEpochInitialize()
{
  if (EMmark) return;
  RandomVariable::EMEpochInitialize();
  if (learn_something)
    {
      numerator.clear();
      denom = 0.0;
    }
}

void MinusRandomVariable::EMBprop(const Vec obs, real posterior)
{
  if (learn_something)
  {
    if (learn_X0())
      // numerator += posterior * (obs + other_parent->value->value);
      add(obs,other_parent->value->value,difference);
    else
      // numerator += posterior * (other_parent->value->value - obs);
      substract(other_parent->value->value,obs,difference);

    multiplyAcc(numerator, difference,posterior);
    denom += posterior;
    if (!other_parent->isConstant())
    {
      // propagate to other parent
      if (learn_X0())
        add(obs,parent_to_learn->value->value,difference);
      else
        substract(parent_to_learn->value->value,obs,difference);
      other_parent->EMBprop(difference,posterior);
    }
  }
  else
  {
    if (!X1()->isConstant())
    {
      substract(X0()->value->value,obs,difference);
      X1()->EMBprop(difference,posterior);
    }
    if (!X0()->isConstant())
    {
      add(obs,X1()->value->value,difference);
      X0()->EMBprop(difference,posterior);
    }
  }
}

void MinusRandomVariable::EMUpdate()
{
  if (EMmark) return;
  EMmark=true;
  if (learn_something && denom>0)
    // new value = numerator / denom
    multiply(numerator,real(1.0/denom),parent_to_learn->value->value);
  if (!learn_X0() && !X0()->isConstant())
    X0()->EMUpdate();
  if (!learn_X1() && !X1()->isConstant())
    X1()->EMUpdate();
}

/** RandomVariable ElementWiseDivision **/

ElementWiseDivisionRandomVariable::
ElementWiseDivisionRandomVariable(RandomVar input1, RandomVar input2)
  : FunctionalRandomVariable(input1 & input2, 
                             MAX(input1->length(),input2->length()))
{
  if(input1->length() != input2->length() &&
     input1->length() !=1 && input2->length()!=1)
    PLERROR("ElementWiseDivisionRandomVariable(RandomVariable* in1, RandomVariable* in2) in1 and"
          "in2 must have the same length or one of them must be of length 1");
}
  
void ElementWiseDivisionRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = X0()->value / X1()->value;
}

bool ElementWiseDivisionRandomVariable::invertible(const Var& obs, 
                                       RVInstanceArray& unobserved_parents,
                                       Var** JacobianCorrection)
{
  if (unobserved_parents.size()==2)
    return false; // can't invert if two parents are unobserved
  if (unobserved_parents[0].V == X0())
    unobserved_parents[0].v = obs * X1()->value;
  else
    unobserved_parents[0].v = X0()->value / obs;
  return true;
}

void ElementWiseDivisionRandomVariable::
EMTrainingInitialize(const RVArray& parameters_to_learn)
{
}

void ElementWiseDivisionRandomVariable::EMEpochInitialize()
{
}

void ElementWiseDivisionRandomVariable::EMBprop(const Vec obs, real posterior)
{
}

void ElementWiseDivisionRandomVariable::EMUpdate()
{
  PLERROR("ElementWiseDivisionRandomVariable::EMUpdate() not implemented");
}

/** RandomVariable Product **/

ProductRandomVariable::ProductRandomVariable(RandomVar input1, 
                                             RandomVar input2)
  : FunctionalRandomVariable(input1 & input2, input1->value->matValue.length(),
                             input2->value->matValue.width()),
    m(input1->value->matValue.length()), n(input1->value->matValue.width()),
    l(input2->value->matValue.width()), learn_something(false)
{
  if (n != input2->value->matValue.length())
    PLERROR("ProductRandomVariable(X0,X1): X0(%d,%d)'s width (%d) must match"
          "X1(%d,%d)'s length (%d)", input1->value->matValue.length(),
          input1->value->matValue.width(), input1->value->matValue.width(),
          input2->value->matValue.length(), input2->value->matValue.width(),
          input2->value->matValue.length());
  scalars = (m==1 && n==1 && l==1);
}

void ProductRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = Var(new ProductVariable(X0()->value,
                                  X1()->value));
}

bool ProductRandomVariable::invertible(const Var& obs, 
                                          RVInstanceArray& unobserved_parents,
                                          Var** JacobianCorrection)
{
  if (unobserved_parents.size()==2)
    return false; // can't invert if two parents are unobserved
  if (unobserved_parents[0].V == X0())
    {
      unobserved_parents[0].v = 
        product(obs,rightPseudoInverse(X1()->value));
      **JacobianCorrection = log(abs(det(X1()->value)));
    }
  else
    {
      unobserved_parents[0].v = 
        product( leftPseudoInverse(X0()->value), obs);
      **JacobianCorrection = log(abs(det(X0()->value)));
    }
  return true;
    
}

void ProductRandomVariable::
EMTrainingInitialize(const RVArray& parameters_to_learn)
{
  RandomVariable::EMTrainingInitialize(parameters_to_learn);
  if (learn_X0() && learn_X1())
    PLERROR("ProductRandomVariable: can't learn both X0 and X1");
  if (learn_X0() || learn_X1())
    {
      denom.resize(n,n);
      tmp2.resize(n,n);
      tmp4.resize(n);
      learn_something=true;
      if (learn_X0())
        {
          X0numerator.resize(m,n);
          tmp1.resize(m,n);
          if (!X1()->isNonRandom())
            {
              tmp3.resize(m,l);
              vtmp3 = tmp3.toVec();
            }
        }
      else
        {
          X1numerator.resize(n,l);
          tmp1.resize(n,l);
          if (!X0()->isNonRandom())
            {
              tmp3.resize(n,m);
              vtmp3 = tmp3.toVec();
            }
        }
    }
  else
    learn_something=false;
}

void ProductRandomVariable::EMEpochInitialize()
{
  if (EMmark) return;
  RandomVariable::EMEpochInitialize();
  if (learn_something)
    {
      denom.clear();
      if (learn_X0())
        X0numerator.clear();
      else
        X1numerator.clear();
    }
}

void ProductRandomVariable::EMBprop(const Vec obs, real posterior)
{
  if (learn_something)
    {
      if (learn_X0())
        {
          if (scalars)
            {
              // do the special scalar case separately for efficiency
              real x1 = *(X1()->value->value.data());
              real y = *obs.data();
              *X0numerator.data() += posterior * y * x1;
              *denom.data() += posterior * x1 * x1;
              // propagate EMBprop to X1
              if (!X1()->isNonRandom())
                {
                  real x0 = *(X0()->value->value.data());
                  if (x0==0.0)
                    PLERROR("ProductRandomVariable: can't divide by X0==0");
                  *tmp3.data() = y / x0;
                  X1()->EMBprop(vtmp3,posterior);
                }
            }
          else
            {
              Mat matObs = obs.toMat(m,l);
              Mat& x1 = X1()->value->matValue;
              // numerator += posterior * obs * x1'
              productTranspose(tmp1, matObs,x1);
              multiplyAcc(X0numerator, tmp1,posterior);
              // denominator += posterior * x1 * x1'
              productTranspose(tmp2, x1,x1);
              multiplyAcc(denom, tmp2,posterior);
              // propagate EMBprop to X1
              if (!X1()->isNonRandom())
                {
                  Mat& x0 = X0()->value->matValue;
                  // solve x0 * tmp3 = matObs
                  solveLinearSystem(x0,matObs,tmp3); 
                  X1()->EMBprop(vtmp3,posterior);
                }
            }
        }
      else // learn_X1()
        {
          if (scalars)
            {
              // do the special scalar case separately for efficiency
              real x0 = *(X0()->value->value.data());
              *X1numerator.data() += posterior * *obs.data() * x0;
              *denom.data() += posterior * x0 * x0;
              // propagate EMBprop to X0
              if (!X0()->isNonRandom())
                {
                  real x1 = *(X1()->value->value.data());
                  if (x1==0.0)
                    PLERROR("ProductRandomVariable: can't divide by X1==0");
                  *tmp3.data() = obs[0] / x1;
                  X0()->EMBprop(vtmp3,posterior);
                }
            }
          else
            {
              Mat matObs = obs.toMat(m,l);
              Mat& x0 = X0()->value->matValue;
              // numerator += posterior * x0' * obs
              transposeProduct(tmp1, x0,matObs);
              multiplyAcc(X1numerator, tmp1,posterior);
              // denominator += posterior * x0' * x0
              transposeProduct(tmp2, x0,x0);
              multiplyAcc(denom, tmp2,posterior);
              // propagate EMBprop to X0
              if (!X0()->isNonRandom())
                {
                  Mat& x1 = X1()->value->matValue;
                  // solve tmp3 * x1 = matObs
                  solveTransposeLinearSystem(x1,matObs,tmp3); 
                  X1()->EMBprop(vtmp3,posterior);
                }
            }
        }
    }
  else
    {
      if (scalars)
        {
          if (!X1()->isNonRandom())
            {
              real x0 = *(X0()->value->value.data());
              if (x0==0.0)
                PLERROR("ProductRandomVariable: can't divide by X0==0");
              *tmp3.data() = obs[0] / x0;
              X1()->EMBprop(vtmp3,posterior);
            }
          if (!X0()->isNonRandom())
            {
              real x1 = *(X1()->value->value.data());
              if (x1==0.0)
                PLERROR("ProductRandomVariable: can't divide by X1==0");
              *tmp3.data() = obs[0] / x1;
              X0()->EMBprop(vtmp3,posterior);
            }
        }
      else
        {
          if (!X1()->isConstant())
            {
              Mat matObs = obs.toMat(m,l);
              Mat& x0 = X0()->value->matValue;
              solveLinearSystem(x0,matObs,tmp3); // solve x0 * tmp3 = matObs
              X1()->EMBprop(vtmp3,posterior);
            }
          if (!X0()->isConstant())
            {
              Mat matObs = obs.toMat(m,l);
              Mat& x1 = X1()->value->matValue;
              // solve tmp3 * x1 = matObs
              solveTransposeLinearSystem(x1,matObs,tmp3); 
              X1()->EMBprop(vtmp3,posterior);
            }
        }
    }
}

void ProductRandomVariable::EMUpdate()
{
  if (EMmark) return;
  EMmark=true;
  if (learn_something)
    {
      if (learn_X0())
        {
          if (scalars)
            {
              if (denom(0,0)>0)
                X0()->value->value[0] = X0numerator(0,0)/denom(0,0);
            }
          else
            solveTransposeLinearSystemByCholesky(denom,X0numerator,
                                                 X0()->value->matValue,
                                                 &tmp2,&tmp4);
          if (!X1()->isConstant())
            X1()->EMUpdate();
        }
      else // learn_X1()
        {
          if (scalars)
            {
              if (denom(0,0)>0)
                X1()->value->value[0] = X1numerator(0,0)/denom(0,0);
            }
          else
            solveLinearSystemByCholesky(denom,X1numerator,
                                        X1()->value->matValue,
                                        &tmp2,&tmp4);
          if (!X0()->isConstant())
            X0()->EMUpdate();
        }
    }
  else 
    {
      if (!X0()->isConstant())
        X0()->EMUpdate();
      if (!X1()->isConstant())
        X1()->EMUpdate();
    }
}


/** DiagonalNormalRandomVariable **/
   
DiagonalNormalRandomVariable::DiagonalNormalRandomVariable
(const RandomVar& mean, const RandomVar& log_var, 
 real minimum_standard_deviation)
 :StochasticRandomVariable(mean & log_var, mean->length()),
  minimum_variance(minimum_standard_deviation*minimum_standard_deviation),
  normfactor(mean->length()*Log2Pi), shared_variance(log_var->length()==1),
  mu_num(mean->length()), sigma_num(log_var->length())
{
}

Var DiagonalNormalRandomVariable::logP(const Var& obs, 
    const RVInstanceArray& RHS, RVInstanceArray* parameters_to_learn)
{
  if (mean()->isMarked() && log_variance()->isMarked())
  {
    if (log_variance()->value->getName()[0]=='#') 
      log_variance()->value->setName("log_variance");
    if (mean()->value->getName()[0]=='#') 
      mean()->value->setName("mean");
    Var variance = minimum_variance+exp(log_variance()->value);
    variance->setName("variance");
    if (shared_variance)
      return (-0.5)*(sum(square(obs-mean()->value))/variance+
          (mean()->length())*log(variance) + normfactor);
    else
      return (-0.5)*(sum(square(obs-mean()->value)/variance)+
          sum(log(variance))+ normfactor);
  }
  // else
  // probably not feasible..., but try in case we know a trick
  if (mean()->isMarked())
    return 
      PLearn::logP(ConditionalExpression(RVInstance(marginalize(this,
                log_variance()),
              obs),RHS),true,parameters_to_learn); 
  else
    return PLearn::logP(ConditionalExpression(RVInstance(marginalize(this,mean()),
            obs),RHS),true,parameters_to_learn); 
}

void DiagonalNormalRandomVariable::setValueFromParentsValue()
{
  value = 
    Var(new DiagonalNormalSampleVariable(mean()->value,
                                         sqrt(minimum_variance+
                                              exp(log_variance()->value))));
}

void DiagonalNormalRandomVariable::EMEpochInitialize()
{
  if (EMmark) return;
  RandomVariable::EMEpochInitialize();
  if (learn_the_mean())
    mu_num.clear();
  if (learn_the_variance())
    sigma_num.clear();
  denom = 0.0;
}

void DiagonalNormalRandomVariable::EMBprop(const Vec obs, real posterior)
{
  if (learn_the_mean())
    multiplyAcc(mu_num, obs,posterior);
  else if (!mean()->isConstant())
    {
      if (!shared_variance)
        PLERROR("DiagonalNormalRandomVariable: don't know how to EMBprop "
              "into mean if variance is not shared");
      mean()->EMBprop(obs,posterior/
                      (minimum_variance+exp(log_variance()->value->value[0])));
    }
  if (learn_the_variance())
    {
      if (learn_the_mean())
        {
          // sigma_num[i] += obs[i]*obs[i]*posterior
          if (shared_variance)
            sigma_num[0] += posterior*pownorm(obs)/mean()->length();
          else
            squareMultiplyAcc(sigma_num, obs,posterior);
        }
      else
        {
        // sigma_num[i] += (obs[i]-mean[i])^2*posterior
          if (shared_variance)
            sigma_num[0] += posterior*powdistance(obs,mean()->value->value)
              /mean()->length();
          else
            diffSquareMultiplyAcc(sigma_num, obs,
                                            mean()->value->value,
                                            posterior);
        }
    }
  else if (!log_variance()->isConstant())
    {
      // use sigma_num as a temporary for log_var's observation
      if (shared_variance)
        log_variance()->EMBprop(Vec(1,powdistance(obs,mean()->value->value)
                                    /mean()->length()),
                                posterior);
      else
        {
          substract(obs,mean()->value->value,sigma_num);
          apply(sigma_num,sigma_num,square_f);
          log_variance()->EMBprop(sigma_num,posterior);
        }
    }
  if (learn_the_mean() || learn_the_variance()) denom += posterior;
}

void DiagonalNormalRandomVariable::EMUpdate()
{
  if (EMmark) return;
  EMmark=true;
  // maybe we should issue a warning if
  // (learn_the_mean || learn_the_variance) && denom==0
  // (it means that all posteriors reaching EMBprop were 0)
  //
  if (denom>0 && (learn_the_mean() || learn_the_variance()))
    {
      Vec lv = log_variance()->value->value;
      Vec mv = mean()->value->value;
      if (learn_the_mean())
        multiply(mu_num,real(1.0/denom),mv);
      if (learn_the_variance())
        {
          if (learn_the_mean())
            {
              // variance = sigma_num/denom - squared(mean)
              sigma_num /= denom;
              multiply(mv,mv,mu_num); // use mu_num as a temporary vec
              if (shared_variance)
                lv[0] = sigma_num[0] - PLearn::mean(mu_num);
              else
                substract(sigma_num,mu_num,lv);
              // now lv really holds variance

              // log_variance = log(max(0,variance-minimum_variance))
              substract(lv,minimum_variance,lv);
              max(lv,real(0.),lv);
              apply(lv,lv,safeflog);
            }
          else
            {
              multiply(sigma_num,1/denom,lv);
              // now log_variance really holds variance

              // log_variance = log(max(0,variance-minimum_variance))
              substract(lv,minimum_variance,lv);
              max(lv,real(0.),lv);
              apply(lv,lv,safeflog);
            }
        }
    }
  if (!learn_the_mean() && !mean()->isConstant())
    mean()->EMUpdate();
  if (!learn_the_variance() && !log_variance()->isConstant())
    log_variance()->EMUpdate();
}


/** MixtureRandomVariable **/

MixtureRandomVariable::MixtureRandomVariable
(const RVArray& the_components,const RandomVar& logweights)
  :StochasticRandomVariable(logweights,the_components[0]->length()),
   components(the_components), posteriors(logweights->length()),
   sum_posteriors(logweights->length()), 
   componentsLogP(logweights->length()),
   lw(logweights->length())
{
}

Var MixtureRandomVariable::logP(const Var& obs, const RVInstanceArray& RHS,
    RVInstanceArray* parameters_to_learn)
{
  if (parameters_to_learn!=0) return ElogP(obs,*parameters_to_learn,RHS);
  if (log_weights()->isMarked())
    {
      int n=posteriors.length();
      if (log_weights()->value->getName()[0]=='#') 
        log_weights()->value->setName("log_weights");
      Var weights = softmax(log_weights()->value);
      weights->setName("weights");
      lw = log(weights);
      for (int i=0;i<n;i++)
        componentsLogP[i] = components[i]->logP(obs,RHS) + lw->subVec(i,1);
      logp = logadd(vconcat(componentsLogP));
      return logp;
    }
  // else
  // probably not feasible..., but try in case we know a trick
  return PLearn::logP(ConditionalExpression
                (RVInstance(marginalize(this,log_weights()),obs),RHS),true,0);
}

// compute symbolically 
//   E[ logP(obs,i|new_params) | old_params,obs ]
// where the expectation is over the mixture component index i,
// with "posterior" probabilities P(i|obs,old_params).
// The parameters_to_learn[i].v hold the "new parameters" (to be optimized)
// while the parameters_to_learn[i].V->value hold the "current value"
// of the parameters for the EM algorithm.
Var MixtureRandomVariable::ElogP(const Var& obs, 
                                 RVInstanceArray& parameters_to_learn,
                                 const RVInstanceArray& RHS)
{
  if (log_weights()->isMarked())
    {
      int n=posteriors.length();

      // (1) using the "current" value of the parameters
      Var weights = softmax(log_weights()->value);
      lw = log(weights);
      for (int i=0;i<n;i++)
        // componentsLogP[i] = log(P(obs|i)*P(i)) = log(P(obs,i))
        componentsLogP[i] = components[i]->logP(obs,RHS) + lw->subVec(i,1);
      // logp = log(P(obs)) = log(sum_i P(obs,i))
      logp = logadd(vconcat(componentsLogP));
      // now compute log-posteriors by normalization
      for (int i=0;i<n;i++)
        // componentsLogP[i] = log(P(i|obs))=log(P(obs,i)/P(obs))
        componentsLogP[i] = componentsLogP[i] - logp;

      // (2) now put the "new" value of the parameters (swap with v fields)
      parameters_to_learn.swap_v_and_Vvalue();
      // unmark parents and re-compute value's in terms of ancestors' values
      unmarkAncestors();
      markRHSandSetKnownValues(RHS);
     
      // (3) and compute the  logP of each component weighted by its posterior
      weights = softmax(log_weights()->value);
      for (int i=0;i<n;i++)
        componentsLogP[i] = exp(components[i]->logP(obs,RHS,&parameters_to_learn) 
                                + componentsLogP[i]);
      logp = sum(vconcat(componentsLogP));

      // (4) now put back the "current" value of parameters in their value field
      parameters_to_learn.swap_v_and_Vvalue();
      // unmark parents and re-compute value's in terms of ancestors' values
      unmarkAncestors();
      markRHSandSetKnownValues(RHS);

      return logp;
    }
  // else
  // probably not feasible..., but try in case we know a trick
  return PLearn::logP(ConditionalExpression
      (RVInstance(marginalize(this,log_weights()),obs),
       RHS),true,&parameters_to_learn);
}

void MixtureRandomVariable::setValueFromParentsValue()
{
  Var index = new MultinomialSampleVariable(softmax(log_weights()->value));
  value = components.values()[index]; 
}

void MixtureRandomVariable::
EMTrainingInitialize(const RVArray& parameters_to_learn)
{
  if (EMmark) return;
  EMmark=true;
  learn_the_weights() = parameters_to_learn.contains(log_weights())
    && log_weights()->isConstant();
  for (int i=0;i<components.size();i++)
    components[i]->EMTrainingInitialize(parameters_to_learn);
}

void MixtureRandomVariable::EMEpochInitialize()
{
  if (EMmark) return;
  RandomVariable::EMEpochInitialize();
  if (learn_the_weights())
    sum_posteriors.clear();
  for (int i=0;i<components.size();i++)
    components[i]->EMEpochInitialize();
}

void MixtureRandomVariable::EMBprop(const Vec obs, real posterior)
{
  // ASSUME THAT AN FPROP HAS BEEN PERFORMED
  // so that weights and componentsLogP hold appropriate value
  //
  // compute posterior vector for this observation
  // posteriors = posterior*(components[i]->logP(obs)*weights/normalize)
  real log_p = logp->value[0];
  real *p = posteriors.data();
  int n = lw->value.length();
  for (int i=0;i<n;i++)
    p[i] = componentsLogP[i]->value[0] - log_p;
#ifdef _MSC_VER
  apply(posteriors,posteriors,(tRealFunc)exp);
#else
  apply(posteriors,posteriors,safeexp);
#endif
  if (fabs(sum(posteriors)-1)>1e-5)
    {
      cout << "sum(posteriors) = " << sum(posteriors) << "!" << endl;
    }
  posteriors *= posterior;

  if (learn_the_weights())
    sum_posteriors+=posteriors; 

  // propagate to components
  for (int i=0;i<n;i++)
    components[i]->EMBprop(obs,posteriors[i]);
}

void MixtureRandomVariable::EMUpdate()
{
  if (EMmark) return;
  EMmark=true;
  // update weights
  if (learn_the_weights())
    {
      real denom = sum(sum_posteriors);
      if (denom>0)
        {
          multiply(sum_posteriors,real(1.0/denom),posteriors);
          apply(posteriors,log_weights()->value->value,safeflog);
        }
    }
  // propagate to components
  for (int i=0;i<components.size();i++)
    components[i]->EMUpdate();
}

bool MixtureRandomVariable::canStopEM()
{
  // propagate to components
  bool can=log_weights()->canStopEM();
  for (int i=0;i<components.size() && !can;i++)
    can = components[i]->canStopEM();
  return can;
}

bool MixtureRandomVariable::isDiscrete()
{
  return components[0]->isDiscrete();
}

void MixtureRandomVariable::setKnownValues()
{
  if (!pmark && !marked)
    {
      pmark = true;
      log_weights()->setKnownValues();
      for (int i=0;i<components.size();i++)
        components[i]->setKnownValues();
      setValueFromParentsValue();
    }
}
 
void MixtureRandomVariable::unmarkAncestors() 
{ 
  if (pmark)
    {
      marked=false; 
      pmark=false;
      log_weights()->unmarkAncestors();
      for (int i=0;i<components.size();i++)
        components[i]->unmarkAncestors();
    }
}

void MixtureRandomVariable::clearEMmarks() 
{ 
  if (EMmark)
    {
      EMmark=false; 
      log_weights()->clearEMmarks();
      for (int i=0;i<components.size();i++)
        components[i]->clearEMmarks();
    }
}

/** MultinomialRandomVariable **/

MultinomialRandomVariable::
MultinomialRandomVariable(const RandomVar& log_probabilities)
  :StochasticRandomVariable(log_probabilities,1),
   sum_posteriors(log_probabilities->length())
{
}

Var MultinomialRandomVariable::logP(const Var& obs, const RVInstanceArray& RHS,
    RVInstanceArray* parameters_to_learn)
{
  if (log_probabilities()->isMarked())
    return log(softmax(log_probabilities()->value))[obs];
  // else
  // probably not feasible..., but try in case we know a trick
  return PLearn::logP(ConditionalExpression
      (RVInstance(marginalize(this,log_probabilities()),obs),
       RHS),true,parameters_to_learn); 
}

void MultinomialRandomVariable::EMEpochInitialize()
{
  if (EMmark) return;
  RandomVariable::EMEpochInitialize();
  if (learn_the_probabilities())
    sum_posteriors.clear();
}

void MultinomialRandomVariable::EMBprop(const Vec obs, real posterior)
{
  if (learn_the_probabilities())
    {
      real *p = sum_posteriors.data();
      p[(int)obs[0]] += posterior;
    }
}

void MultinomialRandomVariable::EMUpdate()
{
  if (EMmark) return;
  EMmark=true;
  if (learn_the_probabilities())
    {
      real denom = sum(sum_posteriors);
      if (denom>0)
        // update probabilities
        {
          multiply(sum_posteriors,real(1.0/denom),sum_posteriors);
          apply(sum_posteriors,log_probabilities()->value->value,safeflog);
        }
      // maybe should WARN the user if denom==0 here
    }
}

bool MultinomialRandomVariable::isDiscrete()
{
  return true;
}

void MultinomialRandomVariable::setValueFromParentsValue()
{
  value = 
    Var(new MultinomialSampleVariable(softmax(log_probabilities()->value)));
}

/** SubVecRandomVariable **/

SubVecRandomVariable::SubVecRandomVariable
(const RandomVar& v, int the_start, int the_len)
  :FunctionalRandomVariable(v, the_len), start(the_start)
{
  if (v->length() < the_len)
    PLERROR("new SubVecRandomVariable: input should have length at least %d, has %d",
          the_len, v->length());
}

void SubVecRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = parents[0]->value->subVec(start,value->length());
}

bool SubVecRandomVariable::
invertible(const Var& obs, RVInstanceArray& unobserved_parents,
           Var** JacobianCorrection)
{
  if (length()!=parents[0]->length())
    return false;
  // the only invertible case is when this RandomVar is a copy of its parent
  unobserved_parents[0].v = obs;
  return true;
}

void SubVecRandomVariable::EMBprop(const Vec obs, real posterior)
{
  if (length()!=parents[0]->length())
    PLERROR("SubVecRandomVariable: can't EMBprop unless length==parent.length()");
  // the only "invertible" case is when this RandomVar is a copy of its parent
  parents[0]->EMBprop(obs,posterior);
}

/** ExtendedRandomVariable **/

ExtendedRandomVariable::ExtendedRandomVariable
(const RandomVar& v, real fillvalue, int nextend)
  :FunctionalRandomVariable(v, v->length()+nextend), 
   n_extend(nextend), fill_value(fillvalue)
{
}

void ExtendedRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = extend(parents[0]->value,fill_value,n_extend);
}

bool ExtendedRandomVariable::
invertible(const Var& obs, RVInstanceArray& unobserved_parents,
           Var** JacobianCorrection)
{
  int n = n_extend*parents[0]->width();
  unobserved_parents[0].v = obs->subVec(parents[0]->value->length(),n);
  return true;
}

void ExtendedRandomVariable::EMBprop(const Vec obs, real posterior)
{
  int n = n_extend*parents[0]->width();
  parents[0]->EMBprop(obs.subVec(parents[0]->value->length(),n),posterior);
}

/** ConcatColumnsRandomVariable **/

// *** A REVOIR *** Pascal

ConcatColumnsRandomVariable::ConcatColumnsRandomVariable(const RVArray& a)
  :FunctionalRandomVariable(a, a.length())
{
  setValueFromParentsValue(); // just to check compatibility
  // for (int i=0;i<a.size();i++)
    // int n_rows = a[0]->value->matValue.length();
  // Je commente ca parce que la methode n'existe plus, mais ca avait surement son utilite... 
  // seeAsMatrix(n_rows,length()/n_rows);
}

void ConcatColumnsRandomVariable::setValueFromParentsValue()
{
  if (marked) return;
  value = hconcat(parents.values());
}

bool ConcatColumnsRandomVariable::
invertible(const Var& obs, RVInstanceArray& unobserved_parents,
           Var** JacobianCorrection)
{
  PLERROR("ConcatColumnsRandomVariable::invertible not yet implemented");
  return true;
}

void ConcatColumnsRandomVariable::EMBprop(const Vec obs, real posterior)
{
  PLERROR("ConcatColumnsRandomVariable::EMBprop not yet implemented");
}

/*** RandomVarVMatrix ***/

// DEPRECATED: this class should be rewritten entirely or erased.
// It probably won't work in its current state.

RandomVarVMatrix::
RandomVarVMatrix(ConditionalExpression conditional_expression)
  :VMatrix(-1,-1), instance(Sample(conditional_expression)) // extract the "sampling algorithm"
{
  // make sure all non-random dependencies are computed
  instance->fprop_from_all_sources();
  // extract the path of dependencies from all stochastically sampled Vars to instance
  instance->random_sources().setMark(); // mark the random sources
  instance->markPath(); // mark successors of the random sources
  instance->buildPath(prop_path); // extract path from the random sources to instance
                                  // and clear marks
}

//  template <>
//  void deepCopyField(RandomVar& field, CopiesMap& copies)
//  {
//    if (field)
//      field = static_cast<RandomVariable*>(field->deepCopy(copies));
//  }

} // end of namespace PLearn
