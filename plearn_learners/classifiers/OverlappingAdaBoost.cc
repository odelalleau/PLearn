// -*- C++ -*-

// OverlappingAdaBoost.cc
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

// Authors: Hugo Larochelle

/*! \file OverlappingAdaBoost.cc */

#include "OverlappingAdaBoost.h"
#include <plearn/math/pl_math.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/PLearnerOutputVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/vmat/SelectRowsVMatrix.h>
#include <plearn/math/random.h>
#include <plearn/io/load_and_save.h>
#include <plearn/base/stringutils.h>
#include <plearn/base/tostring.h>
#include <plearn/vmat/RepeatVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/vmat/ConcatRowsVMatrix.h>

namespace PLearn {
using namespace std;

OverlappingAdaBoost::OverlappingAdaBoost()

    : target_error(0.5), power_weights(1), output_threshold(0.5), compute_training_error(1), boosting_type("AdaBoost"),
      //weight_by_resampling(1), 
      early_stopping(1),
      save_often(0), penalty_coefficient(0), nclasses(2), retrain_after_n_epochs(-1), ordinary_boosting(false), scale_using_max_weight(true),
      adjust_training_set_size(0), train_subset(-1)
{ }

PLEARN_IMPLEMENT_OBJECT(

    OverlappingAdaBoost,
    "OverlappingAdaBoost boosting algorithm for binaray or multi-class and multi-target classification",
    "Weak learners are assumed to output a value in [0,1]");

void OverlappingAdaBoost::declareOptions(OptionList& ol)
{
    declareOption(ol, "weak_learners", &OverlappingAdaBoost::weak_learners,
                  OptionBase::learntoption,
                  "The vector of learned weak learners");


    declareOption(ol, "weak_learners_output", &OverlappingAdaBoost::weak_learners_output,
                  OptionBase::learntoption,
                  "The VMat of the output of learned weak learners on the training set");

    declareOption(ol, "voting_weights", &OverlappingAdaBoost::voting_weights,
                  OptionBase::learntoption,
                  "Weights given to the weak learners (their output is linearly combined with these weights\n"
                  "to form the output of the OverlappingAdaBoost learner).\n");

    declareOption(ol, "sum_voting_weights", &OverlappingAdaBoost::sum_voting_weights,
                  OptionBase::learntoption,
                  "Sum of the weak learners voting weights.\n");

    declareOption(ol, "initial_sum_weights", &OverlappingAdaBoost::initial_sum_weights,
                  OptionBase::learntoption,
                  "Initial sum of weights on the examples. Do not temper with.\n");

    declareOption(ol, "weak_learner_template", &OverlappingAdaBoost::weak_learner_template,
                  OptionBase::buildoption,
                  "Template for the regression weak learner to be boosted into a classifier");

    declareOption(ol, "target_error", &OverlappingAdaBoost::target_error,
                  OptionBase::buildoption,
                  "This is the target average weighted error below which each weak learner"
                  "must reach after its training (ordinary adaboost: target_error=0.5).");


    declareOption(ol, "power_weights", &OverlappingAdaBoost::power_weights,
                  OptionBase::buildoption,
                  "Power to apply on the weights to sharpen (>1) or smooth (<1) the weights distribution.");


    declareOption(ol, "boosting_type", &OverlappingAdaBoost::boosting_type,
                  OptionBase::buildoption,
                  "Type of boosting to use. Choose among:\n"
                  " - AdaBoost (supposes weak learners only classify, i.e. output in {0,1})\n"
                  " - ConfidenceBoost (weak learners also give confidence in classification, i.e. output in [0,1]\n");
    declareOption(ol, "ordinary_boosting", &OverlappingAdaBoost::ordinary_boosting,
                  OptionBase::buildoption,
                  "Indication that ordinary boosting should be used.\n");

    declareOption(ol, "scale_using_max_weight", &OverlappingAdaBoost::scale_using_max_weight,
                  OptionBase::buildoption,
                  "Indication that the weights should be scaled by dividing by\n"
                  "the maximum weight value.\n");

    declareOption(ol, "adjust_training_set_size", &OverlappingAdaBoost::adjust_training_set_size,
                  OptionBase::buildoption,
                  "Indication that the training set size should be adjusted\n"
                  "based on the weight distribution. This is useful\n"
                  "for weak learners using gradient descent optimization.\n");

    declareOption(ol, "train_subset", &OverlappingAdaBoost::train_subset,
                  OptionBase::buildoption,
                  "Fraction of the original training set to really be used for training\n"
                  "(the other samples are assumed to be used for validation by the user).\n"
                  "If <= 0, than no validation set is assumed.\n"
                  "If > 1, than it is assumed to be the number of samples\n" 
                  "in the training set, instead of the fraction of samples.\n");

  
/*  declareOption(ol, "weight_by_resampling", &OverlappingAdaBoost::weight_by_resampling,
    OptionBase::buildoption,
    "Whether to train the weak learner using resampling to represent the weighting\n"
    "given to examples. If false then give these weights explicitly in the training set\n"
    "of the weak learner (note that some learners can accomodate weights well, others not).\n");
*/
    declareOption(ol, "output_threshold", &OverlappingAdaBoost::output_threshold,
                  OptionBase::buildoption,
                  "To interpret the output of the learner as a class, it is compared to this\n"
                  "threshold: class 1 if greather than output_threshold, class 0 otherwise.\n");

    declareOption(ol, "provide_learner_expdir", &OverlappingAdaBoost::provide_learner_expdir, OptionBase::buildoption,
                  "If true, each weak learner to be trained will have its experiment directory set to WeakLearner#kExpdir/");

    declareOption(ol, "early_stopping", &OverlappingAdaBoost::early_stopping, OptionBase::buildoption,
                  "If true, then boosting stops when the next weak learner is too weak (avg error > target_error - .01)\n");

    declareOption(ol, "save_often", &OverlappingAdaBoost::save_often, OptionBase::buildoption,
                  "If true, then save the model after training each weak learner, under <expdir>/model.psave\n");
  
    declareOption(ol, "penalty_coefficient", &OverlappingAdaBoost::penalty_coefficient, OptionBase::buildoption,
                  "Penalty coefficient for regularized boosting (should be >= 0)\n");

    declareOption(ol, "compute_training_error", &OverlappingAdaBoost::compute_training_error, OptionBase::buildoption,
                  "Whether to compute training error at each stage.\n");


    declareOption(ol, "retrain_after_n_epochs", &OverlappingAdaBoost::retrain_after_n_epochs, OptionBase::buildoption,
                  "Number of epochs after which boosting retrains new weak learners.\n"
                  "The new weaklearners still have access to the output of the old ones.\n"
                  "If <= 0, then no retraining will be done.\n");

    declareOption(ol, "classes_to_discriminate", &OverlappingAdaBoost::classes_to_discriminate, OptionBase::buildoption,
                  "Classes that we really want to discriminate. The statistic \"subset_class_error\"\n"
                  "gives the class error when only those classes are considered as correct output.\n");

    declareOption(ol, "nclasses", &OverlappingAdaBoost::nclasses, OptionBase::buildoption,
                  "Number of classes.\n");
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void OverlappingAdaBoost::build_()
{

    if(penalty_coefficient < 0)
        PLERROR("Penalty coefficient should not be negative");
}

// ### Nothing to add here, simply calls build_
void OverlappingAdaBoost::build()
{
    inherited::build();
    build_();
}


void OverlappingAdaBoost::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(learners_error, copies);
    deepCopyField(example_weights, copies);
    deepCopyField(voting_weights, copies);
    deepCopyField(weak_learners, copies);
    deepCopyField(weak_learner_template, copies);
    deepCopyField(weak_learners_output,copies);
    deepCopyField(classes_to_discriminate,copies);
    deepCopyField(initial_weights,copies);
}


int OverlappingAdaBoost::outputsize() const
{
    return nclasses == 2 ? 1 : nclasses;
}

void OverlappingAdaBoost::forget()
{
    stage = 0;
    learners_error.resize(0, nstages);
    weak_learners.resize(0, nstages);
    voting_weights.resize(0, nstages);
    weak_learners_output.resize(0,nstages);
    sum_voting_weights = 0;
    if (seed_ >= 0)
        manual_seed(seed_);
    else
        PLearn::seed();
}

void OverlappingAdaBoost::train()
{
    if(!train_set)
        PLERROR("In OverlappingAdaBoost::train, you did not setTrainingSet");
    
    if(!train_stats)
        PLERROR("In OverlappingAdaBoost::train, you did not setTrainStatsCollector");

    if (train_set->targetsize()!=nclasses)
        PLERROR("In OverlappingAdaBoost::train, targetsize should be %d, found %d", nclasses, train_set->targetsize());


    //if (nstages < stage)        //!< Asking to revert to previous stage
    //  forget();


    static Vec input;
    static Vec output;
    static Vec target;
    real weight;
    Vec theta;
    real max_weight;
    real sum_of_square_weights;

    static Mat examples_error;
    //static TVec<int> train_indices;
    //static Vec pseudo_loss;


    input.resize(inputsize());
    target.resize(targetsize());



    if (stage==0)
    {
        example_weights.resize(train_set->length(),outputsize());
        initial_weights.resize(train_set->length(),outputsize());
        if (train_set->weightsize()>0)
        {

            ProgressBar *pb=0;
            if(report_progress) pb = new ProgressBar("OverlappingAdaBoost round " + tostring(stage) +
                                                     ": extracting initial weights", train_set->length());
            initial_sum_weights=0;
            for (int i=0; i<train_set->length(); ++i) {
                if(report_progress) pb->update(i);
                train_set->getExample(i, input, target, weight);
                example_weights(i).fill(weight);
                initial_sum_weights += outputsize()*weight;
            }
            example_weights *= real(1.0)/initial_sum_weights;
            if(report_progress) delete(pb);      
        }

        else 
        {
            example_weights.fill(1.0/(train_set->length()*outputsize()));
            initial_sum_weights = 1;
        }
        sum_voting_weights = 0;
        voting_weights.resize(0,nstages);
        initial_weights << example_weights;
    }


    examples_error.resize(train_set->length(),outputsize());
    output.resize(outputsize());


    VMat unweighted_data = train_set.subMatColumns(0, inputsize());
    VMat target_data = train_set.subMatColumns(inputsize(),outputsize());
    //learners_error.resize(nstages);
    //weak_learners_output.resize(nstages);


    int n;
    for ( ; stage < nstages ; ++stage)
    {
        if(retrain_after_n_epochs > 0 && (stage)%retrain_after_n_epochs == 0)
        {
            example_weights << initial_weights;
            voting_weights.fill(0);
            sum_voting_weights=0;
        }

        max_weight = max(example_weights.toVec());
        sum_of_square_weights = sum(example_weights.toVec());
        VMat weak_learner_training_set;
        VMat weak_learner_validation_set;
        {

            ProgressBar *pb=0;
            if(report_progress)  pb = new ProgressBar("OverlappingAdaBoost round " + tostring(stage) +
                                                      ": making training set for weak learner", n);
            Array<VMat> temp_columns(1);
            // We shall now construct a training set for the new weak learner:
            /*
              if (weight_by_resampling)
              {
              if(nclasses != 2)
              PLERROR("In OverlappingAdaboost::train(): weight by resampling not implemented for multiclass classification");
              // use a "smart" resampling that approximated sampling with replacement
              // with the probabilities given by example_weights.
              map<real,int> indices;
              for (int i=0; i<n; ++i) {
              pb->update(i);
              real p_i = example_weights(i,0);
              int n_samples_of_row_i = int(rint(gaussian_mu_sigma(n*p_i,sqrt(n*p_i*(1-p_i))))); // randomly choose how many repeats of example i
              for (int j=0;j<n_samples_of_row_i;j++)
              {
              if (j==0)
              indices[i]=i;
              else
              {
              Mat data_weights_column = example_weights.copy();
              data_weights_column *= initial_sum_weights; // to bring the weights to the same average level as the original ones
              VMat data_weights = VMat(data_weights_column);
              temp_columns = unweighted_data;
              for(int i=0; i<stage; i++)
              temp_columns = weak_learners_output[i] & temp_columns;
              expended_data = new ConcatColumnsVMatrix(temp_columns);
              expended_data->defineSizes(inputsize()+stage,1,0);
              temp_columns = temp_columns & data_weights;
              weak_learner_training_set = new ConcatColumnsVMatrix(temp_columns);
              weak_learner_training_set->defineSizes(inputsize()+example_weights.width()*stage, 1, example_weights.width());
              }
              }

              if(report_progress) delete(pb);
              train_indices.resize(0,n);
              map<real,int>::iterator it = indices.begin();
              map<real,int>::iterator last = indices.end();
              for (;it!=last;++it)
              train_indices.push_back(it->second);
              temp_columns[0] = unweighted_data;
              for(int i=0; i<stage; i++)
              temp_columns = weak_learners_output[i] & temp_columns;
              expended_data = new ConcatColumnsVMatrix(temp_columns);
              expended_data->defineSizes(inputsize()+stage, 1, 0);
              weak_learner_training_set = new SelectRowsVMatrix( expended_data, train_indices);
              weak_learner_training_set->defineSizes(inputsize()+stage, 1, 0);
        
              }
              else
              {
            */
            Mat data_weights_column = example_weights.copy();
            if(power_weights != 1)
            {
                real temp_sum = 0;
                for(int i=0; i<data_weights_column.length(); i++)
                    for(int j=0; j<data_weights_column.width(); j++)
                    {
                        data_weights_column(i,j) = pow(data_weights_column(i,j),power_weights);
                        temp_sum += data_weights_column(i,j);
                    }
                data_weights_column /= temp_sum;
            }
       
            if(scale_using_max_weight)
                data_weights_column *= initial_sum_weights/max_weight; // to bring the weights to the same average level as the original ones
            else
                data_weights_column *= initial_sum_weights; // to bring the weights to the same average level as the original ones
            VMat data_weights = VMat(data_weights_column);
            temp_columns[0] = unweighted_data;
            if(!ordinary_boosting)
            {
                for(int i=0; i<stage; i++)
                    temp_columns.push_back(weak_learners_output[i]);
            }
            if(nclasses != 2)
            {
                temp_columns.push_back(data_weights);
                temp_columns.push_back(target_data);
            }
            else
            {
                temp_columns.push_back(target_data);
                temp_columns.push_back(data_weights);
            }
            weak_learner_training_set = new ConcatColumnsVMatrix(temp_columns);
            if(ordinary_boosting)
                weak_learner_training_set->defineSizes(inputsize()+(nclasses != 2 ? outputsize() : 0), outputsize(), (nclasses != 2 ? 0 : 1));
            else
                weak_learner_training_set->defineSizes(inputsize()+outputsize()*stage+(nclasses != 2 ? outputsize() : 0), outputsize(), (nclasses != 2 ? 0 : 1));
            //}
        }


        // Create new weak-learner and train it
        PP<PLearner> new_weak_learner = ::PLearn::deepCopy(weak_learner_template);

        // Divide in validation and train subsets
        if(train_subset <=1 && train_subset > 0)
        {
            weak_learner_validation_set = new SubVMatrix(weak_learner_training_set,train_subset,0,1-train_subset,weak_learner_training_set->width());
            weak_learner_training_set = new SubVMatrix(weak_learner_training_set,0.0,0,train_subset,weak_learner_training_set->width());
        }
        else if(train_subset > 1)
        {
            weak_learner_validation_set = new SubVMatrix(weak_learner_training_set,(int)train_subset,0,weak_learner_training_set->length()-(int)train_subset,weak_learner_training_set->width());
            weak_learner_training_set = new SubVMatrix(weak_learner_training_set,0,0,(int)train_subset,weak_learner_training_set->width());
        }

        // Adjust training set size for learners using
        // iterative training methods (e.g. gradient descent)
        // Note that this really makes more sense when used with
        // should be used with scale_using_max_weight
 
        if(adjust_training_set_size)
        {
            if(train_subset <= 0)
                new_weak_learner->setTrainingSet(repeat_vmatrix(weak_learner_training_set,(int)(max_weight/sum_of_square_weights + 0.5)));
            else
                new_weak_learner->setTrainingSet(vconcat(repeat_vmatrix(weak_learner_training_set,(int)(max_weight/sum_of_square_weights + 0.5))
                                                         &
                                                         repeat_vmatrix(weak_learner_validation_set,(int)(max_weight/sum_of_square_weights + 0.5))));
        }
        else
        {
            if(train_subset <= 0)
                new_weak_learner->setTrainingSet(weak_learner_training_set);
            else
                new_weak_learner->setTrainingSet(vconcat(weak_learner_training_set,weak_learner_validation_set));
        }
        n = weak_learner_training_set->length();
        new_weak_learner->setTrainStatsCollector(new VecStatsCollector);
    
        if(expdir!="" && provide_learner_expdir)
            new_weak_learner->setExperimentDirectory(append_slash(expdir+"WeakLearner" + tostring(stage) + "Expdir"));

        new_weak_learner->train();
    

        if(!ordinary_boosting)
        {
            TVec< PP<PLearner> > pl(1); pl[0] = new_weak_learner;
            if(train_subset <= 0)
                weak_learners_output.push_back(new MemoryVMatrix(new PLearnerOutputVMatrix(weak_learner_training_set, pl, false, false, false, false)));
            else
                weak_learners_output.push_back(new MemoryVMatrix(new PLearnerOutputVMatrix(vconcat(weak_learner_training_set&weak_learner_validation_set), pl, false, false, false, false)));
            input.resize(input.length()+1);
        }

        weak_learners.push_back(new_weak_learner);

        // This should be done after a full iteration
        if (save_often && expdir!="")
            PLearn::save(append_slash(expdir)+"model.psave", *this);
    

        // compute the new learner's weight
        //theta = 2*penalty_coefficient*new_weak_learner->penalty_cost();
        theta.fill(0);


        for(int i=0; i<theta.length(); i++)
            if(theta[i] >= 1) PLWARNING("theta[%d] is >= 1, this could cause problems",i);
    

        if(boosting_type == "ConfidenceBoost")
        {
            // Find optimal weight with line search, blame Norman if this doesn't work ;) 
      

            real ax = -10;
            real bx = 1;
            real cx = 100;
            real xmin;
            real tolerance = 0.001;
            int itmax = 10000000;

            int iter;
            real xtmp;
            real fa, fb, fc, ftmp;

            // compute function for fa, fb and fc

            fa = 0;
            fb = 0;
            fc = 0;


            for (int i=0; i<n; ++i) {
                weak_learner_training_set->getExample(i, input, target, weight);
                new_weak_learner->computeOutput(input,output);
                for(int j=0; j<outputsize(); j++)
                {
                    real y_i=(2*target[j]-1);
                    real f_i=(2*output[j]-1);
                    fa += example_weights(i,j)*exp(-1*ax*(f_i*y_i - theta[j]));
                    fb += example_weights(i,j)*exp(-1*bx*(f_i*y_i - theta[j]));
                    fc += example_weights(i,j)*exp(-1*cx*(f_i*y_i - theta[j]));
                }
            }
            for(iter=1;iter<=itmax;iter++)
            {
                if(verbosity>4)
                    cout << "fb = " << fb << endl;
                if (abs(cx-ax) <= tolerance)
                {
                    xmin=bx;
                    if(verbosity>3)
                    {
                        cout << "nIters for minimum: " << iter << endl;
                        cout << "xmin = " << xmin << endl;
                    }
                    break;
                }
                if (abs(bx-ax) > abs(bx-cx)) 
                {
                    xtmp = (bx + ax) * 0.5;

              
                    ftmp = 0;
                    for (int i=0; i<n; ++i) {
                        weak_learner_training_set->getExample(i, input, target, weight);
                        new_weak_learner->computeOutput(input,output);
                        for(int j=0; j<outputsize(); j++)
                        {
                      
                            real y_i=(2*target[j]-1);
                            real f_i=(2*output[j]-1);
                            ftmp += example_weights(i,j)*exp(-1*xtmp*(f_i*y_i-theta[j]));
                        }
                    }
                    if (ftmp > fb)
                    {
                        ax = xtmp;
                        fa = ftmp;
                    }
                    else
                    {
                        cx = bx;
                        fc = fb;
                        bx = xtmp;
                        fb = ftmp;
                    }
              
            
                }
                else
                {
                    xtmp = (bx + cx) * 0.5;
                    ftmp = 0;
                    for (int i=0; i<n; ++i) {
                        weak_learner_training_set->getExample(i, input, target, weight);
                        new_weak_learner->computeOutput(input,output);
                        for(int j=0; j<outputsize(); j++)
                        {
                      
                            real y_i=(2*target[j]-1);
                            real f_i=(2*output[j]-1);
                            ftmp += example_weights(i,j)*exp(-1*xtmp*(f_i*y_i-theta[j]));
                        }
                    }
                    if (ftmp > fb)
                    {
                        cx = xtmp;
                        fc = ftmp;
                    }
                    else
                    {
                        ax = bx;
                        fa = fb;
                        bx = xtmp;
                        fb = ftmp;
                    }
              
                }
            }
            if(verbosity>3)
            {
                cout << "Too many iterations in Brent" << endl;
            }
            xmin=bx;
            voting_weights.push_back(xmin);
            sum_voting_weights += abs(voting_weights[stage]);
          
            // calculate weighted training error 
            {
                ProgressBar *pb=0;     
                if(report_progress) pb = new ProgressBar("computing weighted training error of weak learner",train_set->length());
                learners_error.push_back(0);
                for (int i=0; i<train_set->length(); ++i) {
                    if(report_progress) pb->update(i);
                    if(i<weak_learner_training_set->length())
                        weak_learner_training_set->getExample(i, input, target, weight);
                    else
                        weak_learner_validation_set->getExample(i-weak_learner_training_set.length(), input, target, weight);
                    new_weak_learner->computeOutput(input,output);
                    for(int j=0;j<outputsize();j++)
                    {
                        real y_i=target[j];
                        real f_i=output[j];
                        //examples_error(i,j) = 2*(f_i+y_i-2*f_i*y_i);
                        examples_error(i,j) = (2*f_i-1)*(2*y_i-1);
                        if(i<weak_learner_training_set->length())
                            learners_error[stage] += example_weights(i,j)*exp(-voting_weights[stage]*examples_error(i,j));
                    }
                }
                if(report_progress) delete(pb);
            }


        }
        else if(boosting_type == "AdaBoost")
        {
            // calculate the weighted training error 
            {
                ProgressBar *pb=0;     
                if(report_progress) pb = new ProgressBar("computing weighted training error of weak learner",train_set->length());
                learners_error.push_back(0);
                for (int i=0; i<train_set->length(); ++i) {
                    if(report_progress) pb->update(i);
                    if(i<weak_learner_training_set->length())
                        weak_learner_training_set->getExample(i, input, target, weight);
                    else
                        weak_learner_validation_set->getExample(i-weak_learner_training_set.length(), input, target, weight);
                    new_weak_learner->computeOutput(input,output);
                    for(int j=0;j<outputsize();j++)
                    {
                        real y_i=target[j];
                        real f_i=output[j];
            
                        if (y_i==1)
                        {
                            if (f_i<output_threshold)
                            {
                                if(i<weak_learner_training_set->length())
                                    learners_error[stage] += example_weights(i,j);
                                examples_error(i,j)=1;
                            }
                            else examples_error(i,j) = -1;
                        }
                        else
                        {
                            if (f_i>=output_threshold) {
                                if(i<weak_learner_training_set->length())
                                    learners_error[stage] += example_weights(i,j);
                                examples_error(i,j)=1;
                            }
                            else examples_error(i,j)=-1;
                        }            
                    }
          
                }
                if(report_progress) delete(pb);
            }

            voting_weights.push_back(safeflog(((1-learners_error[stage])*target_error)/(learners_error[stage]*(1-target_error)) 
                                              * (1-theta[0])/(1+theta[0])));
            sum_voting_weights += fabs(voting_weights[stage]);
        }
        else
            PLERROR("In OverlappingAdaBoost::computeOutput: boosting type %s not supported", boosting_type.c_str());

        // update the example weights


        real sum_w=0;
        for (int i=0;i<train_set->length();i++)
        {
            for(int j=0; j<outputsize(); j++)
            {
                example_weights(i,j) *= exp(-voting_weights[stage]*examples_error(i,j));
                sum_w += example_weights(i,j);
            }
        }
        example_weights *= real(1)/sum_w;

        if (verbosity>1)
            cout << "weak learner at stage " << stage << " has average loss = " << learners_error[stage] << endl;
    
    }
}


void OverlappingAdaBoost::computeOutput(const Vec& input, Vec& output) const
{

    output.resize(outputsize());
    output.clear();
    real sum_out=0;
    Vec temp_input(input.length()+nstages*outputsize()); 
    temp_input.resize(input.size());
    temp_input << input;
    Vec bogus_weights(outputsize());
    static Vec weak_learner_output(outputsize());
    for (int i=0;i<voting_weights.length();i++)
    {

        if(nclasses == 2)
        {
            weak_learners[i]->computeOutput(temp_input,weak_learner_output);
            if(boosting_type == "AdaBoost")
                sum_out += (weak_learner_output[0] < output_threshold ? -1 : 1) *voting_weights[i];
            else if(boosting_type == "ConfidenceBoost")
                sum_out += (2*weak_learner_output[0]-1)*voting_weights[i];
            else
                PLERROR("In OverlappingAdaBoost::computeOutput: boosting type %s not supported", boosting_type.c_str());
        }
        else
        {
            weak_learners[i]->computeOutput(concat(temp_input,bogus_weights),weak_learner_output);
            for(int j=0; j<outputsize(); j++)
            {
                if(boosting_type == "AdaBoost")
                    output[j] += (weak_learner_output[j] < output_threshold ? -1 : 1) *voting_weights[i];
                else if(boosting_type == "ConfidenceBoost")
                    output[j] += (2*weak_learner_output[j]-1)*voting_weights[i];
                else
                    PLERROR("In OverlappingAdaBoost::computeOutput: boosting type %s not supported", boosting_type.c_str());
            }
        }
        if(!ordinary_boosting)
        {
            temp_input.resize(temp_input.length()+outputsize()); 
            temp_input.subVec(temp_input.length()-outputsize(),outputsize()) << weak_learner_output;
        }
    }
    if(nclasses == 2)
        output[0] = (sum_out/sum_voting_weights+1)/2;
    else
        for(int j=0; j<outputsize(); j++)
            output[j] = (output[j]/sum_voting_weights+1)/2;
}

void OverlappingAdaBoost::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                  const Vec& target, Vec& costs) const
{

    costs.resize(4+nclasses);
    costs.clear();

    if(nclasses != 2)
        costs[0] = argmax(target) != argmax(output);
    else
    {
        if (target[0] == 0) {
            costs[0] = output[0] >= output_threshold; 
        }
        else if (target[0] == 1) {
            costs[0] = output[0] < output_threshold; 
        }
    }
                   
    for(int i=0; i<outputsize(); i++)
    {
        if (target[i] == 0) {
            costs[2] += output[0] >= output_threshold; 
        }
        else if (target[i] == 1) {
            costs[2] += output[0] < output_threshold; 
        }
        else PLERROR("OverlappingAdaBoost::computeCostsFromOutputs: target must be "
                     "either 0 or 1; current target=%f", target[i]);
        costs[1] += exp(-1.0*sum_voting_weights*(2*output[i]-1)*(2*target[i]-1));
        costs[4+i] = exp(-1.0*sum_voting_weights*(2*output[i]-1)*(2*target[i]-1));
    }

    if(classes_to_discriminate.length() > 1)
    {
        int arg_class = -1;
        real max_output = -1;
        for(int i=0; i<classes_to_discriminate.length(); i++)
        {
            if(output[classes_to_discriminate[i]] > max_output)
            {
                max_output = output[classes_to_discriminate[i]];
                arg_class = classes_to_discriminate[i];
            }
        }
        if(arg_class == argmax(target))
            costs[3] = 0;
        else
            costs[3] = 1;
    }
    else
        costs[3] = 1;
}

TVec<string> OverlappingAdaBoost::getTestCostNames() const
{
    return getTrainCostNames();
}

TVec<string> OverlappingAdaBoost::getTrainCostNames() const
{

    TVec<string> costs(4+nclasses);
    if(nclasses == 2)
        costs[0] = "binary_class_error";
    else
        costs[0] = "class_error";
    costs[1] = "exp_neg_margin";
    costs[2] = "multiclass_error";
    costs[3] = "subset_class_error";
    for(int i=0; i<nclasses; i++)
        costs[4+i] = "exp_neg_margin_" + tostring(i);
    return costs;
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
