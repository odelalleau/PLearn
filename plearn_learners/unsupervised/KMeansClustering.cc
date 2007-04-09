// -*- C++ -*-

// KMeansClustering.cc
//
// Copyright (C) 2004 Jean-Sébastien Senécal 
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
 ******************************************************* */

// Authors: Jean-Sébastien Senécal

/*! \file KMeansClustering.cc */


#include "KMeansClustering.h"
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

KMeansClustering::KMeansClustering()
    : inherited(), n_clusters_(0), clusters_()
{
}

PLEARN_IMPLEMENT_OBJECT(KMeansClustering,
                        "The K-Means algorithm.",
                        "This class implements the K-means algorithm. The outputs contain the "
                        "negative squared euclidian distance to each centroid.");

void KMeansClustering::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_clusters", &KMeansClustering::n_clusters_, OptionBase::buildoption,
                  "The number of clusters.");
    declareOption(ol, "clusters", &KMeansClustering::clusters_, OptionBase::learntoption,
                  "The learned centroids.");
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void KMeansClustering::build_()
{
}

void KMeansClustering::build()
{
    inherited::build();
    build_();
}


void KMeansClustering::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(clusters_, copies);
}


int KMeansClustering::outputsize() const
{
    return n_clusters_;
}

void KMeansClustering::forget()
{
    if (n_clusters_ <= 0)
        PLERROR("In KMeansClustering::build_(): number of clusters (%d) should be > 0", n_clusters_);

    static Vec input;  // static so we don't reallocate/deallocate memory each time...
    static Vec target; // (but be careful that static means shared!)

    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    clusters_.resize(n_clusters_, inputsize());

    real weight;
    
    manual_seed(seed_);
  
    // Build a vector of samples indexes to initialize clusters centers.
    Vec start_idx(n_clusters_, -1.0);
    int idx;
    for (int i=0; i<n_clusters_; i++)
    {
        bool uniq=false;
        while (!uniq)
        {
            uniq=true;
            idx = uniform_multinomial_sample(train_set.length());
            for (int j=0; j < n_clusters_ && start_idx[j] != -1.0; j++)
                if (start_idx[j] == idx)
                {
                    uniq=false;
                    break;
                }
        }
        start_idx[i] = idx;
        train_set->getExample(idx,input,target,weight);
        clusters_(i) << input;
    }

    stage = 0;
}
    
void KMeansClustering::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.

    PLASSERT( n_clusters_ > 0 );
  
    static Vec input;  // static so we don't reallocate/deallocate memory each time...
    static Vec target; // (but be careful that static means shared!)

    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    clusters_.resize(n_clusters_, inputsize());
    
    real weight;

    if(!train_stats)  // make a default stats collector, in case there's none
        train_stats = new VecStatsCollector();

    if(nstages<stage) // asking to revert to a previous stage!
        forget();  // reset the learner to stage=0

    Vec samples_per_cluster(n_clusters_);
  
    Mat new_clusters(n_clusters_,train_set->inputsize());
    TVec<int> cluster_idx(train_set.length());
    TVec<int> old_cluster_idx(train_set.length());
    Vec train_costs(nTrainCosts());
    clusters_.resize(n_clusters_,train_set->inputsize());

    bool stop = false;
    // Training loop.
    while(!stop && stage<nstages)
    {
        // Clear statistics of previous epoch.
        train_stats->forget();

        // Init.
        new_clusters.clear();
        samples_per_cluster.clear();
        old_cluster_idx << cluster_idx;
        train_costs.clear();

        // Redistribute points in closest centroid.
        for (int i=0; i<train_set.length(); i++)
        {
            train_set->getExample(i,input,target,weight);
            real dist, bestdist=1E300;
            int bestclust=0;
      
            if (n_clusters_ > 1)
                for (int j=0; j<n_clusters_; j++)
                    if ((dist = powdistance(clusters_(j), input, 2)) < bestdist)
                    {
                        bestdist = dist;
                        bestclust = j;
                    }
      
            cluster_idx[i] = bestclust;
            samples_per_cluster[bestclust] += weight;
            new_clusters(bestclust) += input * weight;
            train_costs[0] += bestdist;
        }

        train_costs[0] /= train_set.length();

        // Update train statistics.
        train_stats->update(train_costs);
        train_stats->finalize(); // finalize statistics for this epoch

        // Compute new centroids.
        for (int i=0; i<n_clusters_; i++)
            if (samples_per_cluster[i]>0)
                new_clusters(i) /= samples_per_cluster[i];
        clusters_ << new_clusters;

        // Check if things have changed (if not, stop training).
        stop=true;
        if (n_clusters_ > 1)
            for (int i=0;i<train_set.length();i++)
                if (old_cluster_idx[i] != cluster_idx[i])
                {
                    stop=false;
                    break;
                }

        ++stage; // next stage
    }
}


void KMeansClustering::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    int nout = outputsize();
    output.resize(nout);

    for (int j=0; j<n_clusters_; j++)
        output[j] = -powdistance(clusters_(j), input, 2);
}

void KMeansClustering::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                               const Vec& target, Vec& costs) const
{
    // Compute the costs from *already* computed output.
    costs.resize(1);
    int cluster = argmax(output);
  
    costs[0] = - output[cluster];
}

TVec<string> KMeansClustering::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutpus
    // (these may or may not be exactly the same as what's returned by getTrainCostNames).
    return TVec<string>(1, "squared_reconstruction_error");
}

TVec<string> KMeansClustering::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes and 
    // for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by getTestCostNames).
    return getTestCostNames();
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
