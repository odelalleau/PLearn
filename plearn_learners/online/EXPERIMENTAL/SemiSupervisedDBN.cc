// -*- C++ -*-

// SemiSupervisedDBN.cc
//
// Copyright (C) 2008 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file SemiSupervisedDBN.cc */


#include "SemiSupervisedDBN.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SemiSupervisedDBN,
    "Deep Belief Net, possibly supervised, trained only with CD",
    "");

SemiSupervisedDBN::SemiSupervisedDBN():
    learning_rate(0),
    n_classes(0),
    share_layers(false),
    n_layers(0)
{
    random_gen = new PRandom();
}

void SemiSupervisedDBN::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &SemiSupervisedDBN::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    declareOption(ol, "", &SemiSupervisedDBN::,
                  OptionBase::buildoption,
                  "");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void SemiSupervisedDBN::build_()
{
    bool rbms_need_build = false;
    if (layer_sizes.length() != rbms.size()+1)
        rbms_need_build = true;
    else
    {
        n_rbms = rbms.size();
        for (int i=0; i<n_rbms; i++)
        {
            if (layer_sizes[i] != rbms[i]->input_size)
            {
                rbms_need_build = true;
                break;
            }
            if (layer_is_supervised[i] && (rbms[i]->target_size != n_target))
            {
                rbms_need_build = true;
                break;
            }
            if (!layer_is_supervised[i] && (rbms[i]->target_size != 0))
            {
                rbms_need_build = true;
                break;
            }
            if (layer_sizes[i+1] != rbms[i]->hidden_size)
            {
                rbms_need_build = true;
                break;
            }
        }
    }

    if (rbms_need_build)
        build_rbms();
}

void SemiSupervisedDBN::build_rbms()
{
    n_layers = layer_sizes.length();
    n_rbms = n_layers - 1;
    rbms.resize(n_rbms);
    for (int i=0; i<n_rbms; i++)
    {
        if (rbms[i].isNull())
            rbms[i] = new InferenceRBM;

        if (i==0 && first_layer_type == "gaussian")
            rbms[i]->input_layer = new RBMGaussianLayer(layer_sizes[0]);
        else if (i>0 && share_layers)
            rbms[i]->input_layer = rbms[i-1]->hidden_layer;
        else
            rbms[i]->input_layer = new RBMGaussianLayer(layer_sizes[i]);


        if (layer_is_supervised[i])
            rbms[i]->target = new RBMMultinomialLayer(n_classes);

        rbms[i]->hidden = new RBMBinomialLayer(layer_sizes[i+1]);

        rbms[i]->random_gen = random_gen;
        rbms[i]->build();
        rbms[i]->setLearningRate(learning_rate);
    }
}


void SemiSupervisedDBN::build()
{
    inherited::build();
    build_();
}


void SemiSupervisedDBN::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("SemiSupervisedDBN::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int SemiSupervisedDBN::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).
}

void SemiSupervisedDBN::forget()
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
}

void SemiSupervisedDBN::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    /* TYPICAL CODE:

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    while(stage<nstages)
    {
        // clear statistics of previous epoch
        train_stats->forget();

        //... train for 1 stage, and update train_stats,
        // using train_set->getExample(input, target, weight)
        // and train_stats->update(train_costs)

        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }
    */
}


void SemiSupervisedDBN::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);
    // ...
}

void SemiSupervisedDBN::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output.
// ...
}

TVec<string> SemiSupervisedDBN::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).
    // ...
}

TVec<string> SemiSupervisedDBN::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by
    // getTestCostNames).
    // ...
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
