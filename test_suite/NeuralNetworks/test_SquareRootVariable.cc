/* *******************************************************
   * $Id: /u/lisa/dorionc/PLearn/PLearnLibrary/TestSuite/NeuralNetworks/simpleFunctions.cc
   * AUTHOR: Christian Dorion & Kim Levy
   ******************************************************* */

/*! \file PLearnLibrary/PLearnCore/TMat.h */

#ifndef __TEST_SRV_CC_
#define __TEST_SRV_CC_

#include "test_SquareRootVariable.h"

using namespace PLearn;
using namespace std;

  real f1(real x, real y, real z) { return ( sqrt(x)+sqrt(y) )/ sqrt(z); }
  real f2(real x, real y, real z) { return exp(sqrt((x+y)/z)); }
  real f(real x, real y, real z)
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return f1(x,y,z) * f2(x,y,z);
  }
  
  
  real f3(real x, real y, real z) { return 0.5/sqrt((x+y)/z) *(1/z); }
  //! df3_dy is equivalent to df3_dx
  real df3_dx(real x, real y, real z) { return 0.5*(-1/2)*pow((x+y)/z,-3/2) *(1/z); }
  

  

  real df1_dx(real x, real y, real z) 
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return 0.5/(sqrt(x)*sqrt(z));
  }

  real ddf1_dx(real x, real y, real z)
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return 0.5/sqrt(z) * (-1/2) * pow(x, -3/2);
  }

  real df1_dy(real x, real y, real z) 
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return 0.5/(sqrt(y)*sqrt(z));
  }

  real ddf1_dy(real x, real y, real z)
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return 0.5/sqrt(z) * (-1/2) * pow(y, -3/2);
  }   
  



  //! df2_dy is equivalent to df2_dx
  real df2_dx(real x, real y, real z) 
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return f2(x,y,z)*f3(x,y,z);
  } 

  //! ddf2_dx is equivalent to ddf2_dx
  real ddf2_dx(real x, real y, real z)
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return df2_dx(x,y,z)*f3(x,y,z) + f2(x,y,z)*df3_dx(x,y,z);
  }


  

  real df_dx(real x, real y, real z)
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return df1_dx(x,y,z)*f2(x,y,z) + f1(x,y,z)*df2_dx(x,y,z);
  }

  real ddf_dx(real x, real y, real z)
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return 
      ddf1_dx(x,y,z) * f2(x,y,z) +
      2 * df1_dx(x,y,z) * df2_dx(x,y,z) +
      f1(x,y,z) * ddf2_dx(x,y,z);
  }
 
  //! reminder: df2_dx<=>df2_dy and ddf2_dx<=>ddf2_dy 
  real df_dy(real x, real y, real z)
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return df1_dy(x,y,z)*f2(x,y,z) + f1(x,y,z)*df2_dx(x,y,z);
  }

  //! reminder: df2_dx<=>df2_dy and ddf2_dx<=>ddf2_dy
  real ddf_dy(real x, real y, real z)
  {
    T_ASSERT(z!=0 && x!=0 && y!=0, "Some of the given values are zero");
    return 
      ddf1_dy(x,y,z) * f2(x,y,z) +
      2 * df1_dy(x,y,z) * df2_dx(x,y,z) +
      f1(x,y,z) * ddf2_dx(x,y,z);
  }

  



  /*! Verifies fprop and bprop for 'SquareRootVariable'
    The function used is f = (sqrt(x)+sqrt(y))/sqrt(z) * exp(sqrt((x+y)/z))*/
  bool squareRootTest()
  {
    cout << "-------- Manual results --------" << endl; 
    cout << "--------------------------------" << endl; 
    cout << "f(9, 1, 4)= " << ::f(9, 1, 4) << endl;
    cout << "df_dx(9, 1, 4)= " << df_dx(9, 1, 4) << endl;
    cout << "df_dy(9, 1, 4)= " << df_dy(9, 1, 4) << endl;
    cout << "ddf_dx(9, 1, 4)= " << ddf_dx(9, 1, 4) << endl;
    cout << "ddf_dy(9, 1, 4)= " << ddf_dy(9, 1, 4) << endl << endl;    

    Var x(1, "x");
    x = 9;
    
    Var y(1, "y");
    y = 1;

    Var z(1, "z");
    z = 4;

    Var f1 = (squareroot(x) + squareroot(y)) / squareroot(z);
    f1->setName("f1");

    Var f2 = exp(squareroot((x+y)/z));
    f2->setName("f2");

    Var f = f1*f2;
    f->setName("function f");
    
    //! Propagates the values of x, y & z to get the value of f 
    VarArray v = propagationPath(f);
    v.fprop();
    f->gradientdata[0] = 1.0;
    v.bprop();
    //f->resizeDiagHessian();
    //f->diaghessiandata[0] = 1.0;
    //v.bbprop();
 
    //displayVarGraph(f, true);
 
    cout << "----------- Var results --------" << endl;
    cout << "--------------------------------" << endl;  
    
    cout << "f {x=9, y=1, z=4}: \n" << f << endl;
    T_ASSERT(f->value[0] == ::f(x->value[0], y->value[0], z->value[0]),
             "Forward propagation through the vars didn't give the expected result!");
    cout <<"Forward propagation through the vars has been verified" << endl << endl; 


    cout << "df_dx {x=9, y=1, z=4}:\n" << x->gradientdata[0] << endl;   
    T_ASSERT(REL_FEQUAL( x->gradientdata[0], ::df_dx(x->value[0], y->value[0], z->value[0]) ),
             "1-Backward propagation through the vars didn't give the expected result!");
    cout << "df_dy {x=9, y=1, z=4}:\n" << y->gradientdata[0] << endl;   
    T_ASSERT(REL_FEQUAL( y->gradientdata[0], ::df_dy(x->value[0], y->value[0], z->value[0]) ),
             "2-Backward propagation through the vars didn't give the expected result!"); 
    cout <<"Backward propagation through the vars has been verified" << endl << endl;

//     ! Not implemented yet!!!
//     cout << "ddf_dx {x=9, y=1, z=4}:\n" << x->diaghessiandata[0] << endl;   
//     T_ASSERT(REL_FEQUAL( x->diaghessiandata[0], ::ddf_dx(x->value[0], y->value[0], z->value[0]) ),
//              "1-BackBack propagation through the vars didn't give the expected result!"); 
//     cout << "ddf_dy {x=9, y=1, z=4}:\n" << y->diaghessiandata[0] << endl;   
//     T_ASSERT(REL_FEQUAL( y->diaghessiandata[0], ::ddf_dy(x->value[0], y->value[0], z->value[0]) ),
//              "2-BackBack propagation through the vars didn't give the expected result!");
  
//     cout << "------SquareRootVariable verified------" << endl << endl;
    return true;
  }

  
  int main()
  {
    DO_TEST("Testing the SquareRootVariable",squareRootTest());
    return 0;
  }
  
#endif // ifndef __TEST_SRV_CC_
