
/* *******************************************************
   * $Id: /u/lisa/dorionc/PLearn/PLearnLibrary/TestSuite/NeuralNetworks/simpleFunctions.h
   * AUTHOR: Christian Dorion & Kim Levy
   ******************************************************* */

/*! \file PLearnLibrary/PLearnCore/TMat.h */

#ifndef __SIMPLE_FUNCTIONS_H_
#define __SIMPLE_FUNCTIONS_H_

#include "t_general.h" 
#include "TMat/TMat_utils.h"
#include "Var_all.h" 
#include "DisplayUtils.h" 

using namespace PLearn;
using namespace std; 

class SimpleFunction
{
private:
  bool varGraphImplemented;
  bool varFunctionApplyed;

protected: 
  Var varX;
  Var varY;
  Var varZ;
  Var varFunction;

  void setVarGraphImplemented(){ varGraphImplemented = true; }

public:
  SimpleFunction():
    varGraphImplemented(false), varFunctionApplyed(false),
    varX(1,"argument X of varFunction"),
    varY(1,"argument Y of varFunction"),
    varZ(1,"argument Z of varFunction"),
    varFunction(1,"Var rep of function")
  {}

  virtual string getClassName() { return "SimpleFunction";}
  virtual real function(real x, real y, real z) = 0;
  virtual void implementVarGraph() = 0; // must call setVarGraphImplemented !!!

  void displayVarGraph(bool values=false){  PLearn::displayVarGraph(varFunction, values); }

  Var applyVarFunction(const Var& x, const Var& y, const Var& z)
  {
    T_PRECONDITION(varGraphImplemented, "Must call implementVarGraph prior to applyVarFunction");

    varX->matValue.resize(x->matValue.length(), x->matValue.width());
    varX->matValue << x->matValue;

    varY->matValue.resize(y->matValue.length(), y->matValue.width());
    varY->matValue << y->matValue;
    
    varZ->matValue.resize(z->matValue.length(), z->matValue.width());
    varZ->matValue << z->matValue;
    
    propagationPath(varFunction).fprop();
    varFunctionApplyed = true;
    return PLearn::deepCopy( varFunction );
  }

  bool compareResults()
  {
    T_PRECONDITION(varFunctionApplyed, "Must call applyVarFunction prior to compareResults");
    Vec x = varX->value;
    Vec y = varY->value;
    Vec z = varZ->value;
    Vec res = varFunction->value;

    T_ASSERT( x.length() == y.length() 
              && y.length() == z.length() 
              && z.length() == res.length(),
              "Abnormal lengths in vars of SimpleFunctions!");
    
    Vec::iterator itX = x.begin();
    Vec::iterator itY = y.begin();
    Vec::iterator itZ = z.begin();
    Vec::iterator itRes = res.begin();
    for(int i=0; i<x.length(); i++){
      T_ASSERT( *itRes == function(*itX, *itY, *itZ),
                "Propagation through the var graph didn't give the expected results");
      ++itX; ++itY; ++itZ; ++itRes;
    }
    
    return true;
  }
  
  virtual ~SimpleFunction(){}
};  

class Sommation: public SimpleFunction
{
public:
  Sommation() : SimpleFunction() {}
  virtual string getClassName() { return "Sommation";}
  virtual real function(real x, real y, real z) { return x+y+z; }
  virtual void implementVarGraph(){
    varFunction = varX+varY+varZ;
    setVarGraphImplemented();
  }
};

class Hypothenuse : public SimpleFunction
{
public:
  Hypothenuse() : SimpleFunction() {}
  virtual string getClassName() { return "Hypothenuse";}
  virtual real function(real x, real y, real z) { return sqrt(square(x)+square(y)+square(z)); }
  virtual void implementVarGraph(){
    Var sqX = square(varX);
    sqX->setName("varX^2");
    
    Var sqY = square(varY);
    sqY->setName("varY^2");
    
    Var sqZ = square(varZ);
    sqZ->setName("varZ^2");

    varFunction = sqrt(sqX+sqY+sqZ);
    setVarGraphImplemented();
  }
};

class MinusHypo
{

};

class SqrtSquareExp
{

};  

class MinusSqrtSquareExp
{

};

class Weardo
{

};
  
#endif // ifndef __SIMPLE_FUNCTIONS_H_
