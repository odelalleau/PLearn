// -*- C++ -*-

// NxProfileLearner.cc
//
// Copyright (C) 2007 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file NxProfileLearner.cc */


#include "NxProfileLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NxProfileLearner,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP");

NxProfileLearner::NxProfileLearner()    :   profile_dim(1),
                                            slr(0.0),
                                            dc(0.0),
                                            L1_penalty_factor(0.0),
                                            L2_penalty_factor(0.0),
                                            n_films(17770),
                                            n_users(480189)
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)

    // ### If this learner needs to generate random numbers, uncomment the
    // ### line below to enable the use of the inherited PRandom object.
    // random_gen = new PRandom();
    if( !random_gen)
        random_gen = new PRandom;
}

void NxProfileLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "profile_dim", &NxProfileLearner::profile_dim,
                  OptionBase::buildoption,
                  "Dimension of the profiles to learn.");
    declareOption(ol, "slr", &NxProfileLearner::slr,
                  OptionBase::buildoption,
                  "Starting learning rate.");
    declareOption(ol, "dc", &NxProfileLearner::dc,
                  OptionBase::buildoption,
                  "Learning rate decrease constant.");

    declareOption(ol, "L1_penalty_factor",
                  &NxProfileLearner::L1_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L1 regularization term, i.e.\n"
                  "minimize L1_penalty_factor * sum_{ij} |weights(i,j)| during training.\n"
                  "Gets multiplied by the learning rate.");
    declareOption(ol, "L2_penalty_factor",
                  &NxProfileLearner::L2_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L2 regularization term, i.e.\n"
                  "minimize 0.5 * L2_penalty_factor * sum_{ij} weights(i,j)^2 during training.\n"
                  "Gets multiplied by the learning rate.");

    declareOption(ol, "ngest_films",
                  &NxProfileLearner::ngest_films,
                  OptionBase::buildoption,
                  "Optional NatGradEstimator object for the gradients on the parameters OF ALL USERS!\n"
                  "NOT A TEMPLATE!");

    declareOption(ol, "ngest_users",
                  &NxProfileLearner::ngest_users,
                  OptionBase::buildoption,
                  "Optional NatGradEstimator object for the gradients on the parameters OF ALL FILMS!\n"
                  "NOT A TEMPLATE!");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NxProfileLearner::build_()
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

    if( !train_set )
        return;

    cout << "build()" << endl;

    if( L1_penalty_factor < 0. )
        PLWARNING("NxProfileLearner::build:\n"
                    "L1_penalty_factor is negative!\n", slr);
    if( L2_penalty_factor < 0. )
        PLWARNING("NxProfileLearner::build:\n"
                    "L2_penalty_factor is negative!\n", slr);
    if( (slr*L2_penalty_factor) > 1. )
        PLWARNING("NxProfileLearner::build:\n"
                    "slr = %f is too large for L2_penalty_factor!\n", slr);

    f_profiles.resize(n_films, profile_dim);     //! matrix of film profiles (n_films*profile_dim)
    u_profiles.resize(n_users, profile_dim);     //! matrix of user profiles (n_users*profile_dim)

    forget();
}

// ### Nothing to add here, simply calls build_
void NxProfileLearner::build()
{
    inherited::build();
    build_();
}


void NxProfileLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(ngest_films, copies);
    deepCopyField(ngest_users, copies);
    deepCopyField(f_profiles, copies);
    deepCopyField(u_profiles, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("NxProfileLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int NxProfileLearner::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).
    return 1;
}

void NxProfileLearner::forget()
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

    cout << "forget" << endl;

    //real delta = 1/sqrt(real(layer_sizes[i]));
    real delta =  1.0/sqrt(real(profile_dim));;
    random_gen->fill_random_uniform(u_profiles,-delta,delta);
    random_gen->fill_random_uniform(f_profiles,-delta,delta);
    stage = 0;

}

void NxProfileLearner::train()
{
    // The role of the train method is to bring the learner up to
    // stage==nstages, updating train_stats with training costs measured
    // on-line in the process.

    static Vec input;  // static so we don't reallocate memory each time...
    static Vec target; // (but be careful that static means shared!)
    input.resize(inputsize());    // the train_set's inputsize()
    target.resize(targetsize());  // the train_set's targetsize()
    real weight, error, lr;

    static Vec f_grad, f_natgrad; 
    static Vec u_grad, u_natgrad;
    f_grad.resize(profile_dim);
    f_natgrad.resize(profile_dim);
    u_grad.resize(profile_dim);
    u_natgrad.resize(profile_dim);

    // indexes for natural gradient estimator calls
    // Since calls must have continuous 't's, and we exceed the int limit
    // we need to do some hacking
    static int ngf_idx=0;
    static int ngu_idx=0;

    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    int nsamples = train_set->length();
    // clear statistics of previous epoch
    train_stats->forget();


    while(stage<nstages)
    {
        PP<ProgressBar> pb;
        if( report_progress )
            pb = new ProgressBar( "Training "+classname(), nsamples);

        // TODO In case minibarches are used, remember to modify in the case of
        // natural gradient updates.        
        lr = slr/(1.0 + stage*dc);
        real L1_delta = lr * L1_penalty_factor;
        real L2_scaling = 1. - lr * L2_penalty_factor;

        for(int i=0; i<nsamples; i++)   {
            train_set->getExample(i, input, target, weight);

            PLASSERT( (input[0]>=0) && (input[0]<n_films) && (input[1]>=0) && (input[1]<n_users) );

            // Save a function call by not using the functions (computeOutput,
            // etc.). Also, we're using squared error cost, but dropping the 2 and taking
            // the negative already.
            error = target[0] - dot( f_profiles((int)input[0]), u_profiles((int)input[1]) );

            // WHAT FOLLOWS SHOULD PROBABLY BE MADE MORE EFFICIENT
            // examples:
            // - for normal gradient multiply lr and error (2 scalars) 
            // before multiplying the vector
            // - Consider operating on the vector's elements
            //- Use the updated film parameters in the user's update

            // the gradients
            f_grad = error * u_profiles((int)input[1]);
            u_grad = error * f_profiles((int)input[0]);

/*            // Update the parameters
            if( !ngest_films )  {
                f_profiles((int)input[0]) += lr * f_grad;
            }   else    {
                (*ngest_films)( ngf_idx, f_grad, f_natgrad );
                // do index shananigans
                ngf_idx++;
                ngf_idx = ngf_idx%ngest_films->cov_minibatch_size + ngest_films->cov_minibatch_size;
                ngest_films->previous_t = ngf_idx-1;
                // perform parameter update
                f_profiles((int)input[0]) += lr * f_natgrad;
            }

            if( !ngest_users )  {
                u_profiles((int)input[1]) += lr * u_grad;
            }   else    {
                (*ngest_users)( ngu_idx, u_grad, u_natgrad );
                // do index shananigans
                ngu_idx++;
                ngu_idx = ngu_idx%ngest_users->cov_minibatch_size + ngest_users->cov_minibatch_size;
                ngest_users->previous_t = ngu_idx-1;
                // perform parameter update
                u_profiles((int)input[1]) += lr * u_natgrad;
            }
*/
            // L1 regularization
            if( L1_penalty_factor != 0. )    {
                for( int d=0; d<profile_dim; d++ )  {
                    // films
                    if( f_profiles((int)input[0], d) > L1_delta )
                        f_profiles((int)input[0], d) -= L1_delta;
                    else if( f_profiles((int)input[0], d) < -L1_delta )
                        f_profiles((int)input[0], d) += L1_delta;
                    else
                        f_profiles((int)input[0], d) = 0.;
                    // users
                    if( u_profiles((int)input[1], d) > L1_delta )
                        u_profiles((int)input[1], d) -= L1_delta;
                    else if( u_profiles((int)input[1], d) < -L1_delta )
                        u_profiles((int)input[1], d) += L1_delta;
                    else
                        u_profiles((int)input[1], d) = 0.;
                }
            }

            // L2 regularization
            if( L2_penalty_factor != 0. )    {
                f_profiles((int)input[0]) *= L2_scaling;
                u_profiles((int)input[1]) *= L2_scaling;
            }

            //train_stats->update(train_costs)
            if( pb )
                pb->update(i);

        }
        ++stage;
        train_stats->finalize(); // finalize statistics for this epoch
    }


}

// Compute the output from the input.
void NxProfileLearner::computeOutput(const Vec& input, Vec& output) const
{
    int nout = outputsize();
    output.resize(nout);

    PLASSERT( (input[0]>=0) && (input[0]<n_films) && (input[1]>=0) && (input[1]<n_users) );

    output[0] = dot( f_profiles((int)input[0]), u_profiles((int)input[1]) );

/*    cout << " f " << filmProfileID << " " << f_profiles(filmProfileID) << endl;
    cout << " u " << userProfileID << " " << u_profiles(userProfileID) << endl;
    cout << "output[0] " << output[0];*/
}

// Compute the costs from *already* computed output.
void NxProfileLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    real error = target[0] - output[0];
    // the 16 is to put the error on the 1-5 rating basis
    costs[0] = 16.0 * error * error;
//cout << " error " << error << " cost[0] " << costs[0] << endl;
}

TVec<string> NxProfileLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).
    return getTrainCostNames();
}

TVec<string> NxProfileLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes
    // and for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by
    // getTestCostNames).
    TVec<string> costs;
    costs.resize(1);
    costs[0]="MSE";
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
