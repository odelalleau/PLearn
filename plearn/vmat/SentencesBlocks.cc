// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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

#include "SentencesBlocks.h"

namespace PLearn {
using namespace std;

/** SentencesBlocks **/

SentencesBlocks::SentencesBlocks(int n_blocks, VMat d, Vec separator)
    : TVec<VMat>(n_blocks)
{
    if (n_blocks==1)
    {
        (*this)[0]=d;
        return;
    }
    int total_size = d->length();
    if (total_size < n_blocks * 2)
        PLERROR("SentencesBlocks: can't have blocks of size < 2 in average");
    Vec v(d->width());
    int b=0;
    int previous_previous_block=0,previous_beginning_of_block = 0, previous_beginning_of_sentence=0;
    int next_target = (int)(total_size / (real)n_blocks);
    for (int i=0;i<total_size && b<n_blocks-1;i++)
    {
        d->getRow(i,v);
        if (v==separator)
        {
            if (i>=next_target)
            {
                int cut=0;
                if (i-next_target < next_target-previous_beginning_of_sentence ||
                    previous_beginning_of_sentence < previous_beginning_of_block)
                    cut=i+1;
                else
                {
                    cut=previous_beginning_of_sentence;
                    previous_beginning_of_sentence = i+1;
                }
                (*this)[b++] = d.subMatRows(previous_beginning_of_block,
                                            cut-previous_beginning_of_block);
                previous_previous_block = previous_beginning_of_block;
                previous_beginning_of_block=cut;
                if (b<n_blocks)
                {
                    if (b>n_blocks-3)
                        next_target = (int)((total_size - cut) / (real)(n_blocks-b));
                    else
                        next_target = (int)(total_size * (real)(b+1.0) / n_blocks);
                }
            }
            else
                previous_beginning_of_sentence=i+1;
        }
    }
    if (b==n_blocks-1)
        (*this)[b++] = d.subMatRows(previous_beginning_of_block,
                                    total_size-previous_beginning_of_block);
    if (b<n_blocks-1) // we have to backtrack, split previous block in two
    {
        if (b<n_blocks-2)
            PLERROR("SentencesBlocks: blocks are too small!");
        if (previous_beginning_of_sentence<previous_beginning_of_block)
            PLERROR("SentencesBlocks: Blocks are too small!");
        int cut = previous_beginning_of_sentence;
        (*this)[b++] = d.subMatRows(previous_beginning_of_block,
                                    cut-previous_beginning_of_block);
        previous_beginning_of_block=cut;
        (*this)[b++] = d.subMatRows(previous_beginning_of_block,
                                    total_size-previous_beginning_of_block);
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
