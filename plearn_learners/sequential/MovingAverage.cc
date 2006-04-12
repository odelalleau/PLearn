// -*- C++ -*-

// MovingAverage.cc
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



#include "MovingAverage.h"
//#include "TMat_maths.h"
//#include "TMat.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(MovingAverage, "ONE LINE DESCR", "NO HELP");

MovingAverage::MovingAverage()
    : window_length(-1)
{}

void MovingAverage::build_()
{
    if(cost_funcs.size() < 1)
        PLERROR("In MovingAverage::build_()  Empty cost_funcs : must at least specify one cost function!");
    if (window_length < 1)
        PLERROR("In MovingAverage::build_()  window_length has not been set!");

    max_train_len = window_length;

    forget();
}

void MovingAverage::build()
{
    inherited::build();
    build_();
}

void MovingAverage::declareOptions(OptionList& ol)
{
    declareOption(ol, "window_length", &MovingAverage::window_length,
                  OptionBase::buildoption, "the length of the moving average window \n");

    declareOption(ol, "cost_funcs", &MovingAverage::cost_funcs,
                  OptionBase::buildoption, "a list of cost functions to use \n");

    inherited::declareOptions(ol);
}

void MovingAverage::train()
{
    ProgressBar* pb;

    static Vec input(0);
    static Vec target(targetsize());
    static Vec output(outputsize());
    static Vec cost(targetsize());
    static Mat all_targets;

    int target_pos = inputsize();
    int start = MAX(window_length-1, last_train_t+1);
    if (report_progress)
        pb = new ProgressBar("Training MovingAverage learner", train_set.length()-start);
    //train_stats->forget();
    for (int t=start; t<train_set.length(); t++)
    {
#ifdef DEBUG
        cout << "MovingAverage::train -- t = " << t << endl;
#endif
        all_targets = train_set.subMat(t-window_length+1, target_pos, window_length, targetsize());
        columnMean(all_targets,output);
        predictions(t) << output;
        if (t >= horizon)
        {
            Vec out = predictions(t-horizon);
            train_set->getSubRow(t, target_pos, target);
            if (!target.hasMissing() && !out.hasMissing())
            {
                computeCostsFromOutputs(input, out, target, cost);
                errors(t) << cost;
                train_stats->update(cost);
#ifdef DEBUG
                cout << "MovingAverage::train update train_stats pour t = " << t << endl;
#endif
            }
        }
        if (pb) pb->update(t-start);
    }
    last_train_t = MAX(train_set.length()-1, last_train_t);
#ifdef DEBUG
    cout << "MovingAverage.last_train_t = " << last_train_t << endl;
#endif

    train_stats->finalize();

    if (pb) delete pb;
}
 
void MovingAverage::test(VMat testset, PP<VecStatsCollector> test_stats,
                         VMat testoutputs, VMat testcosts) const
{
    ProgressBar* pb;

    static Vec input(0);
    static Vec target(targetsize());
    static Vec output(outputsize());
    static Vec cost(targetsize());
    static Mat all_targets;

    int start = MAX(window_length-1, last_test_t+1);
    start = MAX(last_train_t+1,start);
    int target_pos = inputsize();
    if (report_progress)
        pb = new ProgressBar("Testing MovingAverage learner", testset.length()-start);
    //test_stats->forget();
    for (int t=start; t<testset.length(); t++)
    {
#ifdef DEBUG
        cout << "MovingAverage::test -- t = " << t << endl;
#endif
        all_targets = testset.subMat(t-window_length+1, target_pos, window_length, targetsize()).toMat();
        columnMean(all_targets,output);
        predictions(t) << output;
        if (testoutputs) testoutputs->appendRow(output);
        if (t >= horizon)
        {
            Vec out = predictions(t-horizon);
            testset->getSubRow(t, target_pos, target);
            if (!target.hasMissing() && !out.hasMissing())
            {
                computeCostsFromOutputs(input, out, target, cost);
                errors(t) << cost;
                if (testcosts) testcosts->appendRow(cost);
                test_stats->update(cost);
#ifdef DEBUG
                cout << "MovingAverage::test update test_stats pour t = " << t << endl;
#endif
            }
        }
        if (pb) pb->update(t-start);
    }
    last_test_t = MAX(testset.length()-1, last_test_t);
#ifdef DEBUG
    cout << "MovingAverage.last_test_t = " << last_test_t << endl;
#endif

    if (pb) delete pb;
}

void MovingAverage::computeCostsFromOutputs(const Vec& inputs, const Vec& outputs,
                                            const Vec& targets, Vec& costs) const
{
    for (int i=0; i<cost_funcs.size(); i++)
    {
        if (cost_funcs[i]=="mse" || cost_funcs[i]=="MSE")
            costs << square(outputs-targets);
        else
            PLERROR("This cost_funcs is not implemented.");
    }
}

TVec<string> MovingAverage::getTrainCostNames() const
{ return cost_funcs; }

TVec<string> MovingAverage::getTestCostNames() const
{ return getTrainCostNames(); }

void MovingAverage::forget()
{ inherited::forget(); }

/*
  void MovingAverage::makeDeepCopyFromShallowCopy(CopiesMap& copies)
  {
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(cost_funcs, copies);
  }
*/


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
