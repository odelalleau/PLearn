#include "FieldConvertCommand.h"
#include "getDataSet.h"
#include "/u/jkeable/LisaPLearn/UserExp/jkeable/bell/convert/TextFilesVMatrix.h"
#include "VVMatrix.h"
#include <fstream.h>
#include "ProgressBar.h"
#include "random.h"

#define NORMALIZE 1
#define MISSING_BIT 2
#define ONEHOT 4
#define SKIP 16

using namespace PLearn;

//! This allows to register the 'JulianDateCommand' command in the command registry
PLearnCommandRegistry FieldConvertCommand::reg_(new FieldConvertCommand);

void FieldConvertCommand::run(const vector<string> & args)
{
  // set default values
  UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS=.3f;
  PVALUE_THRESHOLD=0.025f;
  target=-1;
  report_fn="FieldConvertReport.txt";
    
  for(int i=0;i<(signed)args.size();i++)
  {
    vector<string> val = split(args[i],"=");
    if(val.size()<2)
      PLERROR("bad argument: %s ",args[i].c_str());
    if(val[0]=="source")
      source_fn=val[1];
    else if(val[0]=="destination")
      desti_fn=val[1];
    else if(val[0]=="target")
      target=toint(val[1]);
    else if(val[0]=="force")
      force_fn=val[1];
    else if(val[0]=="report")
      report_fn=val[1];
    else if(val[0]=="min_fraction")
      UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS=toreal(val[1]);
    else if(val[0]=="min_pvalue")
      PVALUE_THRESHOLD=toreal(val[1]);
    else if(val[0]=="onehot_with_correl")
      onehot_with_correl=tobool(val[1]);
    else PLERROR("unknown argument: %s ",val[0].c_str());
  }
  if(source_fn=="")
    PLERROR("you must specify source file");
  if(desti_fn=="")
    PLERROR("you must specify destination .vmat");
  if(target==-1)
    PLERROR("you must specify source target field index");

 // manual map between field index and types
  map<int,int> force;

  real beta_hat,student;

  VMat vm = getDataSet(source_fn);
 
  cout<<"### using field "<<target<<" as target"<<endl;

  ////////////////////////////////////////////////////
  // read user custom operation from file 'force_fname'
  vector<string> forcelines;
  if(force_fn!="")
    forcelines = getNonBlankLines(loadFileAsString(force_fn));
  for(int i=0; i<(signed)forcelines.size();i++)
  {
    vector<string> vec = split(forcelines[i],"=");
    vector<string> leftpart = split(vec[0],"-");
    int rpart = toint(vec[1]);

    if(leftpart.size()>1)
    {
      // we have a range
      int a = toint(leftpart[0]);
      int b = toint(leftpart[1]);
      
      for(int j=a;j<=b;j++)
        force[j]=rpart;
    }
    else 
    {
      if(vm->fieldIndex(vec[0])==-1)
        cout<<"field : "<<vec[0]<<" doesn't exist in matrix"<<endl;
      force[vm->fieldIndex(vec[0])] = rpart;
    }
  }
  ///////////////////////////////////////////////////

  TVec<StatsCollector> sc;
  sc = vm->getStats();

  ofstream out(desti_fn.c_str());
  ofstream report(report_fn.c_str());
  out<<"<SOURCES>\n"+source_fn+"\n</SOURCES>\n<PROCESSING>\n";

  // process each field
  for(int i=0;i<vm->width();i++)
  {
    type=0;
    beta_hat=0;
    string message;
    int action = 0;
    int count= sc[i].getCounts()->size()-1;

    // is this field's type forced ?
    if(force.find(i)!=force.end())
      type = force[i];
    else if(i==target)
      // add target ONLY at the end of the process 
      // (so it's the last column of the dataset)
      type=3;
    // no ? then find out type by ourselves
    else
    {
      if(sc[i].nnonmissing()==0)
        type=3;

      // test for constant values
      else if(count<=1)
        if(sc[i].nmissing()>0 && sc[i].nmissing()<vm->length())
          type=4;
        else 
          type=3;
      else if(sc[i].max()>-1000 && vm->getStringToRealMapping(i).size()>0)
        message="Field uses both string map & numerical values";
      else if(sc[i].min()>19700000 && sc[i].max()<20040000)
        message="Looks like a date. edit the source file to change the 'TextFilesVMatrix' field type (use jdate). otherwise, edit force.txt to confirm so other type.";
      
      // Test whether there are enough unique values to assume continuous data (having a string map implies discrete data)
      else if((count >= MIN( UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS * sc[i].nnonmissing(), 2000)) 
          && vm->getStringToRealMapping(i).size()==0)
        type=1;
      else
        // if there are fractional parts, assume continuous
        for(int j=0;j<vm->length();j++)
        {
          real val = vm->get(j,i);
          if(!is_missing(val) && ((val-(int)val)>0))
          {
            type=1;
            break;
          }
        }
      // if the data doesn't look continuous (small numb. of unique 
      // values and no fractional parts), 'type' still equals 0
      if(type==0 && message=="")

        // nevermind

//          if(vm->getStringToRealMapping(i).size()>0)
//            // if there is a string mapping, let the user find out by himself
//            // (i.e. : maybe there exists an ordering for the strings
//            // but the computer cannot find out by itself)
//            message = "has string mapping. (maybe strings have a relevant sorting, but the mappings are unordered). Count:"+tostring(count);
//        else

        // perform correlation test
        {
          real sigma_hat=0,sigma_beta_hat=0;
          real xmean = sc[i].mean();
          real ymean = sc[target].mean();
          real x_minus_xmean_square=0;
          
          int len_nm = (int)sc[i].nnonmissing();
          int len = vm->length();
          
          Vec x(vm->length());
          Vec y(vm->length());
          vm->getColumn(i, x);
          vm->getColumn(target, y);
          
          // compute beta-hat
          for(int j=0;j<len;j++)
            if(!is_missing(x[j]))
            {
              beta_hat += (x[j] - xmean) * (y[j] - ymean);
              x_minus_xmean_square += (x[j] - xmean)*(x[j]-xmean);
            }
          
          beta_hat /= x_minus_xmean_square;
          
          // compute sigma-hat
          for(int j=0;j<len;j++)
            if(!is_missing(x[j]))
               sigma_hat += square(y[j]-ymean - beta_hat*(x[j]-xmean));
          sigma_hat /= len_nm-2;

          sigma_beta_hat = sigma_hat / x_minus_xmean_square;
          
          real t = beta_hat / sqrt(sigma_beta_hat);
          
          student = 2 * student_t_cdf(-fabs(t), len_nm-2);
          if(student < PVALUE_THRESHOLD)
          {
            // then assume data is discrete but correlated
            type=5;
//            cout<<"##"<<i<<": nmiss:"<<sc[i].nnonmissing()<<" b:"<<beta_hat<<" sigma_beta_hat:"<<sigma_beta_hat<<" T:"<<student<<endl;
          }
        }
      
      // if we're still not sure (that is type==0 && message=="")
      if(type==0 && message=="")
        // is data 'uncorrelated + discrete + sparse'? Yes : Flag 
        if((float)(sc[i].max()-sc[i].min()+1) > (float)(count)*2 )
          type=1;
        else if((float)(sc[i].max()-sc[i].min()+1) != (float)(count) )
          message = "(edit force.txt): Data is made of a semi-sparse (density<50%) distribution of integers (uncorrelated with target). max: "+tostring(sc[i].max())+" min:"+tostring(sc[i].min())+" count:"+tostring(count);
        else
          // data is discrete, not sparse, and not correlated to target,
          // then simply make it as onehot
          type = 2;
    }

    // now find out which actions to perform according to type 

    if(type==0)
      cout<<tostring(i)+" ("+vm->fieldName(i)+") "<<message<<endl;
    else if(type==1 || type==4)
    {
      action |= NORMALIZE;
      if(sc[i].nmissing()>0)
        action |= MISSING_BIT;
    }  
    else if(type==2)
    {
      action = ONEHOT;
      if(sc[i].nmissing()>0)
        action |= MISSING_BIT;
    }  
    else if(type==3)
    {
      action = SKIP;
    }
    else if(type==5)
      // type = 5 : data is discrete & correlated with target
    {
      action |= NORMALIZE;
      action |= ONEHOT;
      if(sc[i].nmissing()>0)
        action |= MISSING_BIT;
    }
    else if(type==4)
      action=MISSING_BIT;
    
    // perform actions
    
    if(action&NORMALIZE)
    {
      // if there are Nans, add a test
      if(sc[i].nmissing()>0)
      {
        // find out 'mode' of the distribution, if any
        double maxi=-1;
        real missingval;
        for(map<real,StatsCollectorCounts>::iterator it = sc[i].getCounts()->begin(); it!=sc[i].getCounts()->end(); ++it)
          if(it->second.n > maxi)
          {
            maxi=it->second.n;
            missingval=it->first;
          }
        if(maxi<10)
          missingval=sc[i].mean();
        
        out<<"@"<<vm->fieldName(i)<<" isnan "<<missingval<<" @"<<vm->fieldName(i)<<" ifelse "<<sc[i].mean()<<" - "<<sc[i].stddev()<<" / :"<<vm->fieldName(i)<<"\n";      
      }
      else out<<"@"<<vm->fieldName(i)<<" "<<sc[i].mean()<<" - "<<sc[i].stddev()<<" / :"<<vm->fieldName(i)<<"\n";      
    }

    if(action&ONEHOT)
    {
      out<<"@"<<vm->fieldName(i)<<" "<<sc[i].getAllValuesMapping()<<" "<<count<<" onehot :"<<vm->fieldName(i)<<":0:"<<(count-1)<<endl;
    }

    if(action&MISSING_BIT)
    {
      out<<"@"<<vm->fieldName(i)<<" isnan 1 0 ifelse :"<<vm->fieldName(i)<<"_mbit\n";      
    }

    report<<tostring(i)+" ("+vm->fieldName(i)+") [c="<<count<<" nm="<<sc[i].nnonmissing()<<"] ";
    if(action==0)report<<"~~user intervention required :"<<message;
    if(action&NORMALIZE)report<<"NORMALIZE ";
    if(action&ONEHOT)report<<"ONEHOT("<<count<<") ";
    if(type==5)report<<"correl: "<<beta_hat<<" 2tail-student:"<<student<<" ";
    if(action&MISSING_BIT)report<<"MISSING_BIT ";
    if(action&SKIP)report<<"SKIP ";
    report<<endl;

   
  }
  out<<"[@resp]\n</PROCESSING>\n"<<endl;






}
