#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include <plearn_learners/online/OnlineLearningModule.h>

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
class DERIVEDCLASS : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

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

    // Your other public member functions go here

    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop; it should be
    //! called with the same arguments as fprop for the first two arguments
    //! (and output should not have been modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
    //! JUST CALLS
    //!     bpropUpdate(input, output, input_gradient, output_gradient)
    //! AND IGNORES INPUT GRADIENT.
    // virtual void bpropUpdate(const Vec& input, const Vec& output,
    //                          const Vec& output_gradient);

    //! this version allows to obtain the input gradient as well
    //! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
    // virtual void bpropUpdate(const Vec& input, const Vec& output,
    //                          Vec& input_gradient,
    //                          const Vec& output_gradient);

    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this
    //! back. If these methods are defined, you can use them INSTEAD of
    //! bpropUpdate(...)
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS,
    //! WHICH JUST CALLS
    //!     bbpropUpdate(input, output, input_gradient, output_gradient,
    //!                  out_hess, in_hess)
    //! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
    // virtual void bbpropUpdate(const Vec& input, const Vec& output,
    //                           const Vec& output_gradient,
    //                           const Vec& output_diag_hessian);

    //! this version allows to obtain the input gradient and diag_hessian
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
    //! RAISES A PLERROR.
    // virtual void bbpropUpdate(const Vec& input, const Vec& output,
    //                           Vec& input_gradient,
    //                           const Vec& output_gradient,
    //                           Vec& input_diag_hessian,
    //                           const Vec& output_diag_hessian);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();


    //! optionally perform some processing after training, or after a
    //! series of fprop/bpropUpdate calls to prepare the model for truly
    //! out-of-sample operation.  THE DEFAULT IMPLEMENTATION PROVIDED IN
    //! THE SUPER-CLASS DOES NOT DO ANYTHING.
    // virtual void finalize();

    //! in case bpropUpdate does not do anything, make it known
    //! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false;
    // virtual bool bpropDoesNothing();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(DERIVEDCLASS);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
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
