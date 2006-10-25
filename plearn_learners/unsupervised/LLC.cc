// -*- C++ -*-

// LLC.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file LLC.cc */


#include "LLC.h"
#include <plearn/io/openString.h>
#include <plearn/ker/ReconstructionWeightsKernel.h>
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

/////////
// LLC //
/////////
LLC::LLC() 
    : sum_of_dim(-1),
      knn(5),
      n_comp(1),
      regularization(0),
      train_mixture(true)
{}

PLEARN_IMPLEMENT_OBJECT(LLC,
                        "Locally Linear Coordination.",
                        "This is the algorithm described in 'Automatic alignment of local representations'\n"
                        "by Teh and Roweis (2003).\n"
    );

////////////////////
// declareOptions //
////////////////////
void LLC::declareOptions(OptionList& ol)
{
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // Build options.

    declareOption(ol, "knn", &LLC::knn, OptionBase::buildoption,
                  "Number of neighbors used to compute local reconstruction weights.");

    declareOption(ol, "mixture", &LLC::mixture, OptionBase::buildoption,
                  "A mixture of local dimensionality reducers.");

    declareOption(ol, "n_comp", &LLC::n_comp, OptionBase::buildoption,
                  "Number of components computed.");

    declareOption(ol, "regularization", &LLC::regularization, OptionBase::buildoption,
                  "A regularization coefficient (to use if crash in the eigensystem, but assume the consequences).");

    declareOption(ol, "train_mixture", &LLC::train_mixture, OptionBase::buildoption,
                  "Whether the mixture should be trained or not.");

    // Learnt options.

    declareOption(ol, "L", &LLC::L, OptionBase::learntoption,
                  "The matrix of factors (bias and linear transformation for each neighborhood).");

    declareOption(ol, "sum_of_dim", &LLC::sum_of_dim, OptionBase::learntoption,
                  "Must be equal to mixture->outputsize().");

    // Now call the parent class' declareOptions.
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void LLC::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void LLC::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
    if (sum_of_dim > 0)
        mixture_output.resize(sum_of_dim);
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void LLC::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                  const Vec& target, Vec& costs) const
{
    // No cost to compute.
}                                

///////////////////
// computeOutput //
///////////////////
void LLC::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(n_comp);
    // As in the train method, we assume the mixture has just the nice output
    // we need.
    mixture->computeOutput(input, mixture_output);
    product(output, L, mixture_output);
}    

////////////
// forget //
////////////
void LLC::forget()
{
    stage = 0;
    sum_of_dim = -1;
    L.resize(0,0);
}
    
//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> LLC::getTestCostNames() const
{
    static TVec<string> noCost;
    return noCost;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> LLC::getTrainCostNames() const
{
    static TVec<string> noCost;
    return noCost;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LLC::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("LLC::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////
// outputsize //
////////////////
int LLC::outputsize() const
{
    return n_comp;
}

///////////
// train //
///////////
void LLC::train()
{
    PLASSERT( mixture );
    if (stage >= nstages) {
        PLWARNING("In LLC::train - Learner has already been trained, skipping training");
        return;
    }
    if (verbosity >= 2)
        pout << "Computing local reconstruction weights" << endl;
    PP<ReconstructionWeightsKernel> reconstruct = new ReconstructionWeightsKernel();
    reconstruct->knn = knn + 1; // +1 because it includes the point itself.
    reconstruct->build();
    reconstruct->setDataForKernelMatrix(train_set);
    int n = train_set->length();
    Mat lle_mat(n,n);
    reconstruct->computeLLEMatrix(lle_mat); // Fill lle_mat with W + W' - W' W.
    for (int i = 0; i < n; i++)
        lle_mat(i,i) = lle_mat(i,i) - 1;    // lle_mat = - (I - W') * (I - W)
    if (train_mixture) {
        if (verbosity >= 2)
            pout << "Training mixture" << endl;
        mixture->setTrainingSet(train_set);
        mixture->train();
    }
    // Obtain the number of components in the mixture (= the number of 'experts').
    // We assume here the mixture has a 'n_components' option.
    int n_comp_mixture;
    openString(mixture->getOption("n_components"), PStream::plearn_ascii) >> n_comp_mixture;
    // Obtain the dimension of each expert in the mixture.
    // We assume here the mixture has a 'outputsizes' option which is a TVec<int>
    // containing the outputsize of each expert.
    TVec<int> dimension;
    PStream in = openString(mixture->getOption("outputsizes"), PStream::plearn_ascii);
    in >> dimension; // TODO See what's wrong...
    sum_of_dim = n_comp_mixture;
    for (int k = 0; k < dimension.length(); k++)
        sum_of_dim += dimension[k];
    mixture_output.resize(sum_of_dim);
    // Compute the output of the mixture for all elements in the training set.
    // The output must be a vector of size 'sum_of_dim' which is the concatenation
    // of the output of each expert in the mixture, each weighted by its
    // responsibility r_k (that can depend on x, and such that sum_k r_k = 1),
    // and with a bias (= r_k) added as the first dimension of each expert.
    if (verbosity >= 2)
        pout << "Computing mixture outputs" << endl;
    Mat U(n, sum_of_dim);
    mixture->useOnTrain(U);
    if (verbosity >= 2)
        pout << "Building the generalized eigenvector system" << endl;
    Mat B(sum_of_dim, sum_of_dim);
    transposeProduct(B, U, U);
    B /= real(1.0 / n);           // B = 1/n U' U
    Mat A(sum_of_dim, sum_of_dim);
    Mat tmp(n, sum_of_dim);
    product(tmp, lle_mat, U);
    // A = - U' (I - W') (I - W) U (because we want the smallest eigenvalues).
    transposeProduct(A, U, tmp);
    tmp = Mat(); // Free memory.
    fillItSymmetric(A); // A and B should be already symmetric, but it may be safer
    fillItSymmetric(B); // to ensure it.
    if (verbosity >= 2)
        pout << "Solving the generalized eigensystem" << endl;
    Vec eigen_val;
    Mat eigen_vec;
    if (regularization > 0)
        regularizeMatrix(B, regularization);
    generalizedEigenVecOfSymmMat(A, B, 1, n_comp + 1, eigen_val, eigen_vec);
    // Ignore the smallest eigenvalue (should be 0).
    if (verbosity >= 5)
        pout << "Smallest eigenvalue: " << eigen_val[0] << endl;
    L = eigen_vec.subMatRows(1, eigen_vec.length() - 1);
    if (verbosity >= 2)
        pout << "Training is over" << endl;
    stage = 1;
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
