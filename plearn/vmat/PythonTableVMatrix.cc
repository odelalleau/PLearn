// -*- C++ -*-

// PythonTableVMatrix.cc
//
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

// Authors: Xavier Saint-Mleux

/*! \file PythonTableVMatrix.cc */


#include "PythonTableVMatrix.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    PythonTableVMatrix,
    "VMatrix wrapper for a python table (module apstat.data.table)",
    "VMatrix wrapper for a python table (module apstat.data.table).\n"
    "The underlying table must be numeric.\n"
    );

PythonTableVMatrix::PythonTableVMatrix(PyObject* table)
    :the_table(table)
{
}

void PythonTableVMatrix::getNewRow(int i, const Vec& v) const
{
    PLASSERT(the_table);
    PyObject* row= PyObject_CallMethod(the_table, "getRow", "i", i);
    if(!row)
    {
        if (PyErr_Occurred()) PyErr_Print();
        PLERROR("in PythonTableVMatrix::getNewRow : "
                "call to underlying table's 'getRow' failed.");
    }
    v << PythonObjectWrapper(row).as<Vec>();
    Py_DECREF(row);
}

void PythonTableVMatrix::declareOptions(OptionList& ol)
{

    declareOption(ol, "table", &PythonTableVMatrix::the_table,
                  OptionBase::buildoption,
                  "underlying table");

    inherited::declareOptions(ol);
}

void PythonTableVMatrix::build_()
{
    if(!the_table) return;
    PyObject* pywidth= PyObject_CallMethod(the_table, "width", 0);
    if(!pywidth)
    {
        if (PyErr_Occurred()) PyErr_Print();
        PLERROR("in PythonTableVMatrix::build_ : "
                "call to underlying table's 'width' failed.");
    }
    width_= PythonObjectWrapper(pywidth);
    Py_DECREF(pywidth);
    PyObject* pylength= PyObject_CallMethod(the_table, "length", 0);
    if(!pylength)
    {
        if (PyErr_Occurred()) PyErr_Print();
        PLERROR("in PythonTableVMatrix::build_ : "
                "call to underlying table's 'length' failed.");
    }
    length_= PythonObjectWrapper(pylength);
    Py_DECREF(pylength);
    PyObject* pyfieldnames= PyObject_GetAttrString(the_table, "fieldnames");
    if(!pyfieldnames)
    {
        if (PyErr_Occurred()) PyErr_Print();
        PLERROR("in PythonTableVMatrix::build_ : "
                "access to underlying table's 'fieldnames' failed.");
    }
    declareFieldNames(PythonObjectWrapper(pyfieldnames));
    Py_DECREF(pyfieldnames);

    if(PyObject_HasAttrString(the_table, "inputsize"))
    {
        PyObject* inpsz= PyObject_GetAttrString(the_table, "inputsize");
        if(!inpsz)
        {
            if (PyErr_Occurred()) PyErr_Print();
            PLERROR("in PythonTableVMatrix::build_ : "
                    "access to underlying table's 'inputsize' failed.");
        }
        inputsize_= PythonObjectWrapper(inpsz).as<int>();
    }
    if(PyObject_HasAttrString(the_table, "targetsize"))
    {
        PyObject* targsz= PyObject_GetAttrString(the_table, "targetsize");
        if(!targsz)
        {
            if (PyErr_Occurred()) PyErr_Print();
            PLERROR("in PythonTableVMatrix::build_ : "
                    "access to underlying table's 'targetsize' failed.");
        }
        targetsize_= PythonObjectWrapper(targsz).as<int>();
    }
    if(PyObject_HasAttrString(the_table, "weightsize"))
    {
        PyObject* wgtsz= PyObject_GetAttrString(the_table, "weightsize");
        if(!wgtsz)
        {
            if (PyErr_Occurred()) PyErr_Print();
            PLERROR("in PythonTableVMatrix::build_ : "
                    "access to underlying table's 'weightsize' failed.");
        }
        weightsize_= PythonObjectWrapper(wgtsz).as<int>();
    }


}

void PythonTableVMatrix::build()
{
    inherited::build();
    build_();
}

void PythonTableVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(trainvec, copies);

    PLERROR("PythonTableVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
