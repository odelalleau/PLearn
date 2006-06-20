// -*- C++ -*-

// BasisSelectionRegressor.cc
//
// Copyright (C) 2006 Pascal Vincent
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

/*! \file BasisSelectionRegressor.cc */


#include "BasisSelectionRegressor.h"
#include <plearn/math/RealFunctionOfInputFeature.h>
#include <plearn/math/RealFunctionFromKernel.h>
#include <plearn/math/ConstantRealFunction.h>
#include <plearn/math/RealFunctionProduct.h>
#include <plearn/math/RealRangeIndicatorFunction.h>
#include <plearn/math/TruncatedRealFunction.h>
#include <plearn/vmat/MemoryVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BasisSelectionRegressor,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

BasisSelectionRegressor::BasisSelectionRegressor()
    : consider_constant_function(false),
      consider_raw_inputs(true),
      consider_input_range_indicators(false),
      n_kernel_centers_to_pick(-1),
      consider_interaction_terms(false),
      residue_sum(0),
      residue_sum_sq(0),
      weights_sum(0)
{}

void BasisSelectionRegressor::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &BasisSelectionRegressor::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...


    //#####  Public Build Options  ############################################

    declareOption(ol, "consider_constant_function", &BasisSelectionRegressor::consider_constant_function,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "explicit_functions", &BasisSelectionRegressor::explicit_functions,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "consider_raw_inputs", &BasisSelectionRegressor::consider_raw_inputs,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "consider_input_range_indicators", &BasisSelectionRegressor::consider_input_range_indicators,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "kernels", &BasisSelectionRegressor::kernels,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "kernel_centers", &BasisSelectionRegressor::kernel_centers,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "n_kernel_centers_to_pick", &BasisSelectionRegressor::n_kernel_centers_to_pick,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "consider_interaction_terms", &BasisSelectionRegressor::consider_interaction_terms,
                  OptionBase::buildoption,
                  "");

    declareOption(ol, "learner", &BasisSelectionRegressor::learner,
                  OptionBase::buildoption,
                  "");


    //#####  Public Learnt Options  ############################################

    declareOption(ol, "selected_functions", &BasisSelectionRegressor::selected_functions,
                  OptionBase::learntoption,
                  "");

    declareOption(ol, "alphas", &BasisSelectionRegressor::alphas,
                  OptionBase::learntoption,
                  "");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void BasisSelectionRegressor::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
}

// ### Nothing to add here, simply calls build_
void BasisSelectionRegressor::build()
{
    inherited::build();
    build_();
}


void BasisSelectionRegressor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("BasisSelectionRegressor::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int BasisSelectionRegressor::outputsize() const
{
    return 1;
}

void BasisSelectionRegressor::forget()
{
    
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */

    selected_functions.resize(0);
    target.resize(0);
    residue.resize(0);
    weights.resize(0);
    features.resize(0,0);
    if(n_kernel_centers_to_pick>=0)
        kernel_centers.resize(0,0);
    if(learner.isNotNull())
        learner->forget();

    stage = 0;
}

void BasisSelectionRegressor::appendCandidateFunctionsOfSingleField(int fieldnum, TVec<RealFunc>& functions) const
{
    if(consider_raw_inputs)
        functions.append(new RealFunctionOfInputFeature(fieldnum));
    if(consider_input_range_indicators)
    {
        PLERROR("consider_input_range_indicators mode not yet properly implemented");
        // StatsCollector& st = train_set->getStats(fieldnum);
        // st.getBinMapping();        
    }
}

void BasisSelectionRegressor::appendKernelFunctions(TVec<RealFunc>& functions) const
{
    if(kernel_centers.length()<=0 && n_kernel_centers_to_pick>=0)
    {
        int nc = n_kernel_centers_to_pick;
        kernel_centers.resize(nc, inputsize());
        Vec target;
        real weight;        
        int l = train_set->length();
        if(random_gen.isNull())
            random_gen = new PRandom();
        random_gen->manual_seed(seed_);
        for(int i=0; i<nc; i++)
        {
            Vec input = kernel_centers(i);
            int rowpos = min(int(l*random_gen->uniform_sample()),l-1);
            train_set->getExample(rowpos, input, target, weight);
        }
    }

    for(int i=0; i<kernel_centers.length(); i++)
    {
        Vec center = kernel_centers(i);
        for(int k=0; k<kernels.length(); k++)
            functions.append(new RealFunctionFromKernel(kernels[k],center));
    }
}

void BasisSelectionRegressor::appendConstantFunction(TVec<RealFunc>& functions) const
{
    functions.append(new ConstantRealFunction());
}

void BasisSelectionRegressor::buildAllCandidateFunctions(TVec<RealFunc>& functions) const
{
    functions.resize(0);

    if(consider_constant_function)
        appendConstantFunction(functions);
    
    if(explicit_functions.length()>0)
        functions.append(explicit_functions);

    for(int fieldnum=0; fieldnum<inputsize(); fieldnum++)
        appendCandidateFunctionsOfSingleField(fieldnum, functions);
    
    if(kernels.length()>0)
        appendKernelFunctions(functions);

    if(consider_interaction_terms)
    {
        int candidate_start = consider_constant_function?1:0; // skip bias
        int ncandidates = functions.length();
        int nselected = selected_functions.length();
        for(int k=0; k<nselected; k++)
            for(int j=candidate_start; j<ncandidates; j++)
                functions.append( new RealFunctionProduct(selected_functions[k],functions[j]) );
    }
}

/* Returns the index of the most correlated (or anti-correlated) feature 
among the full candidate features. 
*/
void BasisSelectionRegressor::findMostCorrelatedCandidateFunction(const TVec<RealFunc>& functions, const Vec& residue,
                                                                  int& best_featurenum, real& best_abs_correl) const
{
    int n_candidates = functions.size();
    static Mat tmpvecs;
    tmpvecs.resize(6, n_candidates);
    Vec E_x = tmpvecs(0);
    Vec V_x = tmpvecs(1);
    Vec E_xy = tmpvecs(2);
    Vec V_xy = tmpvecs(3);
    Vec covar = tmpvecs(4);
    Vec correl = tmpvecs(5);
    real wsum;
    real E_y;
    real V_y;
    computeWeightedCorrelationsWithY(functions, residue,  
                                     wsum,
                                     E_x, V_x,
                                     E_y, V_y,
                                     E_xy, V_xy,
                                     covar, correl);
    best_featurenum = -1;
    best_abs_correl = 0;
    for(int j=0; j<n_candidates; j++)
    {
        real abs_correl = fabs(correl[j]);
        if(abs_correl>best_abs_correl)
        {
            best_featurenum = j;
            best_abs_correl = abs_correl;
        }
    }
}

void BasisSelectionRegressor::computeWeightedCorrelationsWithY(const TVec<RealFunc>& functions, const Vec& Y,  
                                                               real& wsum,
                                                               Vec& E_x, Vec& V_x,
                                                               real& E_y, real& V_y,
                                                               Vec& E_xy, Vec& V_xy,
                                                               Vec& covar, Vec& correl) const
{
    int n_candidates = functions.length();
    E_x.resize(n_candidates);
    E_x.fill(0.);
    V_x.resize(n_candidates);
    V_x.fill(0.);
    E_y = 0.;
    V_y = 0.;
    E_xy.resize(n_candidates);
    E_xy.fill(0.);
    V_xy.resize(n_candidates);
    V_xy.fill(0.);
    wsum = 0;

    Vec input;
    Vec target;
    Vec candidate_features;
    real w;
    int l = train_set->length();
    for(int i=0; i<l; i++)
    {
        real y = Y[i];
        train_set->getExample(i, input, target, w);
        evaluate_functions(functions, input, candidate_features);
        wsum += w;
        E_y  += w*y;
        V_y  += w*y*y;
        for(int j=0; j<n_candidates; j++)
        {
            real x = candidate_features[j];
            E_x[j] += w*x;
            V_x[j] += w*x*x;
            real xy = x*y;
            E_xy[j] += w*xy;
            V_xy[j] += w*xy*xy;
        }
    }

    // Finalize computation
    real inv_wsum = 1.0/wsum;
    E_y *= inv_wsum;
    V_y  = V_y*inv_wsum - square(E_y);
    covar.resize(n_candidates);
    correl.resize(n_candidates);
    for(int j=0; j<n_candidates; j++)
    {
        real E_x_j = E_x[j]*inv_wsum;
        E_x[j] = E_x_j;
        real V_x_j = V_x[j]*inv_wsum - square(E_x_j);
        V_x[j] = V_x_j;
        real E_xy_j = E_xy[j]*inv_wsum;
        E_xy[j] = E_xy_j;
        real V_xy_j = V_xy[j]*inv_wsum - square(E_xy_j);
        V_xy[j] = V_xy_j;
        real covar_j = E_xy_j - square(E_x_j);
        real correl_j = covar_j/sqrt(V_x_j*V_y);
        covar[j] = covar_j;
        correl[j] = correl_j;
    }
}


//! Note that the returned statistics (except wsum) are invariant to arbitrary scaling of the weights
//! Note that the variances computed are the sample variances, which are not an unbiased estimator. 
void weighted_XY_statistics(const Vec& X, const Vec& Y, const Vec& W, 
                                  real& wsum,
                                  real& E_x, real& V_x,
                                  real& E_y, real& V_y,
                                  real& E_xy, real& V_xy,
                                  real& covar, real& correl)
{
    E_x = 0;
    V_x = 0;
    E_y = 0;
    V_y = 0;
    E_xy = 0;
    V_xy = 0;
    wsum = 0;

    const real* pX = X.data();
    const real* pY = Y.data();
    const real* pW = W.data();

    int l = X.length();
    while(l--)
    {
        real x = *pX++;
        real y = *pY++;
        real w = *pW++;
        wsum += w;
        E_x  += w*x;
        V_x  += w*x*x;
        E_y  += w*y;
        V_y  += w*y*y;
        real xy = x*y;
        E_xy += w*xy;
        V_xy += w*xy*xy;
    }
    real inv_wsum = 1.0/wsum;
    E_x  *= inv_wsum;
    V_x  = V_x*inv_wsum - E_x*E_x;
    E_y  *= inv_wsum;
    V_y  = V_y*inv_wsum - E_y*E_y;
    E_xy = E_xy*inv_wsum;
    V_xy = V_xy*inv_wsum - E_xy*E_xy;

    covar = E_xy - E_x*E_y;
    correl = covar/sqrt(V_x*V_y);
}

void BasisSelectionRegressor::appendFunction(RealFunc f)
{
    int l = train_set->length();
    int nf = selected_functions.length();    
    features.resize(l,nf,l*nf,true);  // enlarge width while preserving content
    real weight;
    for(int i=0; i<l; i++)
    {
        train_set->getExample(i,input,target,weight);
        features(i,nf) = f->evaluate(input);
    }
    selected_functions.append(f);
}

void BasisSelectionRegressor::retrainLearner()
{
    int l  = selected_functions.length();
    int nf = selected_functions.length();
    features.resize(l,nf+2, l*nf, true); // enlarge width while preserving content
    // append residue (as target) and weight columns to features matrix
    for(int i=0; i<l; i++)
    {
        features(i,nf) = residue[i];
        features(i,nf+1) = weights[i];
    }

    VMat newtrainset = new MemoryVMatrix(features);
    newtrainset->defineSizes(nf,1,1);
    learner->setTrainingSet(newtrainset);
    learner->forget();
    learner->train();
    // resize features matrix so it containsonly the features
    features.resize(l,nf);
}


void BasisSelectionRegressor::train()
{
    if (!initTrain())
        return;

    Vec train_costs(1);
    while(stage<nstages)
    {
        if(target.length()==0)
        {
            initTargetResidueWeight();
            if(selected_functions.length()>0)
            {
                recomputeFeatures();
                recomputeResidue();
            }
        }

        buildAllCandidateFunctions(candidate_functions);
        int best_featurenum = -1;
        real best_abs_correl = 0;
        findMostCorrelatedCandidateFunction(candidate_functions, residue, best_featurenum, best_abs_correl);
        perr << "\n\n*** Stage " << stage << " *****" << endl
             << "Most correlated: index=" << best_featurenum << endl 
             << "  abs_correl=" << best_abs_correl << endl
             << "  function= " << candidate_functions[best_featurenum]
             << endl;
        appendFunction(candidate_functions[best_featurenum]);
        perr << "residue_sum_sq before retrain: " << residue_sum_sq << endl;
        retrainLearner();
        recomputeResidue();
        perr << "residue_sum_sq after retrain: " << residue_sum_sq << endl;

        // clear statistics of previous epoch
        train_stats->forget();
        train_costs[0] = residue_sum_sq;
        train_stats->update(train_costs, weights_sum);
        train_stats->finalize(); // finalize statistics for this epoch
        ++stage;
    }
}

void BasisSelectionRegressor::initTargetResidueWeight()
{
    int l = train_set.length();
    residue.resize(l);
    target.resize(l);
    residue_sum = 0.;
    residue_sum_sq = 0.;
    weights.resize(l);
    weights_sum = 0.;

    Vec input(inputsize());
    Vec target(targetsize());
    real w;
    for(int i=0; i<l; i++)
    {
        train_set->getExample(i, input, target, w);
        real t = target[0];
        target[i] = t;
        residue[i] = t;
        residue_sum += w*t;
        residue_sum_sq += w*square(t);        
        weights[i] = w;
        weights_sum += w;
    }

    /*
    bias = residue_sum/weights_sum;

    // Now sutract the bias
    residue_sum = 0.;
    residue_sum_sq = 0.;    
    for(int i=0; i<l; i++)
    {
        real w = weights[i];
        real resval = residue[i]-bias;
        residue[i] = resval;
        residue_sum += w*resval;
        residue_sum_sq += w*square(resval);        
    }
    */
}

void BasisSelectionRegressor::recomputeFeatures()
{
    int l = train_set.length();
    int nf = selected_functions.length();
    features.resize(l,nf);
    real weight = 0;
    for(int i=0; i<l; i++)
    {
        train_set->getExample(i, input, target, weight);
        Vec v = features(i);
        evaluate_functions(selected_functions, input, v);        
    }    
}

void BasisSelectionRegressor::recomputeResidue()
{
    int l = train_set.length();
    residue.resize(l);
    residue_sum = 0;
    residue_sum_sq = 0;
    Vec output(outputsize());
    // perr << "recomp_residue: { ";
    for(int i=0; i<l; i++)
    {
        // train_set->getExample(i, input, target, w);
        real t = target[i];
        real w = weights[i];
        computeOutputFromFeaturevec(features(i),output);
        real resid = t-output[0];
        residue[i] = resid;
        residue_sum += resid;
        residue_sum_sq += w*square(resid);
    }
    // perr << "}" << endl;
}

void BasisSelectionRegressor::computeOutputFromFeaturevec(const Vec& featurevec, Vec& output) const
{
    int nout = outputsize();
    if(nout!=1)
        PLERROR("outputsize should always be one for this learner");
    output.resize(nout);

    if(learner.isNull())
        output[0] = dot(alphas, featurevec);
    else
        learner->computeOutput(featurevec, output);
}

void BasisSelectionRegressor::computeOutput(const Vec& input, Vec& output) const
{
    evaluate_functions(selected_functions, input, featurevec);
    computeOutputFromFeaturevec(featurevec, output);
}

void BasisSelectionRegressor::printModelFunction(PStream& out) const
{
    for(int k=0; k<selected_functions.length(); k++)
    {
        out << "+ " << alphas[k] << "* " << selected_functions[k];
        out << endl;
    }
}

void BasisSelectionRegressor::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    costs.resize(1);
    costs[0] = square(output[0]-target[0]);
}

TVec<string> BasisSelectionRegressor::getTestCostNames() const
{
    return getTrainCostNames();
}

TVec<string> BasisSelectionRegressor::getTrainCostNames() const
{
    TVec<string> costnames(1);
    costnames[0] = "MSE";
    return costnames;
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
