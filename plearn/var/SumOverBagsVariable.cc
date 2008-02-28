// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
 * This file is part of the PLearn library.
 ******************************************************* */

#include "SumOverBagsVariable.h"
#include <plearn/math/TMat_maths.h>
//#include "PLMPI.h"
//#include "DisplayUtils.h"

namespace PLearn {
using namespace std;



/** SumOverBagsVariable **/

PLEARN_IMPLEMENT_OBJECT(SumOverBagsVariable, "Variable that sums the value of a Func each time evaluated on a subsequence of a VMat\n", 
                        "returns\n"
                        "   Sum_{bags in vmat} f(inputs and targets in bag)\n"
                        "(it can average this sum over the number of bags if the 'average' option is set).\n"
                        "By convention a bag is a sequence of rows of the vmat in which the last column of the target\n"
                        "indicates whether the row is the first one (and/or) the last one, with its two least significant bits:\n"
                        "   last_column_of_target == 1 ==> first row\n"
                        "   last_column_of_target == 2 ==> last row\n"
                        "   last_column_of_target == 0 ==> intermediate row\n"
                        "   last_column_of_target == 1+2==3 ==> single-row bag (both first and last).\n"
                        "The option n_samples controls how many terms in the sum are considered at a time:\n"
                        "   n_samples <= 0: sum over the whole vmat (e.g. for batch gradient computation)\n"
                        "   n_samples = 1: sum over a single bag at a time (e.g. for stochastic gradient)\n"
                        "                  where each fprop or fbprop advances to the next bag\n"
                        "   otherwise: sum over n_samples bags at a time (e.g. for min-batch training)\n"
                        "The last column of the target is not given in the call to f, but a bag_size input is provided instead.\n"
                        "The inputs to f are: (matrix of bag inputs, the bag size, the bag target, [the bag weight])\n"
                        "(the bag weight is included only if there are weights in the original VMat)."
    );

/////////////////////////
// SumOverBagsVariable //
/////////////////////////
SumOverBagsVariable::SumOverBagsVariable()
    : vmat(), f(),
      average(0),
      max_bag_size(-1), n_samples(1),
      transpose(0),
      curpos()
{}

SumOverBagsVariable::SumOverBagsVariable(VMat the_vmat, Func the_f, int max_bagsize, int nsamples, bool the_average, bool the_transpose)
    : inherited(nonInputParentsOfPath(the_f->inputs,the_f->outputs), 
                the_f->outputs[0]->length(), 
                the_f->outputs[0]->width()),
      vmat(the_vmat), f(the_f),
      average(the_average),
      max_bag_size(max_bagsize), n_samples(nsamples),
      transpose(the_transpose),
      curpos(0), bag_size(0)
{
    build();
}

///////////
// build //
///////////
void SumOverBagsVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void SumOverBagsVariable::build_()
{
    if (vmat)
    {
        if (f->outputs.size()!=1)
            PLERROR("SumOverBagsVariable: expected a func with a single output variable (you may use concat to form a single output Var)");
        if (vmat->weightsize()!=0 && vmat->weightsize()!=1)
            PLERROR("SumOverBagsVariable expected vmat->weightsize to be 0 or 1");
    
        if (transpose) {
            input_values.resize(vmat->inputsize(), max_bag_size);
        } else {
            input_values.resize(max_bag_size,vmat->inputsize());
        }
        output_value.resize(f->outputs[0]->nelems());
        output_av = Array<Vec>(output_value);
        gradient_av = Array<Vec>(gradient);
        f->inputs.setDontBpropHere(true);

        bag_size_vec.resize(1);
        bag_target_and_bag_signal.resize(vmat->targetsize());
        bag_target.resize(vmat->targetsize() - 1);
        bag_signal = bag_target_and_bag_signal.subVec(vmat->targetsize()-1,1);
        int ws = vmat->weightsize();
        bag_weight.resize(ws);
        if (ws > 0) {
            f_inputs.resize(4);
            f_inputs[3] = bag_weight;
        } else {
            f_inputs.resize(3);
        }
        f_inputs[0] = input_values.toVec();
        f_inputs[1] = bag_size_vec;
        f_inputs[2] = bag_target;
        unused_gradients.resize(f_inputs.size());
        for (int i=0;i<f_inputs.size();i++) unused_gradients[i] = f_inputs[i].copy();
    }
}

////////////////////
// declareOptions //
////////////////////
void SumOverBagsVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "f", &SumOverBagsVariable::f, OptionBase::buildoption, 
                  "    Func that is applied on each bag, whose input is the following array of Vars:\n"
                  "    (matrix of bag inputs, the bag size, the bag target, [the bag weight]).\n");

    declareOption(ol, "vmat", &SumOverBagsVariable::vmat, OptionBase::buildoption, 
                  "    VMatrix that contains the data, with multiple consecutive rows forming one bag.\n"
                  "    The last column of the target indicates the beginning and end of each bag, as follows:\n"
                  "   last_column_of_target == 1 ==> first row\n"
                  "   last_column_of_target == 2 ==> last row\n"
                  "   last_column_of_target == 0 ==> intermediate row\n"
                  "   last_column_of_target == 1+2==3 ==> single-row bag (both first and last).\n");

    declareOption(ol, "average", &SumOverBagsVariable::average, OptionBase::buildoption, 
                  "    If set to 1, then will compute the mean of the sum, and not the sum itself.");

    declareOption(ol, "max_bag_size", &SumOverBagsVariable::max_bag_size, OptionBase::buildoption, 
                  "    maximum number of examples in a bag (more than that in vmat will trigger a run-time error).\n");

    declareOption(ol, "n_samples", &SumOverBagsVariable::n_samples, OptionBase::buildoption, 
                  "    number of bags to iterate over (1 for online gradient, <=0 for batch).");

    declareOption(ol, "transpose", &SumOverBagsVariable::transpose, OptionBase::buildoption, 
                  "    If set to 1, then the bag inputs will be put in columns instead of rows.\n"
                  "    This can be useful if the Func f takes column vars as inputs.");

    inherited::declareOptions(ol);
}

///////////////////
// recomputeSize //
///////////////////
void SumOverBagsVariable::recomputeSize(int& l, int& w) const
{
    if (f && f->outputs.size()) {
        l = f->outputs[0]->length();
        w = f->outputs[0]->width();
    } else
        l = w = 0;
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SumOverBagsVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(vmat, copies);
    deepCopyField(f, copies);
    deepCopyField(output_value, copies);
    deepCopyField(input_values, copies);
    deepCopyField(bag_size_vec, copies);
    deepCopyField(bag_target_and_bag_signal, copies);
    deepCopyField(bag_target, copies);
    deepCopyField(bag_signal, copies);
    deepCopyField(bag_weight, copies);
    deepCopyField(f_inputs, copies);
    deepCopyField(unused_gradients, copies);
    deepCopyField(output_av, copies);
    deepCopyField(gradient_av, copies);
}


/////////////////
// fpropOneBag //
/////////////////
void SumOverBagsVariable::fpropOneBag(bool do_bprop)
{
    static real dummy_weight=0;
    bool reached_end_of_bag=false;
    if (transpose) {
        input_values.resize(input_values.length(), max_bag_size);
    } else {
        input_values.resize(max_bag_size,input_values.width());
    }
    for (bag_size=0;!reached_end_of_bag;bag_size++)
    {
        if (bag_size>=max_bag_size)
            PLERROR("SumOverBagsVariable: bag size=%d > expected max. bag size(%d)",
                    bag_size,max_bag_size);
        Vec input_value;
        if (transpose) {
            input_value.resize(input_values.length());
        } else {
            input_value = input_values(bag_size);
        }
        if (vmat->weightsize()>0)
        {
            real& weight = bag_weight[0];
            vmat->getExample(curpos,input_value,bag_target_and_bag_signal,weight);
        }
        else
            vmat->getExample(curpos, input_value,
                             bag_target_and_bag_signal, dummy_weight);
        if (bag_size == 0) {
            // It's the first element: we copy the good target.
            bag_target << bag_target_and_bag_signal.subVec(
                                    0, bag_target_and_bag_signal.length() - 1);
        } else {
#ifdef BOUNDCHECK
            // Safety check: make sure the target is the same for all elements
            // in the bag.
            Vec targ = bag_target_and_bag_signal.subVec(
                    0, bag_target_and_bag_signal.length() - 1);
            PLASSERT( targ.length() == bag_target.length() );
            for (int i = 0; i < targ.length(); i++)
                if (!is_equal(bag_target[i], targ[i]))
                    PLERROR("In SumOverBagsVariable::fpropOneBag - A bag must "
                            "have the same target across all elements in it");
#endif
        }
        if (transpose) {
            // Need to put input_value into input_values, because it uses a separate
            // storage.
            input_values.column(bag_size) << input_value;
        }
        if (bag_size==0 && !(int(bag_signal[0]) & 1))
            PLERROR("SumOverBagsVariable: data synchronization error, first row of bag has wrong bag signal");
        reached_end_of_bag = (int(bag_signal[0]) & 2) != 0;
        if(++curpos == vmat->length())
        {
            curpos = 0;
            if (!reached_end_of_bag)
            {
                PLERROR("SumOverBagsVariable: last bag of VMatrix is not complete");
                return;
            }
        }
    }
    bag_size_vec[0]=bag_size;
    if (do_bprop)
        f->fbprop(f_inputs,output_av,unused_gradients,gradient_av);
    else
        f->fprop(f_inputs,output_av);
    value += output_value;
}

///////////
// fprop //
///////////
void SumOverBagsVariable::fprop()
{
    value.clear();
    f->recomputeParents();
    if (n_samples==1)
        fpropOneBag();
    else if (n_samples<=0) // one pass through the whole data set
    {
        curpos=0;
        int count_bags = 0;
        do {
            fpropOneBag();
            count_bags++;
        }
        while (curpos>0);
        if (average) {
            value /= count_bags;
        }
    }
    else {
        for (int i=0;i<n_samples;i++)
            fpropOneBag();
        if (average) {
            value /= n_samples;
        }
    }
}


////////////
// fbprop //
////////////
void SumOverBagsVariable::fbprop()
{
    value.clear();
    f->recomputeParents();
    if (n_samples==1)
        fpropOneBag(true);
    else if (n_samples<=0) // one pass through the whole data set
    {
        if (average) {
            // We don't know in advance how many bags there are, so the gradient
            // can't be propagated correctly.
            PLERROR("In SumOverBagsVariable::fbprop - If you want to get the average, you must tell me the number of bags in n_samples > 0, because I'm too dumb to guess it.");
        }
        curpos = 0;
        do {
            fpropOneBag(true);
        }
        while (curpos>0);
    }
    else {
        if (average) {
            gradient /= n_samples;
        }
        for (int i=0;i<n_samples;i++)
            fpropOneBag(true);
        if (average) {
            value /= n_samples;
        }
    }
}

///////////
// bprop //
///////////
void SumOverBagsVariable::bprop()
{ 
    fbprop();
}

///////////////
// printInfo //
///////////////
void SumOverBagsVariable::printInfo(bool print_gradient)
{
    f->fproppath.printInfo(print_gradient);
    cout << info() << " : " << getName() << "(max_bag_size=" << max_bag_size << ", ";
    cout << ", n_samples=" << n_samples << ") = " << value;
    if (print_gradient) cout << " gradient=" << gradient;
    cout << endl; 
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
