// -*- C++ -*-
//
// ArrayAllocatorIndex.h
//
// Copyright (C) 2000 Nicolas Chapados
// 
// Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
// 
//    1. Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//       
//    3. The name of the authors may not be used to endorse or promote
//       products derived from this software without specific prior written
//      permission. 
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
 * * $Id: ArrayAllocatorIndex.h,v 1.2 2003/03/06 18:26:00 ducharme Exp $
 * ******************************************************* */


#ifndef ARRAYALLOCATORINDEX_H
#define ARRAYALLOCATORINDEX_H

namespace PLearn <%
using namespace std;



//!  This type represents an index into the allocated memory,
//!  as a bit-field parameterized by the template argument SizeBits.
template <class IndexBase, unsigned SizeBits>
class ArrayAllocatorIndex {
public:
    typedef IndexBase index_base;
    typedef ArrayAllocatorIndex<IndexBase,SizeBits> index_type;
    
    //!  assume that a char contains 8 bits
    unsigned index : 8*sizeof(IndexBase) - SizeBits;
    unsigned size : SizeBits;

    ArrayAllocatorIndex(unsigned the_index, unsigned the_size=0)
      : index(the_index), size(the_size) {}
    
    bool operator==(index_type other) const {
      return index == other.index;     //!<  don't consider block size
    }
    bool operator!=(index_type other) const {
      return index != other.index;     //!<  don't consider block size
    }
    bool isNull() const {
      return index == 0;
    }
    void swap(index_type& other) {
      std::swap(index, other.index);
      std::swap(size,  other.size);
    }
};

%> // end of namespace PLearn


#endif
