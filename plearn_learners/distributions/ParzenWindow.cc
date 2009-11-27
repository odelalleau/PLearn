// -*- C++ -*-

//
// Copyright (C) 2004-2005 University of Montreal
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


/*! \file ParzenWindow.cc */
#include "ParzenWindow.h"

//#include <plearn/math/plapack.h>
//#include <plearn/base/general.h>
//#include <plearn/math/TMat.h>
//#include <plearn/math/TMat_maths.h>
//#include <plearn/math/BottomNI.h>

namespace PLearn {

PLEARN_IMPLEMENT_OBJECT(ParzenWindow,
                        "Parzen Window density estimate.",
                        "Standard Parzen Window algorithm. The user only needs\n"
                        "to set the isotropic_sigma parameter"
    );

// TODO Allow the user to specify the kernel.

/////////////////////
// ParzenWindow //
/////////////////////
ParzenWindow::ParzenWindow()
    : isotropic_sigma(1)
{
    nstages = 1;
}

ParzenWindow::ParzenWindow(real the_isotropic_sigma)
    : isotropic_sigma(the_isotropic_sigma)
{
}

// ### Nothing to add here, simply calls build_
void ParzenWindow::build()
{
    inherited::build();
    build_();
}

// TODO Hide the options from GaussMix that are overwritten.
// TODO Yeah that's really needed!

////////////////////
// declareOptions //
////////////////////
void ParzenWindow::declareOptions(OptionList& ol)
{
    declareOption(ol,"isotropic_sigma", &ParzenWindow::isotropic_sigma, OptionBase::buildoption,
                  "Spherical standard deviation parameter (NOTE: old implementation called this sigma_square, but it really was sigma, hence the renaming)");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ParzenWindow::build_()
{}

void ParzenWindow::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("ParzenWindow::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void ParzenWindow::train()
{
    if(stage<1)
    {
        int l = train_set.length();
        type = "spherical";
        L = l;
        D = -1;
        GaussMix::build();  // rebuild because options chnged
        resizeDataBeforeTraining(); // TODO See exactly what this does.
    
        // new code proprly taking sample weights into account
        bool has_weights = train_set->hasWeights();
        real default_weight = 1.0/l;
        Vec target;    
        real weight = 0;
        real weight_sum = 0;
    
        for(int i=0; i<l; i++)
        {
            // if(i%100==0)
            //    cerr << "[SEQUENTIAL TRAIN: processing pattern #" << i << "/" << l << "]\n";            
            Vec input = center(i);
            train_set->getExample(i,input,target,weight);
            sigma[i] = isotropic_sigma;
            if(has_weights)
            {
                alpha[i] = weight;
                weight_sum += weight;
            }
            else
                alpha[i] = default_weight;
            // resizeStuffBeforeTraining(); TODO Put back?
        }
        if(has_weights)
            alpha /= weight_sum;
        GaussMix::build();
        
        stage = 1;
        // precomputeStuff(); TODO Put back?
        build(); // rebuild
    }
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
