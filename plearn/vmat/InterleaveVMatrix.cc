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

#include "InterleaveVMatrix.h"

namespace PLearn {
using namespace std;


/** InterleaveVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
    InterleaveVMatrix,
    "Interleave several VMats row-wise",
    "This class interleaves several VMats (with consecutive rows always coming\n"
    "from a different source VMat) thus possibly including more than once the\n"
    "rows of the small VMats.  For example, if source1.length()==10 and\n"
    "source2.length()==30 then the resulting VM will have 60 rows, and 3\n"
    "repetitions of each row of source1, with rows taken as follows:\n"
    "\n"
    "  source1.row(0), source2.row(0), source1.row(1), source2.row(1), ...,\n"
    "  source1.row(9), source2.row(9), source1.row(0), cource2.row(10), ...\n"
    "\n"
    "Note that if source2.length() is not a multiple of source1.length() some\n"
    "records from source1 will be repeated once more than others.\n"
    );

InterleaveVMatrix::InterleaveVMatrix()
{ }

InterleaveVMatrix::InterleaveVMatrix(TVec<VMat> the_sources)
    : sources(the_sources)
{
    if (sources.size() > 0)
        build();
}

InterleaveVMatrix::InterleaveVMatrix(VMat source1, VMat source2)
    : sources(2)
{
    sources[0] = source1;
    sources[1] = source2;
    build();
}

void InterleaveVMatrix::build()
{
    inherited::build();
    build_();
}

void InterleaveVMatrix::build_()
{
    if (sources) {
        int n = sources.size();
        if (n<1)
            PLERROR("InterleaveVMatrix expects >= 1 sources, got %d",n);

        width_ = sources[0]->width();
        int maxl = 0;
        for (int i = 0; i < n; i++) {
            if (sources[i]->width() != width_)
                PLERROR("InterleaveVMatrix: source %d has %d width, while 0-th has %d",
                        i, sources[i]->width(), width_);
            int l = sources[i]->length();
            if (l > maxl)
                maxl=l;
            updateMtime(sources[i]);
        }
        length_ = n * maxl;

        // Finally copy remaining meta information from first VMatrix
        setMetaInfoFrom(sources[0]);
    }
}

void InterleaveVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "sources", &InterleaveVMatrix::sources,
                  OptionBase::buildoption,
                  "Set of VMats to be concatenated");

    inherited::declareOptions(ol);
}

real InterleaveVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j>=width())
        PLERROR("In InterleaveVMatrix::get OUT OF BOUNDS");
#endif
    int n = sources.size();
    int m = i%n; // which source
    int pos = int(i/n) % sources[m].length(); // position within sources[m]
    return sources[m]->get(pos,j);
}

void InterleaveVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j+v.length()>width())
        PLERROR("In InterleaveVMatrix::getRow OUT OF BOUNDS");
#endif
    int n = sources.size();
    int m = i%n; // which source
    int pos = int(i/n) % sources[m].length(); // position within sources[m]
    sources[m]->getSubRow(pos, j, v);
}

void InterleaveVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(sources, copies);
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
