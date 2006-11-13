// -*- C++ -*-

// PTester.cc
//
// Copyright (C) 2002 Pascal Vincent, Frederic Morin
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

/*! \file PTester.cc */

#include <plearn/io/load_and_save.h>
#include <plearn/io/openString.h>
#include <plearn/math/VecStatsCollector.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <assert.h>
#include "PTester.h"

#include <plearn/base/stringutils.h>
#if USING_MPI
#include <plearn/sys/PLMPI.h>
#endif

namespace PLearn {
using namespace std;

TVec<string> addprepostfix(const string& prefix, const TVec<string>& names, const string& postfix)
{
    TVec<string> newnames(names.size());
    TVec<string>::const_iterator it = names.begin();
    TVec<string>::iterator newit = newnames.begin();
    while(it!=names.end())
    {
        *newit = prefix + *it + postfix;
        ++it;
        ++newit;
    }
    return newnames;
}

template<class T> TVec<T> operator&(const T& x, const TVec<T>& v)
{
    int l = v.size();
    TVec<T> res(1+l);
    res[0] = x;
    res.subVec(1,l) << v;
    return res;
}

PTester::PTester()
    :  provide_learner_expdir(false),
       report_stats(true),
       save_data_sets(false),
       save_initial_learners(false),
       save_initial_tester(true),
       save_learners(true),
       save_stat_collectors(true),
       save_test_costs(false),
       save_test_outputs(false),
       call_forget_in_run(true),
       save_test_confidence(false),
       train(true),
       enforce_clean_expdir(true)
{}

PLEARN_IMPLEMENT_OBJECT(
    PTester,
    "Manages a learning experiment, with training and estimation of generalization error.",
    "The PTester class allows you to describe a typical learning experiment that you wish to perform, \n"
    "as a training/testing of a learning algorithm on a particular dataset.\n"
    "The splitter is used to obtain one or several (such as for k-fold) splits of the dataset \n"
    "and training/testing is performed on each split. \n"
    "Requested statistics are computed, and all requested results are written in an appropriate \n"
    "file inside the specified experiment directory. \n"
    "Statistics can be either specified entirely from the 'statnames' option, or built from\n"
    "'statnames' and 'statmask'. For instance, one may set:\n"
    "   statnames = [ \"NLL\" \"mse\" ]\n"
    "   statmask  = [ [ \"E[*]\" ] [ \"test#1-2#.*\" ] [ \"E[*]\" \"STDERROR[*]\" ] ]\n"
    "and this will compute:\n"
    "   E[test1.E[NLL]], STDERROR[test1.E[NLL]], E[test2.E[NLL]], STDERROR[test2.E[NLL]]\n"
    "   E[test1.E[mse]], STDERROR[test1.E[mse]], E[test2.E[mse]], STDERROR[test2.E[mse]]\n"
    );


void PTester::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "expdir", &PTester::expdir, OptionBase::buildoption,
        "Path of this tester's directory in which to save all tester results.\n"
        "The directory will be created if it does not already exist.\n"
        "If this is an empty string, no directory is created and no output file is generated.\n");

    declareOption(
        ol, "dataset", &PTester::dataset, OptionBase::buildoption,
        "The dataset to use to generate splits. \n"
        "(This is ignored if your splitter is an ExplicitSplitter)\n"
        "Data-sets are seen as matrices whose columns or fields are layed out as \n"
        "follows: a number of input fields, followed by (optional) target fields, \n"
        "followed by a (optional) weight field (to weigh each example).\n"
        "The sizes of those areas are given by the VMatrix options \n"
        "inputsize targetsize, and weightsize, which are typically used by the \n"
        "learner upon building\n");

    declareOption(
        ol, "splitter", &PTester::splitter, OptionBase::buildoption,
        "The splitter to use to generate one or several train/test tuples from the dataset.");

    declareOption(
        ol, "statnames", &PTester::statnames, OptionBase::buildoption,
        "A list of global statistics we are interested in.\n"
        "These are strings of the form S1[dataset.S2[cost_name]] where:\n"
        "  - dataset is train or test1 or test2 ... (train being \n"
        "    the first dataset in a split, test1 the second, ...) \n"
        "  - cost_name is one of the training or test cost names (depending on dataset) understood \n"
        "    by the underlying learner (see its getTrainCostNames and getTestCostNames methods) \n"
        "  - S1 and S2 are a statistic, i.e. one of: E (expectation), V(variance), MIN, MAX, STDDEV, ... \n"
        "    S2 is computed over the samples of a given dataset split. S1 is over the splits. \n"
        "They can also be strings of the form S1[dataset.perf_evaluator_name.cost_name] \n"
        "(see option perf_evaluators) \n");

    declareOption(
        ol, "statmask", &PTester::statmask, OptionBase::buildoption,
        "A list of lists of masks. If provided, each of the lists is used to compose the statnames_processed.\n"
        "If not provided the statnames are those in the 'statnames' list. See the class help for an example.\n");

    declareOption(
        ol, "learner", &PTester::learner, OptionBase::buildoption,
        "The learner to train/test.\n");

    declareOption(
        ol, "perf_evaluators", &PTester::perf_evaluators, OptionBase::buildoption,
        "If specified, the performance evaluations returned by these named performance evaluators,\n"
        "will be appended to the list of cost statistics computed by the learner's test method.\n"
        "They will be accessible through the syntax: perf_evaluator_name.cost_name \n");

    declareOption(
        ol, "report_stats", &PTester::report_stats, OptionBase::buildoption,
        "If true, the computed global statistics specified in statnames will be saved in global_stats.pmat \n"
        "and the corresponding per-split statistics will be saved in split_stats.pmat \n"
        "For reference, all cost names (as given by the learner's getTrainCostNames() and getTestCostNames() ) \n"
        "will be reported in files train_cost_names.txt and test_cost_names.txt");

    declareOption(
        ol, "save_initial_tester", &PTester::save_initial_tester, OptionBase::buildoption,
        "If true, this PTester object will be saved in its initial state in tester.psave \n"
        "Thus if the initial .plearn file gets lost, or modified, we can always see what this tester was.\n");

    declareOption(
        ol, "save_stat_collectors", &PTester::save_stat_collectors, OptionBase::buildoption,
        "If true, stat collectors for split#k will be saved in Split#k/train_stats.psave and Split#k/test#i_stats.psave");

    declareOption(
        ol, "save_learners", &PTester::save_learners, OptionBase::buildoption,
        "If true, the final trained learner for split#k will be saved in Split#k/final_learner.psave");

    declareOption(
        ol, "save_initial_learners", &PTester::save_initial_learners, OptionBase::buildoption,
        "If true, the initial untrained learner for split#k (just after forget() has been called) will be saved in Split#k/initial_learner.psave");

    declareOption(
        ol, "save_data_sets", &PTester::save_data_sets, OptionBase::buildoption,
        "If true, the data set generated for split #k will be saved as Split#k/training_set.psave Split#k/test1_set.psave ...");

    declareOption(
        ol, "save_test_outputs", &PTester::save_test_outputs, OptionBase::buildoption,
        "If true, the outputs of the test for split #k will be saved in Split#k/test#i_outputs.pmat");

    declareOption(
        ol, "call_forget_in_run", &PTester::call_forget_in_run, OptionBase::buildoption,
        "Indication that run() should make perform() call forget() on the learner to train (won't work for more than 1 split).\n");

    declareOption(
        ol, "save_test_costs", &PTester::save_test_costs, OptionBase::buildoption,
        "If true, the costs of the test for split #k will be saved in Split#k/test#i_costs.pmat");

    declareOption(
        ol, "provide_learner_expdir", &PTester::provide_learner_expdir, OptionBase::buildoption,
        "If true, each learner to be trained will have its experiment directory set to Split#k/LearnerExpdir/");

    declareOption(
        ol, "train", &PTester::train, OptionBase::buildoption,
        "If true, the learners are trained, otherwise only tested (in that case it is advised\n"
        "to load an already trained learner in the 'learner' field)");

    declareOption(
        ol, "template_stats_collector", &PTester::template_stats_collector, OptionBase::buildoption,
        "If provided, this instance of a subclass of VecStatsCollector will be used as a template\n"
        "to build all the stats collector used during training and testing of the learner");

    declareOption(
        ol, "global_template_stats_collector", &PTester::global_template_stats_collector, OptionBase::buildoption,
        "If provided, this instance of a subclass of VecStatsCollector will be used as a template\n"
        "to build all the global stats collector that collects statistics over splits");

    declareOption(
        ol, "final_commands", &PTester::final_commands, OptionBase::buildoption,
        "If provided, the shell commands given will be executed after training is completed");

    declareOption(
        ol, "save_test_confidence", &PTester::save_test_confidence,
        OptionBase::buildoption,
        "Whether to save confidence intervals for the test outputs;\n"
        "make sense mostly if 'save_test_outputs' is also true.  The\n"
        "intervals are saved in a file SETNAME_confidence.pmat (default=false)");

    declareOption(
        ol, "enforce_clean_expdir", &PTester::enforce_clean_expdir,
        OptionBase::buildoption,
        "If this option is true, the PTester ensures that the expdir does not\n"
        "already exist when the experiment is started, and gives a PLerror\n"
        "otherwise.  This is the usual and traditional default behavior for\n"
        "PTester.  However, in some contexts, one KNOWS that the expdir is brand\n"
        "new (e.g. generated by plargs.expdir in a PTester), and might contain\n"
        "some precomputed results that are being generated as the model is\n"
        "loaded, so it is not empty.  In those contexts, it makes sense to allow\n"
        "this option to be false.\n");

    inherited::declareOptions(ol);
}

void PTester::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this
    // different than for declareOptions()
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(
        rmm, "perform", &PTester::perform,
        (BodyDoc("Performs the test, and returns the global stats specified in statnames.\n"
                 "If 'call_forget' is set to false then the call to setTrainingSet()\n"
                 "won't call forget and build.  This is useful for continuation of an\n"
                 "incremental training (such as after increasing the number of epochs\n"
                 "(nstages) ), or generally when trying different option values that\n"
                 "don't require the learning to be restarted from scratch.  However\n"
                 "call_forget will be forced to true (even if passed as false) if the\n"
                 "splitter returns more than one split.\n"),
         ArgDoc ("call_forget", "Whether forget() should be called in setTrainingSet()."),
         RetDoc ("Vector of test statistics corresponding to the requested statnames")));

    declareMethod(
        rmm, "getStatNames", &PTester::getStatNames,
        (BodyDoc("Return the statnames (potentially modified by statmask, if provided);\n"
                 "see the 'statnames' and 'statmask' options."),
         RetDoc ("Name of computed statistics.")));

    declareMethod(
        rmm, "setExperimentDirectory", &PTester::setExperimentDirectory,
        (BodyDoc("The experiment directory is the directory in which files related to\n"
                 "this model are to be saved.  If it is an empty string, it is understood\n"
                 "to mean that the user doesn't want any file created by this learner.\n"),
         ArgDoc ("expdir", "Directory name where experimental results should be saved")));

    declareMethod(
        rmm, "getExperimentDirectory", &PTester::getExperimentDirectory,
        (BodyDoc("Return the currently-set experiment directory (see setExperimentDirectory)."),
         RetDoc ("Current expdir.")));
}

// The following is no longer necessary with the declareMethod mechanism
// (to be deleted)

/**
 * void PTester::call(const string& methodname, int nargs, PStream& io)
 * {
 *     if(methodname=="perform")
 *     {
 *         if(nargs!=1) PLERROR("PTester remote method perform takes 1 argument");
 *         bool call_forget;
 *         io >> call_forget;
 *         Vec result = perform(call_forget);
 *         prepareToSendResults(io, 1);
 *         io << result;
 *         io.flush();
 *     }
 *     else if(methodname=="getStatNames")
 *     {
 *         if(nargs!=0) PLERROR("PTester remote method getStatNames takes 0 argument");
 *         TVec<string> result = getStatNames();
 *         prepareToSendResults(io, 1);
 *         io << result;
 *         io.flush();
 *     }
 *     else if(methodname=="setExperimentDirectory")
 *     {
 *         if(nargs!=1) PLERROR("PTester remote method setExperimentDirectory takes 1 argument");
 *         PPath the_expdir;
 *         io >> the_expdir;
 *         setExperimentDirectory(the_expdir);
 *         prepareToSendResults(io, 0);
 *         io.flush();
 *     }
 *     else if(methodname=="getExperimentDirectory")
 *     {
 *         if(nargs!=0) PLERROR("PTester remote method getExperimentDirectory takes 0 arguments");
 *         PPath result = getExperimentDirectory();
 *         prepareToSendResults(io, 1);
 *         io << result;
 *         io.flush();
 *     }
 *     else
 *         inherited::call(methodname, nargs, io);
 * }
 */

void PTester::build_()
{

#if USING_MPI
    if (PLMPI::rank!=0)
        expdir = "";
#endif
    statnames_processed.resize(statnames.length());
    statnames_processed << statnames;
    if (statmask) {
        // First process statmask to remove potential ranges, like test#1-3#.
        // The result is stored in the 'sm' variable.
        TVec< TVec<string> > sm(statmask.length());
        for (int i = 0; i < statmask.length(); i++) {
            for (int j = 0; j < statmask[i].length(); j++) {
                string mask = statmask[i][j];
                size_t pos;
                bool is_range = false;
                if ((pos = mask.find('#')) != string::npos) {
                    // There is a '#' character.
                    size_t pos2;
                    if ((pos2 = mask.find('#', pos + 1)) != string::npos) {
                        // There is a second '#' character.
                        vector<string> range = split(mask.substr(pos + 1, pos2 - pos - 1), '-');
                        if (range.size() == 2) {
                            // We have a range.
                            is_range = true;
                            int left = atoi(range[0].c_str());
                            int right = atoi(range[1].c_str());
                            int delta = 1;
                            if (left > right)
                                delta = -1;
                            right += delta;
                            for (int k = left; k != right; k += delta)
                                sm[i].append(mask.substr(0, pos) + tostring(k) + mask.substr(pos2 + 1, mask.size() - pos2));
                        }
                    }
                }
                if (!is_range)
                    // There is no range.
                    sm[i].append(mask);
            }
        }
        TVec< TVec<string> > temp(2);
        int d = 0;
        if (statnames.isEmpty())
            PLERROR("In PTester::build_ - If you use 'statmask' then 'statnames' cannot "
                    "be empty (use statnames = [ \"\" ] if you want to specify all "
                    "statistics through statmask)");
        temp[d] = statnames_processed;
        for (int i=0;i<sm.length();i++) {
            temp[1-d].resize(temp[d].length() * sm[i].length());

            for (int j=0;j<sm[i].length();j++) {
                string mask = sm[i][j];
                size_t pos;
                if ((pos=mask.find('*'))==string::npos) {
                    // This may actually be useful, if we want to force a value.
                    for (int k = 0; k < temp[d].length(); k++) {
                        temp[1-d][j + k * sm[i].length()] = mask;
                    }
                } else {
                    for (int k=0;k<temp[d].length();k++) {
                        if (temp[d][k].find('*')!=string::npos) {
                            PLERROR("In PTester::build_ : elements of statnames cannot contain the '*' character");
                        }
                        string elem = mask;
                        elem.replace(pos,1,temp[d][k]);
                        temp[1-d][j + k * sm[i].length()] = elem;
                    }
                }
            }
            d = 1-d;
        }
        statnames_processed = temp[d];
    }
}

// ### Nothing to add here, simply calls build_
void PTester::build()
{
    inherited::build();
    build_();
}

/////////
// run //
/////////
void PTester::run()
{
    perform(call_forget_in_run);
}

////////////////////////////
// setExperimentDirectory //
////////////////////////////
void PTester::setExperimentDirectory(const PPath& the_expdir)
{
    expdir = the_expdir / "";
}

/////////////
// perform //
/////////////
Vec PTester::perform(bool call_forget)
{
    if(!learner)
        PLERROR("No learner specified for PTester.");
    if(!splitter)
        PLERROR("No splitter specified for PTester");

    int nstats;
    nstats = statnames_processed.length();
    Vec global_result(nstats);

    {
        if(expdir!="")
        {
            if(pathexists(expdir) && enforce_clean_expdir)
                PLERROR("Directory (or file) %s already exists.\n"
                        "First move it out of the way.", expdir.c_str());
            if(!force_mkdir(expdir))
                PLERROR("In PTester Could not create experiment directory %s",expdir.c_str());
            expdir = expdir.absolute() / "";

            // Save this tester description in the expdir
            if(save_initial_tester)
                PLearn::save( expdir / "tester.psave", *this);
        }

        splitter->setDataSet(dataset);

        int nsplits = splitter->nsplits();
        if(nsplits>1)
            call_forget = true;

        TVec<string> testcostnames = learner->getTestCostNames();
        TVec<string> traincostnames = learner->getTrainCostNames();

        int nsets = splitter->nSetsPerSplit();

        // Stats collectors for individual sets of a split:
        TVec< PP<VecStatsCollector> > stcol(nsets);
        for(int setnum=0; setnum<nsets; setnum++)
        {
            if (template_stats_collector)
            {
                CopiesMap copies;
                stcol[setnum] = template_stats_collector->deepCopy(copies);
            }
            else
                stcol[setnum] = new VecStatsCollector();

            if(setnum==0)
                stcol[setnum]->setFieldNames(traincostnames);
            else
                stcol[setnum]->setFieldNames(testcostnames);

            stcol[setnum]->build();
            stcol[setnum]->forget();
        }

        PP<VecStatsCollector> train_stats = stcol[0];
        learner->setTrainStatsCollector(train_stats);

        // Global stats collector
        PP<VecStatsCollector> global_statscol;
        if (global_template_stats_collector)
        {
            CopiesMap copies;
            global_statscol = global_template_stats_collector->deepCopy(copies);
            global_statscol->build();
            global_statscol->forget();
        }
        else
            global_statscol = new VecStatsCollector();

        // Stat specs
        TVec<StatSpec> statspecs(nstats);
        for(int k=0; k<nstats; k++) {
            statspecs[k].init(statnames_processed[k]);
        }
        // Hack to accumulate statistics over splits. We store in 'acc' the sets
        // which need to accumulate statistics.
        TVec<int> acc;
        for (int k = 0; k < nstats; k++)
            if (statspecs[k].extstat == "ACC")
            {
                if (statspecs[k].setnum == 0)
                    PLERROR("In PTester::perform - For now, you cannot accumulate train stats");
                if (acc.find(statspecs[k].setnum) == -1)
                    acc.append(statspecs[k].setnum);
            }
            else if (acc.find(statspecs[k].setnum) != -1)
                PLERROR("In PTester::perform - You can't have stats with and without 'ACC' for set %d", statspecs[k].setnum);

        // int traincostsize = traincostnames.size();
        // int testcostsize = testcostnames.size();

        VMat global_stats_vm;    // the vmat in which to save global result stats specified in statnames
        VMat split_stats_vm;   // the vmat in which to save per split result stats
        if(expdir!="" && report_stats)
        {
            saveStringInFile(expdir/"train_cost_names.txt", join(traincostnames,"\n")+"\n");
            saveStringInFile(expdir/"test_cost_names.txt", join(testcostnames,"\n")+"\n");

            global_stats_vm = new FileVMatrix(expdir/"global_stats.pmat", 1, nstats);
            for(int k=0; k<nstats; k++)
                global_stats_vm->declareField(k,statspecs[k].statName());
            global_stats_vm->saveFieldInfos();

            split_stats_vm = new FileVMatrix(expdir/"split_stats.pmat", 0, 1+nstats);
            split_stats_vm->declareField(0,"splitnum");
            for(int k=0; k<nstats; k++)
                split_stats_vm->declareField(k+1,statspecs[k].setname + "." + statspecs[k].intstatname);
            split_stats_vm->saveFieldInfos();
        }

        for(int splitnum=0; splitnum<nsplits; splitnum++)
        {
            PPath splitdir;
            bool is_splitdir = false;
            if(!expdir.isEmpty()) {
                splitdir = expdir / ("Split"+tostring(splitnum));
                is_splitdir = true;
            }

            TVec<VMat> dsets = splitter->getSplit(splitnum);
            VMat trainset = dsets[0];
            if(is_splitdir && save_data_sets)
                PLearn::save(splitdir/"training_set.psave",trainset);

            if(train && provide_learner_expdir)
            {
                if(is_splitdir)
                    learner->setExperimentDirectory( splitdir/"LearnerExpdir/" );
                else
                    learner->setExperimentDirectory("");
            }

            learner->setTrainingSet(trainset, call_forget && train);
            if(dsets.size()>1)
                learner->setValidationSet(dsets[1]);

            int outputsize = learner->outputsize();


            if (train)
            {
                if(is_splitdir && save_initial_learners)
                    PLearn::save(splitdir/"initial_learner.psave",learner);

                train_stats->forget();
                learner->train();
                train_stats->finalize();
                if(is_splitdir && save_stat_collectors)
                    PLearn::save(splitdir/"train_stats.psave",train_stats);
                if(is_splitdir && save_learners)
                    PLearn::save(splitdir/"final_learner.psave",learner);
            }
            else
                learner->build();

            // perf_eval_costs[setnum][perf_evaluator_name][costname] will contain value
            // of the given cost returned by the given perf_evaluator on the given setnum
            TVec< map<string, map<string, real> > > perf_eval_costs(dsets.length());
            for(int setnum=1; setnum<dsets.length(); setnum++)
            {
                VMat testset = dsets[setnum];
                PP<VecStatsCollector> test_stats = stcol[setnum];
                string setname = "test"+tostring(setnum);
                if(is_splitdir && save_data_sets)
                    PLearn::save(splitdir/(setname+"_set.psave"),testset);
                VMat test_outputs;
                VMat test_costs;
                VMat test_confidence;
                if (is_splitdir)
                    force_mkdir(splitdir); // TODO Why is this done so late?

                if(is_splitdir && save_test_outputs)
                    test_outputs = new FileVMatrix(splitdir/(setname+"_outputs.pmat"),0,learner->getOutputNames());
                    //test_outputs = new FileVMatrix(splitdir/(setname+"_outputs.pmat"),0,outputsize);
                else if(!perf_evaluators.empty()) // we don't want to save test outputs to disk, but we need them for pef_evaluators
                { // So let's store them in a MemoryVMatrix
                    Mat data(testset.length(),outputsize);
                    data.resize(0,outputsize);
                    test_outputs = new MemoryVMatrix(data);
                    test_outputs->declareFieldNames(learner->getOutputNames());
                }

                if(is_splitdir && save_test_costs)
                    test_costs = new FileVMatrix(splitdir/(setname+"_costs.pmat"),0,learner->getTestCostNames());
                    //test_costs = new FileVMatrix(splitdir/(setname+"_costs.pmat"),0,testcostsize);
                if(is_splitdir && save_test_confidence)
                    test_confidence = new FileVMatrix(splitdir/(setname+"_confidence.pmat"),
                                                      0,2*outputsize);

                bool reset_stats = (acc.find(setnum) == -1);

                //perr << "reset_stats= " << reset_stats << endl;

                if (reset_stats)
                    test_stats->forget();
                if (testset->length()==0)
                    PLWARNING("PTester:: test set %s is of length 0, costs will be set to -1",setname.c_str());

                // Before each test set, reset the internal state of the learner
                learner->resetInternalState();

                learner->test(testset, test_stats, test_outputs, test_costs);
                if (reset_stats)
                    test_stats->finalize();
                if(is_splitdir && save_stat_collectors)
                    PLearn::save(splitdir/(setname+"_stats.psave"),test_stats);

                perf_evaluators_t::iterator it = perf_evaluators.begin();
                perf_evaluators_t::iterator itend = perf_evaluators.end();
                while(it!=itend)
                {
                    PPath perf_eval_dir;
                    if(is_splitdir)
                        perf_eval_dir = splitdir/setname/("perfeval_"+it->first);
                    Vec perf_costvals = it->second->evaluatePerformance(learner, testset, test_outputs, perf_eval_dir);
                    TVec<string> perf_costnames = it->second->getCostNames();
                    if(perf_costvals.length()!=perf_costnames.length())
                        PLERROR("vector of costs returned by performance evaluator differ in size with its vector of costnames");
                    map<string, real>& costmap = perf_eval_costs[setnum][it->first];
                    for(int costi = 0; costi<perf_costnames.length(); costi++)
                        costmap[perf_costnames[costi]] = perf_costvals[costi];
                    ++it;
                }
                computeConfidence(testset, test_confidence);
            }

            Vec splitres(1+nstats);
            splitres[0] = splitnum;

            for(int k=0; k<nstats; k++)
            {
                StatSpec& sp = statspecs[k];
                if (sp.setnum>=stcol.length())
                    splitres[k+1] = MISSING_VALUE;
//            PLERROR("PTester::perform, trying to access a test set (test%d) beyond the last one (test%d)",
//                    sp.setnum, stcol.length()-1);
                else
                {
                    if (acc.find(sp.setnum) == -1)
                    {
                        string left, right;
                        split_on_first(sp.intstatname, ".",left,right);
                        if(right!="" && perf_evaluators.find(left)!=perf_evaluators.end())
                        { // looks like a cost from a performance evaluator
                            map<string, real>& costmap = perf_eval_costs[sp.setnum][left];
                            if(costmap.find(right)==costmap.end())
                                PLERROR("No cost named %s appears to be returned by evaluator %s",right.c_str(),left.c_str());
                            splitres[k+1] = costmap[right];
                        }
                        else // must be a cost from a stats collector
                            splitres[k+1] = stcol[sp.setnum]->getStat(sp.intstatname);
                    }
                    else
                        splitres[k+1] = MISSING_VALUE;
                }
            }

            if(split_stats_vm)
            {
                split_stats_vm->appendRow(splitres);
                split_stats_vm->flush();
            }

            global_statscol->update(splitres.subVec(1,nstats));
        }


        global_statscol->finalize();
        for(int k=0; k<nstats; k++) {
            if (acc.find(statspecs[k].setnum) == -1)
                global_result[k] = global_statscol->getStats(k).getStat(statspecs[k].extstat);
            else {
                int j = statspecs[k].setnum;
                stcol[j]->finalize();
                global_result[k] = stcol[j]->getStat(statspecs[k].intstatname);
            }
        }

        if(global_stats_vm)
            global_stats_vm->appendRow(global_result);

    }

#if USING_MPI
    if (PLMPI::rank==0)
#endif
    // Perform the final commands provided in final_commands.
    for (int i = 0; i < final_commands.length(); i++) {
        system(final_commands[i].c_str());
    }

    return global_result;
}

void PTester::computeConfidence(VMat test_set, VMat confidence)
{
    PLASSERT(learner);
    if (!confidence)
        return;
    ProgressBar* pb = 0;
    const int n = test_set.length();
    if (learner->report_progress)
        pb = new ProgressBar("Computing Confidence Intervals", n);
    Vec input, target, output;
    TVec< pair<real,real> > intervals;
    Vec intervals_real;
    real weight;
    for (int i=0 ; i<n ; ++i) {
        if (pb)
            pb->update(i);
        test_set.getExample(i, input, target, weight);
        learner->computeOutput(input,output);
        learner->computeConfidenceFromOutput(input,output,0.95,intervals);
        intervals_real.resize(2*intervals.size());
        for (int j=0 ; j<intervals.size() ; ++j) {
            intervals_real[2*j] = intervals[j].first;
            intervals_real[2*j+1] = intervals[j].second;
        }
        confidence->putOrAppendRow(i,intervals_real);
    }
    delete pb;
}


TVec<string> PTester::getStatNames()
{
    return statnames_processed;
}


//#####  StatSpec  #########################################################

void StatSpec::init(const string& statname)
{
    parseStatname(statname);
}

void StatSpec::parseStatname(const string& statname)
{
    PStream in = openString(statname, PStream::plearn_ascii);
    if(in.smartReadUntilNext("[", extstat)==EOF)
        PLERROR("No opening bracket found in statname %s", statname.c_str());
    string token;
    int nextsep = in.smartReadUntilNext(".[",token);
    if(nextsep==EOF)
        PLERROR("Expected dataset.xxxSTATxxx after the opening bracket. Got %s", token.c_str());
    else if(nextsep=='[') // Old format (for backward compatibility) ex: E[E[train.mse]]
    {
        PLWARNING("In StatSpec::parseStatname - You are still using the old statnames format, please use the new one!");
        // TODO Remove the old format some day?
        intstatname = token;
        if(in.smartReadUntilNext(".",setname)==EOF)
            PLERROR("Error while parsing statname: expected a dot");
        string costname;
        if(in.smartReadUntilNext("]",costname)==EOF)
            PLERROR("Error while parsing statname: expected a closing bracket");
        intstatname = intstatname+"["+costname+"]";
    }
    else // We've read a dot. That's the new format E[train.E[mse]]
    {
        setname = token;
        if(in.smartReadUntilNext("]",intstatname)==EOF)
            PLERROR("Error while parsing statname: expected a closing bracket");
    }

    if(setname=="train")
        setnum = 0;
    else if(setname=="test")
        setnum = 1;
    else if(setname.substr(0,4)=="test")
    {
        setnum = toint(setname.substr(4));
        if(setnum==0)
            PLERROR("In parseStatname: use the name train instead of test0.\n"
                    "The first set of a split is the training set. The following are test sets named test1 test2 ...");
        if(setnum<=0)
            PLERROR("In parseStatname: parse error for %s",statname.c_str());
    }
    else
        PLERROR("In parseStatname: parse error for %s",statname.c_str());
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PTester::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(statnames, copies);
    deepCopyField(statnames_processed, copies);
    deepCopyField(dataset, copies);
    deepCopyField(final_commands, copies);
    deepCopyField(global_template_stats_collector, copies);
    deepCopyField(learner, copies);
    deepCopyField(splitter, copies);
    deepCopyField(statmask, copies);
    deepCopyField(template_stats_collector, copies);
    for(perf_evaluators_t::iterator it= perf_evaluators.begin(); it != perf_evaluators.end(); ++it)
        deepCopyField(it->second, copies);
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
