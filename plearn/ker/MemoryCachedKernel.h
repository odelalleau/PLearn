// -*- C++ -*-

// MemoryCachedKernel.h
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

/*! \file MemoryCachedKernel.h */


#ifndef MemoryCachedKernel_INC
#define MemoryCachedKernel_INC

#include <plearn/ker/Kernel.h>

namespace PLearn {

/**
 *  Provide some memory-management utilities for kernels.
 *
 *  This class is intended as a base class to provide some memory management
 *  utilities for the data-matrix set with setDataForKernelMatrix function.  In
 *  particular, it provides a single (inline, non-virtual) function to access a
 *  given input vector of the data matrix.  If the data VMatrix passed to
 *  setDataForKernelMatrix is within a certain size threshold, the VMatrix is
 *  converted to a Mat and cached to memory (without requiring additional space
 *  if the VMatrix is actually a MemoryVMatrix), and all further element access
 *  are done without requiring virtual function calls.
 *
 *  IMPORTANT NOTE: the 'cache_gram_matrix' option is enabled automatically by
 *  default for this class.  This makes the computation of the Gram matrix
 *  derivatives (with respect to kernel hyperparameters) quite faster in many
 *  cases.  If you really don't want this caching to occur, just set it
 *  explicitly to false.
 *
 *  This class also provides utility functions to derived classes to compute
 *  the Gram matrix and its derivative (with respect to kernel hyperparameters)
 *  without requiring virtual function calls in data access or evaluation
 *  function.
 */
class MemoryCachedKernel : public Kernel
{
    typedef Kernel inherited;

public:
    //#####  Public Build Options  ############################################

    /**
     *  Threshold on the number of elements to cache the data VMatrix into a
     *  real matrix.  Above this threshold, the VMatrix is left as-is, and
     *  element access remains virtual.  (Default value = 1000000)
     */
    int m_cache_threshold;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    MemoryCachedKernel();


    //#####  Kernel Member Functions  #########################################

    //! Optionally cache the data to a real Mat if its number of elements lies
    //! within the threshold.
    virtual void setDataForKernelMatrix(VMat the_data);

    //! Update the cache if a new row is added to the data
    virtual void addDataForKernelMatrix(const Vec& newRow);

    //! Return true if the cache is active after setting some data
    bool dataCached() const { return m_data_cache.size() > 0; }


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(MemoryCachedKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    /**
     *  Interface for derived classes: access row i of the data matrix.  Note:
     *  the contents of the Vec SHOULD ABSOLUTELY NOT BE MODIFIED after calling
     *  this function.  For performance, the Vec may not contain a copy of the
     *  input vector, but may point to the original data
     */
    inline void dataRow(int i, Vec& row) const;

    /**
     *  Interface for derived classes: access row i of the data matrix and
     *  return it as a POINTER to a Vec.  NOTE: this version ASSUMES that the
     *  cache exists.  You can verify this with the dataCached() function.
     */
    inline Vec* dataRow(int i) const;

    /**
     *  Interface to ease derived-class implementation of computeGramMatrix
     *  that avoids virtual function calls in kernel evaluation.  The
     *  computeGramMatrixNV function should be called directly by the
     *  implementation of computeGramMatrix in a derived class, passing the
     *  name of the derived class as a template argument.
     */
    template <class DerivedClass>
    void computeGramMatrixNV(Mat K, const DerivedClass* This) const;

    /**
     *  Interface to ease derived-class implementation of
     *  computeGramMatrixDerivative, that avoids virtual function calls as much
     *  as possible.  This template is instantiated with a member function
     *  pointer in the derived class to compute the actual element-wise
     *  derivative (with respect to some kernel hyperparameter, depending on
     *  which a different member pointer is passed).  Both GCC 3.3.6 and 4.0.3
     *  (which I tested on) generate very efficient code to call a member
     *  function passed as a template argument within a loop [although the
     *  generated code looks very different in both cases].
     *
     *  The member function is called with the following arguments:
     *
     *  - i   : current row i of the data matrix
     *  - j   : current row j of the data matrix
     *  - arg : integer argument passed to the function; may be used to index
     *          into a vector of hyperparameters
     *  - K   : kernel value for (x1,x2); obtained from cache if available
     *
     *  The last argument to computeGramMatrixDerivNV,
     *  'derivative_func_requires_K', specifies whether the derivativeFunc
     *  requires the value of K in order to compute the derivative.  Passing
     *  the value 'false' can avoid unnecessary kernel computations in cases
     *  where the Gram matrix is not cached.  In this case, the derivativeFunc
     *  is called with a MISSING_VALUE for its argument K.
     */
    template <class DerivedClass,
              real (DerivedClass::*derivativeFunc)(int, int, int, real) const>
    void computeGramMatrixDerivNV(Mat& KD, const DerivedClass* This, int arg,
                                  bool derivative_func_requires_K = true) const;
    
    
private:
    //! This does the actual building.
    void build_();

protected:
    //! In-memory cache of the data matrix
    Mat m_data_cache;

    //! Cache of vectors for each row of the data matrix; this avoids
    //! reconstructing a Vec each time we want to access a row.
    TVec<Vec> m_row_cache;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MemoryCachedKernel);


//#####  dataRow  #############################################################

inline void MemoryCachedKernel::dataRow(int i, Vec& row) const
{
    if (m_data_cache.isNotNull()) {
        row = m_data_cache(i);
        row.subVecSelf(0, dataInputsize());
    }
    else {
        row.resize(dataInputsize());
        data->getSubRow(i, 0, row);
    }
}

inline Vec* MemoryCachedKernel::dataRow(int i) const
{
    // Note: ASSUME that the cache exists; will boundcheck in dbg/safeopt if
    // not.
    return &m_row_cache[i];
}


//#####  computeGramMatrixNV  #################################################

template <class DerivedClass>
void MemoryCachedKernel::computeGramMatrixNV(Mat K, const DerivedClass* This) const
{
    if (!data)
        PLERROR("Kernel::computeGramMatrix: setDataForKernelMatrix not yet called");
    if (!is_symmetric)
        PLERROR("Kernel::computeGramMatrix: not supported for non-symmetric kernels");
    if (K.length() != data.length() || K.width() != data.length())
        PLERROR("Kernel::computeGramMatrix: the argument matrix K should be\n"
                "of size %d x %d (currently of size %d x %d)",
                data.length(), data.length(), K.length(), K.width());
    if (cache_gram_matrix && gram_matrix_is_cached) {
        K << gram_matrix;
        return;
    }
                
    int l=data->length();
    int m=K.mod();
    PP<ProgressBar> pb;
    int count = 0;
    if (report_progress)
        pb = new ProgressBar("Computing Gram matrix for " + classname(),
                             (l * (l + 1)) / 2);

    Vec row_i, row_j;
    real Kij;
    real* Ki;
    real* Kji;
    for (int i=0 ; i<l ; ++i) {
        Ki = K[i];
        Kji = &K[0][i];
        dataRow(i, row_i);
        for (int j=0; j<=i; ++j, Kji += m) {
            dataRow(j, row_j);
            Kij = This->DerivedClass::evaluate(row_i, row_j);
            *Ki++ = Kij;
            if (j<i)
                *Kji = Kij;
        }
        if (report_progress) {
            count += i + 1;
            PLASSERT( pb );
            pb->update(count);
        }
    }
    if (cache_gram_matrix) {
        gram_matrix.resize(l,l);
        gram_matrix << K;
        gram_matrix_is_cached = true;
    }
}


//#####  computeGramMatrixDerivNV  ############################################

template <class DerivedClass,
          real (DerivedClass::*derivativeFunc)(int, int, int, real) const>
void MemoryCachedKernel::computeGramMatrixDerivNV(Mat& KD, const DerivedClass* This,
                                                  int arg, bool require_K) const
{
    if (!data)
        PLERROR("Kernel::computeGramMatrixDerivative: "
                "setDataForKernelMatrix not yet called");
    if (!is_symmetric)
        PLERROR("Kernel::computeGramMatrixDerivative: "
                "not supported for non-symmetric kernels");

    int W = nExamples();
    KD.resize(W,W);
    
    real KDij;
    real* KDi;
    real  K  = MISSING_VALUE;
    real* Ki = 0;                       // Current row of kernel matrix, if cached

    for (int i=0 ; i<W ; ++i) {
        KDi  = KD[i];
        if (gram_matrix_is_cached)
            Ki = gram_matrix[i];
        
        for (int j=0 ; j <= i ; ++j) {
            // Access the current kernel value depending on whether it's cached
            if (Ki)
                K = *Ki++;
            else if (require_K) {
                Vec& row_i = *dataRow(i);
                Vec& row_j = *dataRow(j);
                K = This->DerivedClass::evaluate(row_i, row_j);
            }

            // Compute and store the derivative
            KDij   = (This->*derivativeFunc)(i, j, arg, K);
            *KDi++ = KDij;
        }
    }
}



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
