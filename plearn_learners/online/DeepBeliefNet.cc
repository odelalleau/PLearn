// -*- C++ -*-

// DeepBeliefNet.cc
//
// Copyright (C) 2006 Pascal Lamblin
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

/*! \file DeepBeliefNet.cc */


#include "DeepBeliefNet.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DeepBeliefNet,
    "ONE LINE DESCR",
    "NO HELP"
);

//////////////////
// DeepBeliefNet //
//////////////////
DeepBeliefNet::DeepBeliefNet()
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

////////////////////
// declareOptions //
////////////////////
void DeepBeliefNet::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &DeepBeliefNet::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void DeepBeliefNet::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void DeepBeliefNet::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    // ### In general, you will want to call this class' specific methods for
    // ### conditional distributions.
    // DeepBeliefNet::setPredictorPredictedSizes(predictor_size,
    //                                          predicted_size,
    //                                          false);
    // DeepBeliefNet::setPredictor(predictor_part, false);
}

/////////
// cdf //
/////////
real DeepBeliefNet::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for DeepBeliefNet"); return 0;
}

/////////////////
// expectation //
/////////////////
void DeepBeliefNet::expectation(Vec& mu) const
{
    PLERROR("expectation not implemented for DeepBeliefNet");
}

// ### Remove this method if your distribution does not implement it.
////////////
// forget //
////////////
void DeepBeliefNet::forget()
{
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    PLERROR("forget method not implemented for DeepBeliefNet");
}

//////////////
// generate //
//////////////
void DeepBeliefNet::generate(Vec& y) const
{
    PLERROR("generate not implemented for DeepBeliefNet");
}

// ### Default version of inputsize returns learner->inputsize()
// ### If this is not appropriate, you should uncomment this and define
// ### it properly here:
// int DeepBeliefNet::inputsize() const {}

/////////////////
// log_density //
/////////////////
real DeepBeliefNet::log_density(const Vec& y) const
{
    PLERROR("density not implemented for DeepBeliefNet"); return 0;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void DeepBeliefNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("DeepBeliefNet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// resetGenerator //
////////////////////
void DeepBeliefNet::resetGenerator(long g_seed) const
{
    PLERROR("resetGenerator not implemented for DeepBeliefNet");
}

//////////////////
// setPredictor //
//////////////////
void DeepBeliefNet::setPredictor(const Vec& predictor, bool call_parent) const
{
    if (call_parent)
        inherited::setPredictor(predictor, true);
    // ### Add here any specific code required by your subclass.
}

////////////////////////////////
// setPredictorPredictedSizes //
////////////////////////////////
bool PDistribution::setPredictorPredictedSizes(int the_predictor_size,
                                               int the_predicted_size,
                                               bool call_parent)
{
    if (call_parent)
        inherited::setPredictorPredictedSizes(the_predictor_size,
                                              the_predicted_size, true);
    // ### Add here any specific code required by your subclass.
}

/////////////////
// survival_fn //
/////////////////
real DeepBeliefNet::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for DeepBeliefNet"); return 0;
}

// ### Remove this method, if your distribution does not implement it.
///////////
// train //
///////////
void DeepBeliefNet::train()
{
    PLERROR("train method not implemented for DeepBeliefNet");
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

//////////////
// variance //
//////////////
void DeepBeliefNet::variance(Mat& covar) const
{
    PLERROR("variance not implemented for DeepBeliefNet");
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
