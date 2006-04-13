// -*- C++ -*-

// NGramTree.cc
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

/*! \file NGramTree.cc */


#include "NGramTree.h"

namespace PLearn {
using namespace std;

NGramTree::NGramTree()
{
    forget();
}

NGramTree::NGramTree(PP<SymbolNode> root_)
{
    setRoot(root_);
}

PLEARN_IMPLEMENT_OBJECT(NGramTree,
                        "NGram tree that counts frequencies",
                        "NGramTree uses a suffix tree on the context ( i.e. the (n-1)gram) to count frequencies of ngrams and\n"
                        "and their suffixes with their corresponding normalization factor for probability estimation.");


void NGramTree::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "root", &NGramTree::root, OptionBase::learntoption,
                  "root of the NGramTree");

    inherited::declareOptions(ol);
}

void NGramTree::build_(){}

// ### Nothing to add here, simply calls build_
void NGramTree::build()
{
    inherited::build();
    build_();
}

void NGramTree::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    //PLERROR("NGramTree::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void NGramTree::add(TVec<int> ngram)
{
    if(ngram.length() == 0)
        return;
    PP<SymbolNode> it = root;
    it->incr(ngram[ngram.length()-1]);
    for(int i=ngram.length()-2; i>=0; i--)
    {
        it = it->add(ngram[i]);
        it->incr(ngram[ngram.length()-1]);
    }
}

TVec<int> NGramTree::freq(TVec<int> ngram)
{
    TVec<int> ret(ngram.length());
    ret.fill(0);
    ret[0] = root->freq(ngram[ngram.length()-1]);

    int n=1;
    PP<SymbolNode> it = root;
    for(int i=ngram.length()-2; i>=0; i--)
    {
        it = it->child(ngram[i]);
        if(!it)
            break;
        ret[n] = it->freq(ngram[ngram.length()-1]);
        n++;
    }
    return ret;
}

TVec<int> NGramTree::normalization(TVec<int> ngram)
{
    TVec<int> ret(ngram.length());
    ret.fill(0);
    ret[0] = root->freq();

    int n=1;
    PP<SymbolNode> it = root;
    for(int i=ngram.length()-2; i>=0; i--)
    {
        it = it->child(ngram[i]);
        if(!it)
            break;
        ret[n] = it->freq();
        n++;
    }
    return ret;
}

int NGramTree::n_children(TVec<int> sequence)
{
    if(sequence.length()==0)
        return 0;

    PP<SymbolNode> it = root;
    for(int i=sequence.length()-2; i>=0; i--)
    {
        it = it->child(sequence[i]);
        if(!it)
            return 0;
    }

    return it->n_children();;
}

TVec<int> NGramTree::n_freq(TVec<int> sequence)
{
    TVec<int> ret(0);
    if(sequence.length()==0)
        return ret;

    ret.resize(sequence.length());
    ret.fill(0);

    PP<SymbolNode> it = root;
    int n=0;
    ret[n++] = it->n_freq();
    for(int i=sequence.length()-2; i>=0; i--)
    {
        it = it->child(sequence[i]);
        if(!it)
            return ret;
        ret[n++] = it->n_freq();
    }
    return ret;
}
/*
  TVec<PP<NGramTree> > NGramTree::getSubTrees(TVec<int> sequence)
  {
  PP<SymbolNode> it = root;

  TVec<PP<NGramTree> > ret(0);

  if(sequence.length()==0)
  {
  ret.resize(it->n_children());
  TVec<PP<SymbolNode> > nodes = it->getChildren();
  for(int j=0; j<nodes.length(); j++)
  ret[j] = new NGramTree(nodes[j]);
  return ret;
  }

  for(int i=sequence.length()-1; i>=0; i--)
  {
  it = it->child(sequence[i]);
  if(!it)
  break;
  if(i==0)
  {
  ret.resize(it->n_children());
  TVec<PP<SymbolNode> > nodes = it->getChildren();
  for(int j=0; j<nodes.length(); j++)
  {
  ret[j] = new NGramTree(nodes[j]);
  }
  return ret;
  }
  }
  return ret;
  }
*/

void NGramTree::forget()
{
    root = new SymbolNode();
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
