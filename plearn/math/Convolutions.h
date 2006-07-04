// -*- C++ -*-
// Convolutions.h

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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

/*! \file plearn/math/Convolutions.h */

#ifndef MODULE_CONVOLUTIONS
#define MODULE_CONVOLUTIONS

#include <plearn/math/TMat.h>

namespace PLearn {
using namespace std;

  //! Convolve a source signal of length NS with a kernel of length NK
  //! and put result in a destination signal which should be of length NS-NK+1.
  //! This is the same as convolve1D(source_signal, kernel, dest_signal, 1), i.e.
  //! the kernel window is stepped by one. The destination signal is
  //!    dest_signal[i] = sum_{j=0}^{NK-1} source_signal[i+j]*kernel[j]
  void convolve1D(const Vec& source_signal, const Vec& kernel, 
                  Vec& dest_signal)
  {
    int ns=source_signal.length();
    int nk=kernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    if (nd!=ns-nk+1)
      PLERROR("convolve1D: source_signal.length()[%d]-kernel.length()[%d]+1 should equal dest_signal.length()[%d]\n",
              ns,nk,nd);
#endif
    real* s=source_signal.data();
    real* k=kernel.data();
    real* d=dest_signal.data();
    for (int i=0;i<nd;i++,s++)
      {
        real somme=0;
        for (int j=0;j<nk;j++)
          somme += s[j]*k[j];
        d[i]=somme;
      }
  }

  //! Convolve a source signal of length NS with a kernel of length NK with steps S,
  //! and put result in a destination signal which should be of length NS-NK+1.
  //! The destination signal is
  //!    dest_signal[i] = sum_{j=0}^{NK-1} source_signal[i*step+j]*kernel[j]
  void convolve1D(const Vec& source_signal, const Vec& kernel, 
                  Vec& dest_signal, int step)
  {
    int ns=source_signal.length();
    int nk=kernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    if (step<1)
      PLERROR("convolve1D: step[%d] should be a positive integer\n",step);
    if (ns!=nd*step+nk-1)
      PLERROR("convolve1D: source_signal.length()[%d] should equal step[%d] times dest_signal.length()[%d] + kernel.length()[%d] - step\n",
              ns,step,nd,nk);
#endif
    real* s=source_signal.data();
    real* k=kernel.data();
    real* d=dest_signal.data();
    for (int i=0;i<nd;i++,s+=step)
      {
        real somme=0;
        for (int j=0;j<nk;j++)
          somme += s[j]*k[j];
        d[i]=somme;
      }
  }

  //! Back-convolve INTO a "source" signal of length NS with a kernel of length NK
  //! and FROM a "destination" signal which should be of length NS-NK+1.
  //! This is EXACTLY the TRANSPOSE operation of a convolve1D with the same
  //! argument, with computations flowing in the other direction.
  //! This is the same as backConvolve1D(source_signal, kernel, dest_signal, 1), i.e.
  //! the kernel window is stepped by one. The source signal is set with
  //!    source_signal[k] = sum_{j=0}^{NK-1} 1_{k>=j && k-j<ND} dest_signal[k-j]*kernel[j]
  //! If the accumulate flag is set, then source_signal is incremented rather than set.
  //! N.B. THIS IS THE SAME AS COMPUTING dC/dsource_signal (into the source_signal argument),
  //! GIVEN dC/ddest_signal, i.e. this function does part of the work done by convolve1Dbackprop
  //! and convolve1DbackpropUpdate (it does not compute the gradient with respect to the kernel).
  void backConvolve1D(const Vec& source_signal, const Vec& kernel, 
                      Vec& dest_signal, bool accumulate=true)
  {
    int ns=source_signal.length();
    int nk=kernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    if (nd!=ns-nk+1)
      PLERROR("backConvolve1D: source_signal.length()[%d]-kernel.length()[%d]+1 should equal dest_signal.length()[%d]\n",
              ns,nk,nd);
#endif
    if (!accumulate)
        dest_signal.clear();
    real* s=source_signal.data();
    real* k=kernel.data();
    real* d=dest_signal.data();
    for (int i=0;i<nd;i++,s++)
      {
  //!    source_signal[k] = sum_{j=0}^{NK-1} 1_{k>=j && k-j<ND} dest_signal[k-j]*kernel[j], i.e.
  //! letting k=i+j, k-j<nd means i<nd, 
  //! for i=0 to nd-1:
  //!   for j=0 to nk-1:
  //!    source_signal[i+j] += dest_signal[i]*kernel[j]
        real di=d[i];
        for (int j=0;j<nk;j++)
          s[j] += di*k[j];
      }
  }


  //! Increment dC/dsource_signal and dC/dkernel, given dC/ddest_signal, with dest_signal computed
  //! as per convolve1D(source_signal, kernel, dest_signal):
  //!    dC/dsource_signal[k] += sum_{j=0}^{NK-1} 1_{k>=j && k-j<ND} dC_ddest_signal[k-j]*kernel[j]
  //!    dC/dkernel[j] += sum_{k=0}^{ND-1} 1_{k>=j && k-j<ND} dC_ddest_signal[k-j]*source_signal[k]
  //! (consider the equivalence: k = i+j)
  void convolve1Dbackprop(const Vec& source_signal, const Vec& kernel, 
                          const Vec& dC_ddest_signal, 
                          Vec& dC_dsource_signal, Vec& dC_dkernel)
  {
    int ns=source_signal.length();
    int nk=kernel.length();
    int nd=dC_ddest_signal.length();
#ifdef BOUNDCHECK
    if (nd!=ns-nk+1)
      PLERROR("convolve1Dbackprop: source_signal.length()[%d]-kernel.length()[%d]+1 should equal dest_signal.length()[%d]\n",
              ns,nk,nd);
    if (dC_dsource_signal.length()!=ns)
      PLERROR("convolve1Dbackprop: source_signal.length()[%d] should equal dC_dsource_signal.length()[%d]\n",
              ns,dC_dsource_signal.length());
    if (dC_dkernel.length()!=nk)
      PLERROR("convolve1Dbackprop: kernel.length()[%d] should equal dC_dkernel.length()[%d]\n",
              nk,dC_dkernel.length());
#endif
    dest_signal.clear();
    real* s=source_signal.data();
    real* dCds=source_signal.data();
    real* k=kernel.data();
    real* dCdk=dC_dkernel.data();
    real* dCdd=dC_ddest_signal.data();
    for (int i=0;i<nd;i++,dCds++,s++)
      {
  //! for i=0 to nd-1:
  //!   for j=0 to nk-1:
  //!     dC_dsource_signal[i+j] += dC_ddest_signal[i]*kernel[j]
  //!     dC_dkernel[j] += dC_ddest_signal[i]*source_signal[i+j]
        real di=dCdd[i];
        for (int j=0;j<nk;j++)
        {
          dCds[j] += di*k[j];
          dCdk[j] += di*s[j];
        }
      }
  }


  //! Convolve a (N1S x N2S) source image with a (N1K x N2K) kernel matrix,
  //! and put result in a destination matrix of dimensions (N1D x N2D), with NiS = NiD + NiK - 1.
  //! This is the same as convolve2D(source_image, kernel, dest_image, 1, 1), i.e.
  //! the kernel window is stepped by one in both directions. The destination image is
  //!    dest_image[i,j] = sum_{k1=0}^{N1K-1} sum_{k2=0}^{N2K-1} source_image[i+k1,j+k2]*kernel[k1,k2]
  void convolve2D(const Mat& source_image, const Mat& kernel, 
                  Mat& dest_image)
  {
    int n1s=source_image.length();
    int n2s=source_image.width();
    int n1k=kernel.length();
    int n2k=kernel.width();
    int n1d=dest_image.length();
    int n2d=dest_image.width();
#ifdef BOUNDCHECK
    if (n1d!=n1s-n1k+1)
      PLERROR("convolve2D: source_image.length()[%d]-kernel.length()[%d]+1 should equal dest_image.length()[%d]\n",
              n1s,n1k,n1d);
    if (n2d!=n2s-n2k+1)
      PLERROR("convolve2D: source_image.width()[%d]-kernel.width()[%d]+1 should equal dest_image.width()[%d]\n",
              n2s,n2k,n2d);
#endif
    int sm = source_image.mod();
    real* s = source_image.data();
    for (int i=0;i<n1d;i+,s+=sm)
      {
        real* d = dest_image(i);
        for (int j=0;i<n2d;j+)
          {
            real somme=0;
            real* s1=&s[j];
            for (int k1=0;k1<n1k;k1++,s1+=sm)
              {
                real* k=kernel(k1);
                for (int k2=0;k2<n2k;k2++)
                  somme += s1[k2]*k[k2];
              }
            d[j]=somme;
          }
      }
  }

} // end of namespace PLearn

#endif //!<  MODULE_CONVOLUTIONS


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
