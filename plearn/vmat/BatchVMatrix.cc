// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Olivier Delalleau
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

#include "BatchVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BatchVMatrix,
    "Replicates small parts of a matrix into mini-batches",
    "VMat class that replicates small parts of a matrix (mini-batches), \n"
    "so that each mini-batch appears twice (consecutively).");

BatchVMatrix::BatchVMatrix()
    : batch_size(0),
      last_batch(-1),
      last_batch_size(-1)
{ }

  
////////////////////
// declareOptions //
////////////////////
void BatchVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "m", &BatchVMatrix::m, OptionBase::buildoption,
                  "The matrix viewed by the BatchVMatrix\n");
    declareOption(ol, "batch_size", &BatchVMatrix::batch_size, OptionBase::buildoption,
                  "The size of each mini-batch\n");
    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void BatchVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(m, copies);
}

///////////
// build //
///////////
void BatchVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void BatchVMatrix::build_()
{
    if (m) {
        if (batch_size < 1)
            PLERROR("BatchVMatrix::build_: the 'batch_size' option must be nonnegative");
    
        width_ = m->width();
        length_ = m->length() * 2;
        fieldinfos = m->getFieldInfos();
        last_batch = (m->length()-1) / batch_size;
        last_batch_size = m->length() % batch_size;
        if (last_batch_size == 0)
            last_batch_size = batch_size;
    }
}

/////////
// get //
/////////
real BatchVMatrix::get(int i, int j) const {
    int n_batch = i / (2 * batch_size);
    int k = batch_size;
    if (n_batch == last_batch) {
        // This is the last batch
        k = last_batch_size;
    }
    int i_ = n_batch * batch_size + (i - n_batch * 2 * batch_size) % k;
    return m->get(i_, j);
}

/////////
// put //
/////////
void BatchVMatrix::put(int i, int j, real value) {
    int n_batch = i / (2 * batch_size);
    int i_ = n_batch * batch_size + (i - n_batch * 2 * batch_size) % batch_size;
    m->put(i_, j, value);
}

} // end of namespcae PLearn


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
