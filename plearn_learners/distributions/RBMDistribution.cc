// -*- C++ -*-

// RBMDistribution.cc
//
// Copyright (C) 2008 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file RBMDistribution.cc */


#include "RBMDistribution.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMDistribution,
    "Distribution learnt by a Restricted Boltzmann Machine.",
    "The RBM is train by standard Contrastive Divergence in online mode."
);

/////////////////////
// RBMDistribution //
/////////////////////
RBMDistribution::RBMDistribution()
{}

////////////////////
// declareOptions //
////////////////////
void RBMDistribution::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "rbm", &RBMDistribution::rbm,
                  OptionBase::buildoption,
        "Underlying RBM modeling the distribution.");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void RBMDistribution::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void RBMDistribution::build_()
{
    if (!rbm)
        return;
    int n_ports = rbm->nPorts();
    ports_val.resize(n_ports);
    predicted_size = rbm->visible_layer->size;
    // Rebuild the PDistribution object to take size into account.
    inherited::build();
}

/////////
// cdf //
/////////
real RBMDistribution::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for RBMDistribution"); return 0;
}

/////////////////
// expectation //
/////////////////
void RBMDistribution::expectation(Vec& mu) const
{
    PLERROR("In RBMDistribution::expectation - Not implemeted");
}

////////////
// forget //
////////////
void RBMDistribution::forget()
{
    rbm->forget();
    learner = NULL;
    inherited::forget();
    n_predicted = rbm->visible_layer->size;
}

//////////////
// generate //
//////////////
void RBMDistribution::generate(Vec& y) const
{
    work1.resize(1, 0);
    ports_val.fill(NULL);
    ports_val[rbm->getPortIndex("visible_sample")] = &work1;
    rbm->fprop(ports_val);
    y.resize(work1.width());
    y << work1(0);
}

///////////////
// generateN //
///////////////
void RBMDistribution::generateN(const Mat& Y) const
{
    work1.resize(Y.length(), 0);
    ports_val.fill(NULL);
    ports_val[rbm->getPortIndex("visible_sample")] = &work1;
    rbm->fprop(ports_val);
    Y << work1;
}

/////////////////
// log_density //
/////////////////
real RBMDistribution::log_density(const Vec& y) const
{
    ports_val.fill(NULL);
    work1.resize(1, 0);
    work2.resize(1, y.length());
    work2 << y;
    ports_val[rbm->getPortIndex("neg_log_likelihood")] = &work1;
    ports_val[rbm->getPortIndex("visible")] = &work2;
    rbm->fprop(ports_val);
    return -work1(0, 0);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RBMDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("RBMDistribution::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// resetGenerator //
////////////////////
void RBMDistribution::resetGenerator(long g_seed)
{
    if (!rbm->random_gen)
        PLERROR("In RBMDistribution::resetGenerator - The underlying RBM "
                "must have a random number generator");
    if (g_seed != 0)
        rbm->random_gen->manual_seed(g_seed);
    inherited::resetGenerator(g_seed);
}

/////////////////
// survival_fn //
/////////////////
real RBMDistribution::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for RBMDistribution"); return 0;
}

///////////
// train //
///////////
void RBMDistribution::train()
{
    if (!learner) {
        // First build the learner that will train a RBM.
        learner = new ModuleLearner();
        learner->module = rbm;
        learner->seed_ = this->seed_;
        learner->use_a_separate_random_generator_for_testing =
            this->use_a_separate_random_generator_for_testing;
        learner->input_ports = TVec<string>(1, "visible");
        learner->target_ports.resize(0);
        learner->cost_ports.resize(0);
        learner->build();
        learner->setTrainingSet(this->train_set);
    }
    learner->nstages = this->nstages;
    learner->train();
    this->stage = learner->stage;
}

//////////////
// variance //
//////////////
void RBMDistribution::variance(Mat& covar) const
{
    PLERROR("variance not implemented for RBMDistribution");
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
