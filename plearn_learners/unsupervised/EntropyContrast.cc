
// -*- C++ -*-

// EntropyContrast.cc
//
// Copyright (C) 2004  Dan Popovici 
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
 * $Id: EntropyContrast.cc,v 1.3 2004/08/16 19:45:08 dpopovici Exp $ 
 ******************************************************* */

/*! \file EntropyContrast.cc */
#include "EntropyContrast.h"
#include <plearn/vmat/VMat_maths.h>
//#include "TMat_maths.h"
#include <plearn/math/plapack.h>
#include <plearn/math/random.h>
namespace PLearn {
  using namespace std;

  EntropyContrast::EntropyContrast() 
    :nconstraints(4) //TODO: change to input_size 
  {
    learning_rate = 0.001;
    decay_factor = 0;
    weight_real = weight_gen = weight_extra = 1;
    nconstraints = 0 ; 
    n = 0 ; 
    evaluate_every_n_epochs = 1;
    evaluate_first_epoch = true;
    evaluation_method = "no_evaluation";
    // Continuous
    nhidden = 0 ;
    alpha = 0.0 ; 
  }

  PLEARN_IMPLEMENT_OBJECT(EntropyContrast, 
                          "Performs a EntropyContrast search", 
                          "Detailed Description ");

  void EntropyContrast::declareOptions(OptionList& ol)
  {
  
    declareOption(ol, "nconstraints", &EntropyContrast::nconstraints, OptionBase::buildoption,
                  "The number of constraints to create (that's also the outputsize)");
    declareOption(ol, "learning_rate", &EntropyContrast::learning_rate, OptionBase::buildoption,
                  "The learning rate of the algorithm");
    declareOption(ol, "decay_factor", &EntropyContrast::decay_factor, OptionBase::buildoption,
                  "The decay factor of the learning rate");
    
    declareOption(ol, "weight_decay_hidden", &EntropyContrast::weight_decay_hidden, OptionBase::buildoption,
                  "The decay factor for the hidden units");
    declareOption(ol, "weight_decay_output", &EntropyContrast::weight_decay_output, OptionBase::buildoption,
                  "The decay factor for the output units");

    declareOption(ol, "constr_type", &EntropyContrast::constr_type, OptionBase::buildoption,
                  "Type of the constraint");
    declareOption(ol, "cost_real", &EntropyContrast::cost_real, OptionBase::buildoption,
                  "The method to compute the real cost");
    declareOption(ol, "cost_gen", &EntropyContrast::cost_gen, OptionBase::buildoption,
                  "The method to compute the cost for the generated cost");
    declareOption(ol, "cost_extra", &EntropyContrast::cost_extra, OptionBase::buildoption,
                  "The method to compute the extra cost");
    declareOption(ol, "gen_method", &EntropyContrast::gen_method, OptionBase::buildoption,
                  "Method used to generate new points");
    declareOption(ol, "weight_real", &EntropyContrast::weight_real, OptionBase::buildoption,
                  "the relative weight of the cost of the real data, by default it is 1");
    declareOption(ol, "weight_gen", &EntropyContrast::weight_gen, OptionBase::buildoption,
                  "the relative weight of the cost of the generated data, by default it is 1");  
    declareOption(ol, "weight_extra", &EntropyContrast::weight_extra, OptionBase::buildoption,
                  "the relative weight of the extra cost, by default it is 1");  
    declareOption(ol, "evaluation_method", &EntropyContrast::evaluation_method, OptionBase::buildoption,
                  "Method for evaluation of constraint learning");  
    declareOption(ol, "evaluate_every_n_epochs", &EntropyContrast::evaluate_every_n_epochs, OptionBase::buildoption,
                  "Number of epochs after which the constraints evaluation is done");  
    declareOption(ol, "test_set", &EntropyContrast::test_set, OptionBase::buildoption,
                  "VMat test set");  
    // Continuous options
    declareOption(ol, "nhidden", &EntropyContrast::nhidden, OptionBase::buildoption,
                  "the number of hidden units");

    // Discrete options
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

// Functions for the continuous case

/////////////////////////////////////////////////
/// Initialize all the data structures for the NNet
/////////////////////////////////////////////////
  void EntropyContrast::initialize_NNcontinuous()
  {
    fill_random_uniform(w,-1.0,1.0) ; 
    fill_random_uniform(v,-1.0,1.0) ; 

    fill_random_uniform(bias_hidden,-1.0,1.0) ; 
    fill_random_uniform(bias_output,-1.0,1.0) ; 

    mu_f.fill(0.0) ; 
    sigma_f.fill(1.0) ;

    mu_f_hat.fill(0.0) ; 
    sigma_f_hat.fill(1.0) ; 

    // the extra_diversity constraint
    mu_g = 0.0 ; 
    sigma_g = 1.0 ; 
    sigma_g.fill(1.0) ; 
    mu_g.fill(0.0) ;

    mu_f_square.fill(0.0) ; 
    sigma_f_square.fill(1.0) ; 


    full = 1.0 ; 
  }

/////////////////////////////////////////////////
/// compute the hidden units of the NNet given the input
/////////////////////////////////////////////////
  void EntropyContrast::computeNNcontinuous_hidden(const Vec& input_units,Vec &hidden_units)
  {

    for (int i = 0 ; i < nhidden ; ++i )
    {
      hidden_units[i] = bias_hidden[i] ; 
      for (int j = 0 ; j < n ; ++j)
        hidden_units[i] += v(i,j) * input_units[j] ; 
    }
    compute_tanh(hidden_units,hidden_units) ; 

  }


/////////////////////////////////////////////////
/// Compute the output units given the hidden units
/////////////////////////////////////////////////
  void EntropyContrast::computeNNcontinuous_constraints(Vec& hidden_units,Vec &output_units)
  {
    product(output_units,w,hidden_units) ; 
  }
 
 
/////////////////////////////////////////////////
/// Compute the output units and also the hidden units given the input
/////////////////////////////////////////////////
  void EntropyContrast::get_NNcontinuous_output(const Vec & input_units,Vec &output_units,Vec &hidden_units)  
  {   
   
    computeNNcontinuous_hidden(input_units,hidden_units) ;   // compute the hidden units 
    
    computeNNcontinuous_constraints(hidden_units,output_units) ;   // compute the hidden units 
 
  }
 
/////////////////////////////////////////////////
/// Fill a vector with numbers from a gaussian having mu=0 , and sigma=1
/// This can be used for the generataion of the data 
/////////////////////////////////////////////////
  void EntropyContrast::gen_normal_0_1(Vec & output)
  {
    for (int i = 0 ; i < output.length() ; ++ i)  
      output[i] = gaussian_mu_sigma(0,1) ; 
  }


/////////////////////////////////////////////////
/// Given the output of the NNet it updates the running averages(mu, variance) 
/////////////////////////////////////////////////
  void EntropyContrast::update_mu_sigma_f(const Vec & f_x,Vec & mu, Vec &sigma) 
  {
    // :update mu_f_hat 
    mu = mu * alpha  + f_x * (1-alpha) ; 
    
    // :update sigma_f_hat
    sigma = alpha * (sigma) + (1-alpha) * square(f_x - mu)  ;

  }
    
/////////////////////////////////////////////////
/// Update the weight of a sample(alpha). It can range from 1/2, when the first sample is presented, to 1/inputsize if stage > 0 
/////////////////////////////////////////////////
  void EntropyContrast::update_alpha(int stage,int current_input_index)
  {
    
    if (stage==0)
      alpha = 1.0 - 1.0 / ( current_input_index + 2 )  ; 
    else
      alpha = 1.0 - 1.0/inputsize;
  }
 
/////////////////////////////////////////////////
///  Compute the cost and the gradiant for the diversity cost given by C_i = sum_j<i {cov((f_i)^2,(f_j)^2)}  
/////////////////////////////////////////////////
  void EntropyContrast::compute_diversity_cost(const Vec & f_x,const Vec & cost,Vec & grad_C_extra_cost_wrt_f_x)
  {     
    cost.fill (0.0);
    for (int i = 0; i < nconstraints; ++i)
    {
      for (int j = 0; j <= i; ++j)
        cost[i] += pow (f_x[j], 2);

      cost[i] /= i + 1;
    }
    Vec full_sum(nconstraints) ;
    full_sum[0] =  (pow(f_x[0],2) - (sigma_f[0] + pow(mu_f[0],2) ) ) ;
    for (int i = 1 ; i<nconstraints ; ++i)
    {
      full_sum[i] = full_sum[i-1] + (pow(f_x[i],2) - (sigma_f[i] + pow(mu_f[i],2) ) ) ;
      grad_C_extra_cost_wrt_f_x[i] = full_sum[i-1] * f_x[i] / train_set.length() ; 
    }
        
  }


  
/////////////////////////////////////////////////
/// Compute df/dx, given x(input)
/////////////////////////////////////////////////
  void EntropyContrast::compute_df_dx(Mat &df_dx,const Vec &input) {
    Vec ones(nhidden) ; 
    ones.fill(1) ; 
    Vec hidden(nhidden);
    hidden = product(v,input) ; 
    Vec diag(nhidden) ;
    diag = ones - square(tanh(hidden)) ; 
    diagonalizedFactorsProduct(df_dx,w,diag,v);
    
  }
  
/////////////////////////////////////////////////
/// Compute d Var(f_x) / df_x
/////////////////////////////////////////////////
  void EntropyContrast:: get_real_grad_variance_wrt_f(const Vec & f_x, Vec & grad ) 
  {
    for (int i = 0 ; i < f_x.length() ; ++i)   
    {
      grad[i] = (f_x[i] - mu_f[i]) / sigma_f[i] ; 
    }
  }
 
/////////////////////////////////////////////////
/// Compute d Var(f_x_hat) / df_x_hat
/////////////////////////////////////////////////
  void EntropyContrast:: get_gen_grad_variance_wrt_f(const Vec & f_x_hat, Vec & grad ) 
  {
    for (int i = 0 ; i < f_x_hat.length() ; ++i)   
    {
      grad[i] = (f_x_hat[i] - mu_f_hat[i]) / sigma_f_hat[i] ; 
    }
  }
 
/////////////////////////////////////////////////
/// Compute all the gradiants wrt to the parameters of the neural network
/////////////////////////////////////////////////
  void EntropyContrast::set_NNcontinuous_gradient(Vec &grad_C_real_wrt_f_x,Mat& grad_H_f_x_wrt_w, Mat& grad_H_f_x_wrt_v, 
                                                  Vec & hidden_units, Vec & input_units,  Vec &grad_H_f_x_wrt_bias_hidden, Vec &grad_H_f_x_wrt_bias_output)
  {
    // set the gradiant grad_H_f_x_wrt_w ; 

    for (int i = 0 ; i < nconstraints ; ++ i)
      for (int j = 0 ; j < nhidden ; ++j)
      {
        grad_H_f_x_wrt_w(i,j) = grad_C_real_wrt_f_x[i] * hidden_units[j] ; 
      }

    // set the gradiant grad_H_f_x_wrt_bias_z_output ; 
    for (int i = 0 ; i < nconstraints ; ++i)
      grad_H_f_x_wrt_bias_output[i] = grad_C_real_wrt_f_x[i] ;



    // set the gradiant grad_H_f_x_wrt_v ;  
    real sum; // keep sum v_i_k * x_k
    real grad_tmp ; // keep sum grad_C_wrt_f * grad_f_k_wrt_z
    for (int i = 0 ; i < nhidden ; ++ i)
    {
      sum = 0 ; 
      for (int k = 0 ; k < n ; ++ k)
        sum+=v(i,k) * input_units[k] ; 

      grad_tmp = 0; 
      for (int l = 0 ; l < nconstraints ; ++l)
        grad_tmp += grad_C_real_wrt_f_x[l] * w(l,i)  ; 

      for(int j=0 ; j<n ; ++j)   
        grad_H_f_x_wrt_v(i,j) = grad_tmp * (1 - tanh(bias_hidden[i] + sum) * tanh(bias_hidden[i] + sum)) * input_units[j];

      grad_H_f_x_wrt_bias_hidden[i] = grad_tmp * (1 - tanh(bias_hidden[i] + sum) * tanh(bias_hidden[i] + sum));

    }
  }
/////////////////////////////////////////////////
/// update the parameters of the NNet from the extra cost given by the derivative(angle) cost  
/////////////////////////////////////////////////
  void EntropyContrast::update_NNcontinuous_from_extra_cost()
  {
//TODO: maybe change the learning_rate used for the extra_cost  
    Mat grad_extra_wrt_w, grad_extra_wrt_v ; 
    Vec grad_extra_wrt_bias_hidden , grad_extra_wrt_bias_output ; 

    for (int i = 0 ; i < nhidden ; ++i) 
      for(int j = 0 ; j < n ; ++ j) 
        v(i,j)-= learning_rate * grad_extra_wrt_v(i,j)  ;

    for (int i = 0 ; i < nconstraints ; ++i) 
      for(int j = 0 ; j < nhidden ; ++ j) 
        w(i,j)-= learning_rate * grad_extra_wrt_w(i,j)  ;

    for(int j = 0 ; j < nhidden ; ++ j) 
      bias_hidden[j] -= learning_rate * grad_extra_wrt_bias_hidden[j] ; 

    for(int j = 0 ; j < nconstraints ; ++ j) 
      bias_output[j] -= learning_rate * grad_extra_wrt_bias_output[j]; 
    
  }
/////////////////////////////////////////////////
/// update the parameters of the NNet from the regular cost
/////////////////////////////////////////////////
  void EntropyContrast::update_NNcontinuous()
  {
    for (int i = 0 ; i < nhidden ; ++i) 
      for(int j = 0 ; j < n ; ++ j) 
        v(i,j)-= learning_rate * (grad_H_f_x_wrt_v(i,j) - grad_H_f_x_hat_wrt_v(i,j)) + weight_decay_hidden * v(i,j)  ;

    for (int i = 0 ; i < nconstraints ; ++i) 
      for(int j = 0 ; j < nhidden ; ++ j) 
        w(i,j)-= learning_rate * (grad_H_f_x_wrt_w(i,j) - grad_H_f_x_hat_wrt_w(i,j)) + weight_decay_output * w(i,j)  ;

    for(int j = 0 ; j < nhidden ; ++ j) 
      bias_hidden[j] -= learning_rate * (grad_H_f_x_wrt_bias_hidden[j] - grad_H_f_x_hat_wrt_bias_hidden[j] ); 

    for(int j = 0 ; j < nconstraints ; ++ j) 
      bias_output[j] -= learning_rate * (grad_H_f_x_wrt_bias_output[j] - grad_H_f_x_hat_wrt_bias_output[j] ); 
  } 

/////////////////////////////////////////////////
/// compute the grad extra_cost wrt df_dx
/////////////////////////////////////////////////
  void EntropyContrast::compute_extra_grad_wrt_df_dx(Mat & grad_C_wrt_df_dx)
  {
    for(int i=0;i< n;i++){
      grad_C_wrt_df_dx[0][i] = 0.0 ; 
    }
    

    // compute dot product g_i , g_j
    Mat dot_g(nconstraints,nconstraints) ; 
    for (int i=0; i<nconstraints ;++i)
      for (int j=0; j<i ; ++j)
        dot_g(i,j) = dot(df_dx(i),df_dx(j)) ; 
    
    for (int j = 1; j<nconstraints; ++j )
    {
      for (int k = 0; k<n ; ++k)
      {
      for (int i = 0; i < j ; ++i)
        grad_C_wrt_df_dx[j][k] += 2 * dot_g(j,i) * df_dx(i,k) ; 
      }
    }

    
  }

/////////////////////////////////////////////////
/// do the bprop step for NNet, compute all the gradiants
/////////////////////////////////////////////////
  void EntropyContrast::set_NNcontinuous_gradient_from_extra_cost(Mat &grad_C_wrt_df_dx,const Vec &input) 
  {

    //compute a = 1 - tanh^2(v * x)
    //        b = 1 - tanh( v * x ) ; 
    Vec ones(nhidden) ; 
    Vec b(nhidden) ; 
    ones.fill(1) ; 
    Vec hidden(nhidden);
    hidden = product(v,input) ; 
    Vec diag(nhidden) ;
    diag = ones - square(tanh(hidden)) ; 

    b = ones - tanh(hidden) ; 
    
    Mat a(nhidden,nhidden) ; 
    a.fill(0.0) ; 
    addToDiagonal(a,diag) ; 
    // compute dC / dw = dC/dg * v' * a
    productTranspose(grad_extra_wrt_w,grad_C_wrt_df_dx,v) ; 
    product(grad_extra_wrt_w,grad_extra_wrt_w,a) ;         

    // compute dC/dv = a * w' * dC/dg -2 * (dC/da * b * a) x' ; 
    {
    Mat tmp(nhidden,nconstraints) ; 
    product(tmp,a,transpose(w)) ; 
    product(grad_extra_wrt_v,tmp,grad_C_wrt_df_dx) ; 
    }
    
    // compute dC/da
    {
    Vec grad_C_wrt_a ; 
    Mat tmp(nhidden,n) ; 
    product(tmp,transpose(w),grad_C_wrt_df_dx) ; 
    Mat tmp_a(nhidden,nhidden) ; 
    product(tmp_a,tmp,transpose(v)) ;
    
//    grad_extra_wrt_v += (-2 * diag * b * diag(tmp_a) ) * transpose(input) ; 
    Vec temp(nhidden) ; 
    for (int i= 0 ; i < nhidden ; ++i)
    {
      temp[i] = tmp_a(i,i) * b[i] * a(i,i) ; 
    
      for (int j = 0 ; j < n ; ++j)
      {
        grad_extra_wrt_v(i,j) +=  temp[i] * input[j] ;
      }
    }
    
    }
    //FIXME : add update to bias 
        
  }
  
  

  void EntropyContrast::build_()
  {
    if (!train_set.isNull()) 
    {
      n = train_set->width() ; // setting the input dimension

      inputsize = train_set->length() ; // set the number of training inputs
      
      x.resize(n) ; // the current input sample, presented 

      f_x.resize(nconstraints) ; // the constraints on the real sample 
           
      grad_C_real_wrt_f_x.resize(nconstraints); // the gradient of the real cost wrt to the constraints
 
      x_hat.resize(n) ; // the current generated sample

      f_x_hat.resize(nconstraints) ;  // the constraints on the generated sample
      
      grad_C_generated_wrt_f_x_hat.resize(nconstraints); // the gradient of the generated cost wrt to the constraints

      grad_C_extra_cost_wrt_f_x.resize(nconstraints);

      starting_learning_rate = learning_rate;

      n_seen_examples = 0;

      // Continuous

      w.resize(nconstraints,nhidden)   ; // setting the size of the weights between the hidden layer and the output(the constraints) 

      z_x.resize(nhidden) ; // set the size of the hidden units
      z_x_hat.resize(nhidden) ; // set the size of the hidden units     

      v.resize(nhidden,n) ; // set the size of the weights between the hidden input and the hidden units      

      mu_f.resize(nconstraints)  ; // the average of the constraints over time, used in the computation on certain gradiants

      mu_f_hat.resize(nconstraints)  ; // the average of the constraints over time, used in the computation on certain gradiants

      sigma_f.resize(nconstraints) ; // the variance of the constraints over time,, sued in the computation on certain gradiants

      sigma_f_hat.resize(nconstraints) ;//the variance of the constraints over time,, sued in the computation on certain gradiants

      mu_f_square.resize(nconstraints)  ; 
      sigma_f_square.resize(nconstraints)  ;  

      bias_hidden.resize(nhidden) ; 
      bias_output.resize(nconstraints); 

      grad_H_f_x_wrt_bias_output.resize(nconstraints) ; 
      grad_H_f_x_wrt_bias_hidden.resize(nhidden) ;

      grad_H_f_x_hat_wrt_bias_output.resize(nconstraints) ; 
      grad_H_f_x_hat_wrt_bias_hidden.resize(nhidden) ;

      grad_H_f_x_hat_wrt_w.resize(nconstraints,nhidden); 
      grad_H_f_x_wrt_w.resize(nconstraints,nhidden) ; 

      grad_H_g_wrt_w.resize(nconstraints,nhidden) ;


      grad_H_f_x_wrt_v.resize(nhidden,n) ; 
      grad_H_f_x_hat_wrt_v.resize(nhidden,n) ; 

      // used for the computation of the extra diversity constraints
      sigma_g.resize(nconstraints) ; 
      mu_g.resize(nconstraints) ; 
      g_x.resize(nconstraints) ;  
      grad_C_wrt_df_dx.resize(nconstraints,n) ; 
      df_dx.resize(nconstraints,n) ; 
      
      grad_extra_wrt_w.resize(nconstraints, nhidden) ; 
      grad_extra_wrt_v.resize(nhidden, n) ; 
   
      full_sum.resize(nconstraints) ;
    }

  }

  // ### Nothing to add here, simply calls build_
  void EntropyContrast::build()
  {
    inherited::build();
    build_();
  }


  void EntropyContrast::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    inherited::makeDeepCopyFromShallowCopy(copies);
//    deepCopyField(eigenvecs, copies);
  }

            
    
  int EntropyContrast::outputsize() const
  {
    return nconstraints;
  }

  void EntropyContrast::forget()
  {
    // Initialization

    // continuous
    if(constr_type == "NNcontinuous")
      initialize_NNcontinuous() ; 
    ;
  }
    
  void EntropyContrast::train()
  {
    int t ; 
    manual_seed(12345678);
    forget();    
    real partial ; // temporary 
    for (;stage < nstages;stage++)
    {
      cout << getInfo() << endl;
      cout << "Stage = " << stage << endl;
      cout << "Learning rate = " << learning_rate << endl;

      for (t = 0 ; t < train_set.length(); ++ t)  
      {
        partial = 0.0 ; 
        if (constr_type == "NNcontinuous") 
          update_alpha(stage,t) ; // used in the update of the running averages 
   
        train_set->getRow(t,x);

        //////////////////////
        // Real data section
        //////////////////////
        
        // Get constraint output for real data (fill the f_x field)

        // constinuous
        if(constr_type == "NNcontinuous")          
        {
          get_NNcontinuous_output(x,f_x,z_x) ; // this also computes the value of the hidden units , which will be needed when we compute all the gradiants 
         update_mu_sigma_f(f_x,mu_f,sigma_f) ; 

          if (cost_real == "constraint_variance")
            update_mu_sigma_f(square(f_x),mu_f_square,sigma_f_square) ; 
        }                                  

        // Get gradient for cost function for real data (fill grad_C_real_wrt_f_x)

        // continuous
        if(cost_real == "constraint_variance")
        {
          // compute gradiant of the cost wrt to f_x
          get_real_grad_variance_wrt_f(f_x,grad_C_real_wrt_f_x) ; 
        }
        ;

        // Adjust weight of the gradient

        for(int it=0; it<grad_C_real_wrt_f_x.length(); it++)
          grad_C_real_wrt_f_x[it] *= weight_real;

        ////////////////////////
        // Extra cost function
        ////////////////////////

        // continuous
        if(cost_extra == "variance_sum_square_constraints")
            compute_diversity_cost(f_x,g_x,grad_C_extra_cost_wrt_f_x) ; // this also computes the gradiant extra_cost wrt to the constrains f_i(x) grad_C_extra_cost_wrt_f_x
        if(cost_extra == "derivative")
        {
           compute_df_dx(df_dx,x) ; 
           compute_extra_grad_wrt_df_dx(grad_C_wrt_df_dx) ; 
        }

        // discrete


        for(int it=0; it<grad_C_generated_wrt_f_x_hat.length(); it++)
            grad_C_extra_cost_wrt_f_x[it] *= weight_extra;


        // Set gradient for the constraint using real data

        // continuous
        if(constr_type == "NNcontinuous")
        {
            // set the gradiant of the cost wrt to the weights w,v and to the bias
            set_NNcontinuous_gradient(grad_C_real_wrt_f_x,grad_H_f_x_wrt_w,grad_H_f_x_wrt_v,z_x,x,
                    grad_H_f_x_wrt_bias_hidden,grad_H_f_x_wrt_bias_output);
            if (cost_extra == "derivative"){
            set_NNcontinuous_gradient_from_extra_cost(grad_C_wrt_df_dx,x) ;   
            }
        }
        if(cost_extra == "variance_sum_square_constraints"){
            // combine the grad_real & grad_extra
            for(int it=0; it<grad_C_real_wrt_f_x.length(); it++)
                grad_C_real_wrt_f_x[it] += grad_C_extra_cost_wrt_f_x[it];
        }        
        // discrete

        ///////////////////////////
        // Generated data section
        ///////////////////////////

        // Generate a new point (fill x_hat)
      
        // continuous
        if(gen_method == "N(0,1)")
          gen_normal_0_1(x_hat) ; 
        ;
        // Get constraint output from generated data (fill the f_x_hat field)

        // continuous
        if(constr_type == "NNcontinuous")
          get_NNcontinuous_output(x_hat,f_x_hat,z_x_hat) ;
          update_mu_sigma_f(f_x_hat,mu_f_hat,sigma_f_hat) ; 
        ;
        // Get gradient for cost function for generated data (fill grad_C_generated_wrt_f_x_hat)

        // continuous
        if(cost_gen == "constraint_variance")
          get_gen_grad_variance_wrt_f(f_x_hat,grad_C_generated_wrt_f_x_hat) ; 
        // discrete
        
        // Adjust weight of the gradient
        

        for(int it=0; it<grad_C_generated_wrt_f_x_hat.length(); it++)
          grad_C_generated_wrt_f_x_hat[it] *= weight_gen;

        // Set gradient for the constraint using generated data
      
        // continuous
        if(constr_type == "NNcontinuous")
          set_NNcontinuous_gradient(grad_C_generated_wrt_f_x_hat,grad_H_f_x_hat_wrt_w,grad_H_f_x_hat_wrt_v,z_x_hat,x_hat,
                                    grad_H_f_x_hat_wrt_bias_hidden,grad_H_f_x_hat_wrt_bias_output);       
        // discrete

        ///////////
        // Update
        ///////////


        // continuous
        if(constr_type == "NNcontinuous")
          update_NNcontinuous();
        ;
        n_seen_examples++;

        full = alpha *  full  + (1-alpha) * (f_x[0] * f_x[0] - (sigma_f[0] + mu_f[0]*mu_f[0])) * (f_x[1] * f_x[1] - (sigma_f[1] + mu_f[1]*mu_f[1]) ) ;
      }

      learning_rate = starting_learning_rate / (1 + decay_factor*n_seen_examples);
      
      /////////////////////
      // Train evaluation
      /////////////////////


      if(stage % evaluate_every_n_epochs == 0 && !(!evaluate_first_epoch && stage == 0)) 
      {
        // continuous
        if(evaluation_method == "dump_all") 
        {
          if (n_seen_examples == 250000)
          {
            FILE * f1 = fopen("gen1.dat","wt") ;
            FILE * f2 = fopen("gen2.dat","wt") ;
            
            for (int i = -10 ; i <= 10 ; ++ i)
              for (int j = -1 ; j <= 9 ; ++ j )
                for (int k = -1 ; k <= 9 ; ++ k )
              {
                Mat res(2,3) ; 
                Vec input(3) ;
                Vec ones(nhidden) ; 
                ones.fill(1) ; 
                input[0] = (real)i / 10 ; 
                input[1] = (real)j / 10 ;
                input[2] = (real)k / 100 ; 
                Vec hidden(nhidden);
                hidden = product(v,input) ; 
                Vec diag(nhidden) ;
                diag = ones - square(tanh(hidden)) ; 
                diagonalizedFactorsProduct(res,w,diag,v); 
                fprintf(f1,"%f %f %f %f %f %f\n",(real)i/10,(real)j/10,(real)k/100,res(0,0),res(0,1),res(0,2)) ;                 
//              fprintf(f2,"%f %f %f %f %f %f\n",(real)i/10,(real)j/10,(real)k/100,res(1,0),res(1,1),res(1,2)) ; 
                real norm0 = sqrt(res(0,0)*res(0,0)+res(0,1)*res(0,1)+res(0,2)*res(0,2)) ; 
                real norm1 = sqrt(res(1,0)*res(1,0)+res(1,1)*res(1,1)+res(1,2)*res(1,2)) ; 
                real angle = res(0,0) / norm0 * res(1,0) / norm1 + res(0,1) / norm0 * res(1,1) / norm1 + res(0,2) / norm0 * res(1,2) / norm1 ;  
                fprintf(f2,"%f %f %f %f\n",(real)i/10,(real)j/10,(real)k/100,angle) ; 
                //                fprintf(f2,"%f %f %f %f\n",(real)i/10,(real)j/10,res(1,0),res(1,1)) ; 
              }
            fclose(f1) ; 
            fclose(f2) ; 
/*            FILE * f3 = fopen("gen3.dat","wt");
            FILE * f4 = fopen("gen4.dat","wt");
            real eps = 0.001 ; 
            for (int j = 90 ; j >= -10 ; --j)
            {
              for (int i = -100 ; i <= 100 ; ++ i )
              {
                bool close = false ; 
                for (int k = 0 ; k < train_set.length(); ++k)
                {
                  if (pow(train_set->get(k,0) - (real)i/100,2) + pow(train_set->get(k,1) - real(j)/100,2) < eps )
                  {
                    close = true ; 
                    break ; 
                  }
                }
                if (close)
                  fprintf(f4,"%f ",1.0) ;
                else
                {
                  fprintf(f4,"%f ",0.0) ;
                }              

                Vec input(n) ; 
                input[0] = (real)i/100 ;
                input[1] = (real)j/100 ; 
                Vec hidden(nhidden) ; 
                Vec output(nconstraints) ;
                computeNNcontinuous_hidden(input,hidden) ;
                computeNNcontinuous_constraints(hidden,output) ;
                fprintf(f3,"%f ",output[0]) ; 
              }
              fprintf(f3,"\n") ; 
              fprintf(f4,"\n") ; 
            }

            fclose(f3) ; 
            fclose(f4) ; 
            // generate data that will be used to create a colormap
*/
            
            exit(0) ; 
          }
          for (int i = 0 ; i < f_x.length() ;++i)
            cout << f_x[i] << " ";
          cout << endl << "cov = " << full/train_set.length() << endl ; 
          cout << "var f_square: " << sigma_f_square[0] << " "<< sigma_f_square[1] <<  endl ; 
          cout << "corr: " << full / sqrt(sigma_f_square[0] / sqrt(sigma_f_square[1])) << endl ; 
        }
        // discrete
      }
      cout << "--------------------------------" << endl;
    }
  }
  void EntropyContrast::computeOutput(const Vec& input, Vec& output) const
  {
  }    


  void EntropyContrast::reconstruct(const Vec& output, Vec& input) const
  {
  }

  void EntropyContrast::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                const Vec& target, Vec& costs) const
  {
  }                                

  TVec<string> EntropyContrast::getTestCostNames() const
  {
    return TVec<string>(1,"squared_reconstruction_error");
  }

  TVec<string> EntropyContrast::getTrainCostNames() const
  {
    return TVec<string>();
  }



} // end of namespace PLearn
