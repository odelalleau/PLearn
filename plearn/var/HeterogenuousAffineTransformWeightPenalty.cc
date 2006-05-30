// -*- C++ -*-

// HeterogenuousAffineTransformVariable.cc
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

/*! \file HeterogenuousAffineTransformWeightPenalty.cc */


#include "HeterogenuousAffineTransformWeightPenalty.h"

namespace PLearn {
using namespace std;

/** HeterogenuousAffineTransformWeightPenalty **/

PLEARN_IMPLEMENT_OBJECT(
    HeterogenuousAffineTransformWeightPenalty,
    "Penalty associated to an affine transform with continuous and discrete input",
    "Weight decay penalty associated to an affine transform with continuous and\n"
    "discrete inputs. The way the weight decay works with the discrete component\n"
    "weights is that a weight decay is applied only to the activated weights, i.e.\n"
    "to the rows of the weight matrices corresponding to the discrete components'\n"
    "values."
    );

HeterogenuousAffineTransformWeightPenalty::HeterogenuousAffineTransformWeightPenalty()
    : weight_decay_(0.0), bias_decay_(0.0), penalty_type_("L2_square")
{}

// constructors from input variables.
HeterogenuousAffineTransformWeightPenalty::HeterogenuousAffineTransformWeightPenalty(Var input, VarArray weights, TVec<bool> the_input_is_discrete, real weight_decay, real bias_decay, string penalty_type)
     : inherited(input & weights, 1, 1), input_is_discrete(the_input_is_discrete), 
       weight_decay_(weight_decay),bias_decay_(bias_decay),penalty_type_(penalty_type)
 //     : inherited(input & weights, input->length() != 1 ? weights[0]->width() : 1 , input->width() != 1 ? weights[0]->width() : 1)
 {
     build();
 }

void HeterogenuousAffineTransformWeightPenalty::recomputeSize(int& l, int& w) const
{    
    l = w = 1;
}

void HeterogenuousAffineTransformWeightPenalty::fprop()
{
    int n = varray[1]->width();
    int l = varray.length()-1;

    if (penalty_type_ == "L1_square")
    {
        if(!fast_exact_is_equal(bias_decay_, 0)) valuedata[0] = sqrt(fabs(bias_decay_))*sumabs(varray.last()->value);
        else valuedata[0] = 0;

        for(int i=1; i<l; i++)
        {
            if(input_is_discrete[i-1])
            {
                real* row = varray[i]->matValue.row((int)varray[0]->valuedata[i-1]).data();
                for(int j=0; j<n; j++)
                    valuedata[0] += sqrt(fabs(weight_decay_))*fabs(*row++);
            }
            else
            {
                for(int j=0; j<n; j++)
                    valuedata[0] += sqrt(fabs(weight_decay_))*fabs(varray[i]->valuedata[j]);
            }
        }
        valuedata[0] *= valuedata[0];
    }
    else if (penalty_type_ == "L1")
    {
        if(!fast_exact_is_equal(bias_decay_, 0)) valuedata[0] = bias_decay_*sumabs(varray.last()->value);
        else valuedata[0] = 0;

        for(int i=1; i<l; i++)
        {
            if(input_is_discrete[i-1])
            {
                real* row = varray[i]->matValue.row((int)varray[0]->valuedata[i-1]).data();
                for(int j=0; j<n; j++)
                    valuedata[0] += weight_decay_*fabs(*row++);
            }
            else
            {
                for(int j=0; j<n; j++)
                    valuedata[0] += weight_decay_*fabs(varray[i]->valuedata[j]);
            }
        }
    }
    else if (penalty_type_ == "L2_square")
    {
        if(!fast_exact_is_equal(bias_decay_, 0)) valuedata[0] = bias_decay_*sumsquare(varray.last()->value);
        else valuedata[0] = 0;

        for(int i=1; i<l; i++)
        {
            if(input_is_discrete[i-1])
            {
                real* row = varray[i]->matValue.row((int)varray[0]->valuedata[i-1]).data();
                for(int j=0; j<n; j++)
                    valuedata[0] += weight_decay_*square_f(*row++);
            }
            else
            {
                for(int j=0; j<n; j++)
                    valuedata[0] += weight_decay_*square_f(varray[i]->valuedata[j]);
            }
        }
    }

}

void HeterogenuousAffineTransformWeightPenalty::bprop()
{
    int n = varray[1]->width();
    int l = varray.length()-1;

    if (penalty_type_ == "L1_square")
    {
        real delta;
        if(!fast_exact_is_equal(bias_decay_, 0))
        {
            delta = 2*sqrt(valuedata[0]*bias_decay_)*gradientdata[0];
            for(int j=0; j<n; j++)
                if(varray.last()->valuedata[j] > 0)
                    varray.last()->gradientdata[j] += delta;
                else if(varray.last()->valuedata[j] < 0)
                    varray.last()->gradientdata[j] -= delta;
        }

        delta = 2*sqrt(valuedata[0]*weight_decay_)*gradientdata[0];
        for(int i=1; i<l; i++)
        {
            if(input_is_discrete[i-1])
            {
                int r = (int)varray[0]->valuedata[i-1];
                real* row = varray[i]->matValue.row(r).data();
                real* grow = varray[i]->matGradient.row(r).data();
                for(int j=0; j<n; j++)
                {
                    if(row[j] > 0)
                        grow[j] += delta;
                    else if( row[j] < 0)
                        grow[j] -= delta;
                }
                varray[i]->updateRow(r);
            }
            else
            {
                for(int j=0; j<n; j++)
                    if(varray[i]->valuedata[j] > 0)
                        varray[i]->gradientdata[j] += delta;
                    else if(varray[i]->valuedata[j] < 0)
                        varray[i]->gradientdata[j] -= delta;
            }
        }
    }
    else if (penalty_type_ == "L1")
    {
        real delta;
        if(!fast_exact_is_equal(bias_decay_, 0))
        {
            delta = bias_decay_*gradientdata[0];
            for(int j=0; j<n; j++)
                if(varray.last()->valuedata[j] > 0)
                    varray.last()->gradientdata[j] += delta;
                else if(varray.last()->valuedata[j] < 0)
                    varray.last()->gradientdata[j] -= delta;
        }

        delta = weight_decay_*gradientdata[0];
        for(int i=1; i<l; i++)
        {
            if(input_is_discrete[i-1])
            {
                int r = (int)varray[0]->valuedata[i-1];
                real* row = varray[i]->matValue.row(r).data();
                real* grow = varray[i]->matGradient.row(r).data();
                for(int j=0; j<n; j++)
                {
                    if(row[j] > 0)
                        grow[j] += delta;
                    else if( row[j] < 0)
                        grow[j] -= delta;
                }
                varray[i]->updateRow(r);
            }
            else
            {
                for(int j=0; j<n; j++)
                    if(varray[i]->valuedata[j] > 0)
                        varray[i]->gradientdata[j] += delta;
                    else if(varray[i]->valuedata[j] < 0)
                        varray[i]->gradientdata[j] -= delta;
            }
        }
    }
    else if (penalty_type_ == "L2_square")
    {
        if(!fast_exact_is_equal(bias_decay_, 0))
        {
            for(int j=0; j<n; j++)
                varray.last()->gradientdata[j] += 2*bias_decay_*varray.last()->valuedata[j]*gradientdata[0];
        }

        for(int i=1; i<l; i++)
        {
            if(input_is_discrete[i-1])
            {
                int r = (int)varray[0]->valuedata[i-1];
                real* row = varray[i]->matValue.row((int)varray[0]->valuedata[i-1]).data();
                real* grow = varray[i]->matGradient.row((int)varray[0]->valuedata[i-1]).data();
                for(int j=0; j<n; j++)
                {
                    grow[j] += 2*weight_decay_*row[j]*gradientdata[0];
                }
                varray[i]->updateRow(r);
            }
            else
            {
                for(int j=0; j<n; j++)
                    varray[i]->gradientdata[j] += 2*weight_decay_*varray[i]->valuedata[j]*gradientdata[0];
            }
        }
    }

}

void HeterogenuousAffineTransformWeightPenalty::build()
{
    inherited::build();
    build_();
}

void HeterogenuousAffineTransformWeightPenalty::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(input_is_discrete, copies);
    //PLERROR("HeterogenuousAffineTransformWeightPenalty::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void HeterogenuousAffineTransformWeightPenalty::declareOptions(OptionList& ol)
{    
    declareOption(ol, "input_is_discrete", &HeterogenuousAffineTransformWeightPenalty::input_is_discrete,
                  OptionBase::buildoption,
                  "Indication whether each component of the input is discrete or not.");
    declareOption(ol, "weight_decay_", &HeterogenuousAffineTransformWeightPenalty::weight_decay_,
                  OptionBase::buildoption,
                  "Weight decay parameter.");
    declareOption(ol, "bias_decay_", &HeterogenuousAffineTransformWeightPenalty::bias_decay_,
                  OptionBase::buildoption,
                  "Bias decay parameter.");
    declareOption(ol, "penalty_type_", &HeterogenuousAffineTransformWeightPenalty::penalty_type_,
                  OptionBase::buildoption,
                  "Penalty to use on the weights.\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L1_square\": square of the L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void HeterogenuousAffineTransformWeightPenalty::build_()
{
    if(varray[0]->size() != varray.length()-2)
        PLERROR("In HeterogenuousAffineTransformWeightPenalty::build_(): The number of weight variables (%d) and input size (%d) is not the same", varray.length()-2, varray[0]->size());
    if(input_is_discrete.length() != varray[0]->size())
        PLERROR("In HeterogenuousAffineTransformWeightPenalty::build_(): input_is_discrete size (%d) and input size (%d) does not match", input_is_discrete.length(), varray[0]->size());
    if(!varray[0]->isVec())
        PLERROR("In HeterogenuousAffineTransformWeightPenalty::build_(): input should be a vector");
    for(int i=1; i<varray.length(); i++)
    {
        if(varray[i]->width() != varray[1]->width())
            PLERROR("In HeterogenuousAffineTransformWeightPenalty::build_(): %dth weight matrix has width %d, should be %d", i, varray[i]->width(), size());
        if(i<varray.length()-1 && input_is_discrete[i-1])
            varray[i-1]->allowPartialUpdates();
    }

    string pt = lowerstring( penalty_type_ );
    if( pt == "l1" )
        penalty_type_ = "L1";
    else if( pt == "l1_square" || pt == "l1 square" || pt == "l1square" )
        penalty_type_ = "L1_square";
    else if( pt == "l2_square" || pt == "l2 square" || pt == "l2square" )
        penalty_type_ = "L2_square";
    else if( pt == "l2" )
    {
        PLWARNING("In HeterogenuousAffineTransformWeightPenalty::build_(): L2 penalty not supported, assuming you want L2 square");
        penalty_type_ = "L2_square";
        }
    else
        PLERROR("In HeterogenuousAffineTransformWeightPenalty::build_(): penalty_type_ \"%s\" not supported", penalty_type_.c_str());
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
