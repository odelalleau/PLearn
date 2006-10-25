// -*- C++ -*-

// ObjectGraphIterator.cc
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

/*! \file ObjectGraphIterator.cc */

#include <algorithm>
#include <assert.h>
#include <deque>
#include <set>

#include "ObjectGraphIterator.h"
#include <plearn/base/Object.h>
#include <plearn/base/OptionBase.h>
#include <plearn/base/TypeFactory.h>

namespace PLearn {
using namespace std;


//#####  ObjectOptionsIterator  ###############################################

ObjectOptionsIterator::ObjectOptionsIterator()
    : m_invalid(true), m_skip_nulls(true),
      m_object(0), m_options(0),
      m_cur_option(), m_cur_index(), m_max_index()
{ }


ObjectOptionsIterator::ObjectOptionsIterator(const Object* root,
                                             bool skip_nulls)
    : m_invalid(true),
      m_skip_nulls(skip_nulls),
      m_object(root), m_options(0),
      m_cur_option(), m_cur_index(), m_max_index()
{
    if (m_object) {
        m_options = &m_object->getOptionList();
        PLASSERT( m_options );

        if (m_options->size() > 0) {
            m_invalid = false;
            m_cur_option = -1;
            ++(*this);                           // find first valid option
        }
    }
}


bool ObjectOptionsIterator::operator==(const ObjectOptionsIterator& rhs) const
{
    return (m_invalid && rhs.m_invalid)      ||
        (! m_invalid                         &&
         m_object      == rhs.m_object       &&
         m_options     == rhs.m_options      &&
         m_cur_option  == rhs.m_cur_option   &&
         m_cur_index   == rhs.m_cur_index    &&
         m_max_index   == rhs.m_max_index);
}


const Object* ObjectOptionsIterator::operator*() const
{
    PLASSERT( !m_invalid && m_object && m_options );

    if (m_max_index > 0)
        return (*m_options)[m_cur_option]->getIndexedObject(m_object,
                                                            m_cur_index);
    else
        return (*m_options)[m_cur_option]->getAsObject(m_object);
}


string ObjectOptionsIterator::getCurrentOptionName() const
{
    PLASSERT( !m_invalid && m_object && m_options );
    string optionname = (*m_options)[m_cur_option]->optionname();
    if (m_max_index > 0)
        optionname += '[' + tostring(m_cur_index) + ']';
    return optionname;
}


OptionBase::flag_t ObjectOptionsIterator::getCurrentOptionFlags() const
{
    PLASSERT( !m_invalid && m_object && m_options );
    return (*m_options)[m_cur_option]->flags();
}


const ObjectOptionsIterator& ObjectOptionsIterator::operator++()
{
    PLASSERT( !m_invalid && m_object && m_options );

    do {
        // Start by considering current iteration within an indexable option
        if (m_max_index > 0) {
            ++m_cur_index;
            PLASSERT( m_cur_index <= m_max_index );
            if (m_cur_index < m_max_index)
                return *this;
        }

        // Otherwise find next option accessible as object, and that is not
        // marked with the 'nontraversable' flag.  Go to invalid state if
        // cannot find any
        const int n = m_options->size();
        for ( ++m_cur_option ; m_cur_option < n ; ++m_cur_option ) {
            if (    (*m_options)[m_cur_option]->isAccessibleAsObject() &&
                 ! ((*m_options)[m_cur_option]->flags() & OptionBase::nontraversable) )
            {
                m_cur_index = 0;
                m_max_index = (*m_options)[m_cur_option]->indexableSize(m_object);
                break;
            }
        }

        m_invalid = (m_cur_option >= n);

        // Condition below: keep incrementing as long as we are hitting a null
        // object (and skipping nulls is requested), or an empty array/vector
    } while (!m_invalid &&
             ( (!m_max_index && m_skip_nulls && !**this) ||
               ( m_max_index && m_cur_index >= m_max_index ) ));

    return *this;
}


//#####  ObjectGraphIterator  #################################################

ObjectGraphIterator::ObjectGraphIterator():
    m_it(0),
    m_end(0),
    m_isa_tester()
{ }


ObjectGraphIterator::ObjectGraphIterator(
    const Object* root, TraversalType tt,
    bool compute_optnames, const string& base_class_filter)
    : m_isa_tester()
{
    // Build the traversal graph
    buildTraversalGraph(root, tt, compute_optnames);
    m_it  = m_object_list.begin();
    m_end = m_object_list.end();

    // Filter by base-class if required
    if (base_class_filter != "") {
        const TypeMapEntry& tme =
            TypeFactory::instance().getTypeMapEntry(base_class_filter);
        m_isa_tester = tme.isa_method;
        if (! m_isa_tester)
            PLERROR("ObjectGraphIterator: requested type filter \"%s\" does not "
                    "have an ISA type predicate function", base_class_filter.c_str());
    }

    // If first object is not acceptable according to predicate, skip to next
    // valid object
    if (!invalid() && m_isa_tester && !m_isa_tester(**this))
        ++(*this);
}


bool ObjectGraphIterator::operator==(const ObjectGraphIterator& rhs) const
{
    return (invalid() && rhs.invalid()) ||
        (! invalid()                    &&
         m_it   == rhs.m_it             &&
         m_end  == rhs.m_end);
}


const ObjectGraphIterator& ObjectGraphIterator::operator++()
{
    PLASSERT( !invalid() );
    for ( ++m_it ; m_it != m_end && m_isa_tester && !m_isa_tester(**this) ; ++m_it )
        continue;
    return *this;
}


//#####  buildTraversalGraph  #################################################

void ObjectGraphIterator::buildTraversalGraph(const Object* root,
                                              TraversalType tt,
                                              bool compute_optnames)
{
    // The deque is used to maintain either a FIFO or LIFO of nodes to visit.
    // Deletion is always carried out with a pop_back.
    typedef std::deque<ObjectAndName> Q;
    typedef std::set<const Object*> SeenSet;
    typedef void (Q::*QAppender)(Q::const_reference);

    Q object_queue;
    SeenSet seen;
    const SeenSet::iterator not_seen = seen.end();
    QAppender appender;
    
    switch (tt & ~Reversed) {
    case BreadthOrder  : appender = &Q::push_front; break;
    case DepthPreOrder : appender = &Q::push_back;  break;
    default:
        PLERROR("ObjectGraphIterator::buildTraversalGraph: unknown traversal type (%d)",tt);
    }

    // Start out by appending the root
    m_object_list.resize(0);
    (object_queue.*appender)(make_pair(root, ""));
    seen.insert(root);

    // Traverse the graph
    while (object_queue.size() > 0) {
        ObjectAndName cur_objname = object_queue.back();
        object_queue.pop_back();
        m_object_list.push_back(cur_objname);

        ObjectOptionsIterator options(cur_objname.first), end_options;
        for ( ; options != end_options ; ++options) {
            const Object* candidate_object = *options;
            if (candidate_object && seen.find(candidate_object) == not_seen) {
                string new_optname;
                if (compute_optnames) {
                    new_optname = cur_objname.second;
                    if (new_optname != "")
                        new_optname += ".";
                    new_optname += options.getCurrentOptionName();
                }
                (object_queue.*appender)(make_pair(candidate_object,
                                                   new_optname));
                seen.insert(candidate_object);
            }
        }
    }

    // Finally reverse the elements if required
    if (tt & Reversed)
        std::reverse(m_object_list.begin(), m_object_list.end());
}


//#####  setoption_broadcast  #################################################

void setoption_broadcast(const Object* o, const string& class_name,
                         const string& option_name, const string& option_value,
                         ObjectGraphIterator::TraversalType tt)
{
    ObjectGraphIterator grit(o, tt, false, class_name), grend;
    for ( ; grit != grend ; ++grit)
        const_cast<Object*>(*grit)->setOption(option_name, option_value);
}

} // end of namespace PLearn


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
