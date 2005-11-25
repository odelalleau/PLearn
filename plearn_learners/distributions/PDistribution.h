// -*- C++ -*-

// PDistribution.h
//
// Copyright (C) 2003 Pascal Vincent 
// Copyright (C) 2004-2005 University of Montreal
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

/*! \file PDistribution.h */
#ifndef PDistribution_INC
#define PDistribution_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/math/PRandom.h>

namespace PLearn {
using namespace std;

//! Note that many methods are declared as 'const' because of the 'const'
//! plague, but are actually not true 'const' methods.
//! This is also why a lot of stuff is mutable.
// TODO Would it be possible to remove some 'const' stuff for cleaner code?
class PDistribution: public PLearner
{

private:

    typedef PLearner inherited;

    //! Global storage to save memory allocations.
    mutable Vec store_expect, store_result;
    mutable Mat store_cov;

protected:

    //! Random number generator.
    PP<PRandom> random;

    //! The step when plotting the curve (upper case outputs_def).
    real delta_curve;

    mutable Vec input_part;       //!< Used to store the x part in p(y|x).
    mutable Vec target_part;      //!< Used to store the y part in p(y|x).

    // *********************
    // * protected options *
    // *********************

    mutable int n_input;
    mutable int n_target;
    int n_input_;
    int n_target_;

public:

    // ************************
    // * public build options *
    // ************************

    real lower_bound, upper_bound;
    int n_curve_points;
    string outputs_def; // TODO Replace this by a TVec<string>

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    PDistribution();

    // ********************
    // * PLearner methods *
    // ********************

private: 

    //! This does the actual building. 
    void build_();

protected: 

    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods
    PLEARN_DECLARE_OBJECT(PDistribution);

    // **************************
    // **** PLearner methods ****
    // **************************

    //! Returned value depends on outputs_def.
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    virtual void forget();

    //! The role of the train method is to bring the learner up to stage==nstages,
    //! updating the train_stats collector with training costs measured on-line in the process.
    virtual void train();

    //! Produce outputs according to what is specified in outputs_def.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes negative log likelihood (NLL). If the first output is neither
    //! the log density nor the density, an error will be raised.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;

    // *******************************
    // **** PDistribution methods ****
    // *******************************

protected:

    //! Split an input into the part corresponding to the 'real' input (in
    //! 'input_part'), and the target (in 'target_part').
    //! Also call setInput(..) with the new input part.
    //! If 'input' turns out to only have a target part (i.e. its length is
    //! equal to 'n_target'), then no input part will be set (it is assumed
    //! to stay the same as before).
    void splitCond(const Vec& input) const;

    //! Called in computeOutput when an unknown character is found.
    // TODO Can we find a better way to perform this?
    virtual void unknownOutput(char def, const Vec& input, Vec& output, int& k) const;

public:

    //! Set the 'input' and 'target' sizes for this distribution.
    //! 'n_input' is the size of the input, i.e. of x in p(y|x).
    //! 'n_target' is the size of the target, i.e. of y in p(y|x).
    //! This is a virtual method: if 'call_parent' is set to true, then the
    //! inherited::setInputTargetSizes(..) method will also be called, with the
    //! same arguments: this is useful in the build process, where each class
    //! can call only its own method by setting 'call_parent' to false.
    //! Note that semantically, this method should not be 'const': however, one
    //! may need to call it from other 'const' methods, so that it is 'const'
    //! for convenience.
    virtual bool setInputTargetSizes(int n_input, int n_target,
                                     bool call_parent = true) const;

    //! Set the value for the input part of a conditional probability.
    //! This needs to be implemented in subclasses if there is something
    //! special to do (like precomputing some stuff).
    //! The default behavior is just to fill 'input_part' with the first
    //! 'n_input' elements of 'input'.
    //! As with 'setInputTargetSizes(..)', the boolean 'call_parent' indicates
    //! whether or not one should call inherited::setInput(..) with the same
    //! arguments.
    virtual void setInput(const Vec& input, bool call_parent = true) const;

    //! Return [ "NLL" ] (the only cost computed by a PDistribution).
    virtual TVec<string> getTestCostNames() const;

    //! Return [ ].
    virtual TVec<string> getTrainCostNames() const;

    //! Return log of probability density log(p(y | x)).
    virtual real log_density(const Vec& y) const;

    //! Return probability density p(y | x) (default version simply returns
    //! exp(log_density(y))).
    virtual real density(const Vec& y) const;
  
    //! Return survival function: P(Y>y | x).
    virtual real survival_fn(const Vec& y) const;

    //! Return cdf: P(Y<y | x).
    virtual real cdf(const Vec& y) const;

    //! Return E[Y | x].
    virtual void expectation(Vec& mu) const;

    //! Return Var[Y | x].
    virtual void variance(Mat& cov) const;

    //! Reset the random number generator used by generate() using the given seed.
    //! Default behavior is to call random->manual_seed(g_seed) and to save the
    //! given seed.
    //! This method is called in build().
    virtual void resetGenerator(long g_seed);
  
    //! Return a pseudo-random sample generated from the distribution.
    virtual void generate(Vec& y) const;

    //! X must be a N x n_target matrix. that will be filled.
    //! This will call generate N times to fill the N rows of the matrix. 
    void generateN(const Mat& Y) const;

    //! 'Get' accessor.
    int getNTarget() { return n_target; }

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PDistribution);
  
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
