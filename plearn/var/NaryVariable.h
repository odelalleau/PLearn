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
   * $Id: NaryVariable.h,v 1.3 2002/09/23 20:31:11 wangxian Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/NaryVariable.h */

#ifndef NARYVARIABLE_INC
#define NARYVARIABLE_INC

#include "VarArray.h"
#include "TMat.h"
#include "VMat.h"
#include "Func.h"
#include "Popen.h"

namespace PLearn <%
using namespace std;


class NaryVariable: public Variable
{
  typedef Variable inherited;
    
protected:
  //!  Default constructor for persistence
  NaryVariable() {}
  
public: // Temporarily public for GradientOptimizer hack (speed contest, Pascal)
  VarArray varray;

public:
  NaryVariable(const VarArray& the_varray, int thelength, int thewidth=1);
  DECLARE_ABSTRACT_NAME_AND_DEEPCOPY(NaryVariable);
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;

  virtual bool markPath();
  virtual void buildPath(VarArray& proppath);
  virtual VarArray sources();
  virtual VarArray random_sources();
  virtual VarArray ancestors();
  virtual void unmarkAncestors();
  virtual VarArray parents();
  void printInfo(bool print_gradient) { 
    cout <<  getName() << "[" << (void*)this << "] " << classname() << "(" << (void*)varray[0];
    for (int i=1;i<varray.size();i++)
      cout << "," << (void*)varray[i];
    cout << ") = " << value;
    if (print_gradient) cout << " gradient=" << gradient;
    cout << endl; 
  }
  virtual void resizeRValue();
};

//!  concatenation of the rows of several variables
class ConcatRowsVariable: public NaryVariable
{
protected:
  //!  protected default constructor for persistence
  ConcatRowsVariable() {}

public:
  //!  all the variables must have the same number of columns
  ConcatRowsVariable(const VarArray& vararray);
  DECLARE_NAME_AND_DEEPCOPY(ConcatRowsVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};

//!  concatenation of the columns of several variables
class ConcatColumnsVariable: public NaryVariable
{
protected:
  //!  protected default constructor for persistence
  ConcatColumnsVariable() {}

public:
  //!  all the variables must have the same number of rows
  ConcatColumnsVariable(const VarArray& vararray);
  DECLARE_NAME_AND_DEEPCOPY(ConcatColumnsVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void deepRead(istream& in, DeepReadMap& old2new);
  virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

class SumOfVariable: public NaryVariable
{
  protected:
    //!  protected default constructor for persistence
    SumOfVariable() : distr(), f(), nsamples(), curpos() {}

  public:
  //protected:
    VMat distr;
    Func f;
    int nsamples;
    int curpos; //!<  current pos in VMat 
    // To avoid allocation/deallocations in fprop/bprop
    Vec input_value;
    Vec input_gradient;
    Vec output_value;
    
  public:
    //!  Sum_{inputs \in distr} f(inputs)
    SumOfVariable(VMat the_distr, Func the_f, int the_nsamples=-1);
    
    DECLARE_NAME_AND_DEEPCOPY(SumOfVariable);
    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
    virtual void fprop();
    virtual void bprop();
    virtual void fbprop();
    virtual void symbolicBprop();
    virtual void rfprop();
    
    void printInfo(bool print_gradient);
};

class MatrixSumOfVariable: public NaryVariable
{
  protected:
    //!  protected default constructor for persistence
    MatrixSumOfVariable() : distr(), f(), nsamples(), input_size(), curpos() {}

  public:
  //protected:
    VMat distr;
    Func f;
    int nsamples;
    int input_size;
    int curpos; //!<  current pos in VMat 
    // To avoid allocation/deallocations in fprop/bprop
    Vec input_value;
    Vec input_gradient;
    Vec output_value;
    
  public:
    //!  Sum_{inputs \in distr} f(inputs)
    MatrixSumOfVariable(VMat the_distr, Func the_f, int the_nsamples=-1, int the_input_size=-1);
    
    DECLARE_NAME_AND_DEEPCOPY(MatrixSumOfVariable);
    virtual void recomputeSize(int& l, int& w) const;
    virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
    virtual void fprop();
    virtual void bprop();
    virtual void fbprop();
    virtual void symbolicBprop();
    virtual void rfprop();
    
    void printInfo(bool print_gradient);
};


class ConcatOfVariable: public NaryVariable
{
protected:
  //!  protected default constructor for persistence
  ConcatOfVariable() {}

protected:

    VMat distr;
    Func f;

    Vec input_value;  //!<  Vec to hold one input sample
    Vec input_gradient; //!<  //!<  Vec to hold the gradient for one input sample

public:
    ConcatOfVariable(VMat the_distr, Func the_f);
  DECLARE_NAME_AND_DEEPCOPY(ConcatOfVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void fprop();
  virtual void bprop();
  virtual void fbprop();
};

/*! returns the value of v within the_values_of_v that gives the lowest
   value of expression (which may depend on inputs). */
class ArgminOfVariable: public NaryVariable
{
protected:
  //!  protected default constructor for persistence
  ArgminOfVariable() : inputs(), expression(), values_of_v(), v(),
                       index_of_argmin(), vv_path(), e_path(), v_path() {}

protected:
  VarArray inputs;
  Var expression;
  Var values_of_v;
  Var v;
  int index_of_argmin;

  VarArray vv_path; //!<  values_of_v(inputs)
  VarArray e_path; //!<  expression(v&inputs)
  VarArray v_path; //!<  expression(v) 

public:
  ArgminOfVariable(Variable* the_v,
                   Variable* the_expression,
                   Variable* the_values_of_v,
                   const VarArray& the_inputs);
  DECLARE_NAME_AND_DEEPCOPY(ArgminOfVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void fprop();
  virtual void bprop();
};

class MatrixElementsVariable: public NaryVariable
{
protected:
  //!  protected default constructor for persistence
  MatrixElementsVariable() : i(), j(), ni(), nj(), expression(),
                             parameters(), full_fproppath(), fproppath(),
                             bproppath() {}

protected:
  Var i;
  Var j;
  int ni;
  int nj;
  Var expression;
  VarArray parameters;

  VarArray full_fproppath; //!<  output(inputs&parameters)
  VarArray fproppath; //!<  output(inputs)
  VarArray bproppath; //!<  output(parameters)

public:
  MatrixElementsVariable(Variable* the_expression, const Var& i_index,
                         const Var& j_index,
                         int number_of_i_values, int number_of_j_values,
                         const VarArray& the_parameters);
  DECLARE_NAME_AND_DEEPCOPY(MatrixElementsVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void fprop();
  virtual void bprop();
  virtual void fbprop();
};


//!  Variable that is the element of the input1 VarArray indexed 
//!  by the input2 variable 
class VarArrayElementVariable: public NaryVariable
{
protected:
  //!  protected default constructor for persistence
  VarArrayElementVariable() {}

public:
  VarArrayElementVariable(VarArray& input1, const Var& input2);
  DECLARE_NAME_AND_DEEPCOPY(VarArrayElementVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

/*!   Variable that represents the element-wise IF-THEN-ELSE:
  the first parent is the test (0 or different from 0),
  the second parent is the value returned when the test is !=0,
  the third parent is the value returned when the test is ==0,
*/
class IfThenElseVariable: public NaryVariable
{
protected:
  //!  Default constructor for persistence
  IfThenElseVariable() {}

public:
  IfThenElseVariable(Var IfVar, Var ThenVar, Var ElseVar);
  DECLARE_NAME_AND_DEEPCOPY(IfThenElseVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
  Var& If() { return varray[0]; }
  Var& Then() { return varray[1]; }
  Var& Else() { return varray[2]; }
};


%> // end of namespace PLearn

#endif





