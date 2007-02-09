// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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

#ifndef SumOfVariable_INC
#define SumOfVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


/**
 *  Sums the value of a Function evaluated on each row of a VMatrix
 *
 *  SumOfVariable computes the sum of the value of a Func evaluated on each row
 *  of a VMat.  This summation is not necessarily constrained to be over all
 *  the rows: each fprop computes the sum over 'nsample' rows of the associated
 *  VMatrix.  This Variable is used within the implementation of NNet to create
 *  the optimization criterion over the training set (which corresponds here to
 *  the VMatrix we are summing over).
 */
class SumOfVariable: public NaryVariable
{
    typedef NaryVariable inherited;

// protected:
public:
    VMat distr;
    Func f;

    int nsamples; //!< number of consecutive samples from the dataset distr
                  //!< that every propagation operation will use

    int curpos;   //!< position of current sample in dataset distr 

    bool loop;    //!< if true, every propagation operation, before returning,
                  //!< will set back curpos to the value it had when entering
                  //!< the call. So that curpos will be unchanged by the call.

    // To avoid allocation/deallocations in fprop/bprop
    Vec input_value;
    Vec input_gradient;
    Vec output_value;
    //! Indication that sizefprop should be used on f
    bool do_sizeprop;

    int beginpos;
    int endpos;
    
public:
    //!  protected default constructor for persistence
    SumOfVariable() : distr(), f(), nsamples(0), curpos(0), loop(false) {}

    //!  Sum_{inputs \in distr} f(inputs)
    SumOfVariable(VMat the_distr, Func the_f, int the_nsamples=-1, bool the_do_resizeprop=false);
    
    PLEARN_DECLARE_OBJECT(SumOfVariable);
    static void declareOptions(OptionList &ol);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual void fprop();
    virtual void bprop();
    virtual void fbprop();
    virtual void symbolicBprop();
    virtual void rfprop();

    VMat getDataSet() const
    { return distr; }

    void setDataSet(VMat dset)
    {
        if(distr.isNotNull() && distr.length()==nsamples)
            nsamples = -1;
        
        distr = dset;
        if(nsamples == -1)
            nsamples = distr->length();

        curpos = 0;
    }

    void setCurrentSamplePos(int pos)
    { curpos = pos; }

    int getCurrentSamplePos() const
    { return curpos; }

    //! This allows to control over which part of the dataset
    //! the next propagation operation(s) will sum.
    /** The call sets the curpos, nsamples and loop options.  Thus the next
        propagation call will start at sample curpos=startpos and sum over
        nsamples=n consecutive samples.  If loop (assigned the value do_loop)
        is true, then curpos will be left unchanged by propagation calls, which
        will thus always sum over the same nsamples samples.  If loop is
        false however, any propagation call will move curpos by nsamples, thus
        a subsequent propagation call will sum over the *next* nsamples (which
        will correspond to the same smaples only if nsamples == distr.length()) **/

    void setSampleRange(int startpos, int n, bool do_loop)
    {
        curpos = startpos;
        nsamples = n;
        loop = do_loop;
    }

    void printInfo(bool print_gradient);

protected:
    void build_();
};

DECLARE_OBJECT_PTR(SumOfVariable);

//!  sumOf
inline Var sumOf(VMat distr, Func f, int nsamples=-1, bool the_do_sizeprop=false)
{ 
    if(nsamples<0) nsamples = distr.length();
    return new SumOfVariable(distr,f,nsamples,the_do_sizeprop); 
}

//!  deprecated old version do not use!
inline Var sumOf(Var output, const VarArray& inputs, VMat distr, int nsamples=-1, VarArray parameters=VarArray(), bool the_do_sizeprop=false)
{ 
    if(nsamples<0) nsamples = distr.length();
    return sumOf(distr,Func(inputs,output),nsamples,the_do_sizeprop); 
}

//!  meanOf
inline Var meanOf(VMat distr, Func f, int nsamples=-1, bool the_do_sizeprop=false)
{ 
    if(nsamples<0) nsamples = distr.length();
    return new SumOfVariable(distr,f/nsamples,nsamples, the_do_sizeprop); 
}

//!  deprecated old version do not use!
inline Var meanOf(Var output, const VarArray& inputs, VMat distr, int nsamples=-1, VarArray parameters=VarArray(), bool the_do_sizeprop=false)
{ 
    if(nsamples<0) nsamples = distr.length();
    return meanOf(distr, Func(inputs,output), nsamples, the_do_sizeprop); 
}

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
