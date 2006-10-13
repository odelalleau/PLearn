// -*- C++ -*-
// convolutions.h

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of
// Montreal
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
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file plearn/math/convolutions.h */

#ifndef convolutions_INC
#define convolutions_INC

#include <plearn/math/TMat.h>

namespace PLearn {

//! Convolve a source signal of length NS with a kernel of length NK
//! with steps S, and put result in a destination signal which should
//! be of length NS-NK+1.
//! The destination signal is
//!    dest_signal[i] = sum_{j=0}^{NK-1} source_signal[i*step+j]*kernel[j]
void convolve1D(const Vec& source_signal, const Vec& kernel,
                const Vec& dest_signal, int step=1, bool accumulate=true);


//! Back-convolve INTO a "source" signal of length NS with a kernel of length
//! NK and FROM a "destination" signal which should be of length NS-NK+1
//! This is EXACTLY the TRANSPOSE operation of a convolve1D with the same
//! arguments, with computations flowing in the other direction.
//! for i=0 to nd-1:
//!   for j=0 to nk-1:
//!    source_signal[i*step+j] += dest_signal[i]*kernel[j]
//! If the accumulate flag is not set, then source_signal is first cleared.
//! N.B. THIS IS THE SAME AS COMPUTING dC/dsource_signal (into the
//! source_signal argument), GIVEN dC/ddest_signal, i.e. this function
//! does part of the work done by convolve1Dbackprop.
void backConvolve1D(const Vec& source_signal, const Vec& kernel,
                    const Vec& dest_signal, int step=1, bool accumulate=true);


//! Increment dC/dsource_signal and dC/dkernel, given dC/ddest_signal, with
//! dest_signal computed as per convolve1D(source_signal, kernel, dest_signal):
//!    dC/dsource_signal[k] += sum_{j=0}^{NK-1} 1_{k>=j && k-j<ND} dC_ddest_signal[k-j]*kernel[j]
//!    dC/dkernel[j] += sum_{k=0}^{ND-1} 1_{k>=j && k-j<ND} dC_ddest_signal[k-j]*source_signal[k]
//! (consider the equivalence: k = i+j)
void convolve1Dbackprop(const Vec& source_signal, const Vec& kernel,
                        const Vec& dC_ddest_signal,
                        const Vec& dC_dsource_signal, const Vec& dC_dkernel,
                        int step=1, bool accumulate=true);


//! Same as above, but increments only dC/dkernel, not dC/dsource_signal
//!    dC/dkernel[j] += sum_{k=0}^{ND-1} 1_{k>=j && k-j<ND} dC_ddest_signal[k-j]*source_signal[k]
//! (consider the equivalence: k = i+j)
void convolve1Dbackprop(const Vec& source_signal,
                        const Vec& dC_ddest_signal,
                        const Vec& dC_dkernel,
                        int step=1, bool accumulate=true);


//! Increment dC/ddest_signal and dC/dkernel, given dC/ddest_signal, with
//! source_signal computed as per
//! backConvolve1D(source_signal, kernel, dest_signal):
//!    dC/ddest_signal[i] += sum_{j=0}^{NK-1} dC_dsource_signal[i+j]*kernel[j]
//!    dC/dkernel[j] += sum_{i=0}^{ND-1} dC_dsource_signal[i+j]*dest_signal[i]
void backConvolve1Dbackprop(const Vec& kernel, const Vec& dest_signal,
                            const Vec& dC_ddest_signal,
                            const Vec& dC_dsource_signal,
                            const Vec& dC_dkernel,
                            int step=1, bool accumulate=true);


//! Same as above, but increments only dC/dkernel, not  dC/ddest_signal
//!    dC/dkernel[j] += sum_{i=0}^{ND-1} dC_dsource_signal[i+j]*dest_signal[i]
void backConvolve1Dbackprop(const Vec& dest_signal,
                            const Vec& dC_dsource_signal,
                            const Vec& dC_dkernel,
                            int step=1, bool accumulate=true);


//! Convolve a (N1S x N2S) source image with a (N1K x N2K) kernel matrix,
//! and put result in a destination matrix of dimensions (N1D x N2D), stepping
//! by (step1,step2) in each direction, with NiS = NiD*stepi + NiK - 1.
//! The destination image is
//!    dest_image[i,j] = 
//!      sum_{k1=0}^{N1K-1} sum_{k2=0}^{N2K-1} source_image[i*step1+k1,j*step2+k2]*kernel[k1,k2]
void convolve2D(const Mat& source_image, const Mat& kernel,
                const Mat& dest_image,
                int step1=1, int step2=1, bool accumulate=true);


//! Back-convolve INTO a (N1S x N2S) "source" image with a (N1K x N2K)
//! kernel matrix, and FROM a "destination" image of dimensions (N1D x
//! N2D), with NiS = NiD + NiK - 1.
//! This is EXACTLY the TRANSPOSE of convolve2D(source_image, kernel,
//! dest_image, 1, 1) with the same arguments, computations flowing in
//! the other direction.
//! The kernel window is stepped by one in both directions. The
//! destination image is
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     source_image[i1+j1,i2+j2] += dest_image[i1,i2]*kernel[j1,j2]
//! If the accumulate flag is not set, then source_image is first cleared.
//! N.B. When dest_image has been computed from kernel and source_image
//! using convolve2D, THIS IS THE SAME AS COMPUTING dC/dsource_image
//! (into the source_image argument), GIVEN dC/ddest_image, i.e.
//! this function does part of the work done by convolve2Dbackprop.
void backConvolve2D(const Mat& source_image, const Mat& kernel,
                    const Mat& dest_image,
                    int step1=1, int step2=1, bool accumulate=true);


//! Increment dC/dsource_image and dC/dkernel, given dC/ddest_image, with
//! dest_image computed as per convolve2D(source_image, kernel, dest_image):
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     dC/dsource_image[i1+j1,i2+j2] += dC/dest_image[i1,i2]*kernel[j1,j2]
//!     dC/dkernel[j1,j2] += dC/dest_image[i1,i2]*source_image[i1+j1,i2+j2]
void convolve2Dbackprop(const Mat& source_image, const Mat& kernel,
                        const Mat& dC_ddest_image,
                        const Mat& dC_dsource_image, const Mat& dC_dkernel,
                        int step1=1, int step2=1, bool accumulate=true);


//! As above, but increments only dC/dkernel, not dC/dsource_image
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     dC/dkernel[j1,j2] += dC/dest_image[i1,i2]*source_image[i1+j1,i2+j2]
void convolve2Dbackprop(const Mat& source_image,
                        const Mat& dC_ddest_image,
                        const Mat& dC_dkernel,
                        int step1=1, int step2=1, bool accumulate=true);


//! Increment dC/ddest_image and dC/dkernel, given dC/dsource_image, with
//! source_image computed as per
//! backConvolve2D(source_image, kernel, dest_image):
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     dC/ddest_image[i1,i2] += dC/dsource_image[i1+j1,i2+j2]*kernel[j1,j2]
//!     dC/dkernel[j1,j2] += dC/dsource_image[i1+j1,i2+j2]*dest_image[i1,i2]
void backConvolve2Dbackprop(const Mat& kernel, const Mat& dest_image,
                            const Mat& dC_ddest_image,
                            const Mat& dC_dsource_image, const Mat& dC_dkernel,
                            int step1=1, int step2=1, bool accumulate=true);


//! As above, but increments only dC/dkernel, not dC/ddest_image
//! for i1=0 to N1D-1:
//!  for i2=0 to N2D-1:
//!   for j1=0 to N1K-1:
//!    for j2=0 to N2K-1:
//!     dC/dkernel[j1,j2] += dC/dsource_image[i1+j1,i2+j2]*dest_image[i1,i2]
void backConvolve2Dbackprop(const Mat& dest_image,
                            const Mat& dC_dsource_image,
                            const Mat& dC_dkernel,
                            int step1=1, int step2=1, bool accumulate=true);

} // end of namespace PLearn

#endif //!<  convolutions_INC


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
