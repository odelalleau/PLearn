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

#include "NegLogProbCostFunction.h"
#include <plearn/math/TMat_maths.h>

#if USING_MPI
#include <plearn/sys/PLMPI.h>
#endif

namespace PLearn {
using namespace std;



PLEARN_IMPLEMENT_OBJECT(NegLogProbCostFunction, "ONE LINE DESCR", "NO HELP");


#define smoothmap sigmoid
real NegLogProbCostFunction::evaluate(const Vec& output, const Vec& target) const
{
    real prob = 0.;
    int desired_class = int(target[0]);
    if (desired_class == -1) desired_class=0;
    if(output.length()==1) // we assume output[0] gives the probability of having class 1 
    {
        prob = output[0];
        if(smooth_map_outputs)
            prob = smoothmap(prob);
        if(desired_class==0)
            prob = 1-prob;
    }
    else 
    {
        if(!normalize) // we assume output gives a real probability for each class
#if USING_MPI
#define SEND_PROB_TAG 981
        {
            // EACH CPU ONLY CARRIES THE CORRECT outputs IN THE INTERVAL
            // GIVEN BY [out_start,out_end).
            // THE RESULT WILL BE THAT CPU 0 WILL HAVE
            // THE PROBABILITY OF desired_class IN prob
            // (and the other CPUs will have some dummy value)
            if (PLMPI::size>1 && out_end>=0)
            {
                if (desired_class>=out_start && desired_class<out_end)
                {
                    prob = output[desired_class];
                    if (PLMPI::rank>0) // send it to CPU 0
                    {
                        MPI_Send(&prob,1,PLMPI_REAL,0,SEND_PROB_TAG,MPI_COMM_WORLD);
                    }
                }
                else
                {
                    if (PLMPI::rank==0)
                    {
                        MPI_Status status;
                        MPI_Recv(&prob,1,PLMPI_REAL,MPI_ANY_SOURCE,SEND_PROB_TAG,MPI_COMM_WORLD,&status);
                    }
                    else 
                    {
                        prob = 1; // dummy value (whose log exists)
                    }
                }
            }
            else
                prob = output[desired_class];
        }
#else
        prob = output[desired_class];
#endif
        else // outputs may not sum to 1, so we'll normalize them
        {
#if USING_MPI
            if (PLMPI::size>1 && out_end>=0)
                PLERROR("condprob used in parallel mode: normalize not implemented");
#endif
            real* outputdata = output.data();
            if(smooth_map_outputs) // outputs may be slightly smaller than 0 or slightly larger than 1
            {                      // so we'll smooth-map them to fit in range [0,1] before normalizing 
                real outputsum = 0.0;
                for(int i=0; i<output.length(); i++)
                    outputsum += smoothmap(outputdata[i]);
                prob = smoothmap(outputdata[desired_class])/outputsum;
            }
            else
                prob = output[desired_class]/sum(output, false);
        }
    }
    return -safeflog(prob);
}

void NegLogProbCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "normalize", &NegLogProbCostFunction::normalize, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "smooth_map_outputs", &NegLogProbCostFunction::smooth_map_outputs, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
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
