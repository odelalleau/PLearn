// -*- C++ -*-

// PLearner.cc
//
// Copyright (C) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio, Nicolas Chapados, Charles Dugas, Rejean Ducharme, Universite de Montreal
// Copyright (C) 2001,2002 Francis Pieraut, Jean-Sebastien Senecal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Julien Keable
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

#include "PLearner.h"
#include <plearn/base/stringutils.h>
#include <plearn/io/fileutils.h>
#include <plearn/io/pl_log.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/misc/PLearnService.h>
#include <plearn/misc/RemotePLearnServer.h>
// #include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/vmat/PLearnerOutputVMatrix.h>

namespace PLearn {
using namespace std;

PLearner::PLearner()
    : n_train_costs_(-1),
      n_test_costs_(-1),
      seed_(-1), 
      stage(0),
      nstages(1),
      report_progress(true),
      verbosity(1),
      nservers(0),
      save_trainingset_prefix(""),
      inputsize_(-1),
      targetsize_(-1),
      weightsize_(-1),
      n_examples(-1),
      forget_when_training_set_changes(false)  
{}

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    PLearner,
    "The base class for all PLearn learning algorithms",
    "PLearner provides a base class for all learning algorithms within PLearn.\n"
    "It presents an abstraction of learning that centers around a \"train-test\"\n"
    "paradigm:\n"
    "\n"
    "- Phase 1: TRAINING.  In this phase, one must first establish an experiment\n"
    "  directory (usually done by an enclosing PTester) to store any temporary\n"
    "  files that the learner might seek to create.  Then, one sets a training\n"
    "  set VMat (also done by the enclosing PTester), which contains the set of\n"
    "  input-target pairs that the learner should attempt to represent.  Finally\n"
    "  one calls the train() virtual member function to carry out the actual\n"
    "  action of training the model.\n"
    "\n"
    "- Phase 2: TESTING.  In this phase (to be done after training), one\n"
    "  repeatedly calls functions from the computeOutput() family to evaluate\n"
    "  the trained model on new input vectors.\n"
    "\n"
    "Note that the PTester class is the usual \"driver\" for a PLearner (and\n"
    "automatically calls the above functions in the appropriate order), in the\n"
    "usual scenario wherein one wants to evaluate the generalization performance\n"
    "on a dataset.\n"
    );

void PLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(tmp_output, copies);
    // TODO What's wrong with this?
    deepCopyField(train_set, copies);
    deepCopyField(validation_set, copies);
    deepCopyField(train_stats, copies);
    deepCopyField(random_gen, copies);
}

void PLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "expdir", &PLearner::expdir, OptionBase::buildoption, 
                  "Path of the directory associated with this learner, in which\n"
                  "it should save any file it wishes to create. \n"
                  "The directory will be created if it does not already exist.\n"
                  "If expdir is the empty string (the default), then the learner \n"
                  "should not create *any* file. Note that, anyway, most file creation and \n"
                  "reporting are handled at the level of the PTester class rather than \n"
                  "at the learner's. \n");

    declareOption(ol, "seed", &PLearner::seed_, OptionBase::buildoption, 
        "The initial seed for the random number generator used in this\n"
        "learner, for instance for parameter initialization.\n"
        "If -1 is provided, then a 'random' seed is chosen based on time\n"
        "of day, ensuring that different experiments run differently.\n"
        "If 0 is provided, no (re)initialization of the random number\n"
        "generator is performed.\n"
        "With a given positive seed, build() and forget() should always\n"
        "initialize the parameters to the same values.");

    declareOption(ol, "stage", &PLearner::stage, OptionBase::learntoption, 
                  "The current training stage, since last fresh initialization (forget()): \n"
                  "0 means untrained, n often means after n epochs or optimization steps, etc...\n"
                  "The true meaning is learner-dependant."
                  "You should never modify this option directly!"
                  "It is the role of forget() to bring it back to 0,\n"
                  "and the role of train() to bring it up to 'nstages'...");

    declareOption(ol, "n_examples", &PLearner::n_examples, OptionBase::learntoption, 
                  "The number of samples in the training set.\n"
                  "Obtained from training set with setTrainingSet.");

    declareOption(ol, "inputsize", &PLearner::inputsize_, OptionBase::learntoption, 
                  "The number of input columns in the data sets."
                  "Obtained from training set with setTrainingSet.");

    declareOption(ol, "targetsize", &PLearner::targetsize_, OptionBase::learntoption, 
                  "The number of target columns in the data sets."
                  "Obtained from training set with setTrainingSet.");

    declareOption(ol, "weightsize", &PLearner::weightsize_, OptionBase::learntoption, 
                  "The number of cost weight columns in the data sets."
                  "Obtained from training set with setTrainingSet.");

    declareOption(ol, "forget_when_training_set_changes", &PLearner::forget_when_training_set_changes, OptionBase::buildoption, 
                  "Whether or not to call the forget() method (re-initialize model as before training) in setTrainingSet when the\n"
                  "training set changes (e.g. of dimension).");

    declareOption(ol, "nstages", &PLearner::nstages, OptionBase::buildoption, 
                  "The stage until which train() should train this learner and return.\n"
                  "The meaning of 'stage' is learner-dependent, but for learners whose \n"
                  "training is incremental (such as involving incremental optimization), \n"
                  "it is typically synonym with the number of 'epochs', i.e. the number \n"
                  "of passages of the optimization process through the whole training set, \n"
                  "since the last fresh initialisation.");

    declareOption(ol, "report_progress", &PLearner::report_progress, OptionBase::buildoption, 
                  "should progress in learning and testing be reported in a ProgressBar.\n");

    declareOption(ol, "verbosity", &PLearner::verbosity, OptionBase::buildoption, 
                  "Level of verbosity. If 0 should not write anything on perr. \n"
                  "If >0 may write some info on the steps performed along the way.\n"
                  "The level of details written should depend on this value.");

    declareOption(ol, "nservers", &PLearner::nservers, OptionBase::buildoption, 
                  "Max number of computation servers to use in parallel with the main process.\n"
                  "If <=0 no parallelization will occur at this level.\n");

    declareOption(ol, "save_trainingset_prefix", &PLearner::save_trainingset_prefix,
                  OptionBase::buildoption,
                  "Whether the training set should be saved upon a call to\n"
                  "setTrainingSet().  The saved file is put in the learner's expdir\n"
                  "(assuming there is one) and has the form \"<prefix>_trainset_XXX.pmat\"\n"
                  "The prefix is what this option specifies.  'XXX' is a unique\n"
                  "serial number that is globally incremented with each saved\n"
                  "setTrainingSet.  This option is useful when manipulating very\n"
                  "complex nested learner structures, and you want to ensure that\n"
                  "the inner learner is getting the correct results.  (Default="",\n"
                  "i.e. don't save anything.)\n");
  
    inherited::declareOptions(ol);
}

void PLearner::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this
    // different than for declareOptions()
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(
        rmm, "setTrainingSet", &PLearner::setTrainingSet,
        (BodyDoc("Declares the training set.  Then calls build() and forget() if\n"
                 "necessary.\n"),
         ArgDoc ("training_set", "The training set VMatrix to set; should have\n"
                 "its inputsize, targetsize and weightsize fields set properly.\n"),
         ArgDoc ("call_forget", "Whether the forget() function should be called\n"
                 "upon setting the training set\n")));

    declareMethod(
        rmm, "setExperimentDirectory", &PLearner::setExperimentDirectory,
        (BodyDoc("The experiment directory is the directory in which files related to\n"
                 "this model are to be saved.  If it is an empty string, it is understood\n"
                 "to mean that the user doesn't want any file created by this learner.\n"),
         ArgDoc ("expdir", "Experiment directory to set")));

    declareMethod(
        rmm, "getExperimentDirectory", &PLearner::getExperimentDirectory,
        (BodyDoc("This returns the currently set experiment directory\n"
                 "(see setExperimentDirectory)\n"),
         RetDoc ("Current experiment directory")));

    declareMethod(
        rmm, "forget", &PLearner::forget,
        (BodyDoc("(Re-)initializes the PLearner in its fresh state (that state may depend\n"
                 "on the 'seed' option) and sets 'stage' back to 0 (this is the stage of\n"
                 "a fresh learner!)\n"
                 "\n"
                 "A typical forget() method should do the following:\n"
                 "\n"
                 "- call inherited::forget() to initialize the random number generator\n"
                 "  with the 'seed' option\n"
                 "\n"
                 "- initialize the learner's parameters, using this random generator\n"
                 "\n"
                 "- stage = 0;\n"
                 "\n"
                 "This method is typically called by the build_() method, after it has\n"
                 "finished setting up the parameters, and if it deemed useful to set or\n"
                 "reset the learner in its fresh state.  (remember build may be called\n"
                 "after modifying options that do not necessarily require the learner to\n"
                 "restart from a fresh state...)  forget is also called by the\n"
                 "setTrainingSet method, after calling build(), so it will generally be\n"
                 "called TWICE during setTrainingSet!\n")));

    declareMethod(
        rmm, "train", &PLearner::train,
        (BodyDoc("The role of the train method is to bring the learner up to\n"
                 "stage==nstages, updating the stats with training costs measured on-line\n"
                 "in the process.\n")));

    declareMethod(
        rmm, "resetInternalState", &PLearner::resetInternalState,
        (BodyDoc("If the learner is a stateful one (inherits from StatefulLearner),\n"
                 "this resets the internal state to its initial value; by default,\n"
                 "this function does nothing.")));

    declareMethod(
        rmm, "computeOutput", &PLearner::remote_computeOutput,
        (BodyDoc("On a trained learner, this computes the output from the input"),
         ArgDoc ("input", "Input vector (should have width inputsize)"),
         RetDoc ("Computed output (will have width outputsize)")));

    declareMethod(
        rmm, "use", &PLearner::remote_use,
        (BodyDoc("Compute the output of a trained learner on every row of an\n"
                 "input VMatrix.  The outputs are stored in a .pmat matrix\n"
                 "under the specified filename."),
         ArgDoc ("input_vmat", "VMatrix containing the inputs"),
         ArgDoc ("output_pmat_fname", "Name of the .pmat to store the computed outputs")));

    declareMethod(
        rmm, "use2", &PLearner::remote_use2,
        (BodyDoc("Compute the output of a trained learner on every row of an\n"
                 "input VMatrix.  The outputs are returned as a matrix.\n"),
         ArgDoc ("input_vmat", "VMatrix containing the inputs"),
         RetDoc ("Matrix holding the computed outputs")));

    declareMethod(
        rmm, "computeOutputAndCosts", &PLearner::remote_computeOutputAndCosts,
        (BodyDoc("Compute both the output from the input, and the costs associated\n"
                 "with the desired target.  The computed costs\n"
                 "are returned in the order given by getTestCostNames()\n"),
         ArgDoc ("input",  "Input vector (should have width inputsize)"),
         ArgDoc ("target", "Target vector (for cost computation)"),
         RetDoc ("- Vec containing output \n"
                 "- Vec containing cost")));

    declareMethod(
        rmm, "computeCostsFromOutputs", &PLearner::remote_computeCostsFromOutputs,
        (BodyDoc("Compute the costs from already-computed output.  The computed costs\n"
                 "are returned in the order given by getTestCostNames()"),
         ArgDoc ("input",  "Input vector (should have width inputsize)"),
         ArgDoc ("output", "Output vector computed by previous call to computeOutput()"),
         ArgDoc ("target", "Target vector"),
         RetDoc ("The computed costs vector")));

    declareMethod(
        rmm, "computeCostsOnly", &PLearner::remote_computeCostsOnly,
        (BodyDoc("Compute the costs only, without the outputs; for some learners, this\n"
                 "may be more efficient than calling computeOutputAndCosts() if the\n"
                 "outputs are not needed.  (The default implementation simply calls\n"
                 "computeOutputAndCosts() and discards the output.)\n"),
         ArgDoc ("input",  "Input vector (should have width inputsize)"),
         ArgDoc ("target", "Target vector"),
         RetDoc ("The computed costs vector")));

    declareMethod(
        rmm, "computeConfidenceFromOutput", &PLearner::remote_computeConfidenceFromOutput,
        (BodyDoc("Compute a confidence intervals for the output, given the input and the\n"
                 "pre-computed output (resulting from computeOutput or similar).  The\n"
                 "probability level of the confidence interval must be specified.\n"
                 "(e.g. 0.95).  Result is stored in a TVec of pairs low:high for each\n"
                 "output variable (this is a \"box\" interval; it does not account for\n"
                 "correlations among the output variables).\n"),
         ArgDoc ("input",       "Input vector (should have width inputsize)"),
         ArgDoc ("output",      "Output vector computed by previous call to computeOutput()"),
         ArgDoc ("probability", "Level at which the confidence interval must be computed,\n"
                                "e.g. 0.95\n"),
         RetDoc ("Vector of pairs low:high giving, respectively, the lower-bound confidence\n"
                 "and upper-bound confidence for each dimension of the output vector.  If this\n"
                 "vector is empty, then confidence intervals could not be computed for the\n"
                 "given learner.  Note that this is the PLearner default (not to compute\n"
                 "any confidence intervals), but some learners such as LinearRegressor\n"
                 "know how to compute them.")));

    declareMethod(
        rmm, "batchComputeOutputAndConfidencePMat",
        &PLearner::remote_batchComputeOutputAndConfidence,
        (BodyDoc("Repeatedly calls computeOutput and computeConfidenceFromOutput with the\n"
                 "rows of inputs.  Writes outputs_and_confidence rows (as a series of\n"
                 "triples (output, low, high), one for each output).  The results are\n"
                 "stored in a .pmat whose filename is passed as argument.\n"),
         ArgDoc ("input_vmat",  "VMatrix containing the input rows"),
         ArgDoc ("probability", "Level at which the confidence interval must be computed,\n"
                                "e.g. 0.95\n"),
         ArgDoc ("result_pmat_filename", "Filename where to store the results")));

    declareMethod(
        rmm, "getTestCostNames", &PLearner::getTestCostNames,
        (BodyDoc("Return the name of the costs computed by computeCostsFromOutputs()\n"
                 "and computeOutputAndCosts()"),
         RetDoc ("List of test cost names")));

    declareMethod(
        rmm, "getTrainCostNames", &PLearner::getTrainCostNames,
        (BodyDoc("Return the names of the objective costs that the train\n"
                 "method computes and for which it updates the VecStatsCollector\n"
                 "train_stats."),
         RetDoc ("List of train cost names")));
}

////////////////////////////
// setExperimentDirectory //
////////////////////////////
void PLearner::setExperimentDirectory(const PPath& the_expdir) 
{ 
    if(the_expdir=="")
        expdir = "";
    else
    {
        if(!force_mkdir(the_expdir))
            PLERROR("In PLearner::setExperimentDirectory Could not create experiment directory %s",
                    the_expdir.absolute().c_str());
        expdir = the_expdir / "";
    }
}

void PLearner::setTrainingSet(VMat training_set, bool call_forget)
{ 
    // YB: je ne suis pas sur qu'il soit necessaire de faire un build si la
    // LONGUEUR du train_set a change?  les methodes non-parametriques qui
    // utilisent la longueur devrait faire leur "resize" dans train, pas dans
    // build.
    bool training_set_has_changed = !train_set || !(train_set->looksTheSameAs(training_set));
    train_set = training_set;
    if (training_set_has_changed)
    {
        inputsize_ = train_set->inputsize();
        targetsize_ = train_set->targetsize();
        weightsize_ = train_set->weightsize();
        if (forget_when_training_set_changes)
            call_forget=true;
    }
    n_examples = train_set->length();
    if (training_set_has_changed || call_forget)
        build(); // MODIF FAITE PAR YOSHUA: sinon apres un setTrainingSet le build n'est pas complete dans un NNet train_set = training_set;
    if (call_forget)
        forget();

    // Save the new training set if desired
    if (save_trainingset_prefix != "" && expdir != "") {
        static int trainingset_serial = 1;
        PPath fname = expdir / (save_trainingset_prefix + "_trainset_" +
                                tostring(trainingset_serial++) + ".pmat");
        train_set->savePMAT(fname);
    }
}

void PLearner::setValidationSet(VMat validset)
{ validation_set = validset; }


void PLearner::setTrainStatsCollector(PP<VecStatsCollector> statscol)
{ train_stats = statscol; }


int PLearner::inputsize() const
{ 
    if (inputsize_<0)
        PLERROR("Must specify a training set before calling PLearner::inputsize()"); 
    return inputsize_; 
}

int PLearner::targetsize() const 
{ 
    if(targetsize_ == -1) 
        PLERROR("In PLearner::targetsize - 'targetsize_' is -1, either no training set has beeen specified or its sizes were not set properly");
    return targetsize_; 
}

int PLearner::weightsize() const 
{ 
    if(weightsize_ == -1) 
        PLERROR("In PLearner::weightsize - 'weightsize_' is -1, either no training set has beeen specified or its sizes were not set properly");
    return weightsize_; 
}

////////////
// build_ //
////////////
void PLearner::build_()
{
    if(expdir!="")
    {
        if(!force_mkdir(expdir))
            PLWARNING("In PLearner Could not create experiment directory %s",expdir.c_str());
        else
            expdir = expdir.absolute() / "";
    }
    if (random_gen && seed_ != 0)
        random_gen->manual_seed(seed_);
}

///////////
// build //
///////////
void PLearner::build()
{
    inherited::build();
    build_();
}

////////////
// forget //
////////////
void PLearner::forget()
{
    if (random_gen && seed_ != 0)
        random_gen->manual_seed(seed_);
}

int PLearner::nTestCosts() const 
{ 
    if(n_test_costs_<0)
        n_test_costs_ = getTestCostNames().size(); 
    return n_test_costs_;
}

int PLearner::nTrainCosts() const 
{ 
    if(n_train_costs_<0)
        n_train_costs_ = getTrainCostNames().size();
    return n_train_costs_; 
}

int PLearner::getTestCostIndex(const string& costname) const
{
    TVec<string> costnames = getTestCostNames();
    for(int i=0; i<costnames.length(); i++)
        if(costnames[i]==costname)
            return i;
    PLERROR("In PLearner::getTestCostIndex, No test cost named %s in this learner.\n"
            "Available test costs are: %s", costname.c_str(),
            tostring(costnames).c_str());
    return -1;
}

int PLearner::getTrainCostIndex(const string& costname) const
{
    TVec<string> costnames = getTrainCostNames();
    for(int i=0; i<costnames.length(); i++)
        if(costnames[i]==costname)
            return i;
    PLERROR("In PLearner::getTrainCostIndex, No train cost named %s in this learner.\n"
            "Available train costs are: %s", costname.c_str(), tostring(costnames).c_str());
    return -1;
}
                                
void PLearner::computeOutputAndCosts(const Vec& input, const Vec& target, 
                                     Vec& output, Vec& costs) const
{
    computeOutput(input, output);
    computeCostsFromOutputs(input, output, target, costs);
}

void PLearner::computeCostsOnly(const Vec& input, const Vec& target,  
                                Vec& costs) const
{
    tmp_output.resize(outputsize());
    computeOutputAndCosts(input, target, tmp_output, costs);
}

bool PLearner::computeConfidenceFromOutput(
    const Vec& input, const Vec& output,
    real probability,
    TVec< pair<real,real> >& intervals) const
{
    // Default version does not know how to compute confidence intervals
    intervals.resize(output.size());
    intervals.fill(std::make_pair(MISSING_VALUE,MISSING_VALUE));  
    return false;
}

void PLearner::batchComputeOutputAndConfidence(VMat inputs, real probability, VMat outputs_and_confidence) const
{
    Vec input(inputsize());
    Vec output(outputsize());
    int outsize = outputsize();
    Vec output_and_confidence(3*outsize);
    TVec< pair<real,real> > intervals;
    int l = inputs.length();
    for(int i=0; i<l; i++)
    {
        inputs->getRow(i,input);
        computeOutput(input,output);
        computeConfidenceFromOutput(input,output,probability,intervals);
        for(int j=0; j<outsize; j++)
        {
            output_and_confidence[3*j] = output[j];
            output_and_confidence[3*j+1] = intervals[j].first;
            output_and_confidence[3*j+2] = intervals[j].second;
        }
        outputs_and_confidence->putOrAppendRow(i,output_and_confidence);
    }
}

/////////
// use //
/////////
void PLearner::use(VMat testset, VMat outputs) const
{
    int l = testset.length();
    int w = testset.width();

    TVec< PP<RemotePLearnServer> > servers;
    if(nservers>0)
        servers = PLearnService::instance().reserveServers(nservers);

    if(servers.length()==0) 
    { // sequential code      
        Vec input;
        Vec target;
        real weight;
        Vec output(outputsize());

        ProgressBar* pb = NULL;
        if(report_progress)
            pb = new ProgressBar("Using learner",l);

        for(int i=0; i<l; i++)
        {
            testset.getExample(i, input, target, weight);
            computeOutput(input, output);
            outputs->putOrAppendRow(i,output);
            if(pb)
                pb->update(i);
        }

        if(pb)
            delete pb;
    }
    else // parallel code
    {
        int n = servers.length(); // number of allocated servers
        DBG_LOG << "PLearner::use parallel code using " << n << " servers" << endl;
        for(int k=0; k<n; k++)  // send this object with objid 0
            servers[k]->newObject(0, *this);
        int chunksize = l/n;
        if(chunksize*n<l)
            ++chunksize;
        if(chunksize*w>1000000) // max 1 Mega elements
            chunksize = max(1,1000000/w);
        Mat chunk(chunksize,w);
        int send_i=0;
        Mat outmat;
        int receive_i = 0;
        while(send_i<l)
        {
            for(int k=0; k<n && send_i<l; k++)
            {
                int actualchunksize = chunksize;
                if(send_i+actualchunksize>l)
                    actualchunksize = l-send_i;
                chunk.resize(actualchunksize,w);
                testset->getMat(send_i, 0, chunk);
                VMat inputs(chunk);
                inputs->copySizesFrom(testset);
                DBG_LOG << "PLearner::use calling use2 remote method with chunk starting at " 
                        << send_i << " of length " << actualchunksize << ":" << inputs << endl;
                servers[k]->callMethod(0,"use2",inputs);
                send_i += actualchunksize;
            }
            for(int k=0; k<n && receive_i<l; k++)
            {
                outmat.resize(0,0);
                servers[k]->getResults(outmat);
                for(int ii=0; ii<outmat.length(); ii++)
                    outputs->putOrAppendRow(receive_i++,outmat(ii));
            }
        }
        if(send_i!=l || receive_i!=l)
            PLERROR("In PLearn::use parallel execution failed to complete successfully.");
    }
}

VMat PLearner::processDataSet(VMat dataset) const
{
    // new code using PLearnerOutputVMatrix
    return new PLearnerOutputVMatrix(dataset, this);

    /* Old code for direct in-memory computation

    Vec input(dataset->inputsize());
    int nout = outputsize();
    int targetsize = dataset->targetsize();
    int weightsize = dataset->weightsize();
    int extrasize  = dataset->extrasize(); 
    Vec processed_row(nout+targetsize+weightsize+extrasize);
    Vec output = processed_row.subVec(0,nout);
    Vec target = processed_row.subVec(nout,targetsize);
    Vec weight = processed_row.subVec(nout+targetsize,weightsize);
    Vec extra  = processed_row.subVec(nout+targetsize+weightsize,extrasize);

    real w;
    int l = dataset.length();
    
    // For now we cache this in memory. A future implementation may offer 
    // other options, such as caching in file, or on-the-fly computation
    // vmatrix.
    Mat processed_data(l,nout+targetsize+weightsize+extrasize);

    for(int i=0; i<l; i++)
    {
        dataset->getExample(i, input, target, w);
        if(weightsize==1)
            weight[0] = w;
        dataset->getExtra(i,extra);
        computeOutput(input, output);
        processed_data(i) << processed_row;
    }

    VMat processed_vmat = new MemoryVMatrix(processed_data);
    processed_vmat->defineSizes(nout, targetsize, weightsize, extrasize);
    TVec<string> fieldnames = concat(getOutputNames(),  
                                     dataset->targetFieldNames(),
                                     dataset->weightFieldNames(),
                                     dataset->extraFieldNames() );
    processed_vmat->declareFieldNames(fieldnames);
    return processed_vmat;
    */

}


TVec<string> PLearner::getOutputNames() const
{
    int n = outputsize();
    TVec<string> outnames(n);
    char tmp[21];
    tmp[20] = '\0';
    for(int k=0; k<n; k++)
    {
        snprintf(tmp,20,"out%d",k);
        outnames[k] = tmp;
    }
    return outnames;
}

////////////////
// useOnTrain //
////////////////
void PLearner::useOnTrain(Mat& outputs) const {
    // NC declares this method to be tested...
    // PLWARNING("In PLearner::useOnTrain - This method has not been tested yet, remove this warning if it works fine");
    VMat train_output(outputs);
    use(train_set, train_output);
}

//////////
// test //
//////////
void PLearner::test(VMat testset, PP<VecStatsCollector> test_stats, 
                    VMat testoutputs, VMat testcosts) const
{
    int l = testset.length();
    Vec input;
    Vec target;
    real weight;

    Vec output(outputsize());

    Vec costs(nTestCosts());

    // testset->defineSizes(inputsize(),targetsize(),weightsize());

    ProgressBar* pb = NULL;
    if(report_progress) 
        pb = new ProgressBar("Testing learner",l);

    if (l == 0) {
        // Empty test set: we give -1 cost arbitrarily.
        costs.fill(-1);
        test_stats->update(costs);
    }

    /*
    perr << "PLearner::test class=" << this->classname()
         << "\tl=" << l 
         << "\tinputsize=" << testset->inputsize() 
         << "\ttargetsize=" << testset->targetsize() 
         << "\tweightsize=" << testset->weightsize() 
         << endl;
    */

    for(int i=0; i<l; i++)
    {
        testset.getExample(i, input, target, weight);
      
        // Always call computeOutputAndCosts, since this is better
        // behaved with stateful learners
        computeOutputAndCosts(input,target,output,costs);
      
        if(testoutputs)
            testoutputs->putOrAppendRow(i,output);

        if(testcosts)
            testcosts->putOrAppendRow(i, costs);

        if(test_stats)
            test_stats->update(costs,weight);

        if(report_progress)
            pb->update(i);
    }

    if(pb)
        delete pb;

}

///////////////
// initTrain //
///////////////
bool PLearner::initTrain()
{
    string warn_msg = "In PLearner::trainingCheck (called by '" +
        this->classname() + "') - ";
    
    // Check 'nstages' is valid.
    if (nstages < 0) {
        PLWARNING((warn_msg + "Option nstages (set to " + tostring(nstages)
                    + ") must be non-negative").c_str());
        return false;
    }

    // Check we actually need to train.
    if (stage == nstages) {
        if (verbosity >= 1)
            PLWARNING((warn_msg + "The learner is already trained").c_str());
        return false;
    }

    if (stage > nstages) {
        if (verbosity >= 1) {
            string msg = warn_msg + "Learner was already trained up to stage "
                + tostring(stage) + ", but asked to train up to nstages="
                + tostring(nstages) + ": it will be reverted to stage 0 and "
                                      "trained again";
            PLWARNING(msg.c_str());
        }
        forget();
    }

    // Check there is a training set.
    if (!train_set) {
        if (verbosity >= 1)
            PLWARNING((warn_msg + "No training set specified").c_str());
        return false;
    }

    // Initialize train_stats if needed.
    if (!train_stats)
        train_stats = new VecStatsCollector();

    // Everything is fine.
    return true;
}

////////////////////////
// resetInternalState //
////////////////////////
void PLearner::resetInternalState()
{ }

bool PLearner::isStatefulLearner() const
{
    return false;
}

//! Version of computeOutput that returns a result by value
Vec PLearner::remote_computeOutput(const Vec& input) const
{
    tmp_output.resize(outputsize());
    computeOutput(input, tmp_output);
    return tmp_output;
}

//! Version of use that's called by RMI
void PLearner::remote_use(VMat inputs, string output_fname) const
{
    VMat outputs = new FileVMatrix(output_fname, inputs.length(), outputsize());
    use(inputs,outputs);
}

//! Version of use2 that's called by RMI
Mat PLearner::remote_use2(VMat inputs) const
{
    Mat outputs(inputs.length(), outputsize());
    use(inputs,outputs);
    return outputs;
}
    
//! Version of computeOutputAndCosts that's called by RMI

tuple<Vec,Vec> PLearner::remote_computeOutputAndCosts(const Vec& input, const Vec& target) const
{
    tmp_output.resize(outputsize());
    Vec costs(nTestCosts());
    computeOutputAndCosts(input,target,tmp_output,costs);
    return make_tuple(tmp_output, costs);
}

//! Version of computeCostsFromOutputs that's called by RMI
Vec PLearner::remote_computeCostsFromOutputs(const Vec& input, const Vec& output,
                                             const Vec& target) const
{
    Vec costs(nTestCosts());
    computeCostsFromOutputs(input,output,target,costs);
    return costs;
}

//! Version of computeCostsOnly that's called by RMI
Vec PLearner::remote_computeCostsOnly(const Vec& input, const Vec& target) const
{
    Vec costs(nTestCosts());
    computeCostsOnly(input,target,costs);
    return costs;
}

//! Version of computeConfidenceFromOutput that's called by RMI
TVec< pair<real,real> >
PLearner::remote_computeConfidenceFromOutput(const Vec& input, const Vec& output,
                                             real probability) const
{
    TVec< pair<real,real> > intervals(output.length());
    bool ok = computeConfidenceFromOutput(input, output, probability, intervals);
    if (ok)
        return intervals;
    else
        return TVec< pair<real,real> >();
}

//! Version of batchComputeOutputAndConfidence that's called by RMI
void PLearner::remote_batchComputeOutputAndConfidence(VMat inputs, real probability,
                                                      string pmat_fname) const
{
    TVec<string> fieldnames;
    for(int j=0; j<outputsize(); j++)
    {
        fieldnames.append("output_"+tostring(j));
        fieldnames.append("low_"+tostring(j));
        fieldnames.append("high_"+tostring(j));
    }
    VMat out_and_conf = new FileVMatrix(pmat_fname,inputs.length(),fieldnames);
    batchComputeOutputAndConfidence(inputs, probability, out_and_conf);
}

/**
 *  void PLearner::call(const string& methodname, int nargs, PStream& io)
 *  {
 *      if(methodname=="setTrainingSet")
 *      {
 *          if(nargs!=2) PLERROR("PLearner remote method setTrainingSet takes 2 argument");
 *          VMat training_set;
 *          bool call_forget;
 *          io >> training_set >> call_forget;
 *          setTrainingSet(training_set, call_forget);
 *          prepareToSendResults(io, 0);
 *          io.flush();
 *      }
 *      else if(methodname=="setExperimentDirectory")
 *      {
 *          if(nargs!=1) PLERROR("PLearner remote method setExperimentDirectory takes 1 argument");
 *          PPath the_expdir;
 *          io >> the_expdir;
 *          setExperimentDirectory(the_expdir);
 *          prepareToSendResults(io, 0);
 *          io.flush();      
 *      }
 *      else if(methodname=="getExperimentDirectory")
 *      {
 *          if(nargs!=0) PLERROR("PLearner remote method getExperimentDirectory takes 0 arguments");
 *          PPath result = getExperimentDirectory();
 *          prepareToSendResults(io, 1);
 *          io << result;
 *          io.flush();      
 *      }
 *      else if(methodname=="forget")
 *      {
 *          if(nargs!=0) PLERROR("PLearner remote method forget takes 0 arguments");
 *          forget();
 *          prepareToSendResults(io, 0);
 *          io.flush();      
 *      }
 *      else if(methodname=="train")
 *      {
 *          if(nargs!=0) PLERROR("PLearner remote method train takes 0 arguments");
 *          train();
 *          prepareToSendResults(io, 0);
 *          io.flush();      
 *      }
 *      else if(methodname=="resetInternalState")
 *      {
 *          if(nargs!=0) PLERROR("PLearner remote method resetInternalState takes 0 arguments");
 *          resetInternalState();
 *          prepareToSendResults(io, 0);
 *          io.flush();
 *      }
 *      else if(methodname=="computeOutput")
 *      {
 *          if(nargs!=1) PLERROR("PLearner remote method computeOutput takes 1 argument");
 *          Vec input;
 *          io >> input;
 *          tmp_output.resize(outputsize());
 *          computeOutput(input,tmp_output);
 *          prepareToSendResults(io, 1);
 *          io << tmp_output;
 *          io.flush();    
 *      }
 *      else if(methodname=="use") // use inputs_vmat output_pmat_fname --> void
 *      {
 *          if(nargs!=2) PLERROR("PLearner remote method use requires 2 argument");
 *          VMat inputs;
 *          string output_fname;
 *          io >> inputs >> output_fname;
 *          VMat outputs = new FileVMatrix(output_fname, inputs.length(), outputsize());
 *          use(inputs,outputs);
 *          prepareToSendResults(io, 0);
 *          io.flush();      
 *      }
 *      else if(methodname=="use2") // use inputs_vmat --> outputs
 *      {
 *          if(nargs!=1) PLERROR("PLearner remote method use2 requires 1 argument");
 *          VMat inputs;
 *          io >> inputs;
 *          // DBG_LOG << " Arg0 = " << inputs << endl;
 *          Mat outputs(inputs.length(),outputsize());
 *          use(inputs,outputs);
 *          prepareToSendResults(io, 1);
 *          io << outputs;
 *          io.flush();      
 *      }
 *      else if(methodname=="computeOutputAndCosts")
 *      {
 *          if(nargs!=2) PLERROR("PLearner remote method computeOutputAndCosts takes 2 arguments");
 *          Vec input, target;
 *          io >> input >> target;
 *          tmp_output.resize(outputsize());
 *          Vec costs(nTestCosts());
 *          computeOutputAndCosts(input,target,tmp_output,costs);
 *          prepareToSendResults(io, 2);
 *          io << tmp_output << costs;
 *          io.flush();
 *      }
 *      else if(methodname=="computeCostsFromOutputs")
 *      {
 *          if(nargs!=3) PLERROR("PLearner remote method computeCostsFromOutputs takes 3 arguments");
 *          Vec input, output, target;
 *          io >> input >> output >> target;
 *          Vec costs;
 *          computeCostsFromOutputs(input,output,target,costs);
 *          prepareToSendResults(io, 1);
 *          io << costs;
 *          io.flush();
 *      }
 *      else if(methodname=="computeCostsOnly")
 *      {
 *          if(nargs!=3) PLERROR("PLearner remote method computeCostsOnly takes 3 arguments");
 *          Vec input, target;
 *          io >> input >> target;
 *          Vec costs(nTestCosts());
 *          computeCostsOnly(input,target,costs);
 *          prepareToSendResults(io, 1);
 *          io << costs;
 *          io.flush();
 *      }
 *      else if(methodname=="computeConfidenceFromOutput")
 *      {
 *          if(nargs!=3) PLERROR("PLearner remote method computeConfidenceFromOutput takes 3 arguments: input, output, probability");
 *          Vec input, output;
 *          real probability;
 *          io >> input >> output >> probability;
 *        
 *          TVec< pair<real,real> > intervals(output.length());
 *          bool ok = computeConfidenceFromOutput(input, output, probability, intervals);
 *          prepareToSendResults(io, 2);
 *          io << ok << intervals;
 *          io.flush();
 *      }
 *      else if(methodname=="batchComputeOutputAndConfidencePMat") // input_vmat probability result_pmat_filename
 *      {
 *          if(nargs!=3) 
 *              PLERROR("PLearner remote method batchComputeOutputAndConfidencePMat takes 3 arguments:\n"
 *                      "input_vmat, probability, result_pmat_filename");
 *          VMat inputs;
 *          real probability;
 *          string pmat_fname;
 *          io >> inputs >> probability >> pmat_fname;
 *          TVec<string> fieldnames;
 *          for(int j=0; j<outputsize(); j++)
 *          {
 *              fieldnames.append("output_"+tostring(j));
 *              fieldnames.append("low_"+tostring(j));
 *              fieldnames.append("high_"+tostring(j));
 *          }
 *          VMat out_and_conf = new FileVMatrix(pmat_fname,inputs.length(),fieldnames);
 *          batchComputeOutputAndConfidence(inputs, probability, out_and_conf);
 *          prepareToSendResults(io,0);
 *          io.flush();
 *      }  
 *      else if(methodname=="getTestCostNames")
 *      {
 *          if(nargs!=0) PLERROR("PLearner remote method getTestCostNames takes 0 arguments");
 *          TVec<string> result = getTestCostNames();
 *          prepareToSendResults(io, 1);
 *          io << result;
 *          io.flush();     
 *      }
 *      else if(methodname=="getTrainCostNames")
 *      {
 *          if(nargs!=0) PLERROR("PLearner remote method getTrainCostNames takes 0 arguments");
 *          TVec<string> result = getTrainCostNames();
 *          prepareToSendResults(io, 1);
 *          io << result;
 *          io.flush();     
 *      }
 *      else
 *          inherited::call(methodname, nargs, io);
 *  }
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
