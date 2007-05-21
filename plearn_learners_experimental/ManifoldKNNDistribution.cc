// -*- C++ -*-

// ManifoldKNNDistribution.cc
//
// Copyright (C) 2007 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file ManifoldKNNDistribution.cc */


#include "ManifoldKNNDistribution.h"
#include <plearn/math/pl_erf.h>
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ManifoldKNNDistribution,
    "K nearest neighbors density estimator locally taking into account the manifold",
    ""
    );

/////////////////////////////
// ManifoldKNNDistribution //
/////////////////////////////
ManifoldKNNDistribution::ManifoldKNNDistribution()
    : manifold_dimensionality(5),
      min_sigma_square(1e-5),
      center_around_manifold_neighbors(false)
{}

////////////////////
// declareOptions //
////////////////////
void ManifoldKNNDistribution::declareOptions(OptionList& ol)
{
    declareOption(ol, "knn_manifold", &ManifoldKNNDistribution::knn_manifold,
                  OptionBase::buildoption,
                  "Nearest neighbors search algorithms for local manifold structure"
                  "estimation.");

    declareOption(ol, "knn_density", &ManifoldKNNDistribution::knn_density,
                  OptionBase::buildoption,
                  "Nearest neighbors search algorithms for density "
                  "estimation from ellipsoid\n"
                  "volume.");

    declareOption(ol, "manifold_dimensionality", 
                  &ManifoldKNNDistribution::manifold_dimensionality,
                  OptionBase::buildoption,
                  "Dimensionality of the manifold.");

    declareOption(ol, "min_sigma_square", 
                  &ManifoldKNNDistribution::min_sigma_square,
                  OptionBase::buildoption,
                  "Minimum variance in all directions on the manifold. This value"
                  "is added\n"
                  "to the estimated covariance matrix.");

    declareOption(ol, "center_around_manifold_neighbors", 
                  &ManifoldKNNDistribution::center_around_manifold_neighbors,
                  OptionBase::buildoption,
                  "Indication that the estimation of the manifold tangent vectors\n"
                  "should be made around the knn_manifold neighbors' mean vector,\n"
                  "not around the test point."
                  );

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ManifoldKNNDistribution::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ManifoldKNNDistribution::build_()
{
    if(min_sigma_square < 0)
        PLERROR("In ManifoldKNNDistribution::build_(): min_sigma_square should be"
                " >= 0.");

    if(!knn_manifold)
        PLERROR("In ManifoldKNNDistribution::build_(): knn_manifold must be"
                " provided.");

    if(!knn_density)
        PLERROR("In ManifoldKNNDistribution::build_(): knn_density must be"
                " provided.");

    if(inputsize_ > 0)
    {
        if(manifold_dimensionality > inputsize_)
            manifold_dimensionality = inputsize_;
        if(manifold_dimensionality < 1)
            PLERROR("In ManifoldKNNDistribution::build_(): manifold_dimensionality"
                    " should be > 0.");
        eig_vectors.resize(manifold_dimensionality,inputsize_);
        eig_values.resize(manifold_dimensionality);
        Ut.resize(inputsize_,inputsize_);
        V.resize(knn_manifold->num_neighbors,knn_manifold->num_neighbors);
        eig_vectors_projection.resize(manifold_dimensionality);
        neighbors_mean.resize(inputsize_);
    }

    if(train_set)
    {
        knn_manifold->setTrainingSet(train_set,true);
        knn_density->setTrainingSet(train_set,true);
    }
}

/////////
// cdf //
/////////
real ManifoldKNNDistribution::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for ManifoldKNNDistribution"); return 0;
}

/////////////////
// expectation //
/////////////////
void ManifoldKNNDistribution::expectation(Vec& mu) const
{
    PLERROR("expectation not implemented for ManifoldKNNDistribution");
}

// ### Remove this method if your distribution does not implement it.
////////////
// forget //
////////////
void ManifoldKNNDistribution::forget()
{}

//////////////
// generate //
//////////////
void ManifoldKNNDistribution::generate(Vec& y) const
{
    PLERROR("generate not implemented for ManifoldKNNDistribution");
}

/////////////////
// log_density //
/////////////////
real ManifoldKNNDistribution::log_density(const Vec& y) const
{
    computeLocalPrincipalComponents(y,eig_values,eig_vectors);

    // Find volume of ellipsoid defined by eig_values, eig_vectors and
    // min_sigma_square that covers all the nearest_neighbors found by knn_density
    knn_density->computeOutput(y,nearest_neighbors_density_vec);
    nearest_neighbors_density = 
        nearest_neighbors_density_vec.toMat(knn_density->num_neighbors,inputsize_);
    nearest_neighbors_density -= y;
    real max = -1;
    real scaled_projection;
    for(int i=0; i<nearest_neighbors_density.length(); i++)
    {
        scaled_projection = 0;
        product(eig_vectors_projection,eig_vectors,nearest_neighbors_density(i));
        for(int j=0; j<eig_values.length(); j++)
            scaled_projection += mypow(eig_vectors_projection[j],2) * 
                (1/(eig_values[j]+min_sigma_square) 
                 - 1/min_sigma_square) ;
        scaled_projection += pownorm(nearest_neighbors_density(i),2)
            /min_sigma_square;
        if(max < scaled_projection)
            max = scaled_projection;
    }

    // Compute log-volume of the ellipsoid: pi
    real log_vol = 0.5 * inputsize_ * pl_log(scaled_projection);
    for(int i=0; i<manifold_dimensionality; i++)
        log_vol += 0.5 * pl_log(eig_values[i]+min_sigma_square);
    log_vol += (inputsize_-manifold_dimensionality)*0.5*pl_log(min_sigma_square);
    log_vol += 0.5*inputsize_*pl_log(Pi) - pl_gammln(0.5*inputsize_+1);
    
    return pl_log((real)knn_density->num_neighbors)-pl_log((real)n_examples)-log_vol;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ManifoldKNNDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(knn_manifold, copies);
    deepCopyField(knn_density, copies);
    deepCopyField(nearest_neighbors_manifold, copies);
    deepCopyField(nearest_neighbors_manifold_vec, copies);
    deepCopyField(nearest_neighbors_density, copies);
    deepCopyField(nearest_neighbors_density_vec, copies);
    deepCopyField(eig_vectors, copies);
    deepCopyField(eig_values, copies);
    deepCopyField(Ut, copies);
    deepCopyField(V, copies);
    deepCopyField(S, copies);
    deepCopyField(eig_vectors_projection, copies);
    deepCopyField(neighbors_mean,copies);
}

////////////////////
// resetGenerator //
////////////////////
void ManifoldKNNDistribution::resetGenerator(long g_seed) const
{
    PLERROR("resetGenerator not implemented for ManifoldKNNDistribution");
}

/////////////////
// survival_fn //
/////////////////
real ManifoldKNNDistribution::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for ManifoldKNNDistribution"); return 0;
}

// ### Remove this method, if your distribution does not implement it.
///////////
// train //
///////////
void ManifoldKNNDistribution::train()
{}

//////////////
// variance //
//////////////
void ManifoldKNNDistribution::variance(Mat& covar) const
{
    PLERROR("variance not implemented for ManifoldKNNDistribution");
}

void ManifoldKNNDistribution::computeLocalPrincipalComponents(const Vec& x, 
                                         Vec& eig_values, Mat& eig_vectors) const
{
    knn_manifold->computeOutput(x,nearest_neighbors_manifold_vec);
    nearest_neighbors_manifold = 
        nearest_neighbors_manifold_vec.toMat(knn_manifold->num_neighbors,inputsize_);

    if(center_around_manifold_neighbors)
    {
        columnMean(nearest_neighbors_manifold,neighbors_mean);
        nearest_neighbors_manifold -= neighbors_mean;
    }
    else
        nearest_neighbors_manifold -= x;
    
    // Compute principal components
    // N.B. this is the SVD of F'
    lapackSVD(nearest_neighbors_manifold, Ut, S, V,'A',1.5);
    for (int k=0;k<manifold_dimensionality;k++)
    {
        eig_values[k] = mypow(S[k],2);
        eig_vectors(k) << Ut(k);
    }  
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
