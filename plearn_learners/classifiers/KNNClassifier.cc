// -*- C++ -*-

// KNNClassifier.cc
//
// Copyright (C) 2004 Nicolas Chapados 
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

// Authors: Nicolas Chapados

/*! \file KNNClassifier.cc */


#include "KNNClassifier.h"
#include <assert.h>
#include <math.h>
#include <plearn_learners/nearest_neighbors/ExhaustiveNearestNeighbors.h>
#include <plearn/ker/GaussianKernel.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    KNNClassifier,
    "Classical K-Nearest-Neighbors classification algorithm",
    "This class provides a simple N-class classifier based upon an enclosed\n"
    "K-nearest-neighbors finder (derived from GenericNearestNeighbors;\n"
    "specified with the 'knn' option).  The target variable (the class), is\n"
    "assumed to be coded an integer variable (the class number, from 0 to\n"
    "C-1, where C is the number of classes); the number of classes is\n"
    "specified with the option 'nclasses'. The structure of the learner\n"
    "output is a vector of probabilities for each class (even if\n"
    "numclasses==2, which is NOT collapsed into a probability of the positive\n"
    "class).\n"
    "\n"
    "The class contains several options to determine the number of neighbors\n"
    "to use (K).  This number always overrides the option 'num_neighbors'\n"
    "that may have been specified in the GenericNearestNeighbors utility\n"
    "object.  Basically, the generic formula for the number of neighbors is\n"
    "\n"
    "    K = max(kmin, kmult*(n^kpow)),\n"
    "\n"
    "where 'kmin', 'kmult', and 'kpow' are options, and 'n' is the number of\n"
    "examples in the training set.\n"
    "\n"
    "The costs output from this class are:\n"
    "\n"
    "- 'class_error', the classification error, i.e.\n"
    "      classerror = max_i output[i] != target\n"
    "\n"
    "- 'neglogprob', the total negative log-probability of target, i.e.\n"
    "      neglogprob = -log(output[target])\n"
    "\n"
    "If the option 'use_knn_costs_as_weights' is true (by default), it is\n"
    "assumed that the costs coming from the 'knn' object are kernel\n"
    "evaluations for each nearest neighbor.  These are used as weights to\n"
    "determine the final class probabilities.  (NOTE: it is important to use\n"
    "a kernel that computes a SIMILARITY MEASURE, and not a DISTANCE MEASURE;\n"
    "the default GaussianKernel has the proper behavior.)  If the option\n"
    "is false, an equal weighting is used (equivalent to square window).\n"
    "\n"
    "The weights originally present in the training set ARE TAKEN INTO\n"
    "ACCOUNT when weighting each observation: they serve to multiply the\n"
    "kernel values to give the effective weight for an observation.\n"
    );

KNNClassifier::KNNClassifier()
    : 
      nclasses(-1),
      kmin(5),
      kmult(0.0),
      kpow(0.5),
      use_knn_costs_as_weights(true),
      kernel()
{ }

void KNNClassifier::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "knn", &KNNClassifier::knn, OptionBase::buildoption,
        "The K-nearest-neighbors finder to use (default is an\n"
        "ExhaustiveNearestNeighbors with a GaussianKernel, sigma=1)");

    declareOption(
        ol, "nclasses", &KNNClassifier::nclasses, OptionBase::buildoption,
        "Number of classes in the problem.  MUST be specified.");
  
    declareOption(
        ol, "kmin", &KNNClassifier::kmin, OptionBase::buildoption,
        "Minimum number of neighbors to use (default=5)");

    declareOption(
        ol, "kmult", &KNNClassifier::kmult, OptionBase::buildoption,
        "Multiplicative factor on n^kpow to determine number of neighbors to\n"
        "use (default=0)");

    declareOption(
        ol, "kpow", &KNNClassifier::kpow, OptionBase::buildoption,
        "Power of the number of training examples to determine number of\n"
        "neighbors (default=0.5)");

    declareOption(
        ol, "use_knn_costs_as_weights", &KNNClassifier::use_knn_costs_as_weights,
        OptionBase::buildoption,
        "Whether to weigh each of the K neighbors by the kernel evaluations,\n"
        "obtained from the costs coming out of the 'knn' object (default=true)");

    declareOption(
        ol, "kernel", &KNNClassifier::kernel, OptionBase::buildoption,
        "Disregard the 'use_knn_costs_as_weights' option, and use this kernel\n"
        "to weight the observations.  If this object is not specified\n"
        "(default), and the 'use_knn_costs_as_weights' is false, the\n"
        "rectangular kernel is used.");
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void KNNClassifier::build_()
{
    if (!knn)
        knn=new ExhaustiveNearestNeighbors(new GaussianKernel(), false);

    if (nclasses <= 1)
        PLERROR("KNNClassifier::build_: the 'nclasses' option must be specified and >= 2");

    if (kmin <= 0)
        PLERROR("KNNClassifier::build_: the 'kmin' option must be strictly positive");

}

// ### Nothing to add here, simply calls build_
void KNNClassifier::build()
{
    inherited::build();
    build_();
}


void KNNClassifier::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(knn_output,    copies);
    deepCopyField(knn_costs,     copies);
    deepCopyField(class_weights, copies);
    deepCopyField(knn,           copies);
    deepCopyField(kernel,        copies);
    inherited::makeDeepCopyFromShallowCopy(copies);
}


int KNNClassifier::outputsize() const
{
    return nclasses;
}


void KNNClassifier::setTrainingSet(VMat training_set, bool call_forget)
{
    PLASSERT( knn );
    inherited::setTrainingSet(training_set,call_forget);

    // Now we carry out a little bit of tweaking on the embedded knn:
    // - ask to output targets only
    // - set number of neighbors
    // - set training set (which performs a build if necessary)
    int n = training_set.length();
    int num_neighbors = max(kmin, int(kmult*pow(double(n),double(kpow))));
    knn->num_neighbors = num_neighbors;
    knn->copy_input  = kernel.isNotNull();
    knn->copy_target = true;
    knn->copy_weight = true;
    knn->copy_index  = false;
    knn->setTrainingSet(training_set,call_forget);
    knn_costs.resize(num_neighbors); // Changed for compatibility with HyperLearner
    //knn_costs.resize(knn->nTestCosts());
    knn_output.resize(knn->outputsize());
}

void KNNClassifier::forget()
{
    PLASSERT( knn );
    knn->forget();
}
    
void KNNClassifier::train()
{
    PLASSERT( knn );
    knn->train();
}

void KNNClassifier::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());

    // The case where a user-specified kernel complicates the situation 
    const int inputsize = input.size();
    Vec knn_targets;                           //!< not used by knn
    knn->computeOutputAndCosts(input, knn_targets, knn_output, knn_costs);
    real* output_data = knn_output.data();
  
    // Cumulate the class weights.  Compute the kernel if it's required.
    class_weights.resize(nclasses);
    class_weights.fill(0.0);
    real total_weight = 0.0;
    for (int i=0, n=knn->num_neighbors ; i<n ; ++i) {
        real w = -1.0;                           //!< safety net
        if (kernel) {
            Vec cur_input(inputsize, output_data);
            w = kernel(cur_input, input);
            output_data += inputsize;
        }
        else if (use_knn_costs_as_weights)
            w = knn_costs[i];
        else
            w = 1.0;
        int nn_class = int(*output_data++);
        if (nn_class < 0 || nn_class >= nclasses)
            PLERROR("KNNClassifier::computeOutput: expected the class to be between 0 "
                    "and %d but found %d", nclasses-1, nn_class);
        w *= *output_data++;                     //!< account for training weight
        PLASSERT( w >= 0.0 );
        class_weights[nn_class] += w;
        total_weight += w;
    }

    // If the total weight is too small, output zero probability for all classes
    if (total_weight < 1e-6) {
        output.fill(0.0);
        return;
    }
  
    // Now compute probabilities
    for (int i=0, n = nclasses; i<n ; ++i)
        class_weights[i] /= total_weight;

    // And output them
    copy(class_weights.begin(), class_weights.end(), output.begin());
}

void KNNClassifier::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                            const Vec& target, Vec& costs) const
{
    costs.resize(nTestCosts());
    int sel_class = argmax(output);
    costs[0] = sel_class != int(target[0]);
    costs[1] = -pl_log(1e-10+output[int(target[0])]);
}

TVec<string> KNNClassifier::getTestCostNames() const
{
    static TVec<string> costs(2);
    costs[0] = "class_error";
    costs[1] = "neglogprob";
    return costs;
}

TVec<string> KNNClassifier::getTrainCostNames() const
{
    return TVec<string>();
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
