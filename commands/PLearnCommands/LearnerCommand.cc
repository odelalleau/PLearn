// -*- C++ -*-

// LearnerCommand.cc
//
// Copyright (C) 2004 Pascal Vincent 
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

// Authors: Pascal Vincent

/*! \file LearnerCommand.cc */


#include "LearnerCommand.h"
#include <plearn_learners/generic/PLearner.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/db/getDataSet.h>
#include <plearn/io/load_and_save.h>
#include <plearn/io/openString.h>
#include <plearn/io/PyPLearnScript.h>       //!< For smartLoadObject(..)
#include <plearn/base/lexical_cast.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'LearnerCommand' command in the command registry
PLearnCommandRegistry LearnerCommand::reg_(new LearnerCommand);

LearnerCommand::LearnerCommand():
    PLearnCommand("learner",

                  "Allows to train, use and test a learner",

                  "The following forms of the learner command are allowed:\n"
                  "\n"
                  "learner train <learner_spec.plearn> <trainset.vmat> <trained_learner.psave> [no_forget]\n"
                  "  -  Will train the specified learner on the specified trainset and save the resulting trained learner as\n"
                  "     trained_learner.psave. If the optional keyword argument 'no_forget' is provided, then the learner will\n"
                  "     not be reset by calling forget before training.\n"
                  "\n"
                  "learner test <trained_learner.psave> <testset.vmat> <cost.stats> [<outputs.pmat> [<costs.pmat>]] [--set_testset_as_trainingset]\n"
                  "  -  Tests the specified learner on the testset. Will produce a cost.stats file (viewable with the plearn stats\n"
                  "     command) and optionally saves individual outputs and costs\n"
                  "\n"
                  "learner compute_outputs <trained_learner.psave> <test_inputs.vmat> <outputs.pmat> (or 'learner co' as a shortcut)\n"
                  "\n"
                  "learner process_dataset <trained_learner.psave> <dataset.vmat> <processed_dataset.pmat>\n"
                  "  - process a full dataset (possibly containing input,target,weight,extra,parts). \n"
                  "    writes processed dataset as a pmat. This calls method processDataset whose default version \n"
                  "    uses computeOutput to process the input part, and simply passes on the other parts unchanged.\n"
                  "    Typical usage: preprocessing data with PCA for ex. \n\n"
                  // "learner compute_costs <trained_learner.psave> <testset.vmat> <outputs.pmat> <costs.pmat>\n" 
                  "learner compute_outputs_on_1D_grid <trained_learner.psave> <gridoutputs.pmat> <xmin> <xmax> <nx> (shortcut: learner cg1)\n"
                  "  -  Computes output of learner on nx equally spaced points in range [xmin, xmax] and writes the list of (x,output)\n"
                  "     in gridoutputs.pmat \n"
                  "\n"
                  "learner compute_outputs_on_2D_grid <trained_learner.psave> <gridoutputs.pmat> <xmin> <xmax> <ymin> <ymax> <nx> <ny> (shortcut: learner cg2)\n"
                  "  -  Computes output of learner on the regular 2d grid specified and writes the list of (x,y,output) in gridoutputs.pmat\n"
                  "\n"
                  "learner compute_outputs_on_auto_grid <trained_learner.psave> <gridoutputs.pmat> <trainset.vmat> <nx> [<ny>] (shortcut: learner cg)\n"
                  "  -  Automatically determines a bounding-box from the trainset (enlarged by 5%), and computes the output along a\n"
                  "     regular 1D grid of <nx> points or a regular 2D grid of <nx>*<ny> points. (Note: you can also invoke command vmat\n"
                  "     bbox to determine the bounding-box by yourself, and then invoke learner cg1 or learner cg2 appropriately)\n"
                  "\n"
                  "learner analyze_inputs <data.vmat> <results.pmat> <epsilon> <learner_1> ... <learner_n>\n"
                  "  -  Analyze the influence of inputs of given learners. The output of each sample in the data VMatrix is computed\n"
                  "     when each input is perturbed, so as to estimate the derivative of the output with respect to the input. This\n"
                  "     is averaged over all samples and all learners so as to estimate the influence of each input. In the results.pmat\n"
                  "     file, are stored the average, variance, min and max of the derivative for all inputs (and outputs).\n"
                  "\n"
                  "The datasets do not need to be .vmat they can be any valid vmatrix (.amat .pmat .dmat)"
        ) 
{}

///////////
// train //
///////////
void LearnerCommand::train(const PPath& learner_spec_file,
                           const PPath& trainset_spec,
                           const PPath& save_learner_file,
                           bool no_forget)
{
    PP<PLearner> learner;
    string learner_spec = readFileAndMacroProcess(learner_spec_file);
    PStream in = openString(learner_spec, PStream::plearn_ascii);
    in >> learner;
    if (learner.isNull())
        PLERROR("This file cannot be correctly parsed (syntax error?): %s",
                learner_spec_file.absolute().c_str());
    VMat trainset = getDataSet(trainset_spec);
    PP<VecStatsCollector> train_stats = new VecStatsCollector();
    learner->setTrainStatsCollector(train_stats);
    learner->setTrainingSet(trainset, !no_forget);
    learner->train();
    PLearn::save(save_learner_file, learner);
}

//////////
// test //
//////////
void LearnerCommand::test(const string& trained_learner_file, const string& testset_spec, const string& stats_file, const string& outputs_file, const string& costs_file, const bool set_testset_as_trainingset)
{
    PP<PLearner> learner =
        (PLearner*) smartLoadObject(trained_learner_file);
    VMat testset = getDataSet(testset_spec);
    int l = testset.length();
    VMat testoutputs;
    if(outputs_file!="")
        testoutputs = new FileVMatrix(outputs_file,l,learner->outputsize());
    VMat testcosts;
    if(set_testset_as_trainingset)
        learner->setTrainingSet(testset, false);
    if(costs_file!="")
        testcosts = new FileVMatrix(costs_file,l,learner->getTestCostNames());

    PP<VecStatsCollector> test_stats = new VecStatsCollector;
    test_stats->build();
    test_stats->forget();
    learner->test(testset, test_stats, testoutputs, testcosts);
    test_stats->finalize();

    PLearn::save(stats_file,test_stats);
}

/////////////////////
// compute_outputs //
/////////////////////
void LearnerCommand::compute_outputs(const string& trained_learner_file, const string& test_inputs_spec, const string& outputs_file)
{
    PP<PLearner> learner =
        (PLearner*) smartLoadObject(trained_learner_file);
    VMat testinputs = getDataSet(test_inputs_spec);
    int l = testinputs.length();
    VMat testoutputs = new FileVMatrix(outputs_file,l,learner->getOutputNames());
    learner->use(testinputs,testoutputs);
}

/////////////////////
// process_dataset //
/////////////////////
void LearnerCommand::process_dataset(const string& trained_learner_file, const string& dataset_spec, const string& processed_dataset_pmat)
{
    PP<PLearner> learner =
        (PLearner*) smartLoadObject(trained_learner_file);
    VMat dataset = getDataSet(dataset_spec);
    VMat processed = learner->processDataSet(dataset);
    processed->savePMAT(processed_dataset_pmat);
}

////////////////////////////////
// compute_outputs_on_2D_grid //
////////////////////////////////
void LearnerCommand::compute_outputs_on_2D_grid(const string& trained_learner_file, const string& grid_outputs_file, 
                                                real xmin, real xmax, real ymin, real ymax,
                                                int nx, int ny)
{
    if(nx<2 || ny<2)
        PLERROR("In LearnerCommand::compute_outputs_on_2D_grid invalid nx or ny. Must have at least a 2x2 grid");
    PP<PLearner> learner;
    PLearn::load(trained_learner_file,learner);
    if(learner->inputsize()!=2)
        PLERROR("In LearnerCommand::compute_outputs_on_2D_grid learner must have inputsize==2 (it's %d)",learner->inputsize());
    int outputsize = learner->outputsize();
    VMat gridoutputs = new FileVMatrix(grid_outputs_file,0,2+outputsize);
    real deltax = (xmax-xmin)/(nx-1);
    real deltay = (ymax-ymin)/(ny-1);

    Vec v(2+outputsize);
    Vec input = v.subVec(0,2);
    Vec output = v.subVec(2,outputsize);

    real outputsum = 0;

    real x = xmin;
    for(int i=0; i<nx; i++, x+=deltax)
    {
        input[0] = x;
        real y = ymin;
        for(int j=0; j<ny; j++, y+=deltay)
        {
            input[1] = y;
            learner->computeOutput(input,output);
            outputsum += output[0];
            gridoutputs->appendRow(v);
        }
    }

    cerr << "integral: " << outputsum*deltax*deltay << endl;
  
}

////////////////////////////////
// compute_outputs_on_1D_grid //
////////////////////////////////
void LearnerCommand::compute_outputs_on_1D_grid(const string& trained_learner_file, const string& grid_outputs_file, 
                                                real xmin, real xmax, int nx)
{
    if(nx<2)
        PLERROR("In LearnerCommand::compute_outputs_on_1D_grid invalid nx. Must be at least 2");
    PP<PLearner> learner;
    PLearn::load(trained_learner_file,learner);
    if(learner->inputsize()!=1)
        PLERROR("In LearnerCommand::compute_outputs_on_1D_grid learner must have inputsize==1 (it's %d)",learner->inputsize());
    int outputsize = learner->outputsize();
    VMat gridoutputs = new FileVMatrix(grid_outputs_file,0,1+outputsize);
    real deltax = (xmax-xmin)/(nx-1);

    Vec v(1+outputsize);
    Vec input = v.subVec(0,1);
    Vec output = v.subVec(1,outputsize);
  
    real x=xmin;
    for(int i=0; i<nx; i++, x+=deltax)
    {
        input[0] = x;
        learner->computeOutput(input,output);
        gridoutputs->appendRow(v);
    }  
}

//////////////////////////////////
// compute_outputs_on_auto_grid //
//////////////////////////////////
void LearnerCommand::compute_outputs_on_auto_grid(const string& trained_learner_file, const string& grid_outputs_file, 
                                                  const string& dataset_spec, real extra_percent,
                                                  int nx, int ny)
{
    TVec< pair<real,real> > bbox = getDataSet(dataset_spec)->getBoundingBox(extra_percent);
    if(ny>0)
        compute_outputs_on_2D_grid(trained_learner_file, grid_outputs_file, 
                                   bbox[0].first, bbox[0].second, bbox[1].first, bbox[1].second,
                                   nx, ny);
    else
        compute_outputs_on_1D_grid(trained_learner_file, grid_outputs_file, 
                                   bbox[0].first, bbox[0].second,
                                   nx);
}

////////////////////
// analyze_inputs //
////////////////////
void LearnerCommand::analyze_inputs(const string& data_file, const string& result_file, real epsilon, const TVec<string>& learner_files) {
    // Load dataset and learners.
    cout << "Loading dataset and learners" << endl;
    VMat data = getDataSet(data_file);
    int dim = data->inputsize();
    if (dim <= 0)
        PLERROR("In LearnerCommand::analyze_inputs - Cannot analyze inputs if the data's inputsize is not set");
    int n_learners = learner_files.length();
    TVec< PP<PLearner> > learners(n_learners);
    for (int i = 0; i < learner_files.length(); i++)
        PLearn::load(learner_files[i], learners[i]);
    int n_outputs = learners[0]->outputsize();
    // Analyze inputs.
    PP<ProgressBar> pb = new ProgressBar("Analyzing inputs", data->length());
    Vec v(dim);
    Vec w(dim);
    Mat outputs(n_learners, n_outputs);
    Vec new_output(n_outputs);
    Vec dummy_target;
    real dummy_weight;
    Vec deriv(n_outputs);
    TVec<string> stats;
    stats.append("E");
    stats.append("V");
    stats.append("MIN");
    stats.append("MAX");
    int n_stats = stats.length(); // Number of statistics computed for each input and output.
    VMat results = new FileVMatrix(result_file, data->inputsize(), n_stats * n_outputs + 1);
    TVec<VecStatsCollector> statscol(dim);
    Vec output_k;
    for (int i = 0; i < data->length(); pb->update(++i)) {
        data->getExample(i, v, dummy_target, dummy_weight);
        w << v;
        for (int k = 0; k < n_learners; k++) {
            output_k = outputs(k);
            learners[k]->computeOutput(w, output_k);
        }
        for (int j = 0; j < dim; j++) {
            // Analyze j-th input.
            w[j] += epsilon;
            for (int k = 0; k < n_learners; k++) {
                learners[k]->computeOutput(w, new_output);
                // Compute the derivative of the m-th output with respect to the j-th input.
                for (int m = 0; m < n_outputs; m++)
                    deriv[m] = (new_output[m] - outputs(k,m)) / epsilon;
                statscol[j].update(deriv);
            }
            w[j] = v[j];
        }
    }
    // Compiling stats.
    pb = new ProgressBar("Compiling statistics", dim);
    for (int j = 0; j < dim; pb->update(++j)) {
        statscol[j].finalize();
        Vec all(1 + n_outputs * n_stats);
        for (int i = 0; i < n_stats; i++)
            all.subVec(1 + i * n_outputs, n_outputs) << statscol[j].getAllStats(stats[i]);
        all[0] = j;
        results->putRow(j, all);
        results->addStringMapping(0, data->fieldName(j), j);
    }
    TVec<string> fieldnames;
    fieldnames.append("Field");
    for (int i = 0; i < n_stats; i++)
        for (int j = 0; j < n_outputs; j++)
            fieldnames.append(stats[i]);
    results->declareFieldNames(fieldnames);
    results->saveAllStringMappings();
}

/////////
// run //
/////////
//! The actual implementation of the 'LearnerCommand' command 
void LearnerCommand::run(const vector<string>& args)
{
    string command = args[0];
    if(command=="train")
    {
        if (args.size()==4 || args.size() == 5)
            train(args[1], args[2], args[3],
                  args.size() == 5 && args[4] == "no_forget");
        else 
            PLERROR("LearnerCommand::run you must provide 'plearn learner "
                    "train learner_spec_file trainset_spec save_learner_file "
                    "[no_forget]'");
    }
    else if(command=="test")    
    {
        if (args.size()>3)
        {
            string trained_learner_file = args[1];
            string testset_spec = args[2];
            string stats_basename = args[3];
            string outputs_file;
            bool set_testset_as_trainingset = false;
            if(args.size()>4){
                if(args[4]=="--set_testset_as_trainingset")
                    set_testset_as_trainingset = true;
                else
                    outputs_file = args[4];
            }
            string costs_file;
            if(args.size()>5){
                if(args[5]=="--set_testset_as_trainingset")
                    set_testset_as_trainingset = true;
                else
                    costs_file = args[5];
            }
            if(args.size()>6){
                PLCHECK(args[6]=="--set_testset_as_trainingset");
                set_testset_as_trainingset = true;
            }
            test(trained_learner_file, testset_spec, stats_basename, outputs_file, costs_file,
                 set_testset_as_trainingset);
        }
        else
            PLERROR("LearnerCommand::run you must provide at least 'plearn learner test <trained_learner.psave> <testset.vmat> <cost.stats>'");
    }
    else if ((command=="compute_outputs") ||(command=="co"))
    {
        if (args.size()==4)
            compute_outputs(args[1],args[2],args[3]);
        else
            PLERROR("LearnerCommand::run you must provide 'plearn learner compute_outputs learner_spec_file trainset_spec save_learner_file'");
    }
    else if (command=="process_dataset")
    {
        if (args.size()==4)
            process_dataset(args[1],args[2],args[3]);
        else
            PLERROR("LearnerCommand::run you must provide: plearn learner process_dataset <trained_learner.psave> <dataset.vmat> <processed_dataset.pmat>");
    }
    else if (command=="compute_outputs_on_1D_grid" || command=="cg1")
    {
        if(args.size()!=6)
            PLERROR("Subcommand learner compute_outputs_on_1D_grid requires 5 arguments. Check the help!");
        compute_outputs_on_1D_grid(args[1], args[2], toreal(args[3]), toreal(args[4]), toint(args[5]));
    }
    else if (command=="compute_outputs_on_2D_grid" || command=="cg2")
    {
        if(args.size()!=9)
            PLERROR("Subcommand learner compute_outputs_on_2D_grid requires 8 arguments. Check the help!");
        compute_outputs_on_2D_grid(args[1], args[2], 
                                   toreal(args[3]), toreal(args[4]),
                                   toreal(args[5]), toreal(args[6]),
                                   toint(args[7]), toint(args[8]) );
    }
    else if (command=="compute_outputs_on_auto_grid" || command=="cg")
    {
        if(args.size()<5)
            PLERROR("Subcommand learner compute_outputs_on_auto_grid requires 4 or 5 arguments. Check the help!");
        int nx = toint(args[4]);
        int ny = 0;
        if(args.size()==6)
            ny = toint(args[5]);      
        compute_outputs_on_auto_grid(args[1], args[2],
                                     args[3], 0.05,
                                     nx, ny);
    }
    else if (command == "analyze_inputs") {
        if (args.size() < 5)
            PLERROR("In LearnerCommand::run - The 'analyze_inputs' subcommand requires 4 arguments (see help)");
        real epsilon;
        if (!pl_isnumber(args[3], &epsilon))
            PLERROR("In LearnerCommand::run - The 'epsilon' option must be a real number");
        TVec<string> learners;
        for (size_t i = 4; i < args.size(); i++)
            learners.append(args[i]);
        analyze_inputs(args[1], args[2], epsilon, learners);
    }
    else
        PLERROR("Invalid command %s check the help for available commands",command.c_str());
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
