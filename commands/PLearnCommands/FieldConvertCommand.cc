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
 * $Id$
 ******************************************************* */

#include "FieldConvertCommand.h"
#include <plearn/base/stringutils.h>
#include <plearn/db/getDataSet.h>
#include <plearn/io/openFile.h>
#include <plearn/io/fileutils.h>      //!< For newFilename()
#include <plearn/math/pl_erf.h>       //!< For gauss_01_quantile()
#include <plearn/math/random.h>
#include <plearn/vmat/SelectRowsVMatrix.h>
#include <plearn/vmat/VMat.h>

#define NORMALIZE 1
#define MISSING_BIT 2
#define ONEHOT 4
#define SKIP 16
#define UNIFORMIZE 32

using namespace PLearn;

//! This allows to register the 'FieldConvertCommand' command in the command registry
PLearnCommandRegistry FieldConvertCommand::reg_(new FieldConvertCommand);


/////////////////////////
// FieldConvertCommand //
/////////////////////////
FieldConvertCommand::FieldConvertCommand()
    :  PLearnCommand("FieldConvert",

                     "Reads a dataset and generates a .vmat file based on the data, but optimized for training.\n",

                     "The nature of each field of the original dataset is automatically detected, and determines the approriate treatment.\n"
                     "The possible field types with the corresponding treatment can be one of :\n"
                     "continuous      - quantitative data (data is real): the field is replaced by the normalized data (minus means, divided by stddev)\n"
                     "binary          - binary discrete data (is processed as a continuous field)\n"
                     "discrete_uncorr - discrete integers (qualitative data, e.g : postal codes, categories) not corr. with target: the field is replaced by a group of fields in a one-hot fashion.\n"
                     "discrete_corr   - discrete integers, correlated with target : both the normalized and the onehot versions of the field are used in the new dataset\n"
                     "constant        - constant data : the field is skipped (it is not present in the new dataset)\n"
                     "skip            - irrelevant data : the field is skipped (it is not present in the new dataset)\n"
                     "\n"
                     "When there are ambiguities, messages are displayed for the problematic field(s) and they are skipped. The user must use a 'force' file,\n"
                     "to explicitely force the types of the ambiguous field(s). The file is made of lines of the following possible formats:\n"
                     "FIELDNAME=type\n"
                     "fieldNumberA-fieldNumberB=type   [e.g : 200-204=constant, to force a range]\n"
                     "FIELDNAME+=\"processing\" (n_inputs) [to add a home-made processing after a field; the number of inputs thus added must be given]\n"
                     "\n"
                     "Note that for all types but skip, if the field contains missing values, an additionnal 'missing-bit' field is added and is '1' only for missing values.\n"
                     "The difference between types constant and skip is only cosmetic: constant means the field is constant, while skip means either there are too many missing values or it has been forced to skip.\n"
                     "A report file is generated and contains the information about the processing for each field.\n"
                     "Target index of source needs to be specified (ie. to perform corelation test). It can be any field of the "
                     "source dataset, but will be the last field of the new dataset.*** We assume target is never missing *** \n\n"
                     "usage : FieldConvert\n"
                     "        *source                = [source dataset]\n"
                     "        *destination           = [new dataset with vmat extension]\n"
                     "        *target                = [field index of target]\n"
                     "         force                 = [force file]\n"
                     "         report                = [report file] (default = 'FieldConvertReport.txt')\n"
                     "         min_fraction          = [if number of unique values is > than 'fraction' * NonMISSING -> the field is continuous]\n"
                     "                                 (default = 0.3)\n"
                     "         max_pvalue            = [maximum pvalue to assume correlation with target] (default = 0.025)\n"
                     "         frac_missing_to_skip  = [if MISSING >= 'frac_missing_to_skip * number of samples then this field is skipped]\n"
                     "                                 (default = 1.0)\n"
                     "         frac_enough           = [if a field is discrete, only values represented by at least frac_enough * nSamples\n"
                     "                                 elements will be kept] (default = 0.005)\n"
                     "         precompute            = [none | pmat | ... : possibly add a <PRECOMPUTE> tag in the destination] (default = none)\n"
                     "         discrete_tolerance    = [if a discrete field has float values, its one hot mapping will be enlarged according to\n"
                     "                                 this factor] (default = 0.001)\n"
                     "         uniformize            = [0 | 1 | 2: whether fields should be uniformized, 2 meaning all fields and 1 meaning only\n"
                     "                                 fields obviously not following a normal distribution] (default = 0)\n"
                     "         frac_missing_sample   = [if a sample has more than 'frac_missing_sample' * n_fields missing fields, then this sample\n"
                     "                                 will be removed from the dataset (before analyzing the dataset's fields] (default = 1)\n"
                     "\n"
                     "where fields with asterix * are not optional\n"
        ) 
{}

/////////
// run //
/////////
void FieldConvertCommand::run(const vector<string> & args)
{
    // set default values
    UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS = 0.3;
    PVALUE_THRESHOLD = 0.025;
    FRAC_MISSING_TO_SKIP = 1.0;
    FRAC_ENOUGH = 0.005;
    DISCRETE_TOLERANCE = 1e-3;
    real FRAC_MISSING_SAMPLE = 1;
    target = -1;
    report_fn="FieldConvertReport.txt";
    precompute = "none";
    int uniformize = 0;
    
    for(unsigned int i=0;i<args.size();i++)
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
        else if(val[0]=="discrete_tolerance")
            DISCRETE_TOLERANCE = toreal(val[1]);
        else if(val[0]=="uniformize")
            uniformize = toint(val[1]);
        else if(val[0]=="frac_enough")
            FRAC_ENOUGH=toreal(val[1]);
        else if(val[0]=="precompute")
            precompute = val[1];
        else if (val[0] == "frac_missing_sample")
            FRAC_MISSING_SAMPLE = toreal(val[1]);
        else PLERROR("unknown argument: %s ",val[0].c_str());
    }
    if(source_fn=="")
        PLERROR("you must specify source file");
    if(desti_fn=="")
        PLERROR("you must specify destination .vmat");
    if(target==-1)
        PLERROR("you must specify source target field index");

    // Manual map between field index and types.
    map<int, FieldType> force;
    map<int, string> additional_proc;
    map<int, int> additional_proc_size;

    real beta_hat,student=-1;
    real correlation = -1;

    // Get the dataset.
    VMat vm_orig = getDataSet(source_fn);
    VMat vm;
    int n_removed = 0;
    TVec<int> to_keep;
    if (FRAC_MISSING_SAMPLE < 1) {
        // We may have to remove some samples first.
        ProgressBar pb("Removing samples with too many missing values", vm_orig->length());
        int w = vm_orig->width();
        Vec row(w);
        int count;
        int max_count = int(w * FRAC_MISSING_SAMPLE);
        for (int i = 0; i < vm_orig->length(); i++) {
            vm_orig->getRow(i, row);
            count = 0;
            for (int j = 0; j < w; j++) {
                if (is_missing(row[j]))
                    count++;
            }
            if (count <= max_count)
                to_keep.append(i);
            else
                n_removed++;
            pb.update(i+1);
        }
        pb.close();
        cout << "Removed " << n_removed << " samples that were missing more than " << max_count << " fields." << endl;
        if (n_removed > 0) {
            vm = new SelectRowsVMatrix(vm_orig, to_keep);
            vm->setMetaDataDir(newFilename("/tmp", "select_rows_temp_dir", true));
        }
        else
            vm = vm_orig;
    } else {
        vm = vm_orig;
    }

    // A vector where we store the indices of the fields to be uniformized.
    TVec<int> need_to_be_uniformized;

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
        size_t pos_of_equal = forcelines[i].find('=');
        if (pos_of_equal == string::npos)
            PLERROR("In FieldConvertCommand - A line in the force file does not contain the '=' character");
        vector<string> vec(2);
        vec[0] = forcelines[i].substr(0, pos_of_equal);
        vec[1] = forcelines[i].substr(pos_of_equal + 1);
/*    cout << "vec[0] = " << vec[0] << endl;
      cout << "vec[1] = " << vec[1] << endl; */
        vector<string> leftpart = split(vec[0],"-");
        if (leftpart.size() == 1 && leftpart[0].substr(leftpart[0].size() - 1) == "+") {
            // Syntax: field+="processing" (number of inputs added)
            int field_index = vm->getFieldIndex(leftpart[0].substr(0, leftpart[0].size() - 1));
            if (field_index == -1)
                PLERROR("In FieldConvertCommand - A field was not found in the source VMatrix");
            if (additional_proc[field_index] != "")
                PLERROR("In FieldConvertCommand - There can be only one additional processing specified for each field");
            size_t last_open_par = vec[1].rfind('(');
            if (last_open_par == string::npos)
                PLERROR("In FieldConvertCommand - You must specify the number of inputs added in a processing");
            string added_inputs = vec[1].substr(last_open_par + 1, vec[1].rfind(')') - last_open_par - 1);
            // cout << "added_inputs = " << added_inputs << endl;
            additional_proc_size[field_index] = toint(added_inputs);
            size_t first_comma = vec[1].find('"');
            size_t last_comma = vec[1].rfind('"', last_open_par);
            additional_proc[field_index] = vec[1].substr(first_comma + 1, last_comma - first_comma - 1);
            // cout << "Processing added: " << additional_proc[field_index] << endl;
        } else {
            FieldType rpart = stringToFieldType(vec[1]);

            if(leftpart.size()>1)
            {
                // we have a range
                int a = vm->getFieldIndex(leftpart[0]);
                int b = vm->getFieldIndex(leftpart[1]);
                for(int j=a;j<=b;j++) {
                    if (force.find(j) != force.end())
                        PLERROR("In FieldConvertCommand::run - Duplicate force type for variable %d", j);
                    force[j]=rpart;
                }
            }
            else 
            {
                int index = vm->getFieldIndex(vec[0]);
                if (index == -1)
                    cout<<"field : "<<vec[0]<<" doesn't exist in matrix"<<endl;
                if (force.find(index) != force.end())
                    PLERROR("In FieldConvertCommand::run - Duplicate force type for variable %d", index);
                force[index] = rpart;
            }
        }
    }
    ///////////////////////////////////////////////////

    TVec<StatsCollector> sc;
    sc = vm->getStats();

    PStream out;
    PStream out_uni;
    PPath filename_non_uni = desti_fn + ".non_uniformized.vmat";
    if (uniformize > 0) {
        // We write two files: the one with the preprocessing and another one with
        // the uniformization.
        out = openFile(filename_non_uni, PStream::raw_ascii, "w");
        out_uni = openFile(desti_fn, PStream::raw_ascii, "w");
    } else {
        out = openFile(desti_fn, PStream::raw_ascii, "w");
    }
    PStream report = openFile(report_fn, PStream::raw_ascii, "w");
    out<<"<SOURCES>\n";
    if (n_removed == 0) {
        out << source_fn;
    } else {
        out << "@" << endl
            << "SelectRowsVMatrix(" << endl
            << "  source = AutoVMatrix(specification = \"" << source_fn << "\")" << endl
            << "  indices = [ " << to_keep << " ]" << endl
            << ")";
    }
    out << "\n</SOURCES>\n<PROCESSING>\n";

    // Minimun number of representants of a class to be considered significant.
    int n_enough = (int) (FRAC_ENOUGH * vm->length());

    PP<ProgressBar> pb = new ProgressBar("Analyzing fields", vm->width());

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

        // Test for fields to be skipped, when not enough data is available.
        if(sc[i].nnonmissing() <= (1-FRAC_MISSING_TO_SKIP) * vm->length()) {
            if (type != unknown && type != skip && type != constant) {
                // We forced the type to something that should not be skipped.
                cout << "Warning: you forced the type of field number " << i << ", "
                     << "but there are too many missing values so it'll be skipped. "
                     << "If you want to keep it, you'll have to add it by hand to the resulting .vmat"
                     << endl;
            }
            type=skip;
        }

        // Test whether there are only 2 unique values: in this case, we don't
        // need a one hot, and we set it to binary (which will be processed the
        // same as continuous).
        if (count == 2 && type != skip) {
            Vec counts(2);
            int k = 0;
            for(map<real,StatsCollectorCounts>::iterator it = sc[i].getCounts()->begin(); k <= 1; ++it) {
                counts[k++] = it->second.n;
            }
            if (counts[0] >= n_enough && counts[1] >= n_enough) {
                if (type != unknown && type != binary) {
                    cout << "Warning: type for field number " << i << " set to binary, "
                         << "but you had forced it to something else." << endl;
                }
                type = binary;
            } else {
                // Not enough representants for one of the classes.
                if (type != unknown && type != skip) {
                    cout << "Warning: field number " << i << " is binary but doesn't have "
                         << "enough representants of each class, thus it'll be skipped, "
                         << "even if you had forced it to some other type (edit the resulting "
                         << ".vmat if you really want to add it)." << endl;
                }
                type = skip;
                // cout << "Skipped binary field " << i << " (counts_0 = "
                //     << counts[0] << ", counts_1 = " << counts[1] << ")" << endl;
            }
        }

        // Test for constant values.
        if(count<=1 && type != skip && type != constant) {
            if(sc[i].nmissing()>0 && sc[i].nmissing()<vm->length()) {
                // This case actually never occurs in the Bell database.
                // That's why we leave it to the user.
                message = "Constant field, but there are " + tostring(sc[i].nmissing()) +
                    " missing values. Force the type, or modify this program !";
            }
            else {
                // Either there is no missing value, or they are all missing.
                if (type != unknown) {
                    cout << "Warning: field number " << i << " has been forced, but "
                         << "appears to be constant. Edit the resulting .vmat if you "
                         << "really want to add it." << endl;
                }
                type=constant;
            }
        }

        // Test if there exist fractional parts.
        // This test has two goals:
        //  - if we don't know the type, a fractional part indicates continuous data
        //  - if the type is discrete, we need to be careful in the one hot ranges
        //    because taking exact float values is not a good idea
        bool may_be_fraction = false;
        if (type == continuous || type == binary) {
            may_be_fraction = true;
        } else if (type != skip && type != constant) {
            int k = 0;
            for (map<real,StatsCollectorCounts>::iterator it = sc[i].getCounts()->begin(); k < count; ++it) {
                real val = it->first;
                k++;
                if(!fast_exact_is_equal(val-(int)val, 0))
                {
                    may_be_fraction = true;
                    break;
                }
            }
        }

        // Did we find the type already?
        if (type == unknown && message == "")
        {

            if(sc[i].max()>-1000 && vm->getStringToRealMapping(i).size()>0)
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
            else {
                // if there are fractional parts, assume continuous
                if (may_be_fraction) {
                    type=continuous;
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

                int len_nm = 0;
                int len = vm->length();

                Vec x(len);
                Vec y(len);
                vm->getColumn(i, x);
                vm->getColumn(target, y);
          
                // compute beta-hat
                for(int j=0;j<len;j++)
                    if(!is_missing(x[j]) && !is_missing(y[j]))
                    {
                        real xdiff = x[j] - xmean;
                        real ydiff = y[j] - ymean;
                        beta_hat += xdiff * ydiff;
                        x_minus_xmean_square += xdiff * xdiff;
                        y_minus_ymean_square += ydiff * ydiff;
                        len_nm++;
                    }
          
                // Correlation^2 = sum_xy^2 / (sum_xx * sum_yy).
                correlation = fabs(beta_hat) / sqrt(x_minus_xmean_square * y_minus_ymean_square);

                beta_hat /= x_minus_xmean_square;

                // compute sigma-hat
                for(int j=0;j<len;j++)
                    if(!is_missing(x[j]) && !is_missing(y[j]))
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
                else if(!fast_exact_is_equal(real(sc[i].max()-sc[i].min()+1), real(count)) )
                    message = "(edit force.txt): Data is made of a semi-sparse (density<50%) distribution of integers (uncorrelated with target). max: "+tostring(sc[i].max())+" min:"+tostring(sc[i].min())+" count:"+tostring(count);
                else {
                    // data is discrete, not sparse, and not correlated to target,
                    // then simply make it as onehot
                    type = discrete_uncorr;
                    // cout << "Discrete uncorrelated: " << i << endl;
                }
        }

        // Now find out which actions to perform according to type.

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
    
        // Perform actions.
    
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
        
                out << "isnan " << missingval << " @" << vm->fieldName(i) << " ifelse ";
            }

            // Uniformize all fields when 'uniformize' is set to 2.
            bool to_uniformize = (uniformize == 2);
            // If this field violates the normal assumption, and the user set the
            // 'uniformize' option to 1, then we should keep this field intact, and
            // remember it will need to be uniformized in the final vmat.
            bool apply_normalization = true;
            if (uniformize == 1) {
                real max = sc[i].max();
                real min = sc[i].min();
                real mu = sc[i].mean();
                real sigma = sc[i].stddev();
                int nsamp = (int) sc[i].nnonmissing();
                real confidence = 0.05;
                real alpha = gauss_01_quantile(pow((1 - confidence), 1 / real(nsamp)));
                if ( (max - mu) / sigma > alpha  || (min - mu) / sigma < - alpha) {
                    // Normal assumption violated.
                    to_uniformize = true;
                }
            }
            if (to_uniformize) {
                action ^= NORMALIZE;    // Remove the 'normalize' action.
                action |= UNIFORMIZE;   // And add the 'uniformize' one.
                apply_normalization = false;
                out << ":" << vm->fieldName(i) << endl;
                need_to_be_uniformized.append(inputsize);
            }

            // And apply normalization if we still need to do it.
            if (apply_normalization) {
                real mu = sc[i].mean();
                real sigma = sc[i].stddev();
                out << mu << " - " << sigma << " / :" << vm->fieldName(i)<<"\n";
            }

            // Increase the counter of inputs.
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
                // TODO TMP
//        RealMapping rm = sc[i].getBinMapping(1,1);
//        out << "@" << vm->fieldName(i) << " " << rm << " onehot :"
//            << vm->fieldName(i) << ":0:" << rm.size() << endl;
                real tol = 0;
                if (may_be_fraction) {
                    // We need to take a margin because of floating point precision.
                    tol = DISCRETE_TOLERANCE;
                }
                RealMapping rm = sc[i].getAllValuesMapping(&to_be_included, 0, true, tol);
                out << "@"<<vm->fieldName(i) <<" " << rm << " "
                    << rm.size() << " onehot :"
                    << vm->fieldName(i)<<"_:0:"<< (rm.size() - 1) << endl;
/*        out << "@"<<vm->fieldName(i) <<" " << sc[i].getAllValuesMapping(&to_be_included, 0, true) << " "
          << count - n_discarded << " onehot :"
          << vm->fieldName(i)<<"_:0:"<<(count - 1 - n_discarded) << endl; */
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
        if (action & UNIFORMIZE) report << "UNIFORMIZE ";
        if (action&ONEHOT) report<<"ONEHOT("<<count<<") - discarded: " << n_discarded << " ";
        if (type==discrete_corr) report<<"correl: "<<correlation<<" 2tail-student:"<<student<<" ";
        if (action&MISSING_BIT) report<<"MISSING_BIT ";
        if (action&SKIP) report<<"SKIP ";
        if (additional_proc[i] != "") {
            // There is an additional processing to add after this field.
            out << additional_proc[i] << endl;
            inputsize += additional_proc_size[i];
            report << "ADD_PROC ";
        }
        report<<endl;

        pb->update(i);
   
    }

    // Add the target.
    out << "%" << target << " :target\n</PROCESSING>"<<endl;

    // Add the sizes.
    out << endl  << "<SIZES>"  << endl
        << inputsize  << endl // inputsize
        << "1"        << endl // targetsize
        << "0"        << endl // weightsize
        << "</SIZES>" << endl;

    // Now build the uniformized VMatrix if 'uniformize' has been set.
    if (uniformize > 0) {
        // Prepare the 'shift' and 'scale' vectors to map uniformized fields to
        // [-1,1] instead of default [0,1].
        Vec shift(inputsize + 1);  // +1 because of the target.
        Vec scale(inputsize + 1);
        shift.fill(0);
        scale.fill(1);
        for (int i = 0; i < need_to_be_uniformized.length(); i++) {
            shift[need_to_be_uniformized[i]] = -0.5;
            scale[need_to_be_uniformized[i]] = 2;
        }
        // Write the .vmat file.
        out_uni << "# Preprocessed VMat" << endl;
        out_uni << "<SOURCES>" << endl;
        out_uni << "@" << endl
                << "ShiftAndRescaleVMatrix(" << endl
                << "  automatic = 0" << endl
                << "  shift = [" << shift << "]" << endl
                << "  scale = [" << scale << "]" << endl
                << "  underlying_vmat =" << endl;
        out_uni << "   PLearnerOutputVMatrix(" << endl;
        out_uni << "     train_learners = 1" << endl;
        out_uni << "     data = AutoVMatrix(specification = \"" << filename_non_uni << "\")" << endl;
        out_uni << "     learners = [" << endl;
        out_uni << "       UniformizeLearner(" << endl;
        out_uni << "         which_fieldnums = ";
        out_uni << "[ " << need_to_be_uniformized << "]" << endl;
        out_uni << "       )" << endl;
        out_uni << "     ]" << endl;
        out_uni << "   )" << endl
                << ")" << endl;
        out_uni << "</SOURCES>" << endl << endl;
        out_uni << "<SIZES>"  << endl
                << inputsize  << endl // inputsize
                << "1"        << endl // targetsize
                << "0"        << endl // weightsize
                << "</SIZES>" << endl;
    }

    // Possibly add the <PRECOMPUTE> tag.
    if (precompute != "none") {
        out << endl << "<PRECOMPUTE>" << endl << precompute << endl << "</PRECOMPUTE>" << endl;
        if (uniformize > 0) {
            out_uni << endl << "<PRECOMPUTE>" << endl << precompute << endl << "</PRECOMPUTE>" << endl;
        }
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


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
