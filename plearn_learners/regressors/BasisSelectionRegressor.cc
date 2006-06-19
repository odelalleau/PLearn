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

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BasisSelectionRegressor,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

BasisSelectionRegressor::BasisSelectionRegressor()
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

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

    declareOption(ol, "featurespecs", &BasisSelectionRegressor::featurespecs,
                  OptionBase::learntoption,
                  "Help text describing this option");
    
    // Mat features;

    declareOption(ol, "alphas", &BasisSelectionRegressor::alphas,
                  OptionBase::learntoption,
                  "Help text describing this option");

    declareOption(ol, "bias", &BasisSelectionRegressor::bias,
                  OptionBase::learntoption,
                  "Help text describing this option");
    /*
    Vec target;
    Vec residue;
    double residue_sum;
    double residue_sum_sq;
    Vec weights;
    double weights_sum;
    */

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
}

TVec<string> BasisSelectionRegressor::producePossibleVPLFunctionsForField(int fieldnum)
{
    TVec<string> vplcodes;
    vplcodes.append("@"+train_set->fieldName(fieldnum));
    return vplcodes;
}

void BasisSelectionRegressor::produceCandidateVPLCodes()
{
    int insize = inputsize();
    candidate_vplcodes.resize(0);
    for(int j=0; j<insize; j++)
    {
        TVec<string> codes_for_field = getPossibleVPLFunctionsForField(j);
        candidate_vplcodes.append(codes_for_field);
    }
    string fullcode = join(vplcodes,"\n");
    TVec<string> fieldnames;
    candidates_program.compileString(fullcode, fieldnames);
}

//! Returns the number of already selected features
int BasisSelectionRegressor::nSelectedFeatures() const
{
    return precomp_features.width(); 
}

int BasisSelectionRegressor::nSimpleCandidateFeatures() const
{
    switch(feature_mode)
    {
    case 'I': 
        return inpusize();
    case 'F': // using VPL generated features
        return candidate_vplcodes.length();
    default:
        PLERROR("Invalid feature_mode");
    }
}

int BasisSelectionRegressor::nFullCandidateFeatures() const
{
    int n = nSimpleCandidateFeatures();
    if(consider_interactions)
        n *= 1+nSelectedFeatures();
    return n;
}

void BasisSelectionRegressor::computeSimpleCandidateFeatures(const Vec& input, Vec& candidate_features)
{
    int nfeatures = nSimpleCandidateFeatures();
    candidate_features.resize(nfeatures);
    switch(feature_mode)
    {
    case 'I': // simply copy the inputs (i.e. work as a simple feature selector)
        candidate_features << input;
        break;
    case 'F': // use VPL generated features
        candidates_program.run(input, candidate_features);
        break;
    default:
        PLERROR("Invalid feature_mode");
    }
}

void BasisSelectionRegressor::computeFullCandidateFeatures(const Vec& input, const Vec& precomp_features_row, Vec& candidate_features)
{
    if(!consider_interactions)
        computeSimpleCandidateFeatures(input, candidate_features);
    else // consider inteactions with already selected features
    {
        int n_simple = nSimpleCandidateFeatures();
        int n_full = nFullCandidateFeatures();
        candidate_features.resize(n_full);
        Vec simple_candidate_features = candidate_features.subVec(0,n_simple);
        computeSimpleCandidateFeatures(input, simple_candidate_features);
        int n_selected = nSelectedFeatures();
        for(int i=0; i<n_simple; i++)
        {
            real simple_fval = simple_candidate_features[i];
            for(int j=0; j<n_selected; j++)
            {
                real selected_fval = precomp_feature_row[j];
                candidate_features[(n_simple+1)*i+j] = simple_fval*selected_fval;
            }
        }
    }
}

/* Returns the index of the most correlated (or anti-correlated) feature 
among the full candidate features. If none has a |correlation| with the residue > 1e-6,
the call returns -1
*/
int BasisSelectionRegressor::findMostCorrelatedCandidateFeature(const Vec& residue)
{
    int n_candidates = nFullCandidateFeatures();
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
    computeWeightedCorrelationsWithY(residue,  
                                     wsum,
                                     E_x, V_x,
                                     E_y, V_y,
                                     E_xy, V_xy,
                                     covar, correl);
    int best_featurenum = -1;
    real best_abs_correl = 1e-6;
    for(int j=0; j<n_candidates; j++)
    {
        real abs_correl = fabs(correl[j]);
        if(abs_correl>best_abs_correl)
        {
            best_featurenum = j;
            best_abs_correl = abs_correl;
        }
    }

    return best_featurenum;
}

void BasisSelectionRegressor::computeWeightedCorrelationsWithY(const Vec& Y,  
                                                    real& wsum,
                                                    Vec& E_x, Vec& V_x,
                                                    real& E_y, real& V_y,
                                                    Vec& E_xy, Vec& V_xy,
                                                    Vec& covar, Vec& correl)
{
    int n_candidates = nFullCandidateFeatures();
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
        computeFullCandidateFeatures(input, precomp_features(i), candidate_features);
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
    V_x  = V_x*inv_wsum - E_x*Ex;
    E_y  *= inv_wsum;
    V_y  = V_y*inv_wsum - E_y*E_y;
    E_xy = E_xy*inv_wsum;
    V_xy = V_xy*inv_wsum - E_xy*E_xy;

    covar = E_xy - E_x*E_y;
    correl = cov/sqrt(V_x*V_y);
}

void BasisSelectionRegressor::recomputeResidue()
{
    
    residue += bias;

    int l = targetvec.length();
    
    for(int i=0; i<l; i++)
    {
        
    }
}

void BasisSelectionRegressor::train()
{
    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    if (!initTrain())
        return;

    Vec train_costs(1);

    Vec feature;

    int l = train_set->length();
    residue.resize(l);
    targetvec.resize(l);

    bias = 0.;
    real wsum;
    while(stage<nstages)
    {
        if(stage==0)
        {
            wsum = 0.;
            for(int i=0; i<l; i++)
            {
                train_set->getExample(i,input,target,weight);
                wsum += weight;
                real t = target[0];
                targetvec[i] = t;
                bias += weight*t;
            }
            bias /= wsum;
            recomputeResidue();
        }
        else
        {
        }

        int BasisSelectionRegressor::findMostCorrelatedCandidateFeature(const Vec& residue)

        // clear statistics of previous epoch
        train_stats->forget();

        perr << "*** Stage " << stage << endl;
        if(stage==0)
            initResidueWeightAndBias();
        else
        {
            findBestNewFeature(newfs, alpha, b, newsqerror);
            perr << "+ " << alpha << "*";
            printFeature(perr, newfs);
            perr << "\n b: " << b << endl;
            perr << "newsqerror: " << newsqerror << endl;        
            appendFeature(newfs, alpha, b);
        }
        perr << "new bias: " << bias << endl;
        perr << "residue_sum_sq: " << residue_sum_sq << endl;
        perr << "recomputed residue_sum_sq: " << recompute_residue_sum_sq() << endl;
        train_costs[0] = residue_sum_sq;
        train_stats->update(train_costs, weights_sum);

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
}


#if 0
// Old code

void BasisSelectionRegressor::initResidueWeightAndBias()
{
    int l = train_set.length();
    residue.resize(l);
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
        real resval = target[0];
        residue[i] = resval;
        residue_sum += w*resval;
        residue_sum_sq += w*square(resval);        
        weights[i] = w;
        weights_sum += w;
    }
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
    
}

void BasisSelectionRegressor::updateResidue(const Vec& feature, real alpha, real b)
{
    int l = residue.length();
    residue_sum = 0.;
    residue_sum_sq = 0.;

    perr << "prev_residue: " << residue << endl;
    perr << "new feature: " << feature << endl;
    for(int i=0; i<l; i++)
    {
        residue[i] -= alpha*feature[i]+b;
        real resval = residue[i];
        real w = weights[i];
        residue_sum += w*resval;
        residue_sum_sq += w*square(resval);        
    }
    perr << "new_residue: " << residue << endl;
}


/*
void BasisSelectionRegressor::updateFeatureStats()
{
    int n = featurespecs.length();
    Vec newfeature = features(n-1);
    int l = newfeature.length();
    real m = mean(newfeature);
    // real m = dot(newfeature,weights)/weights_sum;
    featuremean.append(m);
    featurecov.resize(1+n, 1+n, 10000, true);
    Vec newcovrow = featurecov(n);
    newcovrow.fill(0.);

    real biascov = 0;
    for(int i; i<l; i++)
    {
    }

    for(int i=1; i<l; i++)
    {
        residue[i] -= alpha*feature[i]+b;
        real resval = residue[i];
        real w = weights[i];
        residue_sum += w*resval;
        residue_sum_sq += w*square(resval);        
    }
    perr << "new_residue: " << residue << endl;
}
*/

void BasisSelectionRegressor::backfit()
{
    PLERROR("backfit not yet implemented");
}

void BasisSelectionRegressor::appendFeature(const IMPFeatureSpec& fs, real alpha, real b)
{
    Vec featurevec;
    computeFeatureVec(fs, featurevec);
    featurespecs.append(fs);
    features.appendRow(featurevec);
    alphas.append(alpha);
    bias += b;
    updateResidue(featurevec, alpha, b);
    // updateFeatureStats(featurevec);
}

Vec BasisSelectionRegressor::getFieldValues(int fieldpos)
{
    static Vec cached_column;
    static int cached_col_num = -1;
    if(fieldpos!=cached_col_num)
    {
        cached_column.resize(train_set.length());
        train_set->getColumn(fieldpos,cached_column);
        cached_col_num = fieldpos;
    }
    return cached_column;
}

void BasisSelectionRegressor::computeFeatureVec(const IMPFeatureSpec& fs, Vec& featurevec)
{
    Vec fieldvalues = getFieldValues(fs.fieldpos);
    Vec other_featurevec;
    if(fs.combintype!='0')
        other_featurevec = features(fs.other_feature_pos);
    int l = fieldvalues.length();
    featurevec.resize(l);
    for(int i=0; i<l; i++)
    {
        double other_feature_val = (other_featurevec.length()<=0 ?0. :other_featurevec[i]);
        featurevec[i] = fs.evaluateFeature(fieldvalues[i], other_feature_val);
    }    
}

void BasisSelectionRegressor::fitFeature(const IMPFeatureSpec& fs, real& alpha, real& b, real& newsqerror)
{
    static Vec featurevec;
    computeFeatureVec(fs, featurevec);
    int l = residue.length();

    double feature_sum    = 0.;
    double feature_sum_sq = 0.;
    double dotprod        = 0.;
    for(int i=0; i<l; i++)
    {
        double res  = residue[i];
        double fval = featurevec[i];
        double w    = weights[i];
        feature_sum += w*fval;
        feature_sum_sq += w*square(fval);
        dotprod += w*fval*res;
    }
    
    alpha = (weights_sum*dotprod - feature_sum*residue_sum)
        /(weights_sum*feature_sum_sq - square(feature_sum));

    b = (residue_sum - alpha*feature_sum)/weights_sum;

    newsqerror = residue_sum_sq - 2.*alpha*dotprod 
        - 2.*b*residue_sum + square(alpha)*feature_sum_sq 
        + 2.*alpha*b*feature_sum+weights_sum*square(b);
}


// First call should call this with  best_newsqerror = FLT_MAX;
void BasisSelectionRegressor::fitFeatureAndKeepBest(const IMPFeatureSpec& fs,  
                                                      IMPFeatureSpec& best_fs,  real& best_alpha, real& best_b, 
                                                      real& best_newsqerror)
{
    real alpha = 0;
    real b = 0;
    real newsqerror = 0;

    perr << "Trying feature: ";
    printFeature(perr, fs);
    perr << endl;

    fitFeature(fs, alpha, b, newsqerror);
    perr << "-> newsqerror = " << newsqerror << endl;

    if(newsqerror<best_newsqerror)
    {
        best_newsqerror = newsqerror;
        best_alpha = alpha;
        best_b = b;
        best_fs = fs;
    }
}

void BasisSelectionRegressor::printFunction(PStream& out) const
{
    out << bias << endl;
    for(int k=0; k<featurespecs.length(); k++)
    {
        out << "+ " << alphas[k] << "*";
        printFeature(out, featurespecs[k]);
        out << endl;
    }
}

void BasisSelectionRegressor::printFeature(PStream& out, const IMPFeatureSpec& fs) const
{
    string fieldname = train_set->fieldName(fs.fieldpos);
    string refvalstring = train_set->getValString(fs.fieldpos,fs.refval);
    string refval = tostring(fs.refval);
    if(refvalstring!="")
        refval = refval+" \""+refvalstring+"\"";

    switch(fs.functype)
    {
    case '/':
        out << "max(" << fieldname << "-" << refval  << ", 0)";
        break;
    case '>':
        out << "(" << fieldname << ">=" << refval << ")";
        break;
    case '=':
        out << "(" << fieldname << "==" << refval << ")";
        break;
    case '1':
        out << "1";
        break;
    default:
        PLERROR("Invalid functype %c",fs.functype);
    }
    switch(fs.combintype)
    {
    case '0':
        break;
    case '*':
        out << "*";
        printFeature(out,featurespecs[fs.other_feature_pos]);
        break;
    case '!':
        out << "*(1-";
        printFeature(out,featurespecs[fs.other_feature_pos]);
        out << ")";
        break;
    default:
        PLERROR("Invalid combintype %c",fs.combintype);
    }
}

void BasisSelectionRegressor::findBestNewFeature(IMPFeatureSpec& best_fs, real& best_alpha, real& best_b, real& best_newsqerror)
{
    best_newsqerror = FLT_MAX;

    TVec<IMPFeatureSpec> specs;
    IMPFeatureSpec fs;
    for(int fieldpos=0; fieldpos<inputsize(); fieldpos++)
    {
        fs.fieldpos = fieldpos;
        StatsCollector& st = train_set->getStats(fieldpos);
        if(st.nmissing()>0)
        {
            fs.refval = MISSING_VALUE;
            fs.functype = '=';
            fitFeatureAndKeepBest(fs, best_fs, best_alpha, best_b, best_newsqerror);
        }
        map<real,StatsCollectorCounts>::iterator it = st.counts.begin();
        map<real,StatsCollectorCounts>::iterator itend = st.counts.end();
        
        while(it!=itend)
        {
            real refval = it->first;
            fs.refval = refval;
            fs.combintype = '0';
            if(train_set->getValString(fieldpos,refval)!="") // categorical value
            {
                fs.functype = '=';
                fitFeatureAndKeepBest(fs, best_fs, best_alpha, best_b, best_newsqerror);
                fs.combintype = '*';
                for(int k=0; k<featurespecs.length(); k++)
                {
                    fs.other_feature_pos = k;
                    fitFeatureAndKeepBest(fs, best_fs, best_alpha, best_b, best_newsqerror);
                }                    
            }
            else
            {
                fs.functype = '/';
                fitFeatureAndKeepBest(fs, best_fs, best_alpha, best_b, best_newsqerror);
                fs.combintype = '*';
                for(int k=0; k<featurespecs.length(); k++)
                {
                    fs.other_feature_pos = k;
                    fitFeatureAndKeepBest(fs, best_fs, best_alpha, best_b, best_newsqerror);
                }                    
                fs.combintype = '0';
                fs.functype = '>';
                fitFeatureAndKeepBest(fs, best_fs, best_alpha, best_b, best_newsqerror);
                fs.combintype = '*';
                for(int k=0; k<featurespecs.length(); k++)
                {
                    fs.other_feature_pos = k;
                    fitFeatureAndKeepBest(fs, best_fs, best_alpha, best_b, best_newsqerror);
                }                    
            }
            ++it;
        }
        
    }


}

// OLD TRAIN METHOD
void BasisSelectionRegressor::train()
{
    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()

    if (!initTrain())
        return;

    Vec train_costs(1);

    Vec feature;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        IMPFeatureSpec newfs;
        real alpha = 0.;
        real b = 0.;
        real newsqerror = 0.;
        perr << "*** Stage " << stage << endl;
        if(stage==0)
            initResidueWeightAndBias();
        else
        {
            findBestNewFeature(newfs, alpha, b, newsqerror);
            perr << "+ " << alpha << "*";
            printFeature(perr, newfs);
            perr << "\n b: " << b << endl;
            perr << "newsqerror: " << newsqerror << endl;        
            appendFeature(newfs, alpha, b);
        }
        perr << "new bias: " << bias << endl;
        perr << "residue_sum_sq: " << residue_sum_sq << endl;
        perr << "recomputed residue_sum_sq: " << recompute_residue_sum_sq() << endl;
        train_costs[0] = residue_sum_sq;
        train_stats->update(train_costs, weights_sum);

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
}
#endif

real BasisSelectionRegressor::recompute_residue_sum_sq() const
{
    real rsq = 0;
    int l = train_set.length();
    Vec input(inputsize());
    Vec target(targetsize());
    real w;
    perr << "recomp_residue: { ";
    for(int i=0; i<l; i++)
    {
        train_set->getExample(i, input, target, w);
        real t = target[0];
        real pred = computePrediction(input);
        perr << t-pred << " ";
        rsq += w*square(t-pred);
    }
    perr << "}" << endl;
    
    return rsq;
}

real BasisSelectionRegressor::computePrediction(const Vec& input) const
{
    int nfeatures = alphas.length();
    Vec featurevec(nfeatures);
    
    real pred = bias;
    for(int k=0; k<nfeatures; k++)
    {
        IMPFeatureSpec& fs = featurespecs[k];
        real fieldval = input[fs.fieldpos];
        real other_feature_val = (fs.combintype=='0' ?0. :featurevec[fs.other_feature_pos]);        
        featurevec[k] = fs.evaluateFeature(fieldval, other_feature_val);
        pred += alphas[k]*featurevec[k];
    }
    
    return pred;
}

void BasisSelectionRegressor::computeOutput(const Vec& input, Vec& output) const
{
    int nout = outputsize();
    if(nout!=1)
        PLERROR("outputsize should always be one for this learner");
    output.resize(nout);
    output[0] = computePrediction(input);
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
