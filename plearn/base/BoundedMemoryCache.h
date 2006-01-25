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


/*! \file plearn/base/BoundedMemoryCache.h */

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
 * Similar semantically to a map<KeyType,ValueType>, except that
 * a maximum memory usage can be set, and less frequently and less recently accessed
 * elements that make memory usage go above that limit are dropped.
 * The heuristic for giving more priority to more recently accessed stuff is the 'move-to-front' 
 * heuristic. When an entry is read or set it is  moved to the front of a list. When
 * the allowed memory has been overrun the last element of the list is
 * dropped. The list is implemented with the new class plearn/base/DoublyLinkedList,
 * a doubly-linked list with removeLast() and moveToFront() methods.
 *
 */
template <class KeyType, class ValueType>
class BoundedMemoryCache
{
protected:
    int      n_elements; /*!<  The actual number of elements stored in memory */
    int      max_memory; /*!< The maximum memory used to store the elements */
    int      current_memory; /*!< The current memory used to store the elements */
public:
    DoublyLinkedList<KeyType>* doubly_linked_list;
    mutable int      n_successful_hits; /*!< Number of times operator() was called successfully */
    mutable int      n_failures; /*!< Number of times operator() was called and entry not found */

    map<KeyType, pair<ValueType,DoublyLinkedListElement<KeyType>*> > elements; /*!< the indexed elements. Each entry has (value, pointer into doubly_linked_list) */

    inline BoundedMemoryCache(int max_memory_=0) 
        : n_elements(0), max_memory(max_memory_), current_memory(0), doubly_linked_list(new DoublyLinkedList<KeyType>),
          n_successful_hits(0), n_failures(0)
    {}

    inline void clear() { 
        n_elements=0; current_memory=0; elements.clear(); 
        while (BoundedMemoryCache<KeyType,ValueType>::doubly_linked_list->first) BoundedMemoryCache<KeyType,ValueType>::doubly_linked_list->removeLast();
    }

    //! Try to get value associataed with key. If not in cache return 0, else return pointer to value.
    //! Recently accessed keys (with set or operator()) are less likely to be removed.
    const ValueType* operator()(KeyType& key) const { 
        typename map<KeyType,pair<ValueType,DoublyLinkedListElement<KeyType>*> >::const_iterator it = elements.find(key);
        if (it==elements.end()) 
        {
            n_failures++;
            return 0;
        }
        n_successful_hits++;
        doubly_linked_list->moveToFront(it->second.second); 
#ifdef BOUNDCHECK
        if (!doubly_linked_list->last || doubly_linked_list->last->next)
            PLERROR("something wrong with last element of doubly linked list!");
        if (dbg)
            verifyInvariants();
#endif
        return &(it->second.first);
    }
    ValueType* operator()(KeyType& key) { 
        typename map<KeyType,pair<ValueType,DoublyLinkedListElement<KeyType>*> >::iterator it = elements.find(key);
        if (it==elements.end()) 
        {
            n_failures++;
            return 0;
        }
        n_successful_hits++;
        doubly_linked_list->moveToFront(it->second.second); 
#ifdef BOUNDCHECK
        if (!doubly_linked_list->last || doubly_linked_list->last->next)
            PLERROR("something wrong with last element of doubly linked list!");
        if (dbg)
            verifyInvariants();
#endif
        return &(it->second.first);
    }

    typedef int* pointer;

    //! Associate value to key. 
    //! Recently accessed keys (with set or operator()) are less likely to be removed.
    void set(KeyType& key, const ValueType& value) {
        typename map<KeyType,pair<ValueType,DoublyLinkedListElement<KeyType>*> >::iterator it = elements.find(key);
        if (it==elements.end()) { // first time set
            DoublyLinkedListElement<KeyType>* p=doubly_linked_list->pushOnTop(key);
#ifdef BOUNDCHECK
            if (!doubly_linked_list->last || doubly_linked_list->last->next)
                PLERROR("something wrong with last element of doubly linked list!");
#endif
            pair<ValueType,DoublyLinkedListElement<KeyType>*> el(value,p);
            elements[key] = el;
            current_memory += sizeInBytes(el);
            n_elements++;
#ifdef BOUNDCHECK
            if (dbg)
                verifyInvariants();
#endif
        }
        else { // already there, move it to front of list and update the value
            ValueType& v = elements[key].first;
            current_memory += sizeInBytes(value) - sizeInBytes(v);
            doubly_linked_list->moveToFront(it->second.second);
#ifdef BOUNDCHECK
            if (!doubly_linked_list->last || doubly_linked_list->last->next)
                PLERROR("something wrong with last element of doubly linked list!");
#endif
            v = value;
#ifdef BOUNDCHECK
            if (dbg)
                verifyInvariants();
#endif
        }
        removeExcess();
    }
    //! Check if this key is in cache. This does not change the access priority of the key.
    inline  bool isCached(KeyType& key) const { return elements.find(key)!=elements.end(); }
    inline  int nElements() const { return n_elements; }
    inline  int currentMemory() const { return current_memory; }
    inline  int maxMemory() const { return max_memory; }
    inline  void setMaxMemory(int new_max_memory) {
        max_memory=new_max_memory;
        removeExcess();
    }

    inline  float successRate() { return float(n_successful_hits)/(n_successful_hits + n_failures); }

    inline virtual ~BoundedMemoryCache() { clear(); }

    // check that all pointers to doubly linked list elements are still valid
    void verifyInvariants() {
        if (!doubly_linked_list->last || doubly_linked_list->last->next)
            PLERROR("something wrong with last element of doubly linked list!");
        if (max_memory - current_memory>500) return;
        typename map<KeyType,pair<ValueType,DoublyLinkedListElement<KeyType>*> >::iterator it = elements.begin();
        for (;it!=elements.end();++it)
        {
            DoublyLinkedListElement<KeyType>* p = it->second.second;
            if (!p) 
                PLERROR("BoundedMemoryCache::verifyInvariants(): null linked list pointer!");
            DoublyLinkedListElement<KeyType>* next = p->next;
            DoublyLinkedListElement<KeyType>* previous = p->previous;
            if (previous && previous->next != p)
                PLERROR("BoundedMemoryCache::verifyInvariants(): element not part of list (previous->next != element)\n");
            if (next && next->previous != p)
                PLERROR("BoundedMemoryCache::verifyInvariants(): element not part of list (next->previous != element)\n");
            typename map<KeyType,pair<ValueType,DoublyLinkedListElement<KeyType>*> >::iterator pi = elements.find(p->entry);
            if (pi->second.second!=p)
                PLERROR("BoundedMemoryCache::verifyInvariants(): incoherent pointers between map and list\n");
            if (pi->first!=p->entry)
                PLERROR("BoundedMemoryCache::verifyInvariants(): incoherent keys between map and list\n");
        }
    }

protected:
    //! remove last element until current_memory <= max_memory;
    inline  virtual void removeExcess()
    {
        while (current_memory > max_memory)
        {
            KeyType& key = doubly_linked_list->last->entry;
            current_memory -= sizeInBytes(elements[key]);
            elements[key].second=0; elements.erase(key);
            doubly_linked_list->removeLast();
            n_elements--;
#ifdef BOUNDCHECK
            if (!doubly_linked_list->last || doubly_linked_list->last->next)
                PLERROR("something wrong with last element of doubly linked list!");
            if (dbg)
                verifyInvariants();
#endif
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
