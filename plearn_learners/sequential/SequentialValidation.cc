// -*- C++ -*-

// SequentialValidation.cc
//
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
// Copyright (C) 2003 Pascal Vincent
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


// From stdlib
#include <sys/types.h>
#include <unistd.h>                          // for getpid

// From PLeearn
#include "SequentialValidation.h"
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/base/stringutils.h>
#include <plearn/io/MatIO.h>
#include <plearn/io/load_and_save.h>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    SequentialValidation,
    "The SequentialValidation class allows you to describe a typical "
    "sequential validation experiment that you wish to perform.",
    "NO HELP");

SequentialValidation::SequentialValidation()
    : init_train_size(1),
      warmup_size(0),
      train_step(1),
      last_test_time(-1),
      expdir(""),
      report_stats(true),
      save_final_model(true),
      save_initial_model(false),
      save_initial_seqval(true),
      save_data_sets(false),
      save_test_outputs(false),
      save_test_costs(false),
      save_stat_collectors(false),
      provide_learner_expdir(true),
      save_sequence_stats(true),
      report_memory_usage(false)
{}

void SequentialValidation::build_()
{
    if ( dataset && dataset->inputsize() < 0 )
        dataset->defineSizes(dataset->width(), 0, 0);
}

void SequentialValidation::build()
{
    inherited::build();
    build_();
}

void SequentialValidation::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "report_stats", &SequentialValidation::report_stats,
        OptionBase::buildoption,
        "If true, the computed global statistics specified in statnames will be saved in global_stats.pmat \n"
        "and the corresponding per-split statistics will be saved in split_stats.pmat \n"
        "For reference, all cost names (as given by the learner's getTrainCostNames() and getTestCostNames() ) \n"
        "will be reported in files train_cost_names.txt and test_cost_names.txt");

    declareOption(
        ol, "statnames", &SequentialValidation::statnames,
        OptionBase::buildoption,
        "A list of global statistics we are interested in.\n"
        "These are strings of the form S1[S2[dataset.cost_name]] where:\n"
        "  - dataset is train or test1 or test2 ... (train being \n"
        "    the first dataset in a split, test1 the second, ...) \n"
        "  - cost_name is one of the training or test cost names (depending on dataset) understood \n"
        "    by the underlying learner (see its getTrainCostNames and getTestCostNames methods) \n"
        "  - S1 and S2 are a statistic, i.e. one of: E (expectation), V(variance), MIN, MAX, STDDEV, ... \n"
        "    S2 is computed over the samples of a given dataset split. S1 is over the splits. \n");

    declareOption(
        ol, "timewise_statnames", &SequentialValidation::timewise_statnames,
        OptionBase::buildoption,
        "Statistics to be collected into a VecStatsCollector at each timestep.");
  
    declareOption(
        ol, "expdir", &SequentialValidation::expdir,
        OptionBase::buildoption,
        "Path of this experiment's directory in which to save all experiment results (will be created if it does not already exist). \n");

    declareOption(
        ol, "learner", &SequentialValidation::learner,
        OptionBase::buildoption,
        "The SequentialLearner to train/test. \n");

    declareOption(
        ol, "accessory_learners", &SequentialValidation::accessory_learners,
        OptionBase::buildoption,
        "Accessory learners that must be managed in parallel with the main one." );
  
    declareOption(
        ol, "dataset", &SequentialValidation::dataset,
        OptionBase::buildoption,
        "The dataset to use for training/testing. \n");

    declareOption(
        ol, "init_train_size", &SequentialValidation::init_train_size,
        OptionBase::buildoption,
        "Size of the first training set.  Before starting the train/test cycle,\n"
        "the method setTestStartTime() is called on the learner with init_train_size\n"
        "as argument.");

    declareOption(
        ol, "warmup_size", &SequentialValidation::warmup_size,
        OptionBase::buildoption,
        "If specified, this is a number of time-steps that are taken FROM THE\n"
        "END of init_train_size to start \"testing\" (i.e. alternating between\n"
        "train and test), but WITHOUT ACCUMULATING ANY TEST STATISTICS.  In\n"
        "other words, this is a \"warmup\" period just before the true test.\n"
        "Before starting the real test period, the setTestStartTime() method is\n"
        "called on the learner, followed by resetInternalState().  Note that\n"
        "the very first \"init_train_size\" is REDUCED by the warmup_size.\n");
  
    declareOption(
        ol, "train_step", &SequentialValidation::train_step,
        OptionBase::buildoption,
        "At how many timesteps must we retrain? (default: 1)");
  
    declareOption(
        ol, "last_test_time", &SequentialValidation::last_test_time,
        OptionBase::buildoption,
        "The last time-step to use for testing (Default = -1, i.e. use all data)");
  
    declareOption(
        ol, "save_final_model", &SequentialValidation::save_final_model,
        OptionBase::buildoption,
        "If true, the final model will be saved in model.psave \n");

    declareOption(
        ol, "save_initial_model", &SequentialValidation::save_initial_model,
        OptionBase::buildoption,
        "If true, the initial model will be saved in initial_model.psave. \n");

    declareOption(
        ol, "save_initial_seqval", &SequentialValidation::save_initial_seqval,
        OptionBase::buildoption,
        "If true, this SequentialValidation object will be saved in sequential_validation.psave. \n");

    declareOption(
        ol, "save_data_sets", &SequentialValidation::save_data_sets,
        OptionBase::buildoption,
        "If true, the data sets (train/test) for each split will be saved. \n");

    declareOption(
        ol, "save_test_outputs", &SequentialValidation::save_test_outputs,
        OptionBase::buildoption,
        "If true, the outputs of the tests will be saved in test_outputs.pmat \n");

    declareOption(
        ol, "save_test_costs", &SequentialValidation::save_test_costs,
        OptionBase::buildoption,
        "If true, the costs of the tests will be saved in test_costs.pmat \n");

    declareOption(
        ol, "save_stat_collectors", &SequentialValidation::save_stat_collectors,
        OptionBase::buildoption,
        "If true, stat collectors of each data sets (train/test) will be saved for each split. \n");

    declareOption(
        ol, "provide_learner_expdir", &SequentialValidation::provide_learner_expdir,
        OptionBase::buildoption,
        "If true, learning results from the learner will be saved. \n");

    declareOption(
        ol, "save_sequence_stats",
        &SequentialValidation::save_sequence_stats,
        OptionBase::buildoption,
        "Whether the statistics accumulated at each time step should\n"
        "be saved in the file \"sequence_stats.pmat\".  WARNING: this\n"
        "file can get big!  (Default = 1, i.e. true)");

    declareOption(
        ol, "report_memory_usage",
        &SequentialValidation::report_memory_usage,
        OptionBase::buildoption,
        "Whether to report memory usage in a directory expdir/MemoryUsage.\n"
        "Memory usage is reported AT THE BEGINNING OF EACH time-step, using\n"
        "both the /proc/PID/status method, and the 'mem_usage PID' method\n"
        "(if available).  This is only supported on Linux at the moment.\n"
        "(Default = false)");

    declareOption(
        ol, "measure_after_train",
        &SequentialValidation::measure_after_train,
        OptionBase::buildoption,
        "List of options to \"measure\" AFTER training at each timestep, but\n"
        "BEFORE testing.  The options are specified as a list of pairs\n"
        "'option':'filename', where the option is measured with respect to the\n"
        "sequential validation object itself.  Hence, if the learner contains\n"
        "an option 'advisor' that you want to save at each time step, you would\n"
        "write [\"learner.advisor\":\"advisor.psave\"].  The files are saved in the\n"
        "splitdir directory, which is unique for each timestep.");
  
    inherited::declareOptions(ol);
}

void SequentialValidation::run()
{  
    if (expdir=="")
        PLERROR("No expdir specified for SequentialValidation.");
    else
    {
        if(pathexists(expdir))
            PLERROR("Directory (or file) %s already exists. First move it out of the way.", expdir.c_str());
        if(!force_mkdir(expdir))
            PLERROR("Could not create experiment directory %s", expdir.c_str());
    }

    if (!learner)
        PLERROR("SequentialValidation::run: learner not specified.");

    if (warmup_size >= init_train_size)
        PLERROR("SequentialValidation::run: 'warmup_size' must be strictly smaller than "
                "'init_train_size'");

    if (warmup_size < 0 || init_train_size < 0)
        PLERROR("SequentialValidation::run: negative warmup_size or init_train_size.");
  
    // Get a first dataset to set inputsize() and targetsize()
    VMat train_vmat = trainVMat(init_train_size);
    for ( int a=0; a < accessory_learners.length(); a++ )
        accessory_learners[a]->setTrainingSet( train_vmat, false );
    learner->setTrainingSet( train_vmat, false );
  
    setExperimentDirectory( append_slash(expdir) );

    // If we need to report memory usage, create the appropriate directory
    if (report_memory_usage)
        force_mkdir( expdir / "MemoryUsage" );

    // Save this experiment description in the expdir (buildoptions only)
    if (save_initial_seqval)
        PLearn::save(expdir / "sequential_validation.psave", *this);

    // Create the stat collectors and set them into the learner(s)
    createStatCollectors();
    createStatSpecs();
  
    // Warm up the model before starting the real experiment; this is done
    // after setting the training stats collectors into everybody...
    if (warmup_size > 0)
        warmupModel(warmup_size);

    // Create all VMatrix related to saving statistics
    if (report_stats)
        createStatVMats();

    // Final model initialization before the test
    setTestStartTime(init_train_size, true /* call_build */);
  
    VMat test_outputs;
    VMat test_costs;
    if (save_test_outputs)
        test_outputs = new FileVMatrix(expdir / "test_outputs.pmat",0,
                                       learner->outputsize());
    if (save_test_costs)
        test_costs = new FileVMatrix(expdir / "test_costs.pmat",0,
                                     learner->getTestCostNames());

    // Some further initializations
    int maxt = (last_test_time >= 0? last_test_time : maxTimeStep() - 1);
    int splitnum = 0;
    output.resize(learner->outputsize());
    costs.resize(learner->nTestCosts());
    for (int t=init_train_size; t <= maxt; t++, splitnum++)
    {
#ifdef DEBUG
        cout << "SequentialValidation::run() -- sub_train.length = " << t << " et sub_test.length = " << t+horizon << endl;
#endif
        if (report_memory_usage)
            reportMemoryUsage(t);

        // Create splitdirs
        PPath splitdir = expdir / "test_t="+tostring(t);
        if (save_data_sets                 ||
            save_initial_model             ||
            save_stat_collectors           ||
            save_final_model               ||
            measure_after_train.size() > 0 ||
            measure_after_test.size()  > 0  )
            force_mkdir(splitdir);
    
        // Ensure a first train and, afterwards, train only if we arrive at an allowed
        // training time-step
        if ( t == init_train_size || shouldTrain(t)) {
            // Compute training set.  Don't compute test set right away in case
            // it's a complicated structure that cannot co-exist with an
            // instantiated training set
            VMat sub_train = trainVMat(t);
            if (save_data_sets)
                PLearn::save(splitdir / "training_set.psave", sub_train);
            if (save_initial_model)
                PLearn::save(splitdir / "initial_learner.psave",learner);

            // Perform train
            trainLearners(sub_train);
      
            // Save post-train stuff
            if (save_stat_collectors)
                PLearn::save(splitdir / "train_stats.psave",train_stats);
            if (save_final_model)
                PLearn::save(splitdir / "final_learner.psave",learner);
            measureOptions(measure_after_train, splitdir);
        }

        // TEST: simply use computeOutputAndCosts for 1 observation in this
        // implementation
        VMat sub_test = testVMat(t);
        testLearners(sub_test);
    
        // Save what is required from the test run
        if (save_data_sets)
            PLearn::save(splitdir / "test_set.psave", sub_test);
        if (test_outputs)
            test_outputs->appendRow(output);
        if (test_costs)
            test_costs->appendRow(costs);
        if (save_stat_collectors)
            PLearn::save(splitdir / "test_stats.psave",test_stats);
        measureOptions(measure_after_test, splitdir);

        const int nstats = statnames.size();
        Vec splitres(1+nstats);
        splitres[0] = splitnum;

        // Compute statnames for this split only
        for(int k=0; k<nstats; k++)
        {
            StatSpec& sp = statspecs[k];
            if (sp.setnum>=stcol.length())
                PLERROR("SequentialValidation::run, trying to access a test set (test%d) beyond the last one (test%d)",
                        sp.setnum, stcol.length()-1);
            splitres[k+1] = stcol[sp.setnum]->getStat(sp.intstatname);
        }

        if (split_stats_vm)
            split_stats_vm->appendRow(splitres);

        // Add to overall stats collector
        sequence_stats->update(splitres.subVec(1,nstats));

        // Now compute timewise statnames.  First loop is on the inner
        // statistics; then update the stats collector; then loop on the outer
        // statistics
        if (timewise_stats_vm) {
            const int timewise_nstats = timewise_statnames.size();
            Vec timewise_res(timewise_nstats);
            for (int k=0; k<timewise_nstats; ++k) {
                StatSpec& sp = timewise_statspecs[k];
                if (sp.setnum>=stcol.length())
                    PLERROR("SequentialValidation::run, trying to access a test set "
                            "(test%d) beyond the last one (test%d)",
                            sp.setnum, stcol.length()-1);
                timewise_res[k] = stcol[sp.setnum]->getStat(sp.intstatname);
            }
            timewise_stats->update(timewise_res);
            for (int k=0; k<timewise_nstats; ++k)
                timewise_res[k] =
                    timewise_stats->getStats(k).getStat(timewise_statspecs[k].extstat);
            timewise_stats_vm->appendRow(timewise_res);
        }
    }

    sequence_stats->finalize();

    const int nstats = statnames.size();
    Vec global_result(nstats);
    for (int k=0; k<nstats; k++)
        global_result[k] = sequence_stats->getStats(k).getStat(statspecs[k].extstat);

    if (global_stats_vm)
        global_stats_vm->appendRow(global_result);
  
    reportStats(global_result);
}

void SequentialValidation::warmupModel(int warmup_size)
{
    PLASSERT( warmup_size < init_train_size );
    setTestStartTime(init_train_size - warmup_size, true /* call_build */);
  
    for (int t = init_train_size-warmup_size ; t<init_train_size ; ++t) {
        VMat sub_train = trainVMat(t);           // train
        trainLearners(sub_train);

        VMat sub_test = testVMat(t);             // test
        testLearners(sub_test);
    }
}

void SequentialValidation::setTestStartTime(int test_start_time, bool call_build)
{
    // Ensure correct build of learner and reset internal state.  We call
    // setTestStartTime TWICE, because some learners need it before build,
    // and because other learners, such as SequentialSelector-types, will not
    // have finished to construct the complete structure of sub-learners
    // until AFTER build, and we want the setTestStartTime() message to
    // propagate to everybody.

    PLASSERT( test_start_time > 0 );
  
    // Start with the accessory learners
    for (int a=0, n=accessory_learners.length() ; a<n ; ++a ) {
        if (call_build) {
            accessory_learners[a]->setTestStartTime(test_start_time);
            accessory_learners[a]->build();
        }
        accessory_learners[a]->setTestStartTime(test_start_time);
        accessory_learners[a]->resetInternalState();
    }

    // And now the main learner
    if (call_build) {
        learner->setTestStartTime(test_start_time);
        learner->build();
    }
    learner->setTestStartTime(test_start_time);
    learner->resetInternalState();
}

void SequentialValidation::setExperimentDirectory(const PPath& _expdir)
{
    expdir = _expdir;
    if(provide_learner_expdir)
        learner->setExperimentDirectory(expdir / "Model");
}

void SequentialValidation::reportStats(const Vec& global_result)
{
    if (!report_stats)
        return;
  
    saveAscii(expdir+"global_result.avec", global_result);
//  saveAscii(expdir+"predictions.amat", learner->predictions);
//  saveAscii(expdir+"errors.amat", learner->errors, learner->getTestCostNames());
}

void SequentialValidation::reportMemoryUsage(int t)
{
    pid_t pid = getpid();
    char t_str[100];
    sprintf(t_str, "%05d", t);

    string memdir = append_slash(expdir) + "MemoryUsage";
    string method1 = string("cat /proc/")+tostring(pid)+"/status > "
        + memdir + "/status_" + t_str;
    string method2 = string("mem_usage ")+tostring(pid)+" > "
        + memdir + "/mem_usage_" + t_str;

    system(method1.c_str());
    system(method2.c_str());
}

bool SequentialValidation::shouldTrain(int t)
{
    if ( train_step <= 0 )
        return false;

    return (t - init_train_size) % train_step == 0;
}

VMat SequentialValidation::trainVMat(int t)
{
    // exclude t, last training pair is (t-2,t-1)
    PLASSERT( dataset );
    return dataset.subMatRows(0,t);
}

VMat SequentialValidation::testVMat(int t)
{
    PLASSERT( dataset );
    return dataset.subMatRows(0,t+1);
}

int SequentialValidation::maxTimeStep() const
{
    PLASSERT( dataset );
    return dataset.length();
}

void SequentialValidation::measureOptions(
    const TVec< pair<string,string> >& options, PPath where_to_save)
{
    for (int i=0, n=options.size() ; i<n ; ++i) {
        const string& optionname = options[i].first;
        PPath filename = where_to_save / options[i].second;
        string optvalue = getOption(optionname);
        PStream out = openFile(filename, PStream::raw_ascii, "w");
        out << optvalue;
    }
}

void SequentialValidation::createStatCollectors()
{
    // Always manage the accessory_learners first since they may be used
    // within the main trader.
    accessory_train_stats = new VecStatsCollector(); 
    for (int a=0, n=accessory_learners.length() ; a<n ; ++a)
        accessory_learners[a]->setTrainStatsCollector( accessory_train_stats );
  
    // stats for a train on one split
    stcol.resize(2);
    train_stats = new VecStatsCollector();
    train_stats->setFieldNames(learner->getTrainCostNames());
    learner->setTrainStatsCollector(train_stats);  
    stcol[0] = train_stats;

    // stats for a test on one split
    test_stats = new VecStatsCollector();
    test_stats->setFieldNames(learner->getTestCostNames());
    stcol[1] = test_stats;

    // stats over all sequence
    sequence_stats = new VecStatsCollector();

    // timewise stats (may not be used)
    timewise_stats = new VecStatsCollector();
}

void SequentialValidation::createStatSpecs()
{
    // Stat specs (overall)
    const int nstats = statnames.length();
    statspecs.resize(nstats);
    for (int k=0; k<nstats; k++)
        statspecs[k].init(statnames[k]);

    // Stat specs (timewise)
    const int timewise_nstats = timewise_statnames.length();
    timewise_statspecs.resize(timewise_nstats);
    for (int k=0; k<timewise_nstats; ++k)
        timewise_statspecs[k].init(timewise_statnames[k]);
}

void SequentialValidation::createStatVMats()
{
    TVec<string> traincostnames = learner->getTrainCostNames();
    TVec<string> testcostnames  = learner->getTestCostNames();
    const int nstats = statnames.size();
    const int timewise_nstats = timewise_statnames.size();

    saveStringInFile(expdir / "train_cost_names.txt", join(traincostnames,"\n")+"\n");
    saveStringInFile(expdir / "test_cost_names.txt",  join(testcostnames,"\n")+"\n");

    global_stats_vm = new FileVMatrix(expdir / "global_stats.pmat", 0, nstats);
    for(int k=0; k<nstats; k++)
        global_stats_vm->declareField(k,statspecs[k].statName());
    global_stats_vm->saveFieldInfos();

    if (save_sequence_stats) {
        split_stats_vm = new FileVMatrix(expdir+"sequence_stats.pmat", 0,
                                         1+nstats);
        split_stats_vm->declareField(0,"splitnum");
        for(int k=0; k<nstats; k++)
            split_stats_vm->declareField(k+1,statspecs[k].setname + "." + statspecs[k].intstatname);
        split_stats_vm->saveFieldInfos();
    }

    if (timewise_nstats > 0) {
        timewise_stats_vm = new FileVMatrix(expdir+"timewise_stats.pmat", 0,
                                            timewise_nstats);
        for (int k=0; k<timewise_nstats; ++k)
            timewise_stats_vm->declareField(k, timewise_statspecs[k].statName());
        timewise_stats_vm->saveFieldInfos();
    }
}

void SequentialValidation::trainLearners(VMat training_set)
{
    for (int a=0, n=accessory_learners.length(); a<n ; ++a)
    {
        accessory_train_stats->forget();
        accessory_learners[a]->setTrainingSet(training_set, false);
        accessory_learners[a]->train();        
    }
    train_stats->forget();
    learner->setTrainingSet(training_set, false);
    learner->train();
    train_stats->finalize();  
}

void SequentialValidation::testLearners(VMat test_set)
{
    double weight;
    test_set.getExample(test_set.length()-1, input, target, weight);
    for (int a=0, n=accessory_learners.length() ; a<n ; ++a )
    {
        accessory_learners[a]->setTestSet(test_set);         // temporary hack
        accessory_learners[a]->computeOutputAndCosts(input, target,
                                                     dummy_output, dummy_costs);
    }
    test_stats->forget();
    learner->setTestSet(test_set);           // temporary hack
    learner->computeOutputAndCosts(input, target, output, costs);
    test_stats->update(costs);
    test_stats->finalize();
}

void SequentialValidation::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(train_stats,            copies);
    deepCopyField(accessory_train_stats,  copies);
    deepCopyField(test_stats,             copies);
    deepCopyField(sequence_stats,         copies);
    deepCopyField(timewise_stats,         copies);
    deepCopyField(stcol,                  copies);
    deepCopyField(statspecs,              copies);
    deepCopyField(timewise_statspecs,     copies);
    deepCopyField(global_stats_vm,        copies);
    deepCopyField(split_stats_vm,         copies);
    deepCopyField(timewise_stats_vm,      copies); 
    deepCopyField(input,                  copies);
    deepCopyField(target,                 copies);
    deepCopyField(dummy_output,           copies);
    deepCopyField(dummy_costs,            copies);
    deepCopyField(output,                 copies);
    deepCopyField(costs,                  copies);
  
    deepCopyField(dataset,                copies);
    deepCopyField(learner,                copies);
    deepCopyField(accessory_learners,     copies);  
    deepCopyField(statnames,              copies);
    deepCopyField(timewise_statnames,     copies);
    deepCopyField(measure_after_train,    copies);
    deepCopyField(measure_after_test,     copies);
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
