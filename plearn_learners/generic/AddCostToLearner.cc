// -*- C++ -*-

// AddCostToLearner.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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

/*! \file AddCostToLearner.cc */


#include "AddCostToLearner.h"
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/var/SumOverBagsVariable.h>   //!< For the bag signal constants.
#include <plearn/var/VarArray.h>
#include <plearn/var/VecElementVariable.h>
#include <plearn/sys/Profiler.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(AddCostToLearner,
                        "A PLearner that just adds additional costs to another PLearner.",
                        "In addition, this learner can be used to compute costs on bags instead of\n"
                        "individual samples, using the option 'compute_costs_on_bags' (this will\n"
                        "also automatically remove the bags column from the training set, so that\n"
                        "the learner can be trained as usual).\n"
                        "\n"
                        "Note that for now, the added costs are only added as test costs.\n"
                        "\n"
                        "Feel free to make this class evolve by adding new costs, or rewriting it\n"
                        "in a better fashion, because this one is certainly not perfect.\n"
                        "To use the lift cost, do the following:\n"
                        " (1) add a cost of type 'lift_output' to this object's option 'costs'\n"
                        " (2) replace the template_stats_collector of your PTester with one like this:\n"
                        "   template_stats_collector =\n"
                        "     LiftStatsCollector (\n"
                        "      lift_fraction = 0.1 ;\n"
                        "      output_column = \"lift_output\" ;\n"
                        "      opposite_lift = 0 ; # to set to 1 if we want to optimize it\n"
                        "      sign_trick = 1 ;\n"
                        " (3) ask for the lift in the stats:\n"
                        "   statnames = [\n"
                        "     \"E[test1.LIFT]\"\n"
                        "     \"E[test1.LIFT_MAX]\"\n"
                        "   ];" );

//////////////////////
// AddCostToLearner //
//////////////////////
AddCostToLearner::AddCostToLearner()
    : bag_size(0),
      train_time(0),
      total_train_time(0),
      test_time(0),
      total_test_time(0),
      train_time_b(false),
      test_time_b(false),
      check_output_consistency(1),
      combine_bag_outputs_method(1),
      compute_costs_on_bags(0),
      force_output_to_target_interval(0),
      from_max(1),
      from_min(-1),
      rescale_output(0),
      rescale_target(0),
      to_max(1),
      to_min(0),
      n_classes(-1),
      confusion_matrix_target(0),
      find_class_threshold(false)
{}

////////////////////
// declareOptions //
////////////////////
void AddCostToLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "check_output_consistency", &AddCostToLearner::check_output_consistency, OptionBase::buildoption,
                  "If set to 1, additional checks will be performed to make sure the output\n"
                  "is compatible with the costs to be computed. This may slow down the costs\n"
                  "computation, but is also safer.");

    declareOption(ol, "combine_bag_outputs_method", &AddCostToLearner::combine_bag_outputs_method, OptionBase::buildoption,
                  "The method used to combine the individual outputs of the sub-learner to\n"
                  "obtain a global output on the bag (irrelevant if 'compute_costs_on_bags' == 0):\n"
                  " - 1 : o = 1 - (1 - o_1) * (1 - o_2) * .... * (1 - o_n)\n"
                  " - 2 : o = max(o_1, o_2, ..., o_n)");

    declareOption(ol, "compute_costs_on_bags", &AddCostToLearner::compute_costs_on_bags, OptionBase::buildoption,
                  "If set to 1, then the costs will be computed on bags, but the sub-learner will\n"
                  "be trained without the bag information (see SumOverBagsVariable for info on bags).");

    declareOption(ol, "costs", &AddCostToLearner::costs, OptionBase::buildoption,
        "The costs to be added:\n"
        " - 'class_error': classification error. If the sub-learner's output\n"
        "   has the same length as the target vector, then they are compared\n"
        "   component-wise. Otherwise, the target must be a one-dimensional\n"
        "   vector (an integer corresponding to the class), and the output\n"
        "   from the sub-learner is interpreted as a vector of weights for\n"
        "   each class.\n"
        " - 'binary_class_error': classification error for a one-dimensional\n"
        "   target that must be either 0 or 1. The output must also be one-\n"
        "   dimensional, and is interpreted as the predicted probability for\n"
        "   class 1 (thus class 1 is chosen when the output is > 0.5)\n"
        " - 'linear_class_error': as class_error execpt that the output is the\n"
        "   difference between the class values\n"
        " - 'square_class_error': as class_error execpt that the output is the\n"
        "   square of the difference between the class values\n"
        " - 'confusion_matrix': give the confusion matrix for the target\n"
        "   'confusion_matrix_target', where the row is the predicted class\n"
        "    and the column is the target class\n"
        " - 'lift_output': to compute the lift cost (for the positive class)\n"
        " - 'opposite_lift_output': to compute the lift cost (for the negative) class\n"
        " - 'cross_entropy': -t*log(o) - (1-t)*log(1-o)\n"
        " - 'NLL': -log(o[t])\n"
        " - 'mse': the mean squared error (o - t)^2\n"
        " - 'squared_norm_reconstruction_error': | ||i||^2 - ||o||^2 |\n"
        " - 'train_time': the time spend in the last call to the train() function\n"
        " - 'total_train_time': the total time spend in the train() function\n"
        " - 'test_time': the time spend in test() fct the between the last two call to train()\n"
        " - 'total_test_time': the sum of test_time\n"
        " - 'type1_err': SUM[type1_err] will return the number of type 1 error(false positive).\n"
        "                E[type1_err], will return the false positive rate: # false positive/# of positive\n" 
        " - 'type2_err': idem as type1_err but for the type 2 error(false negative)\n" 
        " - 'sensitivity': E[sensitivity] return nb true pos/nb total pos"
        " - 'specificity': E[specificity] return nb true neg/nb total ng"
    );

    declareOption(ol, "force_output_to_target_interval", &AddCostToLearner::force_output_to_target_interval, OptionBase::buildoption,
                  "If set to 1 and 'rescale_output' is also set to 1, then the scaled output\n"
                  "will be forced to belong to [to_min, to_max], which may not be the case otherwise\n"
                  "if the output doesn't originate from [from_min, from_max].");
      
    declareOption(ol, "rescale_output", &AddCostToLearner::rescale_output, OptionBase::buildoption,
                  "If set to 1, then the output will be rescaled before computing the costs, according\n"
                  "to the values of from_min, from_max, to_min, to_max. This means it will map\n"
                  "[from_min, from_max] to [to_min, to_max].");

    declareOption(ol, "rescale_target", &AddCostToLearner::rescale_target, OptionBase::buildoption,
                  "Same as 'rescale_output', but for the target.");

    declareOption(ol, "from_max", &AddCostToLearner::from_max, OptionBase::buildoption,
                  "Upper bound of the source interval [from_min, from_max] (used in rescaling).");

    declareOption(ol, "from_min", &AddCostToLearner::from_min, OptionBase::buildoption,
                  "Lower bound of the source interval [from_min, from_max] (used in rescaling).");

    declareOption(ol, "to_max", &AddCostToLearner::to_max, OptionBase::buildoption,
                  "Upper bound of the destination interval [to_min, to_max] (used in rescaling).");

    declareOption(ol, "to_min", &AddCostToLearner::to_min, OptionBase::buildoption,
                  "Lower bound of the destination interval [to_min, to_max] (used in rescaling).");
    
    declareOption(ol, "n_classes", &AddCostToLearner::n_classes, OptionBase::buildoption,
        "The number of classes. Only needed for the 'confusion_matrix' cost.");

    declareOption(ol, "confusion_matrix_target",
                  &AddCostToLearner::confusion_matrix_target,
                  OptionBase::buildoption,
        "Index of the target for which the confusion matrix is computed.");

    declareOption(ol, "find_class_threshold",
                  &AddCostToLearner::find_class_threshold,
                  OptionBase::buildoption,
        "If true, then during training we find the best threshold between\n"
        "classes.");

    declareOption(ol, "train_time",
                  &AddCostToLearner::train_time, OptionBase::learntoption,
                  "The time spent in the last call to train() in second.");

    declareOption(ol, "total_train_time",
                  &AddCostToLearner::total_train_time, OptionBase::learntoption,
                  "The total time spent in the train() function in second.");

    declareOption(ol, "test_time",
                  &AddCostToLearner::test_time, OptionBase::learntoption,
                  "The time spent in the last call to test() in second.");

    declareOption(ol, "total_test_time",
                  &AddCostToLearner::total_test_time, OptionBase::learntoption,
                  "The total time spent in the test() function in second.");

    declareOption(ol, "train_time_b",
                  &AddCostToLearner::train_time, OptionBase::learntoption,
                  "If we should calculate the time spent in the train.");

    declareOption(ol, "test_time_b",
                  &AddCostToLearner::test_time, OptionBase::learntoption,
                  "If we should calculate the time spent in the test.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void AddCostToLearner::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void AddCostToLearner::build_()
{
    // Give a default size to bag_outputs.
    bag_outputs.resize(10, 1);
    // Make sure all costs are valid.
    int n = costs.length();
    int min_verb = 2;
    bool display = (verbosity >= min_verb);
    int os = learner_->outputsize();
    if (os < 0) {
        // The sub-learner does not know its outputsize yet: we skip the build for
        // now, it will have to be done later.
        if (display)
            cout << "In AddCostToLearner::build_ - The sub-learner does not know its outputsize yet, skipping" << endl;
        return;
    }
    sub_learner_output.resize(os);
    desired_target.resize(os);
    if (rescale_output || rescale_target) {
        real from_fac = from_max - from_min;
        real to_fac = to_max - to_min;
        fac = to_fac / from_fac;
    }
    output_min = -REAL_MAX;
    output_max = REAL_MAX;
    if (n > 0 && display) {
        cout << "Additional costs computed: ";
    }
    for (int i = 0; i < n; i++) {
        string c = costs[i];
        if (display) cout << c << " ";
        if (c == "lift_output") {
            // Output should be positive.
            output_min = max(output_min, real(0));
        } else if (c == "opposite_lift_output") {
            // 1 - output should be positive.
            output_max = min(output_max, real(1));
        } else if (c == "cross_entropy") {
            // Output should be in [0,1].
            output_min = max(output_min, real(0));
            output_max = min(output_max, real(1));
            {
                Var zero = var(0);
                output_var = accessElement(sub_learner_output, zero);
                target_var = accessElement(desired_target, zero);
                cross_entropy_var = cross_entropy(output_var, target_var);
                cross_entropy_prop = propagationPath(cross_entropy_var);
            }
        } else if (c == "mse") {
        } else if (c == "squared_norm_reconstruction_error") {
        } else if (c == "class_error") {
        } else if (c == "binary_class_error") {
        } else if (c == "train_time") {
            train_time_b=true;
        } else if (c == "total_train_time") {
            train_time_b=true;
        } else if (c == "test_time") {
            test_time_b=true;
        } else if (c == "total_test_time") {
            test_time_b=true;
        } else if (c == "linear_class_error") {
        } else if (c == "square_class_error") {
        } else if (c == "confusion_matrix") {
            if(n_classes<=0)
                PLERROR("In AddCostToLearner::build_ there must be a positive number of class. n_classes ="+n_classes);
            output_min = 0;
            output_max = n_classes;
        } else if (c == "NLL") {
            // Output should be in [0,1].
            output_min = max(output_min, real(0));
            output_max = min(output_max, real(1));
        } else if (c == "type1_err") {
            output_min = 0;
            output_max = 1;
        } else if (c == "type2_err") {
            output_min = 0;
            output_max = 1;
        } else if (c == "sensitivity") {
            output_min = 0;
            output_max = 1;
        } else if (c == "specificity") {
            output_min = 0;
            output_max = 1;
        } else {
            PLERROR("In AddCostToLearner::build_ - Invalid cost requested %s (make sure you are using the new costs syntax)",c.c_str());
        }
    }
    if (n > 0 && display) {
        cout << endl;
    }
    
    if(test_time_b)
        Profiler::reset("AddCostToLearner::test");

    if(test_time_b || train_time_b)
        Profiler::activate();
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void AddCostToLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                const Vec& target, Vec& costs,
                                                const bool add_sub_learner_costs) const
{
    int n_original_costs = learner_->nTestCosts();
    // We give only costs.subVec to the sub-learner because it may want to resize it.
    costs.resize(nTestCosts());
    Vec sub_costs = costs.subVec(0, n_original_costs);
    int target_length = target.length();
    if(add_sub_learner_costs){
        if (compute_costs_on_bags) {
            learner_->computeCostsFromOutputs(input, output, target.subVec(0, target_length - 1), sub_costs);
        } else {
            learner_->computeCostsFromOutputs(input, output, target, sub_costs);
        }
    }

    if (compute_costs_on_bags) {
        // We only need to compute the costs when the whole bag has been seen,
        // otherwise we just store the outputs of each sample in the bag and fill
        // the cost with MISSING_VALUE.
        int bag_signal = int(target[target_length - 1]);
        if (bag_signal & SumOverBagsVariable::TARGET_COLUMN_FIRST) {
            // Beginning of the bag.
            bag_size = 0;
        }
        if (bag_outputs.width() != output.length()) {
            // Need to resize bag_outputs.
            bag_outputs.resize(bag_outputs.length(), output.length());
        }
        if (bag_outputs.length() <= bag_size) {
            // Need to resize bag_outputs.
            bag_outputs.resize(bag_outputs.length() * 2, bag_outputs.width());
        }
        bag_outputs(bag_size) << output;
        bag_size++;
        if (bag_signal & SumOverBagsVariable::TARGET_COLUMN_LAST) {
            // Reached the end of the bag: we can compute the output for the bag.
            bag_outputs.resize(bag_size, bag_outputs.width());
            combined_output.resize(output.length());
            switch (combine_bag_outputs_method) {
            case 1: // o = 1 - (1 - o_1) * (1 - o_2) * .... * (1 - o_n)
            {
                real prod;
                for (int j = 0; j < bag_outputs.width(); j++) {
                    prod = 1;
                    for (int i = 0; i < bag_outputs.length(); i++) {
                        prod = prod * (1 - bag_outputs(i, j));
                    }
                    combined_output[j] = 1 - prod;
                }
            }
            break;
            case 2: // o = max(o_1, o_2, ..., o_n)
            {
                for (int j = 0; j < bag_outputs.width(); j++) {
                    combined_output[j] = max(bag_outputs.column(j));
                }
            }
            break;
            default:
                PLERROR("In AddCostToLearner::computeCostsFromOutputs - Unknown value for 'combine_bag_outputs_method'");
            }
            // We re-compute the sub-learner's costs with the brand new combined bag output.
            if(add_sub_learner_costs)
                learner_->computeCostsFromOutputs(input, combined_output, target.subVec(0, target_length - 1), sub_costs);
        } else {
            costs.fill(MISSING_VALUE);
            return;
        }
    } else {
        combined_output = output;
    }

    Vec the_target;
    if (compute_costs_on_bags) {
        the_target = target.subVec(0, target_length - 1);
    } else {
        the_target = target;
    }

    // Optional rescaling.
    if (!rescale_output) {
        sub_learner_output << combined_output;
    } else {
        int n = output.length();
        real scaled_output;
        for (int i = 0; i < n; i++) {
            scaled_output = (combined_output[i] - from_min) * fac + to_min;
            if (force_output_to_target_interval) {
                if (scaled_output > to_max) {
                    scaled_output = to_max;
                } else if (scaled_output < to_min) {
                    scaled_output = to_min;
                }
            }
            sub_learner_output[i] = scaled_output;
        }
    }
    if (!rescale_target) {
        desired_target.resize(the_target.length());
        desired_target << the_target;
    } else {
        int n = output.length();
        if (n != target_length)
            PLERROR("In AddCostToLearner::computeCostsFromOutputs - When rescaling, "
                    "output and target are expected to have the same length");
        for (int i = 0; i < n; i++) {
            desired_target[i] = (the_target[i] - from_min) * fac + to_min;
        }
    }

    if (check_output_consistency) {
        real out;
        for (int i = 0; i < sub_learner_output.length(); i++) {
            out = sub_learner_output[i];
            if (out < output_min) {
                if (fast_is_equal(out, output_min))
                    sub_learner_output[i] = output_min;
                else 
                    PLERROR("In AddCostToLearner::computeCostsFromOutputs - "
                            "Sub-learner output (%f) is lower than %f",
                            out, output_min);
            }
            if (out > output_max) {
                if (fast_is_equal(out, output_max))
                    sub_learner_output[i] = output_max;
                else
                    PLERROR("In AddCostToLearner::computeCostsFromOutputs - "
                            "Sub-learner output (%f) is higher than %f",
                            out, output_max);
            }
        }
    }
    int ind_cost = n_original_costs - 1;
    for (int i = 0; i < this->costs.length(); i++) {
        string c = this->costs[i];
        ind_cost++;
        if (c == "lift_output" || c == "opposite_lift_output") {
#ifdef BOUNDCHECK
            if (desired_target.length() != 1 && (sub_learner_output.length() != 1 || sub_learner_output.length() != 2)) {
                PLERROR("In AddCostToLearner::computeCostsFromOutputs - Lift cost is "
                        "only meant to be used with a one-dimensional target, and a "
                        "one-dimensional output or a two-dimensional output (which "
                        "would give the weights for classes 0 and 1 respectively)");
            }
#endif
            {
                // The 'lift cost', which actually isn't a cost, is the output when
                // the target is 1, and -output when the target is 0.
                // The 'opposite_lift cost' is 1-output when the target is 0, and
                // -(1-output) when thte target is 1.
#ifdef BOUNDCHECK
                if (!fast_exact_is_equal(desired_target[0], 0) &&
                    !fast_exact_is_equal(desired_target[0], 1)) {
                    // Invalid target.
                    PLERROR("In AddCostToLearner::computeCostsFromOutputs - Target "
                            "%f isn't compatible with lift", desired_target[0]);
                }
#endif
                bool opposite_lift = (c == "opposite_lift_output");
                if (fast_exact_is_equal(desired_target[0], 1)) {
                    if (sub_learner_output.length() == 1)
                        if (opposite_lift)
                            costs[ind_cost] = sub_learner_output[0] - 1;
                        else
                            costs[ind_cost] = sub_learner_output[0];
                    else
                        if (opposite_lift)
                            costs[ind_cost] = - (sub_learner_output[0] - sub_learner_output[1] + 1) / 2.0;
                        else
                            costs[ind_cost] = (sub_learner_output[1] - sub_learner_output[0] + 1) / 2.0;
                } else {
                    if (sub_learner_output.length() == 1)
                        if (opposite_lift)
                            costs[ind_cost] = 1 - sub_learner_output[0];
                        else
                            costs[ind_cost] = - sub_learner_output[0];
                    else
                        if (opposite_lift)
                            costs[ind_cost] = (sub_learner_output[0] - sub_learner_output[1] + 1) / 2.0;
                        else
                            costs[ind_cost] = - (sub_learner_output[1] - sub_learner_output[0] + 1) / 2.0;
                }
            }
        } else if (c == "cross_entropy") {
#ifdef BOUNDCHECK
            if (!fast_exact_is_equal(desired_target[0], 0) &&
                !fast_exact_is_equal(desired_target[0], 1)) {
                // Invalid target.
                PLERROR("In AddCostToLearner::computeCostsFromOutputs - Target isn't compatible with cross_entropy");
            }
#endif
            cross_entropy_prop.fprop();
            costs[ind_cost] = cross_entropy_var->valuedata[0];
        } else if (c == "NLL") {
            PLASSERT_MSG(fast_exact_is_equal(desired_target[0],
                        round(desired_target[0])), "The target must be an "
                    "integer");
            int class_target = int(round(desired_target[0]));
            PLASSERT_MSG(class_target < sub_learner_output.length(),
                    "The sub learner output must have a size equal to the "
                    "number of classes");
            costs[ind_cost] = - pl_log(sub_learner_output[class_target]);
       } else if (c == "class_error") {
            int output_length = sub_learner_output.length();
            bool good = true;
            if (output_length == target_length) {
                for (int k = 0; k < desired_target.length(); k++)
                    if (!is_equal(desired_target[k],
                                  sub_learner_output[k])) {
                        good = false;
                        break;
                    }
            } else if (target_length == 1) {
                // We assume the target is a number between 0 and c-1, and the output
                // is a vector of length c giving the weight for each class.
                good = is_equal(argmax(sub_learner_output), desired_target[0]);
            } else {
                PLERROR("In AddCostToLearner::computeCostsFromOutputs - Wrong "
                        "output and/or target for the 'class_error' cost");
            }
            costs[ind_cost] = good ? 0 : 1;
        } else if (c == "binary_class_error") {
            PLASSERT( target_length == 1 );
            real t = desired_target[0];
            PLASSERT( fast_exact_is_equal(t, 0) || fast_exact_is_equal(t, 1));
            PLASSERT( sub_learner_output.length() == 1 );
            real predict = sub_learner_output[0] > 0.5 ? 1 : 0;
            costs[ind_cost] = is_equal(t, predict) ? 0 : 1;
         } else if (c == "linear_class_error") {
            int output_length = sub_learner_output.length();
            int diff = 0;
            if (output_length == target_length) {
                for (int k = 0; k < desired_target.length(); k++)
                    diff += abs(int(round(desired_target[k])) - int(round(sub_learner_output[k])));
            } else if (target_length == 1) {
                // We assume the target is a number between 0 and c-1, and the output
                // is a vector of length c giving the weight for each class.
                diff = abs(argmax(sub_learner_output) - int(round(desired_target[0])));
            } else {
                PLERROR("In AddCostToLearner::computeCostsFromOutputs - Wrong "
                        "output and/or target for the 'linear_class_error' cost");
            }
            costs[ind_cost] = diff;
         } else if (c == "square_class_error") {
            int output_length = sub_learner_output.length();
            int diff = 0;
            if (output_length == target_length) {
                for (int k = 0; k < desired_target.length(); k++) {
                    int d = int(round(desired_target[k])) - int(round(sub_learner_output[k]));
                    diff += d*d;
                }
            } else if (target_length == 1) {
                // We assume the target is a number between 0 and c-1, and the output
                // is a vector of length c giving the weight for each class.
                diff = argmax(sub_learner_output) - int(round(desired_target[0]));
                diff *= diff;
            } else {
                PLERROR("In AddCostToLearner::computeCostsFromOutputs - Wrong "
                        "output and/or target for the 'square_class_error' cost");
            }
            costs[ind_cost] = diff;
        } else if (c == "confusion_matrix") {

#ifdef BOUNDCHECK
            if (confusion_matrix_target >= target_length || confusion_matrix_target<-1)
                PLERROR("In AddCostToLearner::computeCostsFromOutputs - confusion_matrix_target(%d) "
                        "not in the range of target_length(%d)", confusion_matrix_target, target_length);
#endif
            int sub_learner_out;
            real the_target;
            if (confusion_matrix_target==-1) {
                //output are probability
                sub_learner_out = argmax(sub_learner_output);
                the_target = desired_target[0];
            }else{
                sub_learner_out = int(round(sub_learner_output[confusion_matrix_target]));
                the_target = desired_target[confusion_matrix_target];
            }
	    if(sub_learner_out<0){
	      PLWARNING("In AddCostToLearner::computeCostsFromOutputs - bad value for sub_learner_out %d, we use 0 instead", sub_learner_out);
	      sub_learner_out = 0;
	    }
	    if(sub_learner_out>=n_classes){
	      PLWARNING("In AddCostToLearner::computeCostsFromOutputs - bad value for sub_learner_out %d, we use %d instead", sub_learner_out,n_classes -1);
	      sub_learner_out = n_classes - 1;
	    }
            PLCHECK(sub_learner_out<n_classes && sub_learner_out>=0);
//if outside allowd range, will access the wrong element in the cost vector
#ifdef BOUNDCHECK
            if (sub_learner_out >= n_classes
                || is_missing(sub_learner_out))
                PLERROR("In AddCostToLearner::computeCostsFromOutputs - bad output value of sub_learner: sub_learner_out=%d,  "
                        " missing or higher or egual to n_classes (%d)",
                        sub_learner_out,n_classes);
            if (the_target >= n_classes
                ||is_missing(the_target))
                PLERROR("In AddCostToLearner::computeCostsFromOutputs - bad output value of the_target=%f, missing or higher or egual to n_classes (%d)",
                        the_target, n_classes);
#endif
            for(int local_ind = ind_cost ; local_ind < (n_classes*n_classes+ind_cost); local_ind++){
                costs[local_ind] = 0;
            }
            int local_ind = ind_cost + sub_learner_out + int(round(the_target))*n_classes;

            costs[local_ind] = 1;
            ind_cost += n_classes*n_classes - 1;//less one as the loop add one
        } else if (c == "mse") {
            costs[ind_cost] = powdistance(desired_target, sub_learner_output);
        } else if (c == "squared_norm_reconstruction_error") {
            PLWARNING("In AddCostToLearner::computeCostsFromOutputs - 'squared_norm_reconstruction_error'"
                      " has not been tested yet, please remove this warning if it works correctly");
            costs[ind_cost] = abs(pownorm(input, 2) - pownorm(sub_learner_output, 2));
        } else if (c == "train_time") {
            costs[ind_cost] = train_time;
        } else if (c == "total_train_time") {
            costs[ind_cost] = total_train_time;
        } else if (c == "test_time") {
            costs[ind_cost] = test_time;
        } else if (c == "total_test_time") {
            costs[ind_cost] = total_test_time;
        } else if (c == "type1_err") {
            //false positive error
            //faux negatif/(faux negatif+vrai positif)
#ifdef BOUNDCHECK
            PLASSERT(sub_learner_output.length()==1);
#endif
            real target=desired_target[0];
            real out=sub_learner_output[0];
            if(fast_is_equal(target,1)){
                if (fast_is_equal(out,0))
                    costs[ind_cost] = 1;
                else
                    costs[ind_cost] = 0;
            }else
                costs[ind_cost] = MISSING_VALUE;
        } else if (c == "type2_err") {
            //false negative error
            //faux positif/(faux positif+ vrai negatif)
#ifdef BOUNDCHECK
            PLASSERT(sub_learner_output.length()==1);
#endif
            real target=desired_target[0];
            real out=sub_learner_output[0];
            if(fast_is_equal(target,0)){
                if(fast_is_equal(out,1))
                    costs[ind_cost] = 1;
                else
                    costs[ind_cost] = 0;
            }else
                costs[ind_cost] = MISSING_VALUE;
        } else if (c == "sensitivity") {
            //nb true pos/(nb true pos + nb false neg)
            //equiv to=nb true pos/nb total pos
            //should use X[test1.E[sensitivity]] to have the real value
#ifdef BOUNDCHECK
            PLASSERT(sub_learner_output.length()==1);
#endif
            real target=desired_target[0];
            real out=sub_learner_output[0];
                
            if(fast_is_equal(target,1)){
                if(fast_is_equal(out,1))
                    costs[ind_cost] = 1;
                else
                    costs[ind_cost] = 0;
            }else
                costs[ind_cost] = MISSING_VALUE;
        } else if (c == "specificity") {
            //nb true neg/(nb true neg + nb false pos)
            //equiv to=nb true neg/nb total ng
            //should use X[test1.E[specificity]] to have the real value
#ifdef BOUNDCHECK
            PLASSERT(sub_learner_output.length()==1);
#endif
            real target=desired_target[0];
            real out=sub_learner_output[0];
             
            if( fast_is_equal(target, 0)){
                if(fast_is_equal(out, 0))
                    costs[ind_cost] = 1;
                else
                costs[ind_cost] = 0;
            } else
                costs[ind_cost] = MISSING_VALUE;
        } else {
            PLERROR("In AddCostToLearner::computeCostsFromOutputs - Unknown cost");
        }
    }
}

///////////
// train //
///////////
void AddCostToLearner::train()
{
    Profiler::start("AddCostToLearner::train");

    int find_threshold = -1;
    if(find_class_threshold){
        for (int i = 0; i < this->costs.length(); i++) {
            if(costs[i]=="square_class_error" || costs[i]=="linear_class_error" || costs[i]=="class_error" )
                find_threshold = i;
            break;
        }
        PLASSERT_MSG(-1 != find_threshold , "We where asked to find the "
                "threshold and no *class_error costs are selected.\n"
                "We use the first *class_error cost to select the threshold");
    }
    inherited::train();
    
    if(-1 != find_threshold){
        
        Vec input;
        Vec target;
        Vec output;
        Vec outcosts;
        real weight;
        output.resize(learner_->outputsize());
        outcosts.resize(learner_->nTestCosts());
        class_threshold.resize(n_classes);
        Vec test_threshold;
        Vec best_threshold;
        test_threshold.resize(n_classes);
        best_threshold.resize(n_classes);
        double best_class_error = -1;
        int costs_index = -1;
        TVec<string> costsnames=getTestCostNames();
        Vec paramtotry;
        for(float f=0;f<3;f+=0.1)
            paramtotry.append(f);

        //find the index of the costs to use.
        for(int i=0;i<costsnames.size();i++){
            string str1 = costsnames[i];
            string str2 = costs[find_threshold];
            if( str1 == str2){
                costs_index = i;
                break;
            }
        }

        for(int a=0;a<paramtotry.size();a++){
            for(int b=a+1;b<paramtotry.size();b++){
                test_threshold[0] = paramtotry[a];
                test_threshold[1]  = paramtotry[b];
                double cum_class_error = 0;
                for(int i=0;i<train_set->length();i++){
                    learner_->getTrainingSet().getExample(i, input, target, weight);
                    computeOutputAndCosts(input, target, output, outcosts);
                    cum_class_error += outcosts[costs_index];
                }
                if(best_class_error == -1 || best_class_error > cum_class_error){
                    best_threshold << test_threshold;
                    best_class_error = cum_class_error;
                }
            }
        }
        class_threshold << best_threshold;
        if(verbosity >=2)
            for(int i=0;i<class_threshold.size();i++)
                cout << "class_threshold[" << i << "] = " <<class_threshold[i] << endl;

    }
    Profiler::end("AddCostToLearner::train");
    if(train_time_b){
        const Profiler::Stats& stats = Profiler::getStats("AddCostToLearner::train");
        real tmp=stats.wall_duration/Profiler::ticksPerSecond();
        train_time=tmp - total_train_time;
        total_train_time=tmp;
    }
    if(test_time_b){
        //we get the test_time here as we want the test time for all dataset.
        //if we put it in the test function, we would have it for one dataset.
        const Profiler::Stats& stats_test = Profiler::getStats("AddCostToLearner::test");
        real tmp=stats_test.wall_duration/Profiler::ticksPerSecond();
        test_time=tmp-total_test_time;
        total_test_time=tmp;  
    }
}

//////////
// test //
//////////
void AddCostToLearner::test(VMat testset, PP<VecStatsCollector> test_stats,
                            VMat testoutputs, VMat testcosts) const
{
    Profiler::start("AddCostToLearner::test");
    inherited::test(testset, test_stats, testoutputs, testcosts);
    Profiler::end("AddCostToLearner::test");
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void AddCostToLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
                                             Vec& output, Vec& costs) const {
    PLASSERT( learner_ );
    //done this way to use a possibly optimizer version 
    //of computeOutputAndCosts from the sub learner as with NatGradNNet

    Vec sub_costs = costs.subVec(0, learner_->nTestCosts());
    learner_->computeOutputAndCosts(input, target, output, sub_costs);
    computeCostsFromOutputs(input,output,target,costs,false);
}

///////////////////////////
// computeOutputsAndCosts //
///////////////////////////
void AddCostToLearner::computeOutputsAndCosts(const Mat& input, const Mat& target,
                                             Mat& output, Mat& costs) const
{
    PLASSERT( learner_ );
    //done this way to use a possibly optimizer version 
    //of computeOutputsAndCosts from the sub learner as with NatGradNNet
    //with a minibatch_size>1
    Mat sub_costs = costs.subMatColumns(0, learner_->nTestCosts());
    learner_->computeOutputsAndCosts(input, target, output, sub_costs);
    for (int i=0;i<input.length();i++)
    {
        Vec in_i = input(i);
        Vec out_i = output(i); 
        Vec target_i = target(i);
        Vec c_i = costs(i);
        computeCostsFromOutputs(in_i,out_i,target_i,c_i,false);
    }
    
}
////////////
// forget //
////////////
void AddCostToLearner::forget()
{
    inherited::forget();
    bag_size = 0;
}
    
//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> AddCostToLearner::getTestCostNames() const
{
    TVec<string> sub_costs = learner_->getTestCostNames();
    for (int i = 0; i < this->costs.length(); i++) {
        if(costs[i] == "confusion_matrix")
            for(int conf_i=0; conf_i< n_classes;conf_i++)
                for(int conf_j=0; conf_j<n_classes;conf_j++){
                    string s = "confusion_matrix_target"+tostring(conf_i)+"_pred"+tostring(conf_j);
                    sub_costs.append(s);
                }
        else
            sub_costs.append(costs[i]);
    }
    return sub_costs;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> AddCostToLearner::getTrainCostNames() const
{
    // The added costs are only test costs (so far).
    return learner_->getTrainCostNames();
}

//! To use varDeepCopyField.
#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AddCostToLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(combined_output,      copies);
    deepCopyField(bag_outputs,          copies);
    deepCopyField(cross_entropy_prop,   copies);
    varDeepCopyField(cross_entropy_var, copies);
    deepCopyField(desired_target,       copies);
    varDeepCopyField(output_var,        copies);
    deepCopyField(sub_learner_output,   copies);
    deepCopyField(sub_input,            copies);
    varDeepCopyField(target_var,        copies);
    deepCopyField(class_threshold,      copies);
    deepCopyField(costs,                copies);
}

////////////////////
// setTrainingSet //
////////////////////
void AddCostToLearner::setTrainingSet(VMat training_set, bool call_forget) {
    bool training_set_has_changed = !train_set || !(train_set->looksTheSameAs(training_set));
    if (compute_costs_on_bags) {
        // We need to remove the bag information (assumed to be in the last column
        // of the target) when giving the training set to the sub learner.
        // TODO Write a SubTargetVMatrix to make it easier.
        if (training_set->inputsize() < 0 || training_set->targetsize() < 0) {
            PLERROR("In AddCostToLearner::setTrainingSet - The inputsize and / or targetsize of the training set isn't specified");
        }
        VMat sub_training_set;
        if (training_set->weightsize() > 0) {
            sub_training_set = new ConcatColumnsVMatrix(
                new SubVMatrix(training_set, 0, 0, training_set->length(), training_set->inputsize() + training_set->targetsize() - 1),
                new SubVMatrix(training_set, 0, training_set->inputsize() + training_set->targetsize(), training_set->length(), training_set->weightsize())
                );
        } else {
            sub_training_set = new SubVMatrix(training_set, 0, 0, training_set->length(), training_set->width() - 1);
        }
        sub_training_set->defineSizes(training_set->inputsize(), training_set->targetsize() - 1, training_set->weightsize());
        learner_->setTrainingSet(sub_training_set, false);
        // 'call_forget' is set to false for the same reason as in EmbeddedLearner.
        if (call_forget && !training_set_has_changed)
            learner_->build(); // See EmbeddedLearner comments.
    } else {
        learner_->setTrainingSet(training_set, false);
        if (call_forget && !training_set_has_changed)
            learner_->build(); // See EmbeddedLearner comments.
    }
    PLearner::setTrainingSet(training_set, call_forget);
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
