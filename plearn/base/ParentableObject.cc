// -*- C++ -*-

// ParentableObject.cc
//
// Copyright (C) 2005-2006 Nicolas Chapados 
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

#define PL_LOG_MODULE_NAME "ParentableObject"

#include "ParentableObject.h"
#include <plearn/base/ObjectGraphIterator.h>
#include <plearn/base/stringutils.h>
#include <plearn/io/pl_log.h>

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
    "\n"
    "Each ParentableObject can be marked with the kind of parent it wants.  The\n"
    "following kinds are supported:\n"
    "\n"
    "- AnyParent    : anybody that calls setParent gets to be the parent.\n"
    "\n"
    "- WeakParent   : the first caller to setParent gets to be the parent, the\n"
    "                 other ones are ignored (default).\n"
    "\n"
    "- UniqueParent : the first caller to setParent gets to be the parent, the\n"
    "                 other ones yield PLERRORs.\n"
    );

ParentableObject::ParentableObject(bool adoptive_parent, ParentKind pk) 
    : m_parent(0), m_adoptive_parent(adoptive_parent), m_parent_kind(pk)
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

void ParentableObject::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this
    // different than for declareOptions()
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(
        rmm, "getParent", (const Object* (ParentableObject::*)() const)&ParentableObject::parent,
        (BodyDoc("Return the parent object")));

    declareMethod(
        rmm, "setParent", &ParentableObject::setParent,
        (BodyDoc("Setter for the parent object"),
         ArgDoc ("parent", "Pointer to the new parent of this object")));

    declareMethod(
        rmm, "checkParent", &ParentableObject::checkParent,
        (BodyDoc("After the m_parent field of a child has been set, this function is\n"
                 "called so that the child has a chance to check the parent (e.g. dynamic\n"
                 "type checking).  By default it just asserts that there is a parent.")));
}

void ParentableObject::build_()
{
    updateChildrensParent(this);
}

void ParentableObject::updateChildrensParent(Object* parent)
{
    if (! m_adoptive_parent) {
        // Set the backpointers of sub-objects under the current object to
        // this.  Skip nonparentable options.
        for (ObjectOptionsIterator it(this), end ; it != end  ; ++it)
        {
            if (it.getCurrentOptionFlags() & OptionBase::nonparentable)
                continue;
            
            if (const ParentableObject* cpo = dynamic_cast<const ParentableObject*>(*it)) {
                ParentableObject* po = const_cast<ParentableObject*>(cpo);
                po->setParent(parent);
                if (parent)
                    po->checkParent();
            }
        }
    }
}

void ParentableObject::setParent(Object* parent)
{
    if (m_parent == parent)
        return;

    // Behave appropriately according to the type of parent we want
    switch (m_parent_kind) {
    case AnyParent:
        break;
        
    case WeakParent:
        if (m_parent) {
            MODULE_LOG << "Object at " << ((void*)this)
                       << " (" << left(classname(),30) << ") "
                       << " ignoring candidate weak parent " << ((void*)parent)
                       << " (" << (parent? parent->classname() : string("NULL")) << ") "
                       << endl;
            return;
        }
        break;
        
    case UniqueParent:
        if (m_parent)         // Because of test above, m_parent != parent
            PLERROR("ParentableObject::setParent: for object at 0x%p (%s),\n"
                    "trying to override existing parent at 0x%p (%s) with\n"
                    "new parent at 0x%p (%s) -- parenting mode set to UniqueParent.",
                    (void*)this, classname().c_str(),
                    (void*)m_parent, m_parent->classname().c_str(),
                    (void*)parent, (parent? parent->classname().c_str() : "NULL"));
        break;
    }

    // Do some logging
    if (m_parent) {
        MODULE_LOG << "Object at " << ((void*)this)
                   << " (" << left(classname(),30) << ") "
                   << " getting parent " << ((void*)parent)
                   << " (" << (parent? parent->classname() : string("NULL")) << ") "
                   << " overriding previous " << ((void*)m_parent)
                   << " (" << m_parent->classname() << ") "
                   << endl;
    }
    else {
        MODULE_LOG << "Object at " << ((void*)this)
                   << " (" << left(classname(),30) << ") "
                   << " getting parent " << ((void*)parent)
                   << " (" << (parent? parent->classname() : string("NULL")) << ") "
                   << endl;
    }
    
    m_parent = parent;
}

void ParentableObject::checkParent() const
{
    PLASSERT( m_parent );
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

TransparentParentable::TransparentParentable(bool adoptive_parent, ParentKind pk)
    : inherited(adoptive_parent, pk)
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

void TransparentParentable::updateChildrensParent(Object* the_parent)
{
    if (the_parent == this)
        // parent() may be null -- this is fine
        inherited::updateChildrensParent(parent());
    else
        inherited::updateChildrensParent(the_parent);
}

void TransparentParentable::setParent(Object* parent)
{
    // Set it for ourselves and our children
    inherited::setParent(parent);
    updateChildrensParent(parent);
}

void TransparentParentable::checkParent() const
{
    // Forward the call to sub-objects
    for (ObjectOptionsIterator it(this), end ; it != end ; ++it)
    {
        if (it.getCurrentOptionFlags() & OptionBase::nonparentable)
            continue;
        
        if (const ParentableObject* po = dynamic_cast<const ParentableObject*>(*it))
            const_cast<ParentableObject*>(po)->checkParent();
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
