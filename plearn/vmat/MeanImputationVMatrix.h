// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003 Olivier Delalleau
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


/* ******************************************************************
 * $Id: MeanImputationVMatrix.h 3658 2005-07-06 20:30:15  Godbout $
 ****************************************************************** */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef MeanImputationVMatrix_INC
#define MeanImputationVMatrix_INC

#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

//!  provides mean imputation for missing variables

class MeanImputationVMatrix: public SourceVMatrix
{
    typedef SourceVMatrix inherited;

private:

    bool obtained_inputsize_from_source;
    bool obtained_targetsize_from_source;
    bool obtained_weightsize_from_source;

public:

    TVec<real> variable_mean;
    TVec<int>  variable_present_count;
    real       number_of_train_samples;

public:

    MeanImputationVMatrix();
    MeanImputationVMatrix(VMat the_source, real the_number_of_train_samples = 0.0);
    virtual            ~MeanImputationVMatrix();

    static void        declareOptions(OptionList &ol);

    virtual void       build();
    virtual void       makeDeepCopyFromShallowCopy(CopiesMap& copies);

public:

    virtual void       getExample(int i, Vec& input, Vec& target, real& weight);
    virtual real       get(int i, int j) const;
    virtual void       put(int i, int j, real value);
    virtual void       getSubRow(int i, int j, Vec v) const;
    virtual void       putSubRow(int i, int j, Vec v);
    virtual void       appendRow(Vec v);
    virtual void       insertRow(int i, Vec v);
    virtual void       getRow(int i, Vec v) const;
    virtual void       putRow(int i, Vec v);
    virtual void       getColumn(int i, Vec v) const;
    TVec<real> getMeanVector();

private:

    void       build_();
    void       computeMeanVector();

    PLEARN_DECLARE_OBJECT(MeanImputationVMatrix);

};

DECLARE_OBJECT_PTR(MeanImputationVMatrix);

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
