#include "Var_all.h"
#include "NaryVariable.h"
#include "Func.h"
#include "GradientOptimizer.h"
#include "VarArray.h"
#include "databases.h"
#include "random.h"

//#include "DisplayUtils.h"

using namespace PLearn;
using namespace std;

void net2W(int nhidden, int nepoch, int nclasses, VMat trainset)
{
  int inputsize = trainset.width()-1; 
  
  cout << " ------------ net2W !------------------" << endl
       << "- " << trainset.length() << " samples" << endl
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
  
  Var cost = neg_log_pi(output,classnum);
  cost->setName("COST");
  
  //displayVarGraph(cost);

  VarArray params = w1&w2;
  Var classerror = classification_loss(output, classnum);
  classerror->setName("CLASS_ERROR");
  
  // Func(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs)
  Var totalcost = meanOf(trainset, Func(input&classnum, hconcat(cost&classerror)), 1);
  //totalcost->setName("COST");
  
//   GradientOptimizer(VarArray the_params, Var the_cost,
//                     real the_start_learning_rate=0.01, 
//                     real the_decrease_constant=0.01,
//                     int n_updates=1, const string& filename="", 
//                     int every_iterations=1)
  GradientOptimizer opt(params, totalcost, 0.01, 0.00, trainset.length()*nepoch, "compareNet.log", trainset.length());
  opt.optimize();

  cout << "------------End of net2W !---------------" << endl << endl;
}


void net3W(int nhidden1,int nhidden2, int nepoch, int nclasses, VMat trainset)
{
  int inputsize = trainset.width()-1; 
  
  cout << " ------------ net3W !------------------" << endl
       << "- " << trainset.length() << " samples" << endl
       << "- " << nclasses << " classes " << endl
       << "- " << inputsize << " inputs " << endl 
       << "- " << nhidden1 << " hidden1" << endl
       << "- " << nhidden2 << " hidden2" << endl  
       << " ---------------------------------------" << endl;
    
  Var input(inputsize, "INPUT");
  Var classnum(1, "CLASSNUM");
  
  Var w1(1+inputsize, nhidden1,"W1");
  fill_random_uniform(w1->value, -1./sqrt(inputsize), +1./sqrt(inputsize));
  
  Var hidden1 = tanh(affine_transform(input,w1));
  hidden1->setName("HIDDEN_1");
  
  Var w2(1+nhidden1, nhidden2, "W2");
  fill_random_uniform(w2->value, -1./sqrt(nhidden1), +1./sqrt(nhidden1));
  
  Var hidden2 = tanh(affine_transform(hidden1,w2));
  hidden2->setName("HIDDEN_2");
  
  Var w3(1+nhidden2, nclasses, "W3");
  fill_random_uniform(w3->value, -1./sqrt(nhidden2), +1./sqrt(nhidden2));

  Var output = softmax(affine_transform(hidden2,w3));
  output->setName("OUTPUT");

  
  Var cost = neg_log_pi(output ,classnum);
  cost->setName("COST");
  
  //displayVarGraph(cost);
  
  VarArray params = w1&w2&w3;
  Var classerror = classification_loss(output, classnum);
  classerror->setName("CLASS_ERROR");
  
  // Func(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs)
  Var totalcost = meanOf(trainset, Func(input&classnum, hconcat(cost&classerror)), 1);
  //totalcost->setName("COST");
  
//   GradientOptimizer(VarArray the_params, Var the_cost,
//                     real the_start_learning_rate=0.01, 
//                     real the_decrease_constant=0.01,
//                     int n_updates=1, const string& filename="", 
//                     int every_iterations=1)
  GradientOptimizer opt(params, totalcost, 0.01, 0.00, trainset.length()*nepoch, "compareNet.log", trainset.length());
  opt.optimize();

  cout << "------------End of net3W !---------------" << endl << endl;
}



void netNormalFill(int nhidden, int nepoch, int nclasses, VMat trainset)
{
  
  int inputsize = trainset.width()-1; 
  
  cout << " ------------ netNormalFill !------------------" << endl
       << "- " << trainset.length() << " samples" << endl
       << "- " << nclasses << " classes " << endl
       << "- " << inputsize << " inputs " << endl 
       << "- " << nhidden << " hidden" << endl
       << " ---------------------------------------" << endl;
  
  
  Var input(inputsize, "INPUT");
  Var classnum(1, "CLASSNUM");
  
  Var w1(1+inputsize, nhidden,"W1");

  //void fill_random_normal(const Mat& dest, real mean, real sdev)
  fill_random_normal(w1->value, -1./sqrt(inputsize), +1./sqrt(inputsize));
  
  Var hidden = tanh(affine_transform(input,w1));
  hidden->setName("HIDDEN");
  
  Var w2(1+nhidden, nclasses, "W2");
  fill_random_normal(w2->value, -1./sqrt(nhidden), +1./sqrt(nhidden));
  
  Var output = softmax(affine_transform(hidden,w2));
  output->setName("OUTPUT");
  
  Var cost = neg_log_pi(output,classnum);
  cost->setName("COST");
  
  //displayVarGraph(cost);

  VarArray params = w1&w2;
  Var classerror = classification_loss(output, classnum);
  classerror->setName("CLASS_ERROR");
  
  // Func(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs)
  Var totalcost = meanOf(trainset, Func(input&classnum, hconcat(cost&classerror)), 1);
  //totalcost->setName("COST");
  
//   GradientOptimizer(VarArray the_params, Var the_cost,
//                     real the_start_learning_rate=0.01, 
//                     real the_decrease_constant=0.01,
//                     int n_updates=1, const string& filename="", 
//                     int every_iterations=1)
  GradientOptimizer opt(params, totalcost, 0.01, 0.00, trainset.length()*nepoch, "compareNet.log", trainset.length());
  opt.optimize();

  cout << "------------End of netNormalFill !---------------" << endl << endl;
}

void netSigmoid(int nhidden, int nepoch, int nclasses, VMat trainset)
{
  
  int inputsize = trainset.width()-1; 
  
  cout << " ------------ netSigmoid !------------------" << endl
       << "- " << trainset.length() << " samples" << endl
       << "- " << nclasses << " classes " << endl
       << "- " << inputsize << " inputs " << endl 
       << "- " << nhidden << " hidden" << endl
       << " ---------------------------------------" << endl;
  
  
  Var input(inputsize, "INPUT");
  Var classnum(1, "CLASSNUM");
  
  Var w1(1+inputsize, nhidden,"W1");

  //void fill_random_normal(const Mat& dest, real mean, real sdev)
  fill_random_normal(w1->value, -1./sqrt(inputsize), +1./sqrt(inputsize));
  
  Var hidden = sigmoid(affine_transform(input,w1));
  hidden->setName("HIDDEN");
  
  Var w2(1+nhidden, nclasses, "W2");
  fill_random_normal(w2->value, -1./sqrt(nhidden), +1./sqrt(nhidden));
  
  Var output = softmax(affine_transform(hidden,w2));
  output->setName("OUTPUT");
  
  Var cost = neg_log_pi(output,classnum);
  cost->setName("COST");
  
  //displayVarGraph(cost);

  VarArray params = w1&w2;
  Var classerror = classification_loss(output, classnum);
  classerror->setName("CLASS_ERROR");
  
  // Func(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs)
  Var totalcost = meanOf(trainset, Func(input&classnum, hconcat(cost&classerror)), 1);
  //totalcost->setName("COST");
  
//   GradientOptimizer(VarArray the_params, Var the_cost,
//                     real the_start_learning_rate=0.01, 
//                     real the_decrease_constant=0.01,
//                     int n_updates=1, const string& filename="", 
//                     int every_iterations=1)
  GradientOptimizer opt(params, totalcost, 0.01, 0.00, trainset.length()*nepoch, "compareNet.log", trainset.length());
  opt.optimize();

  cout << "------------End of netSigmoid !---------------" << endl << endl;
}

void netDecrease(int nhidden, int nepoch, int nclasses, VMat trainset)
{
  int inputsize = trainset.width()-1; 
  
  cout << " ------------ netDecrease !------------------" << endl
       << "- " << trainset.length() << " samples" << endl
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
  
  Var cost = neg_log_pi(output,classnum);
  cost->setName("COST");
  
  //displayVarGraph(cost);

  VarArray params = w1&w2;
  Var classerror = classification_loss(output, classnum);
  classerror->setName("CLASS_ERROR");
  
  // Func(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs)
  Var totalcost = meanOf(trainset, Func(input&classnum, hconcat(cost&classerror)), 1);
  //totalcost->setName("COST");
  
//   GradientOptimizer(VarArray the_params, Var the_cost,
//                     real the_start_learning_rate=0.01, 
//                     real the_decrease_constant=0.01,
//                     int n_updates=1, const string& filename="", 
//                     int every_iterations=1)
  GradientOptimizer opt(params, totalcost, 0.01, 0.000001, trainset.length()*nepoch, "compareNet.log", trainset.length());
  opt.optimize();

  cout << "------------End of netDecrease !---------------" << endl << endl;
}

void netLearningRate(int nhidden, int nepoch, int nclasses, VMat trainset)
{
  int inputsize = trainset.width()-1; 
  
  cout << " ------------ netLearningRate !------------------" << endl
       << "- " << trainset.length() << " samples" << endl
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
  
  Var cost = neg_log_pi(output,classnum);
  cost->setName("COST");
  
  //displayVarGraph(cost);

  VarArray params = w1&w2;
  Var classerror = classification_loss(output, classnum);
  classerror->setName("CLASS_ERROR");
  
  // Func(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs)
  Var totalcost = meanOf(trainset, Func(input&classnum, hconcat(cost&classerror)), 1);
  //totalcost->setName("COST");
  
//   GradientOptimizer(VarArray the_params, Var the_cost,
//                     real the_start_learning_rate=0.01, 
//                     real the_decrease_constant=0.01,
//                     int n_updates=1, const string& filename="", 
//                     int every_iterations=1)
  GradientOptimizer opt(params, totalcost, 0.05, 0.00, trainset.length()*nepoch, "compareNet.log", trainset.length());
  opt.optimize();

  cout << "------------End of netLearningRate !---------------" << endl << endl;
}


void netOneHot(int nhidden, int nepoch, int nclasses, VMat trainset)
{
  int inputsize = trainset.width()-1; 
  
  cout << " ------------ netOneHot !------------------" << endl
       << "- " << trainset.length() << " samples" << endl
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
  
  Var cost = onehot_squared_loss(output, classnum, 0, 1);
  cost->setName("COST");
  
  //displayVarGraph(cost);

  VarArray params = w1&w2;
  Var classerror = classification_loss(output, classnum);
  classerror->setName("CLASS_ERROR");
  
  // Func(const VarArray& the_inputs, const VarArray& parameters_to_optimize,const VarArray& the_outputs)
  Var totalcost = meanOf(trainset, Func(input&classnum, hconcat(cost&classerror)), 1);
  //totalcost->setName("COST");
  
//   GradientOptimizer(VarArray the_params, Var the_cost,
//                     real the_start_learning_rate=0.01, 
//                     real the_decrease_constant=0.01,
//                     int n_updates=1, const string& filename="", 
//                     int every_iterations=1)
  GradientOptimizer opt(params, totalcost, 0.01, 0.00, trainset.length()*nepoch, "compareNet.log", trainset.length());
  opt.optimize();

  cout << "------------End of netOneHot !---------------" << endl << endl;
}


int main()
{
  VMat trainset = loadLetters();
  cout << "Loading Letters..." << endl << endl;
  
  // Based on simplenet: 
  //net2W(100, 10, 26, trainset);// 0.27445-->0.0538
  //net3W(50,100, 10, 26, trainset);// 0.28785-->0.03425
  //netNormalFill(100, 10, 26, trainset);// 0.41905-->0.23245 
  //netSigmoid(100, 10, 26, trainset);// 0.4421-->0.1215 
  netDecrease(100, 10, 26, trainset);// 0.74095-->0.65175
  //netLearningRate(100, 10, 26, trainset);// 0.24885-->0.0677
  //netOneHot(100, 10, 26, trainset);// 0.4594-->0.11005
  return 0;
}
