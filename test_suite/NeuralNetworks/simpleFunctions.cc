/* *******************************************************
   * $Id: /u/lisa/dorionc/PLearn/PLearnLibrary/TestSuite/NeuralNetworks/simpleFunctions.cc
   * AUTHOR: Christian Dorion & Kim Levy
   ******************************************************* */

/*! \file PLearnLibrary/PLearnCore/TMat.h */

#include "simpleFunctions.h"

using namespace PLearn;
using namespace std;  

  real f1(real x, real y, real z) { return sqrt((square(x) + square(y) + square(z))); }
  real f2(real x, real y, real z) { return exp(square((x-y)/(z+1000))); }
  real f(real x, real y, real z)
  {
    if(z == -1000)
      return 0;
    return f1(x,y,z) * f2(x,y,z);
  }
  
  bool scalarVarTest()
  {
    Var x(1, "x");
    x = 0;
    cout << x << endl;

    Var y(1, "y");
    y = 1;
    cout << y << endl;

    Var z(1, "z");
    z = 2;
    cout << z << endl;

    Var f1 = sqrt(square(x) + square(y) + square(z));
    f1->setName("f1");

    Var mille(1, "mille");
    mille = 1000;

    Var f2 = exp(square((x-y)/(z+mille)));
    f2->setName("f2");

    Var f = f1*f2;
    f->setName("function f");
    
    // Propagates the values of x, y & z to get the value of f 
    propagationPath(f).fprop();
    cout << f << endl;
    
    //displayVarGraph(f, true);
    T_ASSERT( REL_FEQUAL(f->value[0], ::f(x->value[0], y->value[0], z->value[0])),
             "Propagation through the vars didn't gave the expected result!");
  
    return true;
  }

  bool matrixVarTest()
  {
    Array< Mat > matArray(3);
    loadSameSize<real>(FMATDIR "/Random/", matArray);
    
    Var x(matArray[0].length(), matArray[0].width(), "x");
    x = matArray[0];
    cout << x << endl;
    
    Var y(matArray[1].length(), matArray[1].width(), "y");
    y = matArray[1];
    cout << y << endl;

    Var z(matArray[2].length(), matArray[2].width(), "z");
    z = matArray[2];
    cout << z << endl;
    
    Var f1 = sqrt(square(x) + square(y) + square(z));
    f1->setName("f1");

    Var mille(1, "mille");
    mille = 1000;
    
    Var f2 = exp(square((x-y)/(z+mille)));
    f2->setName("f2");

    Var f = f1*f2;
    f->setName("function f");
    
    // Propagates the values of x, y & z to get the value of f 
    propagationPath(f).fprop();
    cout << f << endl;

    Mat fmat = f->matValue;
    
    for(int r=0; r<fmat.length(); r++)
      for(int c=0; c<fmat.width(); c++){
        if( is_missing(fmat(r,c)) ) raw_input(tostring( ::f(matArray[0](r,c), matArray[1](r,c), matArray[2](r,c)) ));
        T_ASSERT( REL_FEQUAL(fmat(r,c),::f(matArray[0](r,c), matArray[1](r,c), matArray[2](r,c))),
                 "Propagation through the vars didn't gave the expected result!");
      }
    
    return true;
  }


  void simpleFunctions(SimpleFunction& f, const Var& x, const Var& y, const Var& z)
  {
    f.implementVarGraph();
    f.applyVarFunction(x, y, z);
    DO_TEST("Testing simple function " + f.getClassName(), f.compareResults());
  }
  
  int main()
  {
    DO_TEST("scalarVarTest", scalarVarTest());
    DO_TEST("matrixVarTest", matrixVarTest());
   
    Var x(1, "x"); x = 0;
    Var y(1, "y"); y = 1;
    Var z(1, "z"); z = 2;
    
    Sommation s;
    simpleFunctions(s, x, y, z);

    Hypothenuse h;
    simpleFunctions(h, x, y, z);
    
    return 0;
  }
  
