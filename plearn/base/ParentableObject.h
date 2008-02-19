// -*- C++ -*-

// ParentableObject.h
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

/*! \file ParentableObject.h */


#ifndef ParentableObject_INC
#define ParentableObject_INC

#include <plearn/base/Object.h>

namespace PLearn {

//#####  ParentableObject  ####################################################

/**
 *  Object which maintains a "parent" pointer as part of an object graph.
 *
 *  The purpose of \c ParentableObject is to facilitate the building of complex
 *  graphs of \c Objects.  The basic ideas are as follows:
 *
 *  - The object contains a member called \c parent, which is simply a
 *    backpointer to the "parent" object in the graph.  This is a dumb
 *    pointer to avoid cycles as the forward pointers will usually be PP's.
 *
 *  - The \c build_() method looks at all the options of itself, and for those
 *    objects that are \c ParentableObject, it sets their \c parent pointer
 *    to \c this.  (This mechanism could conceptually be put in \c Object
 *    itself, but out of caution we leave it in \c ParentableObject for now).
 *
 *  In other words, this class both provides the backpointer in an object
 *  graph, and provides the mechanism to update the backpointer according to
 *  arbitrary forward pointers (as long as the forward pointers are accessible
 *  as options through an \c ObjectOptionsIterator.)
 *
 *  We want some objects to be able to HAVE A PARENT (hence be a
 *  ParentableObject), but not to act as the parent for somebody else.  They
 *  are 'adoptive', since they are not the biological parents of their
 *  children.  An option can be specified upon construction of Parentable
 *  object that disables the parent-setting aspect, i.e. none of its subobjects
 *  can have this as their parent.  This is useful if some subobjects are
 *  pointed to by multiple ParentableObjects, and we want to control who gets
 *  to be the legitimate parent.
 *
 *  Each ParentableObject can be marked with the kind of parent it wants.  The
 *  following kinds are supported:
 *
 *  - AnyParent    : anybody that calls setParent gets to be the parent.
 *
 *  - WeakParent   : the first caller to setParent gets to be the parent, the
 *                   other ones are ignored (default).
 *
 *  - UniqueParent : the first caller to setParent gets to be the parent, the
 *                   other ones yield PLERRORs.
 */
class ParentableObject : public Object
{
    typedef Object inherited;

public:
    //! Kind of parenting relationship; see class help for details.
    enum ParentKind {
        AnyParent,
        WeakParent,
        UniqueParent
    };
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    ParentableObject(bool adoptive_parent = false, ParentKind = WeakParent);

    //! Accessor for parent object
    Object* parent()                       { return m_parent; }
    const Object* parent() const           { return m_parent; }

    //! Function that actually does the heavy work of traversing the
    //! descendants and setting _their_ backpointers to point to the
    //! specified parent
    virtual void updateChildrensParent(Object* parent);
    
    //! Setter for the parent object; virtual since "transparent parentables"
    //! might wish to forward it to children
    virtual void setParent(Object* parent);

    //! After the m_parent field of a child has been set, this function is
    //! called so that the child has a chance to check the parent (e.g. dynamic
    //! type checking).  By default it just asserts that there is a parent.
    virtual void checkParent() const;
    
    
    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(ParentableObject);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);
    
protected:
    //! Backpointer to parent
    Object* m_parent;

    //! If true, don't set the subobjects' parent to this
    bool m_adoptive_parent;

    //! The kind of parent
    ParentKind m_parent_kind;

private: 
    //! Simply call updateChildrensParent with this
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ParentableObject);


//#####  TypedParentableObject  ###############################################

/**
 *  This is a simple subclass of ParentableObject that injects type information
 *  about the parent.  Its build function also dynamically ensures that the
 *  parent is of the right type.
 */
#define TEMPLATE_DEF_TypedParentableObject  class ParentT
#define TEMPLATE_ARGS_TypedParentableObject ParentT
#define TEMPLATE_NAME_TypedParentableObject \
        string("TypedParentableObject< ") + TypeTraits<ParentT>::name() + " >"

template <class ParentT>
class TypedParentableObject : public ParentableObject
{
    typedef ParentableObject inherited;

public:
    //! Default constructor
    TypedParentableObject(bool adoptive_parent = false, ParentKind = WeakParent);
    
    //! Typed version of parent accessor (hides the inherited one; this is OK)
    ParentT* parent()
    {
        return static_cast<ParentT*>(m_parent);
    }

    const ParentT* parent() const
    {
        return static_cast<const ParentT*>(m_parent);
    }

    //! Override to ensure that the assumed parent type is satisfied
    virtual void checkParent() const;
    
    
    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.    
    PLEARN_DECLARE_TEMPLATE_OBJECT(TypedParentableObject)

    // Simply calls inherited::build() then build_() 
    virtual void build();

private: 
    //! This does the actual building. 
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_TEMPLATE_OBJECT_PTR(TypedParentableObject);

PLEARN_IMPLEMENT_TEMPLATE_OBJECT(
    TypedParentableObject,
    "Injects type information about the parent",
    "This is a simple subclass of ParentableObject that injects type information\n"
    "about the parent.  Its build function also dynamically ensures that the\n"
    "parent is of the right type."
    )

template <class T>
TypedParentableObject<T>::TypedParentableObject(bool adoptive_parent, ParentKind pk)
    : inherited(adoptive_parent, pk)
{ }

template <class T>
void TypedParentableObject<T>::build()
{
    inherited::build();
    build_();
}

template <class T>
void TypedParentableObject<T>::build_()
{
    // We simply dynamically check that the parent, if any, makes sense
    if (m_parent)
        checkParent();
}

template <class T>
void TypedParentableObject<T>::checkParent() const
{
    // We simply dynamically check that the parent, makes sense
    PLASSERT( m_parent );
    if (! dynamic_cast<T*>(m_parent))
        PLERROR("TypedParentableObject::checkParent: Expected a parent of type %s\n"
                "but got one of type %s", T::_classname_().c_str(),
                m_parent->classname().c_str());
}


//#####  TransparentParentable  ###############################################

/**
 *  Special type of ParentableObject that cannot act as a visible parent.
 *
 *  Suppose that you have the following object structure:
 *
 *    MasterManager contains a list of ObjectDescriptor
 *      Each ObjectDescriptor contains a list of ChildrenObject
 *
 *  The idea here is that ObjectDescriptor is a simple holder class that
 *  provides additional information for how ChildrenObject should be built in
 *  the context of MasterManager.  However, we want to have the situation
 *  wherein the parent() of each ChildrenObject is the MasterManager, and not
 *  the ObjectDescriptors.
 *
 *  That's the purpose of TransparentParentable: if you make ObjectDescriptor a
 *  derived class of TransparentParentable, they are skipped when going up on
 *  the children-parent paths.
 */
class TransparentParentable : public ParentableObject
{
    typedef ParentableObject inherited;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    TransparentParentable(bool adoptive_parent = false, ParentKind = WeakParent);

    //! Overridden to ensure that if "the_parent == this", we actually set the
    //! children's parent to parent().
    virtual void updateChildrensParent(Object* the_parent);
    
    //! Setter directly calls all of its parentable children; transparent
    //! of transparent should work fine and skip both of them.
    virtual void setParent(Object* parent);

    //! Just forward the call to its subobjects
    virtual void checkParent() const;
    
    
    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(TransparentParentable);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

private: 
    //! Traverse the options of *this, find the ParentableObjects, and update
    //! _their_ backpointers to point to *this
    void build_();
};


// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(TransparentParentable);

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
