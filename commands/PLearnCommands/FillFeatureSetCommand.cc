// -*- C++ -*-

// FillFeatureSetCommand.cc
//
// Copyright (C) 2006 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file FillFeatureSetCommand.cc */


#include "FillFeatureSetCommand.h"
#include <plearn/feat/FeatureSet.h>
#include <plearn/vmat/VMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/db/getDataSet.h>
#include <plearn/io/load_and_save.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'FillFeatureSetCommand' command in the command registry
PLearnCommandRegistry FillFeatureSetCommand::reg_(new FillFeatureSetCommand);

FillFeatureSetCommand::FillFeatureSetCommand()
    : PLearnCommand(
        "fill-feat-set",
        "PLearn command that fills a FeatureSet with the features instantiated in a VMat",
        "Usage: fill-feat-set <dataset> <feat_set> <filled_feat_set> [min_freq] [col]\n"
        "       Will fill the FeatureSet instantiated from <feat_set> \n"
        "       from the input tokens found in the <dataset> VMat.\n"
        "       The filled FeatureSet will be saved in file <filled_feat_set>.\n"
        "       A minimum frequency can be specified for the features to be\n"
        "       included in the FeatureSet. The FeatureSet can be filled by\n"
        "       the features of the tokens found in a particular column of\n"
        "       <dataset>. If not specified, then the tokens found in all the\n"
        "       input columns are used.\n"
        )
{}

//! The actual implementation of the 'FillFeatureSetCommand' command
void FillFeatureSetCommand::run(const vector<string>& args)
{
    if(args.size() != 3 && args.size() != 4 && args.size() != 5)
    {
        cerr << helpmsg << endl;
        return;
    }
    string vmat_file = args[0];
    string feat_file = args[1];
    string filled_feat_file = args[2];
    int min_freq = -1;
    int col = -1;
    if(args.size() >= 4)
        min_freq = toint(args[3]);
    if(args.size() >=5)
        col = toint(args[4]);
    VMat vmat = getDataSet(vmat_file);
    VMat get_input_vmat;
    if(col < 0)
        get_input_vmat = 
            new SubVMatrix(vmat,0,0,vmat->length(),
                           vmat->inputsize());
    else
        get_input_vmat = 
            new SubVMatrix(vmat,0,col,vmat->length(),
                           1);
        
    PP<FeatureSet> feat;
    load(feat_file,feat);
    feat->addFeatures(get_input_vmat,min_freq);
    save(filled_feat_file,feat);
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
