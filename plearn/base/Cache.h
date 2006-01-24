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
 * AUTHORS: Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file plearn/base/Cache.h */

#ifndef Cache_INC
#define Cache_INC

#include <plearn/base/BoundedMemoryCache.h>

namespace PLearn {
using namespace std;

/**
 * Class description:
 *
 * Similar semantically to a map<KeyType,ValueType>, except that
 * a maximum memory usage can be set, and less frequently and less recently accessed
 * elements that make memory usage go above that limit are saved to a file
 * (either a single large file, only if elements all have the size size, or 
 * one file per element). The memory cache is implemented with
 * a BoundedMemoryCache, to which this class adds the ability to
 * store the less frequently accessed items on disk. The elements
 * keys and values (of template types KeyType and ValueType respectively)
 * must be serializable with PLearn's PStream's. If one file per
 * entry is used, the KeyType must be convertible to a string with 'tostring(key)', which
 * will be used as a suffix of the file name (before the .psave suffix). 
 *
 */
template <class KeyType, class ValueType>
class Cache
{
protected:
    BoundedMemoryCache<KeyType,ValueType> memory_cache;
    bool single_file; // whether the filed entries are in a single large file or one file per element
    string file_prefix;
    int n_elements;
public:

    inline Cache(string fileprefix = "cache", bool singlefile=false, int max_memory_=0) 
        : memory_cache(max_memory), single_file(singlefile), file_prefix(fileprefix), n_elements(0)
    {}

    //! Try to get value associataed with key. If not in cache (either memory or disk) return 0, 
    //! else return pointer to value. Recently accessed keys (with set or operator()) are less likely 
    //! to be removed from memory (saved to disk).
    inline const ValueType* operator()(KeyType& key) const { 
        ValueType* value_p = memory_cache(key);
        if (value_p) // we have it in memory
            return *value_p;
        // else check if we have it on file
    }
    inline ValueType* operator()(KeyType& key) { 
    }


    //! Associate value to key. 
    //! Recently accessed keys (with set or operator()) are less likely to be removed from memory.
    inline void set(KeyType& key, const ValueType& value) {
        if (first time it is set)
            n_elements++;
    }
    //! Check if this key is in cache. This does not change the access priority of the key.
    inline bool isCached(KeyType& key) const { return memory_cache.isCached(key) || isOnFile(key); }
    inline bool isInMemory(KeyType& key) const { return memory_cache.isCached(key); }
    inline int nElements() const { return n_elements; }
    inline int nElementsInMemory() const { return memory_cache.n_elements; }
    inline int currentMemory() const { return memory_cache.current_memory; }
    inline int maxMemory() const { return memory_cache.max_memory; }
    inline void setMaxMemory(int new_max_memory) { memory_cache.setMaxMemory(new_max_memory); }
    inline float successRate() { return memory_cache.successRate(); }

protected:

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
