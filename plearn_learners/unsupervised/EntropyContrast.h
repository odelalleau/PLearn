// -*- C++ -*-

// EntropyContrast.h
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
 * $Id: EntropyContrast.h,v 1.3 2004/08/17 16:25:57 mariusmuja Exp $ 
 ******************************************************* */

/*! \file EntropyContrast.h */
#ifndef EntropyContrast_INC
#define EntropyContrast_INC
#define PROB_TABLE_LIMIT 10000000


#include <plearn_learners/generic/PLearner.h>

//Continuous includes

// Discrete includes
#define MY_PRECISION 0.0000000000001
#define MY_LOG_PRECISION 0.0001
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/VarRowVariable.h>
#include <plearn/math/random.h>

namespace PLearn {
  using namespace std;

  class EntropyContrast: public PLearner
  {

  private:
//COMMON
    typedef PLearner inherited;
    int n ; // an alias for the inputsize() 
    int evaluate_every_n_epochs; // number of epochs after which evaluation of constraints must be done
    bool evaluate_first_epoch;

    Mat w;  // weigths from hidden to constraints 
    Mat v;  // weigths from input to hidden

  
    Vec x ;
    Vec f_x ; // the constraints 
    Vec grad_C_real_wrt_f_x;

    Vec grad_C_extra_cost_wrt_f_x;

    Vec x_hat;
    Vec f_x_hat ;
    Vec grad_C_generated_wrt_f_x_hat; 

    VMat test_set;
    VMat validation_set;

    // Continuous fields

    real alpha ; // parameter used, when computing the running mu & sigma
                 // will be changed at every t

    int nhidden; //! the number of hidden units, in the one existing hidden layer

    Vec mu_f; // mean of the constraints , updated when a new f(x) is computed 
    Vec sigma_f ; // std dev of the constriants 
  
    Vec mu_f_hat ; //  mean of the constraints , updated when a new f(x_hat) is computed 
    Vec sigma_f_hat ; // std dev of the predicated constriants

    Vec mu_f_square ; 
    Vec sigma_f_square ;
    Mat grad_H_f_x_wrt_w;
    Mat grad_H_f_x_hat_wrt_w ;
    Mat grad_H_f_x_wrt_v;
    Mat grad_H_f_x_hat_wrt_v ;

  	Vec grad_H_f_x_wrt_bias_output ; 
  	Vec grad_H_f_x_wrt_bias_hidden ; 
    
  	Vec grad_H_f_x_hat_wrt_bias_output ; 
  	Vec grad_H_f_x_hat_wrt_bias_hidden ;
	
    Mat grad_H_g_wrt_w ; 
        
    Vec sigma_g ; 
    Vec mu_g ; 
    Vec g_x; 

    Vec bias_hidden ; 
    Vec z_x; // the hidden units when x is presented 
    
    Vec z_x_hat ; // the hidden_units when x_hat is presented

    
    Vec bias_output ; 

    Vec full_sum ; // used in the computation of grad_H_g_wrt_w ;

    real full ; 

    // constr_extra == derivative
    Mat df_dx ; 
    Mat grad_C_wrt_df_dx ; 
    Mat grad_extra_wrt_w, grad_extra_wrt_v ; 
    Vec grad_extra_wrt_bias_hidden , grad_extra_wrt_bias_output ; 
    
    // Discrete fields

    // Class fields

  public:
  
    // ************************
    // * public build options *
    // ************************

    string cost_real ; 
    string cost_gen ; 
    string cost_extra; 
    string gen_method;
    string evaluation_method;

    int nconstraints; //! The number of constraints  

    int inputsize ; // the number of samples in the input 

    real learning_rate ; //! the learning rate
    real decay_factor ; //! the decay factor of the learning rate  
    real weight_real , weight_gen, weight_extra ;
    real weight_decay_output;
    real weight_decay_hidden;
    int n_seen_examples;
    real starting_learning_rate;
    // saved options

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    EntropyContrast();
    // ******************
    // * PLearner methods *
    // ******************

  private: 
    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();
    string getInfo()
      {
        time_t tt;
        time(&tt);
        string time_str(ctime(&tt));
        vector<string> tokens = split(time_str);
        string info = tokens[3];
        info += "> ";
        return info;
      }

    // Continuous functions
    void initialize_NNcontinuous();
    void update_NNcontinuous();
    void computeNNcontinuous_hidden(const Vec& input_units,Vec &hidden_units);
    void computeNNcontinuous_constraints(Vec& hidden_units,Vec &output_units);
    void get_NNcontinuous_output(const Vec & x , Vec & f_x, Vec & z_x);
    void update_mu_sigma_f(const Vec &  f_x,Vec & mu,Vec & sigma) ;
    void update_alpha(int stage,int current_input_index) ; 
    void compute_diversity_cost(const Vec & f_x,const Vec & cost,Vec & grad_C_extra_cost_wrt_f_x );
    void get_real_grad_variance_wrt_f(const Vec & f_x, Vec & grad ) ; 
    void get_gen_grad_variance_wrt_f(const Vec & f_x, Vec & grad ) ; 
    void set_NNcontinuous_gradient(Vec &grad_C_real_wrt_f_x,Mat &grad_H_f_x_wrt_w, Mat &grad_H_f_x_wrt_v, Vec &z_x, Vec &x,
		Vec &grad_H_f_x_wrt_bias_output, Vec &grad_H_f_x_wrt_bias_hidden);
    void gen_normal_0_1(Vec &output) ;
    void set_NNcontinuous_gradient_from_extra_cost(Mat &grad_C_wrt_df_dx,const Vec &input );   // TODO:fill in the needed parameters
    void compute_df_dx(Mat &df_dx,const Vec & input) ; 
    void compute_extra_grad_wrt_df_dx(Mat &grad_C_wrt_df_dx) ; 
    void update_NNcontinuous_from_extra_cost() ; 
    // Discrete functions


  protected: 
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

  public:

    // ************************
    // **** Object methods ****
    // ************************

    //! simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

    // Declares other standard object methods
    //  If your class is not instantiatable (it has pure virtual methods)
    // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
    PLEARN_DECLARE_OBJECT(EntropyContrast);


    // **************************
    // **** PLearner methods ****
    // **************************


    //! returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options)
    // (PLEASE IMPLEMENT IN .cc)
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    // (PLEASE IMPLEMENT IN .cc)
    virtual void forget();

    
    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the train_stats collector with training costs measured on-line in the process.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void train();

//    virtual Vec test(VMat test_set, const string& save_test_outputs, const string& save_test_costs);

    //! Computes the output from the input
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Reconstructs an input from a (possibly partial) output (i.e. the first few princial components kept).
    void reconstruct(const Vec& output, Vec& input) const;
  
    //! Computes the costs from already computed output.
    //! The only computed cost is the squared_reconstruction_error
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;
                                

    //! Returns [ "squared_reconstruction_error" ]
    virtual TVec<string> getTestCostNames() const;


    //! No trian costs are computed for this learner
    virtual TVec<string> getTrainCostNames() const;


  };

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(EntropyContrast);
  
} // end of namespace PLearn

#endif

