// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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

#ifndef CompactCompactVMatrix_INC
#define CompactCompactVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

/*!   Like MemoryVMatrix this class holds the data in memory,
  but it tries to hold it compactly by using single bits
  for binary variables, single bytes for discrete variables
  whose number of possible values is less than 256, and
  unsigned shorts for the others, using a fixed point
  representation.
*/
class CompactVMatrix : public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;
    class SDBVMatrix;

protected:

    //!  Each row of the matrix holds in order: bits, 1-byte symbols, fixed point numbers
    Storage<unsigned char> data;
    int row_n_bytes; //!<  # of bytes per row
    int n_bits; //!<  number of binary symbols per row
    int n_symbols; //!<  number of 1-byte symbols per row
    int n_fixedpoint; //!<  number of fixed point numbers per row
    int n_variables; //!<  = n_bits + n_symbols + n_fixedpoint
    bool one_hot_encoding; //!<  the 1-byte symbols are converted to one-hot encoding by get
public:
    TVec<int> n_symbol_values; //!<  for each 1-byte symbol, the number of possible values
    int nbits() { return n_bits; }
    int nsymbols() { return n_symbols; }
    int nfixedpoint() { return n_fixedpoint; }
protected:
    Vec fixedpoint_min, fixedpoint_max; //!<  the ranges of each number for fixed point encoding
    Vec delta; //!<  (fixedpoint_max-fixedpoint_min)/2^16
    TVec<int> variables_permutation; //!<  this variable is used only when constructed from VMat
/*!                                        and provides the permutation of the original columns
  in order to order them into (bits, bytes, fixedpoint)
  variables_permutation[new_column]=old_column (not in one-hot code)
*/
    static unsigned char n_bits_in_byte[256];
    static void set_n_bits_in_byte();

public:
    int n_last; //!<  used by dotProduct and squareDifference to specify # of last columns to ignore
protected:
    int normal_width;         //!<  the value of width_ when one_hot_encoding=true
public:
    void setOneHotMode(bool on=true);

    // ******************
    // *  Constructors  *
    // ******************
    CompactVMatrix(); //!<  default constructor (for automatic deserialization)

    CompactVMatrix(int the_length, int n_variables, int n_binary, int n_nonbinary_discrete,
                   int n_fixed_point, TVec<int>& n_symbolvalues, Vec& fixed_point_min,
                   Vec& fixed_point_max, bool one_hot_encoding=true);

/*!       Convert a VMat into a CompactVMatrix: this will use the stats
  computed in the fieldstats of the VMatrix (they will be computed if not
  already) to figure out which variables are binary, discrete
  (and how many symbols), and the ranges of numeric variables.
  THE VMAT DISCRETE VARIABLES MUST NOT BE ALREADY ONE-HOT ENCODED.
  The variables will be permuted according to the permutation vector
  which can be retrieved from the variables_permutation_vector() method.
  By default the last column of the VMat will stay last, thus being coded as
  fixedpoint (so the permutation information may not be necessary if the
  last column represents a target and all the previous ones some inputs.
  keep_last_variables_last is the number of "last columns" to keep in place.
*/
    CompactVMatrix(VMat m, int keep_last_variables_last=1, bool onehot_encoding=true);

    //!  construct from saved CompactVMatrix

    CompactVMatrix(const string& filename, int nlast=1);

/*!       Create a CompactVMatrix with the same structure as cvcm but
  containing the data in m. Both must obviously have the same width.
  If rescale is true, then the min/max values for fixed-point encoding
  are recomputed. If check==true than this is verified and an error message
  is thrown if the floating point data are not in the expected ranges (of cvm).
*/
    CompactVMatrix(CompactVMatrix* cvm, VMat m, bool rescale=false, bool check=true);

    //!  append vm to this VMatrix (the rows of vm are concatenated to the current rows of this VMatrix)
    void append(CompactVMatrix* vm);

/*!       create in the elements of row (except the n_last ones) a perturbed
  version of the i-th row of the database. This
  random perturbation is based on the unconditional
  statistics which should be present in the fieldstats; the
  noise level can be modulated with the noise_level argument
  (a value of 1 will perturb by as much as the noise seen in the
  unconditional statistics). Continuous variables are resampled around
  the current value with sigma = noise_leve * unconditional_sigma.
  Discrete variables are resampled with a distribution that is a mixture:
  (1-noise_level)*(probability mass on all current value)+noise_level*(unconditional distr)
*/
    void perturb(int i, Vec row, real noise_level, int n_last);

/*!       this vector is filled only when the CompactVMatrix was constructed
  from a VMat, and it provides the permutation of the original columns
  to order them into (bits, bytes, fixedpoint)
*/
    TVec<int>& permutation_vector() { return variables_permutation; }

protected:

    //!  decoding (v may be one-hot depending on one_hot_encoding flag)
    virtual void getNewRow(int i, const Vec& v) const;

public:

    //!  encoding (v is not one-hot, and the variables in v are in the "original" order

    //!  return the square difference between row i and row j, excluding n_last columns
    virtual real squareDifference(int i, int j);

    //!  return the dot product of row i with row j, excluding n_last columns
    virtual real dotProduct(int i, int j) const;

    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;


    //!  (i.e. at position i in v we find variable variables_permutation[i] in getRow's result)
    virtual void encodeAndPutRow(int i, Vec v);
    //!  v is possibly one-hot-encoded (according to one_hot_encoding flag) and the
    //!  variables are in the same order as for getRow.
    virtual void putRow(int i, Vec v);
    virtual void putSubRow(int i, int j, Vec v);

    //!  save everything in file (data and auxiliary information), binary format
    virtual void save(const PPath& filename) const
    { Object::save(filename); } //!<  calls write
    //virtual void write(ostream& out) const;

    //!  reverse of write, can be used by calling load(string)
    //virtual void oldread(istream& in);

    PLEARN_DECLARE_OBJECT(CompactVMatrix);
    void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual void build() {}  //!<  nothing to do...

protected:
    //!  auxiliary
    int symbols_offset; //!<  where in each row the symbols start
    int fixedpoint_offset; //!<  where in each row the fixed point numbers start
    Vec row_norms; //!<  to cache the norms of the rows for squareDifference method
};

DECLARE_OBJECT_PTR(CompactVMatrix);

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
