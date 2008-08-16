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
RBMDistribution::RBMDistribution():
    n_gibbs_chains(-1),
    unnormalized_density(false)
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

    declareOption(ol, "n_gibbs_chains", &RBMDistribution::n_gibbs_chains,
                  OptionBase::buildoption,
        "Number of Gibbs chains ran in parallel when generating multiple\n"
        "samples with generateN(). If <0, then there are as many chains as\n"
        "samples. If in the (0,1) interval, then it is the given fraction of\n"
        "the number of generated samples. If an integer >= 1, it is the\n"
        "absolute number of chains that are run simultaneously. Each chain\n"
        "will sample about N/n_chains samples, so as to obtain N samples.");

    declareOption(ol, "unnormalized_density",
                  &RBMDistribution::unnormalized_density,
                  OptionBase::buildoption,
        "If set to True, then the density will not be normalized (so the\n"
        "partition function does not need to be computed). This means the\n"
        "value returned by the 'log_density' method will instead be the\n"
        "negative free energy of the visible input.");

    declareOption(ol, "sample_data",
                  &RBMDistribution::sample_data,
                  OptionBase::buildoption,
        "If provided, this data will be used to initialize the Gibbs\n"
        "chains when generating samples.");
 
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
    work1.resize(0, 0);
    ports_val.fill(NULL);
    ports_val[rbm->getPortIndex("visible_sample")] = &work1;
    if (sample_data) {
        // Pick a random sample to initialize the Gibbs chain.
        int init_i =
            random_gen->uniform_multinomial_sample(sample_data->length());
        real dummy_weight;
        work3.resize(1, sample_data->inputsize());
        Vec w3 = work3.toVec();
        sample_data->getExample(init_i, w3, workv1, dummy_weight);
        ports_val[rbm->getPortIndex("visible")] = &work2;
    }
    rbm->fprop(ports_val);
    y.resize(work1.width());
    y << work1(0);
}

///////////////
// generateN //
///////////////
void RBMDistribution::generateN(const Mat& Y) const
{
    int n = Y.length(); // Number of samples to obtain.
    int n_chains = Y.length();
    if (n_gibbs_chains > 0 && n_gibbs_chains < 1) {
        // Fraction.
        n_chains = min(1, int(round(n_gibbs_chains * n)));
    } else if (n_gibbs_chains > 0) {
        n_chains = int(round(n_gibbs_chains));
        PLCHECK( is_equal(real(n_chains), n_gibbs_chains) );
    }
    int n_gibbs_samples = n / n_chains;
    if (n % n_chains > 0)
        n_gibbs_samples += 1;
    work2.resize(n_chains * n_gibbs_samples, Y.width());
    PP<ProgressBar> pb = verbosity && work2.length() > 10
        ? new ProgressBar("Gibbs sampling", work2.length())
        : NULL;
    int idx = 0;
    for (int j = 0; j < n_chains; j++) {
        ports_val.fill(NULL);
        if (sample_data) {
            // Pick a sample to initialize the Gibbs chain.
            int init_i;
            if (n_chains == sample_data->length())
                // We use each sample once and only once.
                init_i = j;
            else
                // Pick the sample randomly.
                init_i = random_gen->uniform_multinomial_sample(sample_data->length());
            real dummy_weight;
            work3.resize(1, sample_data->inputsize());
            Vec w3 = work3.toVec();
            sample_data->getExample(init_i, w3, workv1, dummy_weight);
            ports_val[rbm->getPortIndex("visible")] = &work3;
        }
        // Crash if not in the specific case where we have sample data and we
        // compute only 1 sample in each chain. This is because otherwise I
        // (Olivier D.) am not sure the chain is properly (i) restarted for
        // each new chain, and (ii) kept intact when continuing the same chain.
        PLCHECK(sample_data && n_gibbs_samples == 1);
        for (int i = 0; i < n_gibbs_samples; i++) {
            work1.resize(0, 0);
            ports_val[rbm->getPortIndex("visible_sample")] = &work1;
            rbm->fprop(ports_val);
            work2(idx) << work1;
            idx++;
            if (pb)
                pb->update(idx);
        }
    }
    if (n_gibbs_samples > 1)
        // We shuffle rows to add more "randomness" since consecutive samples
        // in the same Gibbs chain may be similar.
        random_gen->shuffleRows(work2);
    Y << work2.subMatRows(0, Y.length());
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
    if (unnormalized_density)
        ports_val[rbm->getPortIndex("energy")] = &work1;
    else
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
