
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

#include "Var_operators.h"

#include "PlusConstantVariable.h"
#include "PlusScalarVariable.h"
#include "PlusRowVariable.h"
#include "PlusColumnVariable.h"
#include "PlusVariable.h"

#include "MinusScalarVariable.h"
#include "NegateElementsVariable.h"
#include "MinusRowVariable.h"
#include "MinusColumnVariable.h"
#include "MinusVariable.h"

#include "TimesScalarVariable.h"
#include "TimesColumnVariable.h"
#include "TimesRowVariable.h"
#include "TimesVariable.h"

#include "InvertElementsVariable.h"
#include "DivVariable.h"

#include "EqualVariable.h"

namespace PLearn {
using namespace std;

Var operator+(Var v, real cte)
{ return new PlusConstantVariable(v,cte); }

Var operator+(real cte, Var v)
{ return new PlusConstantVariable(v,cte); }

Var operator-(Var v, real cte)
{ return new PlusConstantVariable(v,-cte); }

Var operator+(Var v1, Var v2)
{ 
    if(v2->isScalar())
        return new PlusScalarVariable(v1,v2);
    else if(v1->isScalar())
        return new PlusScalarVariable(v2,v1);
    else if(v2->isRowVec())
        return new PlusRowVariable(v1,v2);
    else if(v1->isRowVec())
        return new PlusRowVariable(v2,v1);
    else if(v2->isColumnVec())
        return new PlusColumnVariable(v1,v2);
    else if(v1->isColumnVec())
        return new PlusColumnVariable(v2,v1);
    else
        return new PlusVariable(v1,v2);
}

void operator+=(Var& v1, const Var& v2)
{
    if (!v2.isNull())
    {
        if (v1.isNull())
            v1 = v2;
        else 
            v1 = v1 + v2;
    }
}

Var operator-(Var v1, Var v2)
{ 
    if(v2->isScalar())
        return new MinusScalarVariable(v1,v2);
    else if(v1->isScalar())
        return new PlusScalarVariable(new NegateElementsVariable(v2), v1);
    else if(v2->isRowVec())
        return new MinusRowVariable(v1,v2);
    else if(v1->isRowVec())
        return new NegateElementsVariable(new MinusRowVariable(v2,v1));
    else if(v2->isColumnVec())
        return new MinusColumnVariable(v1,v2);
    else if(v1->isColumnVec())
        return new PlusColumnVariable(new NegateElementsVariable(v2), v1);
    else
        return new MinusVariable(v1,v2);
}

void operator-=(Var& v1, const Var& v2)
{
    if (!v2.isNull())
    {
        if (v1.isNull())
            v1 = -v2;
        else
            v1 = v1 - v2;
    }
}

Var operator-(real cte, Var v)
{ return new PlusConstantVariable(new NegateElementsVariable(v),cte); }


//!  element-wise multiplications
Var operator*(Var v1, Var v2)
{ 
    if(v2->isScalar())
        return new TimesScalarVariable(v1,v2);
    else if(v1->isScalar())
        return new TimesScalarVariable(v2,v1);
    else if(v2->isColumnVec())
        return new TimesColumnVariable(v1,v2);
    else if(v1->isColumnVec())
        return new TimesColumnVariable(v2,v1);
    else if(v2->isRowVec())
        return new TimesRowVariable(v1,v2);
    else if(v1->isRowVec())
        return new TimesRowVariable(v2,v1);
    else //!<  v1 and v2 must have the same dimensions (it is checked by the constructor of TimesVariable)
        return new TimesVariable(v1,v2); 
}

Var operator/(real cte, Var v)
{
    if(fast_exact_is_equal(cte, 1.0))
        return invertElements(v);
    else
        return cte*invertElements(v);
}

Var operator/(Var v1, Var v2)
{
    if(v1->length()==v2->length() && v1->width()==v2->width())
        return new DivVariable(v1,v2);
    else
        return v1*invertElements(v2);
}

Var operator==(Var v1, Var v2)
{ return isequal(v1,v2); }

Var operator!=(Var v1, Var v2)
{ return (1.0 - isequal(v1,v2) ); }

Var isdifferent(Var v1, Var v2)
{ return (1.0 - isequal(v1,v2) ); }


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
