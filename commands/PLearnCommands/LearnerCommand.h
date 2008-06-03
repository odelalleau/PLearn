// -*- C++ -*-

// LearnerCommand.h
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

/*! \file LearnerCommand.h */


#ifndef LearnerCommand_INC
#define LearnerCommand_INC

#include "PLearnCommand.h"
#include "PLearnCommandRegistry.h"
#include <plearn/math/pl_math.h>  //!< For definition of real.
#include <plearn/math/TVec.h>     //!< For definition of TVec

namespace PLearn {
using namespace std;

class LearnerCommand: public PLearnCommand
{

public:

    LearnerCommand();                    

    virtual void run(const vector<string>& args);

    static void train(const PPath& learner_spec_file,
                      const PPath& trainset_spec,
                      const PPath& save_learner_file,
                      bool no_forget = false);

    static void test(const string& trained_learner_file, const string& testset_spec, const string& stats_file, const string& outputs_file, const string& costs_file);
    static void compute_outputs(const string& trained_learner_file, const string& test_inputs_spec, const string& outputs_file);

    static void process_dataset(const string& trained_learner_file, const string& dataset_spec, const string& processed_dataset_pmat);

    static void compute_outputs_on_1D_grid(const string& trained_learner_file, const string& grid_outputs_file, 
                                           real xmin, real xmax, int nx);

    static void compute_outputs_on_2D_grid(const string& trained_learner_file, const string& grid_outputs_file, 
                                           real xmin, real xmax, real ymin, real ymax,
                                           int nx, int ny);

    static void compute_outputs_on_auto_grid(const string& trained_learner_file, const string& grid_outputs_file, 
                                             const string& dataset_spec, real extra_percent,
                                             int nx, int ny=0);

    static void analyze_inputs(const string& data_file, const string& result_file, real epsilon, const TVec<string>& learner_files);
  
protected:

    static PLearnCommandRegistry reg_;

};

  
} // end of namespace PLearn

#endif


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
