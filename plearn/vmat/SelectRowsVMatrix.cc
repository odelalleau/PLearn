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
   * $Id: SelectRowsVMatrix.cc,v 1.1 2002/10/03 07:35:28 plearner Exp $
   ******************************************************* */

#include "SelectRowsVMatrix.h"

namespace PLearn <%
using namespace std;


/** SelectRowsVMatrix **/

IMPLEMENT_NAME_AND_DEEPCOPY(SelectRowsVMatrix);

real SelectRowsVMatrix::get(int i, int j) const
{ return distr->get(indices[i], j); }

void SelectRowsVMatrix::getSubRow(int i, int j, Vec v) const
{ distr->getSubRow(indices[i], j, v); }

real SelectRowsVMatrix::dot(int i1, int i2, int inputsize) const
{ return distr->dot(int(indices[i1]), int(indices[i2]), inputsize); }

real SelectRowsVMatrix::dot(int i, const Vec& v) const
{ return distr->dot(indices[i],v); }

real SelectRowsVMatrix::getStringVal(int col, const string & str) const
{ return distr->getStringVal(col, str); }

string SelectRowsVMatrix::getValString(int col, real val) const
{ return distr->getValString(col,val); }

string SelectRowsVMatrix::getString(int row, int col) const
{ return distr->getString(row,col); }

void SelectRowsVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &SelectRowsVMatrix::distr, OptionBase::buildoption, "TODO");
    declareOption(ol, "indices", &SelectRowsVMatrix::indices, OptionBase::buildoption, "TODO");
    inherited::declareOptions(ol);
}


%> // end of namespcae PLearn
