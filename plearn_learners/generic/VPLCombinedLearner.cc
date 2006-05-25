// -*- C++ -*-

// VPLCombinedLearner.cc
//
// Copyright (C) 2005, 2006 Pascal Vincent 
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
 * $Id: VPLCombinedLearner.cc 5480 2006-05-03 18:57:39Z plearner $ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VPLCombinedLearner.cc */


#include "VPLCombinedLearner.h"
#include <plearn/vmat/ProcessingVMatrix.h>
#include <plearn/vmat/FilteredVMatrix.h>
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

VPLCombinedLearner::VPLCombinedLearner() 
    :orig_inputsize(-1),
     orig_targetsize(-1)
{
}

PLEARN_IMPLEMENT_OBJECT(
    VPLCombinedLearner,
    "Learner that will train several sub-learners and whose output will be a VPL-expressed function of the outputs of the sub-learnes.",
    "See VMatLanguage for the definition of the allowed VPL syntax.\n"
    "To allow sub-learners to get their own particular view of the training set,\n"
    "it is often convenient to use VPLPreprocessedLearners for the sub-learners.\n"
    );

void VPLCombinedLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &VPLCombinedLearner::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(ol, "sublearners", &VPLCombinedLearner::sublearners_,
                  OptionBase::buildoption,
                  "The list of sub-learners that will receive the training set.");

    declareOption(ol, "output_prg", &VPLCombinedLearner::output_prg, OptionBase::buildoption,
                  "Program string in VPL language to compute this learner's outputs\n"
                  "from a concatenation of the raw input fields and the sublearners' outputs,\n"
                  "renamed as learner0.outputname learner1.outputname, etc... \n"
                  "Note that outputs are often named out0, out1, out2, ...\n"
                  "Note that new outputnames must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll output the sub-learner's outputs.\n");

    declareOption(ol, "costs_prg", &VPLCombinedLearner::costs_prg, OptionBase::buildoption,
                  "Program string in VPL language to obtain postprocessed test costs\n"
                  "from a concatenation of the raw input fields and arget fields, \n"
                  "and the sublearners' outputs and test costs.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll output the underlying learner's test costs.\n"
                  "Note that this processing is only applied to test costs, not to train costs which are returned as is.");

    declareOption(ol, "orig_fieldnames", &VPLCombinedLearner::orig_fieldnames, OptionBase::learntoption,
                  "original fieldnames of the training set");
    declareOption(ol, "orig_inputsize", &VPLCombinedLearner::orig_inputsize, OptionBase::learntoption,
                  "original inputsize of the training set");
    declareOption(ol, "orig_targetsize", &VPLCombinedLearner::orig_targetsize, OptionBase::learntoption,
                  "original targetsize of the training set");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void VPLCombinedLearner::build_()
{
    if(train_set.isNull() && (orig_inputsize>0 || orig_targetsize>0) ) // we're probably reloading a saved VPLCombinedLearner
    {
        initializeOutputPrograms();
    }
    else
        initializeCostNames();
}

// ### Nothing to add here, simply calls build_
void VPLCombinedLearner::build()
{
    inherited::build();
    build_();
}

void VPLCombinedLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.

    deepCopyField(sublearners_, copies);    

    output_prg_.makeDeepCopyFromShallowCopy(copies);
    costs_prg_.makeDeepCopyFromShallowCopy(copies);
 
    deepCopyField(outputnames_, copies);
    deepCopyField(costnames_, copies);
    deepCopyField(invec_for_output_prg, copies);
    deepCopyField(invec_for_costs_prg, copies);

    deepCopyField(sublearners_outputsizes, copies);
    deepCopyField(sublearners_ntestcosts, copies);
    deepCopyField(orig_fieldnames, copies);
}

void VPLCombinedLearner::setValidationSet(VMat validset)
{
    inherited::setValidationSet(validset);
    for(int k=0; k<sublearners_.length(); k++)
        sublearners_[k]->setValidationSet(validset);
}

void VPLCombinedLearner::setTrainStatsCollector(PP<VecStatsCollector> statscol)
{
    inherited::setTrainStatsCollector(statscol);
    for(int k=0; k<sublearners_.length(); k++)
        sublearners_[k]->setTrainStatsCollector(new VecStatsCollector());
}

int VPLCombinedLearner::outputsize() const
{
    return outputnames_.size();
}

void VPLCombinedLearner::setExperimentDirectory(const PPath& the_expdir)
{
    inherited::setExperimentDirectory(the_expdir);
    for(int k=0; k<sublearners_.length(); k++)
        sublearners_[k]->setExperimentDirectory(the_expdir/("SubLearner_"+tostring(k)));
}

void VPLCombinedLearner::forget()
{
    for(int k=0; k<sublearners_.length(); k++)
        sublearners_[k]->forget();
    stage = 0;
}
    
void VPLCombinedLearner::train()
{
    for(int k=0; k<sublearners_.length(); k++)
        sublearners_[k]->train();
    ++stage;
}

void VPLCombinedLearner::initializeOutputPrograms()
{
    TVec<string> orig_input_fieldnames = orig_fieldnames.subVec(0,orig_inputsize);
    TVec<string> orig_target_fieldnames = orig_fieldnames.subVec(orig_inputsize, orig_targetsize);

    int nlearners = sublearners_.length();
    sublearners_outputsizes.resize(nlearners);
    sublearners_ntestcosts.resize(nlearners);

    TVec<string> infields_for_output_prg = orig_input_fieldnames.copy();
    TVec<string> infields_for_costs_prg = concat(orig_input_fieldnames,orig_target_fieldnames);

    outputnames_.resize(0);
    costnames_.resize(0);

    for(int k=0; k<nlearners; k++)
    {        
        char tmp[100];
        sprintf(tmp,"learner%d.",k);
        string prefix(tmp);

        int nout = sublearners_[k]->outputsize();
        sublearners_outputsizes[k] = nout;
        TVec<string> outputnames = sublearners_[k]->getOutputNames();
        for(int p=0; p<nout; p++)
        {
            string outname = prefix+outputnames[p];
            if(output_prg.empty())
                outputnames_.append(outname);
            else
                infields_for_output_prg.append(prefix+outputnames[p]);
            
            if(!costs_prg.empty())
                infields_for_costs_prg.append(prefix+outputnames[p]);
        }

        int ntest = sublearners_[k]->nTestCosts();
        sublearners_ntestcosts[k] = ntest;
        TVec<string> testcostnames = sublearners_[k]->getTestCostNames();
        for(int p=0; p<ntest; p++)
        {
            string costname = prefix+testcostnames[p];
            if(costs_prg.empty())
                costnames_.append(costname);
            else
                infields_for_costs_prg.append(costname);
        }
    }

    if(!output_prg.empty())
    {
        output_prg_.setSourceFieldNames(infields_for_output_prg);
        output_prg_.compileString(output_prg, outputnames_);
    }

    if(!costs_prg.empty())
    {
        costs_prg_.setSourceFieldNames(infields_for_costs_prg);
        costs_prg_.compileString(costs_prg, costnames_);
    }

}

void VPLCombinedLearner::initializeCostNames()
{
    int nlearners = sublearners_.length();
    sublearners_ntestcosts.resize(nlearners);

    costnames_.resize(0);

    for(int k=0; k<nlearners; k++)
    {        
        char tmp[100];
        sprintf(tmp,"learner%d.",k);
        string prefix(tmp);

        int ntest = sublearners_[k]->nTestCosts();
        sublearners_ntestcosts[k] = ntest;
        TVec<string> testcostnames = sublearners_[k]->getTestCostNames();
        for(int p=0; p<ntest; p++)
        {
            string costname = prefix+testcostnames[p];
            if(costs_prg.empty())
                costnames_.append(costname);
        }
    }

    if(!costs_prg.empty())
        VMatLanguage::getOutputFieldNamesFromString(costs_prg, costnames_);

}

void VPLCombinedLearner::setTrainingSet(VMat training_set, bool call_forget)
{
    bool training_set_has_changed = !train_set || !(train_set->looksTheSameAs(training_set));
    if (call_forget && !training_set_has_changed)
    {
        // In this case, the sublearner's build() will not have been called, which may
        // cause trouble if it updates data from the training set.
        // NOTE: I'M NOT QUITE SURE WHAT THE ABOVE SITUATION MEANS. 
        // BUT FOR NOW, LET'S BELIEVE IT'S TRUE, SO WE MUST CALL build_ ON THE SUBLEARNERS 
        for(int k=0; k<sublearners_.length(); k++)
            sublearners_[k]->build();
    }
        
    orig_fieldnames = training_set->fieldNames();
    orig_inputsize  = training_set->inputsize();
    orig_targetsize  = training_set->targetsize();

    for(int k=0; k<sublearners_.length(); k++)
        sublearners_[k]->setTrainingSet(training_set, call_forget);

    inherited::setTrainingSet(training_set, call_forget); // will call forget if needed

    initializeOutputPrograms();
}


void VPLCombinedLearner::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());
    int nlearners = sublearners_.length();    

    int ninputs = inputsize();
    int outpos = 0;
    if(!output_prg.empty()) // no output_prg: output is simply concatenation of sublearner's outputs
    {
        invec_for_output_prg.resize(output_prg_.inputsize());
        invec_for_output_prg.subVec(outpos, ninputs) << input; // copy input part into invec_for_output_prg
        outpos += ninputs;
    }

    Vec outvec;    
    for(int k=0; k<nlearners; k++)
    {
        int nout = sublearners_outputsizes[k];
        if(output_prg.empty()) // no output_prg: output is simply concatenation of sublearner's outputs
            outvec = output.subVec(outpos,nout);
        else
            outvec = invec_for_output_prg.subVec(outpos, nout);
        sublearners_[k]->computeOutput(input, outvec);
        outpos += nout;
    }

    if(!output_prg.empty())
        output_prg_.run(invec_for_output_prg, output);
}

void VPLCombinedLearner::computeOutputAndCosts(const Vec& input, const Vec& target, 
                                                   Vec& output, Vec& costs) const
{ 
    output.resize(outputsize());
    costs.resize(nTestCosts());

    int ilen = input.length();
    int tlen = target.length();
    assert(ilen==inputsize());
    assert(tlen==targetsize());

    output.resize(outputsize());
    int nlearners = sublearners_.length();    

    int ninputs = inputsize();
    int ntargets= targetsize();
    int outpos = 0;
    int costspos = 0;

    if(!output_prg.empty())
    {
        invec_for_output_prg.resize(output_prg_.inputsize());
        invec_for_output_prg.subVec(outpos,ninputs) << input;
        outpos += ninputs;
    }
    if(!costs_prg.empty())
    {
        invec_for_costs_prg.resize(costs_prg_.inputsize());
        invec_for_costs_prg.subVec(costspos,ninputs) << input;
        costspos += ninputs;
        invec_for_costs_prg.subVec(costspos,ntargets) << target;
        costspos += ntargets;
    }

    Vec outvec;
    Vec costvec;
    for(int k=0; k<nlearners; k++)
    {
        int nout = sublearners_outputsizes[k];
        int ncosts = sublearners_ntestcosts[k];

        if(output_prg.empty())
            outvec = output.subVec(outpos, nout);
        else
            outvec = invec_for_output_prg.subVec(outpos, nout);

        if(costs_prg.empty())
            costvec = costs.subVec(costspos, ncosts);
        else
            costvec = invec_for_costs_prg.subVec(costspos+nout, ncosts);
        
        sublearners_[k]->computeOutputAndCosts(input, target, outvec, costvec);
        if(!costs_prg.empty()) // copy outvec into correct position in invec_for_costs_prg
        {
            invec_for_costs_prg.subVec(costspos, nout) << outvec;
            costspos += nout+ncosts;
        }
        outpos += nout;
    }

    if(!output_prg.empty())
        output_prg_.run(invec_for_output_prg, output);

    if(!costs_prg.empty())
        costs_prg_.run(invec_for_costs_prg, costs);
}


void VPLCombinedLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                     const Vec& target, Vec& costs) const
{ 
    Vec nonconst_output = output; // to make the constipated compiler happy
    computeOutputAndCosts(input, target, nonconst_output, costs); 
}

TVec<string> VPLCombinedLearner::getOutputNames() const
{
    return outputnames_;
}

TVec<string> VPLCombinedLearner::getTestCostNames() const
{
    return costnames_;
}

TVec<string> VPLCombinedLearner::getTrainCostNames() const
{

    return TVec<string>();
}

void VPLCombinedLearner::resetInternalState()
{
    for(int k=0; k<sublearners_.length(); k++)
        sublearners_[k]->resetInternalState();
}

bool VPLCombinedLearner::isStatefulLearner() const
{
    return sublearners_[0]->isStatefulLearner();
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
