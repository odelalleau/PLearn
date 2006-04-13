// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent
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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearnLibrary/PLearnAlgo/GaussianDistribution.cc */

#include "GaussianDistribution.h"
#include <plearn/vmat/VMat_basic_stats.h>
#include <plearn/math/plapack.h>
#include <plearn/math/distr_maths.h>

namespace PLearn {
using namespace std;

#define ZEROGAMMA

PLEARN_IMPLEMENT_OBJECT(GaussianDistribution,
                        "A Gaussian distribution represented compactly by the k leading eigenvalues and eigenvectors of its covariance matrix.",
                        "This class can be used either to fit a Gaussian to data \n"
                        "or to explicitly represent a Gaussian with a covariance matrix \n"
                        "of the form C = VDV' (possibly regularized by adding gamma.I).\n"
                        "When fitting to data, an eigendecomposition of the empirical \n"
                        "covariance matrix is performed, and the top k eigenvalues\n"
                        "and associated eigenvectors V are kept.\n"
                        "The actual variances used for the principal directions in D are obtained\n"
                        "from the empirical or specified eigenvalues in the following way:\n"
                        "  var_i = max(eigenvalue_i+gamma, min_eig) \n"
                        "In addition, a variance for the remaining directions \n"
                        "in the null space of VDV' (directions orthogonal to the \n"
                        "eigenvectors in V) is obtained by:\n"
                        "  remaining_var = use_last_eig?max(last_eigenvalue+gamma, min_eig) \n"
                        "                              :max(gamma, min_eig) \n"
                        "So the full expression of the actual covariance matrix used is: \n"
                        "  C = VDV' + remaining_var.I \n"
                        "with D_ii = max(eigenvalue_i+gamma, min_eig) - remaining_var \n"
                        "Note that with min_eig=0 and use_last_eig=false, we get: \n"
                        "  C = V.diag(eigenvalues).V' + gamma.I \n");



void GaussianDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(mu, copies);
    deepCopyField(eigenvalues, copies);
    deepCopyField(eigenvectors, copies);
}


GaussianDistribution::GaussianDistribution()
    :k(1000),
     gamma(0),
     min_eig(0),
     use_last_eig(false),
     ignore_weights_below(0)
{
}


void GaussianDistribution::declareOptions(OptionList& ol)
{
    // Build options
    declareOption(ol, "k", &GaussianDistribution::k, OptionBase::buildoption,
                  "number of eigenvectors to keep when training");

    declareOption(ol, "gamma", &GaussianDistribution::gamma, OptionBase::buildoption,
                  "Value to add to the empirical eigenvalues to obtain actual variance.\n");
    declareOption(ol, "min_eig", &GaussianDistribution::min_eig, OptionBase::buildoption,
                  "Imposes a minimum over the actual variances to be used.\n"
                  "Actual variance used in the principal directions is max(min_eig, eigenvalue_i+gamma)\n");
    declareOption(ol, "use_last_eig", &GaussianDistribution::use_last_eig, OptionBase::buildoption,
                  "If true, the actual variance used for directions in the nullspace of VDV' \n"
                  "(i.e. orthogonal to the kept eigenvectors) will be the same as the\n"
                  "actual variance used for the last principal direction. \n"
                  "If false, the actual variance used for directions in the nullspace \n"
                  "will be max(min_eig, gamma)\n");

    declareOption(ol, "ignore_weights_below", &GaussianDistribution::ignore_weights_below, OptionBase::buildoption | OptionBase::nosave,
                  "DEPRECATED: When doing a weighted fitting (weightsize==1), points with a weight below this value will be ignored");

    // Learnt options
    declareOption(ol, "mu", &GaussianDistribution::mu, OptionBase::learntoption, "");
    declareOption(ol, "eigenvalues", &GaussianDistribution::eigenvalues, OptionBase::learntoption, "");
    declareOption(ol, "eigenvectors", &GaussianDistribution::eigenvectors, OptionBase::learntoption, "");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GaussianDistribution::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GaussianDistribution::build_()
{
    if (!fast_exact_is_equal(ignore_weights_below, 0))
        PLERROR("In GaussianDistribution::build_ - For the sake of simplicity, the "
                "option 'ignore_weights_below' in GaussianDistribution has been "
                "removed. If you were using it, please feel free to complain.");
}

void GaussianDistribution::forget()
{ }

void GaussianDistribution::train()
{
    VMat training_set = getTrainingSet();
    int l = training_set.length();
    int d = training_set.width();
    int ws = training_set->weightsize();

    if(d!=inputsize()+ws)
        PLERROR("In GaussianDistribution::train width of training_set should be equal to inputsize()+weightsize()");

    // these are used in SVD
    static Mat trainmat;
    static Mat U;

    // The maximum number of eigenvalues we want.
    int maxneigval = min(k, min(l,d));

    // First get mean and covariance
    // (declared static to avoid repeated dynamic memory allocation)
    static Mat covarmat;

    if(ws==0)
        computeMeanAndCovar(training_set, mu, covarmat);
    else if(ws==1)
        computeInputMeanAndCovar(training_set, mu, covarmat);
    else
        PLERROR("In GaussianDistribution, weightsize can only be 0 or 1");

    // cerr << "maxneigval: " << maxneigval << " ";
    // cerr << eigenvalues.length() << endl;
    // cerr << "eig V: \n" << V << endl;

    // Compute eigendecomposition only if there is a training set...
    // Otherwise, just fill the eigen-* matrices to all NaN...
    if (l>0 && maxneigval>0)
    {
        eigenVecOfSymmMat(covarmat, maxneigval, eigenvalues, eigenvectors);
        int neig = 0;
        while(neig<eigenvalues.length() && eigenvalues[neig]>0.)
            neig++;
        eigenvalues.resize(neig);
        eigenvectors.resize(neig,mu.length());
    }
    else
    {
        eigenvalues.resize(0);
        eigenvectors.resize(0, mu.length());
        /*
          eigenvalues.resize(maxneigval);
          eigenvectors.resize(maxneigval, mu.size());
          eigenvalues.fill(0);
          eigenvectors.fill(0);
        */
    }
}

real GaussianDistribution::log_density(const Vec& x) const
{
    static Vec actual_eigenvalues;

    if(min_eig<=0 && !use_last_eig)
        return logOfCompactGaussian(x, mu, eigenvalues, eigenvectors, gamma, true);
    else
    {
        int neig = eigenvalues.length();
        real remaining_eig = 0; // variance for directions in null space
        actual_eigenvalues.resize(neig);
        for(int j=0; j<neig; j++)
            actual_eigenvalues[j] = max(eigenvalues[j]+gamma, min_eig);
        if(use_last_eig)
            remaining_eig = actual_eigenvalues[neig-1];
        else
            remaining_eig = max(gamma, min_eig);
        return logOfCompactGaussian(x, mu, actual_eigenvalues, eigenvectors, remaining_eig);
    }
}

void GaussianDistribution::generate(Vec& x) const
{
    static Vec r;
    int neig = eigenvalues.length();
    int m = mu.length();
    r.resize(neig);

    real remaining_eig = 0;
    if(use_last_eig)
        remaining_eig = max(eigenvalues[neig-1]+gamma, min_eig);
    else
        remaining_eig = max(gamma, min_eig);

    random_gen->fill_random_normal(r);
    for(int i=0; i<neig; i++)
    {
        real neweig = max(eigenvalues[i]+gamma, min_eig)-remaining_eig;
        r[i] *= sqrt(neweig);
    }
    x.resize(m);
    transposeProduct(x,eigenvectors,r);
    if(remaining_eig>0.)
    {
        r.resize(m);
        random_gen->fill_random_normal(r,0,sqrt(remaining_eig));
        x += r;
    }
    x += mu;
}

///////////////
// inputsize //
///////////////
int GaussianDistribution::inputsize() const {
    if (train_set || mu.length() == 0)
        return inherited::inputsize();
    return mu.length();
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
