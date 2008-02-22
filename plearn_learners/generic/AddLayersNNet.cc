// -*- C++ -*-

// AddLayersNNet.cc
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

/*! \file AddLayersNNet.cc */


#include "AddLayersNNet.h"
#include <plearn/math/random.h>      //!< For the seed stuff.
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/SubMatVariable.h>

namespace PLearn {
using namespace std;

//////////////////
// AddLayersNNet //
//////////////////
AddLayersNNet::AddLayersNNet() 
    : added_hidden_transfer_func("tanh")
{}

PLEARN_IMPLEMENT_OBJECT(AddLayersNNet,
                        "This subclass of NNet allows one to add a hidden layer, possibly only for parts of the input.",
                        "The hidden layer is added before the first hidden layer of NNet. You can't add\n"
                        "two successive hidden layers, but you can add a hidden layer for each part of the\n"
                        "input. The input is divided in parts by the 'parts_size' option, and for each part\n"
                        "you can specify how many hidden units we add, with the 'add_hidden' option. If no\n"
                        "hidden layer is added for a part, this part is directly connected to the first\n"
                        "hidden layer of the classical NNet. For each part, a different hidden layer is\n"
                        "created, so that if you want two parts to use the same hidden layer, you should\n"
                        "concatenate those parts into a single one.\n"
                        "In the simple case where you just want to add a single hidden layer, you should set:\n"
                        " - parts_size = [ -1 ]\n"
                        " - add_hidden = [ number_of_hidden_units_added ]\n"
    );

////////////////////
// declareOptions //
////////////////////
void AddLayersNNet::declareOptions(OptionList& ol)
{
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // Build options.

    declareOption(ol, "parts_size", &AddLayersNNet::parts_size, OptionBase::buildoption,
                  "The size of each part. '-1' can be used to specify this part's size should\n"
                  "be such that all inputs are considered ('-1' can thus only appear once).");

    declareOption(ol, "add_hidden", &AddLayersNNet::add_hidden, OptionBase::buildoption,
                  "Specify for each part how many hidden units we want to add.");

    declareOption(ol, "added_hidden_transfer_func", &AddLayersNNet::added_hidden_transfer_func, OptionBase::buildoption,
                  "The transfer function for the added hidden layers.");

    // Learnt options.

    // declareOption(ol, "myoption", &AddLayersNNet::myoption, OptionBase::learntoption,
    //               "Help text describing this option");

    // Now call the parent class' declareOptions.
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void AddLayersNNet::build()
{
    // We ensure that weights are not filled with random numbers, in order to be
    // able to compare with a classical NNet using the same seed.
    string initialization_method_backup = initialization_method;
    bool do_not_change_params_backup = do_not_change_params;
    initialization_method = "zero";
    do_not_change_params = true;
    inherited::build();
    initialization_method = initialization_method_backup;
    do_not_change_params = do_not_change_params_backup;
    build_();
}

////////////
// build_ //
////////////
void AddLayersNNet::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.

    // Don't do anything if we do not have an inputsize.
    if (inputsize_ < 0)
        return;
    if (parts_size.isEmpty() || add_hidden.isEmpty())
        PLERROR("In AddLayersNNet::build_ - You must fill both 'parts_size' and 'add_hidden'");
    if (parts_size.length() != add_hidden.length())
        PLERROR("In AddLayersNNet::build_ - 'parts_size' and 'add_hidden' must have the same length");
    int n_parts = parts_size.length();
    int count_parts_size = 0;
    bool found_minus_one = false;
    int minus_one_index = -1;
    for (int i = 0; i < n_parts; i++) {
        if (parts_size[i] >= 0) {
            count_parts_size += parts_size[i];
        } else if (parts_size[i] == -1) {
            if (found_minus_one) {
                PLERROR("In AddLayersNNet::build_ - There can be only one '-1' in 'parts_size'");
            } else {
                // There is a '-1'.
                found_minus_one = true;
                minus_one_index = i;
            }
        } else {
            // There is a negative value that is not -1, that should not happen.
            PLERROR("In AddLayersNNet::build_ - Wrong value for parts_size[%d]: %d", i, parts_size[i]);
        }
    }
    if (count_parts_size > inputsize_)
        PLERROR("In AddLayersNNet::build_ - The sum of all parts size (%d) exceeds the inputsize (%d)", count_parts_size, inputsize_);
    if (found_minus_one) {
        real_parts_size.resize(parts_size.length());
        real_parts_size << parts_size;
        real_parts_size[minus_one_index] = inputsize_ - count_parts_size;
    } else {
        real_parts_size = parts_size;
        if (count_parts_size != inputsize_)
            PLERROR("In AddLayersNNet::build_ - The sum of all parts size (%d) is less than inputsize (%d)", count_parts_size, inputsize_);
    }

    // Now we redo the graph of variables, even if there is no added layer
    // (because the weights are not initialized in the parent class, since
    // 'initialization_method' is forced to 'zero' at build time).
  
    params.resize(0);

    // Create a Var for each part.
    VarArray input_parts(n_parts);
    int index = 0;
    for (int i = 0; i < n_parts; i++) {
        input_parts[i] = subMat(input, index, 0, real_parts_size[i], 1);
        input_parts[i]->setName("input_part_" + tostring(i));
        index += real_parts_size[i];
    }

    // Add the required hidden layers.
    hidden_layers.resize(n_parts);
    hidden_weights.resize(n_parts);
    for (int i = 0; i < n_parts; i++) {
        if (add_hidden[i] > 0) {
            Var weights = Var(1 + real_parts_size[i], add_hidden[i], ("w_added_" + tostring(i)).c_str());
            hidden_layers[i] = hiddenLayer(input_parts[i], weights, added_hidden_transfer_func);
            hidden_weights[i] = weights;
            params.append(hidden_weights[i]);
        } else {
            hidden_layers[i] = input_parts[i];
        }
    }

    // Create the concatenated "input" to the regular NNet.
    Var concat_input = vconcat(hidden_layers);

    Var hidden_layer;
    Var before_transfer_func;

    // Build main network graph.
    buildOutputFromInput(concat_input, hidden_layer, before_transfer_func);

    // Build target and weight variables.
    buildTargetAndWeight();

    // Build costs.
    buildCosts(output, target, hidden_layer, before_transfer_func);

    // Shared values hack...
    if (!do_not_change_params) {
        if(paramsvalues.length() == params.nelems())
            params << paramsvalues;
        else
        {
            paramsvalues.resize(params.nelems());
            initializeParams();
            if(optimizer)
                optimizer->reset();
        }
        params.makeSharedValue(paramsvalues);
    }

    // Build functions.
    buildFuncs(input, output, target, sampleweight, NULL);

}

////////////////////
// buildPenalties //
////////////////////
void AddLayersNNet::buildPenalties(const Var& hidden_layer) {
    inherited::buildPenalties(hidden_layer);
    if (hidden_weights.length() != parts_size.length())
        // The hidden weights have not yet been correctly initialized.
        return;
    for (int i = 0; i < parts_size.length(); i++) {
        if (add_hidden[i] > 0 && (weight_decay > 0 || bias_decay > 0)) {
            penalties.append(affine_transform_weight_penalty(hidden_weights[i], weight_decay, bias_decay, penalty_type));
        }
    }
}

//////////////////////////////
// getHiddenUnitsActivation //
//////////////////////////////
Vec AddLayersNNet::getHiddenUnitsActivation(int layer) {
    return hidden_layers[layer]->value;
}

//////////////////////
// getHiddenWeights //
//////////////////////
Mat AddLayersNNet::getHiddenWeights(int layer) {
    return hidden_weights[layer]->matValue;
}

////////////////////////////
// getOutputHiddenWeights //
////////////////////////////
Mat AddLayersNNet::getOutputHiddenWeights(int layer) {
    int count = 0;
    for (int i = 0; i < layer; i++)
        count += real_parts_size[i];
    return w1->matValue.subMatRows(count, add_hidden[layer]);
}

//////////////////////
// initializeParams //
//////////////////////
void AddLayersNNet::initializeParams(bool set_seed) {
    // TODO Remove later...
    if (set_seed) {
        if (seed_>=0)
            manual_seed(seed_);
        else
            PLearn::seed();
    }
    for (int i = 0; i < add_hidden.size(); i++)
        if (add_hidden[i] > 0)
            fillWeights(hidden_weights[i], true);
    inherited::initializeParams(false); // TODO Put this first later.
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AddLayersNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(real_parts_size, copies);
    deepCopyField(hidden_layers, copies);
    deepCopyField(hidden_weights, copies);
    deepCopyField(add_hidden, copies);
    deepCopyField(parts_size, copies);
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
