#include "FieldConvertCommand.h"
#include "getDataSet.h"
#include "/u/delallea/LisaPLearn/UserExp/delallea/not-in-cvs/TextFilesVMatrix.h"
#include "VVMatrix.h"
#include <fstream.h>
#include "ProgressBar.h"
#include "random.h"

#define NORMALIZE 1
#define MISSING_BIT 2
#define ONEHOT 4
#define SKIP 16

using namespace PLearn;

//! This allows to register the 'FieldConvertCommand' command in the command registry
PLearnCommandRegistry FieldConvertCommand::reg_(new FieldConvertCommand);

void FieldConvertCommand::run(const vector<string> & args)
{
  // set default values
  UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS=.3f;
  PVALUE_THRESHOLD=0.025f;
  FRAC_MISSING_TO_SKIP=0.9f;
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
    else if(val[0]=="max_pvalue")
      PVALUE_THRESHOLD=toreal(val[1]);
    else if(val[0]=="frac_missing_to_skip")
      FRAC_MISSING_TO_SKIP=toreal(val[1]);
    else if(val[0]=="onehot_with_correl")
      onehot_with_correl=tobool(val[1]);
    // TODO see what is onehot_with_correl
    // Apparently it isn't used anyway.
    else PLERROR("unknown argument: %s ",val[0].c_str());
  }
  if(source_fn=="")
    PLERROR("you must specify source file");
  if(desti_fn=="")
    PLERROR("you must specify destination .vmat");
  if(target==-1)
    PLERROR("you must specify source target field index");

 // manual map between field index and types
  map<int,FieldType> force;

  real beta_hat,student=-1;

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
    FieldType rpart = stringToFieldType(vec[1]);

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

  // Process each field.
  for(int i=0;i<vm->width();i++)
  {
    type=unknown; // At the beginning we don't know the type.
    beta_hat=0;
    string message;
    int action = 0;
    int count = sc[i].getCounts()->size()-1; // Number of unique values.

    // is this field's type forced ?
    if(force.find(i) != force.end())
      type = force[i];
    else if(i==target)
      // add target ONLY at the end of the process 
      // (so it's the last column of the dataset)
      type=skip;
    // no ? then find out type by ourselves
    else
    {

      // Test for fields to be skipped, when not enough data is available.
      if(sc[i].nnonmissing() <= (1-FRAC_MISSING_TO_SKIP) * vm->length()) {
        type=skip;
      }

      // test for constant values
      else if(count<=1) {
        if(sc[i].nmissing()>0 && sc[i].nmissing()<vm->length()) {
          // This case actually never occurs in the Bell database.
          // That's why we leave it to the user.
          message = "Constant field, but there are " + tostring(sc[i].nmissing()) +
            " missing values. Force the type, or modify this program !";
        }
        else 
          // Either there is no missing value, or they are all missing.
          type=constant;
      }
      else if(sc[i].max()>-1000 && vm->getStringToRealMapping(i).size()>0)
        message="Field uses both string map & numerical values";
      else if(sc[i].min()>19700000 && sc[i].max()<20080000)
        // Could be a date between 1970 and 2008.
        message="Looks like a date. Edit the source file to change the 'TextFilesVMatrix' field type (use jdate). Otherwise, edit force.txt to force the type.";
      
      // Test whether there are enough unique values to assume continuous data (having a string map implies discrete data)
      else if((count >= MIN( UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS * sc[i].nnonmissing(), 2000)) 
          && vm->getStringToRealMapping(i).size()==0)
        type=continuous;
      else {
        // if there are fractional parts, assume continuous
        for(int j=0;j<vm->length();j++)
        {
          real val = vm->get(j,i);
          if(!is_missing(val) && ((val-(int)val) != 0))
          {
            type=continuous;
            break;
          }
        }
      }

      // if the data doesn't look continuous (small numb. of unique 
      // values and no fractional parts), 'type' still equals unknown.
      if(type==unknown && message=="")
      {
        // perform correlation test
        real sigma_hat=0,sigma_beta_hat=0;
        real xmean = sc[i].mean();
        real ymean = sc[target].mean();
        real x_minus_xmean_square=0;

        int len_nm = (int)sc[i].nnonmissing();
        int len = vm->length();

        Vec x(len);
        Vec y(len);
        vm->getColumn(i, x);
        vm->getColumn(target, y);
          
        // compute beta-hat
        for(int j=0;j<len;j++)
          if(!is_missing(x[j]))
          {
            real xdiff = x[j] - xmean;
            beta_hat += xdiff * (y[j] - ymean);
            x_minus_xmean_square += xdiff * xdiff;
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
          type = discrete_corr;
          // cout<<"##"<<i<<": nonmiss:"<<sc[i].nnonmissing()<<" b:"<<beta_hat<<" sigma_beta_hat:"<<sigma_beta_hat<<" T:"<<student<<endl;
        }
      }

      // If we're still not sure (that is to say, type==unknown && message=="").
      if(type==unknown && message=="")
        // is data 'uncorrelated + discrete + sparse'? Yes : Flag 
        if((float)(sc[i].max()-sc[i].min()+1) > (float)(count)*2 ) {
          type=continuous;
          // cout << "Uncorrelated + discrete + sparse: " << i << " (max = " << sc[i].max() << ", min = " << sc[i].min() << ", count = " << count << ")" << endl;
        }
        else if((float)(sc[i].max()-sc[i].min()+1) != (float)(count) )
          message = "(edit force.txt): Data is made of a semi-sparse (density<50%) distribution of integers (uncorrelated with target). max: "+tostring(sc[i].max())+" min:"+tostring(sc[i].min())+" count:"+tostring(count);
        else {
          // data is discrete, not sparse, and not correlated to target,
          // then simply make it as onehot
          type = discrete_uncorr;
          // cout << "Discrete uncorrelated: " << i << endl;
        }
    }

    // now find out which actions to perform according to type 

    if(type==unknown)
      cout<<tostring(i)+" ("+vm->fieldName(i)+") "<<message<<endl;
    else if(type==continuous)
    {
      action |= NORMALIZE;
      if(sc[i].nmissing()>0)
        action |= MISSING_BIT;
    }  
    else if(type==discrete_uncorr)
    {
      action = ONEHOT;
      if(sc[i].nmissing()>0)
        action |= MISSING_BIT;
    }  
    else if(type==skip || type==constant)
    {
      action = SKIP;
    }
    else if(type==discrete_corr)
    {
      action |= NORMALIZE;
      action |= ONEHOT;
      if(sc[i].nmissing()>0)
        action |= MISSING_BIT;
    }
    
    // perform actions
    
    if(action&NORMALIZE)
    {
      // if there are Nans, add a test
      if(sc[i].nmissing()>0)
      {
        // find out 'mode' of the distribution, if any
        double maxi=-1;
        real missingval = -1;
        for(map<real,StatsCollectorCounts>::iterator it = sc[i].getCounts()->begin(); it!=sc[i].getCounts()->end(); ++it)
          if(it->second.n > maxi)
          {
            maxi=it->second.n;
            missingval=it->first;
          }
        if(maxi<10)
          // The most frequent value appears less than 10 times: a missing value is replaced by the mean.
          missingval=sc[i].mean();
        else {
          // We replace a missing value by the most frequent value.
          // cout << i << ": maxi >= 10, and missingval = " << missingval << endl;
        }
        
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
    if(type==discrete_corr)report<<"correl: "<<beta_hat<<" 2tail-student:"<<student<<" ";
    if(action&MISSING_BIT)report<<"MISSING_BIT ";
    if(action&SKIP)report<<"SKIP ";
    report<<endl;

   
  }
  out<<"[@resp]\n</PROCESSING>\n"<<endl;

}

///////////////////////
// stringToFieldType //
///////////////////////
PLearn::FieldConvertCommand::FieldType FieldConvertCommand::stringToFieldType(string s) {
  if (s == "continuous")
    return continuous;
  else if (s == "discrete_uncorr")
    return discrete_uncorr;
  else if (s == "discrete_corr")
    return discrete_corr;
  else if (s == "constant")
    return constant;
  else if (s == "skip")
    return skip;
  else {
    PLERROR("In FieldConvertCommand::stringToFieldType Unknown field type: %s",s.c_str());
    return skip;
  }
}

