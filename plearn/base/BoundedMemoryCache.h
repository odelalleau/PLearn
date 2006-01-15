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


/*! \file plearn/math/BoundedMemoryTVec.h */

#ifndef BoundedMemoryTVec_INC
#define BoundedMemoryTVec_INC

#include <map>
#include <plearn/base/DoublyLinkedList.h>
#include <plearn/base/general.h>

namespace PLearn {
using namespace std;

/**
 * Class description:
 *
 * Similar semantically to a map<KeyType,ElementType>, except that
 * a maximum memory usage can be set, and less frequently and less recently accessed
 * elements that make memory usage go above that limit are dropped.
 *
 */
template <class KeyType, class ElementType>
class BoundedMemoryCache
{
protected:
    int      n_elements; /*!<  The actual number of elements stored */
    int      max_memory; /*!< The maximum memory used to store the elements */
    int      current_memory; /*!< The current memory used to store the elements */
    DoublyLinkedList<KeyType>* doubly_linked_list;
    
public:

    map<KeyType, pair<ElementType,DoublyLinkedListElement<KeyType>*> > elements; /*!< the indexed elements. Each entry has (value, pointer into doubly_linked_list) */

    inline BoundedMemoryCache(int max_memory_=0) 
        : n_elements(0), max_memory(max_memory_), current_memory(0), doubly_linked_list(new DoublyLinkedList<KeyType>)
    {}

    //! Try to get value associataed with key. If not in cache return 0, else return pointer to value.
    //! Recently accessed keys (with set or operator()) are less likely to be removed.
    inline const ElementType* operator()(KeyType& key) const { 
        typename map<KeyType,pair<ElementType,DoublyLinkedListElement<KeyType>*> >::const_iterator it = elements.find(key);
        if (it==elements.end()) 
            return 0;
        doubly_linked_list->moveToFront(it->second.second); 
        return &(it->second.first);
    }
    inline ElementType* operator()(KeyType& key) { 
        typename map<KeyType,pair<ElementType,DoublyLinkedListElement<KeyType>*> >::iterator it = elements.find(key);
        if (it==elements.end()) 
            return 0;
        doubly_linked_list->moveToFront(it->second.second); 
        return &(it->second.first);
    }

    typedef int* pointer;

    //! Associate value to key. 
    //! Recently accessed keys (with set or operator()) are less likely to be removed.
    inline void set(KeyType& key, const ElementType& value) {
        typename map<KeyType,pair<ElementType,DoublyLinkedListElement<KeyType>*> >::iterator it = elements.find(key);
        if (it==elements.end()) { // first time set
            current_memory += sizeInBytes(value) + sizeof(DoublyLinkedListElement<KeyType>) + sizeof(pointer) + sizeInBytes(key);
            DoublyLinkedListElement<KeyType>* p=doubly_linked_list->pushOnTop(key);
            elements[key] = pair<ElementType,DoublyLinkedListElement<KeyType>*>(value,p);
            n_elements++;
        }
        else { // already there, move it to front of list and update the value
            ElementType& v = elements[key].first;
            current_memory += sizeInBytes(value) - sizeInBytes(v);
            doubly_linked_list->moveToFront(it->second.second);
            v = value;
        }
        removeExcess();
    }
    //! Check if this key is in cache. This does not change the access priority of the key.
    inline bool isCached(KeyType& key) const { return elements.find(key)!=elements.end(); }
    inline int nElements() const { return n_elements; }
    inline int currentMemory() const { return current_memory; }
    inline int maxMemory() const { return max_memory; }
    inline void setMaxMemory(int new_max_memory) {
        max_memory=new_max_memory;
        removeExcess();
    }

protected:
    //! remove last element until current_memory <= max_memory;
    inline void removeExcess()
    {
        while (current_memory > max_memory)
        {
            KeyType& key = doubly_linked_list->last->entry;
            current_memory -= sizeInBytes(elements[key].first) + sizeof(DoublyLinkedListElement<KeyType>) + sizeof(pointer) + sizeInBytes(key);
            elements.erase(key);
            doubly_linked_list->removeLast();
            n_elements--;
        }
    }
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
