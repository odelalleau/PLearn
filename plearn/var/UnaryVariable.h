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

#ifndef UnaryVariable_INC
#define UnaryVariable_INC

#include "VarArray.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

class UnaryVariable: public Variable
{
public:
    typedef Variable inherited;

public:

    //!  Default constructor for persistence
    UnaryVariable() {}

protected:
    static void declareOptions(OptionList & ol);

protected:
    Var input;

public:
    UnaryVariable(Variable* v, int thelength, int thewidth);

    PLEARN_DECLARE_OBJECT(UnaryVariable);
  
    //! Set this Variable's input (simply call build after setting the new
    //! input).
    void setInput(Var the_input);
  
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual bool markPath();
    virtual void buildPath(VarArray& proppath);
    virtual VarArray sources();
    virtual VarArray random_sources();
    virtual VarArray ancestors();
    virtual void unmarkAncestors();
    virtual void fprop() {} //!< Nothing to do by default.
    virtual void bprop() {} //!< Nothing to do by default.
    virtual VarArray parents();
    void printInfo(bool print_gradient) 
    { 
        pout << getName() << "[" << (void*)this << "] " << *this
             << " (" << (void*)input << ") = " << value;
        if (print_gradient) 
            pout << " gradient=" << gradient;
        pout << endl; 
    }
    virtual void resizeRValue();
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(UnaryVariable);

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
