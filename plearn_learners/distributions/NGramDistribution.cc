// -*- C++ -*-

// NGramDistribution.cc
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

/*! \file NGramDistribution.cc */


#include "NGramDistribution.h"
#include <plearn/vmat/FractionSplitter.h>
#include <plearn/vmat/RepeatSplitter.h>

namespace PLearn {
using namespace std;

///////////////////////
// NGramDistribution //
///////////////////////
NGramDistribution::NGramDistribution() :
    nan_replace(false),n(2),additive_constant(0),discount_constant(0.01), validation_proportion(0.10), smoothing("no_smoothing"),lambda_estimation("manual")
{
    forget();
    // In a N-Gram, the predicted size is always one.
    predicted_size = 1;
}

PLEARN_IMPLEMENT_OBJECT(NGramDistribution,
                        "NGram distribution P(w_i|w_{i-n+1}^{i-1})",
                        "Takes a sequence of contexts of symbols (integers)"
                        "and computes a ngram language model. Several smoothing techniques"
                        "are offered."
    );

////////////////////
// declareOptions //
////////////////////
void NGramDistribution::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "nan_replace", &NGramDistribution::nan_replace, OptionBase::buildoption,
                  "Indication that the missing values in context (nan) should be\n"
                  "replaced by a default value (-1). nan fields should correspond\n"
                  "to context not accessible (like in the beginning of a sentence).\n"
                  "If this parameter is false, than the shortest ngram is inserted\n"
                  "in the NGramTree."
        );

    declareOption(ol, "n", &NGramDistribution::n, OptionBase::buildoption,
        "Length of the n-gram (this option overrides the inherited options\n"
        "'predictor_size' and 'predicted_size', i.e. predictor_size = n-1\n"
        "and predicted_size = 1.");

    declareOption(ol, "additive_constant", &NGramDistribution::additive_constant, OptionBase::buildoption,
                  "Additive constant for add-delta smoothing");
    declareOption(ol, "discount_constant", &NGramDistribution::discount_constant, OptionBase::buildoption,
                  "Discount constant for absolut discounting smoothing");
    declareOption(ol, "validation_proportion", &NGramDistribution::validation_proportion, OptionBase::buildoption,
                  "Proportion of the training set used for validation (EM)");
    declareOption(ol, "smoothing", &NGramDistribution::smoothing, OptionBase::buildoption,
                  "Smoothing method. Choose among:\n"
                  "- \"no_smoothing\"\n"
                  "- \"add-delta\"\n"
                  "- \"jelinek-mercer\"\n"
                  "- \"witten-bell\"\n"
                  "- \"absolute-discounting\"\n"
        );
    declareOption(ol, "lambda_estimation", &NGramDistribution::lambda_estimation, OptionBase::buildoption,
                  "Lambdas estimation method. Choose among:\n"
                  "- \"manual\" (lambdas field should be specified)\n"
                  "- \"EM\"\n"
        );
    declareOption(ol, "lambdas", &NGramDistribution::lambdas, OptionBase::buildoption,
                  "Lambdas of the interpolated ngram");
    declareOption(ol, "tree", &NGramDistribution::tree, OptionBase::buildoption,
                  "NGramTree of the frequencies");

    // Now call the parent class' declareOptions().
    inherited::declareOptions(ol);

    redeclareOption(ol, "predictor_size",  &NGramDistribution::predictor_size,
                  OptionBase::nosave,
                  "Defined at build time.");

    redeclareOption(ol, "predicted_size",  &NGramDistribution::predicted_size,
                  OptionBase::nosave,
                  "Defined at build time.");
}

///////////
// build //
///////////
void NGramDistribution::build()
{
    predictor_size = n - 1;
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void NGramDistribution::build_()
{
    //predictor_size = n - 1;
    // We need to re-build the parent class, so that data related to
    // 'predictor_size' is correctly defined.
    inherited::build();

    if(train_set)
    {
        if(inputsize() != n) PLERROR("In NGramDistribution:build_() : input size should be n=%d", n);
        voc_size = train_set->getValues(0,n-1).length();
        if(voc_size <= 0) PLERROR("In NGramDistribution:build_() : vocabulary size is <= 0");

        if(nan_replace) voc_size++;

        if(smoothing == "absolute-discounting")
        {
            if(discount_constant < 0 || discount_constant > 1)
                PLERROR("In NGramDistribution:build_() : discount constant should be in [0,1]");
        }
    }
}

/////////
// cdf //
/////////
real NGramDistribution::cdf(const Vec& y) const
{
    PLERROR("cdf not implemented for NGramDistribution"); return 0;
}

/////////////////
// expectation //
/////////////////
void NGramDistribution::expectation(Vec& mu) const
{
    PLERROR("expectation not implemented for NGramDistribution");
}


////////////
// forget //
////////////
void NGramDistribution::forget()
{
    tree = new NGramTree();
}

//////////////
// generate //
//////////////
void NGramDistribution::generate(Vec& y) const
{

    PLERROR("generate not implemented for NGramDistribution");
}

/////////////////
// log_density //
/////////////////
real NGramDistribution::log_density(const Vec& y) const
{
    return safeflog(density(y));
}

real NGramDistribution::density(const Vec& y) const
{
    if(is_missing(y[0])) PLERROR("In NGramDistribution:density() : y[0] is missing");

    // Making ngram

    static TVec<int> ngram;

    Vec row(n);
    row[n-1] = y[0];
    for(int i=0; i<n-1; i++)
        row[i] = predictor_part[i];

    getNGrams(row,ngram);

    // Computing P(w_i|w_{i-n+1}^{i-1})

    TVec<int> freq;
    TVec<int> normalization;
    int ngram_length = ngram.length();

    if(smoothing == "no_smoothing")
    {
        freq = tree->freq(ngram);
        normalization = tree->normalization(ngram);
        if(normalization[ngram_length-1] == 0)
            return 1.0/voc_size;
        return ((real)freq[ngram_length-1])/normalization[ngram_length-1];
    }
    else if(smoothing == "add-delta")
    {
        freq = tree->freq(ngram);
        normalization = tree->normalization(ngram);
        return ((real)freq[ngram_length-1] + additive_constant)/(normalization[ngram_length-1] + additive_constant*voc_size);
    }
    else if(smoothing == "jelinek-mercer")
    {
        freq = tree->freq(ngram);
        normalization = tree->normalization(ngram);
        real ret = 1.0/voc_size*lambdas[0];
        real norm = lambdas[0]; // For ngram smaller than n...

        for(int j=0; j<ngram_length;j++)
        {
            if(normalization[j] != 0)
            {
                ret += lambdas[j+1] * (((real)freq[j])/normalization[j]);
                norm += lambdas[j+1];
            }
        }
        return ret/norm;
    }
    else if(smoothing == "absolute-discounting")
    {
        freq = tree->freq(ngram);
        normalization = tree->normalization(ngram);
        TVec<int> n_freq = tree->n_freq(ngram);
        real ret = 0;
        real factor = 1;
        for(int j=ngram_length-1; j>=0; j--)
        {
            if(normalization[j] != 0)
            {
                ret += factor * ((real)(freq[j] > discount_constant ? freq[j] - discount_constant : 0))/ normalization[j];
                factor = factor * ((real)discount_constant)/normalization[j] * n_freq[j];
            }
        }
        ret += factor *1.0/voc_size;

        return ret;
    }
    else if(smoothing == "witten-bell")
    {
        freq = tree->freq(ngram);
        normalization = tree->normalization(ngram);
        TVec<int> n_freq = tree->n_freq(ngram);
        real ret = 1.0/voc_size;
        for(int j=0; j<ngram_length; j++)
        {
            if(normalization[j] != 0)
                ret = (freq[j]+n_freq[j]*ret)/(normalization[j]+n_freq[j]);
        }

        return ret;
    }
    else PLERROR("In NGramDistribution:density() : smoothing technique not valid");
    return 0;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NGramDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(lambdas, copies);
    deepCopyField(tree, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("NGramDistribution::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////////
// survival_fn //
/////////////////
real NGramDistribution::survival_fn(const Vec& y) const
{
    PLERROR("survival_fn not implemented for NGramDistribution"); return 0;
}

//////////////
// variance //
//////////////
void NGramDistribution::variance(Mat& covar) const
{
    PLERROR("variance not implemented for NGramDistribution");
}

void NGramDistribution::getNGrams(Vec row, TVec<int>& ngram) const
{
    if(is_missing(row[row.length()-1])) PLERROR("In getNGrams() : last element of row is NaN");

    int insert_from = 0;
    //Looking for nan
    if(!nan_replace)
        for(int j=0; j<row.length(); j++)
            if(is_missing(row[j]))
                insert_from = j+1;

    ngram.resize(n-insert_from);

    //Making ngram
    for(int j=insert_from; j<row.length(); j++)
    {
        if(is_missing(row[j]))
            ngram[j-insert_from] = -1;
        else
            ngram[j-insert_from] = (int)row[j];
    }
}

void NGramDistribution::train()
{
    VMat contexts_train;
    VMat contexts_validation;


    if(smoothing == "jelinek-mercer" && lambda_estimation == "EM")
    {
        if(validation_proportion <= 0 || validation_proportion >= 1)
            PLERROR("In NGramDistribution:build_() : validation_proportion should be in (0,1)");
        // Making FractionSplitter
        PP<FractionSplitter> fsplit = new FractionSplitter();
        TMat<pair<real,real> > splits(1,2);
        splits(0,0).first = 0; splits(0,0).second = 1-validation_proportion;
        splits(0,1).first = 1-validation_proportion; splits(0,1).second = 1;
        fsplit->splits = splits;
        fsplit->build();

        // Making RepeatSplitter
        PP<RepeatSplitter> rsplit = new RepeatSplitter();
        rsplit->n = 1;
        rsplit->shuffle = true;
        rsplit->seed = 123456;
        rsplit->to_repeat = fsplit;
        rsplit->setDataSet(train_set);
        rsplit->build();

        TVec<VMat> vmat_splits = rsplit->getSplit();
        contexts_train = vmat_splits[0];
        contexts_validation = vmat_splits[1];
    }
    else
        contexts_train = train_set;

    //Putting ngrams in the tree
    Vec row(n);
    TVec<int> int_row(n);


    ProgressBar* pb =  new ProgressBar("Inserting ngrams in NGramTree", contexts_train->length());
    for(int i=0; i<contexts_train->length(); i++)
    {
        contexts_train->getRow(i,row);
        getNGrams(row,int_row);
        tree->add(int_row);

        pb->update(i+1);
    }

    delete(pb);

    // Smoothing techniques parameter estimation
    if(smoothing == "jelinek-mercer")
    {
        //Jelinek-Mercer: EM estimation of lambdas
        if(lambda_estimation == "EM")
        {
            lambdas.resize(n+1); lambdas.fill(1.0/(n+1));
            real diff = EM_PRECISION+1;
            real l_old = 0, l_new = -REAL_MAX;
            Vec e(n+1);
            Vec p(n+1);
            TVec<int> ngram(n);
            real p_sum = 0;
            int n_ngram = 0;
            while(diff > EM_PRECISION)
            {
                if(verbosity > 2)
                    cout << "EM diff: " << diff << endl;
                n_ngram = 0;
                l_old = l_new; l_new = 0;

                // E step

                e.fill(0);
                for(int t=0; t<contexts_validation->length(); t++)
                {
                    p_sum = 0;

                    // get w_{t-n+1}^t

                    contexts_validation->getRow(t,row);
                    getNGrams(row,ngram);

                    TVec<int> freq = tree->freq(ngram);
                    TVec<int> normalization = tree->normalization(ngram);
                    if(normalization[ngram.length()-1] != 0)
                    {
                        n_ngram++;
                        p.fill(0);
                        p[0] = lambdas[0]*1.0/voc_size;
                        p_sum += p[0];
                        for(int j=0; j<ngram.length(); j++)
                        {
                            p[j+1] = lambdas[j+1]*(((real)freq[j])/normalization[j]);
                            p_sum += p[j+1];
                        }

                        for(int j=0; j<e.length(); j++)
                            e[j] += p[j]/p_sum;
                        l_new += safeflog(p_sum);
                    }
                }
                if(n_ngram == 0) PLERROR("In NGramDistribution:train() : no ngram in validation set");
                // M step
                for(int j=0; j<lambdas.length(); j++)
                    lambdas[j] = e[j]/n_ngram;

                diff = l_new-l_old;
            }

            //Test

            real temp = 0;
            for(int j=0; j<lambdas.length(); j++)
                temp += lambdas[j];
            if(abs(temp-1) > THIS_PRECISION)
                PLERROR("oups, lambdas don't sum to one after EM!!");
        }
        else if(lambda_estimation == "manual")
        {
            if(lambdas.length() != n+1) PLERROR("In NGramDistribution:build_() : lambdas' length should be %d, not %d", n+1, lambdas.length());
            real sum = 0;
            for(int j=0; j<lambdas.length(); j++)
            {
                if(lambdas[j]<0) PLERROR("In NGramDistribution:build_() : all lambdas should be non negative");
                sum += lambdas[j];
            }
            if(abs(sum) < THIS_PRECISION)
                lambdas.fill(1.0/(n+1));
            else
                lambdas *= 1.0/sum;
        }
        else PLERROR("In NGramDistribution:build_() : lambda estimation not valid");

    }

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
