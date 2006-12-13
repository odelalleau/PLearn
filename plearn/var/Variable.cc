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

/** Variable **/

#include "Var.h"
#include "VarArray.h"
#include "Func.h"

#include "VarElementVariable.h"
#include "VarRowVariable.h"
#include "SubMatVariable.h"
#include "SubMatTransposeVariable.h"
#include "SourceVariable.h"
#include "PlusScalarVariable.h"
#include "TimesConstantVariable.h"
#include "Var_operators.h"

//#include "Var_utils.h"

namespace PLearn {
using namespace std;

// To be able to use varDeepCopyField.
// extern void varDeepCopyField(Var& field, CopiesMap& copies);

/** Var **/

Var::Var() :PP<Variable>(0) {}
Var::Var(Variable* v) :PP<Variable>(v) {}
Var::Var(Variable* v, const char* name) :PP<Variable>(v) { ptr->setName(name); }
Var::Var(const Var& other) :PP<Variable>(other) {}
Var::Var(const Var& other, const char* name) :PP<Variable>(other) { ptr->setName(name); }
Var::Var(const Var &other, const string &name) : PP<Variable>(other) { ptr->setName(name); }

Var::Var(int the_length, const char* name) 
    :PP<Variable>(new SourceVariable(the_length,1)) 
{ ptr->setName(name); }

Var::Var(int the_length, const string &name)
    : PP<Variable>(new SourceVariable(the_length, 1))
{ ptr->setName(name); }

Var::Var(int the_length, int the_width)
    :PP<Variable>(new SourceVariable(the_length,the_width)) {}

Var::Var(int the_length, int the_width, const char* name)
    :PP<Variable>(new SourceVariable(the_length,the_width)) { ptr->setName(name); }

Var::Var(int the_length, int the_width, const string &name)
    : PP<Variable>(new SourceVariable(the_length, the_width))
{ ptr->setName(name); }

Var::Var(const Vec& v, bool vertical) 
    :PP<Variable>(new SourceVariable(v,vertical)) 
{}

Var::Var(const Mat& m) 
    :PP<Variable>(new SourceVariable(m))
{}

int Var::length() const
{ return (*this)->length(); }

int Var::width() const
{ return (*this)->width(); }

Var Var::operator[](int i) const
{
    if(width()==1)
        return operator()(i,0);
    else if(length()==1)
        return operator()(0,i);
    PLERROR("You shouldnt use operator[](int i) to access a matrix variable, consider using operator() instead");
    return Var();
}

Var Var::operator[](Var index) const
{ 
    if(width()!=1)
        PLERROR("You shouldnt use operator[](Var index) to get the row of a matrix var but operator()(Var index)");
    return new VarElementVariable(*this,index); 
}

Var Var::subMat(int i, int j, int sublength, int subwidth, bool do_transpose) const
{ 
    if(do_transpose)
        return new SubMatTransposeVariable(*this, i, j, sublength, subwidth);
    else 
        return new SubMatVariable(*this, i, j, sublength, subwidth);
}

Var Var::subVec(int start, int len, bool transpose) const
{
    if(width()==1)
        return subMat(start,0,len,1,transpose);
    else if(length()==1)
        return subMat(0,start,1,len,transpose);

    PLERROR("In Variable::subVec variable is not a vec (single column or single row)");
    return Var();
}

Var Var::operator()(Var index) const
{ return new VarRowVariable(*this,index); }

Var Var::operator()(Var i, Var j) const
{ return new VarElementVariable(*this, new PlusScalarVariable(j, new TimesConstantVariable(i,(real)width()))); }

void Var::operator=(real f)
{ 
    if (!isNull())
        (*this)->value.fill(f);
    else
        PLERROR("Var::operator= called on null Var");
}

void Var::operator=(const Vec& v)
{ 
    if (!isNull())
        (*this)->value << v;
    else
        PLERROR("Var::operator= called on null Var");
}

void Var::operator=(const Mat& m)
{ 
    if (!isNull())
        (*this)->matValue << m;
    else
        PLERROR("Var::operator= called on null Var");
}

int Variable::nvars = 0;

Variable::Variable(int thelength, int thewidth, bool call_build_):
    inherited(call_build_),
    varnum(++nvars), marked(false), varname(""),  
    allows_partial_update(false), gradient_status(0),
    matValue(thelength,thewidth), matGradient(thelength,thewidth), 
    min_value(-FLT_MAX), max_value(FLT_MAX), diaghessiandata(0), rvaluedata(0),
    dont_bprop_here(false)
{
    value = matValue.toVec();
    gradient = matGradient.toVec();
    if(value.getStorage())
        valuedata = value.data();
    else
        valuedata = 0;
    if (gradient.getStorage())
        gradientdata = gradient.data();
    else
        gradientdata = 0;
    if (call_build_)
        build_();
}

Variable::Variable(const Mat& m)
    :varnum(++nvars), marked(false), varname(""),  
     allows_partial_update(false), gradient_status(0),
     matValue(m), matGradient(m.length(),m.width()), 
     min_value(-FLT_MAX), max_value(FLT_MAX), diaghessiandata(0), rvaluedata(0),
     dont_bprop_here(false)
{
    if(!m.isCompact())
        PLERROR("To be able to construct a Var that views the same data as a Mat m, the Mat must be compact (width()==mod()). Maybe you can use m.copy() instead of m?");
    value = matValue.toVec();
    gradient = matGradient.toVec();
    if(value.getStorage())
        valuedata = value.data();
    else
        valuedata = 0;
    if (gradient.getStorage())
        gradientdata = gradient.data();
    else
        gradientdata = 0;
}

// shallow copy (same as default copy constructor, except varnum is set to ++nvars.
Variable::Variable(const Variable& v)
    :varnum(++nvars), marked(false), varname(v.getName()), 
     allows_partial_update(v.allows_partial_update), gradient_status(v.gradient_status),
     value(v.value), gradient(v.gradient), 
     matValue(v.matValue),matGradient(v.matGradient),
     valuedata(v.valuedata), gradientdata(v.gradientdata),
     min_value(v.min_value),max_value(v.max_value),
     g(v.g), diaghessian(v.diaghessian), diaghessiandata(v.diaghessiandata),
     rvaluedata(v.rvaluedata), dont_bprop_here(v.dont_bprop_here)
{}


void Variable::declareOptions(OptionList& ol)
{
    declareOption(ol, "varname", &Variable::varname, OptionBase::buildoption, 
                  "An (optional) name for the variable\n");

    declareOption(ol, "value", &Variable::matValue, OptionBase::learntoption, 
                  "Current value of the variable\n");

    /*
      declareOption(ol, "gradient", &Variable::matGradient, OptionBase::learntoption, 
      "Current gradient of the variable\n");
    */

    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void Variable::build_()
{ 
    int l_previous = length();
    int w_previous = width();
    int l, w;
    recomputeSize(l, w);
    if (l && w && (l != l_previous || w != w_previous))
        resize(l, w);
}

///////////
// build //
///////////
void Variable::build()
{
    inherited::build();
    build_();
}


///////////////////
// recomputeSize //
///////////////////
void Variable::recomputeSize(int& l, int& w) const
{ l = length(); w = width(); }

////////////
// resize //
////////////
void Variable::resize(int l, int w)
{
    value = Vec(); 
    // Force mod == width so that the call to 'toVec()' below does not crash.
    matValue.setMod(w);
    matValue.resize(l,w);
    value = matValue.toVec();
    if(value.getStorage())
        valuedata = value.data();
    else
        valuedata = 0;

    gradient = Vec();
    // Same as above.
    matGradient.setMod(w);
    matGradient.resize(l,w);
    gradient = matGradient.toVec();
    if (gradient.getStorage())
        gradientdata = gradient.data();
    else
        gradientdata = 0;
}
  
//////////////
// sizeprop //
//////////////
void Variable::sizeprop()
{
    int l,w;
    recomputeSize(l,w);
    resize(l,w);
}

////////////////
// setParents //
////////////////
void Variable::setParents(const VarArray& parents)
{ PLERROR("In Variable::setParents  setParents() function not implemented for %s", classname().c_str()); }

/////////////////////////
// defineValueLocation //
/////////////////////////
Mat Variable::defineValueLocation(const Mat& m)
{
    if(!m.isCompact())
        PLERROR("In Variable::defineValueLocation, Variables require compact"
                " matrices");
    Mat oldm = matValue;
    matValue = m;
    value = matValue.toVec();
    if(value.getStorage())
        valuedata = value.data();
    else
        valuedata = 0;
    gradient = Vec(); // Temporarily frees a reference to gradient's storage.
    matGradient.setMod(matValue.width());
    matGradient.resize(matValue.length(), matValue.width());
    gradient = matGradient.toVec();
    if (gradient.getStorage())
        gradientdata = gradient.data();
    else
        gradientdata = 0;
    return oldm;
}

////////////////////////////
// defineGradientLocation //
////////////////////////////
Mat Variable::defineGradientLocation(const Mat& m)
{
    if(!m.isCompact())
        PLERROR("In Variable::defineGradientLocation, Variables require"
                " compact matrices");
    Mat oldm = matGradient;
    matGradient = m;
    gradient  = matGradient.toVec();
    if (gradient.getStorage())
        gradientdata = gradient.data();
    else
        gradientdata = 0;
    value = Vec(); // Temporarily frees a reference to value's storage.
    matValue.setMod(matGradient.width());
    matValue.resize(matGradient.length(), matGradient.width());
    value = matValue.toVec();
    if(value.getStorage())
        valuedata = value.data();
    else
        valuedata = 0;
    return oldm;
}

void Variable::newwrite(PStream& out) const
{ 
    switch(out.outmode)
    {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
    {
        // This is just to strip "Variable" out of the class name (as they all
        // end in "Variable")
        string cn=info();
        string::size_type len = cn.length();
        if (len >= 9 && cn.substr(len-8,8) == "Variable")
            out << cn.substr(0,len-8) << endl;
        else
            out << cn << endl;
        break;
    }
    default:
        inherited::newwrite(out);
    }
}

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(Variable,
                                 "The base Variable class",
                                 ""
    );

void Variable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Object::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(value, copies);
    deepCopyField(gradient, copies);
    deepCopyField(matValue, copies);
    deepCopyField(matGradient, copies);
    if (value.getStorage())
        valuedata = value.data();
    else
        valuedata = 0;
    if (gradient.getStorage())
        gradientdata = gradient.data();
    else
        gradientdata = 0;
    varDeepCopyField(g, copies);
}

void Variable::clearDiagHessian() 
{ 
    if(!diaghessian) 
        resizeDiagHessian();
    diaghessian.clear();
}


void Variable::fbprop()
{
    fprop();
    bprop();
}

void Variable::fbbprop()
{
    fprop();
    bprop();
    bbprop();
}

void Variable::bbprop()
{ PLERROR("bbprop not implemented for this variable (%s)",classname().c_str()); }

void Variable::symbolicBprop()
{ PLERROR("symbolicBprop not implemented for this variable (%s)",classname().c_str()); }

void Variable::rfprop()
{ PLERROR("rfprop not implmented for this variable (%s)",classname().c_str()); }

void Variable::setName(const string& the_name)
{ varname = the_name; }

string Variable::getName() const
{
    if (varname.size() == 0)
        return "#" + tostring(varnum);

    return varname;
}

void Variable::oldread(istream& in)
{ PLearn::read(in, value); }

void Variable::write(ostream& out) const
{ PLearn::write(out, value); }


Var Variable::subVec(int start, int len, bool transpose)
{
    if(isColumnVec())
        return subMat(start,0,len,1,transpose);
    else if(isRowVec())
        return subMat(0,start,1,len,transpose);

    PLERROR("In Variable::subVec variable is not a vec (single column or single row)");
    return Var();
}

Var Variable::subMat(int i, int j, int sublength, int subwidth, bool do_transpose)
{ 
    if(do_transpose)
        return new SubMatTransposeVariable(this, i, j, sublength, subwidth); 
    else
        return new SubMatVariable(this, i, j, sublength, subwidth); 
}

void Variable::fprop_from_all_sources() 
{
    VarArray all_sources = sources();
    unmarkAncestors();
    VarArray prop_path = propagationPath(all_sources,Var(this));
    prop_path.fprop();
}

void Variable::printInfos(bool print_gradient)
{
    VarArray ancetres = ancestors();
    unmarkAncestors();
    ancetres.printInfo(print_gradient);
}

void Variable::accg(Var vg)
{
    if(g || (vg.length()==length() && vg.width()==width()))
        g += vg;
    else // g does not exist
    {
        g = Var(length(),width());
        g += vg;
    }
}

void Variable::verifyGradient(real step) 
{ 
    VarArray inputs = sources();
    unmarkAncestors();
    Func f(inputs,Var(this));
    Vec p(inputs.nelems());
    inputs >> p;
    f->verifyGradient(p,step);
}

// set value = value + (step_size*coeff + b) * direction
// with step_size possibly scaled down s.t. box constraints are satisfied
// return true if box constraints have been hit with the update

bool Variable::update(real step_size, Vec direction_vec, real coeff, real b)
{
    static bool hit;
    static real full_coeff;
    if(allows_partial_update)
        PLWARNING("Warning in Variable::update(real,Vec): will update every elements of the Variable");
    hit = false;
    full_coeff = step_size * coeff + b;
    if(min_value>-FLT_MAX || max_value<FLT_MAX)
        // constrained update
    {
        real* direction = direction_vec.data();
        for(int i=0; i<nelems(); i++)
        {
            valuedata[i] += (full_coeff) * direction[i];      
            if(valuedata[i]<min_value)
            {
                valuedata[i]=min_value;
                hit = true;
            }
            else if(valuedata[i]>max_value)
            {
                valuedata[i]=max_value;
                hit = true;
            }
        }
    }
    else
        // unconstrained update
    {
        real* direction = direction_vec.data();
        for(int i=0; i<nelems(); i++)
        {
            valuedata[i] += (full_coeff) * direction[i];      
        }
    }
    return hit;
}

bool Variable::update(Vec step_sizes, Vec direction_vec, real coeff, real b)
{
    if(allows_partial_update)
        PLWARNING("Warning in Variable::update(Vec,Vec): will update every elements of the Variable");
    bool hit=false;
    real* direction = direction_vec.data();
    real* step = step_sizes.data();
    if(min_value>-FLT_MAX || max_value<FLT_MAX)
        // constrained update
    {
        for(int i=0; i<nelems(); i++)
        {
            valuedata[i] += (step[i] * coeff + b) * direction[i];
            if(valuedata[i]<min_value)
            {
                valuedata[i]=min_value;
                hit = true;
            }
            else if(valuedata[i]>max_value)
            {
                valuedata[i]=max_value;
                hit = true;
            }
        }
    }
    else
        // unconstrained update
        for(int i=0; i<nelems(); i++)
            valuedata[i] += (step[i] * coeff + b) * direction[i];
    return hit;
}

bool Variable::update(real step_size, bool clear)
{
    bool hit=false;
    if(min_value>-FLT_MAX || max_value<FLT_MAX)
        // constrained update
    {
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
                        params[i] += step_size*direction[i];      
                        if(params[i]<min_value)
                        {
                            params[i]=min_value;
                            hit = true;
                        }
                        else if(params[i]>max_value)
                        {
                            params[i]=max_value;
                            hit = true;
                        }
                        if (clear)
                            direction[i]=0;
                    }
                }
                if (clear) {
                    rows_to_update.resize(0);
                    gradient_status=0;
                }
            }
        }
        else for (int row=0;row<length();row++)
        {
            real* direction = matGradient[row];
            real* params = matValue[row];
            for(int i=0; i<width(); i++)
            {
                params[i] += step_size*direction[i];      
                if(params[i]<min_value)
                {
                    params[i]=min_value;
                    hit = true;
                }
                else if(params[i]>max_value)
                {
                    params[i]=max_value;
                    hit = true;
                }
                if (clear)
                    direction[i]=0;
            }
        }
    }
    else
        // unconstrained update
    {
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
                        params[i] += step_size*direction[i];      
                        if (clear)
                            direction[i] = 0;
                    }
                }
                if (clear) {
                    rows_to_update.resize(0);
                    gradient_status=0;
                }
            }
        }
        else for (int row=0;row<length();row++)
        {
            real* direction = matGradient[row];
            real* params = matValue[row];      
            for(int i=0; i<width(); i++)
            {
                params[i] += step_size*direction[i];      
                if (clear)
                    direction[i] = 0;
            }
        }
    }
    return hit;
}

///////////////////////////
// updateWithWeightDecay //
///////////////////////////
void Variable::updateWithWeightDecay(real step_size, real weight_decay, bool L1, bool clear)
{
    // we do unconstrained update only here
    if (allows_partial_update && gradient_status!=2)
    {
        if (gradient_status!=0)
        {
            for (int r=0;r<rows_to_update.length();r++)
            {
                int row = rows_to_update[r];
                real* direction = matGradient[row];
                real* params = matValue[row];
                if (L1)
                {
                    real delta = fabs(step_size)*weight_decay;
                    for(int i=0; i<width(); i++)
                    {
                        real pi = params[i];
                        params[i] += step_size*direction[i];
                        if (pi>delta)
                            params[i] -= delta;
                        else if (pi<-delta)
                            params[i] += delta;
                        else
                            params[i] = 0;
                        if (clear)
                            direction[i] = 0;
                    }
                }
                else // L2
                    for(int i=0; i<width(); i++)
                    {
                        params[i] += step_size*(direction[i] + weight_decay*params[i]);
                        if (clear)
                            direction[i] = 0;
                    }
            }
            if (clear) {
                rows_to_update.resize(0);
                gradient_status=0;
            }
        }
    }
    else
        for (int row=0;row<length();row++)
        {
            real* direction = matGradient[row];
            real* params = matValue[row];      
            if (L1)
            {
                real delta = fabs(step_size)*weight_decay;
                for(int i=0; i<width(); i++)
                {
                    real pi = params[i];
                    params[i] += step_size*direction[i];
                    if (pi>delta)
                        params[i] -= delta;
                    else if (pi<-delta)
                        params[i] += delta;
                    if (clear)
                        direction[i] = 0;
                }
            }
            else // L2
                for(int i=0; i<width(); i++)
                {
                    params[i] += step_size*(direction[i] + weight_decay*params[i]);
                    if (clear)
                        direction[i] = 0;
                }
        }
}


////////////////////
// updateAndClear //
////////////////////
void Variable::updateAndClear()
{
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
                    real& param_i = params[i];
                    param_i += direction[i];
                    if (param_i < min_value)
                        param_i = min_value;
                    else if (param_i > max_value)
                        param_i = max_value;
                    direction[i] = 0;
                }              
            }
            rows_to_update.resize(0);
            gradient_status=0;
        }
    }
    else 
    {
        for(int i=0; i<nelems(); i++) {
            real& value = valuedata[i];
            value += gradientdata[i];
            if (value < min_value)
                value = min_value;
            else if (value > max_value)
                value = max_value;
        }
        gradient.clear();
    }
}

// set value = value + step_size * gradient
// with step_size possibly scaled down s.t. box constraints are satisfied
// return true if box constraints have been hit with the update

/*
  bool Variable::update(real step_size)
  {
  bool hit=false;
  if(min_value>-FLT_MAX || max_value<FLT_MAX)
  // constrained update
  {
  real* direction = gradient.data();
  for(int i=0; i<nelems(); i++)
  {
  valuedata[i] += step_size*direction[i];      
  if(valuedata[i]<min_value)
  {
  valuedata[i]=min_value;
  hit = true;
  }
  else if(valuedata[i]>max_value)
  {
  valuedata[i]=max_value;
  hit = true;
  }
  }
  }
  else
  // unconstrained update
  {
  real* direction = gradient.data();
  for(int i=0; i<nelems(); i++)
  valuedata[i] += step_size*direction[i];      
  }

  return hit;
  }
*/


// set value = new_value
// projected down in each direction independently  in the
// subspace in which the box constraints are satisfied.
// return true if box constraints have been hit with the update
bool Variable::update(Vec new_value)
{
    if(allows_partial_update)
        PLWARNING("Warning in Variable::update(Vec): will update every elements of the Variable");
    bool hit=false;
    if(min_value>-FLT_MAX || max_value<FLT_MAX)
        // constrained update
    {
        real* new_v = new_value.data();
        for(int i=0; i<nelems(); i++)
        {
            valuedata[i] = new_v[i];      
            if(valuedata[i]<min_value)
            {
                valuedata[i]=min_value;
                hit = true;
            }
            else if(valuedata[i]>max_value)
            {
                valuedata[i]=max_value;
                hit = true;
            }
        }
    }
    else
        // unconstrained update
    {
        real* new_v = new_value.data();
        for(int i=0; i<nelems(); i++)
            valuedata[i] = new_v[i];      
    }
    return hit;
}

// Using the box constraints on the values, return
// the maximum allowable step_size in the given direction
// i.e., argmax_{step_size} {new = value + step_size * direction, new in box}
real Variable::maxUpdate(Vec direction) 
{
    real max_step_size=FLT_MAX;
    if(min_value>-FLT_MAX || max_value<FLT_MAX)
        // constrained update
    {
        real* dir = direction.data();
        for(int i=0; i<nelems(); i++)
        {
            real v = valuedata[i];
            if (v<min_value || v>max_value)
                PLERROR("Variable::maxUpdate:current value %f already out of bounds (%f,%f)!",
                        v,min_value,max_value);
            if (dir[i]>0) // want to increase value: check max_value
            {
                if (max_value<FLT_MAX) 
                {
                    real maxstep = (max_value - v)/dir[i];
                    if (maxstep < max_step_size) max_step_size = maxstep;
                }
            }
            else if (dir[i]<0) // want to decrease value: check min_value
            {
                if (min_value > -FLT_MAX)
                {
                    real maxstep = (min_value - v)/dir[i];
                    if (maxstep < max_step_size) max_step_size = maxstep;
                }
            }
        }
    }
    // else unconstrained 

    return max_step_size;
}

void Variable::makeSharedValue(real* x, int n)
{
    if (n!=nelems()) PLERROR("Variable::makeSharedValue, n(%d) inconsistent with nelems(%d)",
                             n,nelems());
    real* v=value.data();
    valuedata=x;
    if (x!=v)
        for (int j=0;j<n;j++)
            x[j]=v[j];
    value.storage = new Storage<real>(n,x);
    value.offset_ = 0;
    matValue.storage = value.storage;
    matValue.offset_ = 0;
    matValue.mod_ = matValue.width();
}

void Variable::makeSharedValue(PP<Storage<real> > storage, int offset_)
{
    int n=nelems();
    if (storage->length()<offset_+n) 
        PLERROR("Variable::makeSharedValue, storage(%d) too small({d+%d)",
                storage->length(),offset_,nelems());
    real* v=value.data();
    real* x=valuedata=storage->data+offset_;
    if (x!=v)
        for (int j=0;j<n;j++)
            x[j]=v[j];
    value.storage = storage;
    value.offset_ = offset_;
    matValue.storage = storage;
    matValue.offset_ = offset_;
    matValue.mod_ = matValue.width();
}

void Variable::makeSharedGradient(Vec& v, int offset_)
{
    makeSharedGradient(v.storage,v.offset_+offset_);
}

void Variable::makeSharedGradient(PP<Storage<real> > storage, int offset_)
{
    int n=nelems();
    if (storage->length()<offset_+n) 
        PLERROR("Variable::makeSharedGradient, storage(%d) too small({d+%d)",
                storage->length(),offset_,nelems());
    real* v=gradient.data();
    real* x=gradientdata=storage->data+offset_;
    if (x!=v)
        for (int j=0;j<n;j++)
            x[j]=v[j];
    gradient.storage = storage;
    gradient.offset_ = offset_;
    matGradient.storage = storage;
    matGradient.offset_ = offset_;
    matGradient.mod_ = matGradient.width();
}

  
void Variable::makeSharedGradient(real* x, int n)
{
    if (n!=nelems()) PLERROR("Variable::makeSharedGradient, n(%d) inconsistent with nelems(%d)",
                             n,nelems());
    real* v=gradient.data();
    gradientdata=x;
    if (x!=v)
        for (int j=0;j<n;j++)
            x[j]=v[j];
    gradient.storage = new Storage<real>(n,x);
    gradient.offset_ = 0;
    matGradient.storage = gradient.storage;
    matGradient.offset_ = 0;
    matGradient.mod_ = matGradient.width();
}

void Variable::makeSharedValue(Vec& v, int offset_)
{
    makeSharedValue(v.storage,v.offset_+offset_);
}

void Variable::makeSharedRValue(PP<Storage<real> > storage, int offset_)
{
    resizeRValue();
    int n=nelems();
    if (storage->length()<offset_+n) 
        PLERROR("Variable::makeSharedRValue, storage(%d) too small({d+%d)",
                storage->length(),offset_,nelems());
    real* v=rValue.data();
    real* x=rvaluedata=storage->data+offset_;
    if (x!=v)
        for (int j=0;j<n;j++)
            x[j]=v[j];
    rValue.storage = storage;
    rValue.offset_ = offset_;
    matRValue.storage = storage;
    matRValue.offset_ = offset_;
    matRValue.mod_ = matRValue.width();
}

  
void Variable::makeSharedRValue(real* x, int n)
{
    if (n!=nelems()) PLERROR("Variable::makeSharedRValue, n(%d) inconsistent with nelems(%d)",
                             n,nelems());
    resizeRValue();
    real* v=rValue.data();
    rvaluedata=x;
    if (x!=v)
        for (int j=0;j<n;j++)
            x[j]=v[j];
    rValue.storage = new Storage<real>(n,x);
    rValue.offset_ = 0;
    matRValue.storage = rValue.storage;
    matRValue.offset_ = 0;
    matRValue.mod_ = matRValue.width();
}

void Variable::makeSharedRValue(Vec& v, int offset_)
{
    makeSharedRValue(v.storage,v.offset_+offset_);
}
    
void Variable::resizeDiagHessian()
{
    matDiagHessian.resize(length(),width());
    diaghessian = matDiagHessian.toVec();
    diaghessiandata = diaghessian.data();
}

void Variable::resizeRValue()
{
    if (!rvaluedata)
    {
        matRValue.resize(length(),width());
        rValue = matRValue.toVec();
        rvaluedata = rValue.data();
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
