// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Christopher Kermorvant
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

/*! \file ProbVector.cc */

#include "ProbVector.h"

namespace PLearn {

void ProbVector::smoothNormalize(string name,real discounting_value)
{
  // smooth by discounting  and Normalize

#ifdef DEBUG  
  cout << "Smoothing "<< name;
#endif
  bool err_smooth_flag=false;
  int f;
  int size =  length_;
  real *v = data();
  real word_seen=0;
  real sum_discounted=0.0;
  real sum_v = 0;

  for(f=0;f<size;f++){
    if (v[f]>discounting_value){
      sum_v+=v[f];
      v[f]-=discounting_value;
      sum_discounted +=discounting_value;
      word_seen++;
      continue;
    }
    // in the case of non integer counts (typically during E.M. algo)
    if (v[f]>0 && v[f]<discounting_value){
      sum_v+=v[f];
      err_smooth_flag=true;
    }
  }
#ifdef DEBUG  
  cout << ": discounted " << sum_discounted << " from " << word_seen << " seen events summing " <<sum_v ;
#endif
  // distribute discounted mass
  real unseen_prob=sum_discounted/(sum_v*size);
  word_seen=0;
  for(f=0;f<size;f++){
    // Distribute on both seen and unseen events
    //if (v[f]<=discounting_value){
      word_seen++;
      v[f]/=sum_v;
      v[f]+= unseen_prob;
      // }else{
      //       v[f]/=sum_v;
      //     }
  }
#ifdef DEBUG  
  cout << " redistribute " << unseen_prob << " to "<< word_seen << " unseen events" <<endl;
#endif
  if(err_smooth_flag)PLWARNING("minimal value < discounted value in Backoff  Smoothing smoothNormalize a probVector");
}

void ProbVector::normalize()
{
  // Normalize the vector (sum = 1)
  int f;
  int size =  length_;
  real *v = data();
  real sum_v = 0;

  for(f=0;f<size;f++){
    sum_v+=v[f];
  }
  for(f=0;f<size;f++){
    v[f]/=sum_v;
  }
}

}
