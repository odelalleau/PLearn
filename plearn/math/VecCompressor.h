// -*- C++ -*-

// VecCompressor.h
// Copyright (C) 2001 Pascal Vincent
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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


#ifndef VecCompressor_INC
#define VecCompressor_INC

#include "TMat.h"

namespace PLearn {
using namespace std;
 

/*!  
  It encodes only non zero values, using one byte for small integers,
  and 4-byte floating points for all other values.
  This representation should be reasonably good for both sparse matrices,
  and matrices containing categorical data (represented by small integers)
  possibly with one hot representations.
*/

class VecCompressor
{
protected:

    static inline bool issmallint(real x)
    { int intx = int(x); return fast_exact_is_equal(floor(x), x) && intx<=127 && intx>=-127; }

    static inline bool is0(real x)
    { return fast_exact_is_equal(x, 0.); }

    static inline bool isI(real x)
    { return !fast_exact_is_equal(x, 0.) && issmallint(x); }

    static inline bool isF(real x)
    { return !fast_exact_is_equal(x, 0.) && !issmallint(x); }
  
public:
/*!     writes v in a compressed form in the data buffer passed as argument.
  (make sure enough memory is allocated in the data buffer)
  returns a pointer to the one-after-last element written in the data block
*/
    static signed char* compressVec(const Vec& v, signed char* data);

    //!  uncompresses the data of a vector compressed with compressVec
    //!  v must have the correct size.
    static void uncompressVec(signed char* data, const Vec& v);  

    //!  writes v in compressed format to the given stream
    //!  The written data does not contain size info.
    static void writeCompressedVec(ostream& out, const Vec& v);

    //!  reads data of a compressed vector from the given stream
    //!  v must have the right size already (this is not checked!)
    static void readCompressedVec(istream& in, const Vec& v); 

    //!  Returns the number of bytes that will be used to encode a vector of 
    //!  length n in the worst case
    static size_t worstCaseSize(int n)
    { return 2+4*n+n/128; }
};

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
