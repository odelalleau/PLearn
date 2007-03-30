// -*- C++ -*-

// NetflixVMatrix.h
//
// Copyright (C) 2006 Pierre-Antoine Manzagol
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

/*! \file NetflixVMatrix.h */


#ifndef NetflixVMatrix_INC
#define NetflixVMatrix_INC

#include <plearn/vmat/VMat.h>

// **********************************************************
// **                    W A R N I N G                     **
// **********************************************************
// **   MOST NEW VMatrix SUBCLASSES SHOULD BE WRITTEN AS   **
// **   SUBCLASSES OF RowBufferedVMatrix INSTEAD.          **
// **   Make sure your REALLY want to subclass VMatrix!!!  **
// **********************************************************

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
class NetflixVMatrix : public VMatrix
{
    typedef VMatrix inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!

    string sourceFileName;
    TVec< TVec< int > > data;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    NetflixVMatrix();


    //#####  VMatrix Member Functions  ########################################

    //! Get element at i-th row, j-th column
    //  (Please implement in .cc)
    virtual real get(int i, int j) const;

    /**
     *  Default version calls getSubRow based on inputsize_ targetsize_
     *  weightsize_ But exotic subclasses may construct, input, target and
     *  weight however they please.  If not a weighted matrix, weight should be
     *  set to default value 1.
     */
    virtual void getExample(int i, Vec& input, Vec& target, real& weight);

    //! Get part or all of the i-th, starting at the j-th column,
    //! with v.length() elements; these elements are put in v.
    //  (Please implement in .cc)
    //  (default version repeatedly calls get(i,j) which may have a significant overhead)
    //virtual void getSubRow(int i, int j, Vec v) const;

    //virtual void getRow(int i, Vec v) const;

    // ** The methods can be optionally implemented
    // ** (their default implementation throws an error)

    //!  in case the dimensions of an underlying vmat has changed, recompute it
    // virtual void reset_dimensions();

    // ** for writable VMatrices

    //! Set element (i,j) to given value
    // virtual void put(int i, int j, real value);

    //! (default version repeatedly calls put(i,j,value) which may have a
    //! significant overhead)
    // virtual void putSubRow(int i, int j, Vec v);

    //! copies matrix m at position i,j of this VMat
    // virtual void putMat(int i, int j, Mat m);

    //! This method must be implemented for matrices that are allowed to grow
    // virtual void appendRow(Vec v);

    //! fill with given value
    // virtual void fill(real value);

    //! For matrices stored on disk, this should flush all pending buffered
    //! write operations
    // virtual void flush();

    //! will call putRow if i<length() and appendRow if i==length()
    // void putOrAppendRow(int i, Vec v);

    //! will call putRow if i<length()
    //! if i>= length(), it will call appendRow with 0 filled rows as many
    //! times as necessary before it can append row i
    // void forcePutRow(int i, Vec v);

    // ** Some other VMatrix methods, that can be redefined if a more
    // ** efficient implementation is possible with
    // ** this particular subclass.
    //

    //! default version returns a SubVMatrix referencing the current VMatrix
    //! however this can be overridden to provide more efficient shortcuts
    //! (see MemoryVMatrix::subMat and SubVMatrix::subMat for examples)
    // virtual VMat subMat(int i, int j, int l, int w);

    //! copies the submatrix starting at i,j into m (which must have
    //! appropriate length and width)
    // virtual void getMat(int i, int j, Mat m) const;

    //! copies column i into v (which must have appropriate length equal to the
    //! VMat's length)
    // virtual void getColumn(int i, Vec v) const;

    /**
     * Returns a Mat with the same data as this VMat.  The default version of
     * this method copies the data in a fresh Mat created in memory However
     * this method will typically be overrided by subclasses (such as
     * MemoryVMatrix) whose internal representation is already a Mat in order
     * to return this Mat directly to avoid a new memory allocation and copy of
     * elements. In this case, and in this case only, modifying the elements of
     * the returned Mat will logically result in modified elements in the
     * original VMatrix view of it.
     */
    // virtual Mat toMat() const;

    //! The default implementation of this method does nothing
    //! But subclasses may overload it to reallocate memory to exactly what is
    //! needed and no more.
    // virtual void compacify();

    /**
     * Returns the dot product between row i1 and row i2 (considering only the
     * inputsize first elements).  The default version in VMatrix is somewhat
     * inefficient, as it repeatedly calls get(i,j) The default version in
     * RowBufferedVMatrix is a little better as it buffers the 2 Vecs between
     * calls in case one of them is needed again.  But the real strength of
     * this method is for specialised and efficient versions in subbclasses.
     * This method is typically used by SmartKernels so that they can compute
     * kernel values between input samples efficiently.
    */
    // virtual real dot(int i1, int i2, int inputsize) const;

    //! Returns the result of the dot product between row i and the given vec
    //! (only v.length() first elements of row i are considered).
    // virtual real dot(int i, const Vec& v) const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(NetflixVMatrix);

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
DECLARE_OBJECT_PTR(NetflixVMatrix);

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
