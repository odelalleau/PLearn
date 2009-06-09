// -*- C++ -*-

// UniformizeLearner.cc
//
// Copyright (C) 2004 ApSTAT Technologies Inc. 
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

// Authors: Pascal Vincent

/*! \file UniformizeLearner.cc */

#include "UniformizeLearner.h"

namespace PLearn {
using namespace std;

UniformizeLearner::UniformizeLearner() 
    :weight_field_index(-1),
     nquantiles(200),
     raw_inputs_as_output(false)
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(UniformizeLearner, "Uniformizes selected input fields", 
                        "For each specified field, the full training set column will be read,\n"
                        "then sorted, and we'll store up to nquantiles and their mapping to [0,1] rank (as well as min and max)\n"
                        "Uniformization maps to [0,1]. It is a piecewise linear interpolation between the remembered quantiles\n"
                        "Work with missing value. We don't map them");

void UniformizeLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &UniformizeLearner::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    //build
    
    declareOption(ol, "which_fieldnames", &UniformizeLearner::which_fieldnames, OptionBase::buildoption,
                  "The names of the fields to uniformize.\n"
                  "If both which_fieldnames and which_fieldnums are empty, all fields are normalized.");
    declareOption(ol, "which_fieldnums", &UniformizeLearner::which_fieldnums, OptionBase::buildoption,
                  "The indexes of the fields to uniformize. Leave this option empty if you specify which_fieldnames.\n"
                  "If both which_fieldnames and which_fieldnums are empty, all fields are normalized.");
    declareOption(ol, "nquantiles", &UniformizeLearner::nquantiles, OptionBase::buildoption,
                  "How many intervals to use to divide the sorted values");

    declareOption(ol, "raw_inputs_as_output", &UniformizeLearner::raw_inputs_as_output, OptionBase::buildoption,
                  "If true, raw inputs are appended to uniformized outputs for all uniformized fields.");

    //learnt

    declareOption(ol, "val_to_rank", &UniformizeLearner::val_to_rank, OptionBase::learntoption,
                  "Remembers mapping between a few values and their [0,1] ranking.");

    declareOption(ol, "input_field_names", &UniformizeLearner::input_field_names, OptionBase::learntoption,
                  "Remembers the names of the input fields.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void UniformizeLearner::build_()
{
    
}

// ### Nothing to add here, simply calls build_
void UniformizeLearner::build()
{
    inherited::build();
    build_();
}


void UniformizeLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(val_to_rank,      copies);
    deepCopyField(which_fieldnames, copies);
    deepCopyField(which_fieldnums,  copies);
    deepCopyField(input_field_names,  copies);
}


////////////////
// outputsize //
////////////////
int UniformizeLearner::outputsize() const
{
    int nk= 0;
    if(raw_inputs_as_output)
    {
        nk= which_fieldnames.length();
        if(nk == 0)
            nk= which_fieldnums.size();
        if(nk == 0)//no field specified: uniformize all
            nk= inputsize();
    }
    return nk+inputsize();
}

////////////
// forget //
////////////
void UniformizeLearner::forget()
{
    stage = 0; // untrained
}

////////////////////
// setTrainingSet //
////////////////////
void UniformizeLearner::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);
    VMat dataset = getTrainingSet();

    if(dataset->weightsize() > 1)
        PLERROR("In UniformizeLearner::setTrainingSet: Only one weight supported.");

    if(train_set->weightsize() > 0)
        weight_field_index= dataset->fieldIndex(dataset->weightFieldNames()[0]);
    
    input_field_names.resize(dataset->inputsize());
    input_field_names << dataset->inputFieldNames();

    int nk = which_fieldnames.length();
    if(nk==0)
        nk = which_fieldnums.size();
    else
    {
        which_fieldnums.resize(nk);
        for(int k=0; k<nk; k++)
            which_fieldnums[k] = train_set->getFieldIndex(which_fieldnames[k]);
    }

    if(nk == 0)//no field specified, uniformize all.
    {
        nk= train_set->inputsize();
        which_fieldnums.resize(nk);
        for(int k= 0; k < nk; ++k)
            which_fieldnums[k]= k;
    }
}
    
///////////
// train //
///////////
void UniformizeLearner::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.

    /* TYPICAL CODE:      
       static Vec input  // static so we don't reallocate/deallocate memory each time...
       static Vec target
       input.resize(inputsize())    // the train_set's inputsize()
       target.resize(targetsize())  // the train_set's targetsize()
       real weight

       if(!train_stats)  // make a default stats collector, in case there's none
       train_stats = new VecStatsCollector()

       if(nstages<stage) // asking to revert to a previous stage!
       forget()  // reset the learner to stage=0

       while(stage<nstages)
       {
       // clear statistics of previous epoch
       train_stats->forget() 
          
       //... train for 1 stage, and update train_stats,
       // using train_set->getSample(input, target, weight)
       // and train_stats->update(train_costs)
          
       ++stage
       train_stats->finalize() // finalize statistics for this epoch
       }
    */

    if(stage==0) // untrained
    {
        int nk = which_fieldnames.length();
        if(nk==0)
            nk = which_fieldnums.size();
        else
        {
            which_fieldnums.resize(nk);
            for(int k=0; k<nk; k++)
                which_fieldnums[k] = train_set->getFieldIndex(which_fieldnames[k]);
        }

        if(nk == 0)//no field specified, uniformize all.
        {
            nk= train_set->inputsize();
            which_fieldnums.resize(nk);
            for(int k= 0; k < nk; ++k)
                which_fieldnums[k]= k;
        }


        int l = train_set->length();

        bool weighted= train_set->weightsize() == 1;

        static Vec colw;
        if(weighted)
        {
            colw.resize(l);
            train_set->getColumn(weight_field_index, colw);
        }

        static Vec colv;
        colv.resize(l);
      
        val_to_rank.resize(nk);
        for(int k=0; k<nk; k++)
        {
            train_set->getColumn(which_fieldnums[k],colv);
            if(weighted)
                computeWeightedRankMap(colv, nquantiles, val_to_rank[k], colw);
            else
                computeRankMap(colv, nquantiles, val_to_rank[k]);
        }
        stage = 1; // trained
    }
}

//////////////////
// v_no_missing //
//////////////////
Vec UniformizeLearner::v_no_missing;

////////////////////
// computeRankMap //
////////////////////
void UniformizeLearner::computeRankMap(const Vec& v, int nquantiles,
                                       map<real,real>& rankmap)
{
    v_no_missing.resize(v.length()); // Allocate enough memory.
    if (!v.hasMissing())
        v_no_missing << v;
    else {
        v_no_missing.resize(0);
        for (int i = 0; i < v.length(); i++)
            if (!is_missing(v[i]))
                v_no_missing.append(v[i]);
    }
    rankmap.clear();
    int max_index = v_no_missing.length() - 1;
    sortElements(v_no_missing);
    rankmap[v_no_missing[0]] = 0;
    rankmap[v_no_missing[max_index]] = 1;
    for(int k=1; k<nquantiles; k++)
    {
        real rank = real(k)/real(nquantiles);
        int pos = int(round(rank * max_index));
        real val = v_no_missing[pos];
        if(rankmap.find(val) == rankmap.end())
            rankmap[val] = rank;
    }
}


void UniformizeLearner::computeWeightedRankMap(const Vec& v, int nquantiles, map<real,real>& rankmap, const Vec& weights)
{
    int l= v.length();

    Mat vw(0, 2);

    if (!v.hasMissing())
    {
        vw.resize(l,2);
        vw.column(0) << v;
        vw.column(1) << weights;

    }
    else 
    {
        Vec vvw(2);
        for (int i = 0; i < l; i++)
            if (!is_missing(v[i]))
            {
                vvw[0]= v[i];
                vvw[1]= weights[i];
                vw.appendRow(vvw);
            }
    }

    

    rankmap.clear();
    int max_index = vw.length() - 1;
    sortRows(vw, TVec<int>(1,0));

    for (int i = 1; i < l; i++)
        vw(i,1)+= vw(i-1,1);

    rankmap[vw(0,0)] = 0;
    rankmap[vw(max_index,0)] = 1;
    real totw= vw(max_index,1);

    for(int k=1, i= 0; k<nquantiles; ++k)
    {
        real rank = real(k)/real(nquantiles);
        real qw= totw*rank;
        while(vw(i,1) < qw)
            ++i;

        real val = vw(i,0);

        rank= vw(i,1)/totw;

        if(rankmap.find(val) == rankmap.end())
            rankmap[val]= rank;
    }
}


///////////////
// mapToRank //
///////////////
real UniformizeLearner::mapToRank(real val, const map<real,real>& rankmap)
{
    PLASSERT( !is_missing(val) );
    real minv = rankmap.begin()->first;
    if(val<=minv)
        return 0;
    real maxv = rankmap.rbegin()->first;
    if(val>=maxv)
        return 1;
    map<real,real>::const_iterator high = rankmap.upper_bound(val);
    map<real,real>::const_iterator low = high; --low;

    real rank = low->second + (val-low->first)*(high->second-low->second)/(high->first-low->first);
    return rank;
}

///////////////////
// computeOutput //
///////////////////
void UniformizeLearner::computeOutput(const Vec& input, Vec& output) const
{
/*
    int nout = outputsize();
    output.resize(nout);
    output << input;
    for(int k=0; k<which_fieldnums.size(); k++)
    {
        int fieldnum = which_fieldnums[k];
        if (!is_missing(output[fieldnum]))
            output[fieldnum] = mapToRank(output[fieldnum], val_to_rank[k]);
    }
*/
    int n= outputsize();
    output.resize(n);
    int nk= which_fieldnums.size();
    for(int k= 0; k < nk; ++k){
        real val=input[which_fieldnums[k]];
        if(is_missing(val))
            output[k] = MISSING_VALUE;
        else
            output[k] = mapToRank(val, val_to_rank[k]);
    }
    for(int k= nk; k < n; ++k)
        output[k]= input[k-nk];

}    

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void UniformizeLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                const Vec& target, Vec& costs) const
{
    costs.resize(0);
}                                

TVec<string> UniformizeLearner::getTestCostNames() const
{
    static TVec<string> nocosts;
    return nocosts;
}

TVec<string> UniformizeLearner::getTrainCostNames() const
{
    static TVec<string> nocosts;
    return nocosts;
}


TVec<string> UniformizeLearner::getOutputNames() const
{
    int n = outputsize();
    TVec<string> outnames(n);
    int nk= which_fieldnums.size();

    for(int k= 0; k < nk; ++k)
        outnames[k]= string("uniformized_")+input_field_names[which_fieldnums[k]];
    for(int k= nk; k < n; ++k)
        outnames[k]= input_field_names[k-nk];

    return outnames;
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
