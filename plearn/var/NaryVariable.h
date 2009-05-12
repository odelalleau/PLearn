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

#ifndef NaryVariable_INC
#define NaryVariable_INC

#include "VarArray.h"
#include <plearn/math/TMat.h>
#include <plearn/vmat/VMat.h>
#include "Func.h"

// norman: multi threading unix standard not supported in win32
#ifndef WIN32
#include <plearn/sys/Popen.h>
#endif

namespace PLearn {
using namespace std;

class NaryVariable: public Variable
{
    typedef Variable inherited;

public:
    //#####  Public Build Options  ############################################

    VarArray varray;

public:
    //#####  PLearn::Object Interface  ########################################

    NaryVariable() {}
    NaryVariable(const VarArray& the_varray, int thelength, int thewidth = 1,
                 bool call_build_ = true);

    PLEARN_DECLARE_ABSTRACT_OBJECT(NaryVariable);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void build();
    
    //#####  PLearn::Variable Interface  ######################################

    virtual void setParents(const VarArray& parents);

    virtual bool markPath();
    virtual void buildPath(VarArray& proppath);
    virtual VarArray sources();
    virtual VarArray random_sources();
    virtual VarArray ancestors();
    virtual void unmarkAncestors();
    virtual VarArray parents();
    void printInfo(bool print_gradient) { 
        pout <<  getName() << "[" << (void*)this << "] " << classname() << "(" << (void*)varray[0];
        for (int i=1;i<varray.size();i++)
            pout << "," << (void*)varray[i];
        pout << ") = " << value;
        if (print_gradient) pout << " gradient=" << gradient;
        pout << endl; 
    }
    virtual void resizeRValue();

protected:
    //!  Default constructor for persistence
    static void declareOptions(OptionList & ol);

private:

    void build_();
};


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
