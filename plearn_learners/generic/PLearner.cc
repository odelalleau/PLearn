// -*- C++ -*-

// PLearner.cc
//
// Copyright (C) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio, Nicolas Chapados, Charles Dugas, Rejean Ducharme, Universite de Montreal
// Copyright (C) 2001,2002 Francis Pieraut, Jean-Sebastien Senecal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Julien Keable
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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
#include <plearn/math/pl_erf.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/vmat/RowsSubVMatrix.h>
#include <plearn/misc/PLearnService.h>
#include <plearn/misc/RemotePLearnServer.h>
#include <plearn/vmat/PLearnerOutputVMatrix.h>
#include <plearn/base/RemoteDeclareMethod.h>

namespace PLearn {
using namespace std;

PLearner::PLearner()
    : n_train_costs_(-1),
      n_test_costs_(-1),
      seed_(1827),                           //!< R.I.P. L.v.B.
      stage(0),
      nstages(1),
      report_progress(true),
      verbosity(1),
      nservers(0),
      test_minibatch_size(1),
      save_trainingset_prefix(""),
      parallelize_here(true),
      master_sends_testset_rows(false),
      use_a_separate_random_generator_for_testing(1827),
      finalized(false),
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

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(tmp_output,       copies);
    deepCopyField(train_set,        copies);
    deepCopyField(validation_set,   copies);
    deepCopyField(train_stats,      copies);
    deepCopyField(random_gen,       copies);
    deepCopyField(b_inputs,         copies);
    deepCopyField(b_targets,        copies);
    deepCopyField(b_outputs,        copies);
    deepCopyField(b_costs,          copies);
    deepCopyField(b_weights,        copies);
}

////////////////////
// declareOptions //
////////////////////
void PLearner::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "expdir", &PLearner::expdir, OptionBase::buildoption | OptionBase::nosave | OptionBase::remotetransmit, 
        "Path of the directory associated with this learner, in which\n"
        "it should save any file it wishes to create. \n"
        "The directory will be created if it does not already exist.\n"
        "If expdir is the empty string (the default), then the learner \n"
        "should not create *any* file. Note that, anyway, most file creation and \n"
        "reporting are handled at the level of the PTester class rather than \n"
        "at the learner's. \n");

    declareOption(
        ol, "random_gen", &PLearner::random_gen, OptionBase::learntoption, 
        "The random number generator used in this learner. Constructed from the seed.\n");

    declareOption(
        ol, "seed", &PLearner::seed_, OptionBase::buildoption, 
        "The initial seed for the random number generator used in this\n"
        "learner, for instance for parameter initialization.\n"
        "If -1 is provided, then a 'random' seed is chosen based on time\n"
        "of day, ensuring that different experiments run differently.\n"
        "If 0 is provided, no (re)initialization of the random number\n"
        "generator is performed.\n"
        "With a given positive seed, build() and forget() should always\n"
        "initialize the parameters to the same values.");

    declareOption(
        ol, "stage", &PLearner::stage, OptionBase::learntoption, 
        "The current training stage, since last fresh initialization (forget()): \n"
        "0 means untrained, n often means after n epochs or optimization steps, etc...\n"
        "The true meaning is learner-dependant."
        "You should never modify this option directly!"
        "It is the role of forget() to bring it back to 0,\n"
        "and the role of train() to bring it up to 'nstages'...");

    declareOption(
        ol, "n_examples", &PLearner::n_examples, OptionBase::learntoption, 
        "The number of samples in the training set.\n"
        "Obtained from training set with setTrainingSet.");

    declareOption(
        ol, "inputsize", &PLearner::inputsize_, OptionBase::learntoption, 
        "The number of input columns in the data sets."
        "Obtained from training set with setTrainingSet.");

    declareOption(
        ol, "targetsize", &PLearner::targetsize_, OptionBase::learntoption, 
        "The number of target columns in the data sets."
        "Obtained from training set with setTrainingSet.");

    declareOption(
        ol, "weightsize", &PLearner::weightsize_, OptionBase::learntoption, 
        "The number of cost weight columns in the data sets."
        "Obtained from training set with setTrainingSet.");

    declareOption(
        ol, "forget_when_training_set_changes",
        &PLearner::forget_when_training_set_changes, OptionBase::buildoption, 
        "Whether or not to call the forget() method (re-initialize model \n"
        "as before training) in setTrainingSet when the\n"
        "training set changes (e.g. of dimension).");

    declareOption(
        ol, "nstages", &PLearner::nstages, OptionBase::buildoption, 
        "The stage until which train() should train this learner and return.\n"
        "The meaning of 'stage' is learner-dependent, but for learners whose \n"
        "training is incremental (such as involving incremental optimization), \n"
        "it is typically synonym with the number of 'epochs', i.e. the number \n"
        "of passages of the optimization process through the whole training set, \n"
        "since the last fresh initialisation.");

    declareOption(
        ol, "report_progress", &PLearner::report_progress, OptionBase::buildoption, 
        "should progress in learning and testing be reported in a ProgressBar.\n");

    declareOption(
        ol, "verbosity", &PLearner::verbosity, OptionBase::buildoption, 
        "Level of verbosity. If 0 should not write anything on perr. \n"
        "If >0 may write some info on the steps performed along the way.\n"
        "The level of details written should depend on this value.");

    declareOption(
        ol, "nservers", &PLearner::nservers, OptionBase::buildoption, 
        "DEPRECATED: use parallelize_here instead.\n"
        "Max number of computation servers to use in parallel with the main process.\n"
        "If <=0 no parallelization will occur at this level.\n",
        OptionBase::deprecated_level);

    declareOption(
        ol, "save_trainingset_prefix", &PLearner::save_trainingset_prefix,
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

    declareOption(
        ol, "parallelize_here", &PLearner::parallelize_here, 
        OptionBase::buildoption | OptionBase::nosave,
        "Reserve remote servers at this level if true.\n");

    declareOption(
        ol, "master_sends_testset_rows", &PLearner::master_sends_testset_rows, 
        OptionBase::buildoption | OptionBase::nosave,
        "For parallel PLearner::test : wether the master should read the testset and\n"
        "send rows to the slaves, or send a serialized description of the testset.\n");
  
    declareOption(
        ol, "test_minibatch_size", &PLearner::test_minibatch_size,
        OptionBase::buildoption,
        "Size of minibatches used during testing to take advantage\n"
        "of efficient (possibly parallelized) implementations when\n"
        "multiple examples are processed at once. \n");

    declareOption(
        ol, "use_a_separate_random_generator_for_testing", 
        &PLearner::use_a_separate_random_generator_for_testing,
        OptionBase::buildoption,
        "This option allows to perform testing always in the same\n"
        "conditions in terms of the random generator (if testing involves\n"
        "some non-deterministic component, this can be useful in order\n"
        "to obtain repeatable test results).\n"
        "If non-zero, the base class test() method will use a different\n"
        "random generator than the rest of the code (i.e. training).\n"
        "The non-zero value is the seed to be used during testing.\n"
        "A value of -1 sets the seed differently each time depending on clock.\n"
        "(which is probably not desired here).\n"
        "Note that this option might not be taken into account in some\n"
        "sub-classes that override the PLearner's test method.");

    declareOption(
        ol, "finalized", &PLearner::finalized,
        OptionBase::learntoption,
        "(default false)"
        " After training(when finalized() is called) it will be set to true.\n"
        " When true, it mean the learner it won't be trained again and this\n"
        " allow some optimization.\n");

    inherited::declareOptions(ol);
}

////////////////////
// declareMethods //
////////////////////
void PLearner::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this is different from
    // declareOptions().
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
        rmm, "getTrainingSet", &PLearner::getTrainingSet,
        (BodyDoc("Returns the current training set."),
         RetDoc ("The trainset")));

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
        rmm, "outputsize", &PLearner::outputsize,
        (BodyDoc("Return the learner outputsize")));
    
    declareMethod(
        rmm, "setTrainStatsCollector", &PLearner::setTrainStatsCollector,
        (BodyDoc("Sets the statistics collector whose update() method will be called\n"
                 "during training.\n."),
         ArgDoc ("statscol", "The tatistics collector to set")));

    declareMethod(
        rmm, "getTrainStatsCollector", &PLearner::getTrainStatsCollector,
        (BodyDoc("Returns the statistics collector that was used during training.\n"),
         RetDoc ("Current training statistics collector")));

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
        rmm, "sub_test", &PLearner::sub_test,
        (BodyDoc("Test on a given (chunk of a) testset and return stats, outputs and costs.  "
                 "Used by parallel test"),
         ArgDoc("testset","test set"),
         ArgDoc("test_stats","VecStatsCollector to use"),
         ArgDoc("rtestoutputs","wether to return outputs"),
         ArgDoc("rtestcosts","wether to return costs"),
         RetDoc ("tuple of (stats, outputs, costs)")));

    declareMethod(
        rmm, "test", &PLearner::remote_test,
        (BodyDoc("Test on a given testset and return stats, outputs and costs."),
         ArgDoc("testset","test set"),
         ArgDoc("test_stats","VecStatsCollector to use"),
         ArgDoc("rtestoutputs","whether to return outputs"),
         ArgDoc("rtestcosts","whether to return costs"),
         RetDoc ("tuple of (stats, outputs, costs)")));


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
        rmm, "computeOutputs", &PLearner::remote_computeOutputs,
        (BodyDoc("On a trained learner, this computes the output from the input, one\n"
                 "batch of examples at a time (one example per row of the arg. matrices.\n"),
         ArgDoc ("inputs", "Input matrix (batch_size x inputsize)"),
         RetDoc ("Resulting output matrix (batch_size x outputsize)")));

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
        rmm, "useOnTrain", &PLearner::remote_useOnTrain,
        (BodyDoc("Compute the output of a trained learner on every row of \n"
                 "the trainset.  The outputs are returned as a matrix.\n"),
         RetDoc ("Matrix holding the computed outputs")));

    declareMethod(
        rmm, "computeInputOutputMat", &PLearner::computeInputOutputMat,
        (BodyDoc("Returns a matrix which is a (horizontal) concatenation\n"
                 "and the computed outputs.\n"),
         ArgDoc ("inputs", "VMatrix containing the inputs"),
         RetDoc ("Matrix holding the inputs+computed_outputs")));

    declareMethod(
        rmm, "computeInputOutputConfMat", &PLearner::computeInputOutputConfMat,
        (BodyDoc("Return a Mat that is the contatenation of inputs, outputs, lower\n"
                 "confidence bound, and upper confidence bound.  If confidence intervals\n"
                 "cannot be computed for the learner, they are filled with MISSING_VALUE.\n"),
         ArgDoc ("inputs", "VMatrix containing the inputs"),
         ArgDoc ("probability", "Level at which the confidence intervals should be computed, "
                                "e.g. 0.95."),
         RetDoc ("Matrix holding the inputs+outputs+confidence-low+confidence-high")));

    declareMethod(
        rmm, "computeOutputConfMat", &PLearner::computeOutputConfMat,
        (BodyDoc("Return a Mat that is the contatenation of outputs, lower confidence\n"
                 "bound, and upper confidence bound.  If confidence intervals cannot be\n"
                 "computed for the learner, they are filled with MISSING_VALUE.\n"),
         ArgDoc ("inputs", "VMatrix containing the inputs"),
         ArgDoc ("probability", "Level at which the confidence intervals should be computed, "
                                "e.g. 0.95."),
         RetDoc ("Matrix holding the outputs+confidence-low+confidence-high")));

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
        rmm, "computeOutputsAndCosts", &PLearner::remote_computeOutputsAndCosts,
        (BodyDoc("Compute both the output from the input, and the costs associated\n"
                 "with the desired target.  The computed costs\n"
                 "are returned in the order given by getTestCostNames()\n"
                 "This variant computes the outputs and the costs simultaneously\n"
                 "for a whole batch of examples (rows of the argument matrices)\n"),
         ArgDoc ("inputs", "Input matrix (batch_size x inputsize)"),
         ArgDoc ("targets", "Target matrix (batch_size x targetsize)"),
         RetDoc ("Pair containing first the resulting output matrix\n"
                 "(batch_size x outputsize), then the costs matrix\n"
                 "(batch_size x costsize)")));

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
        rmm, "computeOutputCovMat", &PLearner::remote_computeOutputCovMat,
        (BodyDoc("Version of computeOutput that is capable of returning an output matrix\n"
                 "given an input matrix (set of output vectors), as well as the complete\n"
                 "covariance matrix between the outputs.\n"
                 "\n"
                 "A separate covariance matrix is returned for each output dimension, but\n"
                 "these matrices are allowed to share the same storage.  This would be\n"
                 "the case in situations where the output covariance really depends only\n"
                 "on the location of the training inputs, as in, e.g.,\n"
                 "GaussianProcessRegressor.\n"
                 "\n"
                 "The default implementation is to repeatedly call computeOutput,\n"
                 "followed by computeConfidenceFromOutput (sampled with probability\n"
                 "Erf[1/(2*Sqrt(2))], to extract 1*stddev given by subtraction of the two\n"
                 "intervals, then squaring the stddev to obtain the variance), thereby\n"
                 "filling a diagonal output covariance matrix.  If\n"
                 "computeConfidenceFromOutput returns 'false' (confidence intervals not\n"
                 "supported), the returned covariance matrix is filled with\n"
                 "MISSING_VALUE.\n"),
         ArgDoc ("inputs", "Matrix containing the set of test points"),
         RetDoc ("Two quantities are returned:\n"
                 "- The matrix containing the expected output (as rows) for each input row.\n"
                 "- A vector of covariance matrices between the outputs (one covariance\n"
                 "  matrix per output dimension).\n")));
    
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
{
    train_stats = statscol;
    train_stats->setFieldNames(getTrainCostNames());
}


int PLearner::inputsize() const
{ 
    if (inputsize_<0)
        PLERROR("Must specify a training set before calling PLearner::inputsize()"
                " (or use a training set with a valid inputsize)"); 
    return inputsize_; 
}

int PLearner::targetsize() const 
{ 
    if(targetsize_ == -1) 
        PLERROR("In PLearner::targetsize (%s)- 'targetsize_' is -1,"
                " either no training set has beeen specified or its sizes"
                " were not set properly", this->classname().c_str());
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
    stage = 0;
    finalized=false;
}

//////////////
// finalize //
//////////////
void PLearner::finalize()
{
    finalized=true;
}

////////////////
// nTestCosts //
////////////////
int PLearner::nTestCosts() const 
{ 
    if(n_test_costs_<0)
        n_test_costs_ = getTestCostNames().size(); 
    return n_test_costs_;
}

/////////////////
// nTrainCosts //
/////////////////
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

void PLearner::computeOutputCovMat(const Mat& inputs, Mat& outputs,
                                   TVec<Mat>& covariance_matrices) const
{
    PLASSERT( inputs.width() == inputsize() && outputsize() > 0 );
    const int N = inputs.length();
    const int M = outputsize();
    outputs.resize(N, M);
    covariance_matrices.resize(M);

    bool has_confidence  = true;
    bool init_covariance = 0;
    Vec cur_input, cur_output;
    TVec< pair<real,real> > intervals;
    for (int i=0 ; i<N ; ++i) {
        cur_input  = inputs(i);
        cur_output = outputs(i);
        computeOutput(cur_input, cur_output);
        if (has_confidence) {
            static const real probability = pl_erf(1. / (2*sqrt(2.0)));
            has_confidence = computeConfidenceFromOutput(cur_input, cur_output,
                                                         probability, intervals);
            if (has_confidence) {
                // Create the covariance matrices only once; filled with zeros
                if (! init_covariance) {
                    for (int j=0 ; j<M ; ++j)
                        covariance_matrices[j] = Mat(N, N, 0.0);
                    init_covariance = true;
                }
                
                // Compute the variance for each output j, and set it on
                // element i,i of the j-th covariance matrix
                for (int j=0 ; j<M ; ++j) {
                    float stddev = intervals[j].second - intervals[j].first;
                    float var = stddev*stddev;
                    covariance_matrices[j](i,i) = var;
                }
            }
        }
    }

    // If confidence intervals are not supported, fill the covariance matrices
    // with missing values
    for (int j=0 ; j<M ; ++j)
        covariance_matrices[j] = Mat(N, N, MISSING_VALUE);
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

        PP<ProgressBar> pb;
        if(report_progress)
            pb = new ProgressBar("Using learner",l);

        if (test_minibatch_size==1)
        {
            for(int i=0; i<l; i++)
            {
                testset.getExample(i, input, target, weight);
                computeOutput(input, output);
                outputs->putOrAppendRow(i,output);
                if(pb)
                    pb->update(i);
            }
        } else
        {
            int out_size = outputsize() >= 0 ? outputsize() : 0;
            int n_batches = l/test_minibatch_size, i=0;
            b_inputs.resize(test_minibatch_size,inputsize());
            b_outputs.resize(test_minibatch_size, out_size);
            b_costs.resize(test_minibatch_size,nTestCosts());
            b_targets.resize(test_minibatch_size,targetsize());
            b_weights.resize(test_minibatch_size);
            for (int b=0;b<n_batches;b++,i+=test_minibatch_size)
            {
                testset->getExamples(i,test_minibatch_size,b_inputs,b_targets,b_weights);
                computeOutputs(b_inputs,b_outputs);
                for (int j=0;j<test_minibatch_size;j++)
                {
                    outputs->putOrAppendRow(i+j, b_outputs(j));
                }
                if (pb) pb->update(i+test_minibatch_size);
            }
            if (i<l)
            {
                b_inputs.resize(l-i,inputsize());
                b_outputs.resize(l-i, out_size);
                b_costs.resize(l-i,nTestCosts());
                b_targets.resize(l-i,targetsize());
                b_weights.resize(l-i);
                testset->getExamples(i,l-i,b_inputs,b_targets,b_weights);
                computeOutputs(b_inputs,b_outputs);
                for (int j=0;j<l-i;j++)
                {
                    outputs->putOrAppendRow(i+j, b_outputs(j));
                }
                if (pb) pb->update(l);
            }
        }


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
    // PLearnerOutputVMatrix does exactly this.
    return new PLearnerOutputVMatrix(dataset, this);
}


TVec<string> PLearner::getOutputNames() const
{
    int n = outputsize();
    TVec<string> outnames(n);
    for(int k=0; k<n; k++)
        outnames[k] = "out" + tostring(k);
    return outnames;
}

////////////////
// useOnTrain //
////////////////
void PLearner::useOnTrain(Mat& outputs) const {
    outputs.resize(train_set.length(), outputsize());
    VMat train_output(outputs);
    use(train_set, train_output);
}

Mat PLearner::remote_useOnTrain() const 
{
    Mat outputs;
    useOnTrain(outputs);
    return outputs;
}

//////////
// test //
//////////
void PLearner::test(VMat testset, PP<VecStatsCollector> test_stats,
                    VMat testoutputs, VMat testcosts) const
{

    Profiler::pl_profile_start("PLearner::test");

    int len = testset.length();
    Vec input;
    Vec target;
    real weight;
    int out_size = outputsize() >= 0 ? outputsize() : 0;

    Vec output(out_size);
    Vec costs(nTestCosts());

    if (test_stats) {
        // Set names of test_stats costs
        test_stats->setFieldNames(getTestCostNames());

        if (len == 0) {
            // Empty test set: we give -1 cost arbitrarily.
            costs.fill(-1);
            test_stats->update(costs);
        }
    }

    PP<ProgressBar> pb;
    if (report_progress)
        pb = new ProgressBar("Testing learner", len);

    PP<PRandom> copy_random_gen=0;
    if (use_a_separate_random_generator_for_testing && random_gen)
    {
        CopiesMap copies;
        copy_random_gen = random_gen->deepCopy(copies);
        random_gen->manual_seed(use_a_separate_random_generator_for_testing);
    }

    PLearnService& service(PLearnService::instance());

    //DUMMY: need to find a better way to calc. nservers -xsm
    const int chunksize= 2500;//nb. rows in each chunk sent to a remote server
    const int chunks_per_server= 3;//ideal nb. chunks per server
    int nservers= min(len/(chunks_per_server*chunksize), service.availableServers());

    if(nservers > 1 && parallelize_here && !isStatefulLearner())
    {// parallel test
        CopiesMap copies;
        PP<VecStatsCollector> template_vsc= test_stats? test_stats->deepCopy(copies) : 0;
        TVec<PP<RemotePLearnServer> > servers= service.reserveServers(nservers);
        nservers= servers.length();
        int curpos= 0;
        int chunks_called= 0;
        int last_chunknum= -1;
        map<PP<RemotePLearnServer>, int> learners_ids;
        map<PP<RemotePLearnServer>, int> chunknums;
        map<int, PP<VecStatsCollector> > vscs;
        map<PP<RemotePLearnServer>, int> chunkszs;
        int rowsdone= 0;

        bool rep_prog= report_progress;
        const_cast<bool&>(report_progress)= false;//servers dont report progress
        for(int i= 0; i < nservers; ++i)
            servers[i]->newObjectAsync(*this);
        const_cast<bool&>(report_progress)= rep_prog;

        while(nservers > 0)
        {
            PP<RemotePLearnServer> s= service.waitForResult();
            if(learners_ids.find(s) == learners_ids.end())
            {
                if(curpos < len) // get learner id and send first chunk to process
                {
                    /* step 1 (once per slave) */
                    int id;
                    s->getResults(id);
                    learners_ids[s]= id;
                    int clen= min(chunksize, testset.length()-curpos);
                    chunkszs[s]= clen;
                    VMat sts= new RowsSubVMatrix(testset, curpos, clen);
                    if(master_sends_testset_rows)
                        sts= new MemoryVMatrix(sts.toMat());
                    else
                    {
                        // send testset once and for all, put it in object map of remote server
                        int tsid= s->newObject(*testset);
                        s->link(tsid, testset);
                    }
                    curpos+= clen;
                    s->callMethod(id, "sub_test", sts, template_vsc, 
                                  static_cast<bool>(testoutputs), static_cast<bool>(testcosts));
                    chunknums[s]= chunks_called;
                    ++chunks_called;
                }
                else // all chunks processed, free server
                {
                    /* step 4 (once per slave) */
                    s->getResults(); // learner deleted
                    s->unlink(testset);
                    service.freeServer(s);
                    --nservers;
                }
            }
            else // get chunk result
            {
                PP<VecStatsCollector> vsc;
                VMat chunkout, chunkcosts;

                s->getResults(vsc, chunkout, chunkcosts);

                rowsdone+= chunkszs[s];
                if(report_progress) pb->update(rowsdone);

                int chunknum= chunknums[s];
                if(curpos < len) // more chunks to do, assign one to this server
                {
                    /* step 2 (repeat as needed) */
                    int clen= min(chunksize, testset.length()-curpos);
                    chunkszs[s]= clen;
                    VMat sts= new RowsSubVMatrix(testset, curpos, clen);
                    if(master_sends_testset_rows)
                        sts= new MemoryVMatrix(sts.toMat());
                    curpos+= clen;
                    s->callMethod(learners_ids[s], "sub_test", sts, template_vsc, 
                                  static_cast<bool>(testoutputs), static_cast<bool>(testcosts));
                    chunknums[s]= chunks_called;
                    ++chunks_called;
                }
                else // all chunks processed, delete learner form server
                {
                    /* step 3 (once per slave) */
                    s->deleteObjectAsync(learners_ids[s]);
                    learners_ids.erase(s);
                }

                // now merge chunk results w/ global results
                if(test_stats)
                {
                    vscs[chunknum]= vsc;
                    map<int, PP<VecStatsCollector> >::iterator it= vscs.find(last_chunknum+1);
                    while(it != vscs.end())
                    {
                        ++last_chunknum;
                        test_stats->merge(*(it->second));
                        vscs.erase(it);
                        it= vscs.find(last_chunknum+1);
                    }
                }

                if(testoutputs)
                    for(int i= 0, j= chunknum*chunksize; i < chunksize && j < len; ++i, ++j)
                        testoutputs->forcePutRow(j, chunkout->getRowVec(i));
                if(testcosts)
                    for(int i= 0, j= chunknum*chunksize; i < chunksize && j < len; ++i, ++j)
                        testcosts->forcePutRow(j, chunkcosts->getRowVec(i));
            }
        }
    }
    else // Sequential test 
    {
        if (test_minibatch_size==1)
        {
            for (int i = 0; i < len; i++)
            {
                testset.getExample(i, input, target, weight);
                // Always call computeOutputAndCosts, since this is better
                // behaved with stateful learners
                computeOutputAndCosts(input,target,output,costs);
                if (testoutputs) testoutputs->putOrAppendRow(i, output);
                if (testcosts) testcosts->putOrAppendRow(i, costs);
                if (test_stats) test_stats->update(costs, weight);
                if (report_progress) pb->update(i);
            }
        } else
        {
            int n_batches = len/test_minibatch_size, i=0;
            b_inputs.resize(test_minibatch_size,inputsize());
            b_outputs.resize(test_minibatch_size, out_size);
            b_costs.resize(test_minibatch_size,costs.length());
            b_targets.resize(test_minibatch_size,targetsize());
            b_weights.resize(test_minibatch_size);
            for (int b=0;b<n_batches;b++,i+=test_minibatch_size)
            {
                testset->getExamples(i,test_minibatch_size,b_inputs,b_targets,b_weights);
                computeOutputsAndCosts(b_inputs,b_targets,b_outputs,b_costs);
                for (int j=0;j<test_minibatch_size;j++)
                {
                    if (testoutputs) testoutputs->putOrAppendRow(i+j, b_outputs(j));
                    if (testcosts) testcosts->putOrAppendRow(i+j, b_costs(j));
                    if (test_stats) test_stats->update(b_costs(j), b_weights[j]);
                    if (report_progress) pb->update(i+j);
                }
            }
            if (i<len)
            {
                b_inputs.resize(len-i,inputsize());
                b_outputs.resize(len-i, out_size);
                b_costs.resize(len-i,costs.length());
                b_targets.resize(len-i,targetsize());
                b_weights.resize(len-i);
                testset->getExamples(i,len-i,b_inputs,b_targets,b_weights);
                computeOutputsAndCosts(b_inputs,b_targets,b_outputs,b_costs);
                for (int j=0;j<len-i;j++)
                {
                    if (testoutputs) testoutputs->putOrAppendRow(i+j, b_outputs(j));
                    if (testcosts) testcosts->putOrAppendRow(i+j, b_costs(j));
                    if (test_stats) test_stats->update(b_costs(j), b_weights[j]);
                    if (report_progress) pb->update(i+j);
                }
            }
        }
    }

    if (use_a_separate_random_generator_for_testing && random_gen)
        *random_gen = *copy_random_gen;

    Profiler::pl_profile_end("PLearner::test");

}

void PLearner::computeOutput(const Vec& input, Vec& output) const
{
    PLERROR("PLearner::computeOutput(Vec,Vec) not implemented in subclass %s\n",classname().c_str());
}
void PLearner::computeOutputs(const Mat& input, Mat& output) const
{
    // inefficient default implementation
    int n=input.length();
    PLASSERT(output.length()==n);
    for (int i=0;i<n;i++)
    {
        Vec in_i = input(i);
        Vec out_i = output(i); 
        computeOutput(in_i,out_i);
    }
}
void PLearner::computeOutputsAndCosts(const Mat& input, const Mat& target, 
                                      Mat& output, Mat& costs) const
{
    // inefficient default implementation
    int n=input.length();
    PLASSERT(target.length()==n);
    output.resize(n,outputsize());
    costs.resize(n,nTestCosts());
    for (int i=0;i<n;i++)
    {
        Vec in_i = input(i);
        Vec out_i = output(i); 
        Vec target_i = target(i);
        Vec c_i = costs(i);
        computeOutputAndCosts(in_i,target_i,out_i,c_i);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// sub-test, used by parallel test ('remote' version which returns a tuple w/ results.) //
//////////////////////////////////////////////////////////////////////////////////////////
tuple<PP<VecStatsCollector>, VMat, VMat> PLearner::sub_test(VMat testset, PP<VecStatsCollector> test_stats, bool rtestoutputs, bool rtestcosts) const
{
    VMat testoutputs= 0;
    VMat testcosts= 0;
    int outsize= outputsize();
    int costsize= nTestCosts();
    int len= testset.length();
    if(rtestoutputs) testoutputs= new MemoryVMatrix(len, outsize);
    if(rtestcosts) testcosts= new MemoryVMatrix(len, costsize);
    if(test_stats)
    {
        if(test_stats->maxnvalues > 0) test_stats->maxnvalues= -1; // get all counts from a chunk
        if(test_stats->m_window == -1 || test_stats->m_window > 0)
            test_stats->setWindowSize(-2); // get all observations
    }
    test(testset, test_stats, testoutputs, testcosts);
    return make_tuple(test_stats, testoutputs, testcosts);
}


//////////////////////////////////////////////////////////////////////////////////////////
// remote interface for test                                                            //
//////////////////////////////////////////////////////////////////////////////////////////
tuple<PP<VecStatsCollector>, VMat, VMat> PLearner::remote_test(VMat testset, PP<VecStatsCollector> test_stats, bool rtestoutputs, bool rtestcosts) const
{
    VMat testoutputs= 0;
    VMat testcosts= 0;
    int outsize= outputsize();
    if (outsize < 0)
        // Negative outputsize: the output will be empty to avoid a crash.
        outsize = 0;
    int costsize= nTestCosts();
    int len= testset.length();
    if(rtestoutputs) testoutputs= new MemoryVMatrix(len, outsize);
    if(rtestcosts) testcosts= new MemoryVMatrix(len, costsize);
    test(testset, test_stats, testoutputs, testcosts);
    return make_tuple(test_stats, testoutputs, testcosts);
}

///////////////
// initTrain //
///////////////
bool PLearner::initTrain()
{
    string warn_msg = "In PLearner::initTrain (called by '" +
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

    // Meta learners may need to set the stats_collector of their sub-learners
    setTrainStatsCollector(train_stats);

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


//#####  computeInputOutputMat  ###############################################

Mat PLearner::computeInputOutputMat(VMat inputs) const
{
    int l = inputs.length();
    int nin = inputsize();
    int nout = outputsize();
    Mat m(l, nin+nout);
    for(int i=0; i<l; i++)
    {
        Vec v = m(i);
        Vec invec = v.subVec(0,nin);
        Vec outvec = v.subVec(nin,nout);
        inputs->getRow(i, invec);
        computeOutput(invec, outvec);
    }
    return m;
}


//#####  computeInputOutputConfMat  ###########################################

Mat PLearner::computeInputOutputConfMat(VMat inputs, real probability) const
{
    int l = inputs.length();
    int nin = inputsize();
    int nout = outputsize();
    Mat m(l, nin+3*nout);
    TVec< pair<real,real> > intervals;
    for(int i=0; i<l; i++)
    {
        Vec v = m(i);
        Vec invec   = v.subVec(0,nin);
        Vec outvec  = v.subVec(nin,nout);
        Vec lowconf = v.subVec(nin+nout, nout);
        Vec hiconf  = v.subVec(nin+2*nout, nout);
        inputs->getRow(i, invec);
        computeOutput(invec, outvec);
        bool conf_avail = computeConfidenceFromOutput(invec, outvec,
                                                      probability, intervals);
        if (conf_avail) {
            for (int j=0, n=intervals.size() ; j<n ; ++j) {
                lowconf[j] = intervals[j].first;
                hiconf[j]  = intervals[j].second;
            }
        }
        else {
            lowconf << MISSING_VALUE;
            hiconf  << MISSING_VALUE;
        }
    }
    return m;
}


//#####  computeOutputConfMat  ################################################

Mat PLearner::computeOutputConfMat(VMat inputs, real probability) const
{
    int l = inputs.length();
    int nin = inputsize();
    int nout = outputsize();
    Mat m(l, 3*nout);
    TVec< pair<real,real> > intervals;
    Vec invec(nin);
    for(int i=0; i<l; i++)
    {
        Vec v = m(i);
        Vec outvec  = v.subVec(0, nout);
        Vec lowconf = v.subVec(nout, nout);
        Vec hiconf  = v.subVec(2*nout, nout);
        inputs->getRow(i, invec);
        computeOutput(invec, outvec);
        bool conf_avail = computeConfidenceFromOutput(invec, outvec,
                                                      probability, intervals);
        if (conf_avail) {
            for (int j=0, n=intervals.size() ; j<n ; ++j) {
                lowconf[j] = intervals[j].first;
                hiconf[j]  = intervals[j].second;
            }
        }
        else {
            lowconf << MISSING_VALUE;
            hiconf  << MISSING_VALUE;
        }
    }
    return m;
}


//////////////////////////
// remote_computeOutput //
//////////////////////////
//! Version of computeOutput that returns a result by value
Vec PLearner::remote_computeOutput(const Vec& input) const
{
    int os = outputsize();
    tmp_output.resize(os >= 0 ? os : 0);
    computeOutput(input, tmp_output);
    return tmp_output;
}

///////////////////////////
// remote_computeOutputs //
///////////////////////////
Mat PLearner::remote_computeOutputs(const Mat& input) const
{
    Mat out(input.length(), outputsize() >= 0 ? outputsize() : 0);
    computeOutputs(input, out);
    return out;
}

///////////////////////////////////
// remote_computeOutputsAndCosts //
///////////////////////////////////
pair<Mat, Mat> PLearner::remote_computeOutputsAndCosts(const Mat& input,
                                                       const Mat& target) const
{
    Mat output, cost;
    computeOutputsAndCosts(input, target, output, cost);
    return pair<Mat, Mat>(output, cost);
}

////////////////
// remote_use //
////////////////
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

//! Version of computeOutputCovMat that's called by RMI
tuple<Mat, TVec<Mat> >
PLearner::remote_computeOutputCovMat(const Mat& inputs) const
{
    Mat outputs;
    TVec<Mat> covmat;
    computeOutputCovMat(inputs, outputs, covmat);
    return make_tuple(outputs, covmat);
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
