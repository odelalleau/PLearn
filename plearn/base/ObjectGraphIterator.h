// -*- C++ -*-

// ObjectGraphIterator.h
//
// Copyright (C) 2005 Nicolas Chapados 
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file ObjectGraphIterator.h */


#ifndef ObjectGraphIterator_INC
#define ObjectGraphIterator_INC

// From C++ stdlib
#include <string>
#include <vector>

// From PLearn
#include <plearn/base/OptionBase.h>


namespace PLearn {

// Forward-declare
class Object;


//######################  CLASS  OBJECTOPTIONSITERATOR  #######################

/**
 *  An ObjectOptionsIterator iterates across all accessible sub-objects of a
 *  given \c PLearn::Object.  The iteration is carried out by going through the
 *  list of options within the object, and traversing vectors (and other
 *  indexable options, such as maps, eventually) as appropriate.
 *
 *  The iterator is a model of the STL ForwardIterator.  Default-constructing
 *  this object makes an "invalid" iterator which is always considered the end.
 */
class ObjectOptionsIterator
{
public:
    //! Default constructor: invalid iterator
    ObjectOptionsIterator();

    //! Iterate along all sub-objects of the given object
    ObjectOptionsIterator(const Object* root);

    // Default assignment, copy constructor, destructor

    //! Equality testing
    bool operator==(const ObjectOptionsIterator& rhs) const;

    //! Inequality testing
    bool operator!=(const ObjectOptionsIterator& rhs) const
    {
        return ! (*this == rhs);
    }

    //! Dereference: access the current sub-object.  Note that if
    //! this object MAY be null if the option is indeed null.
    const Object* operator*() const;

    /**
     *  Return the name of the current option such that code
     *  of the following form always succeeds:
     *
     *  @code
     *  root->getOption(current_option_name)
     *  @endcode
     */
    string getCurrentOptionName() const;
    
    // Go to next object or "invalid" state if end of iteration
    const ObjectOptionsIterator& operator++();

    //! Post-increment: implemented in terms of pre-increment.
    //! Note that this is expensive; don't use if not necessary.
    ObjectOptionsIterator operator++(int)
    {
        ObjectOptionsIterator old(*this);
        ++(*this);
        return old;
    }

protected:
    bool m_invalid;                          //!< If true: invalid state
    const Object* m_object;                  //!< Object we are pointing to
    const OptionList* m_options;             //!< OptionList of that object
    int m_cur_option;                        //!< Which option we're at
    int m_cur_index;                         //!< If cur_option is indexable,
                                             //!<   current index within option
    int m_max_index;                         //!< If cur_option is indexable,
                                             //!<   maximum possible inxex+1
};


//#######################  CLASS  OBJECTGRAPHITERATOR  ########################

/**
 *  An ObjectGraphIterator iterates through all objects through options.
 *  From a starting "root" object, the iterator returns pointers to each
 *  encountered object in the graph that is accessible through options.  Since
 *  this is "real" graph traversal, the same object is not returned twice.
 *
 *  The following options are available:
 *
 *  - Either breadth-first or depth-first traversal is supported.  For
 *    depth-first, only pre-order traversal is supported so far.  Note that the
 *    "root" node is visited just like any other node.
 *
 *  - We can filter the returned objects by base-class type; in other words,
 *    the iterator can be forced to return only objects that inherit from a
 *    given base class.
 *
 *  The current implementation starts out by building a traversal list which
 *  records the set of all objects in the graph in the order in which they
 *  should be returned.  Then the iteration per se is simply an STL-like
 *  forward iterator that performs the appropriate filterings.
 *
 *  The iterator skips ALL NULL POINTERS in unused options.
 */
class ObjectGraphIterator
{
public:
    //! Specify the kinds of supported traversals
    enum TraversalType {
        BreadthOrder  = 1,
        DepthPreOrder = 2
    };

public:
    //! Default constructor makes an "invalid" iterator
    ObjectGraphIterator();
    
    //! Constructor takes a root node, an optional traversal type,
    //! and an optional class name for filtering by type
    ObjectGraphIterator(const Object* root, TraversalType tt = DepthPreOrder,
                        const std::string& base_class_filter = "");
    
    // Default assignment, copy constructor, destructor

    //! Return true if the iterator is in an invalid state;
    bool invalid() const
    {
        return m_it == m_end;
    }
    
    //! Equality testing; two invalid iterators always compare equal
    bool operator==(const ObjectGraphIterator& rhs) const;

    //! Inequality testing
    bool operator!=(const ObjectGraphIterator& rhs) const
    {
        return ! (*this == rhs);
    }

    //! Dereference: access the current sub-object.  Note that this never
    //! returns a null pointer.
    const Object* operator*() const
    {
        assert( !invalid() );
        return *m_it;
    }
    
    //! Go to next object or "invalid" state if end of iteration
    const ObjectGraphIterator& operator++();

    //! Post-increment: implemented in terms of pre-increment.
    //! Note that this is expensive; don't use if not necessary.
    ObjectGraphIterator operator++(int)
    {
        ObjectGraphIterator old(*this);
        ++(*this);
        return old;
    }

protected:
    //! Build a traversal graph from a root node; filters are not yet taken
    //! into account at this point
    void buildTraversalGraph(const Object* root, TraversalType tt);

protected:
    typedef std::vector<const Object*> ObjectList;
    typedef bool (*ISA)(const Object*);
    
    ObjectList m_object_list;                //!< from buildTraversalGraph()
    std::string m_base_class_filter;
    ObjectList::iterator m_it;               //!< Iterator within m_object_list
    ObjectList::iterator m_end;              //!< End of m_object_list
    ISA m_isa_tester;                        //!< Predicate for sub-type filter
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
