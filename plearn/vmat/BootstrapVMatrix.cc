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
        "It means that a random sample of the source will be taken. Samples\n"
        "may or may not be repeated depending on the value of the option\n"
        "'allow_repetitions' (the default behavior is not to repeat samples)."
);

//////////////////////
// BootstrapVMatrix //
//////////////////////
BootstrapVMatrix::BootstrapVMatrix():
    rgen(new PRandom()),
    frac(0.6667),
    n_elems(-1),
    operate_on_bags(false),
    own_seed(-3), // Temporary hack value to detect use of deprecated option.
    seed(1827),   // Default fixed seed value (safer than time-dependent).
    shuffle(false),
    allow_repetitions(false)
{}

BootstrapVMatrix::BootstrapVMatrix(VMat m, real the_frac, bool the_shuffle,
                                   int32_t the_seed, bool allow_rep):
    rgen(new PRandom()),
    frac(the_frac),
    n_elems(-1),
    operate_on_bags(false),
    own_seed(-3),
    seed(the_seed),
    shuffle(the_shuffle),
    allow_repetitions(allow_rep)
{
    this->source = m;
    build();
}

BootstrapVMatrix::BootstrapVMatrix(VMat m, real the_frac, 
                                   PP<PRandom> the_rgen,
                                   bool the_shuffle,
                                   bool allow_rep):
    rgen(the_rgen),
    frac(the_frac),
    n_elems(-1),
    operate_on_bags(false),
    own_seed(-3),
    shuffle(the_shuffle),
    allow_repetitions(allow_rep)
{
    PLASSERT( the_rgen );
    // We obtain the seed value that was actually used to initialize the Boost
    // random number generator, to ensure this VMat is always the same after
    // consecutive builds.
    seed = int32_t(the_rgen->get_the_seed());
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

    declareOption(ol, "seed", &BootstrapVMatrix::seed, OptionBase::buildoption,
        "Random generator seed (-1 = from clock and 0 = no initialization).");

    declareOption(ol, "allow_repetitions", &BootstrapVMatrix::allow_repetitions, 
                  OptionBase::buildoption,
                  "Wether examples should be allowed to appear each more than once.",
                  OptionBase::advanced_level);

    declareOption(ol, "operate_on_bags", &BootstrapVMatrix::operate_on_bags, 
                  OptionBase::buildoption,
        "Wether to operate on bags rather than individual samples (see help\n"
        "of SumOverBagsVariable for details on bags).",
        OptionBase::advanced_level);

    declareOption(ol, "own_seed", &BootstrapVMatrix::own_seed,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED: old random generator seed",
                  OptionBase::deprecated_level);

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
    if (source) 
    { 
        updateMtime(source);
        // Ensure we are not using the deprecated 'own_seed' option.
        if (own_seed != -3) {
            PLDEPRECATED("In BootstrapVMatrix::build_ - The 'own_seed' option "
                    "is deprecated: please use 'seed' instead");
            PLASSERT( seed == 0 ); // Old default value for 'seed' was 0.
            seed = own_seed;
            own_seed = -3;
        }

        // Initialize RNG.
        rgen->manual_seed(seed);

        // Create bootstrap sample.
        int l= source.length();

        TVec< TVec<int> > bag_to_indices;
        if (operate_on_bags) {
            // Analyze bags in the source.
            Vec input, target;
            real weight;
            for (int i = 0; i < l; i++) {
                source->getExample(i, input, target, weight);
#ifndef NDEBUG
                // Safety checks on bag information.
                real bi = target.lastElement();
                PLASSERT( is_equal(round(bi), bi) );
                int bii = int(round(bi));
                PLASSERT( bii >= 0 && bii <= 3 );
#endif
                int bag_info = int(round(target.lastElement()));
                if (bag_info % 2 == 1)
                    bag_to_indices.append(TVec<int>());
                bag_to_indices.lastElement().append(i);
            }
            l = bag_to_indices.length();
        }

        int nsamp= (n_elems >= 0) ? n_elems : int(round(frac*l));

        if(allow_repetitions)
        {
            indices.resize(nsamp);
            for(int i= 0; i < nsamp; ++i)
                indices[i]= rgen->uniform_multinomial_sample(l);
        }
        else
        {
            indices = TVec<int>(0, l-1, 1); // Range-vector
            rgen->shuffleElements(indices);
            indices = indices.subVec(0, min(indices.size(), nsamp));
        }
        if (!shuffle)
            sortElements(indices);

        if (operate_on_bags) {
            // Convert bag indices back to sample indices.
            TVec<int> bag_indices = indices.copy();
            indices.resize(0);
            for (int i = 0; i < bag_indices.length(); i++)
                indices.append(bag_to_indices[bag_indices[i]]);
        }

        //if we only shuffle the rows, we remove a useless warning.
        if(frac == 1 && allow_repetitions == 0 && rows_to_remove == 0 && shuffle == 1)
            warn_if_all_rows_selected=false;
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
