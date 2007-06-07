// -*- C++ -*-

// SecondIterationTester.cc
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
 * $Id: SecondIterationTester.cc 5587 2006-05-12 16:31:54Z plearner $ 
 ******************************************************* */

/*! \file SecondIterationTester.cc */
#include "SecondIterationTester.h"


namespace PLearn {
using namespace std;

SecondIterationTester::SecondIterationTester() 
{}

PLEARN_IMPLEMENT_OBJECT(
    SecondIterationTester,
    "Manages a learning experiment, with training and estimation of generalization error.", 
    "The SecondIterationTester class allows you to describe a typical learning experiment that you wish to perform, \n"
    "as a training/testing of a learning algorithm on a particular dataset.\n"
    "The splitter is used to obtain one or several (such as for k-fold) splits of the dataset \n"
    "and training/testing is performed on each split. \n"
    "Requested statistics are computed, and all requested results are written in an appropriate \n"
    "file inside the specified experiment directory. \n"
    "Statistics can be either specified entirely from the 'statnames' option, or built from\n"
    "'statnames' and 'statmask'. For instance, one may set:\n"
    "   statnames = [ \"NLL\" \"mse\" ]\n"
    "   statmask  = [ [ \"E[*]\" ] [ \"test#1-2#.*\" ] [ \"E[*]\" \"STDERROR[*]\" ] ]\n"
    "and this will compute:\n"
    "   E[test1.E[NLL]], STDERROR[test1.E[NLL]], E[test2.E[NLL]], STDERROR[test2.E[NLL]]\n"
    "   E[test1.E[mse]], STDERROR[test1.E[mse]], E[test2.E[mse]], STDERROR[test2.E[mse]]\n"
    );


void SecondIterationTester::declareOptions(OptionList& ol)
{
    inherited::declareOptions(ol);
}

void SecondIterationTester::build_()
{
}

// ### Nothing to add here, simply calls build_
void SecondIterationTester::build()
{
    inherited::build();
    build_();
}

////////////////////////////
// setExperimentDirectory //
////////////////////////////
void SecondIterationTester::setSplitter(string splitter_template)
{ 
 //   splitter = ::PLearn::deepCopy(splitter_template);
    splitter->build();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SecondIterationTester::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
    inherited::makeDeepCopyFromShallowCopy(copies);
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
