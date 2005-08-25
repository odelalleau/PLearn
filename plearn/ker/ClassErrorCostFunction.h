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

#ifndef ClassErrorCostFunction_INC
#define ClassErrorCostFunction_INC

#include "Kernel.h"

namespace PLearn {
using namespace std;




/*!   * If output and target both have length 1, then binary classification 
  with targets -1 and +1 is assumed and the sign of the output is considered
  If ouput is MISSING_VALUE, evaluation returns MISSING_VALUE; MISSING_VALUE
  can be considered as an error if ignore_missing_value is set to false.
  * If output has length>1 and target has length 1, then output is understood 
  as giving a score for each class while target is the index of the correct
  class (numbered from 0)
  * If both output and target have a length>1 then then output is understood 
  as giving a score for each class while the correct class is given by 
  argmax(target)
  In any case, evaluation returns 0 if classification was correct, 1 otherwise
*/
class ClassErrorCostFunction: public Kernel
{
    typedef Kernel inherited;
  
protected:
    bool output_is_classnum;
    bool ignore_missing_values;

public:
/*!       There are several cases:
  1) target is a single value +1 or -1 , and output is a single value whose sign stands for the class
  2) target is a single value in 0..n-1 indicating classnumber and output is a n-dimensional vector of scores
  3) target is a n-dimensional vector whose argmax indicates the class, and output is a n-dimensional vector of scores
  4) target is a single value indicating classnumber, and output is a single value indicating classnumber
  5) target is a single value 0 or 1 , and output is a single value with the threshold 0.5
  Cases 1,2,3 are handled correctly with the default output_is_classnum=false
  For case 4 and 5, you must specify output_is_classnum=true
*/
    ClassErrorCostFunction(bool the_output_is_classnum = false,bool the_ignore_missing_values=true)
        :output_is_classnum(the_output_is_classnum),ignore_missing_values(the_ignore_missing_values) {}

    PLEARN_DECLARE_OBJECT(ClassErrorCostFunction);

    virtual string info() const
    { return "class_error"; }

    virtual real evaluate(const Vec& output, const Vec& target) const;

protected:
    //!  recognized option is "output_is_classnum"  
    static void declareOptions(OptionList &ol);
};

DECLARE_OBJECT_PTR(ClassErrorCostFunction);

inline CostFunc class_error(bool output_is_classnum=false,bool ignore_missing_values=true) 
{ 
    return new ClassErrorCostFunction(output_is_classnum, ignore_missing_values); 
} 


} // end of namespace PLearn

#endif


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
