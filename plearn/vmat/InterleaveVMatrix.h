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

#ifndef InterleaveVMatrix_INC
#define InterleaveVMatrix_INC

#include "VMat.h"

namespace PLearn {
using namespace std;
 

/*!   This class interleaves several VMats (with consecutive rows
  always coming from a different source VMat) thus possibly
  including more than once the rows of the small VMats.
  For example, if source1.length()==10 and source2.length()==30 then
  the resulting VM will have 60 rows, and 3 repetitions
  of each row of source1, with rows taken as follows: 
  source1.row(0), source2.row(0), source1.row(1), source2.row(1), ..., 
  source1.row(9), source2.row(9), source1.row(0), cource2.row(10), ...
  Note that if source2.length() is not a multiple of source1.length()
  some records from source1 will be repeated once more than others.
*/
class InterleaveVMatrix: public VMatrix
{
    typedef VMatrix inherited;

protected:
    TVec<VMat> sources;

public:
    // ******************
    // *  Constructors  *
    // ******************
    InterleaveVMatrix(); //!<  default constructor (for automatic deserialization)

    //! The field names are copied from the first VMat in the array
    InterleaveVMatrix(TVec<VMat> the_sources);

    //! The field names are copied from the first VMat source1
    InterleaveVMatrix(VMat source1, VMat source2);

    PLEARN_DECLARE_OBJECT(InterleaveVMatrix);
    static void declareOptions(OptionList &ol);

    virtual void build();

    virtual real get(int i, int j) const;
    virtual void getSubRow(int i, int j, Vec v) const;
    virtual void reset_dimensions()
    {
        for (int i=0;i<sources.size();i++) sources[i]->reset_dimensions();
        width_=sources[0]->width();
        int maxl = 0;
        int n=sources.size();
        for (int i=0;i<n;i++)
        {
            if (sources[i]->width()!=width_)
                PLERROR("InterleaveVMatrix: source %d has %d width, while 0-th has %d",
                        i, sources[i]->width(), width_);
            int l= sources[i]->length();
            if (l>maxl) maxl=l;
        }
        length_=n*maxl;
    }
private:
    void build_();
    
};

DECLARE_OBJECT_PTR(InterleaveVMatrix);

} // end of namespcae PLearn
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
