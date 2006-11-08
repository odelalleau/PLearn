// -*- C++ -*-

// OutputFeaturesCommand.cc
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

/*! \file OutputFeaturesCommand.cc */


#include "OutputFeaturesCommand.h"
#include <plearn/feat/FeatureSet.h>
#include <plearn/vmat/VMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/db/getDataSet.h>
#include <plearn/io/load_and_save.h>
#include <plearn/feat/HashMapFeatureSet.h>
#include <plearn/feat/CachedFeatureSet.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'OutputFeaturesCommand' command in the command registry
PLearnCommandRegistry OutputFeaturesCommand::reg_(new OutputFeaturesCommand);

OutputFeaturesCommand::OutputFeaturesCommand()
    : PLearnCommand(
        "output-features",
        "PLearn command that outputs the features found in a VMatrix input fields",
        "Usage: output-features <dataset> <feat_set> <output_string> [used_feat_set]\n"
        "       Will use feat_set to obtain the features from dataset and output\n"
        "       them. If output_string == 1, the features will be in string \n"
        "       format and if output_string == 0, the feature IDs will be given.\n"
        "       Optionaly, feat_set can be saved in a new file (used_feat_set)\n"
        "       after it has been used. This can be useful when the feature set\n"
        "       maintains a cache of the previously requested features of tokens\n."
        "       Only the input fields' features are output, with the features of\n"
        "       a single token separated with by ' ' and the features from \n"
        "       different fields separated by '\\t'."
        )
{}

//! The actual implementation of the 'OutputFeaturesCommand' command
void OutputFeaturesCommand::run(const vector<string>& args)
{
    if(args.size() != 3 && args.size() != 4)
    {
        cerr << helpmsg << endl;
        return;
    }
    string vmat_file = args[0];
    string feat_file = args[1];
    string output_string = args[2];
    if(args[2] != "0" && args[2] != "1")
        PLERROR("In OutputFeaturesCommand::run(): output_string should be 0 or 1");
    bool bool_output_string = tobool(output_string);
    string used_feat_file = "";
    if(args.size() == 4) used_feat_file = args[3];

    VMat vmat = getDataSet(vmat_file);
    VMat get_input_vmat;
    get_input_vmat = 
        new SubVMatrix(vmat,0,0,vmat->length(),
                       vmat->inputsize());
    
    PP<FeatureSet> feat;
    load(feat_file,feat);
    
    Vec row(get_input_vmat->width());
    string token;
    TVec<int> features;
    TVec<string> f_str;
    int l = get_input_vmat->length();
    int w = get_input_vmat->width();
    for(int i=0; i<l; i++)
    {
        get_input_vmat->getRow(i,row);
        for(int j=0; j<w; j++)
        {            
            token = get_input_vmat->getValString(j,row[j]);
            feat->getFeatures(token, features);
            if(bool_output_string)
                for(int k=0; k<features.length(); k++)
                {
                    cout << feat->getStringFeature(features[k]);
                    if( k < features.length()-1)
                        cout << " ";
                }
            else
                for(int k=0; k<features.length(); k++)
                {
                    cout << features[k];
                    if( k < features.length()-1)
                        cout << " ";
                    
                }
            if(j < l-1)
                cout << "\t";
        }
        cout << endl; 
    }

    if(used_feat_file != "")
        save(used_feat_file,feat);
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
