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
   * $Id: Isomap.cc,v 1.2 2004/06/23 20:20:46 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file Isomap.cc */

#include "DistanceKernel.h"
#include "GeodesicDistanceKernel.h"
#include "Isomap.h"

namespace PLearn {
using namespace std;

////////////
// Isomap //
////////////
Isomap::Isomap() 
: geodesic_file(""),
  knn(10)
{
  kernel_is_distance = true;
  // Default distance kernel is the classical Euclidean distance.
  distance_kernel = new DistanceKernel(2);
}

PLEARN_IMPLEMENT_OBJECT(Isomap,
    "Performs ISOMAP dimensionality reduction.",
    ""
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

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  // Modify some options from KernelPCA so as to hide them.
  redeclareOption(ol, "kernel_is_distance", &KernelPCA::kernel_is_distance, OptionBase::nosave,
      "In ISOMAP, the kernel is always a distance");

  redeclareOption(ol, "kernel", &KernelPCA::kpca_kernel, OptionBase::learntoption,
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
    this->kpca_kernel = new GeodesicDistanceKernel(distance_kernel, knn, geodesic_file, true);
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
void Isomap::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("Isomap::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn

