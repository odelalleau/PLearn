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

#include "CrossReferenceVMatrix.h"

namespace PLearn {
using namespace std;


/** CrossReferenceVMatrix **/

PLEARN_IMPLEMENT_OBJECT(CrossReferenceVMatrix, "ONE LINE DESC", "ONE LINE HELP");

CrossReferenceVMatrix::CrossReferenceVMatrix()
    : index(0)
{
}

CrossReferenceVMatrix::CrossReferenceVMatrix(VMat the_master,
                                             int the_index,
                                             VMat the_slave)
    : inherited(the_master.length(), the_master.width()+the_slave.width()-1),
      master(the_master),
      index(the_index),
      slave(the_slave)
{
    //fieldinfos = the_master->getFieldInfos();
    // fieldinfos &= the_slave->getFieldInfos();
    build();
}


void
CrossReferenceVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "master", &CrossReferenceVMatrix::master,
                  OptionBase::buildoption, "");

    declareOption(ol, "slave", &CrossReferenceVMatrix::slave,
                  OptionBase::buildoption, "");

    declareOption(ol, "index", &CrossReferenceVMatrix::index,
                  OptionBase::buildoption, "");

    inherited::declareOptions(ol);
}

void
CrossReferenceVMatrix::build()
{
    inherited::build();
    build_();
}

void
CrossReferenceVMatrix::build_()
{
    if (master && slave) {
        fieldinfos = master->getFieldInfos();
        fieldinfos &= slave->getFieldInfos();
        updateMtime(master);
        updateMtime(slave);
    }
}

void CrossReferenceVMatrix::getRow(int i, Vec samplevec) const
{
#ifdef BOUNDCHECK
    if (i<0 || i>=length() || samplevec.length()!=width())
        PLERROR("In CrossReferenceVMatrix::getRow OUT OF BOUNDS");
#endif

    Vec v1(master.width());
    Vec v2(slave.width());
    master->getRow(i, v1);
    int index = (int)v1[index];
    slave->getRow(index, v2);

    for (int j=0; j<index; j++) samplevec[j] = v1[j];
    for (int j=index+1; j<v1.length(); j++) samplevec[j-1] = v1[j];
    for (int j=0; j<v2.length(); j++) samplevec[j+v1.length()-1] = v2[j];
}

real CrossReferenceVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j>=width())
        PLERROR("In CrossReferenceVMatrix::get OUT OF BOUNDS");
#endif

    if (j < index)
        return master->get(i,j);
    else if (j < master.width()-1)
        return master->get(i,j+1);
    else {
        int ii = (int)master->get(i,index);
        int jj = j - master.width() + 1;
        return slave->get(ii,jj);
    }
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
