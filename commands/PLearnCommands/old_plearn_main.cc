// -*- C++ -*-

// old_plearn_main.cc
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux, Rejean Ducharme
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
   * $Id: old_plearn_main.cc,v 1.4 2004/07/09 19:40:48 tihocan Exp $
   ******************************************************* */


// #include <sstream>
#include "old_plearn_main.h"

#include "MatIO.h"
#include "fileutils.h"
#include "getDataSet.h"
#include "random.h"
#include "Learner.h"
#include "Optimizer.h"
#include "Kernel.h"
#include "Experiment.h"
#include "FileVMatrix.h"
#include "SquaredErrorCostFunction.h"
#include "PLMPI.h"

namespace PLearn {
using namespace std;

/*
void interpret(PStream& in)
{
  while(in)
    {
      in.skipBlanksAndComments();
      if(!in)
        break;
      if(in.peek()=='<') / it's either a <INCLUDE ...> or a <DEFINE ...> 
    }
}
*/

//! reads a modelalias -> object_representation map from a model.aliases file
map<string, string> getModelAliases(const string& filename)
{
  map<string, string> aliases;
  ifstream in(filename.c_str());
  if(!in)
    PLERROR("In getModelAliases: could not open file %s", filename.c_str());
  while(in)
  {
    string alias;
    getline(in,alias,'=');
    alias = removeblanks(alias);
    if(alias.length()==0) // read all aliases already
      break; 
    if(alias.find_first_of(" \t\n\r")!=string::npos)
      PLERROR("In getModelAliases: problem, expecting a single word alias followed by an equal (=) sign; read %s",alias.c_str());

    in >> ws;//skipBlanks(in);
    string definition;
    smartReadUntilNext(in, ";", definition);
    remove_comments(definition);
    aliases.insert(make_pair(alias,removeblanks(definition)));
  }  
  
  //  cerr << "Aliases:\n";
  // ::write(cerr,aliases);
  // cerr << endl;
  
  //preprocess references to local aliases
  // e.g.:
  // nn= NeuralNet(...);
  // xx= MultiLearner(learner0= $nn; ...);
  for(map<string, string>::iterator it= aliases.begin(); it != aliases.end(); ++it)
    {
      unsigned int pos= 0;
      while(string::npos != (pos= it->second.find('$', pos)))
	{
	  const string delimiters= ";]";
	  unsigned int n= string::npos;
	  for(unsigned int i= 0; i < delimiters.length(); ++i)
	    {
	      unsigned int n0= it->second.find(delimiters[i], pos);
	      if(n0 < n)
		n= n0;
	    }
	  n-= pos;
	  string alias= removeblanks(it->second.substr(pos+1, n-1));
	  if(aliases.find(alias) == aliases.end())
	    PLERROR("In getModelAliases: alias %s is referenced but not defined.", alias.c_str());
	  it->second.replace(pos, n, aliases[alias]);
	}
    }

  return aliases;
}

void train_and_test(const string& modelalias, string trainalias, vector<string> testaliases)
{
  map<string,string> dataset_aliases = getDatasetAliases(".");
  if(dataset_aliases.empty())
    exitmsg("Problem: No dataset.aliases found in the current directory or its parents");
  if(dataset_aliases.find(trainalias)==dataset_aliases.end())
    exitmsg("Problem: No alias '%s' found in dataset.aliases",trainalias.c_str());
  string trainsetdef = dataset_aliases[trainalias];
  cout << ">> Will be training on alias '" << trainalias << "': " << trainsetdef << endl;
  VMat trainset = getDataSet(trainsetdef,trainalias);
  cout << "   size: " << trainset.length() << " x " << trainset.width() << endl;

  int ntestsets = testaliases.size();
  Array<VMat> testsets(ntestsets);
  for(int i=0; i<ntestsets; i++)
  {
    string alias = testaliases[i];
    if(dataset_aliases.find(alias)==dataset_aliases.end())
      exitmsg("Problem: No alias for '%s' found in dataset.aliases",alias.c_str());
    string testsetdef = dataset_aliases[testaliases[i]];
    cout << ">> Will be testing on alias '" << alias << "': " << testsetdef << endl;
    testsets[i] = getDataSet(testsetdef, alias);
    cout << "   size: " << testsets[i].length() << " x " << testsets[i].width() << endl;
  }

  if(!isfile("model.aliases"))
    exitmsg("Problem: No model.aliases file in current directory");
  map<string, string> model_aliases = getModelAliases("model.aliases");
  if(model_aliases.find(modelalias)==model_aliases.end())
    exitmsg("Problem: Could not find alias %s in file model.aliases",modelalias.c_str());

  string use_saved_model = ""; // look for a possibly last saved model in modelalias directory
  if(isdir(modelalias))
  {
    vector<string> dirlist = lsdir(modelalias);
    vector<string>::iterator it = dirlist.begin();
    vector<string>::iterator itend = dirlist.end();
    int maxmodelnum = -1;
    for(; it!=itend; ++it)
    {
      int itl = it->length();
      if(*it == "model.psave")
      {
        use_saved_model = modelalias + "/" + *it;
        break;
      }
      else if(itl>11 && it->substr(0,5)=="model" && it->substr(itl-6,6)==".psave")
      {
        int modelnum = toint(it->substr(5,itl-11));
        if(modelnum>maxmodelnum)
        {
          modelnum = maxmodelnum;
          use_saved_model = modelalias + "/" + *it;
        }
      }
    }
  }

  PP<Learner> learner;
  if(use_saved_model!="")
  {
    cout << ">> Loading saved learner from file " << use_saved_model << endl;
    learner = dynamic_cast<Learner*>(loadObject(use_saved_model));
    if(!learner)
      exitmsg("Problem in making file %s into a Learner",use_saved_model.c_str());
  }
  else
  {
    string modelspec = model_aliases[modelalias];
    cout << ">> Creating learner: " << modelspec << endl;
    PLearn::read(modelspec, learner);
    // learner = dynamic_cast<Learner*>(newObject(modelspec));  
  }

  //  learner->setOption("save_at_every_epoch","true");

  cout << ">> Learner has inputsize=" << learner->inputsize() << " targetsize=" << learner->targetsize() << " outputsize=" << learner->outputsize() << endl;
  //  if(trainset.width()!=learner->inputsize()+learner->targetsize())
  //    exitmsg("Problem: learner's inputsize+targetsize differs from the width of the trainingset!!!");

  learner->setExperimentDirectory(modelalias);
  learner->setTestDuringTrain(testsets);

  cout << "Training and testing..." << endl;
  learner->train(trainset);

  string psavefile = learner->basename()+".psave";
  cout << ">>> Saving final trained model in file: " << psavefile << endl;
  cerr << "{Temporarily commented out by Pascal: don't want to save the Object.\n"
       << " Also with the current 3 argument version, this systematically calls newsave,\n"
       << " so older objects which don't yet have a functional option system cannot be saved through this: to be fixed!!!\n";

#if 0
  // MACHIN PASTE

  string targetfile = learner->basename()+".targets.pmat";
  string outputfile = learner->basename()+"."+datasetalias+".outputs.pmat";
  string costfile = learner->basename()+"."+datasetalias+".costs.pmat";
  VMat vm = testsets[ntestsets-1];
  int l = vm.length();
  VMat outputmat = new FileVMatrix(outputfile,l,learner->outputsize());
  VMat costmat = new FileVMatrix(costfile,l,learner->costsize());
  VMat targetmat = new FileVMatrix(targetfile,l,learner->targetsize());
  Vec input_and_target(vm.width());
  Vec input = input_and_target.subVec(0,learner->inputsize());
  Vec target = input_and_target.subVec(learner->inputsize(), learner->targetsize());
  Vec output(learner->outputsize());
  Vec cost(learner->costsize());
  Vec costs(learner->costsize(), 0.0);
  {//beg. scope of ProgressBar
  ProgressBar pbar(cout,"Computing output and cost",l);
  for(int i=0; i<l; i++)
    {
      vm->getRow(i,input_and_target);
      learner->useAndCost(input, target, output, cost);
      targetmat->putRow(i,target);
      outputmat->putRow(i,output);
      costmat->putRow(i,cost);
      costs+= cost;
      pbar(i);
    }
  // learner->applyAndComputeCosts(vm,outputmat,costmat); 
  }//end. scope of ProgressBar

  cout << learner->costNames() << endl
       << costs/l << endl;

#endif

 save(psavefile, *learner);

}

vector<string> getMultipleModelAliases(const string& model)
{
  vector<string> result;
  if(model[model.length()-1]!='*')
  {
    result.push_back(model);
    return result;
  }
  string modelprefix=model.substr(0,model.length()-1);
  if(!isfile("model.aliases"))
    exitmsg("Problem: No model.aliases file in current directory");
  map<string, string> model_aliases = getModelAliases("model.aliases");
  for(map<string,string>::iterator it=model_aliases.begin();it!=model_aliases.end();it++)
    if(modelprefix=="" || it->first.find(modelprefix)==0)
      result.push_back(it->first);
  return result;
}


void cross_valid(const string& modelalias, string trainalias,int kval)
{
  map<string,string> dataset_aliases = getDatasetAliases(".");
  if(dataset_aliases.empty())
    exitmsg("Problem: No dataset.aliases found in the current directory or its parents");
  if(dataset_aliases.find(trainalias)==dataset_aliases.end())
    exitmsg("Problem: No alias '%s' found in dataset.aliases",trainalias.c_str());
  string trainsetdef = dataset_aliases[trainalias];
  cout << ">> Will be crossvalidating with a kfold value of "<<kval<<" on alias '" << trainalias << "': " << trainsetdef << endl;
  VMat trainset = getDataSet(trainsetdef,trainalias);
  cout << "   size of whole dataset: " << trainset.length() << " x " << trainset.width() << endl;

  if(!isfile("model.aliases"))
    exitmsg("Problem: No model.aliases file in current directory");
  map<string, string> model_aliases = getModelAliases("model.aliases");
  if(model_aliases.find(modelalias)==model_aliases.end())
    exitmsg("Problem: Could not find alias %s in file model.aliases",modelalias.c_str());

/* not implemented for now, Julien
  string use_saved_model = ""; // look for a possibly last saved model in modelalias directory
  if(isdir(modelalias))
  {
    vector<string> dirlist = lsdir(modelalias);
    vector<string>::iterator it = dirlist.begin();
    vector<string>::iterator itend = dirlist.end();
    int maxmodelnum = -1;
    for(; it!=itend; ++it)
    {
      int itl = it->length();
      if(*it == "model.psave")
      {
        use_saved_model = modelalias + "/" + *it;
        break;
      }
      else if(itl>11 && it->substr(0,5)=="model" && it->substr(itl-6,6)==".psave")
      {
        int modelnum = toint(it->substr(5,itl-11));
        if(modelnum>maxmodelnum)
        {
          modelnum = maxmodelnum;
          use_saved_model = modelalias + "/" + *it;
        }
      }
    }
}
*/

  PP<Learner> learner;
/*  if(use_saved_model!="")
  {
    cout << ">> Loading saved learner from file " << use_saved_model << endl;
    learner = dynamic_cast<Learner*>(loadObject(use_saved_model));
    if(!learner)
      exitmsg("Problem in making file %s into a Learner",use_saved_model.c_str());
      }
      else*/
  {
    string modelspec = model_aliases[modelalias];
    cout << ">> Creating learner: " << modelspec << endl;
    PLearn::read(modelspec, learner);
    // learner = dynamic_cast<Learner*>(newObject(modelspec));  
  }

  //  learner->setOption("save_at_every_epoch","true");

  cout << ">> Learner has inputsize=" << learner->inputsize() << " targetsize=" << learner->targetsize() << " outputsize=" << learner->outputsize() << endl;

  if(trainset.width()!=learner->inputsize()+learner->targetsize())
    exitmsg("Problem: learner's inputsize+targetsize differs from the width of the trainingset!!!");

  learner->setExperimentDirectory(modelalias);
  
  Mat mglobal(0,0);
  Mat mhist(0,0);
  TVec<std::string> fnames;
  
  for(int i=0;i<kval;i++)
  {
    VMat train_k,test_k;
    split(trainset, 1.0f/kval, train_k, test_k, kval-i-1);
    train_k->setAlias(trainset->getAlias()+"_kf"+tostring(kval)+"_"+tostring(i));
    test_k->setAlias(trainset->getAlias()+"_kf"+tostring(kval)+"_-"+tostring(i));
    
    learner->forget();
    learner->setTestDuringTrain(test_k);

    cout << "Training and testing ... train.length="<<train_k.length()<<" test.length="<<test_k.length()<<" step:" << i+1 <<" / "<<kval<<endl;
    learner->train(train_k);
    
    string psavefile = learner->basename()+".psave";
    cout << ">>> Saving final trained model in file: " << psavefile << endl;
    save(psavefile, *learner);

    // collect each k's results to make global results file

    Mat mmhist;
    loadAscii(learner->basename()+"."+test_k->getAlias()+".hist.results",mmhist,fnames);
    if(mhist.width()!=mmhist.width() || mhist.length()!=mmhist.length())
    {
      if(mhist.width()!=0)
        PLWARNING("While merging results file in hist.results: differents parts of the kfold don't have the same number of epochs (are you using early stopping?)");
      mhist.resize(mmhist.length(),mmhist.width());
    }
    mhist+=mmhist;
  }
  
  mhist/=kval;
  Vec best(mhist.width(),FLT_MAX);

  // the following generates a global .results with the best epoch (even without earlystopping)
  // It assumes that the value we minimize is on the third column
  for(int i=0;i<mhist.length();i++)
    if(mhist[i][2]<best[2])
      best=mhist(i);
  ofstream out((learner->getExperimentDirectory()+trainset->getAlias()+".results").c_str());
  string fields;
  for(int i=0;i<fnames.size();i++)
    fields+=fnames[i]+=" ";
  out<<"#: "<<fields<<endl;
  out<<best<<endl;
  ////////////////////////////////////////////////////////////////////////////////

  ofstream out2((learner->getExperimentDirectory()+trainset->getAlias()+".hist.results").c_str());
  out2<<"#: "<<fields<<endl;
  out2<<mhist<<endl;
}


void use(const string& modelfile, const string& datasetalias)
{
  map<string,string> aliases = getDatasetAliases(modelfile);
  if(aliases.empty())
    exitmsg("Problem: could not locate a meaningful dataset.aliases file in this or parent directories");
  if(aliases.find(datasetalias)==aliases.end())
    exitmsg("Problem: no %s in dataset.aliases file",datasetalias.c_str());
  string dataset = aliases[datasetalias];
  VMat vm = getDataSet(dataset);
  cout << ">> Dataset has " << vm.length() << " rows and " << vm.width() << " columns" << endl;
  PP<Learner> learner = dynamic_cast<Learner*>(loadObject(modelfile));
  if(!learner)
    exitmsg("Problem in making file %s into a Learner",modelfile.c_str());

  if(learner->costsize() < 1)
    learner->setTestCostFunctions(Array<Ker>(new SquaredErrorCostFunction()));

  cout << ">> Learner has inputsize=" << learner->inputsize() << " targetsize=" << learner->targetsize() << " outputsize=" << learner->outputsize() << endl;
  //  if(vm.width()!=learner->inputsize()+learner->targetsize())
  //    exitmsg("Problem: learner's inputsize+targetsize differs from the width of the dataset!!!");
  string targetfile = datasetalias+".targets.pmat";
  string outputfile = remove_extension(modelfile)+"."+datasetalias+".outputs.pmat";
  string costfile = remove_extension(modelfile)+"."+datasetalias+".costs.pmat";
  int l = vm.length();
  VMat outputmat = new FileVMatrix(outputfile,l,learner->outputsize());
  VMat costmat = new FileVMatrix(costfile,l,learner->costsize());
  VMat targetmat = new FileVMatrix(targetfile,l,learner->targetsize());
  Vec input_and_target(vm.width());
  Vec input = input_and_target.subVec(0,learner->inputsize());
  Vec target = input_and_target.subVec(learner->inputsize(), learner->targetsize());
  Vec output(learner->outputsize());
  Vec cost(learner->costsize());
  Vec costs(learner->costsize(), 0.0);
  {//beg. scope of ProgressBar
  ProgressBar pbar(cout,"Computing output and cost",l);
  for(int i=0; i<l; i++)
    {
      vm->getRow(i,input_and_target);
      learner->useAndCost(input, target, output, cost);
      targetmat->putRow(i,target);
      outputmat->putRow(i,output);
      costmat->putRow(i,cost);
      costs+= cost;
      pbar(i);
    }
  // learner->applyAndComputeCosts(vm,outputmat,costmat); 
  }//end. scope of ProgressBar

  cout << learner->costNames() << endl
       << costs/l << endl;

}

void usage()
{
  cerr << "Usage: " << endl
       << " * plearn train <modelalias> <trainsetalias> [<testsetalias> <testsetalias> ...]\n"
       << "   Will look for the corresponding alias in the 'model.aliases' file in the current directory \n"
       << "   as well as for the specified dataset aliases in a 'dataset.aliases' file in the current or parent direcotries \n"
       << "   It will then build the specified learner with the specified learneroptions, \n"
       << "   train it on the specified train set, and save results (including test results \n"
       << "   on specified testsets) in <modelalias> directory. \n"
       << "   NOTE: you can train multiple models if you append ('*') to a model alias prefix.\n"
       << "   Dont forget the quotes when you use the wildcard to prevent shelle expansion!\n"
       << "         e.g: 'plearn train 'linear*' train valid'.\n"
       << " * plearn cross kfoldval <modelalias> <trainsetalias>\n"
       << "   As with train, but will perform a crossvalidation training with ?? Pascal, complete ca stp:)\n"
       << " * plearn use <model#.psave> <datasetalias>\n"
       << "   After locating the appropriate dataset.aliases looking in parent directories, \n"
       << "   will apply the saved model to the specified dataset, and compute and create \n"
       << "   <model#>.<datasetalias>.outputs.pmat and <model#>.<datasetalias>.costs.pmat \n"
       << " * plearn listmodels <model> \n"
       << "   list the model aliases in the model.aliases file\n"
       << "   model can optionnaly contain a wildcard '*'\n"

    /*
      << " * plearn test <modeldir> <testsetalias> [<testsetalias> ...] \n"
      << "   Will look for a dataset.aliases file in <modeldir> and its parent directories \n"
      << "   to determine which actual datasets the specified testsetalias arguments refer to.\n"
      << "   It will then keep watching the modeldir for any new, untested, model#.psave \n"
      << "   and test it on the specified sets. \n" 
    */
       << " * plearn help datasets \n"
       << "   Will display info about the dataset specification strings you can use to define \n"
       << "   aliases in the dataset.aliases file \n"
       << " * plearn help Learner \n"
       << "   Will print a list of available learners\n"
       << " * plearn help Optimizer \n"
       << "   Will print a list of available optimizers\n"
       << " * plearn help <object-type> \n"
       << "   Will display help (mostly about available options) for that object-type\n"
       << endl;
  exit(0);
}

int old_plearn_main(int argc, char** argv)
{
  PLMPI::init(&argc, &argv);

  seed();

  if(argc<2)
    usage();
 
  string command = argv[1]; // train, test, help, ....

  if(command=="train")
    {
      vector<string> modelaliases = getMultipleModelAliases(argv[2]);
      string trainalias = argv[3];
      vector<string> testaliases = stringvector(argc-4, argv+4);
      // check for possible wildcards at the end of model alias
      for(unsigned int i=0;i<modelaliases.size();i++)
      {
        cout<<"*** Doing job for alias : "<< modelaliases[i]<<endl;
        train_and_test(modelaliases[i], trainalias, testaliases);
      }
    }
  else if(command=="cross")
    {
      if(argc<4)
        usage();
      int kval=toint(argv[2]);
      vector<string> modelaliases = getMultipleModelAliases(argv[3]);
      string trainalias = argv[4];
      for(unsigned int i=0;i<modelaliases.size();i++)
      {
        cout<<"*** Doing job for alias : "<< modelaliases[i]<<endl;
        //PLERROR("J'ai mis en commentaire cross_valid, parce que la version chek-inee ne compile pas... (Pascal)");
        cross_valid(modelaliases[i], trainalias, kval);
      }
    }
  else if(command=="use")
    {
      vector<string> modelaliases = getMultipleModelAliases(argv[2]);
      string datasetalias = argv[3];
     
      for(unsigned int i=0;i<modelaliases.size();i++)
      {
        cout<<"*** Doing job for alias : "<< modelaliases[i]<<endl;
        use(modelaliases[i], datasetalias);
      }
    }
  else if(command=="help")
    {
      string aboutwhat = argv[2];
      if(aboutwhat=="datasets")
        cout << getDataSetHelp();
      else
        displayObjectHelp(cout, aboutwhat);
    }
  else if(command=="listmodels")
  {
    if(!isfile("model.aliases"))
      exitmsg("Problem: No model.aliases file in current directory");
    string mod;
    if(argc==2)
      mod="*";
    else 
      mod=argv[2];
    vector<string> ali = getMultipleModelAliases(mod);
    cout<<"Model aliases found in model.aliases:"<<endl;
    for(unsigned int i=0;i<ali.size();i++)
      cout<<ali[i]<<endl;
  }

  PLMPI::finalize();
  return 0;

}

} // end of namespace PLearn
