// -*- C++ -*-

// ParentableObject.cc
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

/*! \file ParentableObject.cc */

#include "ParentableObject.h"
#include <plearn/base/ObjectGraphIterator.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    ParentableObject,
    "Object which maintains a \"parent\" pointer as part of an object graph.",
    "The purpose of ParentableObject is to facilitate the building of complex\n"
    "graphs of Objects. The basic ideas are as follows:\n"
    "\n"
    "- The object contains a member called parent, which is simply a\n"
    "  backpointer to the \"parent\" object in the graph.  This is a dumb\n"
    "  pointer to avoid cycles as the forward pointers will usually be PP's.\n"
    "\n"
    "- The build_() method looks at all the options of itself, and for those\n"
    "  objects that are  ParentableObject, it sets their parent pointer\n"
    "  to  this.  (This mechanism could conceptually be put in  Object\n"
    "  itself, but out of caution we leave it in ParentableObject for now).\n"
    "\n"
    "In other words, this class both provides the backpointer in an object\n"
    "graph, and provides the mechanism to update the backpointer according to\n"
    "arbitrary forward pointers (as long as the forward pointers are accessible\n"
    "as options through an ObjectOptionsIterator.)\n"
    );

ParentableObject::ParentableObject(bool adoptive_parent) 
    : m_parent(0), m_adoptive_parent(adoptive_parent)
{ }

// ### Nothing to add here, simply calls build_
void ParentableObject::build()
{
    inherited::build();
    build_();
}

void ParentableObject::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_parent, copies);
}

void ParentableObject::build_()
{
    if (! m_adoptive_parent) {
        // Set the backpointers of sub-objects under the current object to this.
        for (ObjectOptionsIterator it(this), end ; it != end ; ++it) {
            if (const ParentableObject* po = dynamic_cast<const ParentableObject*>(*it))
                const_cast<ParentableObject*>(po)->setParent(this);
        }
    }
}

void ParentableObject::setParent(Object* parent)
{
    m_parent = parent;
}


//#####  TransparentParentable  ###############################################

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    TransparentParentable,
    "Special type of ParentableObject that cannot act as a visible parent.",
    "Suppose that you have the following object structure:\n"
    "\n"
    "  MasterManager contains a list of ObjectDescriptor\n"
    "    Each ObjectDescriptor contains a list of ChildrenObject\n"
    "\n"
    "The idea here is that ObjectDescriptor is a simple holder class that\n"
    "provides additional information for how ChildrenObject should be built in\n"
    "the context of MasterManager.  However, we want to have the situation\n"
    "wherein the parent() of each ChildrenObject is the MasterManager, and not\n"
    "the ObjectDescriptors.\n"
    "\n"
    "That's the purpose of TransparentParentable: if you make ObjectDescriptor a\n"
    "derived class of TransparentParentable, they are skipped when going up on\n"
    "the children-parent paths.\n"
    );

TransparentParentable::TransparentParentable(bool adoptive_parent)
    : inherited(adoptive_parent)
{ }

// ### Nothing to add here, simply calls build_
void TransparentParentable::build()
{
    inherited::build();
    build_();
}

void TransparentParentable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

void TransparentParentable::build_()
{ }

void TransparentParentable::setParent(Object* parent)
{
    // Set it for ourselves
    inherited::setParent(parent);
    
    // Forward the call to sub-objects: set their parent to the one that's
    // passed here
    for (ObjectOptionsIterator it(this), end ; it != end ; ++it) {
        if (const ParentableObject* po = dynamic_cast<const ParentableObject*>(*it))
            const_cast<ParentableObject*>(po)->setParent(parent);
    }
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
