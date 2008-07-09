// -*- C++ -*-

// Stan.cc
//
// Copyright (C) 2008 Stanislas Lauly
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

// Authors: Stanislas Lauly

/*! \file Stan.cc */


#include "Stan.h"
#include <plearn_learners/hyper/HyperLearner.h>
#include <plearn_learners_experimental/DynamicallyLinkedRBMsModel.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'Stan' command in the command registry
PLearnCommandRegistry Stan::reg_(new Stan);

Stan::Stan()
    : PLearnCommand(
        "stan",
        "La commande pour les affaires de Stan",
        "stan generate path_to_model.psave nb_notes \n"
        "ex: stan generate /home/stan/Documents/recherche_maitrise/DDBN_bosendorfer/exp_tar_tm1__in_tm1_tp1/exp-expressive_timing-rnet_1hid-sizes=95-7-7-1-1-1-1-1-1-mds=20-stepsize=1-seed=654321-eoss=8-nhid=40-lrl=0.0001-utlm=1-20080529:185450/Split0/LearnerExpdir/final_learner.psave 1 \n"
        "\n"
        )
{}

//! The actual implementation of the 'Stan' command
void Stan::run(const vector<string>& args)
{
    string subcommand = args[0];
    string modelpath = args[1];

    DynamicallyLinkedRBMsModel* model=0;
    Object* obj = loadObject(modelpath);
    PP<Object> ppobj = obj;
	
    HyperLearner* hyper = dynamic_cast<HyperLearner*>(obj);
    if(hyper!=0)
    {
        PP<PLearner> l = hyper->getLearner();
        model = dynamic_cast<DynamicallyLinkedRBMsModel*>((PLearner*)l);
    }
    else
    {
        model = dynamic_cast<DynamicallyLinkedRBMsModel*>(obj);
    }
	  
    if(model==0)
        PLERROR("Le fichier doit contenir soit un HyperLearner contenant un DynamicallyLinkedRBMsModel, soit un DynamicallyLinkedRBMsModel directement");
    
    perr << "Successfully loaded " << modelpath << endl;
    
    if(subcommand=="generate")
    {        
        int t = toint(args[2]);
        int n = toint(args[3]);
        //perr << "Generating " << nb_notes << " notes!" << endl;
        perr << "Generating " << endl;
        model->generate(t, n);
        
    }
    else
        perr << "No such subcommand: " << subcommand << endl;

    // *** PLEASE COMPLETE HERE ****
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
