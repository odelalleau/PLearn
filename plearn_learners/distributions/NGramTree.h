// -*- C++ -*-

// NGramTree.h
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

/*! \file NGramTree.h */


#ifndef NGramTree_INC
#define NGramTree_INC

#include <plearn/base/Object.h>
#include <plearn_learners/distributions/SymbolNode.h>

namespace PLearn {
using namespace std;

class NGramTree: public Object
{

private:

    typedef Object inherited;

protected:
    // *********************
    // * protected options *
    // *********************

    PP<SymbolNode> root;

public:

    // ************************
    // * public build options *
    // ************************

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    NGramTree();

    //! Makes an NGramTree from a given root
    NGramTree(PP<SymbolNode> root_);

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
    PLEARN_DECLARE_OBJECT(NGramTree);

    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Adds a ngram to the tree
    void add(TVec<int> ngram);

    //! Gives frequencies of the ngram, 1gram, ..., (n-1)gram and ngram (total frequency)
    TVec<int> freq(TVec<int> ngram);

    //! Returns the freqency maps in the paths corresponding to the ngram.
    //! Note that w^i in the ngram w^i_{i-n+1} is not needed, so
    //! it is ignored in the ngram field.
    TVec<map<int,int>*> freqs(TVec<int> ngram);

    //! Gives the normalization factor for the 1gram, ..., (n-1)gram and ngram for max. likelihood estimator
    TVec<int> normalization(TVec<int> ngram);

    //! Gives the number of children of the node corresponding to the given sequence
    //! Sequence is w^i_{i-n+1}, w^i is ignored
    int n_children(TVec<int> sequence);

    //! Gives the number of different symbols in frequencies map of the nodes corresponding to the given sequence
    //! This could be noted as N+1(w^{i-1}_{i-n+1}*)
    //! Sequence is w^i_{i-n+1}, w^i is ignored
    TVec<int> n_freq(TVec<int> sequence);

    /* Hugo: not useful
    //! Gives the subtrees of the node corresponding to the given sequence
    // WARNING: the frequency counts don't mean anything then...
    //          the tree can only be usefull for counting the number of nodes
    //          for example...
    TVec<PP<NGramTree> > getSubTrees(TVec<int> sequence);
    */

    //! Sets root to the given SymbolNode
    void setRoot(PP<SymbolNode> root_){root = root_;}

    //! Reinitialize the NGramTree
    void forget();

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NGramTree);

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
