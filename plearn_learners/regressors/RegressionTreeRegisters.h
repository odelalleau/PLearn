// -*- C++ -*-

// RegressionTreeRegisters.h
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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


/* *********************************************************************************    
 * $Id: RegressionTreeRegisters.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout *
 * This file is part of the PLearn library.                                      *
 ********************************************************************************* */

#ifndef RegressionTreeRegisters_INC
#define RegressionTreeRegisters_INC

#include <plearn/vmat/VMatrix.h>
#include <plearn/base/stringutils.h>
#include <plearn/math/TMat.h>
#include <plearn/vmat/VMat.h>

//!used to limit the memory used by limiting the length of the dataset.
//!work with unsigned int, uint16_t, but fail with uint8_t???
#define RTR_type uint32_t
//!The type for the leave id
#ifndef RTR_type_id
#define RTR_type_id int16_t
#endif
namespace PLearn {
using namespace std;

class RegressionTreeRegisters: public VMatrix
{
    typedef VMatrix inherited;
  
private:

/*
  Build options: they have to be set before training
*/

    int  report_progress;
    int  verbosity;
//    VMat train_set;
  
/*
  Learnt options: they are sized and initialized if need be, at build() or reinitRegisters()
*/

    int       next_id;
    //TMat<int> sorted_row;

    TMat<RTR_type> tsorted_row;
    TVec<RTR_type_id> leave_register;
    VMat tsource;
    VMat source;

public:

    RegressionTreeRegisters();
    RegressionTreeRegisters(VMat source_, bool report_progress_ = false,
                            bool vebosity_ = false);
    virtual              ~RegressionTreeRegisters();
    
    PLEARN_DECLARE_OBJECT(RegressionTreeRegisters);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    void         reinitRegisters();
    inline void         registerLeave(RTR_type_id leave_id, int row)
    { leave_register[row] = leave_id;    }
    inline virtual real get(int i, int j) const{return tsource->get(j,i);}
    inline real         getTarget(int row)const
    {return tsource->get(inputsize(), row);}
    inline real         getWeight(int row)const{
        if (weightsize() <= 0) return 1.0 / length();
        else return tsource->get(inputsize() + targetsize(), row );
    }
    inline void         setWeight(int row,real val){
        PLASSERT(inputsize()>0&&targetsize()>0);
        PLASSERT(weightsize() > 0);
        tsource->put( inputsize() + targetsize(), row, val );
    }
    inline RTR_type_id     getNextId(){
        PLCHECK(next_id<std::numeric_limits<RTR_type_id>::max());
        next_id += 1;return next_id;}
    void         getAllRegisteredRow(RTR_type_id leave_id, int col, TVec<RTR_type> &reg)const;
    void         getAllRegisteredRow(RTR_type_id leave_id, int col, TVec<RTR_type> &reg,
                                     Vec &target, Vec &weight, Vec &value)const;
    void         printRegisters();
    void         getExample(int i, Vec& input, Vec& target, real& weight);
    inline virtual void put(int i, int j, real value)
    {
        PLASSERT(inputsize()>0&&targetsize()>0);
        if(j!=inputsize()+targetsize())
            PLERROR("In RegressionTreeRegisters::put - implemented the put of "
                    "the weightsize only");
        setWeight(i,value);
    }
    
    //! usefull in MultiClassAdaBoost to save memory
    void         setTSortedRow(TMat<RTR_type> t){tsorted_row = t;}
    TMat<RTR_type> getTSortedRow(){return tsorted_row;}

private:
    void         build_();
    void         sortRows();
    void         sortEachDim(int dim);
    void         sortSmallSubArray(int start_index, int end_index, int dim);
    void         swapIndex(int index_i, int index_j, int dim);
    real         compare(real field1, real field2);
    void         verbose(string msg, int level);

    mutable Vec  getExample_tmp;

};

DECLARE_OBJECT_PTR(RegressionTreeRegisters);

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
