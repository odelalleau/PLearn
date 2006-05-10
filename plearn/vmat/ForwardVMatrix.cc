// -*- C++ -*-

// ForwardVMatrix.cc
// Copyright (C) 2002 Pascal Vincent
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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#include "ForwardVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(ForwardVMatrix,
                        "Forward all calls to an underlying VMatrix.",
                        ""
    );

ForwardVMatrix::ForwardVMatrix()
{}

ForwardVMatrix::ForwardVMatrix(VMat the_vm)
    : vm(the_vm)
{
}

void
ForwardVMatrix::build()
{
    inherited::build();
    build_();
}

void
ForwardVMatrix::build_()
{
    if (vm) {
        length_ = vm->length();
        width_ = vm->width();
        writable = vm->isWritable();
        setMetaInfoFrom(vm);
        if (vm->hasMetaDataDir() && !this->hasMetaDataDir())
            setMetaDataDir(vm->getMetaDataDir());

    } else {
        length_ = 0;
        width_ = 0;
    }
}

void ForwardVMatrix::setVMat(VMat the_vm)
{
    if(the_vm)
        vm = the_vm;
    else
        vm = VMat();
    build_();
}

void
ForwardVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "vm", &ForwardVMatrix::vm, OptionBase::buildoption,
                  "The underlying VMat to which all calls are forwarded.");

    inherited::declareOptions(ol);

    // Hide unused options (automatically defined at build time).

    declareOption(ol, "writable",    &ForwardVMatrix::writable, OptionBase::nosave, "");
    declareOption(ol, "length",      &ForwardVMatrix::length_,  OptionBase::nosave, "");
    declareOption(ol, "width",       &ForwardVMatrix::width_,   OptionBase::nosave, "");
}

string ForwardVMatrix::getValString(int col, real val) const
{ return vm->getValString(col, val); }


real ForwardVMatrix::getStringVal(int col, const string& str) const
{ return vm->getStringVal(col, str); }


string ForwardVMatrix::getString(int row,int col) const
{ return vm->getString(row,col); }

map<string,real> ForwardVMatrix::getStringMapping(int col) const
{
    PLERROR("ForwardVMatrix::getStringMapping not implemented yet");
    // making the compiler happy:
    return map<string,real>();

    //  return vm->getStringMapping(col);
}

////////////////////////////
// getStringToRealMapping //
////////////////////////////
const map<string,real>& ForwardVMatrix::getStringToRealMapping(int col) const {
    return vm->getStringToRealMapping(col);
}

////////////////////////////
// getRealToStringMapping //
////////////////////////////
const map<real,string>& ForwardVMatrix::getRealToStringMapping(int col) const {
    return vm->getRealToStringMapping(col);
}

void ForwardVMatrix::computeStats()
{ vm->computeStats(); }


void ForwardVMatrix::save(const PPath& filename) const
{ vm->save(filename); }

void ForwardVMatrix::savePMAT(const PPath& pmatfile) const
{ vm->savePMAT(pmatfile); }
void ForwardVMatrix::saveDMAT(const PPath& dmatdir) const
{ vm->saveDMAT(dmatdir); }

//////////////
// saveAMAT //
//////////////
void ForwardVMatrix::saveAMAT(const PPath& amatfile, bool verbose, bool no_header, bool save_strings) const
{ vm->saveAMAT(amatfile, verbose, no_header, save_strings); }

real ForwardVMatrix::get(int i, int j) const
{ return vm->get(i, j); }


void ForwardVMatrix::put(int i, int j, real value)
{ vm->put(i, j, value); }

void ForwardVMatrix::getSubRow(int i, int j, Vec v) const
{ vm->getSubRow(i, j, v); }

void ForwardVMatrix::putSubRow(int i, int j, Vec v)
{ vm->putSubRow(i, j, v); }

void ForwardVMatrix::appendRow(Vec v)
{ vm->appendRow(v); }

void ForwardVMatrix::getRow(int i, Vec v) const
{
    assert( v.length() == width() );
    vm->getRow(i, v);
}

void ForwardVMatrix::putRow(int i, Vec v)
{ vm->putRow(i, v); }
void ForwardVMatrix::fill(real value)
{ vm->fill(value); }
void ForwardVMatrix::getMat(int i, int j, Mat m) const
{ vm->getMat(i, j, m); }
void ForwardVMatrix::putMat(int i, int j, Mat m)
{ vm->putMat(i, j, m); }


void ForwardVMatrix::getColumn(int i, Vec v) const
{ vm->getColumn(i, v); }

Mat ForwardVMatrix::toMat() const
{ return vm->toMat(); }



void ForwardVMatrix::compacify()
{ vm->compacify(); }


void ForwardVMatrix::reset_dimensions()
{
    if (vm) {
        vm->reset_dimensions();
        length_ = vm->length();
        width_ = vm->width();
        inputsize_ = vm->inputsize();
        targetsize_ = vm->targetsize();
        weightsize_ = vm->weightsize();
    }
}

VMat ForwardVMatrix::subMat(int i, int j, int l, int w)
{ return vm->subMat(i,j,l,w); }

real ForwardVMatrix::dot(int i1, int i2, int inputsize) const
{ return vm->dot(i1, i2, inputsize); }


real ForwardVMatrix::dot(int i, const Vec& v) const
{ return vm->dot(i,  v); }

void ForwardVMatrix::accumulateXtY(int X_startcol, int X_ncols, int Y_startcol, int Y_ncols,
                                   Mat& result, int startrow, int nrows, int ignore_this_row) const
{ vm->accumulateXtY(X_startcol, X_ncols, Y_startcol, Y_ncols,
                    result, startrow, nrows, ignore_this_row); }

void ForwardVMatrix::accumulateXtX(int X_startcol, int X_ncols,
                                   Mat& result, int startrow, int nrows, int ignore_this_row) const
{ vm->accumulateXtX(X_startcol, X_ncols,
                    result, startrow, nrows, ignore_this_row); }

void ForwardVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(vm, copies);
}

}


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
