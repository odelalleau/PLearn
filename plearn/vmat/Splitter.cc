// -*- C++ -*-

// Splitter.cc
//
// Copyright (C) 2002 Pascal Vincent, Frederic Morin
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

/*! \file Splitter.cc */
#include "Splitter.h"
#include "VMat.h"
#include "ConcatRowsVMatrix.h"
#include "ConcatColumnsVMatrix.h"
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    Splitter,
    "Allows to split a VMat into one or several (sub-)VMats",
    "This class is an abstract base class for mechanisms allowing to \"split\" a\n"
    "dataset into one or several partitions (or \"splits\").\n"
    "\n"
    "Thus for instance a subclass can be used to implement k-fold splits (for\n"
    "k-fold cross validation), where each of the k splits returned by\n"
    "getSplit(i=0..k-1) would be an 2-element array containing the corresponding\n"
    "training-set and test-set.\n"
    "\n"
    "A splitter is an essential part of a PTester.\n"
    );

void Splitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(dataset, copies);
}

void Splitter::setDataSet(VMat the_dataset)
{
    dataset = the_dataset;
}

// Useful splitting functions

void split(VMat d, real test_fraction, VMat& train, VMat& test, int i, bool use_all)
{
    int n = d.length();
    real ftest = test_fraction>=1.0 ? test_fraction : test_fraction*real(n);
    int ntest = int(ftest);
    int ntrain_before_test = n - (i+1)*ntest;
    int ntrain_after_test = i*ntest;
    if (use_all) {
        // See how many splits there are.
        int nsplits = int(n / ftest + 0.5);
        // See how many examples will be left.
        int nleft = n - nsplits * ntest;
        // Deduce how many examples to add in each split.
        int ntest_more = nleft / nsplits;
        // And, finally, how many splits will have one more example so that they are
        // all taken somewhere.
        int nsplits_one_more = nleft % nsplits;
        // Now recompute ntest, ntrain_before_test and ntrain_after_test.
        ntest = ntest + ntest_more;
        if (i < nsplits_one_more) {
            ntest++;
            ntrain_before_test = n - (i+1) * ntest;
        } else {
            ntrain_before_test =
                n
                - (nsplits_one_more)          * (ntest + 1)
                - (i - nsplits_one_more + 1)  * ntest;
        }
        ntrain_after_test = n - ntest - ntrain_before_test;
    }

    test = d.subMatRows(ntrain_before_test, ntest);
    if(ntrain_after_test == 0)
        train = d.subMatRows(0,ntrain_before_test);
    else if(ntrain_before_test==0)
        train = d.subMatRows(ntest, ntrain_after_test);
    else
        train = vconcat( d.subMatRows(0,ntrain_before_test),
                         d.subMatRows(ntrain_before_test+ntest, ntrain_after_test) );
}

Vec randomSplit(VMat d, real test_fraction, VMat& train, VMat& test)
{
    int ntest = int( test_fraction>=1.0 ?test_fraction :test_fraction*d.length() );
    int ntrain = d.length()-ntest;
    Vec indices(0, d.length()-1, 1); // Range-vector
    shuffleElements(indices);
    train = d.rows(indices.subVec(0,ntrain));
    test = d.rows(indices.subVec(ntrain,ntest));
    return indices;
}

void split(VMat d, real validation_fraction, real test_fraction, VMat& train, VMat& valid, VMat& test,bool do_shuffle)
{
    int ntest = int( test_fraction>=1.0 ?test_fraction :test_fraction*d.length() );
    int nvalid = int( validation_fraction>=1.0 ?validation_fraction :validation_fraction*d.length() );
    int ntrain = d.length()-(ntest+nvalid);
    Vec indices(0, d.length()-1, 1); // Range-vector
    if (do_shuffle){
        cout<<"shuffle !"<<endl;
        shuffleElements(indices);
    }
    train = d.rows(indices.subVec(0,ntrain));
    valid = d.rows(indices.subVec(ntrain,nvalid));
    test = d.rows(indices.subVec(ntrain+nvalid,ntest));
    cout<<"n_train : "<<ntrain<<endl<<"n_valid : "<<nvalid<<endl<<"n_test : "<<(d.length()-ntrain+nvalid)<<endl;
}

void randomSplit(VMat d, real validation_fraction, real test_fraction, VMat& train, VMat& valid, VMat& test)
{
    split(d,validation_fraction,test_fraction,train,valid,test,true);
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
