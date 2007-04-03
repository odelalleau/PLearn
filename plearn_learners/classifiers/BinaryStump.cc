// -*- C++ -*-

// BinaryStump.cc
//
// Copyright (C) 2004 Hugo Larochelle 
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

// Authors: Hugo Larochelle

/*! \file BinaryStump.cc */


#include "BinaryStump.h"

namespace PLearn {
using namespace std;


void qsort_vec(TVec< pair<int, real> > v, TVec< pair<int,int> > buffer)
{
    TVec< pair<int,real> > temp(v.length());
    temp << v;
    real pivot = temp[0].second;
    int first = 0;
    int last = v.length()-1;
    for(int i=1; i<v.length(); i++)
        if(temp[i].second >= pivot)
            v[last--]=temp[i];
        else
            v[first++]=temp[i];
  
    if(first != last)
        PLERROR("OUPS!!");
  
    v[first] = temp[0];

    int it = 0;
    pair<int,int> inf_sup;
    if(first != 0)
    {
        inf_sup.first = 0;
        inf_sup.second = first;
        buffer[it] = inf_sup;
        it++;
    }
    if(last!=temp.length()-1)
    {
        inf_sup.first = last+1;
        inf_sup.second = v.length()-1-last;
        buffer[it] = inf_sup;
        it++;
    }

    while(it > 0)
    {
        it--;
        temp.resize(buffer[it].second);
        temp << v.subVec(buffer[it].first,buffer[it].second);
        pivot = temp[0].second;
        first = buffer[it].first;
        last = buffer[it].first+buffer[it].second-1;
        for(int i=1; i<buffer[it].second; i++)
            if(temp[i].second >= pivot)
                v[last--]=temp[i];
            else
                v[first++]=temp[i];
    
        if(first != last)
            PLERROR("OUPS!!");
    
        v[first] = temp[0];
    
        int this_it = it;

        if(first != buffer[this_it].first)
        { 
            inf_sup.first =  buffer[this_it].first;
            inf_sup.second = first-buffer[this_it].first;
            buffer[it] = inf_sup;
            it++;
        }
        if(last!=  buffer[this_it].first+temp.length()-1)
        {
            inf_sup.first =  last+1;
            inf_sup.second = buffer[this_it].first+temp.length()-1-last;
            buffer[it] = inf_sup;
            it++;
        }
    }

}

BinaryStump::BinaryStump() 
/* ### Initialize all fields to their default value here */
{
    feature = 0;
    tag = 0;
    threshold = 0;
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(BinaryStump, "Binary stump classifier", 
                        "This algorithm finds the most accurate binary stump\n"
                        "that classifies to a certain tag (0 or 1)\n"
                        "every points that have a certain feature (coordinate)\n"
                        "higher than a learned threshold.\n"
                        "The tag, feature and threshold are chosen to minimize\n"
                        "the weighted classification error.\n"
                        "Only the first target is considered, the others are \n"
                        "ignored.\n");

void BinaryStump::declareOptions(OptionList& ol)
{
    declareOption(ol, "feature", &BinaryStump::feature, OptionBase::learntoption,
                  "Feature tested by the stump");
    declareOption(ol, "threshold", &BinaryStump::threshold, 
                  OptionBase::learntoption,
                  "Threshold for decision");
    declareOption(ol, "tag", &BinaryStump::tag, OptionBase::learntoption,
                  "Tag assigned when feature is lower than the threshold");

    inherited::declareOptions(ol);
}

void BinaryStump::build_(){}

void BinaryStump::build()
{
    inherited::build();
    build_();
}


void BinaryStump::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


int BinaryStump::outputsize() const
{
    return 1;
}

void BinaryStump::forget()
{
    stage = 0;
    feature = 0;
    tag = 0;
    threshold = 0;
}
    
void BinaryStump::train()
{

    if(!train_set)
        PLERROR("In BinaryStump:train() : train_set not specified");

    if(!train_stats)  // make a default stats collector, in case there's none
        train_stats = new VecStatsCollector();
    train_stats->forget();

    int n = train_set->length();
    sf.resize(n);
    //static Vec input; input.resize(inputsize());
    //static Vec target; target.resize(targetsize());
    real input;
    //real weight;
    Vec train_target(n);
    TVec< pair<int,int> > buffer((int)(n*safeflog(n)));

    static Vec example_weights; example_weights.resize(n);

    // Extracting weights
    if(train_set->weightsize() > 0)
    {
        for (int i=0; i<n; ++i) 
        {
            //train_set->getExample(i, input, target, weight);
            //example_weights[i]=weight;
            example_weights[i]= train_set->get(i,inputsize_+targetsize_);
        }
    }
    else
    {
        example_weights.fill(1.0/n);
    }

    for (int i=0; i<n; ++i) 
    {
        train_target[i]= train_set->get(i,inputsize_);
        if(!fast_exact_is_equal(train_target[i], 0) &&
           !fast_exact_is_equal(train_target[i], 1))
            PLERROR("In BinaryStump:train() : target should be either 1 or 0");
    }

    // Choosing best stump

    real best_error = 0;

    {
        real w_sum_1 = 0;
        real w_sum_error = 0;
        real w_sum = 0;

        for(int i=0; i<n; i++)
        {
            w_sum += example_weights[i];
            //train_set->getExample(i,input,target,weight);
            //if(target[0] == 1)
            if(fast_exact_is_equal(train_target[i], 1))
                w_sum_1 += example_weights[i];
        }

        if(w_sum_1 > w_sum - w_sum_1)
        {
            tag = 0;
            w_sum_error = w_sum - w_sum_1;
        }
        else
        {
            tag = 1;
            w_sum_error = w_sum_1;
        }
  
        best_error = w_sum_error;

        // We choose as the first stump to consider, the stump that classifies
        // in the most frequent class
        // every points which have their first coordonate greater than
        // the smallest value for this coordonate in the training set MINUS ONE.
        // This approximatly corresponds to classify any points to the most
        // frequent class.

        feature = 0;
        threshold = sf[0].second-1; 
        PP<ProgressBar> pb;
        if(report_progress)
            pb = new ProgressBar("Finding best stump",inputsize()*sf.length());
        int prog = 0;
        for(int d=0; d<inputsize(); d++)
        {

            // Copying input
            for(int j=0; j<n; j++)
            {
                //train_set->getExample(j,input, target, weight);
                input = train_set->get(j,d);
                //if(target[0] != 0 & target[0] != 1)
                sf[j].first = j;
                //sf[j].second = input[d];
                sf[j].second = input;
            }
      

      
            // Sorting features
            //for(int i=0; i<sf.length();i++)
            qsort_vec(sf,buffer);
      
            if(d==0) // initialize threshold
                threshold = sf[0].second-1; 

            real w_sum_l_1 = 0;
            real w_sum_l = 0;

            for(int i=0; i<sf.length()-1; i++)
            {

                real f1 = sf[i].second;
                real f2 = sf[i+1].second;

                //train_set->getExample(sf[i].first,input,target,weight);
                //target = train_set->getExample(sf[i].first,inputsize_);
                //real classe = target[0];
                real classe = train_target[sf[i].first];
                if(fast_exact_is_equal(classe, 1))
                    w_sum_l_1+=example_weights[sf[i].first];
                w_sum_l += example_weights[sf[i].first];

                if(fast_exact_is_equal(f1, f2))
                    continue;

                real w_sum_error_1 = w_sum_l - w_sum_l_1 + w_sum_1 - w_sum_l_1;
                real c_w_sum_error = 0;
                if(w_sum_error_1 > w_sum - w_sum_error_1)
                {
                    c_w_sum_error = w_sum - w_sum_error_1;
        
                }
                else
                {
                    c_w_sum_error = w_sum_error_1;
                }
      
                // We choose the first stump that minimizes the
                // weighted error

                if(best_error > c_w_sum_error)
                {
                    best_error = c_w_sum_error;
          
                    tag = w_sum_error_1 > w_sum - w_sum_error_1 ? 0 : 1;
                    threshold = (f1+f2)/2;
                    feature = d;
                }
            }
            prog++;
            if(report_progress) pb->update(prog);
        }
    }
  
    Vec costs(1); costs[0] = best_error;
    train_stats->update(costs);
    train_stats->finalize();
    if(verbosity > 1)
        cout << "Weighted error = " << best_error << endl;
    sf = TVec< pair<int, real> >(0);
}


void BinaryStump::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());
    if(input[feature] < threshold) output[0] = tag;
    else output[0] = 1-tag;
}    

void BinaryStump::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                          const Vec& target, Vec& costs) const
{
    costs.resize(outputsize());

    if(!fast_exact_is_equal(target[0], 0) &&
       !fast_exact_is_equal(target[0], 1))
        PLERROR("In BinaryStump:computeCostsFromOutputs() : "
                "target should be either 1 or 0");

    costs[0] = !fast_exact_is_equal(output[0], target[0]); 
}                                

TVec<string> BinaryStump::getTestCostNames() const
{
    return getTrainCostNames();
}

TVec<string> BinaryStump::getTrainCostNames() const
{
    TVec<string> costs(1);
    costs[0] = "binary_class_error";
    return costs;
}


} // end of namespace PLearn


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
