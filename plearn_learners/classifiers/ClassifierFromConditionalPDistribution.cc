
// -*- C++ -*-

// ClassifierFromConditionalPDistribution.cc
//
// Copyright (C) 2003-2005  Pascal Vincent & Olivier Delalleau
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
 * $Id: ClassifierFromConditionalPDistribution.cc 4412 2005-11-02 19:00:20Z tihocan $ 
 ******************************************************* */

/*! \file ClassifierFromConditionalPDistribution.cc */
#include "ClassifierFromConditionalPDistribution.h"
#include <plearn/io/PPath.h>

namespace PLearn {
using namespace std;

ClassifierFromConditionalPDistribution::ClassifierFromConditionalPDistribution() 
    : nclasses(-1),
      output_type("predicted_class")
{}

PLEARN_IMPLEMENT_OBJECT(
    ClassifierFromConditionalPDistribution,
    "Classifier that takes a ConditionalPDistribution and classifies with it.", 
    "ClassifierFromConditionalPDistribution classifies by finding the target\n"
    "class y that maximizes p(y|x), where x is the input.\n");

////////////////////
// declareOptions //
////////////////////
void ClassifierFromConditionalPDistribution::declareOptions(OptionList& ol)
{

    // Build options.

    declareOption(ol, "nclasses", &ClassifierFromConditionalPDistribution::nclasses, OptionBase::buildoption,
                  "The number of classes");

    declareOption(ol, "conditional_distribution", &ClassifierFromConditionalPDistribution::pcd, OptionBase::buildoption,
                  "ConditionalPDistribution that computes p(y|x) for all possible classes y.");

    declareOption(ol, "output_type", &ClassifierFromConditionalPDistribution::output_type, OptionBase::buildoption,
                  "Output type. Choose among: \n"
                  "- \"predicted_class\"\n"
                  "- \"class_probabilities\"\n"
                  "- \"class_log-probabilities\")\n"
                  "Note that this may change the value of output_defs of the conditional_distribution.");
 
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ClassifierFromConditionalPDistribution::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ClassifierFromConditionalPDistribution::build_()
{
    if(pcd && train_set)
    {
        PP<Dictionary> target_dict = train_set->getDictionary(inputsize());
        if(!target_dict && nclasses <= 0) PLERROR("In ClassifierFromConditionalPDistribution::build_(): There is not way to know what are the possible targets (nclasses <= 0 and no Dictionary for target field)");
        if(target_dict) nclasses = -1;
        if(output_type == "predicted_class" || output_type == "class_log-probabilities") pcd->outputs_def = "l";
        else if (output_type == "class_probabilities") pcd->outputs_def = "d";
        else PLERROR("ClassifierFromConditionalPDistribution::build_(): output_type %s is not supported", output_type.c_str());

        pcd_input.resize(train_set->inputsize() + train_set->targetsize());
        pcd_output.resize(1);
    }

}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ClassifierFromConditionalPDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(pcd, copies);
    deepCopyField(target_values, copies);
    deepCopyField(pcd_input, copies);
}

////////////////
// outputsize //
////////////////
int ClassifierFromConditionalPDistribution::outputsize() const
{
    if(nclasses > 0) return nclasses;
    else
    {
        train_set->getValues(0,inputsize(),target_values);
        return target_values.length();
    }
}

////////////
// forget //
////////////
void ClassifierFromConditionalPDistribution::forget()
{
    stage=0;
    pcd->forget();
}

///////////
// train //
///////////
void ClassifierFromConditionalPDistribution::train()
{
    if(targetsize()!=1)
        PLERROR("In ClassifierFromConditionalPDistribution::train - Expecting a targetsize of 1, not %d !!",targetsize());

    if(nstages<stage) // asking to revert to a previous stage!
        forget();  // reset the learner to stage=0

    if(stage==0)
    {
        pcd->setExperimentDirectory(getExperimentDirectory() / "PCD");
        train_set->defineSizes(inputsize()+1,targetsize()-1,weightsize());
        pcd->setTrainingSet(train_set);
        PP<VecStatsCollector> train_stats = new VecStatsCollector();
        train_stats->setFieldNames(pcd->getTrainCostNames());
        pcd->setTrainStatsCollector(train_stats);
        pcd->nstages = nstages;
        if (verbosity >= 2)
        pout << ">>> Training ConditionalPDistribution" << endl;    
        pcd->train();
        if (verbosity >= 2)
        pout << ">>> Training is over" << endl;    
        train_set->defineSizes(inputsize(),targetsize(),weightsize());
    }
    stage = nstages; // trained!
}

///////////////////
// computeOutput //
///////////////////
void ClassifierFromConditionalPDistribution::computeOutput(const Vec& input, Vec& output) const
{    
    pcd_input.subVec(0,inputsize()) << input;
    if(nclasses <= 0)
    {   
        train_set->getValues(input, inputsize(),target_values);
        output.resize(target_values.length());
        for(int i=0; i<target_values.length(); i++)
        {
            pcd_input[inputsize()] = target_values[i];            
            pcd->computeOutput(pcd_input, pcd_output);
            output[i] = pcd_output[0];
        }
    }
    else
    {
        output.resize(nclasses);
        for(int c=0; c<nclasses; c++)
        {
            pcd_input[inputsize()] = c;
            pcd->computeOutput(pcd_input, pcd_output);
            output[c] = pcd_output[0];
        }     
    }

    if(output_type == "predicted_class") 
    {
        if(nclasses <= 0)
            output[0] = (int)target_values[argmax(output)];
        else
            output[0] = argmax(output);
        output.resize(1);
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void ClassifierFromConditionalPDistribution::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                    const Vec& target, Vec& costs) const
{
    if(output_type == "predicted_class")
    {
        costs[0] = (int) (target[0] != output[0]);
    }
    else
    {
        int c;
        if(nclasses <= 0)
        {
            train_set->getValues(input,inputsize(),target_values);
            c = target_values.find(target[0]);           
        }
        else
            c = (int)target[0];
        costs[0] = (int) (argmax(output) != c);
        if(output_type == "class_probabilities") costs[1] = safeflog(output[c]);
        else costs[1] = output[c];
    }
}                                

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> ClassifierFromConditionalPDistribution::getTestCostNames() const
{
    TVec<string> cnames;    
    if(output_type == "predicted_class")
    {
        cnames.resize(1);
        cnames[0] = "class_error";
    }
    else
    {
        cnames.resize(2);
        cnames[0] = "class_error";
        cnames[1] = "NLL";
    }
    return cnames;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> ClassifierFromConditionalPDistribution::getTrainCostNames() const
{
    return TVec<string>();
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
