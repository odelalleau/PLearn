// -*- C++ -*-

// PCA.cc
//
// Copyright (C) 2003  Pascal Vincent 
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

/*! \file PCA.cc */
#include <plearn/vmat/CenteredVMatrix.h>
#include <plearn/vmat/GetInputVMatrix.h>
#include "PCA.h"
#include <plearn/math/plapack.h>
#include <plearn/math/random.h>     //!< For fill_random_normal.
#include <plearn/vmat/VMat_basic_stats.h>

namespace PLearn {
using namespace std;

PCA::PCA() 
    : _oldest_observation(-1),
      algo("classical"),
      _horizon(-1),
      ncomponents(2),
      sigmasq(0),
      normalize(false),
      normalize_warning(true),
      impute_missings(false)
{ }

PLEARN_IMPLEMENT_OBJECT(
    PCA, 
    "Performs a Principal Component Analysis preprocessing (projecting on the principal directions).",
    "This learner finds the empirical covariance matrix of the input part of\n"
    "the training data, and learns to project its input vectors along the\n"
    "principal eigenvectors of that matrix, optionally scaling by the inverse\n"
    "of the square root of the eigenvalues (to obtained 'sphered', i.e.\n"
    "Normal(0,I) data).\n"
    "Alternative EM algorithms are provided, that may be useful when there is\n"
    "a lot of data or the dimension is very high.\n"
    );

void PCA::declareOptions(OptionList& ol)
{
    declareOption(ol, "ncomponents", &PCA::ncomponents, OptionBase::buildoption,
                  "The number of principal components to keep (that's also the outputsize).");
  
    declareOption(ol, "sigmasq", &PCA::sigmasq, OptionBase::buildoption,
                  "This gets added to the diagonal of the covariance matrix prior to eigen-decomposition.");
  
    declareOption(ol, "normalize", &PCA::normalize, OptionBase::buildoption, 
                  "If true, we divide by sqrt(eigenval) after projecting on the eigenvec.");
  
    declareOption( ol, "algo", &PCA::algo,
                   OptionBase::buildoption,
                   "The algorithm used to perform the Principal Component Analysis:\n"
                   "    - 'classical'   : compute the eigenvectors of the covariance matrix\n"
                   "    \n"
                   "    - 'incremental' : Uses the classical algorithm but compute the\n"
                   "                      covariance matrix in an incremental manner. \n"
                   "    \n"
                   "    - 'em'          : EM algorithm from \"EM algorithms for PCA and\n"
                   "                      SPCA\" by S. Roweis\n"
                   "    \n"
                   "    - 'em_orth'     : a variant of 'em', where orthogonal components\n"
                   "                      are directly computed          " );

    declareOption( ol, "horizon", &PCA::_horizon,
                   OptionBase::buildoption,
                   "Incremental algorithm option: This option specifies a window over\n"
                   "which the PCA should be done. That is, if the length of the training\n"
                   "set is greater than 'horizon', the observations that will effectively\n"
                   "contribute to the covariance matrix will only be the last 'horizon'\n"
                   "ones. All negative values being interpreted as 'keep all observations'.\n"
                   "\n"
                   "Default: -1 (all observations are kept)" );
  
    // TODO Option added October 26th, 2004. Should be removed in a few months.
    declareOption(ol, "normalize_warning", &PCA::normalize_warning, OptionBase::buildoption, 
                  "(Temp. option). If true, display a warning about the 'normalize' option.");

    declareOption(ol, "impute_missings", &PCA::impute_missings,
                  OptionBase::buildoption,
                  "If true, if a missing value is encountered on an input variable\n"
                  "for a computeOutput, it is replaced by the estimated mu for that\n"
                  "variable before projecting on the principal components\n");
    
    // saved options
    declareOption(ol, "mu", &PCA::mu, OptionBase::learntoption,
                  "The (weighted) mean of the samples");
    declareOption(ol, "eigenvals", &PCA::eigenvals, OptionBase::learntoption,
                  "The ncomponents eigenvalues corresponding to the principal directions kept");
    declareOption(ol, "eigenvecs", &PCA::eigenvecs, OptionBase::learntoption,
                  "A ncomponents x inputsize matrix containing the principal eigenvectors");
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    declareOption( ol, "oldest_observation", &PCA::_oldest_observation,
                   OptionBase::learntoption,
                   "Incremental algo:\n"
                   "The first time values are fed to _incremental_stats, we must remember\n"
                   "the first observation in order not to remove observation that never\n"
                   "contributed to the covariance matrix.\n"
                   "\n"
                   "Initialized to -1;" );
}

///////////
// build //
///////////
void PCA::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void PCA::build_()
{
    if (normalize_warning)
        PLWARNING("In PCA - The default value for option 'normalize' is now 0 instead of 1. Make sure you did not rely on this default value,"
                  "and set the 'normalize_warning' option to 0 to remove this warning");

    if ( algo == "incremental" )
    {    
        _incremental_stats.compute_covariance  = true;
        _incremental_stats.no_removal_warnings = true;
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void PCA::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                  const Vec& target, Vec& costs) const
{
    static Vec reconstructed_input;
    reconstruct(output, reconstructed_input);
    costs.resize(1);
    costs[0] = powdistance(input, reconstructed_input);
}                                

///////////////////
// computeOutput //
///////////////////
void PCA::computeOutput(const Vec& input, Vec& output) const
{
    static Vec x;
    x.resize(input.length());
    x << input;

    // Perform missing-value imputation if requested
    if (impute_missings)
        for (int i=0, n=x.size() ; i<n ; ++i)
            if (is_missing(x[i]))
                x[i] = mu[i];
                
    // Project on eigenvectors
    x -= mu;
    output.resize(ncomponents);

    if(normalize)
    {
        for(int i=0; i<ncomponents; i++)
            output[i] = dot(x,eigenvecs(i)) / sqrt(eigenvals[i]);
    }
    else
    {
        for(int i=0; i<ncomponents; i++)
            output[i] = dot(x,eigenvecs(i));
    }
}    

////////////////////
// setTrainingSet //
////////////////////

void PCA::setTrainingSet( VMat training_set, bool call_forget )
{
    inherited::setTrainingSet( training_set, call_forget );

    // Even if call_forget is false, the classical PCA algorithm must start
    // from scratch if the dataset changed. If call_forget is true, forget
    // was already called by the inherited::setTrainingSet
    if ( !call_forget && algo == "classical" )
        forget();
  
    if ( algo == "incremental" )
        nstages = training_set.length();
}


////////////
// forget //
////////////
void PCA::forget()
{
    stage           = 0;

    if ( algo == "incremental" )
    {
        _incremental_stats.forget();
        _oldest_observation = -1;
    }
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> PCA::getTestCostNames() const
{
    return TVec<string>(1,"squared_reconstruction_error");
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> PCA::getTrainCostNames() const
{
    return TVec<string>();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PCA::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(mu, copies);
    deepCopyField(eigenvals, copies);
    deepCopyField(eigenvecs, copies);
}


////////////////
// outputsize //
////////////////
int PCA::outputsize() const
{
    return ncomponents;
}

///////////
// train //
///////////

void PCA::classical_algo( )
{  
    if ( ncomponents > train_set->inputsize() )
        PLERROR( "In PCA::train - You asked for %d components, but the "
                 "training set inputsize is only %d",
                 ncomponents, train_set->inputsize() );

    ProgressBar* pb = 0;
    if (report_progress)
        pb = new ProgressBar("Training PCA", 2);

    Mat covarmat;
    computeInputMeanAndCovar(train_set, mu, covarmat);
    if (mu.hasMissing() || covarmat.hasMissing())
        PLERROR("PCA::classical_algo: missing values encountered in training set\n");
    if (pb)
        pb->update(1);
  
    eigenVecOfSymmMat(covarmat, ncomponents, eigenvals, eigenvecs);      
    if (pb)
        pb->update(2);

    stage += 1;
    if (pb)
        delete pb;
}

void PCA::incremental_algo()
{
    ProgressBar* pb = 0;
    if (report_progress)
        pb = new ProgressBar("Incremental PCA", 2);

    /*!
      On the first call, there is no need to manage data prior to the window,
      if any.
    */
    int start = stage;
    if ( stage == 0 && _horizon > 0 )
    {
        int window_start = train_set.length() - _horizon;
        start = window_start > 0 ? window_start : 0;
    }

    /*
      The first time values are fed to _incremental_stats, we must remember
      the first observation in order not to remove observation that never
      contributed to the covariance matrix.

      See the following 'if ( old >= oldest_observation )' statement.
    */
    if ( _oldest_observation == -1 )
        _oldest_observation = start;
    assert( _horizon <= 0 || (start-_horizon) <= _oldest_observation ); 
  
    Vec observation;
    for ( int obs=start; obs < train_set.length(); obs++ )
    {
        observation.resize( train_set.width() );

        // Stores the new observation
        observation << train_set( obs );
        if (observation.hasMissing())
            PLERROR("PCA::incremental_algo: missing values encountered in training set\n");
      
        // This adds the contribution of the new observation
        _incremental_stats.update( observation );
      
        if ( _horizon > 0 &&
             (obs - _horizon) == _oldest_observation )
        {
            // Stores the old observation
            observation << train_set( _oldest_observation );
        
            // This removes the contribution of the old observation
            _incremental_stats.remove_observation( observation );
            _oldest_observation++;
        }
    }

    if (pb)
        pb->update(1);
      
    // Recomputes the eigenvals and eigenvecs from the updated
    // incremental statistics
    mu           =  _incremental_stats.getMean();
    Mat covarmat =  _incremental_stats.getCovariance();
    eigenVecOfSymmMat( covarmat, ncomponents, eigenvals, eigenvecs );      

    if (pb)
    {
        pb->update(2);
        delete pb;
    }
  
    // Remember the number of observation
    stage = train_set.length();
}

// Here, I just copied the ... content of the if ( algo == "em" ) { ... }
// that you could find in train() before... Obviously, there is still some
// clean up to do.
void PCA::em_algo()
{
    ProgressBar* pb = 0;

    int n = train_set->length();
    int p = train_set->inputsize();
    int k = ncomponents;
  
    // Fill the matrix C with random data.
    Mat C(k,p);

    fill_random_normal(C);
    // Center the data.
    VMat centered_data = new CenteredVMatrix(new GetInputVMatrix(train_set));
    Vec sample_mean = static_cast<CenteredVMatrix*>((VMatrix*) centered_data)->getMu();
    mu.resize(sample_mean.length());
    mu << sample_mean;
    Mat Y = centered_data.toMat();
    Mat X(n,k);
    Mat tmp_k_k(k,k);
    Mat tmp_k_k_2(k,k);
    Mat tmp_p_k(p,k);
    Mat tmp_k_n(k,n);
    // Iterate through EM.
    if (report_progress)
        pb = new ProgressBar("Training EM PCA", nstages - stage);
    int init_stage = stage;
    while (stage < nstages) {
        // E-step: X <- Y C' (C C')^-1
        productTranspose(tmp_k_k, C, C);
        matInvert(tmp_k_k, tmp_k_k_2);
        transposeProduct(tmp_p_k, C, tmp_k_k_2);
        product(X, Y, tmp_p_k);
        // M-step: C <- (X' X)^-1 X' Y
        transposeProduct(tmp_k_k, X, X);
        matInvert(tmp_k_k, tmp_k_k_2);
        productTranspose(tmp_k_n, tmp_k_k_2, X);
        product(C, tmp_k_n, Y);
        stage++;
        if (report_progress)
            pb->update(stage - init_stage);
    }
    // Compute the orthonormal projection matrix.
    int n_base = GramSchmidtOrthogonalization(C);
    if (n_base != k) {
        PLWARNING("In PCA::train - The rows of C are not linearly independent");
    }
    // Compute the projected data.
    productTranspose(X, Y, C);
    // And do a PCA to get the eigenvectors and eigenvalues.
    PCA true_pca;
    VMat proj_data(X);
    true_pca.ncomponents = k;
    true_pca.normalize = 0;
    true_pca.setTrainingSet(proj_data);
    true_pca.train();
    // Transform back eigenvectors to input space.
    eigenvecs.resize(k, p);
    product(eigenvecs, true_pca.eigenvecs, C);
    eigenvals.resize(k);
    eigenvals << true_pca.eigenvals;

    if (pb)
        delete pb;
}

// Here, I just copied the ... content of the if ( algo == "em" ) { ... }
// that you could find in train() before... Obviously, there is still some
// clean up to do.
void PCA::em_orth_algo()
{
    ProgressBar* pb = 0;
  
    int n = train_set->length();
    int p = train_set->inputsize();
    int k = ncomponents;
    // Fill the matrix C with random data.
    Mat C(k,p);
    fill_random_normal(C);
    // Ensure it is orthonormal.
    GramSchmidtOrthogonalization(C);
    // Center the data.
    VMat centered_data = new CenteredVMatrix(new GetInputVMatrix(train_set));
    Vec sample_mean = static_cast<CenteredVMatrix*>((VMatrix*) centered_data)->getMu();
    mu.resize(sample_mean.length());
    mu << sample_mean;
    Mat Y = centered_data.toMat();
    Mat Y_copy(n,p);
    Mat X(n,k);
    Mat tmp_k_k(k,k);
    Mat tmp_k_k_2(k,k);
    Mat tmp_p_k(p,k);
    Mat tmp_k_n(k,n);
    Mat tmp_n_1(n,1);
    Mat tmp_n_p(n,p);
    Mat X_j, C_j;
    Mat x_j_prime_x_j(1,1);
    // Iterate through EM.
    if (report_progress)
        pb = new ProgressBar("Training EM PCA", nstages - stage);
    int init_stage = stage;
    Y_copy << Y;
    while (stage < nstages) {
        Y << Y_copy;
        for (int j = 0; j < k; j++) {
            C_j = C.subMatRows(j, 1);
            X_j = X.subMatColumns(j,1);
            // E-step: X_j <- Y C_j'
            productTranspose(X_j, Y, C_j);
            // M-step: C_j <- (X_j' X_j)^-1 X_j' Y
            transposeProduct(x_j_prime_x_j, X_j, X_j);
            transposeProduct(C_j, X_j, Y);
            C_j /= x_j_prime_x_j(0,0);
            // Normalize the new direction.
            PLearn::normalize(C_j, 2.0);
            // Subtract the component along this new direction, so as to
            // obtain orthogonal directions.
            productTranspose(tmp_n_1, Y, C_j);
            negateElements(Y);
            productAcc(Y, tmp_n_1, C_j);
            negateElements(Y);
        }
        stage++;
        if (report_progress)
            pb->update(stage - init_stage);
    }
    // Check orthonormality of C.
    for (int i = 0; i < k; i++) {
        for (int j = i; j < k; j++) {
            real dot_i_j = dot(C(i), C(j));
            if (i != j) {
                if (abs(dot_i_j) > 1e-6) {
                    PLWARNING("In PCA::train - It looks like some vectors are not orthogonal");
                }
            } else {
                if (abs(dot_i_j - 1) > 1e-6) {
                    PLWARNING("In PCA::train - It looks like a vector is not normalized");
                }
            }
        }
    }
    // Compute the projected data.
    Y << Y_copy;
    productTranspose(X, Y, C);
    // Compute the empirical variance on each projected axis, in order
    // to obtain the eigenvalues.
    VMat X_vm(X);
    Vec mean_proj, var_proj;
    computeMeanAndVariance(X_vm, mean_proj, var_proj);
    eigenvals.resize(k);
    eigenvals << var_proj;
    // Copy the eigenvectors.
    eigenvecs.resize(k, p);
    eigenvecs << C;

    if (pb)
        delete pb;
}

void PCA::train()
{
    if ( stage < nstages )
    {
        if ( algo == "classical" )
            classical_algo( );

        else if( algo == "incremental" )
            incremental_algo();
    
        else if ( algo == "em" )
            em_algo();

        else if ( algo == "em_orth" )
            em_orth_algo( );

        else
            PLERROR("In PCA::train - Unknown value for 'algo'");    
    }

    else 
        PLWARNING("In PCA::train - The learner has already been train, skipping training");
}


/////////////////
// reconstruct //
/////////////////
void PCA::reconstruct(const Vec& output, Vec& input) const
{
    input.resize(mu.length());
    input << mu;

    int n = output.length();
    if(normalize)
    {
        for(int i=0; i<n; i++)
            multiplyAcc(input, eigenvecs(i), output[i]*sqrt(eigenvals[i]));
    }
    else
    {
        for(int i=0; i<n; i++)
            multiplyAcc(input, eigenvecs(i), output[i]);
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
