// -*- C++ -*-

// MixtureVMatrix.cc
//
// Copyright (C) 2008 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file MixtureVMatrix.cc */


#include "MixtureVMatrix.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    MixtureVMatrix,
    "Mixes several underlying source VMat, with ponderation.",
    ""
    );

MixtureVMatrix::MixtureVMatrix():
    n_sources(0),
    period_length(0)
{
}

void MixtureVMatrix::getNewRow(int i, const Vec& v) const
{
    // The source it comes from
    int source = period[i % period_length];
    // The number of previous samples of the same source in the same period
    int occurrence = occurrences[i % period_length];
    // The index in this source
    int index = (i/period_length)*weights[source] + occurrence;

    sources[source]->getRow(index % sources[source].length(), v);
}

void MixtureVMatrix::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &MixtureVMatrix::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    declareOption(ol, "sources", &MixtureVMatrix::sources,
                  OptionBase::buildoption,
                  "The VMat to mix");

    declareOption(ol, "weights", &MixtureVMatrix::weights,
                  OptionBase::buildoption,
                  "Weights of the different sources.\n"
                  "If weights[0]==2 and weights[1]==2, then there will be\n"
                  "twice as many exambles coming from sources[0] than from\n"
                  "sources[1], regardless of the sources' length."
                  );

    declareOption(ol, "n_sources", &MixtureVMatrix::n_sources,
                  OptionBase::learntoption,
                  "Number of sources");

    declareOption(ol, "period_length", &MixtureVMatrix::period_length,
                  OptionBase::learntoption,
                  "sum(weights)");

    declareOption(ol, "period", &MixtureVMatrix::period,
                  OptionBase::learntoption,
                  "Sequence of sources to select, ensuring the proportion of\n"
                  "sources and their homogeneity ."
                  );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void MixtureVMatrix::build_()
{
    n_sources = sources.size();
    if (n_sources == 0)
        return;

    PLCHECK_MSG(sources.size() == weights.size(),
                "You should provide as many weights as sources");

    PLCHECK_MSG(length_ >= 0,
                "You should provide a length greater than 0");

    width_ = sources[0]->width();
    for (int i=1; i<n_sources; i++)
        PLCHECK_MSG(sources[i]->width() == width_,
                    "All sources should have the same width");

    if (inputsize_<0 || targetsize_<0 || weightsize_<0 || extrasize_<0
        || inputsize_ + targetsize_ + weightsize_ + extrasize_ != width_)
    {
        inputsize_ = sources[0]->inputsize();
        targetsize_ = sources[0]->targetsize();
        weightsize_ = sources[0]->weightsize();
        extrasize_ = sources[0]->extrasize();
    }

    period_length = sum(weights);
    period.resize(period_length);
    occurrences.resize(period_length);

    bool incorrect_period = false;
    for (int i=0; i<n_sources; i++)
        if (period.count(i) != weights[i])
        {
            incorrect_period = true;
            break;
        }

    if (incorrect_period)
        buildPeriod();
    for(int i=0;i<sources.length();i++)
        updateMtime(sources[i]);
}

void MixtureVMatrix::buildPeriod()
{
    TVec<int> ideal_count(n_sources);
    TVec<int> actual_count(n_sources);
    TVec<int> sources_count(n_sources);

    for (int i=0; i<period_length; i++)
    {
        // Find the source that is the most underrepresented
        ideal_count += weights;

        int max = 0;
        int argmax = -1;
        for (int j=0; j<n_sources; j++)
        {
            if (ideal_count[j] - actual_count[j] > max)
            {
                argmax = j;
                max = ideal_count[j] - actual_count[j];
            }
        }
        PLASSERT(argmax >= 0);

        period[i] = argmax;
        actual_count[argmax] += period_length;
        occurrences[i] = sources_count[argmax];
        sources_count[argmax]++;
    }

#ifdef BOUNDCHECK
    for (int i=0; i<n_sources; i++)
    {
        PLASSERT(period.count(i) == weights[i]);
        PLASSERT( (i==0 && occurrences[0]==0)
                  || period.subVec(0,i-1).count(period[i]) == occurrences[i]);
    }
#endif
}

// ### Nothing to add here, simply calls build_
void MixtureVMatrix::build()
{
    inherited::build();
    build_();
}

void MixtureVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(sources, copies);
    deepCopyField(weights, copies);
    deepCopyField(period, copies);
    deepCopyField(occurrences, copies);
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
