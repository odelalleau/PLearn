// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003-2005 Olivier Delalleau
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

#include "BootstrapVMatrix.h"
#include <plearn/math/random.h>
#include <plearn/math/TMat_sort.h>

namespace PLearn {
using namespace std;

/** BootstrapVMatrix **/

PLEARN_IMPLEMENT_OBJECT(BootstrapVMatrix,
                        "A VMatrix that sees a bootstrap subset of its parent VMatrix.",
                        "It means that a random sample of the source will be taken.\n"
                        "Note that this is not a real bootstrap since a sample can only appear once."
    );

//////////////////////
// BootstrapVMatrix //
//////////////////////
BootstrapVMatrix::BootstrapVMatrix():
    rgen(new PRandom()),
    frac(0.6667),
    n_elems(-1),
    own_seed(-2), // -2 = hack value while 'seed' is still there
    seed(0),
    shuffle(false)
{}

BootstrapVMatrix::BootstrapVMatrix(VMat m, real the_frac, bool the_shuffle,
                                   long the_seed):
    rgen(new PRandom()),
    frac(the_frac),
    n_elems(-1),
    own_seed(the_seed),
    seed(0),
    shuffle(the_shuffle)
{
    this->source = m;
    build();
}

////////////////////
// declareOptions //
////////////////////
void BootstrapVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "shuffle", &BootstrapVMatrix::shuffle, OptionBase::buildoption,
                  "If set to 1, the indices will be shuffled instead of being sorted.");

    declareOption(ol, "frac", &BootstrapVMatrix::frac, OptionBase::buildoption,
                  "The fraction of elements we keep.");

    declareOption(ol, "n_elems", &BootstrapVMatrix::n_elems, OptionBase::buildoption,
                  "The absolute number of elements we keep (will override 'frac' if provided).");

    declareOption(ol, "own_seed", &BootstrapVMatrix::own_seed, OptionBase::buildoption,
                  "The random generator seed (-1 = initialized from clock, 0 = no initialization).");

    declareOption(ol, "seed", &BootstrapVMatrix::seed, OptionBase::buildoption,
                  "DEPRECATED: The random generator seed (-1 = initialized from clock, 0 = no initialization).\n"
                  "Warning: this is a global seed that may affect other PLearn objects.");

    inherited::declareOptions(ol);

    // Hide the 'indices' option, because it will be overridden at build time.
    redeclareOption(ol, "indices", &BootstrapVMatrix::indices, OptionBase::nosave,"");
    redeclareOption(ol, "indices_vmat", &BootstrapVMatrix::indices_vmat, OptionBase::nosave,"");
}

///////////
// build //
///////////
void BootstrapVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void BootstrapVMatrix::build_()
{
    if (source) {
        indices = TVec<int>(0, source.length()-1, 1); // Range-vector
        if (seed != 0)
            own_seed = -2;
        if (own_seed == -2) {
            PLDEPRECATED("In BootstrapVMatrix::build_ - You are using the deprecated option 'seed', "
                         "the 'own_seed' option (the one you should use) will thus be ignored");
            if (seed == -1)
                PLearn::seed();
            else if (seed > 0)
                manual_seed(seed);
            else if (seed != 0)
                PLERROR("In BootstrapVMatrix::build_ - The seed must be either -1 or >= 0");
            shuffleElements(indices);
        } else {
            rgen->manual_seed(own_seed);
            rgen->shuffleElements(indices);
        }
        int n = (n_elems >= 0) ? n_elems : int(round(frac * source.length()));
        indices = indices.subVec(0, n);
        if (!shuffle)
            sortElements(indices);
        // Because we changed the indices, a rebuild may be needed.
        inherited::build();
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void BootstrapVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(rgen, copies);
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
