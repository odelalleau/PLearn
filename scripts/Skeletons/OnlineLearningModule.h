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

    /* Optional

    //! given the input, compute the output (possibly resize it  appropriately)
    //! SOON TO BE DEPRECATED, USE fprop(const TVec<Mat*>& ports_value)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Given a batch of inputs, compute the outputs
    //! SOON TO BE DEPRECATED, USE fprop(const TVec<Mat*>& ports_value)
    virtual void fprop(const Mat& inputs, Mat& outputs);
    */

    /* Optional
    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
    //! Adapt based on the output gradient, and obtain the input gradient.
    //! The flag indicates wether the input_gradient is accumulated or set.
    //! This method should only be called just after a corresponding
    //! fprop; it should be called with the same arguments as fprop
    //! for the first two arguments (and output should not have been
    //! modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient,
                             const Vec& output_gradient,
                             bool accumulate=false);

    //! Batch version
    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate=false);
    */

    /* Optional
    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
       A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
       JUST CALLS
            bpropUpdate(input, output, input_gradient, output_gradient)
       AND IGNORES INPUT GRADIENT.
    //! This version does not obtain the input gradient.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);

    //! Batch version
    //! SOON TO BE DEPRECATED, USE bpropAccUpdate(const TVec<Mat*>& ports_value,
    //!                                           const TVec<Mat*>& ports_gradient)
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             const Mat& output_gradients);
    */

    /* Optional
       N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
       RAISES A PLERROR.
    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this
    //! back. If these methods are defined, you can use them INSTEAD of
    //! bpropUpdate(...)
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              Vec& input_gradient,
                              const Vec& output_gradient,
                              Vec& input_diag_hessian,
                              const Vec& output_diag_hessian,
                              bool accumulate=false);
    */

    /* Optional
       N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS,
       WHICH JUST CALLS
            bbpropUpdate(input, output, input_gradient, output_gradient,
                         out_hess, in_hess)
       AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
    //! This version does not obtain the input gradient and diag_hessian.
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              const Vec& output_gradient,
                              const Vec& output_diag_hessian);
    */


    //! Reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();


    /* Optional
       THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT
       DO ANYTHING.
    //! Perform some processing after training, or after a series of
    //! fprop/bpropUpdate calls to prepare the model for truly out-of-sample
    //! operation.
    virtual void finalize();
    */

    /* Optional
       THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false
    //! In case bpropUpdate does not do anything, make it known
    virtual bool bpropDoesNothing();
    */

    /* Optional
       Default implementation prints a warning and does nothing
    //! If this class has a learning rate (or something close to it), set it.
    //! If not, you can redefine this method to get rid of the warning.
    virtual void setLearningRate(real dynamic_learning_rate);
    */

    //! Return the list of ports in the module.
    //! The default implementation returns a pair ("input", "output") to handle
    //! the most common case.
    virtual const TVec<string>& getPorts();

    //! Return the size of all ports, in the form of a two-column matrix, where
    //! each row represents a port, and the two numbers on a row are
    //! respectively its length and its width (with -1 representing an
    //! undefined or variable value).
    //! The default value fills this matrix with:
    //!     - in the first column (lengths): -1
    //!     - in the second column (widths):
    //!         - -1 if nPorts() != 2
    //!         - 'input_size' for the first row and 'output_size' for the
    //!           second row if nPorts() == 2 (also assuming the port names
    //!           are respectively 'input' and 'output')
    virtual const TMat<int>& getPortSizes();

    /* Optional
    //! Return the width of a specific port.
    int getPortWidth(const string& port);

    //! Return the length of a specific port.
    int getPortLength(const string& port);

    //! Return the number of ports in the module.
    int nPorts();

    //! Return the index (as in the list of ports returned by getPorts()) of
    //! a given port.
    //! If 'port' does not exist, -1 is returned.
    int getPortIndex(const string& port);

    //! Return name of the i-th port.
    string getPortName(int i);

    //! Return a list of strings, that represents the description of the values
    //! taken by a given port: the i-th string is the name for the i-th column
    //! value computed in 'port'.
    //! The default version returns [ "port_name_1", ..., "port_name_n" ] where
    //! 'port_name' is the name of the port, and 'n' its size.
    virtual TVec<string> getPortDescription(const string& port);
    */

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
