// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent
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


#ifndef EmpiricalDistribution_INC
#define EmpiricalDistribution_INC

#include "Distribution.h"

namespace PLearn {
using namespace std;


  class EmpiricalDistribution: public Distribution
  {
  protected:

    VMat data;
    
    typedef Distribution inherited;
    
  public:

    EmpiricalDistribution();
    
    EmpiricalDistribution(int inputsize, bool random_sample_ = true);

    PLEARN_DECLARE_OBJECT(EmpiricalDistribution);
    void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void train(VMat training_set);
  


    //Should return log of probability density log(p(x))
    //Not implemented for this distribution
    virtual double log_density(const Vec& x) const;
  
    //! return survival fn = P(X>x)
    virtual double survival_fn(const Vec& x) const;

    //! return survival fn = P(X<x)
    virtual double cdf(const Vec& x) const;

    //! return E[X] 
    virtual Vec expectation() const;

    //! return Var[X]
    virtual Mat variance() const;


    
    //! return a sample generated from the distribution.
    virtual void generate(Vec& x) const;

    //If true, generate return a random example. If false,
    //generate return the next example in the training_set. 
    bool random_sample;

    //The length of the current training set
    int length;

    //The example to be returned by generate
    mutable int current_sample_x;
    mutable int current_sample_y;
    mutable bool flip;
    
  protected:
    static void declareOptions(OptionList& ol);

  };

  DECLARE_OBJECT_PTR(EmpiricalDistribution);

} // end of namespace PLearn

#endif
