// -*- C++ -*-

// NeuralNetworkARDKernel.h
//
// Copyright (C) 2007 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file NeuralNetworkARDKernel.h */


#ifndef NeuralNetworkARDKernel_INC
#define NeuralNetworkARDKernel_INC

#include <plearn/ker/ARDBaseKernel.h>

namespace PLearn {

/**
 *  Neural network kernel that can be used for Automatic Relevance
 *  Determination
 *
 *  This kernel is designed to be used within a GaussianProcessRegressor.  It
 *  is similar to the "arcsin" kernel of C.E. Rasmussen's GPML code (see
 *  http://www.gaussianprocess.org), but can be used with full Automatic
 *  Relevance Determination (ARD).  It takes the form:
 *
 *    k(x,y) = sf * asin(2*x*P*y / sqrt((1+2*x*P*x)*(1+2*y*P*y))) * k_kron(x,y)
 *
 *  where sf is softplus(isp_signal_sigma), P is softplus(isp_global_sigma +
 *  isp_input_sigma[i])^-2 times the unit matrix, where the x and y vectors on
 *  the right-hand-side have an extra bias (1.0) added in front.  (Note that if
 *  ARD is desired, the number of elements provided for isp_input_sigma must be
 *  ONE MORE than the number of inputs, and the first element of the
 *  isp_input_sigma vector corresponds to this bias).  Also note that in
 *  keeping with Rasmussen and Williams, we raise these elements to the -2
 *  power, so these hyperparameters can be interpreted as true length-scales.
 *  The last factor k_kron(x,y) is the result of the KroneckerBaseKernel
 *  evaluation, or 1.0 if there are no Kronecker terms.  Note that since the
 *  Kronecker terms are incorporated multiplicatively, the very presence of the
 *  term associated to this kernel can be gated by the value of some input
 *  variable(s) (that are incorporated within one or more Kronecker terms).
 *
 *  See SquaredExponentialARDKernel for more information about using this
 *  kernel within a SummationKernel in order to add IID noise to the examples.
 *
 *  Note that to make its operations more robust when used with unconstrained
 *  optimization of hyperparameters, all hyperparameters of this kernel are
 *  specified in the inverse softplus domain.  See IIDNoiseKernel for more
 *  explanations.
 */
class NeuralNetworkARDKernel : public ARDBaseKernel
{
    typedef ARDBaseKernel inherited;

public:
    //#####  Public Build Options  ############################################

    // (No new options other than those inherited)
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    NeuralNetworkARDKernel();


    //#####  Kernel Member Functions  #########################################

    //! Compute K(x1,x2).
    virtual real evaluate(const Vec& x1, const Vec& x2) const;

    //! Compute the Gram Matrix.
    virtual void computeGramMatrix(Mat K) const;
    
    //! Directly compute the derivative with respect to hyperparameters;
    //! for now, this mostly maps to finite differences
    virtual void computeGramMatrixDerivative(Mat& KD, const string& kernel_param,
                                             real epsilon=1e-6) const;
    

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(NeuralNetworkARDKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Derivative function with respect to isp_global_sigma
    real derivIspGlobalSigma(int i, int j, int arg, real K) const;
    
    // Compute derivative w.r.t. isp_signal_sigma for WHOLE MATRIX
    void computeGramMatrixDerivIspSignalSigma(Mat& KD) const;
    
    // Compute derivative w.r.t. isp_input_sigma[arg] for WHOLE MATRIX
    void computeGramMatrixDerivIspInputSigma(Mat& KD, int arg) const;
    
private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NeuralNetworkARDKernel);

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
