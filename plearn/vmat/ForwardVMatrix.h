// ForwardVMatrix.h
// Copyright (C) 2002 Pascal Vincent
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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef ForwardVMatrix_INC
#define ForwardVMatrix_INC

#include "VMat.h"

namespace PLearn {
using namespace std;

class ForwardVMatrix: public VMatrix
{
    typedef VMatrix inherited;

protected:
    VMat vm; // the underlying vmatrix to which all calls will be forwarded

public:

    ForwardVMatrix();
    ForwardVMatrix(VMat vm);

    PLEARN_DECLARE_OBJECT(ForwardVMatrix);
    static void declareOptions(OptionList &ol);

    virtual void build();

    //! This allows to set the underlying vmat
    void setVMat(VMat the_vm);

    //! returns the string associated with value val
    //! for field# col. Or returns "" if no string is associated.
    virtual string getValString(int col, real val) const;

    //! return value associated with a string. Default returns NaN
    virtual real getStringVal(int col, const string & str) const;

    //! returns element as a string, even if its a number (which is always the case unless class is StrTableVMatrix
    virtual string getString(int row,int col) const;

    //! returns the whole string->value mapping
    map<string,real> getStringMapping(int col) const;

    // Overridden from VMatrix.h to forward to the underlying VMat.
    virtual const map<string,real>& getStringToRealMapping(int col) const;
    virtual const map<real,string>& getRealToStringMapping(int col) const;

    virtual void computeStats();

    // default version calls savePMAT
    virtual void save(const PPath& filename) const;

    virtual void savePMAT(const PPath& pmatfile) const;
    virtual void saveDMAT(const PPath& dmatdir) const;
    virtual void saveAMAT(const PPath& amatfile, bool verbose = true,
                          bool no_header = false, bool save_strings = false) const;

    //!  This method must be implemented in all subclasses
    virtual real get(int i, int j) const; //!<  returns element (i,j)

    //!  This method must be implemented in all subclasses of writable matrices
    virtual void put(int i, int j, real value); //!<  sets element (i,j) to value

    //!  It is suggested that this method be implemented in subclasses to speed up accesses
    //!  (default version repeatedly calls get(i,j) which may have a significant overhead)
    virtual void getSubRow(int i, int j, Vec v) const; //!<  fills v with the subrow i lying between columns j (inclusive) and j+v.length() (exclusive)

/*!     It is suggested that this method be implemented in subclasses of writable matrices
  to speed up accesses
  (default version repeatedly calls put(i,j,value) which may have a significant overhead)
*/
    virtual void putSubRow(int i, int j, Vec v);

    //!  This method must be implemented for matrices that are allowed to grow
    virtual void appendRow(Vec v);

    //!  These methods do not usually need to be overridden in subclasses
    //!  (default versions call getSubRow, which should do just fine)
    virtual void getRow(int i, Vec v) const; //!<  copies row i into v (which must have appropriate length equal to the VMat's width)
    virtual void putRow(int i, Vec v);
    virtual void fill(real value);
    virtual void getMat(int i, int j, Mat m) const; //!<  copies the submatrix starting at i,j into m (which must have appropriate length and width)
    virtual void putMat(int i, int j, Mat m); //!<  copies matrix m at position i,j of this VMat

    //!  copies column i into v (which must have appropriate length equal to the VMat's length)
    virtual void getColumn(int i, Vec v) const;

/*!     returns a Mat with the same data as this VMat
  The default version of this method copies the data in a fresh Mat created in memory
  However this method will typically be overrided by subclasses (such as MemoryVMatrix)
  whose internal representation is already a Mat in order to return this Mat directly to avoid
  a new memory allocation and copy of elements. In this case, and in this case only, modifying
  the elements of the returned Mat will logically result in modified elements in the original
  VMatrix view of it.
*/
    virtual Mat toMat() const;

    //!  The default implementation of this method does nothing
    //!  But subclasses may overload it to reallocate memory to exactly what is needed and no more.
    virtual void compacify();

    //!  in case the dimensions of an underlying vmat has changed, recompute it
    virtual void reset_dimensions();

/*!     default version returns a SubVMatrix referencing the current VMatrix
  however this can be overridden to provide more efficient shortcuts
  (see MemoryVMatrix::subMat and SubVMatrix::subMat for examples)
*/
    virtual VMat subMat(int i, int j, int l, int w);

/*!     returns the dot product between row i1 and row i2 (considering only the inputsize first elements).
  The default version in VMatrix is somewhat inefficient, as it repeatedly calls get(i,j)
  The default version in RowBufferedVMatrix is a little better as it buffers the 2 Vecs between calls in case one of them is needed again.
  But the real strength of this method is for specialised and efficient versions in subbclasses.
  This method is typically used by SmartKernels so that they can compute kernel values between input samples efficiently.
*/
    virtual real dot(int i1, int i2, int inputsize) const;

    //!  returns the result of the dot product between row i and the given vec (only v.length() first elements of row i are considered).
    virtual real dot(int i, const Vec& v) const;

    void makeDeepCopyFromShallowCopy(CopiesMap& copies);


/*!     result += transpose(X).Y
  Where X = this->subMatColumns(X_startcol,X_ncols)
  and   Y =  this->subMatColumns(Y_startcol,Y_ncols);
*/
    virtual void accumulateXtY(int X_startcol, int X_ncols, int Y_startcol, int Y_ncols,
                               Mat& result, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;


/*!     A special case of method accumulateXtY
  result += transpose(X).X
  Where X = this->subMatColumns(X_startcol,X_ncols)
*/
    virtual void accumulateXtX(int X_startcol, int X_ncols,
                               Mat& result, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;


private:
    void build_();
};

DECLARE_OBJECT_PTR(ForwardVMatrix);

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
