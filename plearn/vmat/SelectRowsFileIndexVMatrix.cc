// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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

#include "SelectRowsFileIndexVMatrix.h"

namespace PLearn {
using namespace std;

/** SelectRowsFileIndexVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SelectRowsFileIndexVMatrix, "ONE LINE DESC", "NO HELP");

SelectRowsFileIndexVMatrix::SelectRowsFileIndexVMatrix()
{
}

SelectRowsFileIndexVMatrix::SelectRowsFileIndexVMatrix(VMat the_distr, const string& indexfile)
    : inherited(0,the_distr->width()), distr(the_distr), indices(indexfile), index_file(indexfile)
{
    build();
}

void
SelectRowsFileIndexVMatrix::build()
{
    inherited::build();
    build_();
}

void
SelectRowsFileIndexVMatrix::build_()
{
    if (distr) {
        fieldinfos = distr->fieldinfos;
        length_ = indices.length();
        if (length_ == -1) {
            indices.open(index_file);
            length_ = indices.length();
        }
        width_= distr->width();
        defineSizes(distr->inputsize(), distr->targetsize(), distr->weightsize());
    }
}

void
SelectRowsFileIndexVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &SelectRowsFileIndexVMatrix::distr, OptionBase::buildoption, "");
    declareOption(ol, "index_file", &SelectRowsFileIndexVMatrix::index_file, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

real SelectRowsFileIndexVMatrix::get(int i, int j) const
{ return distr->get(indices[i], j); }

void SelectRowsFileIndexVMatrix::getSubRow(int i, int j, Vec v) const
{ distr->getSubRow(indices[i], j, v); }

void SelectRowsFileIndexVMatrix::getRow(int i, Vec v) const
{ distr->getRow(indices[i], v); }

real SelectRowsFileIndexVMatrix::dot(int i1, int i2, int inputsize) const
{ return distr->dot(indices[i1],indices[i2],inputsize); }

real SelectRowsFileIndexVMatrix::dot(int i, const Vec& v) const
{ return distr->dot(indices[i],v); }

real SelectRowsFileIndexVMatrix::getStringVal(int col, const string & str) const
{ return distr->getStringVal(col, str); }

string SelectRowsFileIndexVMatrix::getValString(int col, real val) const
{ return distr->getValString(col,val); }

string SelectRowsFileIndexVMatrix::getString(int row, int col) const
{ return distr->getString(indices[row], col); }

const map<string,real>& SelectRowsFileIndexVMatrix::getStringToRealMapping(int col) const
{ return distr->getStringToRealMapping(col);}

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
