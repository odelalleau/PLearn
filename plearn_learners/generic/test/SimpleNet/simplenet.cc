// Uncommenting this will result ina namespace-reated bug
// that I scould not explain (Pascal)
// #include "MatIO.h"
#include <plearn/var/Var_all.h>
#include <plearn/var/NaryVariable.h>
#include <plearn/var/Func.h>
#include <plearn/opt/GradientOptimizer.h>
#include <plearn/var/VarArray.h>
#include <plearn/db/databases.h>
#include <plearn/math/random.h>

using namespace PLearn;

int main()
{
  try
  {
    // Implantation simplenet de Pascal
    // cout << "tanh: " << ultrafasttanh(1) << " " << fasttanh(1) << " " << tanh(1) << endl;
    
    int nhidden = 100;
    int nepochs = 10;
    
    VMat trainset = loadLetters();
    int nclasses = 26;
    int inputsize = trainset.width()-1;
    
    cout << "Letters: " << trainset.length() << " samples, "
         << nclasses << " classes, " << inputsize << " inputs, " 
         << nhidden << " hidden" << endl;
    
    Var input(inputsize);
    Var classnum(1);
    
    Var w1(1+inputsize, nhidden);
    //  fill_random_normal(w1->matValue.subMatRows(1,inputsize), 0., 1./sqrt( real(inputsize) ));
    fill_random_uniform(w1->value, -1./sqrt(real(inputsize)), +1./sqrt(real(inputsize)));
    Var hidden = tanh(affine_transform(input,w1));  
    Var w2(1+nhidden, nclasses);
    //  fill_random_normal(w2->matValue.subMatRows(1,nhidden), 0., 1./sqrt(real(nhidden)));
    fill_random_uniform(w2->value, -1./sqrt(real(nhidden)), +1./sqrt(real(nhidden)));
    
    // Var output = sigmoid(affine_transform(hidden,w2));
    // Var cost = onehot_squared_loss(output, classnum, 0, 1);
    Var output = softmax(affine_transform(hidden,w2));
    Var cost = neg_log_pi(output,classnum);

    VarArray params = w1&w2;
    
    Var classerror = classification_loss(output, classnum);
    Var totalcost = meanOf(trainset, Func(input&classnum, hconcat(cost&classerror)), 1);
    
    // GradientOptimizer opt(params, totalcost, 0.01, 0.00, trainset.length()*nepochs, "simplenet.log", trainset.length());
    GradientOptimizer opt;
    opt.params = params;
    opt.cost = totalcost;
    opt.start_learning_rate = 1e-2;
    opt.nstages = trainset.length();
    opt.build();
    for (int i = 0; i < nepochs; i++) {
        VecStatsCollector statscol;
        opt.optimizeN(statscol);
        pout << (i + 1) * trainset->length() << " " << statscol.getMean() << endl;
    }
    
    cout << "FINISHED." << endl;
    return 0;
  }
  catch(const PLearnError& err)
  {
    cerr << "FATAL ERROR: " << err.message() << endl;
  }
                     
  return 1;
}
