// -*- C++ -*-

// OracleOracleObjectGenerator.h
//
// Copyright (C) 2004 ApSTAT Technologies Inc.
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

// Authors: Rejean Ducharme

/* *******************************************************
 * $Id$
 ******************************************************* */

#include <plearn/misc/ObjectGenerator.h>
#include "OptionsOracle.h"
#include <plearn/math/TVec.h>

#ifndef OracleObjectGenerator_INC
#define OracleObjectGenerator_INC

namespace PLearn {
using namespace std;

class OracleObjectGenerator: public ObjectGenerator
{
private:
    typedef ObjectGenerator inherited;

protected:

    //! The last parameters returned by the oracle
    TVec<string> last_params;

public:

    //! The template Object from which we will generate other Objects
    PP<OptionsOracle> oracle;
    //PP<CartesianProductOracle> oracle;


    // ******************
    // * Object methods *
    // ******************

private:
    void build_();

protected:
    static void declareOptions(OptionList& ol);

public:

    //! Default constructor
    OracleObjectGenerator();

    //! This will generate the next object in the list of all options
    //! MUST be define by a subclass
    virtual PP<Object> generateNextObject();

    //! This will generate a list of all possible Objects.
    //! By default, just loop over generateNextObject()
    //virtual TVec< PP<Object> > generateAllObjects();

    //! simply calls inherited::build() then build_()
    virtual void build();

    virtual void forget();

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(OracleObjectGenerator);
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(OracleObjectGenerator);

} // end of namespace PLearn

#endif // OracleObjectGenerator_INC


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
