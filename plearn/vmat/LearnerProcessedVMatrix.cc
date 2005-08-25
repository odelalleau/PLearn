
// -*- C++ -*-

// LearnerProcessedVMatrix.cc
//
// Copyright (C) 2003  Pascal Vincent 
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

/*! \file LearnerProcessedVMatrix.cc */
#include "LearnerProcessedVMatrix.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(LearnerProcessedVMatrix, "ONE LINE DESCR", 
                        "LearnerProcessedVMatrix implements a VMatrix processed on the fly by a learner (which will optionally be first trained on the source vmatrix)");


LearnerProcessedVMatrix::LearnerProcessedVMatrix()
    :train_learner('-')
{
    build_();
}

void LearnerProcessedVMatrix::getNewRow(int i, const Vec& v) const
{
    static Vec sv;
    sv.resize(source.width());
    source->getRow(i,sv);
    int nin = source->inputsize();
    int nout = learner->outputsize();
    Vec input = sv.subVec(0,nin);
    Vec output = v.subVec(0,nout);
    learner->computeOutput(input,output);
    int rest = source.width()-nin;
    v.subVec(nout,rest) << sv.subVec(nin,rest);
}

void LearnerProcessedVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "source", &LearnerProcessedVMatrix::source, OptionBase::buildoption,
                  "The source vmatrix whose inputs wil be porcessed by the learner.\n"
                  "If present, the target and weight columns will be appended to the processed input in the resulting matrix.");
  
    declareOption(ol, "learner", &LearnerProcessedVMatrix::learner, OptionBase::buildoption,
                  "The learner used to process the VMat's input.");

    declareOption(ol, "train_learner", &LearnerProcessedVMatrix::train_learner, OptionBase::buildoption, 
                  "Indicates if the learner should be trained on the source, upon building, and if so on what part.\n"
                  " '-': don't train \n"
                  " 'S': supervised training using input and target (possibly weighted if weight is  present) \n"
                  " 'U': unsupervised training using only input part (possibly weighted if weight is present). \n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LearnerProcessedVMatrix::build_()
{
    if (source && learner) {
        switch(train_learner) {
        case '-':
            break;
        case 'S':
            learner->setTrainingSet(source);
            learner->setTrainStatsCollector(new VecStatsCollector());
            learner->train();
            break;
        case 'U':
        {
            VMat inputs = source.subMatColumns(0,source->inputsize());
            learner->setTrainingSet(inputs);
            learner->setTrainStatsCollector(new VecStatsCollector());
            learner->train();
        }
        }
        length_ = source->length();
        width_ = learner->outputsize() + source->targetsize() + source->weightsize();
    }
}

// ### Nothing to add here, simply calls build_
void LearnerProcessedVMatrix::build()
{
    inherited::build();
    build_();
}

void LearnerProcessedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(source, copies);
    deepCopyField(learner, copies);
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
