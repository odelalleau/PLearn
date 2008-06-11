// -*- C++ -*-

// Isomap.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file Isomap.cc */

#include <plearn/ker/DistanceKernel.h>
#include <plearn/ker/GeodesicDistanceKernel.h>
#include "Isomap.h"

namespace PLearn {
using namespace std;

////////////
// Isomap //
////////////
Isomap::Isomap() 
    : geodesic_file(""),
      geodesic_method("dijkstra"),
      knn(10)
{
    kernel_is_distance = true;
    // Default distance kernel is the classical Euclidean distance.
    distance_kernel = new DistanceKernel(2);
    min_eigenvalue = 0;
}

PLEARN_IMPLEMENT_OBJECT(Isomap,
                        "Performs ISOMAP dimensionality reduction.",
                        "Be careful that when looking for the 'knn' nearest neighbors of a point x,\n"
                        "we consider all points from the training data D, including x itself if it\n"
                        "belongs to D. Thus, to obtain the same result as with the classical ISOMAP\n"
                        "algorithm, one should use one more neighbor.\n"
                        "Note also that when used out-of-sample, this will result in a different output\n"
                        "than an algorithm applying the same formula, but considering one less neighbor.\n"
    );

////////////////////
// declareOptions //
////////////////////
void Isomap::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // declareOption(ol, "myoption", &Isomap::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(ol, "knn", &Isomap::knn, OptionBase::buildoption,
                  "The number of nearest neighbors considered.");

    declareOption(ol, "distance_kernel", &Isomap::distance_kernel, OptionBase::buildoption,
                  "The kernel used to compute the input space distances.");

    declareOption(ol, "geodesic_file", &Isomap::geodesic_file, OptionBase::buildoption,
                  "If provided, the geodesic distances will be saved in this file in binary format.");

    declareOption(ol, "geodesic_method", &Isomap::geodesic_method, OptionBase::buildoption,
                  "'floyd' or 'djikstra': the method to compute the geodesic distances."
		  "(cf. GeodesicDistanceKernel)");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Modify some options from KernelPCA so as to hide them.
    redeclareOption(ol, "kernel_is_distance", &Isomap::kernel_is_distance, OptionBase::nosave,
                    "In ISOMAP, the kernel is always a distance");

    redeclareOption(ol, "kernel", &Isomap::kpca_kernel, OptionBase::learntoption,
                    "The underlying KPCA kernel is now obtained from 'distance_kernel'.");

}

///////////
// build //
///////////
void Isomap::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void Isomap::build_()
{
    // Obtain the "real" KPCA kernel by computing the geodesic distances from
    // the 'distance_kernel'.
    // We have to do this iff:
    // 1. A 'distance_kernel' is provided, and
    // 2. either:
    //    2.a. the 'kpca_kernel' field is not set, or
    //    2.b. the 'kpca_kernel' field is not a GeodesicDistanceKernel acting on 'distance_kernel'.
    // This is to ensure that a loaded 'kpca_kernel' won't be overwritten.
    if (distance_kernel &&
        (!kpca_kernel ||
         (dynamic_cast<GeodesicDistanceKernel*>((Kernel*) kpca_kernel))->distance_kernel != distance_kernel)) {
        this->kpca_kernel = new GeodesicDistanceKernel(distance_kernel, knn, geodesic_file, true, geodesic_method);
        // We have modified the KPCA kernel, we must rebuild the KPCA.
        inherited::build();
    }
    if (kpca_kernel)
        kpca_kernel->report_progress = report_progress;
}

////////////
// forget //
////////////
void Isomap::forget()
{
    inherited::forget();
}
    
/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void Isomap::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(distance_kernel, copies);
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
