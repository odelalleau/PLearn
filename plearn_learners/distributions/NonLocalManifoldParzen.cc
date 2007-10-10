// -*- C++ -*-

// NonLocalManifoldParzen.cc
//
// Copyright (C) 2004 Yoshua Bengio & Hugo Larochelle
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

// Authors: Yoshua Bengio & Martin Monperrus

/*! \file NonLocalManifoldParzen.cc */


#include "NonLocalManifoldParzen.h"
#include <plearn/display/DisplayUtils.h>
#include <plearn/math/plapack.h>
#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/ColumnSumVariable.h>
#include <plearn/var/NllGeneralGaussianVariable.h>
#include <plearn/var/NoBpropVariable.h>
#include <plearn/var/ReshapeVariable.h>
#include <plearn/var/SourceVariable.h>
#include <plearn/var/SquareVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/ThresholdBpropVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/vmat/AppendNeighborsVMatrix.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>


namespace PLearn {
using namespace std;


NonLocalManifoldParzen::NonLocalManifoldParzen()
    :  
    reference_set(0), 
    ncomponents(1), 
    nneighbors(5), 
    nneighbors_density(-1), 
    store_prediction(false),
    learn_mu(false),
    sigma_init(0.1), 
    sigma_min(0.00001), 
    mu_nneighbors(2), 
    sigma_threshold_factor(-1), 
    svd_threshold(1e-8), 
    nhidden(10), 
    weight_decay(0),
    penalty_type("L2_square"),
    batch_size(1)
{
}

PLEARN_IMPLEMENT_OBJECT(NonLocalManifoldParzen, 
                        "Non-Local version of Manifold Parzen Windows",
                        "Manifold Parzen Windows density model, where the\n"
                        "parameters of the kernel for each training point\n"
                        "are predicted by a neural network.\n"
    );


void NonLocalManifoldParzen::declareOptions(OptionList& ol)
{

    declareOption(ol, "parameters", &NonLocalManifoldParzen::parameters, 
                  OptionBase::learntoption,
                  "Parameters of the tangent_predictor function.\n"
        );

    declareOption(ol, "reference_set", &NonLocalManifoldParzen::reference_set, 
                  OptionBase::learntoption,
                  "Reference points for density computation.\n"
        );

    declareOption(ol, "ncomponents", &NonLocalManifoldParzen::ncomponents, 
                  OptionBase::buildoption,
                  "Number of \"principal components\" to predict\n"
                  "for kernel parameters prediction.\n"
        );

    declareOption(ol, "nneighbors", &NonLocalManifoldParzen::nneighbors, 
                  OptionBase::buildoption,
                  "Number of nearest neighbors to consider in training procedure.\n"
        );

    declareOption(ol, "nneighbors_density", 
                  &NonLocalManifoldParzen::nneighbors_density, 
                  OptionBase::buildoption,
                  "Number of nearest neighbors to consider for\n"
                  "p(x) density estimation.\n"
        );

    declareOption(ol, "store_prediction", 
                  &NonLocalManifoldParzen::store_prediction, 
                  OptionBase::buildoption,
                  "Indication that the predicted parameters should be stored.\n"
                  "This may make testing faster. Note that the predictions are\n"
                  "stored after the last training stage\n"
        );


    declareOption(ol, "paramsvalues", 
                  &NonLocalManifoldParzen::paramsvalues, 
                  OptionBase::learntoption,
                  "The learned parameter vector.\n"
        );

    // ** Gaussian kernel options

    declareOption(ol, "learn_mu", &NonLocalManifoldParzen::learn_mu, 
                  OptionBase::buildoption,
                  "Indication that the deviation from the training point\n"
                  "in a Gaussian kernel (called mu) should be learned.\n"
        );

    declareOption(ol, "sigma_init", &NonLocalManifoldParzen::sigma_init, 
                  OptionBase::buildoption,
                  "Initial minimum value for sigma noise.\n"
        );

    declareOption(ol, "sigma_min", &NonLocalManifoldParzen::sigma_min, 
                  OptionBase::buildoption,
                  "The minimum value for sigma noise.\n"
        );

    declareOption(ol, "mu_nneighbors", &NonLocalManifoldParzen::mu_nneighbors, 
                  OptionBase::buildoption,
                  "Number of nearest neighbors to learn the mus \n"
                  "(if < 0, mu_nneighbors = nneighbors).\n"
        );

    declareOption(ol, "sigma_threshold_factor", 
                  &NonLocalManifoldParzen::sigma_threshold_factor, 
                  OptionBase::buildoption,
                  "Threshold factor of the gradient on the sigma noise\n"
                  "parameter of the Gaussian kernel. If < 0, then\n"
                  "no threshold is used."
        );

    declareOption(ol, "svd_threshold", 
                  &NonLocalManifoldParzen::svd_threshold, OptionBase::buildoption,
                  "Threshold to accept singular values of F in solving for\n"
                  "linear combination weights on tangent subspace.\n"
        );

    // ** Neural network predictor **

    declareOption(ol, "nhidden", 
                  &NonLocalManifoldParzen::nhidden, OptionBase::buildoption,
                  "Number of hidden units of the neural network.\n"
        );

    declareOption(ol, "weight_decay", &NonLocalManifoldParzen::weight_decay, 
                  OptionBase::buildoption,
                  "Global weight decay for all layers.\n");

    declareOption(ol, "penalty_type", &NonLocalManifoldParzen::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");

    declareOption(ol, "optimizer", &NonLocalManifoldParzen::optimizer, 
                  OptionBase::buildoption,
                  "Optimizer that optimizes the cost function.\n"
        );

    declareOption(ol, "batch_size", 
                  &NonLocalManifoldParzen::batch_size, OptionBase::buildoption,
                  "How many samples to use to estimate the average gradient\n"
                  "before updating the weights. If <= 0, is equivalent to\n"
                  "specifying training_set->length() \n");


    // ** Stored outputs of neural network

    declareOption(ol, "mus", 
                  &NonLocalManifoldParzen::mus, OptionBase::learntoption,
                  "The stored mu vectors for the reference set.\n"
        );

    declareOption(ol, "sns", &NonLocalManifoldParzen::sns, 
                  OptionBase::learntoption,
                  "The stored sigma noise values for the reference set.\n"
        );

    declareOption(ol, "sms", &NonLocalManifoldParzen::sms, 
                  OptionBase::learntoption,
                  "The stored sigma manifold values for the reference set.\n"
        );

    declareOption(ol, "Fs", &NonLocalManifoldParzen::Fs, OptionBase::learntoption,
                  "The storaged \"principal components\" (F) values for\n"
                  "the reference set.\n"
        );


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NonLocalManifoldParzen::build_()
{

    if (inputsize_>0)
    {
        if (nhidden <= 0) 
            PLERROR("NonLocalManifoldParzen::Number of hidden units "
                    "should be positive, now %d\n",nhidden);

        Var log_n_examples(1,1,"log(n_examples)");
        if(train_set)
        {
            L = train_set->length();
            reference_set = train_set; 
        }

        log_L= pl_log((real) L);
        parameters.resize(0);
        
        // Neural network prediction of principal components

        x = Var(inputsize_);
        x->setName("x");

        W = Var(nhidden+1,inputsize_,"W");
        parameters.append(W);

        Var a; // outputs of hidden layer
        a = affine_transform(x,W);
        a->setName("a");

        V = Var(ncomponents*(inputsize_+1),nhidden,"V");
        parameters.append(V);

        // TODO: instead, make NllGeneralGaussianVariable use vector... (DONE)
        //components = reshape(affine_transform(V,a),ncomponents,n);
        components = affine_transform(V,a);
        components->setName("components");

        // Gaussian kernel parameters prediction

        muV = Var(inputsize_+1,nhidden,"muV");
        snV = Var(2,nhidden,"snV");
    
        parameters.append(muV);
        parameters.append(snV);

        if(learn_mu)
            mu = affine_transform(muV,a);
        else
        {
            mu = new SourceVariable(inputsize_,1);
            mu->value.clear();
        }
        mu->setName("mu");

        min_sig = new SourceVariable(1,1);
        min_sig->value[0] = sigma_min;
        min_sig->setName("min_sig");
        init_sig = Var(1,1);
        init_sig->setName("init_sig");
        parameters.append(init_sig);

        sn = square(affine_transform(snV,a)) + min_sig + square(init_sig);
        sn->setName("sn");
        
        if(sigma_threshold_factor > 0)
            sn = threshold_bprop(sn,sigma_threshold_factor);

        predictor = Func(x, parameters , components & mu & sn );
    
        Var target_index = Var(1,1);
        target_index->setName("target_index");
        Var neighbor_indexes = Var(nneighbors,1);
        neighbor_indexes->setName("neighbor_indexes");

        tangent_targets = Var(nneighbors,inputsize_);
        if(mu_nneighbors < 0 ) mu_nneighbors = nneighbors;

        Var nll;
        nll = nll_general_gaussian(components, mu, sn, tangent_targets, 
                                   log_L, learn_mu, mu_nneighbors); 

        Var knn = new SourceVariable(1,1);
        knn->setName("knn");
        knn->value[0] = nneighbors;
        sum_nll = new ColumnSumVariable(nll) / knn;

        // Weight decay penalty
        if(weight_decay > 0 )
        {
            sum_nll += affine_transform_weight_penalty(
                W,weight_decay,0,penalty_type) + 
                affine_transform_weight_penalty(
                V,weight_decay,0,penalty_type) + 
                affine_transform_weight_penalty(
                muV,weight_decay,0,penalty_type) + 
                affine_transform_weight_penalty(
                snV,weight_decay,0,penalty_type);
        }

        cost_of_one_example = Func(x & tangent_targets & target_index & 
                                   neighbor_indexes, parameters, sum_nll);

        if(nneighbors_density >= L || nneighbors_density < 0) 
            nneighbors_density = L;

        // Output storage variables
        t_row.resize(inputsize_);
        Ut_svd.resize(inputsize_,inputsize_);
        V_svd.resize(ncomponents,ncomponents);
        F.resize(components->length(),components->width());
        z.resize(inputsize_);
        x_minus_neighbor.resize(inputsize_);
        neighbor_row.resize(inputsize_);

        // log_density and Kernel methods variables
        U_temp.resize(ncomponents,inputsize_);
        mu_temp.resize(inputsize_);
        sm_temp.resize(ncomponents);
        sn_temp.resize(1);
        diff.resize(inputsize_);

        mus.resize(L, inputsize_);
        sns.resize(L);
        sms.resize(L,ncomponents);
        Fs.resize(L);
        for(int i=0; i<L; i++)
        {
            Fs[i].resize(ncomponents,inputsize_);
        }

        if(paramsvalues.length() == parameters.nelems())
            parameters << paramsvalues;
        else
        {
            paramsvalues.resize(parameters.nelems());
            initializeParams();
            if(optimizer)
                optimizer->reset();
        }
        parameters.makeSharedValue(paramsvalues);
    }

}

void NonLocalManifoldParzen::knn(const VMat& vm, const Vec& x, const int& k, TVec<int>& neighbors, bool sortk) const
{
    int n = vm->length();
    distances.resize(n,2);
    distances.column(1) << Vec(0, n-1, 1);
    dk.setDataForKernelMatrix(vm);
    t_dist.resize(n);
    dk.evaluate_all_i_x(x, t_dist);
    distances.column(0) << t_dist;
    partialSortRows(distances, k, sortk);
    neighbors.resize(k);

    for (int i=0; i < k  && i<n; i++)
    {
        neighbors[i] = int(distances(i,1));
    }

}

// ### Nothing to add here, simply calls build_
void NonLocalManifoldParzen::build()
{
    inherited::build();
    build_();
}

#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif

void NonLocalManifoldParzen::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{  
    inherited::makeDeepCopyFromShallowCopy(copies);

    // Protected

    deepCopyField(cost_of_one_example, copies);
    varDeepCopyField(x, copies);
    varDeepCopyField(W, copies);
    varDeepCopyField(V, copies);
    varDeepCopyField(muV, copies);
    varDeepCopyField(snV, copies);
    varDeepCopyField(tangent_targets, copies);
    varDeepCopyField(components, copies);
    varDeepCopyField(mu, copies);
    varDeepCopyField(sn, copies);
    varDeepCopyField(sum_nll, copies);
    varDeepCopyField(min_sig, copies);
    varDeepCopyField(init_sig, copies);
    deepCopyField(predictor, copies);
    deepCopyField(U_temp,copies);
    deepCopyField(F, copies);
    deepCopyField(distances,copies);
    deepCopyField(mu_temp,copies);
    deepCopyField(sm_temp,copies);
    deepCopyField(sn_temp,copies);
    deepCopyField(diff,copies);
    deepCopyField(z,copies);
    deepCopyField(x_minus_neighbor,copies);
    deepCopyField(t_row,copies);
    deepCopyField(neighbor_row,copies);
    deepCopyField(log_gauss,copies);
    deepCopyField(t_dist,copies);
    deepCopyField(t_nn,copies);
    deepCopyField(Ut_svd, copies);
    deepCopyField(V_svd, copies);
    deepCopyField(S_svd, copies);
    deepCopyField(mus, copies);
    deepCopyField(sns, copies);
    deepCopyField(sms, copies);
    deepCopyField(Fs, copies);
    deepCopyField(train_set_with_targets, copies);
    deepCopyField(targets_vmat, copies);
    varDeepCopyField(totalcost, copies);
    deepCopyField(paramsvalues, copies);
    
    // Public

    deepCopyField(parameters, copies);    
    deepCopyField(reference_set,copies);
    deepCopyField(optimizer, copies);

}


void NonLocalManifoldParzen::forget()
{
    inherited::forget();
    if (train_set) initializeParams();
    if(optimizer) optimizer->reset();
    stage = 0;
}

void NonLocalManifoldParzen::train()
{
    // Check whether gradient descent is going to be done
    // If not, then we don't need to store the parameters,
    // except for sn...
    bool flag = (nstages == stage);

    // Update sigma_min, in case it was changed,
    // e.g. using an HyperLearner
    min_sig->value[0] = sigma_min;

    // Set train_stats if not already done.
    if (!train_stats)
        train_stats = new VecStatsCollector();

    if (!cost_of_one_example)
        PLERROR("NonLocalManifoldParzen::train: build has not been run after setTrainingSet!");

    if(stage == 0)
    {
        targets_vmat = append_neighbors(
            train_set, nneighbors, true);
        nsamples = batch_size>0 ? batch_size : train_set->length();

        totalcost = meanOf(train_set_with_targets, cost_of_one_example, nsamples);

        if(optimizer)
        {
            optimizer->setToOptimize(parameters, totalcost);
            optimizer->build();
        }
        else PLERROR("NonLocalManifoldParzen::train can't train without setting an optimizer first!");
    }

    int optstage_per_lstage = train_set->length()/nsamples;

    PP<ProgressBar> pb;
    if(report_progress>0)
        pb = new ProgressBar("Training NonLocalManifoldParzen from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

    t_row.resize(train_set.width());

    int initial_stage = stage;
    bool early_stop=false;
    while(stage<nstages && !early_stop)
    {
        optimizer->nstages = optstage_per_lstage;
        train_stats->forget();
        optimizer->early_stop = false;
        optimizer->optimizeN(*train_stats);
        train_stats->finalize();
        if(verbosity>2)
            cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
        ++stage;
        if(pb)
            pb->update(stage-initial_stage);
    }
    if(verbosity>1)
        cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

    if(store_prediction && !flag)
    {
        for(int t=0; t<L;t++)
        {
            reference_set->getRow(t,neighbor_row);
            predictor->fprop(neighbor_row, F.toVec() & mus(t) & sns.subVec(t,1));
            // N.B. this is the SVD of F'
            lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
            for (int k=0;k<ncomponents;k++)
            {
                sms(t,k) = mypow(S_svd[k],2);
                Fs[t](k) << Ut_svd(k);
            }
            sns[t] += sigma_min - min_sig->value[0];
        }
    }
}

//////////////////////
// initializeParams //
//////////////////////
void NonLocalManifoldParzen::initializeParams()
{
    real delta = 1.0 / sqrt(real(inputsize_));
    random_gen->fill_random_uniform(W->value, -delta, delta);
    delta = 1.0 / real(nhidden);
    random_gen->fill_random_uniform(V->matValue, -delta, delta);
    random_gen->fill_random_uniform(snV->matValue, -delta, delta);
    random_gen->fill_random_uniform(muV->matValue, -delta, delta);
    W->matValue(0).clear();
    V->matValue(0).clear();
    muV->matValue(0).clear();
    snV->matValue(0).clear();
    init_sig->value[0] = sqrt(sigma_init);
}

/////////////////
// log_density //
/////////////////
real NonLocalManifoldParzen::log_density(const Vec& x) const {
    // Compute log-density.
    real ret = 0;
    t_row << x;
    real mahal = 0;
    real norm_term = 0;

    // Update sigma_min, in case it was changed,
    // e.g. using an HyperLearner

    if(store_prediction && min_sig->value[0] != sigma_min)
    {
        for(int i=0; i<L; i++)
        {
            sns[i] += sigma_min - min_sig->value[0];
        }
    }

    min_sig->value[0] = sigma_min;

    if(nneighbors_density != L)
    {
        // Fetching nearest neighbors for density estimation.
        knn(reference_set,x,nneighbors_density,t_nn,0);
        log_gauss.resize(t_nn.length());
        for(int neighbor=0; neighbor<t_nn.length(); neighbor++)
        {
            reference_set->getRow(t_nn[neighbor],neighbor_row);
            if(!store_prediction)
            {
                predictor->fprop(neighbor_row, F.toVec() & mu_temp & sn_temp);
                // N.B. this is the SVD of F'
                lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
                for (int k=0;k<ncomponents;k++)
                {
                    sm_temp[k] = mypow(S_svd[k],2);
                    U_temp(k) << Ut_svd(k);
                }
            }
            else
            {
                if(learn_mu)
                    mu_temp << mus(t_nn[neighbor]);
                sn_temp[0] = sns[t_nn[neighbor]];
                sm_temp << sms(t_nn[neighbor]);
                U_temp << Fs[t_nn[neighbor]];
            }
            if(learn_mu)
            {
                substract(t_row,neighbor_row,x_minus_neighbor);
                substract(x_minus_neighbor,mu_temp,z);
            }
            else
                substract(t_row,neighbor_row,z);
                
            mahal = -0.5*pownorm(z)/sn_temp[0];
            norm_term = - inputsize_/2.0 * Log2Pi 
                - log_L - 0.5*(inputsize_-ncomponents)*pl_log(sn_temp[0]);


            for(int k=0; k<ncomponents; k++)
            {
                mahal -= square(dot(z,U_temp(k)))*(0.5/(sm_temp[k]+sn_temp[0]) 
                                                   - 0.5/sn_temp[0]);
                norm_term -= 0.5*pl_log(sm_temp[k]+sn_temp[0]);
            }

            log_gauss[neighbor] = mahal + norm_term;
        }
    }
    else
    {
        // Fetching nearest neighbors for density estimation.
        log_gauss.resize(L);
        for(int t=0; t<L;t++)
        {
            reference_set->getRow(t,neighbor_row);
            if(!store_prediction)
            {
                predictor->fprop(neighbor_row, F.toVec() & mu_temp & sn_temp);

                // N.B. this is the SVD of F'
                lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
                for (int k=0;k<ncomponents;k++)
                {
                    sm_temp[k] = mypow(S_svd[k],2);
                    U_temp(k) << Ut_svd(k);
                }
            }
            else
            {
                if(learn_mu)
                    mu_temp << mus(t);
                sn_temp[0] = sns[t];
                sm_temp << sms(t);
                U_temp << Fs[t];
            }

            if(learn_mu)
            {
                substract(t_row,neighbor_row,x_minus_neighbor);
                substract(x_minus_neighbor,mu_temp,z);
            }
            else
                substract(t_row,neighbor_row,z);

            mahal = -0.5*pownorm(z)/sn_temp[0];
            norm_term = - inputsize_/2.0 * Log2Pi - log_L 
                - 0.5*(inputsize_-ncomponents)*pl_log(sn_temp[0]);

            for(int k=0; k<ncomponents; k++)
            {
                mahal -= square(dot(z,U_temp(k)))*(0.5/(sm_temp[k]+sn_temp[0]) 
                                                   - 0.5/sn_temp[0]);
                norm_term -= 0.5*pl_log(sm_temp[k]+sn_temp[0]);
            }

            log_gauss[t] = mahal + norm_term;
        }
    }
    ret = logadd(log_gauss);

    return ret;
}


///////////////////
// computeOutput //
///////////////////
void NonLocalManifoldParzen::computeOutput(const Vec& input, Vec& output) const
{
    switch(outputs_def[0])
    {
        /*
    case 'r':
    {
        string fsave = "";
        VMat temp;
        real step_size = rw_size_step;
        real dp;
        t_row << input;
        Vec last_F(inputsize());
        for(int s=0; s<rw_n_step;s++)
        {
            if(s == 0)
            {
                predictor->fprop(t_row, F.toVec() & mu_temp & sn_temp);
                last_F << F(rw_ith_component);
            }
            predictor->fprop(t_row, F.toVec() & mu_temp & sn_temp);

            // N.B. this is the SVD of F'
            lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
            F(rw_ith_component) << Ut_svd(rw_ith_component);

            if(s % rw_save_every == 0)
            {
                fsave = rw_file_name + tostring(s) + ".amat";
                temp = new MemoryVMatrix(t_row.toMat(1,t_row.length()));
                temp->saveAMAT(fsave,false,true);
                //PLearn::save(fsave,t_row);
            }
            dp = dot(last_F,F(rw_ith_component));
            if(dp>0) dp = 1;
            else dp = -1;
            t_row += step_size*F(rw_ith_component)*abs(S_svd[rw_ith_component])*dp;
            last_F << dp*F(rw_ith_component);
        }
        output << t_row;

        t_row << input;
        for(int s=0; s<rw_n_step;s++)
        {
            if(s == 0)
            {
                predictor->fprop(t_row, F.toVec() & mu_temp & sn_temp);
                last_F << (-1.0)*F(rw_ith_component);
            }


            predictor->fprop(t_row, F.toVec() & mu_temp & sn_temp);

            // N.B. this is the SVD of F'
            lapackSVD(F, Ut_svd, S_svd, V_svd,'A',1.5);
            F(rw_ith_component) << Ut_svd(rw_ith_component);

            if(s % rw_save_every == 0)
            {
                fsave = rw_file_name + tostring(-s) + ".amat";
                temp = new MemoryVMatrix(t_row.toMat(1,t_row.length()));
                temp->saveAMAT(fsave,false,true);
                //PLearn::save(fsave,t_row);
            }
            dp = dot(last_F,F(rw_ith_component));
            if(dp>0) dp = 1;
            else dp = -1;
            t_row += step_size*F(rw_ith_component)*abs(S_svd[rw_ith_component])*dp;
            last_F << dp*F(rw_ith_component);
        }
        break;
    }
    case 't':
    {
        predictor->fprop(input, F.toVec() & mu_temp & sn_temp);
        output << F.toVec();
        break;
    }
        */
    default:
        
        inherited::computeOutput(input,output);
    }
}

////////////////
// outputsize //
////////////////
int NonLocalManifoldParzen::outputsize() const
{
    switch(outputs_def[0])
    {
        /*
    case 'm':
        return ncomponents;
        break;
    case 'r':
        return n;
    case 't':
        return ncomponents*n;
        */
    default:
        return inherited::outputsize();
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
