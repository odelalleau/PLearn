// -*- C++ -*-

// BinaryOpVMatrix.cc
//
// Copyright (C) 2005 Nicolas Chapados
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
 ******************************************************* */

// Authors: Nicolas Chapados

/*! \file BinaryOpVMatrix.cc */


#include "BinaryOpVMatrix.h"

namespace PLearn {
using namespace std;


BinaryOpVMatrix::BinaryOpVMatrix()
{
}

PLEARN_IMPLEMENT_OBJECT(BinaryOpVMatrix,
                        "This VMat allows simple binary operations on two VMatrix.",
                        "It is assumed that the two source matrices are the same size"
    );

void BinaryOpVMatrix::getNewRow(int i, const Vec& v) const
{
    assert( source1 && source2 );
    row1.resize(source1.width());
    row2.resize(source2.width());
    assert( row1.size() == row2.size() );
    assert( v.size()    == row1.size() );

    source1->getRow(i, row1);
    source2->getRow(i, row2);

    for (int i=0, n=row1.size() ; i<n ; ++i)
        v[i] = selected_op(row1[i], row2[i]);
}

void BinaryOpVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "source1", &BinaryOpVMatrix::source1,
                  OptionBase::buildoption,
                  "First source VMatrix to operate on");

    declareOption(ol, "source2", &BinaryOpVMatrix::source2,
                  OptionBase::buildoption,
                  "Second source VMatrix to operate on");

    declareOption(ol, "op", &BinaryOpVMatrix::op, OptionBase::buildoption,
                  "Operation to perform; may be \"add\", \"sub\", \"mult\", \"div\"");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void BinaryOpVMatrix::build_()
{
    if (! source1)
        PLERROR("BinaryOpVMatrix::build_: source1 not defined");
    if (! source2)
        PLERROR("BinaryOpVMatrix::build_: source2 not defined");
    if (source1.length() != source2.length())
        PLERROR("BinaryOpVMatrix::build_: source1 has %d rows but\n"
                "source2 has %d rows; both must have the same number of"
                " rows.\n", source1.length(), source2.length());

    if (source1.width() != source2.width())
        PLERROR("BinaryOpVMatrix::build_: source1 has %d columns but\n"
                "source2 has %d columns; both must have the same number of"
                " columns.", source1.width(), source2.width());

    if (op == "add")
        selected_op = op_add;
    else if (op == "sub")
        selected_op = op_sub;
    else if (op == "mult")
        selected_op = op_mul;
    else if (op == "div")
        selected_op = op_div;
    else
        PLERROR("BinaryOpVMatrix::build_: unknown operation type \"%s\"; supported operatrions "
                "are \"add\", \"sub\", \"mult\", \"div\"", op.c_str());

    // Copy the metainformation from first VMat
    setMetaInfoFrom(source1);
}

// ### Nothing to add here, simply calls build_
void BinaryOpVMatrix::build()
{
    inherited::build();
    build_();
}

void BinaryOpVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(row1, copies);
    deepCopyField(row2, copies);
    deepCopyField(source1, copies);
    deepCopyField(source2, copies);
}

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
