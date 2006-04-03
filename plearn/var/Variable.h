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

#ifndef Variable_INC
#define Variable_INC

#include <plearn/math/TMat.h>
#include <plearn/base/Object.h>

namespace PLearn {
using namespace std;

class Variable;
class VarArray;
class RandomVariable;
class RandomVar;

class Var: public PP<Variable>
{
    friend class RandomVariable;
    friend class RandomVar;

public:
    Var();
    Var(Variable* v);
    Var(Variable* v, const char* name);
    Var(const Var& other);
    Var(const Var& other, const char* name);
    Var(const Var &other, const string &name);
    explicit Var(int the_length, int width_=1);
    Var(int the_length, int the_width, const char* name);
    Var(int the_length, int the_width, const string &name);
    Var(int the_length, const char* name);
    Var(int the_length, const string &name);
    explicit Var(const Vec& vec, bool vertical=true);
    explicit Var(const Mat& mat);

    int length() const;
    int width() const;

    Var subVec(int start, int len, bool transpose=false) const;
    Var subMat(int i, int j, int sublength, int subwidth, bool transpose=false) const;
    Var row(int i, bool transpose=false) const;
    Var column(int j, bool transpose=false) const;
    Var operator()(int i) const;
    Var operator()(int i, int j) const;

    //!  take element i of a vector
    Var operator[](int i) const;
    Var operator[](Var i) const;
  
    //!  take row i of a matrix
    Var operator()(Var index) const;

    //!  take element i,j of a matrix
    Var operator()(Var i, Var j) const;

    void operator=(real f);
    void operator=(const Vec& v);
    void operator=(const Mat& m);
};

class Variable: public Object
{

private:

    typedef Object inherited;

protected:
    //!  Default constructor for persistence
    Variable() : varnum(++nvars), marked(false), varname(), allows_partial_update(false),
                 gradient_status(0), valuedata(0), gradientdata(0), min_value(-FLT_MAX),
                 max_value(FLT_MAX), dont_bprop_here(false) {}

    static void declareOptions(OptionList & ol);
  
    friend class Var;
    friend class RandomVariable;
    friend class ProductRandomVariable;
    friend class Function;

    friend class UnaryVariable;
    friend class BinaryVariable;
    friend class NaryVariable;

public:
    static int nvars; //!<  keeps track of how many vars have been created (also used for the default naming scheme, see getName() )
    int varnum; //!<  number of this variable (the first one created is numbered 1, the second 2, etc...) 

protected:
    bool marked; //!<  used for building the propagation paths
    string varname; //!<  used when printing or drawing the var graph (see setName and getName)

protected:
    bool allows_partial_update; //!<  only if this is true then the following two fields are used.
    int gradient_status; //!<  0: no gradient was accumulated, 1: to some rows, 2: everywhere.
    TVec<int> rows_to_update; //!<  the list of rows to update.

public:
    Vec value;
    Vec gradient;
    Mat matValue;
    Mat matGradient;
    Vec rValue;
    Mat matRValue;
    Mat matDiagHessian; //!<  optionally computed second derivative (see bbprop methods)
    //!  Convenience variables
    real* valuedata; //!<  Set to value.data()
    real* gradientdata; //!<  set to gradient.data()
    real min_value, max_value; //!<  box constraints on values
    Var g; //!<  symbolic gradient used for symbolicBprop
    Vec diaghessian; //!<  optionally computed second derivative (see bbprop methods)
    real* diaghessiandata; //!<  set to diaghessian.data() or NULL if no diaghessian
    real* rvaluedata;
    bool dont_bprop_here; //!< if true, children are encouraged not to bprop gradient in this var (saves computation time)
  
public:
    Variable(int thelength, int thewidth);
    Variable(const Mat& m);  //!<  this variable's value and m will be views of the same data

    int length() const { return matValue.length(); }
    int width() const { return matValue.width(); }
    int size() const { return matValue.size(); } // length*width
    int nelems() const { return size(); }

    //! Recomputes the length l and width w that this 
    //! variable should have, according to its parent 
    //! variables. This is used for ex. by sizeprop()
    //! The default version stupidly returns the
    //! current dimensions, so make sure to overload
    //! it in subclasses if this is not appropriate.
    virtual void recomputeSize(int& l, int& w) const;

    //! resizes the matValue and matGradient fields of this variable
    //! (and updates the value, gradient, valuedata and gradientdata 
    //! fields accordingly)
    void resize(int l, int w);
  
    //! resizes value and gradient fields according to
    //! size given by recomputeSize(...)
    //! This corresponds to "propagating" the size from its
    //! parent's size, much as fprop propagates the values
    void sizeprop();

    //! set this Variable's parents.  To use with default constructor.
    virtual void setParents(const VarArray& parents);

    //! Copy constructor
    Variable(const Variable& v);

private:
    void build_();
public:
    virtual void build();

    bool isScalar() const { return length()==1 && width()==1; }
    bool isVec() const { return length()==1 || width()==1; }
    bool isColumnVec() const { return width()==1; }
    bool isRowVec() const { return length()==1; }

    PLEARN_DECLARE_ABSTRACT_OBJECT(Variable);

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //!  compute output given input
    virtual void fprop() =0;
    //!  compute dC/dinput given dC/doutput

    //! Calls sizeprop, then fprop
    inline void sizefprop()
    { sizeprop(); fprop(); }

    virtual void bprop() =0;
    //!  compute an approximation to diag(d^2/dinput^2) given diag(d^2/doutput^2), 
    //!  with diag(d^2/dinput^2) ~=~ (doutput/dinput)' diag(d^2/doutput^2) (doutput/dinput)
    //! In particular: if 'C' depends on 'y' and 'y' depends on x ...
    //!               d^2C/dx^2 = d^2C/dy^2 * (dy/dx)^2   +   dC/dy * d^2y/dx^2
    //!                          (diaghessian)              (gradient)
    virtual void bbprop();
    //!  do both fprop and bprop
    virtual void fbprop();
    //!  do fprop, bprop and bbprop
    virtual void fbbprop();
    //!  compute a piece of new Var graph that represents the symbolic derivative of this Var
    virtual void symbolicBprop();

    virtual void rfprop();

    virtual void copyValueInto(Vec v) { v << value; }
    virtual void copyGradientInto(Vec g) { g << gradient; }

    virtual void newwrite(PStream& out) const;

    //!  returns the name of this variable. If its name has not been set, 
    //!  it will be assigned a name of V_varnum
    string getName() const;
    //!  call this to set a name for this variable
    void setName(const string& the_name); 
    bool nameIsSet() { return varname.size()>0; }

    //! Defines a new Mat to use as this Var's matValue field,
    //! modifies value and valuedata to keep consistent, and 
    //! returns the previous matValue.
    //! Also resizes the gradient in order to ensure it has same size as the
    //! new value.
    Mat defineValueLocation(const Mat& m);

    //! Defines a new Mat to use as this Var's matGradient field,
    //! modifies gradient and gradientdata to keep consistent, and 
    //! returns the previous matGradient.
    //! Also resizes the value in order to ensure it has same size as the
    //! new gradient.
    Mat defineGradientLocation(const Mat& m);

    virtual void printInfo(bool print_gradient=false) = 0;
    virtual void printInfos(bool print_gradient=false);

    Var subVec(int start, int len, bool transpose=false);
    Var subMat(int i, int j, int sublength, int subwidth, bool transpose=false);
    Var row(int i, bool transpose=false) { return subMat(i,0,1,width(),transpose); }
    Var column(int j, bool transpose=false) { return subMat(0,j,length(),1,transpose); }

    void setDontBpropHere(bool val) { dont_bprop_here = val; }
    void setKeepPositive() { min_value = 0; }
    void setMinValue(real minv=-FLT_MAX) { min_value = minv; }
    void setMaxValue(real maxv=FLT_MAX) { max_value = maxv; }
    void setBoxConstraint(real minv, real maxv) { min_value = minv; max_value = maxv; }

    void setMark() { marked = true; }
    void clearMark() { marked = false; }
    bool isMarked() { return marked; }

    void fillGradient(real value) { gradient.fill(value); }
    void clearRowsToUpdate()
    {
        rows_to_update.resize(0);
        gradient_status=0;
    }
    void clearGradient() 
    { 
        if(!allows_partial_update) 
            gradient.clear(); 
        else
        {
            for (int r=0;r<rows_to_update.length();r++)
            {
                int row = rows_to_update[r];
                matGradient.row(row).clear();
            }
            rows_to_update.resize(0);
            gradient_status=0;
        }
    }
    void clearDiagHessian(); 
    void clearSymbolicGradient() { g = Var(); }

/*!     set value = value + (step_size * coeff + b) * direction 
  with step_size possibly scaled down s.t. box constraints are satisfied
  return true if box constraints have been hit with the update
  If (allows_partial_update) the update is done where necessary. // NB: Wrong ?
*/
    bool update(real step_size, Vec direction_vec, real coeff = 1.0, real b = 0.0);

/*!     set value[i] = value[i] + (step_sizes[i]*coeff + b) * direction[i]
  with step_size possibly scaled down s.t. box constraints are satisfied
  return true if box constraints have been hit with the update
*/
    bool update(Vec step_sizes, Vec direction_vec, real coeff = 1.0, real b = 0.0);

    //! Does value += gradient; gradient.clear();
    inline void updateAndClear();

/*!     set value = value + step_size * gradient
  with step_size possibly scaled down s.t. box constraints are satisfied
  return true if box constraints have been hit with the update
*/
    bool update(real step_size, bool clear=false);

    //! if (L1)
    //!   value += learning_rate*gradient
    //!   decrease |value| by learning_rate*weight_decay if it does not make value change sign
    //! else // L2
    //!   value += learning_rate*(gradient  - weight_decay*value)
    //! if (clear) gradient=0
    void updateWithWeightDecay(real step_size, real weight_decay, bool L1, bool clear=true);

    //!  send message that update may be sometimes needed on only parts of the Variable
    void allowPartialUpdates()
    {
        allows_partial_update=true; 
        rows_to_update.resize(length()); // make sure that there are always enough elements
        rows_to_update.resize(0);
        gradient_status=0;
    }

    //!  send message that updates must be full.
    void disallowPartialUpdates()
    {
        allows_partial_update = false;
        gradient_status=2;
    }

    //!  says that given row has received gradient (should be updated on next call to update)
    void updateRow(int row)
    {
        if (gradient_status!=2 && allows_partial_update && !rows_to_update.contains(row))
        {
            rows_to_update.append(row);
            if (gradient_status==0) gradient_status=1;
        }
    }


/*!     set value = new_value
  projected down in each direction independently  in the
  subspace in which the box constraints are satisfied.
  return true if box constraints have been hit with the update
*/
    bool update(Vec new_value);

/*!     Using the box constraints on the values, return
  the maximum allowable step_size in the given direction
  i.e., argmax_{step_size} {new = value + step_size * direction, new in box}
*/
    real maxUpdate(Vec direction);

/*!     sets the marked flag of all the sVariable that are to be in the fprop path.
  The input sVariable that are of interest are to be marked first.
  Then markPath is to be called from the output Variable of interest
*/
    virtual bool markPath() =0;

    //!  Finally buildPath is to be called from the output Variable of interest
    //!  (this will build the proppath at the same time as erasing the marks)
    virtual void buildPath(VarArray& proppath) =0;

    virtual void oldread(istream& in);
    virtual void write(ostream& out) const;

  
  

    void copyFrom(const Vec& v)    { value << v; }
    void copyTo(Vec& v)    { v << value; }
    void copyGradientFrom(const Vec& v)    { gradient << v; }
    void copyGradientTo(Vec& v) { v << gradient; }
    void makeSharedValue(real* x, int n); //!<  like copyTo but also makes value's point to x
    void makeSharedGradient(real* x, int n); //!<  like copyTo but also makes value's point to x
    //!  make value and matValue point into this storage
    void makeSharedValue(PP<Storage<real> > storage, int offset_=0); 
    void makeSharedGradient(PP<Storage<real> > storage, int offset_=0); 
    void makeSharedValue(Vec& v, int offset_=0);
    void makeSharedGradient(Vec& v, int offset_=0);

    void copyRValueFrom(const Vec& v) { resizeRValue(); rValue << v; }
    void copyRValueTo(Vec& v) { resizeRValue(); v << rValue; }
    void makeSharedRValue(real* x, int n); //!<  like copyTo but also makes value's point to x
    void makeSharedRValue(PP<Storage<real> > storage, int offset_=0);
    void makeSharedRValue(Vec& v, int offset_=0);

    // make this var point to the same things as v, using default operator=
    void makePointTo(Variable* v) { 
        value = v->value;
        valuedata = v->valuedata;
        matValue = v->matValue;
        gradient = v->gradient;
        matGradient = v->matGradient;
        gradientdata = v->gradientdata;
        rows_to_update = v->rows_to_update;
        rValue = v->rValue;
        matRValue = v->matRValue;
        matDiagHessian = v->matDiagHessian;
        diaghessian = v->diaghessian;
        diaghessiandata = v->diaghessiandata;
        rvaluedata = v->rvaluedata;
    }

    virtual bool isConstant() { return false; }

/*!     find all constant sources that influence this Variable, build 
  a propagation path from them to this Variable, and fprop through it.
  This can be useful to make sure that all dependencies are 
  computed at least once. This function uses source(), below.
*/
    virtual void fprop_from_all_sources();

    //!  if not marked, find all constant sources that influence this Variable.
    //!  A constant source is normally a SourceVariable.
    virtual VarArray sources() = 0;

    //!  return ancestors which compute a non-deterministic function 
    //!  of their parents
    virtual VarArray random_sources() = 0;

    //!  if not marked, find all Variables that influence this Variable.
    virtual VarArray ancestors() = 0;
    //!  undo any marking done by a call to sources() or ancestors()
    virtual void unmarkAncestors() = 0;

    //!  returns all the direct parents of this Var that are not marked
    //!  (the call doesn't change any mark)
    virtual VarArray parents() = 0;

    //!  accumulate the symbolic gradient in a smart way...
    virtual void accg(Var v);

    //!  call verify gradient for the mapping from
    //!  all the sources to this Variable.
    virtual void verifyGradient(real step=0.001);

    //!  resize the DiagHessian field
    virtual void resizeDiagHessian();

    virtual void resizeRValue();
};

DECLARE_OBJECT_PTR(Variable);
DECLARE_OBJECT_PP(Var, Variable);

// set value += gradient and clears the gradient
inline void Variable::updateAndClear()
{
    /*
    */
    if (allows_partial_update && gradient_status!=2)
    {
        if (gradient_status!=0)
        {
            for (int r=0;r<rows_to_update.length();r++)
            {
                int row = rows_to_update[r];
                real* direction = matGradient[row];
                real* params = matValue[row];
                for(int i=0; i<width(); i++)
                {
                    params[i] += direction[i];
                    direction[i] = 0;
                }              
            }
            rows_to_update.resize(0);
            gradient_status=0;
        }
    }
    else 
    {
        for(int i=0; i<nelems(); i++)
            valuedata[i] += gradientdata[i];
        gradient.clear();
    }

}

void varDeepCopyField(Var& field, CopiesMap& copies);


inline Var Var::row(int i, bool transpose) const
{
    return subMat(i, 0, 1, width(), transpose);
}
    
inline Var Var::column(int j, bool transpose) const
{
    return subMat(0, j, length(), 1, transpose);
}

inline Var Var::operator()(int i) const
{
    return row(i, false);
}

inline Var Var::operator()(int i, int j) const
{
    return subMat(i, j, 1, 1);
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
