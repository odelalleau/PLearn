// -*- C++ -*-

// DiverseComponentAnalysis.cc
//
// Copyright (C) 2008 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file DiverseComponentAnalysis.cc */


#include "DiverseComponentAnalysis.h"
#include <plearn/vmat/VMat_basic_stats.h>
#include<plearn/math/TMat_maths.h>
#include<plearn/var/ProductVariable.h>
#include<plearn/var/ProductTransposeVariable.h>
#include<plearn/var/TransposeProductVariable.h>
#include<plearn/var/SquareVariable.h>
#include<plearn/var/AbsVariable.h>
#include<plearn/var/SquareRootVariable.h>
#include<plearn/var/ExpVariable.h>
#include<plearn/var/TimesVariable.h>
#include<plearn/var/SumVariable.h>
#include<plearn/var/SigmoidVariable.h>
#include<plearn/var/TanhVariable.h>
#include<plearn/var/NegateElementsVariable.h>
#include<plearn/var/TimesConstantVariable.h>
#include<plearn/var/SumSquareVariable.h>
#include<plearn/var/RowSumSquareVariable.h>
#include<plearn/var/EXPERIMENTAL/ConstrainedSourceVariable.h>
#include<plearn/var/EXPERIMENTAL/Cov2CorrVariable.h>
#include<plearn/var/EXPERIMENTAL/DiagVariable.h>
#include<plearn/var/EXPERIMENTAL/NonDiagVariable.h>
#include<plearn/var/TransposeVariable.h>
#include<plearn/var/ColumnSumVariable.h>
#include<plearn/var/Var_operators.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DiverseComponentAnalysis,
    "Diverse Component Analysis",
    "This is an experimental class that finds linear\n"
    "projection directions that should yield\n"
    "'diverse' components, based on some diversity loss");

DiverseComponentAnalysis::DiverseComponentAnalysis()
    :ncomponents(2),
     nonlinearity("none"),
     cov_transformation_type("cov"),
     diag_add(0.),
     diag_premul(1.0),
     offdiag_premul(1.0),
     diag_nonlinearity("square"),
     offdiag_nonlinearity("square"),
     diag_weight(-1.0),
     offdiag_weight(1.0),
     force_zero_mean(false),
     epsilon(1e-8),
     nu(0),
     constrain_norm_type(-2),
     normalize(false)
/* ### Initialize all fields to their default value here */
{
    // ### If this learner needs to generate random numbers, uncomment the
    // ### line below to enable the use of the inherited PRandom object.
    random_gen = new PRandom();
}

void DiverseComponentAnalysis::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &DiverseComponentAnalysis::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(
        ol, "nonlinearity", &DiverseComponentAnalysis::nonlinearity, OptionBase::buildoption,
        "The nonlinearity to apply after linear transformation of the inputs to obtain the representation.");

    declareOption(
        ol, "force_zero_mean", &DiverseComponentAnalysis::force_zero_mean, OptionBase::buildoption,
        "If true then input mean won't be computes but forced to 0 (and a corresponding different covariance matrix will be computed)");

    declareOption(
        ol, "epsilon", &DiverseComponentAnalysis::epsilon, OptionBase::buildoption,
        "regularization value to add to diagonal of computed input covariance matrix.");

    declareOption(
        ol, "nu", &DiverseComponentAnalysis::nu, OptionBase::buildoption,
        "regularization parameter simulating destruction noise: \n"
        "off-diagonal elements of covariance matrix Cx will be multiplied by 1-nu.");

    declareOption(
        ol, "constrain_norm_type", &DiverseComponentAnalysis::constrain_norm_type, OptionBase::buildoption,
        "How to constrain the norms of rows of W: \n"
        "  -1: L1 norm constrained source \n"
        "  -2: L2 norm constrained source \n"
        "  -3:explicit L2 normalization \n"
        "  >0:add specified value times exp(sumsquare(W)) to the cost\n");

    declareOption(
        ol, "ncomponents", &DiverseComponentAnalysis::ncomponents, OptionBase::buildoption,
        "The number components to keep (that's also the outputsize).");

    declareOption(
        ol, "cov_transformation_type", &DiverseComponentAnalysis::cov_transformation_type, OptionBase::buildoption,
        "Controls the kind of transformation to apply to covariance matrix\n"
        "cov: no transformation (keep covariance)\n"
        "corr: transform into correlations, but keeping variances on the diagonal.\n"
        "squaredist: do a 'squared distance kernel' Dij <- Cii+Cjj-2Cij kind of transformation\n"
        "sincov: instead of ||u|| ||v|| cos(angle(u,v)) we transform it to ||u|| ||v|| |sin(angle(u,v))|\n"
        "        this is computed as sqrt((1-<u.v>^2) * <u,u>^2 * <v,v>^2) where <u,v> is given by the covariance matrix\n");

    declareOption(
        ol, "diag_add", &DiverseComponentAnalysis::diag_add, OptionBase::buildoption,
        "This value will be added to the diagonal (before premultiplying and applying non-linearity)");

    declareOption(
        ol, "diag_premul", &DiverseComponentAnalysis::diag_premul, OptionBase::buildoption,
        "diagonal elements of Cy will be pre-multiplied by diag_premul (before applying non-linearity)");

    declareOption(
        ol, "offdiag_premul", &DiverseComponentAnalysis::offdiag_premul, OptionBase::buildoption,
        "Non-diagonal elements of Cy will be pre-multiplied by diag_premul (before applying non-linearity)");

    declareOption(
        ol, "diag_nonlinearity", &DiverseComponentAnalysis::diag_nonlinearity, OptionBase::buildoption,
        "The kind of nonlinearity to apply to the diagonal elements of Cy\n"
        "after it's been through cov_transformation_type\n"
        "Currently supported: none square abs sqrt sqrtabs exp tanh sigmoid");

    declareOption(
        ol, "offdiag_nonlinearity", &DiverseComponentAnalysis::offdiag_nonlinearity, OptionBase::buildoption,
        "The kind of nonlinearity to apply to the non-diagonal elements of Cy \n"
        "after it's been through cov_transformation_type\n"
        "Currently supported: none square abs sqrt sqrtabs exp tanh sigmoid");

    declareOption(
        ol, "diag_weight", &DiverseComponentAnalysis::diag_weight, OptionBase::buildoption,
        "what weight to give to the sum of transformed diagonal elements in the cost");

    declareOption(
        ol, "offdiag_weight", &DiverseComponentAnalysis::offdiag_weight, OptionBase::buildoption,
        "what weight to give to the sum of transformed non-diagonal elements in the cost");

    declareOption(
        ol, "optimizer", &DiverseComponentAnalysis::optimizer, OptionBase::buildoption,
        "The gradient-based optimizer to use");

    declareOption(
        ol, "normalize", &DiverseComponentAnalysis::normalize, OptionBase::buildoption,
        "If true computed outputs will be scaled so they have unit variance.\n"
        "(see explanation about inv_stddev_of_projections)");


    // learnt options
    declareOption(
        ol, "mu", &DiverseComponentAnalysis::mu, OptionBase::learntoption,
        "The (weighted) mean of the samples");

    declareOption(
        ol, "Cx", &DiverseComponentAnalysis::Cx, OptionBase::learntoption,
        "The (weighted) covariance of the samples");

    declareOption(
        ol, "W", &DiverseComponentAnalysis::W, OptionBase::learntoption,
        "A ncomponents x inputsize matrix containing the learnt projection directions");

    declareOption(
        ol, "bias", &DiverseComponentAnalysis::bias, OptionBase::learntoption,
        "A 1 x ncomponents matrix containing the learnt bias (for the nonlinear case only)");

    declareOption(
        ol, "inv_stddev_of_projections", &DiverseComponentAnalysis::inv_stddev_of_projections, OptionBase::learntoption,
        "As its name implies, this is one over the standard deviation of projected data.\n"
        "when normalize=true computeOutput will multiply the projection by this,\n"
        " elementwise, so that the output should have unit variance" );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DiverseComponentAnalysis::declareMethods(RemoteMethodMap& rmm)
{
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(rmm,
                  "getVarValue",
                  &DiverseComponentAnalysis::getVarValue,
                  (BodyDoc("Returns the matValue of the variable with the given name"),
                   ArgDoc("varname", "name of the variable searched for"),
                   RetDoc("Returns the value of the var as a Mat")));

    declareMethod(rmm,
                  "getVarGradient",
                  &DiverseComponentAnalysis::getVarGradient,
                  (BodyDoc("Returns the matGradient of the variable with the given name"),
                   ArgDoc("varname", "name of the variable searched for"),
                   RetDoc("Returns the gradient of the var as a Mat")));

    declareMethod(rmm,
                  "listVarNames",
                  &DiverseComponentAnalysis::listVarNames,
                  (BodyDoc("Returns a list of the names of all vars"),
                   RetDoc("Returns a list of the names of all vars")));

}

Var DiverseComponentAnalysis::nonlinear_transform(Var in, string nonlinearity)
{
    Var res; // result
    if(nonlinearity=="none" || nonlinearity=="linear")
        res = in;
    else if(nonlinearity=="square")
        res = square(in);
    else if(nonlinearity=="abs")
        res = abs(in);
    else if(nonlinearity=="sqrt")
        res = squareroot(in);
    else if(nonlinearity=="sqrtabs")
        res = squareroot(abs(in));
    else if(nonlinearity=="exp")
        res = exp(in);
    else if(nonlinearity=="tanh")
        res = tanh(in);
    else if(nonlinearity=="sigmoid")
        res = sigmoid(in);
    else
        PLERROR("Unknown nonlinearity %s",nonlinearity.c_str());
    return res;
}

void DiverseComponentAnalysis::build_()
{
    perr << "Entering DiverseComponentAnalysis::build_()" << endl;
    bool rebuild_all = inputsize_>0 && (W.isNull() || (W->matValue.width()!=inputsize_));
    bool rebuild_some = inputsize_>0 && Cyt.isNull();
    bool linear = (nonlinearity=="none" || nonlinearity=="linear");
    if(rebuild_some || rebuild_all)
    {
        perr << "Building with inputsize_ = " << inputsize_ << endl;

        Var nW;
        
        if(constrain_norm_type==-1) // use constrainted source to constrain L1 norms to 1
        {
            perr << "using constrainted source to constrain L1 norms to 1" << endl;
            if(rebuild_all)
                W = new ConstrainedSourceVariable(ncomponents,inputsize_,1);
            nW = W;
        }
        else if(constrain_norm_type==-2) // use constrainted source to constrain L2 norms to 1
        {
            perr << "using constrainted source to constrain L2 norms to 1" << endl;
            if(rebuild_all)
                W = new ConstrainedSourceVariable(ncomponents,inputsize_,2);
            nW = W;
        }
        else if(constrain_norm_type==-3) // compute L2 normalization explicitly
        {
            perr << "Normalizing explicitly" << endl;
            if(rebuild_all)
                W = Var(ncomponents,inputsize_);
            nW = W/squareroot(rowSumSquare(W));
        }
        else  // using ordinary weight decay: nW is not hard-constrained to be normalized
        {
            perr << "Using ordinary weight decay " << constrain_norm_type << endl;
            if(rebuild_all)
                W = Var(ncomponents,inputsize_);
            nW = W;
        }

        if(linear) 
        {
            if(rebuild_all)
                Cx = Var(inputsize_,inputsize_);
            Cx->setName("Cx");
            Cy = product(nW, productTranspose(Cx, nW));
        }
        else // nonlinear trasform
        {
            int l = train_set->length();
            perr << "Building with nonlinear transform and l="<<l <<" examples of inputsize=" << inputsize_ << endl;

            inputdata = Var(l,inputsize_);
            if(rebuild_all)
                bias = Var(1, ncomponents);
            trdata = productTranspose(inputdata,nW)+bias;
            perr << "USING MAIN REPRESENTATION NONLINEARITY: " << nonlinearity << endl;
            trdata = nonlinear_transform(trdata,nonlinearity);
            if(force_zero_mean)
                ctrdata = trdata;
            else
                ctrdata = trdata-(1.0/l)*columnSum(trdata);
            ctrdata->setName("ctrdata");            
            trdata->setName("trdata");            
            Cy = (1.0/l)*transposeProduct(ctrdata,ctrdata);
        }
        perr << "Built Cy of size " << Cy->length() << "x" << Cy->width() << endl;

        if(cov_transformation_type=="cov")
            Cyt = Cy;
        else if(cov_transformation_type=="corr")
            Cyt = cov2corr(Cy,2);
        else if(cov_transformation_type=="squaredist")
        {
            Var dCy = diag(Cy);
            Cyt = Cy*(-2.0)+dCy+transpose(dCy);
        }
        else if(cov_transformation_type=="sincov")
        {
            Var dCy = diag(Cy);
            Cyt = squareroot(((1+1e-6)-square(cov2corr(Cy)))*dCy*transpose(dCy));            
            // Cyt = ((1.0-square(cov2corr(Cy)))*dCy*transpose(dCy));            
        }
        else 
            PLERROR("Invalid cov_transformation_type");

        if(diag_weight!=0)
        {
            Var diagelems = diag(Cyt);
            if(diag_add!=0)
                diagelems = diagelems+diag_add;
            L += diag_weight*sum(nonlinear_transform(diagelems*diag_premul,diag_nonlinearity));
        }
        if(offdiag_weight!=0)
            L += offdiag_weight*sum(nonlinear_transform(nondiag(Cyt)*offdiag_premul,offdiag_nonlinearity));
            
        if(constrain_norm_type>0)
            L += L+constrain_norm_type*exp(sumsquare(W));

        if(!optimizer)
            PLERROR("You must specify the optimizer field (ex: GradientOptimizer)");
        if(linear)
            optimizer->setToOptimize(W, L);
        else
            optimizer->setToOptimize(W&bias, L);
        
        perr << "Built optimizer" << endl;
        nW->setName("W");
        Cy->setName("Cy");
        Cyt->setName("Cyt");
        L->setName("L");

        allvars = Cx & trdata& ctrdata& nW & Cy & Cyt & L;
    }
    perr << "Exiting DiverseComponentAnalysis::build_()" << endl;
}


TVec<string> DiverseComponentAnalysis::listVarNames() const
{
    int n = allvars.length();
    TVec<string> names;
    for(int i=0; i<n; i++)
        if(allvars[i].isNotNull())
            names.append(allvars[i]->getName());
    return names;
}

Mat DiverseComponentAnalysis::getVarValue(string varname) const
{
    for(int i=0; i<allvars.length(); i++)
    {
        Var v = allvars[i];        
        if(v.isNotNull() && v->getName()==varname)
            return v->matValue;
    }
    PLERROR("No Var with name %s", varname.c_str());
    return Mat();
}

Mat DiverseComponentAnalysis::getVarGradient(string varname) const
{
    for(int i=0; i<allvars.length(); i++)
    {
        Var v = allvars[i];
        if(v.isNotNull() && v->getName()==varname)
            return v->matGradient;
    }
    PLERROR("No Var with name %s", varname.c_str());
    return Mat();
}

void DiverseComponentAnalysis::build()
{
    inherited::build();
    build_();
}


void DiverseComponentAnalysis::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(mu, copies);
    deepCopyField(Cx, copies);
    deepCopyField(W, copies);
}


int DiverseComponentAnalysis::outputsize() const
{
    return ncomponents;
}

void DiverseComponentAnalysis::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - call inherited::forget() to initialize its random number generator
        with the 'seed' option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */

    // this will reset stage=0 and reset the random_gen to the initial seed_
    inherited::forget();

    perr << "Called DCS::forget() with inputsize_ = " << inputsize_ << endl;
    if(inputsize_>0)
    {
        random_gen->fill_random_normal(W->value, 0., 1.);
        perr << "Squared norm of first row of W after fill_random_normal: " << pownorm(W->matValue(0)) << endl;
        int normval = (constrain_norm_type==-1 ?1 :2);
        for(int i=0; i<ncomponents; i++)
            PLearn::normalize(W->matValue(i), normval);
        perr << "Squared norm of first row of W after L" << normval << " normalization: " << pownorm(W->matValue(0)) << endl;
    }
}

void DiverseComponentAnalysis::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    /* TYPICAL CODE:

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    */

    if (!initTrain())
        return;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        if(stage==0) // do stage 1
        {
            bool linear = (nonlinearity=="none" || nonlinearity=="linear");
            if(!linear)
            {
                perr << "Nonlinear training to stage 1" << endl;
                Mat X = inputdata->matValue;
                int l = train_set->length();
                Vec target;
                real weight;
                for(int i=0; i<l; i++)
                {
                    Vec Xi = X(i);
                    train_set->getExample(i,Xi,target,weight);
                }
                mu.resize(inputsize_);
                columnMean(X, mu);
                perr << "Nonlinear training to stage 1. DONE." << endl;
            }
            else // linear case
            {
                if(force_zero_mean)
                {
                    mu.resize(inputsize());
                    mu.fill(0);
                    computeInputCovar(train_set, mu, Cx->matValue, epsilon);
                }
                else
                    computeInputMeanAndCovar(train_set, mu, Cx->matValue, epsilon);

                if(nu!=0)
                {
                    Mat C = Cx->matValue;
                    int l = C.length();
                    for(int i=0; i<l; i++)
                        for(int j=0; j<l; j++)
                            if(i!=j)
                                C(i,j) *= (1-nu);
                }
            }
        }
        else
        {
            optimizer->optimizeN(*train_stats);
            Mat C = Cy->matValue;
            int l = C.length();            
            inv_stddev_of_projections.resize(l);
            for(int i=0; i<l; i++)
                inv_stddev_of_projections = 1.0/sqrt(C(i,i));
        }

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
}


void DiverseComponentAnalysis::computeOutput(const Vec& input, Vec& output) const
{
    static Vec x;
    x.resize(input.length());
    x << input;

    // Center and project on directions
    x -= mu;
    output.resize(ncomponents);
    product(output, W->matValue, x);
    if(normalize)
        output *= inv_stddev_of_projections;
}

void DiverseComponentAnalysis::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    costs.resize(0);
}

TVec<string> DiverseComponentAnalysis::getTestCostNames() const
{
    return TVec<string>();
}

TVec<string> DiverseComponentAnalysis::getTrainCostNames() const
{
    return TVec<string>(1,"L");
}


} // end of namespace PLearn


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
