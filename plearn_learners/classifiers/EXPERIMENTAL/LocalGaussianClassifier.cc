// -*- C++ -*-

// LocalGaussianClassifier.cc
//
// Copyright (C) 2007 Pascal Vincent
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

/*! \file LocalGaussianClassifier.cc */


#include "LocalGaussianClassifier.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/distr_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LocalGaussianClassifier,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

LocalGaussianClassifier::LocalGaussianClassifier()
    :nclasses(-1),
     computation_neighbors(-1),
     kernel_sigma(0.1),
     regularization_sigma(1e-6),
     ignore_weights_below(1e-8),
     minus_one_half_over_kernel_sigma_square(0),
     traintarget_ptr(0),
     trainweight_ptr(0)
{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)

    // ### If this learner needs to generate random numbers, uncomment the
    // ### line below to enable the use of the inherited PRandom object.
    // random_gen = new PRandom();
}

void LocalGaussianClassifier::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &LocalGaussianClassifier::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(ol, "nclasses", &LocalGaussianClassifier::nclasses, OptionBase::buildoption,
                  "The number of different classes.\n"
                  "Note that the 'target' part of trining set samples must be an integer\n"
                  "with values between 0 and nclasses-1.\n");

    declareOption(ol, "computation_neighbors", &LocalGaussianClassifier::computation_neighbors, OptionBase::buildoption,
                  "This indicates to how many neighbors we should restrict ourselves for the computation\n"
                  "of the covariance matrix only (since they are much cheaper, weight and mean are always\n"
                  "computed using all points.)\n"
                  "If =0 we do not compute a covariance matrix (i.e. use a spherical cov. of width regularization_sigma).\n"
                  "If <0 we use all training points (with an appropriate weight).\n"
                  "If >1 we consider only that many neighbors of the test point;\n"
                  "If between 0 and 1, it's considered a coefficient by which to multiply\n"
                  "the square root of the number of training points, to yield the actual \n"
                  "number of computation neighbors used");

    declareOption(ol, "kernel_sigma", &LocalGaussianClassifier::kernel_sigma, OptionBase::buildoption,
                  "The sigma (standard deviation) of the weighting Gaussian Kernel\n");

    declareOption(ol, "regularization_sigma", &LocalGaussianClassifier::regularization_sigma, OptionBase::buildoption,
                  "This quantity squared is added to the diagonal of the local empirical covariance matrices.\n");

    declareOption(ol, "ignore_weights_below", &LocalGaussianClassifier::ignore_weights_below, OptionBase::buildoption,
                  "minimal weight below which we ignore the point (i.e. consider the weight is 0)\n");

    declareOption(ol, "train_set", &LocalGaussianClassifier::train_set, OptionBase::learntoption,
                  "We need to store the training set, as this learner is memory-based...");

    /*
    declareOption(ol, "NN", &LocalGaussianClassifier::NN, OptionBase::learntoption,
                  "The nearest neighbor algorithm used to find nearest neighbors");
    */

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LocalGaussianClassifier::build_()
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
    // PLASSERT(weighting_kernel.isNotNull());

    if(train_set.isNotNull())
        setTrainingSet(train_set, false);
    
    minus_one_half_over_kernel_sigma_square = -0.5/(kernel_sigma*kernel_sigma);
}

// ### Nothing to add here, simply calls build_
void LocalGaussianClassifier::build()
{
    inherited::build();
    build_();
}


void LocalGaussianClassifier::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);
    // deepCopyField(weighting_kernel, copies);
    // deepCopyField(NN, copies);
}


int LocalGaussianClassifier::outputsize() const
{
    return nclasses;
}

void LocalGaussianClassifier::forget()
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
    inherited::forget();
}

void LocalGaussianClassifier::train()
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
    if (!initTrain())
        return;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
    */
}

void LocalGaussianClassifier::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);
    
    // int l = train_set.length();
    int is = inputsize();
    int ts = targetsize();
    PLASSERT(ts==1);
    int ws = weightsize();
    PLASSERT(ws==0 || ws==1);
    trainsample.resize(is+ts+ws);
    traininput = trainsample.subVec(0,is);
    traintarget_ptr = &trainsample[is];
    trainweight_ptr = NULL;
    if(ws==1)
        trainweight_ptr = &trainsample[is+ts];
    
    log_counts.resize(nclasses);
    log_counts2.resize(nclasses);
    means.resize(nclasses, is);
    allcovars.resize(nclasses*is, is);
    covars.resize(nclasses);
    for(int c=0; c<nclasses; c++)
        covars[c] = allcovars.subMatRows(c*is, is);
}

real LocalGaussianClassifier::computeLogWeight(const Vec& input, const Vec& traininput) const
{
    return powdistance(input, traininput, 2.0, true)*minus_one_half_over_kernel_sigma_square;
}

void LocalGaussianClassifier::computeOutput(const Vec& input, Vec& output) const
{
    int l = train_set.length();
    PLASSERT(input.length()==inputsize());

    int K = 0;
    if(computation_neighbors>1)
        K = int(computation_neighbors);
    else if(computation_neighbors>0)
        K = int(computation_neighbors*sqrt(l));
    else if(computation_neighbors<0)
        K = l;
    if(K>l)
        K = l;

    pqvec.resize(K+1);
    pair<real,int>* pq = pqvec.begin();
    int pqsize = 0;
    
    log_counts.fill(-FLT_MAX);
    if(K>0)
        log_counts2.fill(-FLT_MAX);
        
    if(verbosity>=3)
        perr << "______________________________________" << endl;
    means.clear();
    real ignore_log_weights_below = pl_log(ignore_weights_below);

    for(int i=0; i<l; i++)
    {
        train_set->getRow(i,trainsample);
        real log_w = computeLogWeight(input, traininput);
        if(trainweight_ptr) 
            log_w += pl_log(*trainweight_ptr);
        if(log_w>=ignore_log_weights_below)
        {
            if(K>0)
                {
                    real d = -log_w;
                    if(pqsize<K)
                    {
                        pq[pqsize++] = pair<real,int>(d,i);
                        if(K<l) // need to maintain heap structure only if K<l
                            push_heap(pq,pq+pqsize);
                    }
                    else if(d<pq->first)
                    {
                        pop_heap(pq,pq+pqsize);
                        pq[pqsize-1] = pair<real,int>(d,i);
                        push_heap(pq,pq+pqsize);
                    }
                }
            int c = int(*traintarget_ptr);
            real lcc = log_counts[c];
            log_counts[c] = (lcc<ignore_log_weights_below ?log_w :logadd(lcc, log_w));
            multiplyAcc(means(c), traininput, exp(log_w));
        }
    }

    if(verbosity>=3)
        perr << "log_counts: " << log_counts << endl;

    for(int c=0; c<nclasses; c++)
        if(log_counts[c]>=ignore_log_weights_below)
            means(c) *= exp(-log_counts[c]);

    allcovars.fill(0.);
    if(K>0) // compute covars?
    {
        for(int k=0; k<pqsize; k++)
        {
            int i = pq[k].second;
            real log_w = -pq[k].first;
            train_set->getRow(i,trainsample);
            int c = int(*traintarget_ptr);
            real lcc = log_counts2[c];
            log_counts2[c] = (lcc<ignore_log_weights_below ?log_w :logadd(lcc, log_w));
            traininput -= means(c);
            externalProductScaleAcc(covars[c], traininput, traininput, exp(log_w));
        }
        
        for(int c=0; c<nclasses; c++)
            if(log_counts2[c]>=ignore_log_weights_below)
                covars[c] *= exp(-log_counts2[c]);
    if(verbosity>=3)
        perr << "log_counts2: " << log_counts2 << endl;
    }

    output.resize(nclasses);
    output.clear();

    for(int c=0; c<nclasses; c++)
    {
        if(log_counts[c]<ignore_log_weights_below)
            output[c] = -FLT_MAX;
        else
        {
            Mat cov = covars[c];
            addToDiagonal(cov, square(regularization_sigma));
            real log_p_x = logOfNormal(input, means(c), cov);
            output[c] = log_p_x + log_counts[c];
            if(verbosity>=4)
            {
                perr << "** Class " << c << " **" << endl;
                perr << "log_p_x: " << log_p_x << endl;
                perr << "log_count: " << log_counts[c] << endl;
                perr << "mean: " << means(c) << endl;
                perr << "regularized covar: \n" << cov << endl;
            }
        }
    }
    if(verbosity>=2)
    {
        perr << "Scores: " << output << endl;
        perr << "argmax: " << argmax(output) << endl;
    }
}

void LocalGaussianClassifier::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    costs.resize(2);
    int c = int(target[0]);
    costs[0] = (argmax(output)==c ?0.0 :1.0);
    costs[1] = logadd(output)-output[c];
}

TVec<string> LocalGaussianClassifier::getTestCostNames() const
{
    TVec<string> names(2);
    names[0] = "class_error";
    names[1] = "NLL";
    return names;
}

TVec<string> LocalGaussianClassifier::getTrainCostNames() const
{
    TVec<string> names;
    return names;
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
