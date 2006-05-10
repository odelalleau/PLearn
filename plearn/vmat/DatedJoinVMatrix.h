// -*- C++ -*-

// DatedJoinVMatrix.h
//
// Copyright (C) 2004 *Yoshua Bengio*
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

// Authors: *Yoshua Bengio*

/*! \file DatedJoinVMatrix.h */


#ifndef DatedJoinVMatrix_INC
#define DatedJoinVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

class DatedJoinVMatrix: public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;

public:
    typedef hash_multimap<Array<real>,int > Maptype;
    //typedef hash_multimap<Array<real>,int> Maptype;

protected:
    // *********************
    // * protected options *
    // *********************

    Vec slave_row, master_row;
    Array<real> key;
    Maptype mp; // maps a key to a list of row indices in the slave
    TVec<int> master2slave;  // maps indices in one db to the other
    TVec<list<int> > slave2master; // there may be more than one master row per slave row, sum them
    int n_master_fields, n_slave_fields; // number of fields of master and slave to copy in result

public:

    // ************************
    // * public build options *
    // ************************

    VMat master, slave;
    TVec<int> master_key_indices;
    TVec<int> slave_key_indices;
    TVec<string> master_key_names;
    TVec<string> slave_key_names;
    TVec<int> slave_field_indices;
    TVec<string> slave_field_names;
    TVec<int> master_field_indices;
    TVec<string> master_field_names;
    int master_date_field_index;
    string master_date_field_name;
    int slave_date_interval_start_field_index;
    int slave_date_interval_end_field_index;
    string slave_date_interval_start_field_name;
    string slave_date_interval_end_field_name;
    int verbosity;
    bool output_the_slave;
    bool output_matching_index;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    DatedJoinVMatrix();

    // ******************
    // * Object methods *
    // ******************

private:
    //! This does the actual building.
    // (Please implement in .cc)
    void build_();

protected:

    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //!  This is the only method requiring implementation
    virtual void getNewRow(int i, const Vec& v) const;

public:

    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(DatedJoinVMatrix);

};
DECLARE_OBJECT_PTR(DatedJoinVMatrix);

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
