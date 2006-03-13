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


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef SelectRowsFileIndexVMatrix_INC
#define SelectRowsFileIndexVMatrix_INC

#include "VMat.h"
#include <plearn/io/IntVecFile.h>

namespace PLearn {
using namespace std;
 

/*!   selects samples from a sub-distribution
  according to given vector of indices
  that is stored on disk as an IntVecFile
*/
class SelectRowsFileIndexVMatrix: public VMatrix
{
    typedef VMatrix inherited;

protected:
    VMat distr;
    IntVecFile indices;
    string index_file;
public:
    // ******************
    // *  Constructors  *
    // ******************
    SelectRowsFileIndexVMatrix(); //!<  default constructor (for automatic deserialization)

    //! Copy the original fieldinfos upon construction
    SelectRowsFileIndexVMatrix(VMat the_distr, const string& indexfile);

    PLEARN_DECLARE_OBJECT(SelectRowsFileIndexVMatrix);
    static void declareOptions(OptionList &ol);

    virtual void build();

    virtual real get(int i, int j) const;
    virtual void getSubRow(int i, int j, Vec v) const;
    virtual void getRow(int i, Vec v) const;
    virtual real getStringVal(int col, const string & str) const;
    virtual string getValString(int col, real val) const;
    virtual string getString(int row,int col) const;
    virtual const map<string,real>& getStringToRealMapping(int col) const;

    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;
    virtual void reset_dimensions() { distr->reset_dimensions(); width_=distr->width(); }
private:
    void build_();
};

DECLARE_OBJECT_PTR(SelectRowsFileIndexVMatrix);

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
