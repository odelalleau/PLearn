// -*- C++ -*-

// SymbolNode.h
//
// Copyright (C) 2004 Hugo Larochelle
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
 ******************************************************* */

// Authors: Hugo Larochelle

/*! \file SymbolNode.h */


#ifndef SymbolNode_INC
#define SymbolNode_INC

#include <plearn/base/Object.h>
#include <plearn/base/ms_hash_wrapper.h>

namespace PLearn {
using namespace std;

class SymbolNode: public Object
{

private:

    typedef Object inherited;

protected:
    // *********************
    // * protected options *
    // *********************

    //! Normalisation factor
    int frequence;

    //! Children and their frequences
    map<int,PP<SymbolNode> > children;
    map<int,int> frequencies;

public:

    // ************************
    // * public build options *
    // ************************

    int symbol;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    SymbolNode();

    SymbolNode(int symbol_);

    // ******************
    // * Object methods *
    // ******************

private:
    //! This does the actual building.
    void build_();

protected:
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

public:
    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(SymbolNode);

    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Adds a child to this node
    PP<SymbolNode> add(int child_);

    //! Increments the node's frequency (normalization factor)
    void incr(){frequence++;}

    //! Increments the given symbol's frequency in frequencies map
    void incr(int symbol);

    //! Gives frequency of the node (normalization factor)
    int freq(){return frequence;}

    //! Gives frequence of a given symbol at that node
    int freq(int symbol);

    //! Gives the number of children
    int n_children(){return children.size();}

    //! Gives the number of different symbol in frequencies map
    int n_freq(){return frequencies.size();}

    //! Gives the corresponding SymbolNode child, or 0 if nonexistant
    PP<SymbolNode> child(int child_);

    //! Gives all children of current node
    TVec<PP<SymbolNode> > getChildren();

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SymbolNode);

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
