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


/*! \file plearn/base/DoublyLinkedList.h */

#ifndef DoublyLinkedList_INC
#define DoublyLinkedList_INC

#include <plearn/base/plerror.h>

namespace PLearn {
using namespace std;

static bool dbg=false;

/**
 * Class description:
 *
 * Element of a DoublyLinkedList.
 *
 */
template <class T>
struct DoublyLinkedListElement {
    T entry;
    DoublyLinkedListElement* next;
    DoublyLinkedListElement* previous;
    DoublyLinkedListElement(T e, DoublyLinkedListElement* n=0, DoublyLinkedListElement* p=0) : entry(e), next(n), previous(p) {}
};

/**
 * Class description:
 *
 * Doubly linked list of with a 'move-to-front' operation, done in constant time.
 *
 */
template <class T>
class DoublyLinkedList {
public:
    DoublyLinkedListElement<T>* first;
    DoublyLinkedListElement<T>* last;
    int n_elements;

    inline DoublyLinkedList() : first(0), last(0), n_elements(0) {}

    inline DoublyLinkedListElement<T>* pushOnTop(T entry) {
        if (dbg) 
            cout << "calling DoublyLinkedList::pushOnTop(" << entry << endl;
        DoublyLinkedListElement<T>* new_element = new DoublyLinkedListElement<T>(entry,first);
        if (!last) 
            last=new_element;
        if (first) 
            first->previous=new_element;
        first=new_element;
        n_elements++;
        return new_element;
    }

    inline void moveToFront(DoublyLinkedListElement<T>* element)
    {
#ifdef BOUNDCHECK
        if (dbg) 
            cout << "calling DoublyLinkedList::moveToFront(" << element << endl;
        if (!element) PLERROR("DoublyLinkedList::moveToFront called with NULL element\n");
#endif
        if (element!=first)
        {
            DoublyLinkedListElement<T>* next = element->next;
            DoublyLinkedListElement<T>* previous = element->previous;
            // there must be a previous o/w element would have been first
#ifdef BOUNDCHECK
            if (!previous)
                PLERROR("DoublyLinkedList::moveToFront called on incorrect element or corrupted list\n");
            if (previous->next != element)
                PLERROR("DoublyLinkedList::moveToFront called on element not part of list (previous->next != element)\n");
            if (next && next->previous != element)
                PLERROR("DoublyLinkedList::moveToFront called on element not part of list (next->previous != element)\n");
#endif
            element->next = first;
            element->previous = 0;
            previous->next = next; 
            if (next)
                next->previous = previous;
            first->previous = element;
            first = element;
            if (element==last)
                last = previous;
        }
        
    }

    inline void removeLast() {
        if (dbg) 
            cout << "calling DoublyLinkedList::removeLast()" << endl;
        if (last)
        {
            DoublyLinkedListElement<T>* before_last = last->previous;
            if (first == last)
                first = 0;
            last->previous=last->next=0; delete last;
            last = before_last;
            if (last)
                last->next = 0;
            n_elements--;
        }
    }

    inline int nElements() { return n_elements; }

    inline virtual ~DoublyLinkedList() {
        while (first)
        {
            DoublyLinkedListElement<T>* next = first->next;
            delete first;
            first = next;
        }
    }
};

template <class T>
inline int sizeInBytes(const DoublyLinkedListElement<T>& element) { return 2*sizeof(DoublyLinkedListElement<T>*)+sizeInBytes(element.entry); }

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
