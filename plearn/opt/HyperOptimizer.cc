// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent, Frederic Morin

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
   * $Id: HyperOptimizer.cc,v 1.3 2002/09/11 07:06:38 morinf Exp $
   * AUTHORS: Pascal Vincent & Frederic Morin
   * This file is part of the PLearn library.
   ******************************************************* */

#include "HyperOptimizer.h"

namespace PLearn <%

// ###### HyperOptimizer ######################################################

IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(HyperOptimizer);

void
HyperOptimizer::declareOptions(OptionList &ol)
{
    declareOption(ol, "objective", &HyperOptimizer::objective, OptionBase::buildoption,
                  "Objective function to be optimized");
    declareOption(ol, "substrategy", &HyperOptimizer::substrategy, OptionBase::buildoption,
                  "Embedded strategies performed during optimization");
    inherited::declareOptions(ol);
}


// ###### HSetVal #############################################################

IMPLEMENT_NAME_AND_DEEPCOPY(HSetVal);

void
HSetVal::declareOptions(OptionList &ol)
{
    declareOption(ol, "param", &HSetVal::param, OptionBase::buildoption,
                  "Hyperparameters' alias to set the value for");
    declareOption(ol, "value", &HSetVal::value, OptionBase::buildoption,
                  "Value of the hyperparameters to be set");
    inherited::declareOptions(ol);
}

void
HSetVal::optimize(PP<Learner> learner, const VMat &dataset, const HAliases &aliases)
{
    string alias = const_cast<HAliases &>(aliases)[param];
    learner->setOption(alias, value);
}


// ###### HTryAll #############################################################

IMPLEMENT_NAME_AND_DEEPCOPY(HTryAll);

void
HTryAll::declareOptions(OptionList &ol)
{
    declareOption(ol, "param", &HTryAll::param, OptionBase::buildoption,
                  "Hyperparameters' alias to set the values for");
    declareOption(ol, "values", &HTryAll::values, OptionBase::buildoption,
                  "Values of the hyperparameters to try");
    inherited::declareOptions(ol);
}

void
HTryAll::optimize(PP<Learner> learner, const VMat &dataset, const HAliases &aliases)
{
    string alias = const_cast<HAliases &>(aliases)[param];
    Vec results(values.size());

    for (int k = 0; k < values.size(); ++k) {
        learner->setOption(alias, values[k]);
        results[k] = objective->test(learner, dataset);
    }
    learner->setOption(alias, values[argmin(results)]);
}


// ###### HCoordinateDescent ##################################################

IMPLEMENT_NAME_AND_DEEPCOPY(HCoordinateDescent);

void
HCoordinateDescent::declareOptions(OptionList &ol)
{
    declareOption(ol, "substrategy", &HCoordinateDescent::substrategy, OptionBase::buildoption,
                  "List of HyperOptimizers to perform optimization on");
    declareOption(ol, "max_iterations", &HCoordinateDescent::max_iterations, OptionBase::buildoption,
                  "Maximum number of iterations to perform");
    inherited::declareOptions(ol);
}

void
HCoordinateDescent::optimize(PP<Learner> learner, const VMat &dataset, const HAliases &aliases)
{
    for (int i = 0; i < max_iterations; ++i)
        for (int j = 0; j < substrategy.size(); ++j)
            substrategy[j]->optimize(learner, dataset, aliases);
}

// ###### HTryCombinations ####################################################

IMPLEMENT_NAME_AND_DEEPCOPY(HTryCombinations);

void
HTryCombinations::declareOptions(OptionList &ol)
{
    declareOption(ol, "params", &HTryCombinations::params, OptionBase::buildoption,
                  "List of hyperparameters to optimize");
    declareOption(ol, "values", &HTryCombinations::values, OptionBase::buildoption,
                  "List of values to select for optimization of the given hyperparameters");
    inherited::declareOptions(ol);
}

void
HTryCombinations::optimize(PP<Learner> learner, const VMat &dataset, const HAliases &aliases)
{
    Array<string> str_aliases(params.size());
    for (int i = 0; i < params.size(); ++i)
        str_aliases[i] = const_cast<HAliases &>(aliases)[params[i]];
    recursive_optimize(learner, dataset, str_aliases, 0);
}

real
HTryCombinations::recursive_optimize(PP<Learner> learner, const VMat &dataset, const Array<string> &aliases, int i)
{
    Vec results(values[i].size());

    for (int k = 0; k < values[i].size(); ++k) {
        learner->setOption(aliases[i], values[i][k]);
        if (i == params.size() - 1)
            results[k] = objective->test(learner, dataset);
        else
            results[k] = recursive_optimize(learner, dataset, aliases, i + 1);
    }
    int best_k = argmin(results);
    learner->setOption(aliases[i], values[i][best_k]);
    return results[best_k];
}

void
HTryCombinations::build()
{
    inherited::build();
    _build();
}

void
HTryCombinations::_build()
{
    if (values.size() != params.size())
        PLERROR("HTryCombinations::_build() - params.size() must match values.size()");
}

%>; // end of namespace PLearn
