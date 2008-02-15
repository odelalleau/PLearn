#include "DERIVEDCLASS.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DERIVEDCLASS,
    "ONE LINE DESCR",
    "NO HELP"
);

//////////////////
// DERIVEDCLASS //
//////////////////
DERIVEDCLASS::DERIVEDCLASS()
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
void DERIVEDCLASS::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &DERIVEDCLASS::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void DERIVEDCLASS::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void DERIVEDCLASS::build_()
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
    // DERIVEDCLASS::setPredictorPredictedSizes(predictor_size,
    //                                          predicted_size,
    //                                          false);
    // DERIVEDCLASS::setPredictor(predictor_part, false);
}

/////////
// cdf //
/////////
real DERIVEDCLASS::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for DERIVEDCLASS"); return 0;
}

/////////////////
// expectation //
/////////////////
void DERIVEDCLASS::expectation(Vec& mu) const
{
    PLERROR("expectation not implemented for DERIVEDCLASS");
}

// ### Remove this method if your distribution does not implement it.
////////////
// forget //
////////////
void DERIVEDCLASS::forget()
{
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    PLERROR("forget method not implemented for DERIVEDCLASS");
}

//////////////
// generate //
//////////////
void DERIVEDCLASS::generate(Vec& y) const
{
    PLERROR("generate not implemented for DERIVEDCLASS");
}

// ### Default version of inputsize returns learner->inputsize()
// ### If this is not appropriate, you should uncomment this and define
// ### it properly here:
// int DERIVEDCLASS::inputsize() const {}

/////////////////
// log_density //
/////////////////
real DERIVEDCLASS::log_density(const Vec& y) const
{
    PLERROR("density not implemented for DERIVEDCLASS"); return 0;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void DERIVEDCLASS::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("DERIVEDCLASS::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// resetGenerator //
////////////////////
void DERIVEDCLASS::resetGenerator(long g_seed)
{
    PLERROR("resetGenerator not implemented for DERIVEDCLASS");
}

//////////////////
// setPredictor //
//////////////////
void DERIVEDCLASS::setPredictor(const Vec& predictor, bool call_parent) const
{
    if (call_parent)
        inherited::setPredictor(predictor, true);
    // ### Add here any specific code required by your subclass.
}

////////////////////////////////
// setPredictorPredictedSizes //
////////////////////////////////
bool DERIVEDCLASS::setPredictorPredictedSizes(int the_predictor_size,
                                               int the_predicted_size,
                                               bool call_parent)
{
    bool sizes_have_changed = false;
    if (call_parent)
        sizes_have_changed = inherited::setPredictorPredictedSizes(
                the_predictor_size, the_predicted_size, true);

    // ### Add here any specific code required by your subclass.

    // Returned value.
    return sizes_have_changed;
}

/////////////////
// survival_fn //
/////////////////
real DERIVEDCLASS::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for DERIVEDCLASS"); return 0;
}

// ### Remove this method, if your distribution does not implement it.
///////////
// train //
///////////
void DERIVEDCLASS::train()
{
    PLERROR("train method not implemented for DERIVEDCLASS");
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
void DERIVEDCLASS::variance(Mat& covar) const
{
    PLERROR("variance not implemented for DERIVEDCLASS");
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
