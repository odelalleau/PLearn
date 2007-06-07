// -*- C++ -*-

// CheckDond2FileSequence.cc
//
// Copyright (C) 2006 Dan Popovici, Pascal Lamblin
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

// Authors: Dan Popovici

/*! \file CheckDond2FileSequence.cc */

#define PL_LOG_MODULE_NAME "CheckDond2FileSequence"
#include <plearn/io/pl_log.h>

#include "CheckDond2FileSequence.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    CheckDond2FileSequence,
    "Checks that the train set is in sequence",
    ""
);

/////////////////////////
// CheckDond2FileSequence //
/////////////////////////
CheckDond2FileSequence::CheckDond2FileSequence()
{
}

////////////////////
// declareOptions //
////////////////////
void CheckDond2FileSequence::declareOptions(OptionList& ol)
{
    declareOption(ol, "key_col", &CheckDond2FileSequence::key_col,
                  OptionBase::buildoption,
                  "The column of the sequence key.");
    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void CheckDond2FileSequence::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(key_col, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void CheckDond2FileSequence::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void CheckDond2FileSequence::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        int row;
        real prev_key;
        Vec input(train_set->width());
        train_set->getRow(0, input);
        prev_key = input[key_col];
        for (row = 1; row < train_set->length(); row++)
        {
            train_set->getRow(row, input);
            if (input[key_col] < prev_key)
            {
                cout << "CheckDond2FileSequence: train set out of sequence" << endl;
                cout << "CheckDond2FileSequence: row: " << row << " previous key: " << prev_key << " current key: " << input[key_col] << endl;
                PLERROR("CheckDond2FileSequence: we are done here");
            }
            if (input[key_col] == prev_key)
            {
                cout << "CheckDond2FileSequence: row: " << row << " previous key: " << prev_key << " current key: " << input[key_col] << endl;
            }
            prev_key = input[key_col];
            if (row % 25000 == 0) cout << "CheckDond2FileSequence: " << row << " records processed." << endl;
        }
        cout << "CheckDond2FileSequence: " << row << " records processed." << endl;
        PLERROR("CheckDond2FileSequence: we are done here");
    }
}

int CheckDond2FileSequence::outputsize() const {return 0;}
void CheckDond2FileSequence::train() {}
void CheckDond2FileSequence::computeOutput(const Vec&, Vec&) const {}
void CheckDond2FileSequence::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> CheckDond2FileSequence::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> CheckDond2FileSequence::getTrainCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
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
