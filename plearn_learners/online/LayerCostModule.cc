// -*- C++ -*-

// LayerCostModule.cc
//
// Copyright (C) 2007 Jerome Louradour
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

// Author: Jerome Louradour

/*! \file LayerCostModule.cc */



#include "LayerCostModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LayerCostModule,
    "Computes a cost function on Layer, given:    \n",
    "* Expectations for a binomial RBM upper layer\n"
    "* sigmoid(activation) for a Neural Network   \n"
    "and Back-propagates the gradient.            \n"
    "\n"
    "This function can be:                        \n"
    "* The average Cross-Entropy                  \n"
    "* The average Kullback-Leibler Divergence    \n"
    "* Pascal's function...                       \n");

LayerCostModule::LayerCostModule():
    histo_size(10),
    alpha(0.0),
    momentum(0.0)
{
    output_size = 1;
/*
    ntest = 0;
*/
}

void LayerCostModule::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "input_size", &LayerCostModule::input_size,
                     OptionBase::nosave,
        "Size of the layer.");

    declareOption(ol, "cost_function", &LayerCostModule::cost_function,
                  OptionBase::buildoption,
        "The cost function applied to the layer:\n"
        "- \"stochastic_cross_entropy\" [default]: average cross-entropy between pairs of binomial units\n"
        "- \"stochastic_kl_div\": average KL divergence between pairs of binomial units\n"
        "- \"kl_div\": KL divergence between distrubution of expectations (sampled with x)\n"
        "- \"kl_div_2\": good version of kl_div\n"
        "- \"kl_div_simple\": simple version of kl_div where we count at least one sample per histogram's bin\n");

    declareOption(ol, "histo_size", &LayerCostModule::histo_size,
                  OptionBase::buildoption,
        "For \"kl_div*\" cost functions,\n"
        "number of bins for the histograms (to estimate distributions of probabilities for expectations).\n"
        "The higher is histo_size, the more precise is the estimation.\n");

    declareOption(ol, "alpha", &LayerCostModule::alpha,
                  OptionBase::buildoption,
        "(>=0) For \"pascal\" cost function,\n"
        "number of bins for the histograms (to estimate distributions of probabilities for expectations).\n"
        "The higher is histo_size, the more precise is the estimation.\n");

    declareOption(ol, "momentum", &LayerCostModule::momentum,
                  OptionBase::buildoption,
        "(in [0,1[) For \"pascal\" cost function, momentum for the moving means\n");

}

void LayerCostModule::build_()
{
    PLASSERT( input_size > 1 );
    PLASSERT( histo_size > 1 );
    PLASSERT( momentum >= 0.0);
    PLASSERT( momentum < 1);
    
    string im = lowerstring( cost_function );
    // choose HERE the *default* cost function
    if( im == "" )
        cost_function = "pascal";
    else
        cost_function = im;

     // list HERE all *stochastic* cost functions
    if( ( cost_function == "stochastic_cross_entropy")
     || ( cost_function == "stochastic_kl_div") )
        is_cost_function_stochastic = true;
	
    // list HERE all *non stochastic* cost functions
    // and the specific initialization
    else if( ( cost_function == "kl_div")
          || ( cost_function == "kl_div_simple")
	  || ( cost_function == "kl_div_2") )
    {
        is_cost_function_stochastic = false;
        expectations_histo.resize(input_size,histo_size);
	LINHISTO_STEP = 1.0/(real)histo_size;
        LOGHISTO_BASE = 10.0;
        LOGHISTO_MIN = (real)pow(LOGHISTO_BASE,-(real)histo_size);
    }
    else if ( ( cost_function == "pascal") )
    {
        is_cost_function_stochastic = false;
	expectations_expectation.resize(input_size);
	expectations_cross_quadratic_mean.resize(input_size,input_size);
/*
        expectations_expectation_testMemory.resize(input_size);
        expectations_cross_quadratic_mean_testMemory.resize(input_size,input_size);
*/
	if( momentum > 0.0)
	{
            expectations_expectation_trainMemory.resize(input_size);
            expectations_cross_quadratic_mean_trainMemory.resize(input_size,input_size);
	}
    }
    else
        PLERROR("LayerCostModule::build_() does not recognize cost function %s",
                 cost_function.c_str());

    // The port story...
    ports.resize(0);
    portname_to_index.clear();
    addPortName("expectations");
    addPortName("cost");

    port_sizes.resize(nPorts(), 2);
    port_sizes.fill(-1);
    port_sizes(getPortIndex("expectations"), 1) = input_size;
    port_sizes(getPortIndex("cost"), 1) = 1;
    
}


// ### Nothing to add here, simply calls build_
void LayerCostModule::build()
{
    inherited::build();
    build_();
}


void LayerCostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(expectations_histo, copies);
    deepCopyField(expectations_expectation, copies);
    deepCopyField(expectations_cross_quadratic_mean, copies);
    deepCopyField(expectations_expectation_trainMemory, copies);
    deepCopyField(expectations_cross_quadratic_mean_trainMemory, copies);
/*
    deepCopyField(expectations_expectation_testMemory, copies);
    deepCopyField(expectations_cross_quadratic_mean_testMemory, copies);
*/
    deepCopyField(ports, copies);
}




///////////
// fprop //
///////////


void LayerCostModule::fprop(const TVec<Mat*>& ports_value)
{

    Mat* expectations = ports_value[getPortIndex("expectations")];
    Mat* cost = ports_value[getPortIndex("cost")];

    PLASSERT( ports_value.length() == nPorts() );

    if ( cost && cost->isEmpty() )
    {
        PLASSERT( expectations && !expectations->isEmpty() );
	cout << "1 regular fprop!!!" << endl;
        fprop(*expectations, *cost);
    }
}

void LayerCostModule::fprop(const Mat& expectations, Mat& costs)
{
    int batch_size = expectations.length();
    costs.resize( batch_size, output_size );
    
    if( !is_cost_function_stochastic )
    {
        costs.clear();

        if( cost_function == "kl_div" )
        {
        //! ************************************************************
        //! (non stochastic) SYMETRIC *** K-L DIVERGENCE ***
        //! between probabilities of expectations vectors for all units
        //! ************************************************************
        //! 
        //!      cost = - \sum_{i} \sum_{j#i} Div_{KL}[ Px(q{i}) | Px(q{j}) ]
        //!
        //!           = - \sum_{i} \sum_{j#i} \sum_Q (Nx_j(Q) - Nx_j(Q)) log( Nx_i(Q) / Nx_j(Q) )
        //!
        //! where  q{i} = P(h{i}=1|x): expectation of the i^th units of the layer
        //!        Px(.): empirical probability (given data x, we sample the q's)
        //!        Q: interval in [0,1] = one bin of the histogram of the expectations q's
        //!        Nx_i(Q): proportion of q{i} that belong to Q, with input data x
        //!
        //! Note: one q{i} *entirely* determines one binomial densities of probability
        //!       ( Bijection {binomial Proba functions} <-> |R )
        //!
        //! ************************************************************

            // Filling the histogram (i.e. emperical distribution)
            // of the expectations
	    computeHisto(expectations);
	    
            // Computing the KL divergence
            for (int i = 0; i < input_size; i++)
                for (int j = 0; j < i; j++)
		{
		    // These variables are used in case one bin of 
		    // the histogram is empty for one unit
		    // and not for another one ( (Nj-Ni).log(Ni/Nj) = nan ).
		    // In such case, we ''differ'' the count for the next bin and so on.
                    real differ_count_i = 0.0;
                    real differ_count_j = 0.0;
		    for (int k = 0; k < histo_size; k++)
		    {
                        real Ni_k = expectations_histo(i,k) + differ_count_i;
			real Nj_k = expectations_histo(j,k) + differ_count_j;
			if( fast_exact_is_equal(Ni_k, 0.0) )
			{
                         // differ_count_j += expectations_histo(j,k);
                            differ_count_j = Nj_k;
			    continue;
			}
                        else if( fast_exact_is_equal(Nj_k, 0.0) )
			{
                            differ_count_i = Ni_k;
			    continue;
			}
                        else
			{
			    costs(0,0) += KLdivTerm(Ni_k,Nj_k);
                            differ_count_i = 0.0;
			    differ_count_j = 0.0;
			}
                    }
		    if( differ_count_i > 0.0 )
		        "cas ou on regroupe avec le dernier";
		    else if ( differ_count_j > 0.0 )
		        "cas ou on regroupe avec le dernier";		    
                }
            // Normalization w.r.t. number of units
            costs(0,0) /= ((real)input_size *(real)(input_size-1)); // / (real)batch_size;
        }
        else if( cost_function == "kl_div_2" )
        {
        //! ************************************************************
        //! (non stochastic) SYMETRIC *** K-L DIVERGENCE ***
        //! between probabilities of expectations vectors for all units
        //! ************************************************************
        //! 
        //!      cost = - \sum_{i} \sum_{j#i} Div_{KL}[ Px(q{i}) | Px(q{j}) ]
        //!
        //!           = - \sum_{i} \sum_{j#i} \sum_Q (Nx_j(Q) - Nx_j(Q)) log( Nx_i(Q) / Nx_j(Q) )
        //!
        //! where  q{i} = P(h{i}=1|x): expectation of the i^th units of the layer
        //!        Px(.): empirical probability (given data x, we sample the q's)
        //!        Q: interval in [0,1] = one bin of the histogram of the expectations q's
        //!        Nx_i(Q): proportion of q{i} that belong to Q, with input data x
        //!
        //! Note: one q{i} *entirely* determines one binomial densities of probability
        //!       ( Bijection {binomial Proba functions} <-> |R )
        //!
        //! ************************************************************

            // Filling the histogram (i.e. emperical distribution)
            // of the expectations
	    computeHisto(expectations);
	    
            // Computing the KL divergence
            for (int i = 0; i < input_size; i++)
                for (int j = 0; j < i; j++)
		{
		    // These variables are used in case one bin of 
		    // the histogram is empty for one unit
		    // and not for another one ( (Nj-Ni).log(Ni/Nj) = nan ).
		    // In such case, we ''differ'' the count for the next bin and so on.
                    real differ_count_i = 0.0;
                    real differ_count_j = 0.0;
		    int n_differ = 0;
		    for (int k = 0; k < histo_size; k++)
		    {
                        real Ni_k = expectations_histo(i,k) + differ_count_i;
			real Nj_k = expectations_histo(j,k) + differ_count_j;
			if( fast_exact_is_equal(Ni_k, 0.0) )
			{
                         // differ_count_j += expectations_histo(j,k);
                            differ_count_j = Nj_k;
			    n_differ += 1;
			}
                        else if( fast_exact_is_equal(Nj_k, 0.0) )
			{
                            differ_count_i = Ni_k;
			    n_differ += 1;
			}
                        else
			{
			    costs(0,0) += KLdivTerm(Ni_k,Nj_k) *(real)(1+n_differ)/(real)histo_size;
                            differ_count_i = 0.0;
			    differ_count_j = 0.0;
			}
                    }
		    if( differ_count_i > 0.0 )
		        "cas ou on regroupe avec le dernier";
		    else if ( differ_count_j > 0.0 )
		        "cas ou on regroupe avec le dernier";		    
                }
            // Normalization w.r.t. number of units
            costs(0,0) /= ((real)input_size *(real)(input_size-1)); // / (real)batch_size;
        }
        else if( cost_function == "kl_div_simple" )
        {
            // Filling the histogram (i.e. emperical distribution)
            // of the expectations
	    computeSafeHisto(expectations);
	    
            // Computing the KL divergence
            for (int i = 0; i < input_size; i++)
                for (int j = 0; j < i; j++)
		    for (int k = 0; k < histo_size; k++)
		    {
                        real Ni_k = expectations_histo(i,k);
			real Nj_k = expectations_histo(j,k);
			costs(0,0) += KLdivTerm(Ni_k,Nj_k);
                    }
            // Normalization w.r.t. number of units
            costs(0,0) /= ((real)input_size *(real)(input_size-1)); // / (real)batch_size;
        }
        else if( cost_function == "pascal" )
        {
        //! ************************************************************
        //! a god-given similarity measure
        //! between expectations vectors for all units
        //! ************************************************************
        //! 
        //!      cost = \sum_{i} \sum_{j#i} exp( Ex[q{i}.q{j}] ) - alpha. \sum_{i} exp( Ex[q{i}] )
        //!
        //! where  q{i} = P(h{i}=1|x): expectation of the i^th units of the layer
        //!        Ex(.): empirical esperance (given data x, we sample the q's)
        //!
        //! ************************************************************

            // Computing statistics on expectations
	    computePascalStatistics(expectations, false);
	    
	    cout << "1 fprop" << endl;
	    	    
            // Computing the cost
            for (int i = 0; i < input_size; i++)
	    {
	        if (alpha > 0.0 )
		    costs(0,0) -= alpha*exp(expectations_expectation[i]);
                for (int j = 0; j < i; j++)
                    costs(0,0) += exp(expectations_cross_quadratic_mean(i,j)) / (real)(input_size-1);
            }

            // Normalization w.r.t. number of units
            costs(0,0) /= (real)input_size;
        }
	
        return; // Do not fprop with the conventional stochastic fprop...
    }
    
    for (int isample = 0; isample < batch_size; isample++)
        fprop(expectations(isample), costs(isample,0));
}

void LayerCostModule::fprop(const Vec& expectation, real& cost) const
{
    PLASSERT( expectation.size() == input_size );
    PLASSERT( is_cost_function_stochastic );

    cost = 0.0;
    real  qi, qj, comp_qi, comp_qj; // The expectations qi=p(h_i=1)
                                      //     and some basic operations on it (e.g.: 1-qi, qi/(1-qi)) 
				      
    if( cost_function == "stochastic_cross_entropy" )
    {
    //! ************************************************************
    //! average *** CROSS ENTROPY ***
    //! between pairs of units (given expectations = sigmoid(act) )
    //! ************************************************************
    //!
    //!      cost = - \sum_{i} \sum_{j#i} CrossEntropy[( P(h_{i}|x) | P(h_{j}|x) )]
    //!
    //!           = - \sum_{i} \sum_{j#i} [ q{i}.log(q{j}) + (1-q{i}).log(1-q{j}) ]
    //!
    //! where |  h_{i}: i^th units of the layer
    //!       \  P(.|x): output for input data x
    //!        \ q{i}=P(h{i}=1|v): expectation of the i^th units of the layer
    //!
    //! ************************************************************

        for( int i = 0; i < input_size; i++ )
        {
           qi = expectation[i];
           comp_qi = 1.0 - qi;
           for( int j = 0; j < i; j++ )
           {
               qj = expectation[j];
               comp_qj = 1.0 - qj;
	       
               // H(pi||pj) = H(pi) + D_{KL}(pi||pj)
               cost += qi*safeflog(qj) + comp_qi*safeflog(comp_qj);
	       
               // The symetric part (loop  j=i+1...size)
               cost += qj*safeflog(qi) + comp_qj*safeflog(comp_qi);
           }
        }
        // Normalization w.r.t. number of units
        cost /= ((real)input_size *(real)(input_size-1));
    }
    
    else if( cost_function == "stochastic_kl_div" )
    {
    //! ************************************************************
    //! average SYMETRIC *** K-L DIVERGENCE ***
    //! between pairs of units (given expectations = sigmoid(act) )
    //! ************************************************************
    //!
    //!      cost = - \sum_{i} \sum_{j#i} Div_{KL} [( P(h_{i}|v) | P(h_{j}|v) )]
    //!
    //!           = - \sum_{i} \sum_{j#i} [ ( q{j} - q{i} ) log( q{i}/(1-q{i})*(1-q{j})/q{j} ) ]
    //!
    //! where |  h_{i}: i^th units of the layer
    //!       \  P(.|v):  output for input data x
    //!        \ q{i}=P(h{i}=1|v): expectation of the i^th units of the layer
    //!
    //! ************************************************************

        for( int i = 0; i < input_size; i++ )
        {
           qi = expectation[i];
           if(fast_exact_is_equal(qi, 1.0))
               comp_qi = REAL_MAX;
           else
               comp_qi = qi/(1.0 - qi);
       
           for( int j = 0; j < i; j++ )
           {
               qj = expectation[j];
               if(fast_exact_is_equal(qj, 1.0))
                   comp_qj = REAL_MAX;
               else
                   comp_qj = qj/(1.0 - qj);
	       
               //     - D_{KL}(pi||pj) - D_{KL}(pj||pi)
               cost += (qj-qi)*safeflog(comp_qi/comp_qj);
           }
        }
        // Normalization w.r.t. number of units
        cost /= ((real)input_size *(real)(input_size-1));   
    }

    else
        PLERROR("LayerCostModule::fprop() not implemented for cost function %s\n"
	        "- It may be a printing error\n"
		"- You can try to call LayerCostModule::fprop(const Mat& expectations, Mat& costs)\n"
		"- Or else write the code corresponding to your cost function",
                 cost_function.c_str());
}




////////////////////
// bpropAccUpdate //
////////////////////


void LayerCostModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                   const TVec<Mat*>& ports_gradient)
{
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( ports_gradient.length() == nPorts() );

    const Mat* expectations = ports_value[getPortIndex("expectations")];
    Mat* expectations_grad = ports_gradient[getPortIndex("expectations")];
    Mat* cost = ports_value[getPortIndex("cost")];
    Mat* cost_grad = ports_gradient[getPortIndex("cost")];

    if( expectations_grad && expectations_grad->isEmpty()
        && cost_grad && !cost_grad->isEmpty() )
    {
        int batch_size = expectations->length();

        PLASSERT( expectations && !expectations->isEmpty());
        PLASSERT( expectations->length() == batch_size );
        PLASSERT( cost_grad->length() == batch_size );

        expectations_grad->resize(batch_size, input_size);
	expectations_grad->clear();

        real qi, qj, comp_qi, comp_qj;
        Vec comp_q(input_size), log_term(input_size);

        if( cost_function == "stochastic_cross_entropy" )
        {
            for (int isample = 0; isample < batch_size; isample++)
            {
        	for (int i = 0 ; i < input_size ; i++ )
                {
                    qi = (*expectations)(isample,i);
            	    comp_qi = 1.0 - qi;
                    comp_q[i] = comp_qi;
                    log_term[i] = safeflog(qi) - safeflog(comp_qi);
                }
                for (int i = 0; i < input_size; i++ )
                {
                    qi = (*expectations)(isample,i);
                    comp_qi = comp_q[i];
                    (*expectations_grad)(isample,i) = 0.0;
                    for (int j = 0; j < i; j++ )
                    {
                        qj = (*expectations)(isample,j);
                        comp_qj=comp_q[j];

                        // log(pj) - log(1-pj) + pj/pi - (1-pj)/(1-pi)
                        (*expectations_grad)(isample,i) += log_term[j] + qj/qi - comp_qi/comp_qj;

                        // The symetric part (loop  j=i+1...input_size)
                        (*expectations_grad)(isample,j) += log_term[i] + qi/qj - comp_qj/comp_qi;
                    }
                }
                // Normalization
                real norm_factor = 1.0 / ((real)input_size *(real)(input_size-1));
                for (int i = 0; i < input_size; i++ )
                {
                    (*expectations_grad)(isample, i) *= (*cost_grad)(isample,0) * norm_factor;
                }
            }
        }

        else if( cost_function == "stochastic_kl_div" )
        {
            for (int isample = 0; isample < batch_size; isample++)
            {
        	for (int i = 0; i < input_size; i++ )
                {
                    qi = (*expectations)(isample,i);
            	    comp_qi = 1.0 - qi;
                    if(fast_exact_is_equal(qi, 1.0) || fast_exact_is_equal(qi, 0.0))
                        comp_q[i] = REAL_MAX;
                    else
                        comp_q[i] = 1.0/(qi*comp_qi);
                    log_term[i] = safeflog(qi) - safeflog(comp_qi);
                }
                for (int i = 0; i < input_size; i++ )
                {
                    qi = (*expectations)(isample,i);
                    comp_qi = comp_q[i];

                    (*expectations_grad)(isample,i) = 0.0;
                    for (int j = 0; j < i ; j++ )
                    {
                        qj = (*expectations)(isample,j);
                        comp_qj=comp_q[j];

                        //   [qj - qi]/[qi (1-qi)] - log[ qi/(1-qi) * (1-qj)/qj]
                        (*expectations_grad)(isample,i) += (qj - qi)*comp_qi - log_term[i] + log_term[j];

                        // The symetric part (loop  j=i+1...input_size)
                        (*expectations_grad)(isample,j) += (qi - qj)*comp_qj - log_term[j] + log_term[i];
                    }
                }
                // Normalization
                real norm_factor = 1.0 / ((real)input_size *(real)(input_size-1));
                for (int i = 0; i < input_size; i++ )
                {
                    (*expectations_grad)(isample, i) *= (*cost_grad)(isample,0) * norm_factor;
                }
            }
        }


        else if( cost_function == "kl_div" )
        {
	    computeHisto(*expectations);
	    real one_count = 1. / (real)batch_size;
	    
            for (int isample = 0; isample < batch_size; isample++)
            {

                // Computing the difference of KL divergence
                // for d_q
                for (int i = 0; i < input_size; i++)
		{
                    (*expectations_grad)(isample, i) = 0.0;
		    
		    qi = (*expectations)(isample,i);
		    int index_i = histo_index(qi);
		    if( ( index_i == histo_size-1 ) ) // we do not care about this...
		        continue;
		    real over_dqi=1.0/dq(qi);
		    int shift_i;
		    if( over_dqi > 0.0)
		        shift_i = 1;
		    else
		        shift_i = -1;
		    // qi + dq(qi) ==> | expectations_histo(i,index_i)   - one_count
		    //                 \ expectations_histo(i,index_i+shift_i) + one_count
		    
		    for (int j = 0; j < i; j++)
		    {
			(*expectations_grad)(isample, i) += delta_KLdivTerm(i, j, index_i, over_dqi, one_count);
			
                        qj = (*expectations)(isample,j);
			int index_j = histo_index(qj);
		        if( ( index_j == histo_size-1 ) || ( index_j == 0 ) )
		            continue;
			real over_dqj=1.0/dq(qj);
 		        int shift_j;
		        if( over_dqj > 0.0)
		            shift_j = 1;
		        else
		            shift_j = -1;
            	        // qj + dq(qj) ==> | expectations_histo(j,index_j)   - one_count
  		        //                 \ expectations_histo(j,index_j+shift_j) + one_count
			
			(*expectations_grad)(isample, j) += delta_KLdivTerm(j, i, index_j, over_dqj, one_count);
                    }
		}

                // Normalization
                real norm_factor = 1.0 / ((real)input_size *(real)(input_size-1));
                for (int i = 0; i < input_size; i++ )
                {
                    (*expectations_grad)(isample, i) *= (*cost_grad)(isample,0) * norm_factor;
                }
            }
        }

        else if( cost_function == "kl_div_2" )
        {
	    computeHisto(*expectations);
	    real one_count = 1. / (real)batch_size;
	    
            for (int isample = 0; isample < batch_size; isample++)
            {

                // Computing the difference of KL divergence
                // for d_q
                for (int i = 0; i < input_size; i++)
		{
                    (*expectations_grad)(isample, i) = 0.0;
		    
		    qi = (*expectations)(isample,i);
		    int index_i = histo_index(qi);
		    if( ( index_i == histo_size-1 ) ) // we do not care about this...
		        continue;
		    real over_dqi=1.0/dq(qi);
		    int shift_i;
		    if( over_dqi > 0.0)
		        shift_i = 1;
		    else
		        shift_i = -1;
		    // qi + dq(qi) ==> | expectations_histo(i,index_i)   - one_count
		    //                 \ expectations_histo(i,index_i+shift_i) + one_count
		    
		    for (int j = 0; j < i; j++)
		    {
			(*expectations_grad)(isample, i) += delta_KLdivTerm_2(i, j, index_i, over_dqi, one_count);
			
                        qj = (*expectations)(isample,j);
			int index_j = histo_index(qj);
		        if( ( index_j == histo_size-1 ) || ( index_j == 0 ) )
		            continue;
			real over_dqj=1.0/dq(qj);
 		        int shift_j;
		        if( over_dqj > 0.0)
		            shift_j = 1;
		        else
		            shift_j = -1;
            	        // qj + dq(qj) ==> | expectations_histo(j,index_j)   - one_count
  		        //                 \ expectations_histo(j,index_j+shift_j) + one_count
			
			(*expectations_grad)(isample, j) += delta_KLdivTerm_2(j, i, index_j, over_dqj, one_count);
                    }
		}

                // Normalization
                real norm_factor = 1.0 / ((real)input_size *(real)(input_size-1));
                for (int i = 0; i < input_size; i++ )
                {
                    (*expectations_grad)(isample, i) *= (*cost_grad)(isample,0) * norm_factor;
                }
            }
        }

        else if( cost_function == "kl_div_simple" )
        {
	    computeSafeHisto(*expectations);
	    real one_count = 1. / (real)(batch_size+histo_size);
	    
            for (int isample = 0; isample < batch_size; isample++)
            {

                // Computing the difference of KL divergence
                // for d_q
                for (int i = 0; i < input_size; i++)
		{
                    (*expectations_grad)(isample, i) = 0.0;
		    
		    qi = (*expectations)(isample,i);
		    int index_i = histo_index(qi);
		    if( ( index_i == histo_size-1 ) ) // we do not care about this...
		        continue;
		    real over_dqi=1.0/dq(qi);
		    int shift_i;
		    if( over_dqi > 0.0)
		        shift_i = 1;
		    else
		        shift_i = -1;
		    // qi + dq(qi) ==> | expectations_histo(i,index_i)   - one_count
		    //                 \ expectations_histo(i,index_i+shift_i) + one_count
		    
		    for (int j = 0; j < i; j++)
		    {
			(*expectations_grad)(isample, i) += delta_SafeKLdivTerm(i, j, index_i, over_dqi, one_count);
			
                        qj = (*expectations)(isample,j);
			int index_j = histo_index(qj);
		        if( ( index_j == histo_size-1 ) || ( index_j == 0 ) )
		            continue;
			real over_dqj=1.0/dq(qj);
 		        int shift_j;
		        if( over_dqj > 0.0)
		            shift_j = 1;
		        else
		            shift_j = -1;
            	        // qj + dq(qj) ==> | expectations_histo(j,index_j)   - one_count
  		        //                 \ expectations_histo(j,index_j+shift_j) + one_count
			
			(*expectations_grad)(isample, j) += delta_SafeKLdivTerm(j, i, index_j, over_dqj, one_count);
                    }
		}

                // Normalization
                real norm_factor = 1.0 / ((real)input_size *(real)(input_size-1));
                for (int i = 0; i < input_size; i++ )
                {
                    (*expectations_grad)(isample, i) *= (*cost_grad)(isample,0) * norm_factor;
                }
            }
        }

        else if( cost_function == "pascal" )
        {
	    computePascalStatistics(*expectations, true);
	    
	    cout << "1 BPropAccUpdate" << endl;
	    
	    real one_count = 1. / (real)batch_size;
	    if( momentum > 0.0 )
	        for (int isample = 0; isample < batch_size; isample++)
		{
                    for (int i = 0; i < input_size; i++)
                    {
                        qi = (*expectations)(isample, i);
			if (alpha > 0.0 )
			    (*expectations_grad)(isample, i) -= alpha*exp(expectations_expectation[i]) *(1.0-momentum)*one_count;
                        for (int j = 0; j < i; j++)
                        {
			    qj = (*expectations)(isample,j);
                            (*expectations_grad)(isample, i) += exp(expectations_cross_quadratic_mean(i,j)) *qj*(1.0-momentum)*one_count / (real)(input_size-1);
                            (*expectations_grad)(isample, j) += exp(expectations_cross_quadratic_mean(i,j)) *qi*(1.0-momentum)*one_count / (real)(input_size-1);
                        }
                    }
                    for (int i = 0; i < input_size; i++)
                    {
	                (*expectations_grad)(isample, i) /= (real)input_size;
	            }
		}
	    else
	        for (int isample = 0; isample < batch_size; isample++)
		{
                    for (int i = 0; i < input_size; i++)
                    {
                        qi = (*expectations)(isample, i);
			if (alpha > 0.0 )
			    (*expectations_grad)(isample, i) -= alpha*exp(expectations_expectation[i]) *one_count;
                        for (int j = 0; j < i; j++)
                        {
			    qj = (*expectations)(isample,j);
                            (*expectations_grad)(isample, i) += exp(expectations_cross_quadratic_mean(i,j)) *qj*one_count / (real)(input_size-1);
                            (*expectations_grad)(isample, j) += exp(expectations_cross_quadratic_mean(i,j)) *qi*one_count / (real)(input_size-1);
                        }
                    }
                    for (int i = 0; i < input_size; i++)
                    {
	                (*expectations_grad)(isample, i) /= (real)input_size;
	            }
		}
        }
	
        else
            PLERROR("LayerCostModule::bpropAccUpdate() not implemented for cost function %s",
                     cost_function.c_str());

/*
        ntest = 0;
*/

        checkProp(ports_gradient);
    }
    else if( !expectations_grad && !cost_grad )
        return;
    else
        PLERROR("In LayerCostModule::bpropAccUpdate - Port configuration not implemented ");

}


////////////////////////////////////////////////////
// Auxiliary Functions for Pascal's cost function //
////////////////////////////////////////////////////
void LayerCostModule::computePascalStatistics(const Mat& expectations, bool duringTraining)
{
    int batch_size = expectations.length();
    real one_count = 1. / (real)batch_size;
    Vec expectation;
    
    expectations_expectation.clear(); 
    expectations_cross_quadratic_mean.clear(); 

    for (int isample = 0; isample < batch_size; isample++)
    {
        expectation = expectations(isample);
        for (int i = 0; i < input_size; i++)
	{
	    expectations_expectation[i] += expectation[i];
	    for (int j = 0; j < i; j++)
                 expectations_cross_quadratic_mean(i,j) += expectation[i] * expectation[j];
        }
    }
    expectations_cross_quadratic_mean *= one_count;
    
    for (int i = 0; i < input_size; i++)
    {
        expectations_expectation[i] *= one_count;
        for (int j = 0; j < i; j++)
        {
             expectations_cross_quadratic_mean(i,j) *= one_count;
//    	     expectations_cross_quadratic_mean(j,i) = expectations_cross_quadratic_mean(i,j);
        }
    }
    if( ( momentum > 0.0 ) && duringTraining )
    {
        for (int i = 0; i < input_size; i++)
        {
	    if(i == 0)
	       cout << ".Check momentum....: expectations_expectation_trainMemory[0] = " << expectations_expectation_trainMemory[0] << endl;

            expectations_expectation[i] = momentum*expectations_expectation_trainMemory[i]
	                                 +(1.0-momentum)*expectations_expectation[i];
            expectations_expectation_trainMemory[i] = expectations_expectation[i];
            for (int j = 0; j < i; j++)
            {
                 expectations_cross_quadratic_mean(i,j) = momentum*expectations_cross_quadratic_mean_trainMemory(i,j)
		                                       +(1.0-momentum)*expectations_cross_quadratic_mean(i,j);
//        	 expectations_cross_quadratic_mean(j,i) = expectations_cross_quadratic_mean(i,j);
        	 expectations_cross_quadratic_mean_trainMemory(i,j) = expectations_cross_quadratic_mean(i,j);
//        	 expectations_cross_quadratic_mean_trainMemory(j,i) = expectations_cross_quadratic_mean(i,j);
            }
        }
    }
/*    else if( !duringTraining )
    {
	PLASSERT( ntest+batch_size > 0 );
	for (int i = 0; i < input_size; i++)
        {
            expectations_expectation[i] = ( (real)ntest*expectations_expectation_testMemory[i]
	                                   +(real)batch_size*expectations_expectation[i] )/(real)(ntest+batch_size);
            expectations_expectation_testMemory[i] = expectations_expectation[i];
            for (int j = 0; j < i; j++)
            {
                 expectations_cross_quadratic_mean(i,j) = ( (real)ntest*expectations_cross_quadratic_mean_testMemory(i,j)
		                                           +(real)batch_size*expectations_cross_quadratic_mean(i,j) );
        	 expectations_cross_quadratic_mean(j,i) = expectations_cross_quadratic_mean(i,j);
        	 expectations_cross_quadratic_mean_testMemory(i,j) = expectations_cross_quadratic_mean(i,j);
        	 expectations_cross_quadratic_mean_testMemory(j,i) = expectations_cross_quadratic_mean(i,j);
            }
        }
	ntest += batch_size;
    }
*/
}

/////////////////////////
// Auxiliary Functions //
/////////////////////////
real LayerCostModule::delta_KLdivTerm(int i, int j, int index_i, real over_dq, real one_count)
{
    PLASSERT( over_dq > 0.0 );

    real grad_update = 0.0;

    real Ni_ki = expectations_histo(i,index_i);
    real Ni_ki_shift1 = expectations_histo(i,index_i+1);	    
    real Nj_ki        = expectations_histo(j,index_i);
    real Nj_ki_shift1 = expectations_histo(j,index_i+1);

    PLASSERT( !fast_exact_is_equal(Ni_ki, 0.0) ); // Verification:
                                                  // if expectations_histo is up to date,
                                                  // the expectation(isample,i) has been counted
    real differ_count_j_before = 0.0;
    real differ_count_j_after = 0.0;
    real differ_count_i_before = 0.0;
    real differ_count_i_after = 0.0;

    // What follows is only valuable when the qi's are increased (dq>0).

    if( !fast_exact_is_equal(Nj_ki, 0.0) )
    // if it is zero, then INCREASING qi will not change anything
    // (it was already counted in the next histograms's bin
    {
        // removing the term of the sum that will be modified
        grad_update -= KLdivTerm( Ni_ki, Nj_ki );
							       
        if( fast_exact_is_equal(Ni_ki, one_count) )
            differ_count_j_after = Nj_ki;
        else
            // adding the term of the sum with its modified value
            grad_update += KLdivTerm( Ni_ki-one_count, Nj_ki );

        if( !fast_exact_is_equal(Nj_ki_shift1,0.0) )
        {
            // adding the term of the sum with its modified value
            grad_update += KLdivTerm( Ni_ki_shift1+one_count, Nj_ki_shift1+differ_count_j_after );

            if( !fast_exact_is_equal(Ni_ki_shift1, 0.0) ) // "cas où on regroupe avec le dernier";
            {
                // removing the term of the sum that will be modified
                grad_update -= KLdivTerm( Ni_ki_shift1, Nj_ki_shift1 );		
            }
	    else
	    {
	        // We search   ki' > k(i)+1   such that   n(i,ki') > 0
		differ_count_j_before = Nj_ki_shift1;
		int ki;
		for (ki = index_i+2; ki < histo_size; ki++)
		{
		    differ_count_j_before += expectations_histo( j, ki );
		    if( expectations_histo( i, ki )>0 )
		        break;
		}
		if( ki < histo_size )
		{
                    grad_update -= KLdivTerm( expectations_histo( i, ki ), differ_count_j_before );

		    if( differ_count_j_before > Nj_ki_shift1 )		
                        grad_update += KLdivTerm( expectations_histo( i, ki ), differ_count_j_before - Nj_ki_shift1 );
		        // pb avec differ_count_j_after plus haut??? semble pas
		}
		else
		    "cas où on regroupe avec le dernier";
	    }
        }
        else
        {
            differ_count_i_before = Ni_ki_shift1;
            differ_count_i_after  = Ni_ki_shift1+one_count;
	    int kj;
	    for( kj = index_i+2; kj < histo_size; kj++)
	    {
	        differ_count_i_after += expectations_histo( i, kj );
		if( differ_count_i_before > 0 )
		    differ_count_i_before += expectations_histo( i, kj );
		if( expectations_histo( j, kj ) > 0 )
		    break;
	    }
	    if( kj < histo_size )
            {
                grad_update += KLdivTerm( differ_count_i_after, expectations_histo( j, kj ) );

		if( differ_count_i_before > 0 )
                    grad_update -= KLdivTerm( differ_count_i_before, expectations_histo( j, kj ) );
	    }
	    else
		"cas où on regroupe avec le dernier";   
        }
    }
    return grad_update *over_dq;
}

real LayerCostModule::delta_KLdivTerm_2(int i, int j, int index_i, real over_dq, real one_count)
{
    PLASSERT( over_dq > 0.0 );

    real grad_update = 0.0;

    real Ni_ki = expectations_histo(i,index_i);
    real Ni_ki_shift1 = expectations_histo(i,index_i+1);	    
    real Nj_ki        = expectations_histo(j,index_i);
    real Nj_ki_shift1 = expectations_histo(j,index_i+1);

    PLASSERT( !fast_exact_is_equal(Ni_ki, 0.0) ); // Verification:
                                                  // if expectations_histo is up to date,
                                                  // the expectation(isample,i) has been counted
    real differ_count_j_before = 0.0;
    real differ_count_j_after = 0.0;
    real differ_count_i_before = 0.0;
    real differ_count_i_after = 0.0;
    int n_differ_j_before = 0;
    int n_differ_j_after = 0;
    int n_differ_i_before = 0;
    int n_differ_i_after = 0;

    // What follows is only valuable when the qi's are increased (dq>0).

    if( !fast_exact_is_equal(Nj_ki, 0.0) )
    // if it is zero, then INCREASING qi will not change anything
    // (it was already counted in the next histograms's bin
    {
        // removing the term of the sum that will be modified
        grad_update -= KLdivTerm( Ni_ki, Nj_ki ) *over_dq;
							       
        if( fast_exact_is_equal(Ni_ki, one_count) )
	{
            differ_count_j_after = Nj_ki;
	    n_differ_j_after += 1;
	}
        else
            // adding the term of the sum with its modified value
            grad_update += KLdivTerm( Ni_ki-one_count, Nj_ki ) *over_dq;

        if( !fast_exact_is_equal(Nj_ki_shift1,0.0) )
        {
            // adding the term of the sum with its modified value
            grad_update += KLdivTerm( Ni_ki_shift1+one_count, Nj_ki_shift1+differ_count_j_after ) *(real)(n_differ_j_after+1)*over_dq ;

            if( !fast_exact_is_equal(Ni_ki_shift1, 0.0) ) // "cas où on regroupe avec le dernier";
            {
                // removing the term of the sum that will be modified
                grad_update -= KLdivTerm( Ni_ki_shift1, Nj_ki_shift1 )*over_dq;		
            }
	    else
	    {
	        // We search   ki' > k(i)+1   such that   n(i,ki') > 0
		differ_count_j_before = Nj_ki_shift1;
                n_differ_j_before += 1;
		int ki;
		for (ki = index_i+2; ki < histo_size; ki++)
		{
		    differ_count_j_before += expectations_histo( j, ki );
		    if( expectations_histo( i, ki )>0 )
		        break;
                    n_differ_j_before += 1;
		}
		if( ki < histo_size )
		{
                    grad_update -= KLdivTerm( expectations_histo( i, ki ), differ_count_j_before )*(real)(1+n_differ_j_before)*over_dq;

		    if( differ_count_j_before > Nj_ki_shift1 )		
                        grad_update += KLdivTerm( expectations_histo( i, ki ), differ_count_j_before - Nj_ki_shift1 )*(real)(n_differ_j_before)*over_dq;
		        // pb avec differ_count_j_after plus haut??? semble pas
		}
		else
		    "cas où on regroupe avec le dernier";
	    }
        }
        else
        {
            differ_count_i_before = Ni_ki_shift1;
	    if( differ_count_i_before>0.0 )
	       n_differ_i_before += 1;
            differ_count_i_after  = Ni_ki_shift1+one_count;
	    n_differ_i_after += 1;
	    int kj;
	    for( kj = index_i+2; kj < histo_size; kj++)
	    {
	        differ_count_i_after += expectations_histo( i, kj );
		if( differ_count_i_before > 0 )
		    differ_count_i_before += expectations_histo( i, kj );
		if( expectations_histo( j, kj ) > 0 )
		    break;
		n_differ_i_after += 1;
		if( differ_count_i_before > 0 )
		    n_differ_i_before += 1;
	    }
	    if( kj < histo_size )
            {
                grad_update += KLdivTerm( differ_count_i_after, expectations_histo( j, kj ) ) *(real)(n_differ_i_after+1)*over_dq;

		if( differ_count_i_before > 0 )
                    grad_update -= KLdivTerm( differ_count_i_before, expectations_histo( j, kj ) ) *(real)(n_differ_i_before+1)*over_dq;
	    }
	    else
		"cas où on regroupe avec le dernier";   
        }
    }
    return grad_update;
}

real LayerCostModule::delta_SafeKLdivTerm(int i, int j, int index_i, real over_dq, real one_count)
{
    //PLASSERT( over_dq > 0.0 )

    real grad_update = 0.0;
		    
    real Ni_ki = expectations_histo(i,index_i);
    PLASSERT( !fast_exact_is_equal(Ni_ki, 0.0) ); // Verification:
                                                  // if expectations_histo is up to date,
                                                  // the expectation(isample,i) has been counted
    real Ni_ki_shift1 = expectations_histo(i,index_i+1);
		    
    real Nj_ki        = expectations_histo(j,index_i);
    real Nj_ki_shift1 = expectations_histo(j,index_i+1);


        // removing the term of the sum that will be modified
        grad_update -= KLdivTerm( Ni_ki, Nj_ki );
							       
        // adding the term of the sum with its modified value
        grad_update += KLdivTerm( Ni_ki-one_count, Nj_ki );

        grad_update += KLdivTerm( Ni_ki_shift1+one_count, Nj_ki_shift1 );
	
        grad_update -= KLdivTerm( Ni_ki_shift1, Nj_ki_shift1 );

    return grad_update *over_dq;
}


real LayerCostModule::KLdivTerm(real pi, real pj)
{
    return ( pj - pi ) * safeflog( pi/pj );
}

void LayerCostModule::computeHisto(const Mat& expectations)
{
    int index, batch_size = expectations.length();
    real one_count = 1. / (real)batch_size;
    Vec expectation;
    
    expectations_histo.clear(); 
    for (int isample = 0; isample < batch_size; isample++)
    {
        expectation = expectations(isample);
        for (int i = 0; i < input_size; i++)
        {
	    index = histo_index(expectation[i]);
            expectations_histo(i,index) += one_count;
        }
    }
}



void LayerCostModule::computeSafeHisto(const Mat& expectations)
{
    int index, batch_size = expectations.length();
    real one_count = 1. / (real)(batch_size+histo_size);
    Vec expectation;
    
    expectations_histo.fill(one_count);
/*
    for (int k = 0; k < histo_size; k++)
        for (int i = 0; i < input_size; i++)
            expectations_histo(i,k) = one_count;
*/
    for (int isample = 0; isample < batch_size; isample++)
    {
        expectation = expectations(isample);
        for (int i = 0; i < input_size; i++)
        {
	    index = histo_index(expectation[i]);
            expectations_histo(i,index) += one_count;
        }
    }
}


// Return the index of the (1D) histogram
// corresponding to the real input value q in [0,1]
//
int LayerCostModule::histo_index(real q)
{
    if( q >=1.0 )
       return histo_size-1;

    if( !(q >= 0.0) || !(q < 1.0) )
        PLERROR("LayerCostModule detected an anormal expectation value (%f)", q);

// LINEAR SCALE
    return (int)floor(q*(real)histo_size);

// LOG SCALE
    return max(  (int)floor(log(LOGHISTO_BASE, q))+histo_size , 0);
}

// Returns the minimum amount dq which have to be added/removed to q
// so that q+dq will be counted in the next/previous bin of the histogram
//   (cf. LayerCostModule::histo_index)
//
// Note: we do not care about cases where histo_index(q)=histo_size
//      (this is done in the bpropAccUpdate code)
//
real LayerCostModule::dq(real q)
{
// LINEAR SCALE
    // ** Simple version **
    return LINHISTO_STEP;

    // ** Elaborated version **
    if( fast_exact_is_equal( round(q*(real)histo_size) , ceil(q*(real)histo_size) ) )
       return LINHISTO_STEP;
    else
       return -LINHISTO_STEP;

    // ** BAD VERSION: too unstable **
    // return (real)histo_index(q+1.0/(real)histo_size)/(real)histo_size - q;

// LOG SCALE
    // ** Simple version **
    //if( q < LOGHISTO_MIN )
    //  return LOGHISTO_BASE * LOGHISTO_MIN - q;
    //return q*(LOGHISTO_BASE-1.0);
    
    // ** BAD VERSION: too unstable **
    // real REF = LOGHISTO_BASE * LOGHISTO_MIN;
    // while( true )
    // {
    //     if( q < REF )
    //         return REF - q;
    //     REF *= LOGHISTO_BASE;
    // }
}





////////////
// forget //
////////////
void LayerCostModule::forget()
{
    if( momentum > 0.0)
    {
        expectations_expectation_trainMemory.clear();
        expectations_cross_quadratic_mean_trainMemory.clear();
    }
/*
    expectations_expectation_testMemory.clear();
    expectations_cross_quadratic_mean_testMemory.clear();
    ntest = 0;
*/
}


/////////////////
// addPortName //
/////////////////
void LayerCostModule::addPortName(const string& name)
{
    PLASSERT( portname_to_index.find(name) == portname_to_index.end() );
    portname_to_index[name] = ports.length();
    ports.append(name);
}

//////////////
// getPorts //
//////////////
const TVec<string>& LayerCostModule::getPorts()
{
    return ports;
}

///////////////////
// getPortsSizes //
///////////////////
const TMat<int>& LayerCostModule::getPortSizes()
{
    return port_sizes;
}

//////////////////
// getPortIndex //
//////////////////
int LayerCostModule::getPortIndex(const string& port)
{
    map<string, int>::const_iterator it = portname_to_index.find(port);
    if (it == portname_to_index.end())
        return -1;
    else
        return it->second;
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
