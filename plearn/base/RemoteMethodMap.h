// -*- C++ -*-

// RemoteMethodMap.h
//
// Copyright (C) 2006 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file RemoteMethodMap.h */


#ifndef RemoteMethodMap_INC
#define RemoteMethodMap_INC

// From PLearn
#include "RemoteTrampoline.h"
#include "PP.h"

// From C++ stdlib
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace PLearn {

/**
 *  Map for determining a trampoline from a method-name+arity.
 *
 *  The basic idea is to provide a thin wrapper over a standard-library map
 *  with the following features:
 *
 *  - PLearn RMI methods can be overloaded by arity (number of arguments,
 *    excluding this).  This is directly supported by this method map, which
 *    looks up a trampoline by the pair method-name:arity.
 *
 *  - A back-pointer to inherited methods is kept as well, and that method
 *    map can be searched if a requested method is not found in the derived
 *    class.  Note that this scheme handles derived-class overrides correctly,
 *    in that the most-derived map is searched first.  The current scheme
 *    supports single inheritance only (only one parent backpointer), which
 *    is the usual convention within PLearn.
 */
class RemoteMethodMap
{
public:
    //! We map from a method-name:arity pair to a trampoline
    typedef map< pair<string,int>, PP<RemoteTrampoline> > MethodMap;

public:
    //! Constructor takes an optional pointer to a map of inherited methods
    //! (obtained from the base class)
    RemoteMethodMap(const RemoteMethodMap* inherited_methods = 0)
        : m_inherited(inherited_methods)
    { }

    ~RemoteMethodMap();

    /**
     *  Establish the pointer to inherited methods.  A typical pattern
     *  within a declareMethods() function would be:
     *
     *  @code
     *  void MyClass::declareMethods(RemoteMethodMap& rmm)
     *  {
     *      rmm.inherited(inherited::getRemoteMethodMap());
     *
     *      // A bunch of declareMethod() here
     *  }
     *  @endcode
     */
    void inherited(const RemoteMethodMap& inherited)
    {
        m_inherited = &inherited;
    }
    
    //! Add a new method into the map and returns true.  If the method
    //! already exists, don't change existing entry and return false.
    //! Never touches 'inherited_methods'
    bool insert(const string& methodname, int arity,
                PP<RemoteTrampoline> trampoline);

    //! Remove a method from the map; return the number of elements deleted.
    //! Never touches 'inherited_methods'
    MethodMap::size_type erase(const string& methodname, int arity);

    //! Lookup the given method from the map.  If 'search_inherited' is true
    //! inherited methods are recursively looked at as well.  Return 0 if not
    //! found.
    const RemoteTrampoline* lookup(const string& methodname, int arity,
                                   bool search_inherited = true) const;

    //! Return the base-class method map
    const RemoteMethodMap* inheritedMethods() const
    {
        return m_inherited;
    }

    int size() const
    {
        return m_methods.size();
    }

    //! Return a begin-iterator to the elements
    MethodMap::const_iterator begin() const
    {
        return m_methods.begin();
    }

    //! Return a end-iterator to the elements
    MethodMap::const_iterator end() const
    {
        return m_methods.end();
    }
    
    //! Returns a list of all methods in the given map as pairs of (funtionname, nargs)
    vector< pair<string, int> > getMethodList() const;

    //! Returns a list of the prototypes of all the methods in the given map
    vector<string> getMethodPrototypes() const;

    //! Returns full help on the specified method in the map.
    //! If nargs is >=0 the call will launch an exception if no method
    //! with the given name and arity (number of arguments) exists in the map.
    //! If nargs is <0 the call will give full help about all 
    //! registered methods with that name (whatever their arity),
    //! or return the string "** No method named ... **"
    string getMethodHelpText(const string& methodname, int arity=-1) const;

    //! Get the method map itself
    const MethodMap& getMap() const
    { return m_methods; }

protected:
    MethodMap m_methods;                     //!< Set of methods at this level
    const RemoteMethodMap* m_inherited;      //!< Backpointer to base-class methods

private:
    // No assignment or copy-construction
    RemoteMethodMap(const RemoteMethodMap&);
    void operator=(const RemoteMethodMap&);
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
