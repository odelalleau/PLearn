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
#include <plearn/io/fileutils.h>
#include <plearn/io/openFile.h>
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
 * ONLY THE FILES ASSOCIATED TO THIS CACHE SHOULD BE STORED IN THE files_directory.
 *
 */
template <class KeyType, class ValueType>
class Cache : public BoundedMemoryCache<KeyType,ValueType>
{
protected:
public:
    bool single_file; // whether the filed entries are in a single large file or one file per element
    string files_directory;
    PStream::mode_t file_format; // possible values are: PStream::{raw,plearn}_{ascii,binary}

    inline Cache(string dir = "cache", int max_memory_=0, bool singlefile=false) 
        : BoundedMemoryCache<KeyType,ValueType>(max_memory_), single_file(singlefile), files_directory(dir), file_format(PStream::plearn_ascii)
    {}

    //! Try to get value associataed with key. If not in cache (either memory or disk) return 0, 
    //! else return pointer to value. Recently accessed keys (with set or operator()) are less likely 
    //! to be removed from memory (saved to disk).
    inline const ValueType* operator()(const KeyType& key) const { 
        ValueType* value_p = BoundedMemoryCache<KeyType,ValueType>::operator()(key);
        if (value_p) // we have it in memory
            return value_p;
        // else check if we have it on file
        value_p = loadValue(key);
        if (value_p)
            BoundedMemoryCache<KeyType,ValueType>::set(key,*value_p);
        return value_p;
    }
    inline ValueType* operator()(const KeyType& key) { 
        ValueType* value_p = BoundedMemoryCache<KeyType,ValueType>::operator()(key);
        if (value_p) // we have it in memory
            return value_p;
        // else check if we have it on file
        value_p = loadValue(key);
        if (value_p)
            BoundedMemoryCache<KeyType,ValueType>::set(key,*value_p);
        return value_p;
    }

    //! Check if this key is in cache. This does not change the access priority of the key.
    inline bool isCached(const KeyType& key) const { return BoundedMemoryCache<KeyType,ValueType>::isCached(key) || isOnFile(key); }
    inline bool isInMemory(const KeyType& key) const { return BoundedMemoryCache<KeyType,ValueType>::isCached(key); }
    inline string filename(const KeyType& key) const { return files_directory + "/" + tostring(key) + ".psave"; }
    inline bool isOnFile(const KeyType& key) const { return isfile(filename(key)); }
    // note that clear REMOVES EVERYTHING: stuff on disk and stuff in memory
    inline void clear() {
        BoundedMemoryCache<KeyType,ValueType>::clear();
        vector<string> filenames = lsdir(files_directory);
        for (unsigned int i=0;i<filenames.size();i++)
            rm(files_directory + "/" + filenames[i]);
    }
    // whereas destroying the cache or calling BoundedMemoryCache::clear() will 
    // delete the stuff from memory WHILE SAVING IT TO DISK, EVERYTHING IS PRESERVED ON DISK.
    inline virtual ~Cache() { 
        BoundedMemoryCache<KeyType,ValueType>::clear(); 
    }

    // makes sure everything in memory also resides on disk (brutally copy everything, which is simpler)
    inline virtual void synchronizeDisk() const {
        typename map<KeyType,pair<ValueType,DoublyLinkedListElement<KeyType>*> >::const_iterator it = BoundedMemoryCache<KeyType,ValueType>::elements.begin();
        typename map<KeyType,pair<ValueType,DoublyLinkedListElement<KeyType>*> >::const_iterator end = BoundedMemoryCache<KeyType,ValueType>::elements.end();
        for (;it!=end;++it)
            saveValue(it->first,it->second.first);
    }

protected:
    inline ValueType* loadValue(const KeyType& key) {
        if (single_file)
        {
            PLERROR("Cache: single_file mode not yet implemented");
        }
        else
        {
            string fname = filename(key);
            if (!isfile(fname))
                return 0;
            PStream filestream = openFile(fname, file_format, "r");
            ValueType* v = new ValueType;
            filestream >> *v;
            return v;
        }
        return 0;
    }
    inline void saveValue(const KeyType& key, const ValueType& value) const {
        if (single_file)
        {
            PLERROR("Cache: single_file mode not yet implemented");
        }
        else
        {
            PStream filestream = openFile(filename(key), file_format, "w");
            filestream << value;
        }
    }

    //! remove last element until current_memory <= max_memory;
    inline void removeExcess()
    {
        while (BoundedMemoryCache<KeyType,ValueType>::current_memory > BoundedMemoryCache<KeyType,ValueType>::max_memory)
        {
            KeyType& key = BoundedMemoryCache<KeyType,ValueType>::doubly_linked_list->last->entry;

            // STORE THE ELEMENT TO BE DELETED ON DISK:
            pair<ValueType,DoublyLinkedListElement<KeyType>*> v = BoundedMemoryCache<KeyType,ValueType>::elements[key];
            saveValue(key, v.first);

            BoundedMemoryCache<KeyType,ValueType>::current_memory -= sizeInBytes(BoundedMemoryCache<KeyType,ValueType>::elements[key]);
            v.second=0;  // this is just to help debugging, really, to help track wrong pointers
            BoundedMemoryCache<KeyType,ValueType>::elements.erase(key);
            BoundedMemoryCache<KeyType,ValueType>::doubly_linked_list->removeLast();
            BoundedMemoryCache<KeyType,ValueType>::n_elements--;
        }

        int left_elements = BoundedMemoryCache<KeyType,ValueType>::n_elements;
    }

    //! remove all elements
    inline void removeAll()
    {
        while (BoundedMemoryCache<KeyType,ValueType>::n_elements)
        {
            KeyType& key = BoundedMemoryCache<KeyType,ValueType>::doubly_linked_list->last->entry;

            // STORE THE ELEMENT TO BE DELETED ON DISK:
            pair<ValueType,DoublyLinkedListElement<KeyType>*> v = BoundedMemoryCache<KeyType,ValueType>::elements[key];
            saveValue(key, v.first);

//            BoundedMemoryCache<KeyType,ValueType>::current_memory -= sizeInBytes(BoundedMemoryCache<KeyType,ValueType>::elements[key]);
            v.second=0;  // this is just to help debugging, really, to help track wrong pointers
            BoundedMemoryCache<KeyType,ValueType>::elements.erase(key);
            BoundedMemoryCache<KeyType,ValueType>::doubly_linked_list->removeLast();
            BoundedMemoryCache<KeyType,ValueType>::n_elements--;
        }

        BoundedMemoryCache<KeyType,ValueType>::current_memory = 0;
        int left_elements = BoundedMemoryCache<KeyType,ValueType>::n_elements;
    }

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
