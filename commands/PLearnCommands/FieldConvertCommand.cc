// -*- C++ -*-

// Copyright (C) 2004 Université de Montréal
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
 * $Id: FieldConvertCommand.cc,v 1.22 2004/03/12 23:31:34 tihocan Exp $
 ******************************************************* */

#include "FieldConvertCommand.h"
#include "getDataSet.h"
//#include "/u/delallea/LisaPLearn/UserExp/delallea/not-in-cvs/TextFilesVMatrix.h"
//#include "TextFilesVMatrix.h"
//#include "VVMatrix.h"
//#include <fstream.h>
//#include "ProgressBar.h"
//#include "pl_erf.h"       //!< For gauss_01_quantile.
#include "random.h"
#include "stringutils.h"
#include "VMat.h"

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
  UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS = 0.3;
  PVALUE_THRESHOLD = 0.025;
  FRAC_MISSING_TO_SKIP = 1.0;
  FRAC_ENOUGH = 0.005;
  target = -1;
  report_fn="FieldConvertReport.txt";
  precompute = "none";
    
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
    else if(val[0]=="frac_enough")
      FRAC_ENOUGH=toreal(val[1]);
    else if(val[0]=="precompute")
      precompute = val[1];
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
  real correlation = -1;

  VMat vm = getDataSet(source_fn);

  if (target < 0 || target > vm->width()) {
    PLERROR("The target column you specified is not valid");
  }
  
  // Compute the result inputsize as the preprocessing goes on.
  int inputsize = 0;
 
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

  // Minimun number of representants of a class to be considered significant.
  int n_enough = (int) (FRAC_ENOUGH * vm->length());

  ProgressBar* pb = new ProgressBar("Analyzing fields", vm->width());

  // Process each field.
  for(int i=0;i<vm->width();i++)
  {
    type=unknown; // At the beginning we don't know the type.
    beta_hat=0;
    string message;
    int action = 0;
    int count = (int)sc[i].getCounts()->size()-1; // Number of unique values.

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
      else if(sc[i].min() >= 0 && sc[i].max() >= 12000 && sc[i].max() <= 20000) {
        // Could be a numeric SAS date.
        // We first make sure they are all integer values.
        bool non_integer = false;
        for(int j=0;j<vm->length();j++)
        {
          real val = vm->get(j,i);
          if(!is_missing(val) && ((val-(int)val) > 0))
            non_integer = true;
        }
        if (!non_integer) {
          message = "Looks like a numeric SAS date. If this is the case, first edit the source (.vmat) file to change the 'TextFilesVMatrix' field type (use sas_date), then edit force.txt to force the type to continuous. If it's not a date, please use force.txt to force the type.";
        }
      }
      else if(sc[i].min()>19700000 && sc[i].max()<20080000)
        // Could be a date between 1970 and 2008.
        message="Looks like a date. Edit the source file to change the 'TextFilesVMatrix' field type (use jdate). Otherwise, edit force.txt to force the type.";
      
      // Test whether there are enough unique values to assume continuous data (having a string map implies discrete data)
      else if((count >= MIN( UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS * sc[i].nnonmissing(), 2000)) 
          && vm->getStringToRealMapping(i).size()==0)
        type=continuous;
      // Test whether there are only 2 unique values: in this case, we don't
      // need a one hot, and we set it to binary (which will be processed the
      // same as continuous).
      else if (count == 2) {
        Vec counts(2);
        int k = 0;
        for(map<real,StatsCollectorCounts>::iterator it = sc[i].getCounts()->begin(); k <= 1; ++it) {
          counts[k++] = it->second.n;
        }
        if (counts[0] >= n_enough && counts[1] >= n_enough) {
          type = binary;
        } else {
          // Not enough representants for one of the classes.
          type = skip;
          // cout << "Skipped binary field " << i << " (counts_0 = "
          //     << counts[0] << ", counts_1 = " << counts[1] << ")" << endl;
        }
      }
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
        real y_minus_ymean_square=0;

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
            real ydiff = y[j] - ymean;
            beta_hat += xdiff * ydiff;
            x_minus_xmean_square += xdiff * xdiff;
            y_minus_ymean_square += ydiff * ydiff;
          }
          
        // Correlation^2 = sum_xy^2 / (sum_xx * sum_yy).
        correlation = fabs(beta_hat) / sqrt(x_minus_xmean_square * y_minus_ymean_square);

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
        if((real)(sc[i].max()-sc[i].min()+1) > (real)(count)*2 ) {
          type=continuous;
          // cout << "Uncorrelated + discrete + sparse: " << i << " (max = " << sc[i].max() << ", min = " << sc[i].min() << ", count = " << count << ")" << endl;
        }
        else if((real)(sc[i].max()-sc[i].min()+1) != (real)(count) )
          message = "(edit force.txt): Data is made of a semi-sparse (density<50%) distribution of integers (uncorrelated with target). max: "+tostring(sc[i].max())+" min:"+tostring(sc[i].min())+" count:"+tostring(count);
        else {
          // data is discrete, not sparse, and not correlated to target,
          // then simply make it as onehot
          type = discrete_uncorr;
          // cout << "Discrete uncorrelated: " << i << endl;
        }
    }

    // The code commented below is a failed attempt to try and solve
    // the problem of weird distributions.
/*    // TO-DO We should also detect when this is mainly outliers.
    // Check the normal assumption is not violated.
    real epsilon_max = 1e6;
    real epsilon_min = 1e-2;
    real epsilon = epsilon_min;
    real max = sc[i].max();
    real min = sc[i].min();
    real mu = sc[i].mean();
    int nsamp = (int) sc[i].nnonmissing();
    real sigma = sc[i].stddev();
    real confidence = 0.05;
    real alpha = gauss_01_quantile(pow((1 - confidence), 1 / real(nsamp)));
    bool max_is_high = true;
    bool min_is_low = true;
    Vec col(0);
    Vec previous_col(0);
    int countlog = 0;
    bool take_opposite = false;
    real previous_min = 0, previous_max = 0, previous_sigma = 0, previous_mu = 0;
    int countlogmax = 50;
    Vec store_min(countlogmax);
    Vec store_epsilon(countlogmax);
    Vec store_sigma(countlogmax);
    // NB: if count == 2, then this is a binary field, and we shouldn't perform
    // this normal assumption analysis.
    while (count > 2 && (max_is_high || min_is_low)) {
      max_is_high = false;
      min_is_low = false;
      if ( (max - mu) / sigma > alpha ) {
        // Max is too high.
//        cout << "Max is too high for index " << i << endl;
        max_is_high = true;
      }
      if ( (min - mu) / sigma < -alpha ) {
        // Min is too low.
//        cout << "Min is too low for index " << i << endl;
        min_is_low = true;
      }
      if (abs(sigma) < 1e-3) {
        cout << "Low sigma, giving up!" << endl;
        max_is_high = false;
        min_is_low = false;
        countlog = 0;
        take_opposite = false;
        max = sc[i].max();
        min = sc[i].min();
        mu = sc[i].mean();
        sigma = sc[i].stddev();
      }
      if (max_is_high || min_is_low) {
        // Damnit! But how many samples are fucked up?
        int n = vm->length();
        if (col.length() == 0) {
          // Not initialized yet.
          col.resize(vm->length());
          previous_col.resize(vm->length());
          vm->getColumn(i,col);
          previous_col << col;
          countlog++; // We will make at least one log.
          previous_min = min;
          previous_max = max;
          previous_sigma = sigma;
          previous_mu = mu;
        }
        real t;
        int nb_high = 0;
        int nb_low = 0;
        for (int k = 0; k < n; k++) {
          t = col[k];
          if (!is_missing(t)) {
            if ( (t - mu) / sigma > alpha) {
              nb_high++;
            } else if ( (t - mu) / sigma < -alpha ) {
              nb_low++;
            }
          }
        }
        if (nb_high > 0) {
//          cout << "There are " << nb_high << " samples above +alpha = " << alpha << endl;
        }
        if (nb_low > 0) {
//          cout << "There are " << nb_low << " samples below -alpha = " << -alpha << endl;
        }
        // Compute the log and see if it fixes it.
        if (max_is_high && min_is_low) {
          // TODO Find out a solution.
//          cout << "Bleh, both max and min are fucked up, dunno what to do with field " << i << endl;
          max_is_high = false;
          min_is_low = false;
          // (also, be careful as countlog > 0 at this time)
        } else {
          if (min_is_low) {
            // First we take the opposite.
            for (int k = 0; k < n; k++) {
              col[k] = -col[k];
              previous_col[k] = -previous_col[k];
            }
            mu = -mu;
            max = -min;
            min = -max;
            if (take_opposite || countlog > 1) {
              // We should take the opposite only on the first step.
              PLERROR("Taking the opposite in later steps, something must be wrong!");
            }
            take_opposite = true;
          }
          // We compute the log and the new stats.
          epsilon *= 10;
          if (epsilon > epsilon_max) {
            // We tried all available values on epsilon, time to take
            // the log again.
            previous_col << col;
            epsilon = epsilon_min;
            countlog++;
            if (countlog > countlogmax) {
              PLERROR("Too many logs taken");
            }
            previous_min = min;
            previous_max = max;
            previous_sigma = sigma;
            previous_mu = mu;
          } else {
            // Epsilon was too low, we try again from fresh with the higher
            // epsilon.
            col << previous_col;
            min = previous_min;
            max = previous_max;
            sigma = previous_sigma;
            mu = previous_mu;
          }
          store_min[countlog - 1] = min;
          store_sigma[countlog - 1] = sigma;
          store_epsilon[countlog - 1] = epsilon;
          real t;
          real old_min = min;
          real sumsq = 0;
          max = -REAL_MAX;
          min = REAL_MAX;
          mu = 0;
          for (int k = 0; k < n; k++) {
            t = col[k];
            if (!is_missing(t)) {
//              t = log(1 + (t - old_min) + epsilon);
              t = log(1 + (t - old_min) / sigma + epsilon);
              col[k] = t;
              mu += t;
              sumsq += t*t;
              if (t > max) {
                max = t;
              } else if (t < min) {
                min = t;
              }
            }
          }
          real sum = mu;
          mu /= real(nsamp);
          sigma = sqrt(sumsq / real(nsamp - 1) + real(nsamp) / real(nsamp - 1) * mu * mu - 2 * mu / real(nsamp - 1) * sum);
          real sigma_c = sqrt(variance(col, mean(col, true)));
          if (abs(sigma - sigma_c) > 1e-3 && sc[i].nmissing() == 0) {
            cout << "Sigma   = " << sigma << endl;
            cout << "Sigma_c = " << sigma_c << endl;
          } else {
            cout << "OK, sigma and sigma_c seem to correspond" << endl;
          }
          
          if (isnan(sigma)) {
            PLERROR("Sigma = nan");
          }
        }
      }
    }
    if (countlog > 0) {
//      cout << "Countlog for index " << i << ": " << countlog << endl;
    }*/

    // now find out which actions to perform according to type 

    // We treat 'binary' as 'continuous'.
    if (type == binary)
      type = continuous;

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

      out << "@" << vm->fieldName(i) << " ";
      // Replace Nans by either the most frequent value or the mean.
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
        
//        out<<"@"<<vm->fieldName(i)<<" isnan "<<missingval<<" @"<<vm->fieldName(i)<<" ifelse "<<sc[i].mean()<<" - "<<sc[i].stddev()<<" / :"<<vm->fieldName(i)<<"\n";      
        out << "isnan " << missingval << " @" << vm->fieldName(i) << " ifelse ";
      }

      /*
      // Apply log transformations if necessary.
      if (take_opposite) {
        // First take the opposite.
        out << "0 exch - ";
      }
      for (int c = 0; c < countlog; c++) {
        // Compute log(1 + (x - min) / sigma + epsilon).
        out << store_min[c] << " - " << store_sigma[c] << " / ";
        out << "1 + " << store_epsilon[c] << " + log ";
      } */

      // And apply normalization.
      real mu = sc[i].mean();
      real sigma = sc[i].stddev();
      out << mu << " - " << sigma << " / :" << vm->fieldName(i)<<"\n";
      inputsize++;
    }

    int n_discarded = 0;
    if(action&ONEHOT) {
      // First see if any value must be discarded, because not present often
      // enough in the dataset.
      int k = 0;
      TVec<bool> to_be_included(count);
      for (int j = 0; j < count; j++) {
        to_be_included[j] = true;
      }
      for(map<real,StatsCollectorCounts>::iterator it = sc[i].getCounts()->begin(); k<((int)sc[i].getCounts()->size()) - 1; ++it) {
        if (it->second.n < n_enough) {
          to_be_included[k] = false;
          n_discarded++;
          // cout << "Field " << i << ": value " << it->first
          //     << " discarded (n = " << it->second.n << ")." << endl;
        }
        k++;
      }
      if (n_discarded <= count - 1) {
        // We only consider this field if there is at least 1 class left.
        out << "@"<<vm->fieldName(i) <<" " << sc[i].getAllValuesMapping(&to_be_included, 0, true) << " "
          << count - n_discarded << " onehot :"
          << vm->fieldName(i)<<":0:"<<(count - 1 - n_discarded) << endl;
        inputsize += count - n_discarded;
      }
    }

    if(action&MISSING_BIT)
    {
      out<<"@"<<vm->fieldName(i)<<" isnan 1 0 ifelse :"<<vm->fieldName(i)<<"_mbit\n";      
      inputsize++;
    }

    report<<tostring(i)+" ("+vm->fieldName(i)+") [c="<<count<<" nm="<<sc[i].nnonmissing()<<"] ";
    if(action==0)report<<"~~user intervention required :"<<message;
    if(action&NORMALIZE) {
      report << "NORMALIZE ";
/*      if (countlog > 0) {
        report << "(after " << countlog << " log) ";
      }*/
    }
    if(action&ONEHOT)report<<"ONEHOT("<<count<<") - discarded: " << n_discarded << " ";
    if(type==discrete_corr)report<<"correl: "<<correlation<<" 2tail-student:"<<student<<" ";
    if(action&MISSING_BIT)report<<"MISSING_BIT ";
    if(action&SKIP)report<<"SKIP ";
    report<<endl;

    pb->update(i);
   
  }

  delete pb;

  // Add the target.
  out << "%" << target << " :target\n</PROCESSING>"<<endl;

  // Add the sizes.
  out << endl << "<SIZES>"  << endl
              << inputsize  << endl // inputsize
              << "1"        << endl // targetsize
              << "0"        << endl // weightsize
              << "</SIZES>" << endl;

  // Possibly add the <PRECOMPUTE> tag.
  if (precompute != "none") {
    out << endl << "<PRECOMPUTE>" << endl << precompute << endl << "</PRECOMPUTE>" << endl;
  }

}

///////////////////////
// stringToFieldType //
///////////////////////
PLearn::FieldConvertCommand::FieldType FieldConvertCommand::stringToFieldType(string s) {
  if (s.find("continuous") != string::npos)
    return continuous;
  else if (s.find("discrete_uncorr")!= string::npos )
    return discrete_uncorr;
  else if (s.find("discrete_corr") != string::npos)
    return discrete_corr;
  else if (s.find("constant") != string::npos)
    return constant;
  else if (s.find("binary") != string::npos)
    return binary;
  else if (s.find("skip") != string::npos)
    return skip;
  else {
    PLERROR("In FieldConvertCommand::stringToFieldType Unknown field type: %s",s.c_str());
    return skip;
  }
}

