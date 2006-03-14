#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include <plearn_learners/distributions/PDistribution.h>

namespace PLearn {

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class DERIVEDCLASS : public UnconditionalDistribution
{
    typedef UnconditionalDistribution inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    DERIVEDCLASS();


    //#####  UnconditionalDistribution Member Functions  ######################

    //! Return log of probability density log(p(y)).
    virtual real log_density(const Vec& x) const;

    //! Return survival function: P(Y>y).
    virtual real survival_fn(const Vec& y) const;

    //! Return cdf: P(Y<y).
    virtual real cdf(const Vec& y) const;

    //! Return E[Y].
    virtual void expectation(Vec& mu) const;

    //! Return Var[Y].
    virtual void variance(Mat& cov) const;

    //! Return a pseudo-random sample generated from the distribution.
    virtual void generate(Vec& y) const;

    //! Reset the random number generator used by generate() using the
    //! given seed.
    virtual void resetGenerator(long g_seed) const;

    // ### These methods may be overridden for efficiency purpose:
    /*
    //! Return probability density p(y)
    virtual real density(const Vec& y) const;
    */


    //#####  PLearner Member Functions  #######################################

    // ### Default version of inputsize returns learner->inputsize()
    // ### If this is not appropriate, you should uncomment this and define
    // ### it properly in the .cc
    // virtual int inputsize() const;

    //! (Re-)initializes the PDistribution in its fresh state (that state may
    //! depend on the 'seed' option) and sets 'stage' back to 0 (this is the
    //! stage of a fresh learner!).
    // ### You may remove this method if your distribution does not
    // ### implement it.
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage == nstages, updating the train_stats collector with training
    //! costs measured on-line in the process.
    // ### You may remove this method if your distribution does not
    // ### implement it.
    virtual void train();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    // ...

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(DERIVEDCLASS);

} // end of namespace PLearn

#endif


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
