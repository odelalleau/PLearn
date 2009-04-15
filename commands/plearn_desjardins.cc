// -*- C++ -*-

// plearn.cc
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux, Rejean Ducharme
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
 * $Id: plearn_light.cc 3995 2005-08-25 13:58:23Z chapados $
 ******************************************************* */

//! All includes should go into plearn_inc.h.
#include "plearn_version.h"
#ifndef WIN32
#include <plearn/misc/ShellScript.h>
#include <plearn/misc/Redirect.h>
#endif

/*****************
 * Miscellaneous *
 *****************/
#include <plearn_learners/testers/PTester.h>

/***********
 * Command *
 ***********/
#include <commands/PLearnCommands/VMatCommand.h>
//#include <commands/PLearnCommands/AutoRunCommand.h>
//#include <commands/PLearnCommands/DiffCommand.h>
//#include <commands/PLearnCommands/FieldConvertCommand.h>
#include <commands/PLearnCommands/HelpCommand.h>
//#include <commands/PLearnCommands/JulianDateCommand.h>
//#include <commands/PLearnCommands/KolmogorovSmirnovCommand.h>
#include <commands/PLearnCommands/LearnerCommand.h>
//#include <commands/PLearnCommands/PairwiseDiffsCommand.h>
#include <commands/PLearnCommands/ReadAndWriteCommand.h>
#include <commands/PLearnCommands/RunCommand.h>
//#include <commands/PLearnCommands/ServerCommand.h>
//#include <commands/PLearnCommands/TestDependenciesCommand.h>
//#include <commands/PLearnCommands/TestDependencyCommand.h>
#include <commands/PLearnCommands/StatsCommand.h>

/************
 * PLearner *
 ************/
#ifndef WIN32
#include <plearn_learners/generic/AddCostToLearner.h>
#endif
#include <plearn_learners/regressors/RegressionTree.h>
#include <plearn_learners/meta/MultiClassAdaBoost.h>
#include <plearn_learners/hyper/HyperLearner.h>
#include <plearn_learners/hyper/HyperOptimize.h>
#include <plearn_learners/hyper/EarlyStoppingOracle.h>
#include <plearn_learners/cgi/StabilisationLearner.h>
#include <plearn_learners/cgi/ConfigParsing.h>

/************
 * Splitter *
 ************/
#include <plearn/vmat/FractionSplitter.h>
#include <plearn/vmat/ExplicitSplitter.h>

/***********
 * VMatrix *
 ***********/
#include <plearn/vmat/AddMissingVMatrix.h>
#include <plearn/vmat/AutoVMatrix.h>
#include <plearn/vmat/AutoVMatrixSaveSource.h>
#include <plearn/vmat/BootstrapVMatrix.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/DichotomizeVMatrix.h>
#include <plearn/vmat/FilteredVMatrix.h>
//#include <plearn/vmat/GaussianizeVMatrix.h>
//#include <plearn/vmat/ConstantVMatrix.h>
#include <plearn/vmat/MemoryVMatrixNoSave.h>
#include <plearn/vmat/MissingInstructionVMatrix.h>
#include <plearn/vmat/ProcessingVMatrix.h>
#include <plearn/vmat/TextFilesVMatrix.h>
//#include <plearn/vmat/TransposeVMatrix.h>
#include <plearn/vmat/VariableDeletionVMatrix.h>
#include <plearn/vmat/MeanMedianModeImputationVMatrix.h>
#include <plearn/vmat/MissingIndicatorVMatrix.h>
#include <plearn/vmat/ValueSelectRowsVMatrix.h>


#include "PLearnCommands/plearn_main.h"

using namespace PLearn;

int main(int argc, char** argv)
{
    return plearn_main( argc, argv, 
                        PLEARN_MAJOR_VERSION, 
                        PLEARN_MINOR_VERSION, 
                        PLEARN_FIXLEVEL       );
}


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
