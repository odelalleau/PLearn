// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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


/*! \file VarArray.h */

#ifndef VARARRAY_INC
#define VARARRAY_INC

#include "Variable.h"
#include <plearn/base/general.h>
#include <plearn/base/Array.h>

namespace PLearn {
using namespace std;

class VarArray: public Array<Var>
{
public:

    typedef Var* iterator;

    VarArray();
    explicit VarArray(int n,int n_extra=10);
    VarArray(int n,int initial_length, int n_extra);
    VarArray(int n,int initial_length, int initial_width, int n_extra);
    VarArray(const Array<Var>& va): Array<Var>(va) {}
    VarArray(Array<Var>& va): Array<Var>(va) {}
    VarArray(const VarArray& va): Array<Var>(va) {}
    VarArray(const Var& v);
    VarArray(const Var& v1, const Var& v2);
    VarArray(Variable* v);
    VarArray(Variable* v1, Variable* v2);

    void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    operator Var()
    { 
        if(size()!=1)
            PLERROR("Cannot cast VarArray containing more than one variable to Var");
        return operator[](0);
    }

    VarArray& operator&=(const Var& v) { PLearn::operator&=(*this,v); return *this;}
    VarArray& operator&=(const VarArray& va) { PLearn::operator&=(*this,va); return *this; }
    VarArray operator&(const Var& v) const { return PLearn::operator&(*this,v); }
    VarArray operator&(const VarArray& va) const { return PLearn::operator&(*this,va); }

    int nelems() const;
    int sumOfLengths() const;
    int sumOfWidths() const;
    int maxWidth() const;
    int maxLength() const;

    //!  returns a VarArray containing all the Var's that are non-null
    VarArray nonNull() const;
    VarArray& subVarArray(int start,int len) const;
    void copyFrom(int start,int len,const VarArray& from);//copy Var of from in courant VarArray 
    void copyFrom(int start,const VarArray& from){copyFrom(start,from.size(),from);}
    void copyFrom(const Vec& datavec);
    void copyValuesFrom(const VarArray& from);
    void copyTo(const Vec& datavec) const;
    void accumulateTo(const Vec& datavec) const;
    void copyGradientFrom(const Vec& datavec);
    void copyGradientFrom(const Array<Vec>& datavec);
    void accumulateGradientFrom(const Vec& datavec);
    void copyGradientTo(const Vec& datavec);
    void copyGradientTo(const Array<Vec>& datavec);
    void accumulateGradientTo(const Vec& datavec);

    //!  UNSAFE: x must point to at least n floats!
    void copyTo(real* x, int n) const;
    void accumulateTo(real* x, int n) const;
    void copyFrom(const real* x, int n);
    void makeSharedValue(real* x, int n); //!<  like copyTo but also makes value's point to x
    void makeSharedGradient(real* x, int n); //!<  like copyGradientTo but also makes value's point to x
    //!  make value and matValue point into this storage
    void makeSharedValue(PP<Storage<real> > storage, int offset_=0); 
    void makeSharedValue(Vec& v, int offset_=0);
    void makeSharedGradient(Vec& v, int offset_=0);
    void makeSharedGradient(PP<Storage<real> > storage, int offset_=0); 
    void copyGradientTo(real* x, int n);
    void copyGradientFrom(const real* x, int n);
    void accumulateGradientFrom(const real* x, int n);
    void accumulateGradientTo(real* x, int n);
    void copyMinValueTo(const Vec& minv);
    void copyMaxValueTo(const Vec& maxv);
    void copyMinValueTo(real* x, int n);
    void copyMaxValueTo(real* x, int n);

    void makeSharedRValue(Vec& v, int offset_=0);
    void makeSharedRValue(PP<Storage<real> > storage, int offset_=0);
    void copyRValueTo(const Vec& datavec);
    void copyRValueFrom(const Vec& datavec);
    void copyRValueTo(real* x, int n);
    void copyRValueFrom(const real* x, int n);
    void resizeRValue();

    void setMark() const;
    void clearMark() const;
    void markPath() const;
    void buildPath(VarArray& fpath) const;
    void setDontBpropHere(bool val);

    void fprop();
    void sizeprop();
    void sizefprop();
    void bprop();
    void bbprop();
    void rfprop();
    void fbprop();
    void sizefbprop();
    void fbbprop();

    void fillGradient(real value);
    void clearGradient();
    void clearDiagHessian();
    VarArray sources() const;
    VarArray ancestors() const;
    void unmarkAncestors() const;

    //!  returns the set of all the direct parents of the Var in this array
    //!  (previously marked parents are NOT included)
    VarArray parents() const;

    //!  computes symbolicBprop on a propagation path
    void symbolicBprop(); 
    //!  returns a VarArray of all the ->g members of the Vars in this array.
    VarArray symbolicGradient(); 
    //!  clears the symbolic gradient in all vars of this array
    void clearSymbolicGradient(); 

/*!     set value = value + (step_size * coeff + b) * direction
  with step_size possibly scaled down s.t. box constraints are satisfied
  return true if box constraints have been hit with the update
*/
    bool update(real step_size, Vec direction, real coeff = 1.0, real b = 0.0);

    //! Update the variables in the VarArray with different step sizes,
    //! and an optional scaling coefficient + constant coefficient
    //! step_sizes and direction must have the same length
    //! As with the update with a fixed step size, there is a possible scaling
    //! down, and the return value indicates contraints have been hit
    bool update(Vec step_sizes, Vec direction, real coeff = 1.0, real b = 0.0);
  
    //! Same as update(Vec step_sizes, Vec direction, real coeff)
    //! except there can be 1 different coeff for each variable
    bool update(Vec step_sizes, Vec direction, Vec coeff);

/*!     Using the box constraints on the values, return
  the maximum allowable step_size in the given direction
  i.e., argmax_{step_size} {new = value + step_size * direction, new in box}
*/
    real maxUpdate(Vec direction);

/*!     set value = value + step_size * gradient
  with step_size possibly scaled down s.t. box constraints are satisfied
  return true if box constraints have been hit with the update
*/
    bool update(real step_size, bool clear=false);

    //! for each element of the array:
    //! if (L1)
    //!   value += learning_rate*gradient
    //!   decrease |value| by learning_rate*weight_decay if it does not make value change sign
    //! else // L2
    //!   value += learning_rate*(gradient  - weight_decay*value)
    //! if (clear) gradient=0
    void updateWithWeightDecay(real step_size, real weight_decay, bool L1=false, bool clear=true);

    //! value += step_size*gradient; gradient.clear();
    inline void updateAndClear();

/*!     set value = new_value
  projected down in each direction independently  in the
  subspace in which the box constraints are satisfied.
  return true if box constraints have been hit with the update
*/
    bool update(Vec new_value);

    void read(istream& in);
    void write(ostream& out) const;
    void printNames()const;
    void printInfo(bool print_gradient=false)
    {
        iterator array = data();
        for (int i=0;i<size();i++)
            if (!array[i].isNull())
                array[i]->printInfo(print_gradient);
    }

    Var operator[](Var index);
    inline Var& operator[](int i) { return Array<Var>::operator[](i); }
    inline const Var& operator[](int i) const
    { return Array<Var>::operator[](i); }
};

inline void VarArray::updateAndClear()
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        array[i]->updateAndClear();
}

template <>
inline void deepCopyField(VarArray& field, CopiesMap& copies)
{
    field.makeDeepCopyFromShallowCopy(copies);
}

inline void operator<<(VarArray& ar, const Vec& datavec)
{ ar.copyFrom(datavec); }

inline void operator>>(VarArray& ar, const Vec& datavec)
{ ar.copyTo(datavec); }

void operator<<(VarArray& ar, const Array<Vec>& values);
void operator>>(VarArray& ar, const Array<Vec>& values);

/*! * The function that computes a propagation path * */
/*!   returns the array of all the variables on which fprop is to be called 
  sequentially to do a full fprop (for bprop it is the reverse order).
  NOTE THAT THE INPUTS ARE NOT IN THE RETURNED PATH !
  so that a clearGradient() call on the array won't erase their gradients
  but fprop and bprop will still take into account their values.
*/
VarArray propagationPath(const VarArray& inputs, const VarArray& outputs);

//! returns the propagationpath going from all sources that influence the outputs
//! to the outputs passing by parameters_to_optimize. The sources themselves are not included in the path.
//VarArray propagationPath(const VarArray& inputs, const VarArray& parameters_to_optimize,const VarArray& outputs);

//!  returns the propagationpath going from all sources that influence the outputs
//!  to the outputs. The sources themselves are not included in the path.
VarArray propagationPath(const VarArray& outputs);

//!  from all sources to all direct non-inputs parents of the path inputs-->outputs
VarArray propagationPathToParentsOfPath(const VarArray& inputs, const VarArray& outputs);

//!  Isn't this useless? as we have a constructor of VarArray from Var that should be called automatically !!!???? (Pascal)
/*! 
  inline VarArray propagationPath(const VarArray& inputs, const Var& output)
  { return propagationPath(inputs, VarArray(output); }
*/

/*!   returns the set of all the direct parents of the vars on the path from
  inputs to outputs. (inputs are not included, neither are the direct 
  parents of inputs unless they are also direct parents of other 
  Vars in the path)
*/
VarArray nonInputParentsOfPath(VarArray inputs, VarArray outputs);

//!  returns all sources that influence the given vars
VarArray allSources(const VarArray& v);

//!  returns all variables of a that are not in b
VarArray operator-(const VarArray& a, const VarArray& b);

//!  returns all sources that influence outputs except those that influence it only through inputs
VarArray nonInputSources(const VarArray& inputs, const VarArray& outputs);

void printInfo(VarArray inputs, const Var& output, bool show_gradients=true);

void printInfo(VarArray& a);

/*! * To allow for easy building of VarArray * */
inline VarArray operator&(Var v1, Var v2)
{ return VarArray(v1,v2); }

DECLARE_TYPE_TRAITS(VarArray);

inline PStream &operator>>(PStream &in, VarArray &o)
{ in >> static_cast<Array<Var> &>(o); return in; }
 
inline PStream &operator<<(PStream &out, const VarArray &o)
{ out << static_cast<const Array<Var> &>(o); return out; }


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
