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
   * $Id: Var_utils.cc,v 1.1 2003/12/16 17:44:52 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "Var_utils.h"
#include "Var_all.h"
#include "Var_operators.h"

namespace PLearn <%
using namespace std;

Var mean(Var v)
{ return sum(v)/real(v->nelems()); }

Var neg_log_pi(Var p, Var index)
{
  if(index->isScalar())  return -log(p[index]);
  else return -log(matrixIndex(p,index));
}

Var softmax(Var input, int index)
{ 
  return 1.0/sum(exp(input-input[index])); //!<  should be numerically more stable
}

Var pownorm(Var input, real n)
{
  if(n==2.0)
    return sum(square(input));
  else if(n==1.0)
    return sum(abs(input));
  else
    return sum(pow(abs(input),n));
}

Var norm(Var input, real n)
{
  if(n==2.0)
    return sqrt(sum(square(input)));
  else if(n==1.0)
    return sum(abs(input));
  else
    return pow(sum(pow(abs(input),n)),1.0/n);
}

Var entropy(Var v, bool normalize)
{
  if(normalize)
    {
      Var absx = abs(v);
      Var normalized = absx/sum(absx);
      return sum(plogp(normalized))*(-1.0/log(2.0));
    }
  else
    return sum(plogp(v))*(-1.0/log(2.0));
}

Var distance(Var input1, Var input2, real n)
{ return norm(input1-input2, n); }

Var powdistance(Var input1, Var input2, real n)
{ return pownorm(input1-input2, n); }



%> // end of namespace PLearn


