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
      check_output_consistency(1),
      combine_bag_outputs_method(1),
      compute_costs_on_bags(0),
      force_output_to_target_interval(0),
      from_max(1),
      from_min(-1),
      rescale_output(0),
      rescale_target(0),
      to_max(1),
      to_min(0)
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
                  " - 'class_error': 1 if (t != o), 0 otherwise\n"
                  " - 'binary_class_error': same as class error with output = (o > 0.5)\n"
                  " - 'lift_output': to compute the lift cost (for the positive class)\n"
                  " - 'opposite_lift_output': to compute the lift cost (for the negative) class\n"
                  " - 'cross_entropy': t*log(o) + (1-t)*log(1-o)\n"
                  " - 'mse': the mean squared error (o - t)^2\n"
                  " - 'squared_norm_reconstruction_error': | ||i||^2 - ||o||^2 |\n"
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
        } else {
            PLERROR("In AddCostToLearner::build_ - Invalid cost requested (make sure you are using the new costs syntax)");
        }
    }
    if (n > 0 && display) {
        cout << endl;
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void AddCostToLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                               const Vec& target, Vec& costs) const
{
    int n_original_costs = learner_->nTestCosts();
    // We give only costs.subVec to the sub-learner because it may want to resize it.
    Vec sub_costs = costs.subVec(0, n_original_costs);
    int target_length = target.length();
    if (compute_costs_on_bags) {
        learner_->computeCostsFromOutputs(input, output, target.subVec(0, target_length - 1), sub_costs);
    } else {
        learner_->computeCostsFromOutputs(input, output, target, sub_costs);
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

    for (int i = 0; i < this->costs.length(); i++) {
        string c = this->costs[i];
        int ind_cost = i + n_original_costs;
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
        } else if (c == "class_error") {
            int output_length = sub_learner_output.length();
            bool good = true;
            if (fast_exact_is_equal(output_length, target_length)) {
                for (int k = 0; k < desired_target.length(); k++)
                    if (!fast_exact_is_equal(desired_target[k],
                                             sub_learner_output[k])) {
                        good = false;
                        break;
                    }
            } else if (target_length == 1) {
                // We assume the target is a number between 0 and c-1, and the output
                // is a vector of length c giving the weight for each class.
                good = (argmax(sub_learner_output) == int(desired_target[0]));
            }
            costs[ind_cost] = real(!good);
        } else if (c == "binary_class_error") {
            PLASSERT( target_length == 1 );
            real t = desired_target[0];
            PLASSERT( fast_exact_is_equal(t, 0) || fast_exact_is_equal(t, 1));
            costs[ind_cost] = real((sub_learner_output[0] > 0.5) ==
                                   fast_exact_is_equal(t,0));
        } else if (c == "mse") {
            costs[ind_cost] = powdistance(desired_target, sub_learner_output);
        } else if (c == "squared_norm_reconstruction_error") {
            PLWARNING("In AddCostToLearner::computeCostsFromOutputs - 'squared_norm_reconstruction_error'"
                      " has not been tested yet, please remove this warning if it works correctly");
            costs[ind_cost] = abs(pownorm(input, 2) - pownorm(sub_learner_output, 2));
        } else {
            PLERROR("In AddCostToLearner::computeCostsFromOutputs - Unknown cost");
        }
    }
}                                

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void AddCostToLearner::computeOutputAndCosts(const Vec& input, const Vec& target,
                                             Vec& output, Vec& costs) const {
    PLearner::computeOutputAndCosts(input, target, output, costs);
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
    deepCopyField(combined_output, copies);
    deepCopyField(bag_outputs, copies);
    deepCopyField(cross_entropy_prop, copies);
    varDeepCopyField(cross_entropy_var, copies);
    deepCopyField(desired_target, copies);
    varDeepCopyField(output_var, copies);
    deepCopyField(sub_learner_output, copies);
    deepCopyField(sub_input, copies);
    varDeepCopyField(target_var, copies);
    deepCopyField(costs, copies);
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
