
/* *******************************************************      
   * $Id: test_BinaryVariable_bbprop.cc,v 1.1 2002/10/14 19:28:32 dorionc Exp $
   * AUTHORS: Kim Levy & Christian Dorion
   * This file is part of the PLearn library.
   ******************************************************* */

/*! \file PLearnLibrary/TestSuite/test_PP.cc */
/*! 
  Tests the constructors and methods of PP.cc  
*/

#include "t_general.h"
#include "Var.h" 
#include "Variable.h"
#include "DisplayUtils.h" 

#define MAX_SIZE 11

using namespace PLearn;
using namespace std;

  //! x=z^2, y=z, x*y=z^3
  bool test_TSV()
  {
    // x=z^2, y=z, x*y=z^3
    Var z(1, "z"); 
    z = 3;
    
    Var x(1,"x");
    x = square(z);

    Var y(1, "y");
    y = 4;

    Var f(1, "f");
    f = z*x;

    VarArray v = propagationPath(f);
    v.fprop();
    v.clearGradient();
    f->gradientdata[0] = 1.0;
    v.bprop();
    // Not fonctionnal because some binaryVariables do not have a bbprop
    //f->resizeDiagHessian();
    //f->diaghessiandata[0] = 0.0;
    //v.clearDiagHessian();
    //v.bbprop();
    
    //displayVarGraph(f, true);
    
    
    cout << "f->value[0]: " << f->value[0] << endl; 
    T_ASSERT( f->value[0] == 27,
             "Forward propagation through the vars didn't give the expected result!");
    cout <<"Forward propagation through the vars has been verified" << endl << endl;
    
    cout << "z->gradientdata[0]: "<< z->gradientdata[0] << endl; 
    T_ASSERT( z->gradientdata[0]== 27,
             "1-Backward propagation through the vars didn't give the expected result!");
    cout <<"Backward propagation through the vars has been verified" << endl << endl;
    
    //! Not implemented yet!!!
//     cout << "z->diaghessiandata[0]: "<< z->diaghessiandata[0] << endl; 
//     T_ASSERT( z->diaghessiandata[0]== 18,
//              "1-BackBackward propagation through the vars didn't give the expected result!");
//     cout <<"BackBackward propagation through the vars has been verified" << endl << endl; 
    
    return true;
  }

// --------------- main ------------------------------------------------------

int main(int argc, char** argv)
{
  DO_TEST("Testing the bbprop function of TimesScalarVariable",test_TSV());
}
