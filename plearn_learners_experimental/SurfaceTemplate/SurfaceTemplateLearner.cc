// -*- C++ -*-

// SurfaceTemplateLearner.cc
//
// Copyright (C) 2006 Pascal Lamblin 
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

/*! \file SurfaceTemplateLearner.cc */


#include "SurfaceTemplateLearner.h"
#include "ScoreLayerVariable.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SurfaceTemplateLearner,
    "Neural-network to learn from molecular alignment.",
    ""
);

////////////////////////////
// SurfaceTemplateLearner //
////////////////////////////
SurfaceTemplateLearner::SurfaceTemplateLearner() 
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

////////////////////
// declareOptions //
////////////////////
void SurfaceTemplateLearner::declareOptions(OptionList& ol)
{
    //declareOption(ol, "fixed_output_weights", SurfaceTemplateLearner::fixed_output_weights, OptionBase::buildoption,

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void SurfaceTemplateLearner::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

///////////
// build //
///////////
void SurfaceTemplateLearner::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SurfaceTemplateLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("SurfaceTemplateLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// setTrainingSet //
////////////////////
void SurfaceTemplateLearner::setTrainingSet(VMat training_set,
                                            bool call_forget)
{
    // Rebuild the internal score layer.
    PP<ScoreLayerVariable> score_layer =
        (ScoreLayerVariable*) ((Variable*) first_hidden_layer);
    score_layer->templates_source = training_set;
    score_layer->setMappingsSource(training_set);
    score_layer->build();

    inherited::setTrainingSet(training_set, call_forget);
}

//////////
// test //
//////////
void SurfaceTemplateLearner::test(VMat testset,
                                  PP<VecStatsCollector> test_stats,
                                  VMat testoutputs, VMat testcosts) const
{
    PP<ScoreLayerVariable> score_layer =
        (ScoreLayerVariable*) ((Variable*) first_hidden_layer);
    score_layer->setMappingsSource(testset);
    inherited::test(testset, test_stats, testoutputs, testcosts);
}

/*
void SurfaceTemplateLearner::forget()
{
    inherited::forget();
}
*/
    
///////////
// train //
///////////
void SurfaceTemplateLearner::train()
{
    PP<ScoreLayerVariable> score_layer =
        (ScoreLayerVariable*) ((Variable*) first_hidden_layer);
    score_layer->setMappingsSource(train_set);
    inherited::train();
}

/*
void SurfaceTemplateLearner::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);
    // ...
}    

void SurfaceTemplateLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output. 
// ...
}                                
*/

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
