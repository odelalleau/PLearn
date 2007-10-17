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
    "Computes a cost function on Layer, given:            \n",
    "* Expectations for a binomial RBM hidden layer, or   \n"
    "* sigmoid(activation) for a layer of a Neural Net, or\n"
    "* real outputs of any layer                          \n"
    "and Back-propagates the gradient.                    \n"
    "\n"
    "Several cost functions can be chosen.\n"
    "Some only apply for binomial layers. \n");

LayerCostModule::LayerCostModule():
    cost_function(""),
    histo_size(10),
    alpha(0.0),
    momentum(0.0),
    cost_function_completename("")
{
    output_size = 1;
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
        "- \"pascal\":"
        " Pascal Vincent's God given cost function.\n"
        "- \"correlation\":"
        " average of a function applied to the correlations between outputs.\n"
        "- \"kl_div\":"
        " KL divergence between distrubution of outputs (sampled with x)\n"
        "- \"kl_div_simple\":"
        " simple version of kl_div where we count at least one sample per histogram's bin\n"
        "- \"stochastic_cross_entropy\" [default]:"
        " average cross-entropy between pairs of binomial units\n"
        "- \"stochastic_kl_div\":"
        " average KL divergence between pairs of binomial units\n"
        );

    declareOption(ol, "histo_size", &LayerCostModule::histo_size,
                  OptionBase::buildoption,
        "For \"kl_div\" cost functions,\n"
        "number of bins for the histograms (to estimate distributions of outputs).\n"
        "The higher is histo_size, the more precise is the estimation.\n");

    declareOption(ol, "alpha", &LayerCostModule::alpha,
                  OptionBase::buildoption,
        "(>=0) For \"pascal\" cost function,\n"
        "number of bins for the histograms (to estimate distributions of outputs).\n"
        "The higher is histo_size, the more precise is the estimation.\n");

    declareOption(ol, "momentum", &LayerCostModule::momentum,
                  OptionBase::buildoption,
        "(in [0,1[) For \"pascal\" cost function, momentum for the moving means.\n");



    declareOption(ol, "inputs_histo", &LayerCostModule::inputs_histo,
                  OptionBase::learntoption,
                  "Histograms (empirical ditribution) of the output, for all units.\n"
        );

    declareOption(ol, "inputs_expectation", &LayerCostModule::inputs_expectation,
                  OptionBase::learntoption,
                  "Expectation of the output (in [0,1[), for all units.\n"
        );

    declareOption(ol, "inputs_stds", &LayerCostModule::inputs_stds,
                  OptionBase::learntoption,
                  "Standard Deviation of the output, for all units.\n"
        );

    declareOption(ol, "inputs_correlations", &LayerCostModule::inputs_correlations,
                  OptionBase::learntoption,
                  "Correlation of the outputs, for all pairs of units.\n"
        );

    declareOption(ol, "inputs_cross_quadratic_mean", &LayerCostModule::inputs_cross_quadratic_mean,
                  OptionBase::learntoption,
                  "Expectation of the cross products between outputs, for all pairs of units.\n"
        );

    declareOption(ol, "cost_function_completename", &LayerCostModule::cost_function_completename,
                  OptionBase::learntoption,
                  "complete name of cost_function (take into account some internal settings).\n"
        );
}

void LayerCostModule::build_()
{
    PLASSERT( histo_size > 1 );
    PLASSERT( momentum >= 0.0);
    PLASSERT( momentum < 1);

    norm_factor = 1./(real)(input_size*(input_size-1));

    string im = lowerstring( cost_function );
    // choose HERE the *default* cost function
    if( im == "" )
        cost_function = "pascal";
    else
        cost_function = im;
    if( ( cost_function_completename == "" ) || !string_ends_with(cost_function_completename, cost_function) )
        cost_function_completename = string(cost_function);

     // list HERE all *stochastic* cost functions
    if( ( cost_function == "stochastic_cross_entropy")
     || ( cost_function == "stochastic_kl_div") )
        is_cost_function_stochastic = true;

    // list HERE all *non stochastic* cost functions
    // and the specific initialization
    else if( ( cost_function == "kl_div")
          || ( cost_function == "kl_div_simple") )
    {
        is_cost_function_stochastic = false;
        if( input_size > 1 )
            inputs_histo.resize(input_size,histo_size);
        HISTO_STEP = 1.0/(real)histo_size;
    }
    else if( (cost_function == "pascal" )
          || (cost_function == "correlation" ) )
    {
        is_cost_function_stochastic = false;
        if( input_size > 1 )
        {
            inputs_expectation.resize(input_size);
            inputs_cross_quadratic_mean.resize(input_size,input_size);
            if( cost_function == "correlation" )
            {
                inputs_stds.resize(input_size);
                inputs_correlations.resize(input_size,input_size);
            }
            if( momentum > 0.0)
            {
                inputs_expectation_trainMemory.resize(input_size);
                inputs_cross_quadratic_mean_trainMemory.resize(input_size,input_size);
            }
            if( cost_function == "pascal" )
                cost_function_completename = addprepostfix( func_pascal_prefix(), "_", cost_function );
            else if( cost_function == "correlation" )
                cost_function_completename = addprepostfix( func_correlation_prefix(), "_", cost_function );
        }
    }
    else
        PLERROR("LayerCostModule::build_() does not recognize cost function %s",
                 cost_function.c_str());

    // The port story...
    ports.resize(0);
    portname_to_index.clear();
    addPortName("input");
    addPortName("cost");

    port_sizes.resize(nPorts(), 2);
    port_sizes.fill(-1);
    port_sizes(getPortIndex("input"), 1) = input_size;
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

    deepCopyField(inputs_histo, copies);

    deepCopyField(inputs_expectation, copies);
    deepCopyField(inputs_stds, copies);

    deepCopyField(inputs_correlations, copies);
    deepCopyField(inputs_cross_quadratic_mean, copies);

    deepCopyField(inputs_expectation_trainMemory, copies);
    deepCopyField(inputs_cross_quadratic_mean_trainMemory, copies);

    deepCopyField(ports, copies);
}




///////////
// fprop //
///////////


void LayerCostModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( input_size > 1 );

    Mat* p_inputs = ports_value[getPortIndex("input")];
    Mat* p_costs = ports_value[getPortIndex("cost")];

    PLASSERT( ports_value.length() == nPorts() );

    if ( p_costs && p_costs->isEmpty() )
    {
        PLASSERT( p_inputs && !p_inputs->isEmpty() );
        cout << "fprop" << endl;
        fprop(*p_inputs, *p_costs);
    }
}

void LayerCostModule::fprop(const Mat& inputs, Mat& costs)
{
    int n_samples = inputs.length();
    costs.resize( n_samples, output_size );

    if( !is_cost_function_stochastic )
    {
        costs.clear(); // costs(i,0) = 0

        if( cost_function == "kl_div" )
        {
        //! ************************************************************
        //! (non stochastic) SYMETRIC *** K-L DIVERGENCE ***
        //! between probabilities of outputs vectors for all units
        //! ************************************************************
        //!
        //!      cost = - MEAN_{i,j#i} Div_{KL}[ Px(q{i}) | Px(q{j}) ]
        //!
        //!           = - MEAN_{i,j#i} SUM_Q (Nx_j(Q) - Nx_j(Q)) log( Nx_i(Q) / Nx_j(Q) )
        //!
        //! where  q{i} = P(h{i}=1|x): output of the i^th units of the layer.
        //!        Px(.): empirical probability (given data x, we sample the q's).
        //!        Q: interval in [0,1] = one bin of the histogram of the outputs q's.
        //!           Q has size HISTO_STEP
        //!        Nx_i(Q): proportion of q{i} that belong to Q, given data x.
        //!
        //! Note1: one q{i} *entirely* determines one binomial densities of probability.
        //!        ( Bijection {binomial Proba functions} <-> |R )
        //!
        //! Note2: there is a special processing for cases when
        //!        NO outputs q{i} were observed for a given unit i
        //!        at a given bin Q of the histograms whereas another q{j}
        //!        has been observed in Q  (normally, KLdiv -> infinity ).
        //!        SEE function computeKLdiv().
        //! ************************************************************

            computeHisto(inputs);

            costs(0,0) = computeKLdiv();
        }
        else if( cost_function == "kl_div_simple" )
        {
        //! ************************************************************
        //! same as above with a very simple version of the KL-div:
        //! when computing the histogram of the outputs for all units.
        //! we add one count per histogram's bin so as to avoid
        //! numerical problems with zeros.
        //!
        //! SEE function computeSafeHisto(real ).
        //! ************************************************************

            computeSafeHisto(inputs);

            // Computing the KL divergence
            for (int i = 0; i < input_size; i++)
                for (int j = 0; j < i; j++)
                    for (int k = 0; k < histo_size; k++)
                        costs(0,0) += KLdivTerm( inputs_histo(i,k), inputs_histo(j,k));

            // Normalization w.r.t. number of units
            costs(0,0) *= norm_factor;
        }
        else if( cost_function == "pascal" )
        {
        //! ************************************************************
        //! a Pascal Vincent's god-given similarity measure
        //! between outputs vectors for all units
        //! ************************************************************
        //!
        //!      cost = MEAN_{i,j#i} f( Ex[q{i}.q{j}] ) - alpha. MEAN_{i} f( Ex[q{i}] )
        //!
        //! where  q{i} = P(h{i}=1|x): output of the i^th units of the layer
        //!        Ex(.): empirical expectation (given data x)
        //!
        //! ************************************************************

            computePascalStatistics(inputs);

            // Computing the cost
            for (int i = 0; i < input_size; i++)
            {
                if (alpha > 0.0 )
                    costs(0,0) -= alpha * func_pascal(inputs_expectation[i]) *(real)(input_size-1);
                for (int j = 0; j < i; j++)
                    costs(0,0) += func_pascal(inputs_cross_quadratic_mean(i,j));
            }

            costs(0,0) *= norm_factor;
        }
        else if( cost_function == "correlation" )
        {
        //! ************************************************************
        //! a correlation measure
        //! between outputs for all units
        //! ************************************************************
        //!
        //!                            ( Ex[q{i}.q{j}] - Ex[q{i}]Ex[q{j}] )
        //!      cost = MEAN_{i,j#i} f(  -------------------------------- )
        //!                           (      StDx(q{i}) * StDx(q{j})     )
        //!
        //! where  q{i} = P(h{i}=1|x): output of the i^th units of the layer
        //!        Ex(.): empirical esperance (given data x)
        //!        StDx(.): empirical standard deviation (given data x)
        //!
        //! ************************************************************

            computeCorrelationStatistics(inputs);

            // Computing the cost
            for (int i = 0; i < input_size; i++)
                for (int j = 0; j < i; j++)
                    costs(0,0) += func_correlation( inputs_correlations(i,j) );

            costs(0,0) *= norm_factor;
        }


        return; // Do not fprop with the conventional stochastic fprop...
    }

    for (int isample = 0; isample < n_samples; isample++)
        fprop(inputs(isample), costs(isample,0));
}

void LayerCostModule::fprop(const Vec& input, real& cost) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( is_cost_function_stochastic );

    cost = 0.0;
    real  qi, qj, comp_qi, comp_qj; // The outputs (units i,j)
                                    // and some basic operations on it (e.g.: 1-qi, qi/(1-qi))

    if( cost_function == "stochastic_cross_entropy" )
    {
    //! ************************************************************
    //! average *** CROSS ENTROPY ***
    //! between pairs of units (given output = sigmoid(act) )
    //! ************************************************************
    //!
    //!      cost = - MEAN_{i,j#i} CrossEntropy[( P(h_{i}|x) | P(h_{j}|x) )]
    //!
    //!           = - MEAN_{i,j#i} [ q{i}.log(q{j}) + (1-q{i}).log(1-q{j}) ]
    //!
    //! where |  h_{i}: i^th units of the layer
    //!       \  P(.|x): output for input data x
    //!        \ q{i}=P(h{i}=1|v): output of the i^th units of the layer
    //!
    //! ************************************************************

        for( int i = 0; i < input_size; i++ )
        {
           qi = input[i];
           comp_qi = 1.0 - qi;
           for( int j = 0; j < i; j++ )
           {
               qj = input[j];
               comp_qj = 1.0 - qj;

               // H(pi||pj) = H(pi) + D_{KL}(pi||pj)
               cost += qi*safeflog(qj) + comp_qi*safeflog(comp_qj);

               // The symetric part (loop  j=i+1...size)
               cost += qj*safeflog(qi) + comp_qj*safeflog(comp_qi);
           }
        }
        // Normalization w.r.t. number of units
        cost *= norm_factor;
    }

    else if( cost_function == "stochastic_kl_div" )
    {
    //! ************************************************************
    //! average SYMETRIC *** K-L DIVERGENCE ***
    //! between pairs of units (given outputs = sigmoid(act) )
    //! ************************************************************
    //!
    //!      cost = - MEAN_{i,j#i} Div_{KL} [( P(h_{i}|v) | P(h_{j}|v) )]
    //!
    //!           = - MEAN_{i,j#i} [ ( q{j} - q{i} ) log( q{i}/(1-q{i})*(1-q{j})/q{j} ) ]
    //!
    //! where |  h_{i}: i^th units of the layer
    //!       \  P(.|v):  output for input data x
    //!        \ q{i}=P(h{i}=1|v): output of the i^th units of the layer
    //!
    //! ************************************************************

        for( int i = 0; i < input_size; i++ )
        {
           qi = input[i];
           if(fast_exact_is_equal(qi, 1.0))
               comp_qi = REAL_MAX;
           else
               comp_qi = qi/(1.0 - qi);

           for( int j = 0; j < i; j++ )
           {
               qj = input[j];
               if(fast_exact_is_equal(qj, 1.0))
                   comp_qj = REAL_MAX;
               else
                   comp_qj = qj/(1.0 - qj);

               //     - D_{KL}(pi||pj) - D_{KL}(pj||pi)
               cost += (qj-qi)*safeflog(comp_qi/comp_qj);
           }
        }
        // Normalization w.r.t. number of units
        cost *= norm_factor;
    }

    else
        PLERROR("LayerCostModule::fprop() not implemented for cost_cfunction %s\n"
                "- It may be a printing error.\n"
                "- You can try to call LayerCostModule::fprop(const Mat& inputs, Mat& costs)"
                "  if your cost function is non stochastic.\n"
                "- Or else write the code corresponding to your cost function.\n",
                 cost_function.c_str());
}




////////////////////
// bpropAccUpdate //
////////////////////


void LayerCostModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                   const TVec<Mat*>& ports_gradient)
{
    PLASSERT( input_size > 1 );
    PLASSERT( ports_value.length() == nPorts() );
    PLASSERT( ports_gradient.length() == nPorts() );

    cout << "bpropAccUpdate" << endl;

    const Mat* p_inputs = ports_value[getPortIndex("input")];
    Mat* p_inputs_grad = ports_gradient[getPortIndex("input")];
    Mat* p_cost_grad = ports_gradient[getPortIndex("cost")];

    if( p_inputs_grad && p_inputs_grad->isEmpty()
        && p_cost_grad && !p_cost_grad->isEmpty() )
    {
        int n_samples = p_inputs->length();

        PLASSERT( p_inputs && !p_inputs->isEmpty());
        PLASSERT( p_inputs->length() == n_samples );
        PLASSERT( p_cost_grad->length() == n_samples );

        p_inputs_grad->resize(n_samples, input_size);
        p_inputs_grad->clear();

        real qi, qj, comp_qi, comp_qj;
        Vec comp_q(input_size), log_term(input_size);

        if( cost_function == "stochastic_cross_entropy" )
        {
            for (int isample = 0; isample < n_samples; isample++)
            {
                for (int i = 0 ; i < input_size ; i++ )
                {
                    qi = (*p_inputs)(isample,i);
                        comp_qi = 1.0 - qi;
                    comp_q[i] = comp_qi;
                    log_term[i] = safeflog(qi) - safeflog(comp_qi);
                }
                for (int i = 0; i < input_size; i++ )
                {
                    qi = (*p_inputs)(isample,i);
                    comp_qi = comp_q[i];
                    (*p_inputs_grad)(isample,i) = 0.0;
                    for (int j = 0; j < i; j++ )
                    {
                        qj = (*p_inputs)(isample,j);
                        comp_qj=comp_q[j];

                        // log(pj) - log(1-pj) + pj/pi - (1-pj)/(1-pi)
                        (*p_inputs_grad)(isample,i) += log_term[j] + qj/qi - comp_qi/comp_qj;

                        // The symetric part (loop  j=i+1...input_size)
                        (*p_inputs_grad)(isample,j) += log_term[i] + qi/qj - comp_qj/comp_qi;
                    }
                }
                for (int i = 0; i < input_size; i++ )
                {
                    (*p_inputs_grad)(isample, i) *= (*p_cost_grad)(isample,0)
		                                    * norm_factor /(real)n_samples;
                }
            }
        }

        else if( cost_function == "stochastic_kl_div" )
        {
            for (int isample = 0; isample < n_samples; isample++)
            {
                for (int i = 0; i < input_size; i++ )
                {
                    qi = (*p_inputs)(isample,i);
                        comp_qi = 1.0 - qi;
                    if(fast_exact_is_equal(qi, 1.0) || fast_exact_is_equal(qi, 0.0))
                        comp_q[i] = REAL_MAX;
                    else
                        comp_q[i] = 1.0/(qi*comp_qi);
                    log_term[i] = safeflog(qi) - safeflog(comp_qi);
                }
                for (int i = 0; i < input_size; i++ )
                {
                    qi = (*p_inputs)(isample,i);
                    comp_qi = comp_q[i];

                    (*p_inputs_grad)(isample,i) = 0.0;
                    for (int j = 0; j < i ; j++ )
                    {
                        qj = (*p_inputs)(isample,j);
                        comp_qj=comp_q[j];

                        //   [qj - qi]/[qi (1-qi)] - log[ qi/(1-qi) * (1-qj)/qj]
                        (*p_inputs_grad)(isample,i) += (qj - qi)*comp_qi - log_term[i] + log_term[j];

                        // The symetric part (loop  j=i+1...input_size)
                        (*p_inputs_grad)(isample,j) += (qi - qj)*comp_qj - log_term[j] + log_term[i];
                    }
                }
                for (int i = 0; i < input_size; i++ )
                {
                    (*p_inputs_grad)(isample, i) *= (*p_cost_grad)(isample,0)
		                                     * norm_factor /(real)n_samples;
                }
            }
        }

        else if( cost_function == "kl_div" )
        {
            computeHisto(*p_inputs);

            for (int isample = 0; isample < n_samples; isample++)
            {

                // Computing the difference of KL divergence
                // for d_q
                for (int i = 0; i < input_size; i++)
                {
                    (*p_inputs_grad)(isample, i) = 0.0;

                    qi = (*p_inputs)(isample,i);
                    int index_i = histo_index(qi);
                    if( ( index_i == histo_size-1 ) ) // we do not care about this...
                        continue;
                    real over_dqi=1.0/dq(qi);
                    int shift_i;
                    if( over_dqi > 0.0)
                        shift_i = 1;
                    else
                        shift_i = -1;
                    // qi + dq(qi) ==> | p_inputs_histo(i,index_i)   - one_count
                    //                 \ p_inputs_histo(i,index_i+shift_i) + one_count

                    for (int j = 0; j < i; j++)
                    {
                        (*p_inputs_grad)(isample, i) += delta_KLdivTerm(i, j, index_i, over_dqi);

                        qj = (*p_inputs)(isample,j);
                        int index_j = histo_index(qj);
                        if( ( index_j == histo_size-1 ) || ( index_j == 0 ) )
                            continue;
                        real over_dqj=1.0/dq(qj);
                         int shift_j;
                        if( over_dqj > 0.0)
                            shift_j = 1;
                        else
                            shift_j = -1;
                            // qj + dq(qj) ==> | p_inputs_histo(j,index_j)   - one_count
                          //                 \ p_inputs_histo(j,index_j+shift_j) + one_count

                        (*p_inputs_grad)(isample, j) += delta_KLdivTerm(j, i, index_j, over_dqj);
                    }
                }

                // Normalization
                for (int i = 0; i < input_size; i++ )
                {
                    (*p_inputs_grad)(isample, i) *= (*p_cost_grad)(isample,0) * norm_factor;
                }
            }


            // debug Check
            int i=0;
            real cost_before = computeKLdiv();
            for (int isample = 0; isample < n_samples; isample++)
            {
                real qi=(*p_inputs)(isample,i);
                if( histo_index(qi) < histo_size-1 )
                {
                  (*p_inputs)(isample,i) += dq(qi);
                  computeHisto(*p_inputs);
                  real cost_after = computeKLdiv();
                  (*p_inputs)(isample,i) -= dq(qi);
                  cout << "\tglobal cost comparison:" << cost_after - cost_before;
                  cout << "  <?>  " << (*p_inputs_grad)(isample, i)*dq(qi) << endl;
                }
            }


        }

        else if( cost_function == "kl_div_simple" )
        {
            computeSafeHisto(*p_inputs);

            for (int isample = 0; isample < n_samples; isample++)
            {

                // Computing the difference of KL divergence
                // for d_q
                for (int i = 0; i < input_size; i++)
                {
                    (*p_inputs_grad)(isample, i) = 0.0;

                    qi = (*p_inputs)(isample,i);
                    int index_i = histo_index(qi);
                    if( ( index_i == histo_size-1 ) ) // we do not care about this...
                        continue;
                    real over_dqi=1.0/dq(qi);
                    int shift_i;
                    if( over_dqi > 0.0)
                        shift_i = 1;
                    else
                        shift_i = -1;
                    // qi + dq(qi) ==> | p_inputs_histo(i,index_i)   - one_count
                    //                 \ p_inputs_histo(i,index_i+shift_i) + one_count

                    for (int j = 0; j < i; j++)
                    {
                        (*p_inputs_grad)(isample, i) += delta_SafeKLdivTerm(i, j, index_i, over_dqi);

                        qj = (*p_inputs)(isample,j);
                        int index_j = histo_index(qj);
                        if( ( index_j == histo_size-1 ) || ( index_j == 0 ) )
                            continue;
                        real over_dqj=1.0/dq(qj);
                         int shift_j;
                        if( over_dqj > 0.0)
                            shift_j = 1;
                        else
                            shift_j = -1;
                            // qj + dq(qj) ==> | p_inputs_histo(j,index_j)   - one_count
                          //                 \ p_inputs_histo(j,index_j+shift_j) + one_count

                        (*p_inputs_grad)(isample, j) += delta_SafeKLdivTerm(j, i, index_j, over_dqj);
                    }
                }

                // Normalization
                for (int i = 0; i < input_size; i++ )
                {
                    (*p_inputs_grad)(isample, i) *= (*p_cost_grad)(isample,0) * norm_factor;
                }
            }
        }

        else if( cost_function == "pascal" )
        {
            computePascalStatistics(*p_inputs);

            if( momentum > 0.0 )
                for (int isample = 0; isample < n_samples; isample++)
                {
                    for (int i = 0; i < input_size; i++)
                    {
                        qi = (*p_inputs)(isample, i);
                        if (alpha > 0.0 )
                            (*p_inputs_grad)(isample, i) -= alpha*deriv_func_pascal(inputs_expectation[i])
			                                         *(1.0-momentum) *one_count
                                                                 *(real)(input_size-1);
                        for (int j = 0; j < i; j++)
                        {
                            real d_temp = deriv_func_pascal(inputs_cross_quadratic_mean(i,j));
                            qj = (*p_inputs)(isample,j);
                            (*p_inputs_grad)(isample, i) += d_temp *qj*(1.0-momentum)*one_count;
                            (*p_inputs_grad)(isample, j) += d_temp *qi*(1.0-momentum)*one_count;
                        }
                    }
                    for (int i = 0; i < input_size; i++)
                    {
                        (*p_inputs_grad)(isample, i) *= norm_factor;
                    }
                }
            else
                for (int isample = 0; isample < n_samples; isample++)
                {
                    for (int i = 0; i < input_size; i++)
                    {
                        qi = (*p_inputs)(isample, i);
                        if (alpha > 0.0 )
                            (*p_inputs_grad)(isample, i) -= alpha*deriv_func_pascal(inputs_expectation[i])
			                                         *one_count
                                                                 *(real)(input_size-1);
                        for (int j = 0; j < i; j++)
                        {
                            real d_temp = deriv_func_pascal(inputs_cross_quadratic_mean(i,j));
                            qj = (*p_inputs)(isample,j);
                            (*p_inputs_grad)(isample, i) += d_temp *qj *one_count;
                            (*p_inputs_grad)(isample, j) += d_temp *qi *one_count;
                        }
                    }
                    for (int i = 0; i < input_size; i++)
                    {
                        (*p_inputs_grad)(isample, i) *= norm_factor;
                    }
                }
        }

        else if( cost_function == "correlation")
        {
            computeCorrelationStatistics(*p_inputs);

            if( momentum > 0.0 )
                PLERROR( "not implemented yet");
            else
                for (int isample = 0; isample < n_samples; isample++)
                {
                    Vec dSTDi_dqi, dCROSSij_dqj;
                    dSTDi_dqi.resize( input_size );
                    dCROSSij_dqj.resize( input_size );

                    for (int i = 0; i < input_size; i++)
                    {
                        qi = (*p_inputs)(isample, i);

                        //!  dCROSSij_dqj[i] = d[ E(QiQj)-E(Qi)E(Qj) ]/d[qj(t)]
                        //!                  = ( qi(t) - E(Qi) ) / n_samples
                        //!
                        //!  dSTDi_dqi[i] = d[ STD(Qi) ]/d[qi(t)]
                        //!               = d[ sqrt( E(Qi^2) -E(Qi)^2 ]/d[qi(t)]
                        //!               = 1 / [ 2.STD(Qi) ] * d[ E(Qi^2) -E(Qi)^2 ]/d[qi(t)]
                        //!               = 1 / [ 2.STD(Qi) ] * [ 2*qi(t) / n_samples - 2*E(Qi) / n_samples ]
                        //!               = ( qi(t) - E(Qi) ) / ( n_samples * STD(Qi) )
                        //!               = dCROSSij_dqj[i] / STD(Qi)
                        //!
                        dCROSSij_dqj[i] = ( qi - inputs_expectation[i] )*one_count;
                        dSTDi_dqi[i] = dCROSSij_dqj[i] / inputs_stds[i];

                        for (int j = 0; j < i; j++)
                        {
                            qj = (*p_inputs)(isample,j);

                            real correlation_denum = inputs_stds[i]*inputs_stds[j];
                            real dfunc_dCorr = deriv_func_correlation( inputs_correlations(i,j) );
                            real correlation_num = ( inputs_cross_quadratic_mean(i,j)
                                                     - inputs_expectation[i]*inputs_expectation[j] );

                            (*p_inputs_grad)(isample, i) += dfunc_dCorr * (
                                                                    correlation_denum * dCROSSij_dqj[j]
                                                                  - correlation_num * dSTDi_dqi[i] * inputs_stds[j]
                                                                    ) / (correlation_denum * correlation_denum);

                            (*p_inputs_grad)(isample, j) += dfunc_dCorr * (
                                                                    correlation_denum * dCROSSij_dqj[i]
                                                                  - correlation_num * dSTDi_dqi[j] * inputs_stds[i]
                                                                    ) / (correlation_denum * correlation_denum);
                        }
                    }
                    for (int i = 0; i < input_size; i++)
                        (*p_inputs_grad)(isample, i) *= norm_factor;
                }
        }
        else
            PLERROR("LayerCostModule::bpropAccUpdate() not implemented for cost function %s",
                     cost_function.c_str());

        checkProp(ports_gradient);
    }
    else if( !p_inputs_grad && !p_cost_grad )
        return;
    else
        PLERROR("In LayerCostModule::bpropAccUpdate - Port configuration not implemented ");

}


////////////////////////////////////////////////////
// Auxiliary Functions for Pascal's cost function //
////////////////////////////////////////////////////
void LayerCostModule::computePascalStatistics(const Mat& inputs)
{
    int n_samples = inputs.length();
    one_count = 1. / (real)n_samples;
    Vec input;

    inputs_expectation.clear();
    inputs_cross_quadratic_mean.clear();

    for (int isample = 0; isample < n_samples; isample++)
    {
        input = inputs(isample);
        for (int i = 0; i < input_size; i++)
        {
            inputs_expectation[i] += input[i];
            for (int j = 0; j < i; j++)
                 inputs_cross_quadratic_mean(i,j) += input[i] * input[j];
        }
    }

    for (int i = 0; i < input_size; i++)
    {
        inputs_expectation[i] *= one_count;
        for (int j = 0; j < i; j++)
        {
             inputs_cross_quadratic_mean(i,j) *= one_count;
        }
    }
    if( ( momentum > 0.0 ) && during_training )
    {
        for (int i = 0; i < input_size; i++)
        {
            inputs_expectation[i] = momentum*inputs_expectation_trainMemory[i]
                                         +(1.0-momentum)*inputs_expectation[i];
            inputs_expectation_trainMemory[i] = inputs_expectation[i];
            for (int j = 0; j < i; j++)
            {
                 inputs_cross_quadratic_mean(i,j) = momentum*inputs_cross_quadratic_mean_trainMemory(i,j)
                                                       +(1.0-momentum)*inputs_cross_quadratic_mean(i,j);
                 inputs_cross_quadratic_mean_trainMemory(i,j) = inputs_cross_quadratic_mean(i,j);
            }
        }
    }
}
string LayerCostModule::func_pascal_prefix()
{
    string prefix = "exp";
    return prefix;
}
real LayerCostModule::func_pascal(real value)
{
    return exp(value);
}
real LayerCostModule::deriv_func_pascal(real value)
{
    return exp(value);
}


void LayerCostModule::computeCorrelationStatistics(const Mat& inputs)
{
    int n_samples = inputs.length();
    one_count = 1. / (real)n_samples;
    Vec input;

    inputs_expectation.clear();
    inputs_cross_quadratic_mean.clear();
    inputs_correlations.clear();

    for (int isample = 0; isample < n_samples; isample++)
    {
        input = inputs(isample);
        for (int i = 0; i < input_size; i++)
        {
            inputs_expectation[i] += input[i];
            inputs_cross_quadratic_mean(i,i) += input[i] * input[i];
            for (int j = 0; j < i; j++)
                 inputs_cross_quadratic_mean(i,j) += input[i] * input[j];
        }
    }

    for (int i = 0; i < input_size; i++)
    {
        //! Normalization
        inputs_expectation[i] *= one_count;
        inputs_cross_quadratic_mean(i,i) *= one_count;

            inputs_stds[i] = sqrt( inputs_cross_quadratic_mean(i,i)
                              - inputs_expectation[i] * inputs_expectation[i] );

        for (int j = 0; j < i; j++)
        {
            //! Normalization
            inputs_cross_quadratic_mean(i,j) *= one_count;

            //! Correlations
            inputs_correlations(i,j) = (
                                  inputs_cross_quadratic_mean(i,j)
                                  - inputs_expectation[i]*inputs_expectation[j]
                                  ) / ( inputs_stds[i] * inputs_stds[j] );
        }
    }
    //! Be careful: 'inputs_correlations' matrix is only computed
    //!  on the triangle subpart 'i' > 'j'
    //!  ('i'/'j': first/second argument)

    if(  during_training )
    {
        if( momentum > 0.0 )
            PLERROR("not implemented yet");
    }
}
string LayerCostModule::func_correlation_prefix()
{
    string prefix = "squared";
    return "square";
}
real LayerCostModule::func_correlation(real correlation)
{
    return correlation * correlation;
}
real LayerCostModule::deriv_func_correlation(real correlation)
{
    return 2 * correlation;
}
/////////////////////////
// Auxiliary Functions //
/////////////////////////


real LayerCostModule::delta_KLdivTerm(int i, int j, int index_i, real over_dq)
{
    PLASSERT( over_dq > 0.0 );

    real grad_update = 0.0;

    real Ni_ki = inputs_histo(i,index_i);
    real Ni_ki_shift1 = inputs_histo(i,index_i+1);
    real Nj_ki        = inputs_histo(j,index_i);
    real Nj_ki_shift1 = inputs_histo(j,index_i+1);

    PLASSERT( !fast_exact_is_equal(Ni_ki, 0.0) ); // Verification:
                                                  // if inputs_histo is up to date,
                                                  // the input(isample,i) has been counted
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
            grad_update += KLdivTerm( Ni_ki-one_count, Nj_ki )
                           *over_dq;

        if( !fast_exact_is_equal(Nj_ki_shift1,0.0) )
        {
            // adding the term of the sum with its modified value
            grad_update += KLdivTerm( Ni_ki_shift1+one_count, Nj_ki_shift1+differ_count_j_after )
                          *(real)(1+n_differ_j_after)*over_dq ;

            if( !fast_exact_is_equal(Ni_ki_shift1, 0.0) ) // "cas ou on regroupe avec le dernier";
            {
                // removing the term of the sum that will be modified
                grad_update -= KLdivTerm( Ni_ki_shift1, Nj_ki_shift1 )
                               *over_dq;
            }
            else
            {
                // We search   ki' > k(i)+1   such that   n(i,ki') > 0
                differ_count_j_before = Nj_ki_shift1;
                n_differ_j_before += 1;
                int ki;
                for (ki = index_i+2; ki < histo_size; ki++)
                {
                    differ_count_j_before += inputs_histo( j, ki );
                    if( inputs_histo( i, ki )>0 )
                        break;
                    n_differ_j_before += 1;
                }
                if( ki < histo_size )
                {
                    grad_update -= KLdivTerm( inputs_histo( i, ki ), differ_count_j_before )
                                   *(real)(1+n_differ_j_before)*over_dq;

                    if( differ_count_j_before > Nj_ki_shift1 )
                        grad_update += KLdivTerm( inputs_histo( i, ki ), differ_count_j_before - Nj_ki_shift1 )
                                       *(real)(n_differ_j_before)*over_dq;
                        // pb avec differ_count_j_after plus haut??? semble pas
                }
                else
                {
                    // cas ou on regroupe avec le dernier (easy)
                }
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
                differ_count_i_after += inputs_histo( i, kj );
                if( differ_count_i_before > 0 )
                    differ_count_i_before += inputs_histo( i, kj );
                if( inputs_histo( j, kj ) > 0 )
                    break;
                n_differ_i_after += 1;
                if( differ_count_i_before > 0 )
                    n_differ_i_before += 1;
            }
            if( kj < histo_size )
            {
                grad_update += KLdivTerm( differ_count_i_after, inputs_histo( j, kj ) )
                               *(real)(1+n_differ_i_after)*over_dq;

                if( differ_count_i_before > 0 )
                    grad_update -= KLdivTerm( differ_count_i_before, inputs_histo( j, kj ) )
                                   *(real)(1+n_differ_i_before)*over_dq;
            }
            else
            {
                // cas ou on regroupe avec le dernier
            }
        }
    }
    return grad_update*over_dq;
}

real LayerCostModule::delta_SafeKLdivTerm(int i, int j, int index_i, real over_dq)
{
    //PLASSERT( over_dq > 0.0 )

    real grad_update = 0.0;

    real Ni_ki = inputs_histo(i,index_i);
    PLASSERT( !fast_exact_is_equal(Ni_ki, 0.0) ); // Verification:
                                                  // if inputs_histo is up to date,
                                                  // the input(isample,i) has been counted
    real Ni_ki_shift1 = inputs_histo(i,index_i+1);

    real Nj_ki        = inputs_histo(j,index_i);
    real Nj_ki_shift1 = inputs_histo(j,index_i+1);


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

real LayerCostModule::computeKLdiv()
{
            real cost = 0;
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
                    real last_positive_Ni_k, last_positive_Nj_k;
                    int last_n_differ;
                    for (int k = 0; k < histo_size; k++)
                    {
                        real Ni_k = inputs_histo(i,k) + differ_count_i;
                        real Nj_k = inputs_histo(j,k) + differ_count_j;
                        if( fast_exact_is_equal(Ni_k, 0.0) )
                        {
                         // differ_count_j += inputs_histo(j,k);
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
                            cost += KLdivTerm(Ni_k,Nj_k) *(real)(1+n_differ) *HISTO_STEP;
                            differ_count_i = 0.0;
                            differ_count_j = 0.0;
                            n_differ = 0;
                            last_positive_Ni_k = Ni_k;
                            last_positive_Nj_k = Nj_k;
                            last_n_differ = n_differ;
                        }
                    }
                    if( differ_count_i > 0.0 )
                    {
                        // cas ou on regroupe avec le dernier
//                        cost -= KLdivTerm(last_positive_Ni_k,last_positive_Nj_k)
//                                  *(real)(1+last_n_differ) *HISTO_STEP;
//                        cost += KLdivTerm(last_positive_Ni_k+differ_count_i,last_positive_Nj_k)
//                                 *(real)(1+last_n_differ+n_differ) *HISTO_STEP;
                    }

                    else if ( differ_count_j > 0.0 )
                    {
                        // cas ou on regroupe avec le dernier
//                        cost -= KLdivTerm(last_positive_Ni_k,last_positive_Nj_k)
//                                 *(real)(1+last_n_differ) *HISTO_STEP;
//                        cost += KLdivTerm(last_positive_Ni_k,last_positive_Nj_k+differ_count_j)
//                                 *(real)(1+last_n_differ+n_differ) *HISTO_STEP;
                    }
                }
            // Normalization w.r.t. number of units
            return cost *norm_factor;
}

void LayerCostModule::computeHisto(const Mat& inputs)
{
    int n_samples = inputs.length();
    one_count = 1. / (real)n_samples;
    Vec input;

    inputs_histo.clear();
    for (int isample = 0; isample < n_samples; isample++)
    {
        input = inputs(isample);
        for (int i = 0; i < input_size; i++)
            inputs_histo(i, histo_index(input[i]) ) += one_count;
    }
}



void LayerCostModule::computeSafeHisto(const Mat& inputs)
{
    int n_samples = inputs.length();
    one_count = 1. / (real)(n_samples+histo_size);
    Vec input;

    inputs_histo.fill(one_count);

    for (int isample = 0; isample < n_samples; isample++)
    {
        input = inputs(isample);
        for (int i = 0; i < input_size; i++)
            inputs_histo(i, histo_index(input[i])) += one_count;
    }
}


// Return the index of the (1D) histogram
// corresponding to the real input value q in [0,1]
//
int LayerCostModule::histo_index(real q)
{
    PLASSERT( (q >= 0.) && (q < 1.) );

    if( fast_exact_is_equal( q, 1. ) )
       return histo_size - 1;

// LINEAR SCALE
    return (int)floor(q*(real)histo_size);
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
    // ** Simple version **
    return HISTO_STEP;

    // ** Elaborated version **
    //if( fast_exact_is_equal( round(q*(real)histo_size) , ceil(q*(real)histo_size) ) )
    //   return HISTO_STEP;
    //else
    //   return -HISTO_STEP;

    // ** BAD VERSION: too unstable **
    // return (real)histo_index(q+1.0/(real)histo_size)/(real)histo_size - q;
}


////////////
// forget //
////////////
void LayerCostModule::forget()
{
    inputs_histo.clear();

    inputs_expectation.clear();
    inputs_stds.clear();

    inputs_correlations.clear();
    inputs_cross_quadratic_mean.clear();
    if( momentum > 0.0)
    {
        inputs_expectation_trainMemory.clear();
        inputs_cross_quadratic_mean_trainMemory.clear();
    }
    one_count = 0.;
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
