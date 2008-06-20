// -*- C ++ -*-

// ManifoldParzen.cc
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

/*! \file ManifoldParzen.cc */


#define PL_LOG_MODULE_NAME "ManifoldParzen"
#include <plearn/io/pl_log.h>

#include "ManifoldParzen.h"
#include <plearn/vmat/VMat_computeNearestNeighbors.h>
#include <plearn/vmat/GetInputVMatrix.h>
#include <plearn_learners/online/GradNNetLayerModule.h>
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ManifoldParzen,
    "Manifold Parzen Windows classifier and distribution",
    "");
  
ManifoldParzen::ManifoldParzen() :
    nneighbors( 1 ),
    ncomponents( 1 ),
    global_lambda0( 0 ),
    learn_mu( false ),
    n_classes( -1 )
{
}

void ManifoldParzen::declareOptions(OptionList& ol)
{
    declareOption(ol, "nneighbors", 
                  &ManifoldParzen::nneighbors,
                  OptionBase::buildoption,
                  "Number of nearest neighbors to use to learn "
                  "the manifold structure..\n");

    declareOption(ol, "ncomponents", 
                  &ManifoldParzen::ncomponents,
                  OptionBase::buildoption,
                  "Dimensionality of the manifold.\n");

    declareOption(ol, "global_lambda0", 
                  &ManifoldParzen::global_lambda0,
                  OptionBase::buildoption,
                  "Additive minimum value for the variance in all directions.\n");

    declareOption(ol, "learn_mu", 
                  &ManifoldParzen::learn_mu,
                  OptionBase::buildoption,
                  "Additive minimum value for the variance in all directions.\n");

    declareOption(ol, "n_classes", 
                  &ManifoldParzen::n_classes,
                  OptionBase::buildoption,
                  "Number of classes. If n_classes = 1, learner will output\n"
                  "log likelihood of a given input. If n_classes > 1,\n"
                  "classification will be performed.\n");

    declareOption(ol, "train_set", 
                  &ManifoldParzen::train_set,
                  OptionBase::learntoption,
                  "Training set.\n"
        );

    declareOption(ol, "eigenvalues", 
                  &ManifoldParzen::eigenvalues,
                  OptionBase::learntoption,
                  ""
        );

    declareOption(ol, "sigma_noises", 
                  &ManifoldParzen::sigma_noises,
                  OptionBase::learntoption,
                  ""
        );

    declareOption(ol, "mus", 
                  &ManifoldParzen::mus,
                  OptionBase::learntoption,
                  ""
        );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ManifoldParzen::build_()
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

    MODULE_LOG << "build_() called" << endl;

    if(inputsize_ > 0 )
    {
        // Builds some variables using the training set
        setTrainingSet(train_set, false);

        if( n_classes <= 0 )
            PLERROR("ManifoldParzen::build_() - \n"
                    "n_classes should be > 0.\n");
        test_votes.resize(n_classes);

        if( nneighbors <= 0 )
            PLERROR("ManifoldParzen::build_() - \n"
                    "nneighbors should be > 0.\n");

        if( weightsize_ > 0 )
            PLERROR("ManifoldParzen::build_() - \n"
                    "usage of weighted samples (weight size > 0) is not\n"
                    "implemented yet.\n");

        if( ncomponents < 1 || ncomponents > inputsize_)
            PLERROR("ManifoldParzen::build_() - \n"
                    "ncomponents should be > 0 and < or = to inputsize.\n");

        if( global_lambda0 < 0)
            PLERROR("ManifoldParzen::build_() - \n"
                    "global_lambda0 should be > or = to 0.\n");

    }
}

// ### Nothing to add here, simply calls build_
void ManifoldParzen::build()
{
    inherited::build();
    build_();
}


void ManifoldParzen::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    // deepCopyField(, copies);

    // Protected options
    deepCopyField(mu, copies);
    deepCopyField(Ut, copies);
    deepCopyField(U, copies);
    deepCopyField(V, copies);
    deepCopyField(diff_neighbor_input, copies);
    deepCopyField(uk, copies);
    deepCopyField(sm_svd, copies);
    deepCopyField(S, copies);
    deepCopyField(diff, copies);
    deepCopyField(eigenvectors, copies);
    deepCopyField(eigenvalues, copies);
    deepCopyField(sigma_noises, copies);
    deepCopyField(mus, copies);
    deepCopyField(class_datasets, copies);
    deepCopyField(nearest_neighbors_indices, copies);
    deepCopyField(test_votes, copies);
}


int ManifoldParzen::outputsize() const
{
    return 1;
}

void ManifoldParzen::forget()
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

    for(int i=0; i < eigenvectors.length(); i++)
      eigenvectors[i].clear();
    eigenvalues.clear();
    sigma_noises.clear();
    mus.clear();
    stage = 0;
}

void ManifoldParzen::train()
{
    MODULE_LOG << "train() called " << endl;

    Vec input( inputsize() );
    Vec nearest_neighbor( inputsize() );
    Mat nearest_neighbors( nneighbors, inputsize() );
    Vec target( targetsize() );
    Vec target2( targetsize() );
    real weight; // unused
    real weight2; // unused

    TVec<string> train_cost_names = getTrainCostNames() ;
    Vec train_costs( train_cost_names.length() );
    train_costs.fill(MISSING_VALUE) ;

    int sample;
    PP<ProgressBar> pb;

    // clear stats of previous epoch
    train_stats->forget();

    if( stage < 1 && nstages > 0 )
      {
	stage = 1;
	MODULE_LOG << "Finding the nearest neighbors" << endl;
	// Find training nearest neighbors
	TVec<int> nearest_neighbors_indices_row;
	nearest_neighbors_indices.resize(train_set->length(), nneighbors);
	if( n_classes > 1 )
	  for(int k=0; k<n_classes; k++)
	    {
	      for(int i=0; i<class_datasets[k]->length(); i++)
		{
		  class_datasets[k]->getExample(i,input,target,weight);
		  nearest_neighbors_indices_row = nearest_neighbors_indices(
									    class_datasets[k]->indices[i]);
		
		  computeNearestNeighbors(
					  new GetInputVMatrix((VMatrix *)class_datasets[k]),input,
					  nearest_neighbors_indices_row,
					  i);
		}
	    }
	else
	  for(int i=0; i<train_set->length(); i++)
	    {
	      train_set->getExample(i,input,target,weight);
	      nearest_neighbors_indices_row = nearest_neighbors_indices(i);
	      computeNearestNeighbors(
				      train_set,input,
				      nearest_neighbors_indices_row,
				      i);
	    }
      
        train_costs.fill(MISSING_VALUE);

        if( report_progress )
	  pb = new ProgressBar( "Training ManifoldParzen",
				train_set->length() );

	eigenvectors.resize( train_set->length() );
	eigenvalues.resize( train_set->length(), ncomponents );
	sigma_noises.resize( train_set->length(), 1 );
	mus.resize( train_set->length(), inputsize() );
	mus.clear();
        for( sample = 0; sample < train_set->length() ; sample++ )
	  { 
            train_set->getExample( sample, input, target, weight );
	    
            // Find nearest neighbors
            if( n_classes > 1 )
	      for( int k=0; k<nneighbors; k++ )
                {
		  class_datasets[(int)round(target[0])]->getExample(
								    nearest_neighbors_indices(sample,k),
								    nearest_neighbor, target2, weight2);
		  
		  if(round(target[0]) != round(target2[0]))
		    PLERROR("ManifoldParzen::train(): similar"
			    " example is not from same class!");
		  nearest_neighbors(k) << nearest_neighbor;
                }
            else
	      for( int k=0; k<nneighbors; k++ )
                {
		  train_set->getExample(
					nearest_neighbors_indices(sample,k),
					nearest_neighbor, target2, weight2);
		  nearest_neighbors(k) << nearest_neighbor;
                }
	    
	    if( learn_mu )
	      {
		mu.resize(inputsize());
		columnMean( nearest_neighbors, mu );
		mus(sample) << mu;
		mus(sample) -= input;
	      }
	    substractFromRows(nearest_neighbors, input, false); // Boolean is somehow unused???
	    lapackSVD(nearest_neighbors, Ut, S, V,'A',1.5);
	    eigenvectors[sample].resize(ncomponents,inputsize());
	    for (int k=0;k<ncomponents;k++)
	      {
		eigenvalues(sample,k) = mypow(S[k],2);
		eigenvectors[sample](k) << Ut(k);
	      }
	    sigma_noises[sample] = 0; // HUGO: Seems stupid for now, but I keep 
	                              //       this variable in case I want to use
	                              //       the last eigen value or something...

            if( pb )
	      pb->update( sample + 1 );
	  }
      }
    
    train_stats->finalize();
    MODULE_LOG << "  train costs = " << train_stats->getMean() << endl;
}

void ManifoldParzen::computeOutput(const Vec& input, Vec& output) const
{

    test_votes.resize(n_classes);
    test_votes.clear();

    // Variables for probability computations
    real log_p_x_g_y = 0;
    real mahal = 0;
    real norm_term = 0;
    real n = inputsize();
    real dotp = 0;
    real coef = 0;
    real sigma_noise = 0;
    
    Vec input_j(inputsize());
    Vec target(targetsize());
    real weight;

    int input_j_index;
    for( int i=0; i<n_classes; i++ )
      {
	for( int j=0; 
	     j<(n_classes > 1 ? 
		class_datasets[i]->length() 
		: train_set->length()); 
	     j++ )
	  {
	    if( n_classes > 1 )
	      {
		class_datasets[i]->getExample(j,input_j,target,weight);
		input_j_index = class_datasets[i]->indices[j];
	      }
	    else
	      {
		train_set->getExample(j,input_j,target,weight);
		input_j_index = j;
	      }
	    
	    U << eigenvectors[input_j_index];
	    sm_svd << eigenvalues(input_j_index);
	    sigma_noise = sigma_noises[input_j_index] + global_lambda0;
	    mu << mus(input_j_index);
	    
	    substract(input,input_j,diff_neighbor_input); 
	    substract(diff_neighbor_input,mu,diff); 
            
	    mahal = -0.5*pownorm(diff)/sigma_noise;      
	    norm_term = - n/2.0 * Log2Pi - 0.5*(n-ncomponents)*
	      pl_log(sigma_noise);
	    
	    for(int k=0; k<ncomponents; k++)
	      { 
		uk = U(k);
		dotp = dot(diff,uk);
		coef = (1.0/(sm_svd[k]+global_lambda0) - 1.0/sigma_noise);
		mahal -= dotp*dotp*0.5*coef;
		norm_term -= 0.5*pl_log(sm_svd[k]+global_lambda0);
	      }
	    
	    if( j==0 )
	      log_p_x_g_y = norm_term + mahal;
	    else
	      log_p_x_g_y = logadd(norm_term + mahal, log_p_x_g_y);
	  }
        
	test_votes[i] = log_p_x_g_y;
      }

    if( n_classes > 1 )
        output[0] = argmax(test_votes);
    else
        output[0] = test_votes[0]-pl_log(train_set->length());
}

void ManifoldParzen::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{

    //Assumes that computeOutput has been called

    costs.resize( getTestCostNames().length() );
    costs.fill( MISSING_VALUE );

    if( n_classes > 1 )
      {
	int target_class = ((int)round(target[0]));
	if( ((int)round(output[0])) == target_class )
	  costs[0] = 0;
	else
	  costs[0] = 1;
	costs[1] = - test_votes[target_class]
	  +pl_log(class_datasets[target_class]->length()); // Must take into account the 1/n normalization
      }
    else
      {
	costs[1] = - output[0]; // 1/n normalization already accounted for
      }
}

TVec<string> ManifoldParzen::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    TVec<string> cost_names(0);

    cost_names.append( "class_error" );
    cost_names.append( "NLL" );

    return cost_names;
}

TVec<string> ManifoldParzen::getTrainCostNames() const
{
    TVec<string> cost_names(0);
    return cost_names ;    
}

void ManifoldParzen::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set,call_forget);
    
    // Separate classes
    if( n_classes > 1 )
    {
        class_datasets.resize(n_classes);
        for(int k=0; k<n_classes; k++)
        {
            class_datasets[k] = new ClassSubsetVMatrix();
            class_datasets[k]->classes.resize(1);
            class_datasets[k]->classes[0] = k;
            class_datasets[k]->source = training_set;
            class_datasets[k]->build();
        }
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
