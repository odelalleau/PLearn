/* *******************************************************
   * $Id: /u/lisa/dorionc/PLearn/PLearnLibrary/TestSuite/NeuralNetworks/simpleFunctions.cc
   * AUTHOR: Christian Dorion & Kim Levy
   ******************************************************* */

/*! \file PLearnLibrary/PLearnCore/TMat.h */


#include "simpleFunctions.h"

using namespace PLearn;
using namespace std;  

#define NB_SAMPLES 2000000
#define LOWER_BOUND -1000
#define HIGHER_BOUND 1000
#define N_HIDDEN 100
#define N_EPOCH 30

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


void simpleFunctions(SimpleFunction& f, const Var& x, const Var& y) //, const Var& z)
{
  f.implementVarGraph();
  f.applyVarFunction(x, y);
  DO_TEST("Testing simple function " + f.classname(), f.compareResults());
}

void choose_function(Sommation s, Hypothenuse h, XTimesExponentialY x_expy,
                     int& c, real x, real y, real& z)
{
  real rand = uniform_sample();
  
  //Sample From Class 1
  if(rand < 1/3){
    c = 1;
    z = s.function(x, y);
    return;
  }

  //Sample From Class 2
  if(rand < 2/3){
    c = 2;
    z = h.function(x, y);
    return;
  }

  //Sample From Class 3
  c = 2;
  z = x_expy.function(x, y);
}

void NN_Application(Sommation s, Hypothenuse h, XTimesExponentialY x_expy)
{
  real x=0, y=0, z=0;
  int c=0;
  Mat samples(NB_SAMPLES, 4);
  Mat::rowelements_iterator row_it = 0;
     
  cout << "Generating " << NB_SAMPLES << " Samples: " << endl
       << "LOWER_BOUND: " << LOWER_BOUND 
       << ", HIGHER_BOUND: " << HIGHER_BOUND << endl;

  for(int sample=0; sample < NB_SAMPLES; sample++)
    {
      //cout << "." << flush;

      x = bounded_uniform(LOWER_BOUND, HIGHER_BOUND);
      y = bounded_uniform(LOWER_BOUND, HIGHER_BOUND);
      choose_function(s, h, x_expy, c, x, y, z);

      row_it = samples.rowelements_begin(sample);
      *row_it = x; row_it++;
      *row_it = y; row_it++;
      *row_it = z; row_it++;
      *row_it = c; 

      //if(sample%1000 == 999)
      //  cout << endl;
    }
  
  VMat trainset(samples);
  int inputsize = samples.width()-1;
  int nclasses = 3;
  int nhidden = N_HIDDEN;
  int nepoch = N_EPOCH;
  
  cout << " ------------ NN_Application !------------------" << endl
       << "- " << NB_SAMPLES << " samples" << endl
       << "- " << nclasses << " classes " << endl
       << "- " << inputsize << " inputs " << endl 
       << "- " << nhidden << " hidden" << endl
       << " ---------------------------------------" << endl;
  
  Var input(inputsize, "INPUT");
  Var classnum(1, "CLASSNUM");
  
  Var w1(1+inputsize, nhidden,"W1");
  fill_random_uniform(w1->value, -1./sqrt(inputsize), +1./sqrt(inputsize));

  Var hidden = tanh(affine_transform(input,w1));
  hidden->setName("HIDDEN");

  Var w2(1+nhidden, nclasses, "W2");
  fill_random_uniform(w2->value, -1./sqrt(nhidden), +1./sqrt(nhidden));

  Var output = softmax(affine_transform(hidden,w2));
  output->setName("OUTPUT");

  // Should be 1ndic
  Var cost = neg_log_pi(output,classnum);
  cost->setName("COST");

  //displayVarGraph(cost);

  PLearn::VarArray params = w1&w2;
  Var classerror = classification_loss(output, classnum);
  classerror->setName("CLASS_ERROR");

  Var totalcost = meanOf(trainset, Func(input&classnum, hconcat(cost&classerror)), 1);

  //   GradientOptimizer(VarArray the_params, Var the_cost,
  //                     real the_start_learning_rate=0.01, 
  //                     real the_decrease_constant=0.01,
  //                     int n_updates=1, const string& filename="", 
  //                     int every_iterations=1)
  GradientOptimizer opt(params, totalcost, 0.01, 0.001, trainset.length()*nepoch, "NN_App.log", trainset.length());
  opt.optimize();

  cout << "------------End of NN_App !---------------" << endl << endl;
}
  
int main()
{
  DO_TEST("scalarVarTest", scalarVarTest());
  DO_TEST("matrixVarTest", matrixVarTest());
   
  Var x(1, "x"); x = 0;
  Var y(1, "y"); y = 1;
        
  Sommation s;
  simpleFunctions(s, x, y);

  Hypothenuse h;
  simpleFunctions(h, x, y);

  XTimesExponentialY x_expy;
  simpleFunctions(x_expy, x, y);

  NN_Application(s, h, x_expy);
 
  return 0;
}
  
