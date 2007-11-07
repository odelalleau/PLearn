// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/db/databases.h */

#ifndef DATABASES_INC
#define DATABASES_INC

#include "UCISpecification.h"
#include <plearn/vmat/VMat.h>
//#include "NistDB.h"

namespace PLearn {
using namespace std;


//!  This will input a 2d binary classification problem (launches a java applet)
Mat input2dSet(const PPath& filename="data2d.amat");

//!  normalize both training_set and test_set according to mean and stddev computed on training_set
void normalizeDataSets(Mat& training_set, Mat& test_set);
void normalizeDataSets(Mat& training_set, Mat& validation_set, Mat& test_set);
void normalizeDataSets(VMat& training_set, VMat& validation_set, VMat& test_set);

// split the data_set into training_set, valid_set and test_set
void splitTrainValidTest(VMat &data_set,VMat &train_set,VMat &valid_set,
			 real valid_fraction,VMat &test_set, real test_fraction,
			 bool normalize=true);

// reduce nb of inputs (usefull for rapid experimentation)
VMat reduceInputSize(real fraction,VMat data);

// reduce nb of examples 
VMat reduceDataSetSize(real fraction,VMat data);

//!  remaps classnums to {0,1} or to {-1,+1}
void remapClassnums(VMat& data, real remap_minval_to, real remap_maxval_to);

//!  These calls return the number of classes...
int loadBreastCancer(VMat& training_set, VMat& validation_set, VMat& test_set, int ntrain, int nvalid, bool uniq=true);
int loadDiabetes(VMat& training_set, VMat& validation_set, VMat& test_set, int ntrain, int nvalid);
int loadATT800(VMat& training_set, VMat& test_set);
int loadLetters(VMat& training_set, VMat& validation_set, VMat& test_set, char* letters="ABCDEFGHIJKLMNOPQRSTUVWXYZ", real validation_fraction=.2, real test_fraction=.4,bool do_shuffle=true);
int loadLetters(VMat& training_set, VMat& validation_set, VMat& test_set, int n_letters, real validation_fraction=.2, real test_fraction=.4,bool do_shuffle=true);

void loadCorelDatamat(int classnum, Mat& train, Mat& valid, Mat& test);
Mat smoothCorelHisto(Mat& data);
void loadCorel(Mat& training_set, Mat& validation_set, Mat& test_set, int negative_class=2, int positive_class=3);
void loadCallxx(int year, VMat& d);

VMat loadBreastCancerWisconsin(bool normalize=true, bool uniq=true);
VMat loadSonar();
VMat loadIonosphere();
VMat loadDiabetes(bool normalize=true);
VMat loadPimaIndians(bool normalize=true);
VMat loadLetters(const char* class0, const char* class1, bool normalize=true);
VMat loadLetters(bool normalize=true);
VMat loadLetters(int n_letters, bool do_shuffle);
void loadUSPS(VMat& trainset, VMat& testset, bool use_smooth=true);
VMat loadUSPS(bool use_smooth=true);
VMat loadHousing(bool normalize=true);
//! Load the train, test and all datasets for a UCI database.
//! The 'normalize' parameter can be changed: if it is set to true in input, it may be changed
//! to false when the method returns (this is because the data will already be normalized, and
//! no additional normalization is needed).
void loadUCI(VMat& trainset, VMat& testset, VMat& allset, string db_spec, string id, bool &normalize, const string& type);
//! Load a specific UCI dataset in the given VMatrix.
void loadUCISet(VMat& data, string file, PP<UCISpecification> uci_spec);
//! Load a AMAT format UCI dataset in the given VMatrix.
void loadUCIAMat(VMat& data, string file, PP<UCISpecification> uci_spec);

//! Load a specific UCI dataset in the given VMatrix.
void loadUCISet(VMat& data, PP<UCISpecification> uci_spec);

/*!   This will return a VMat with a target in the last column in {0,..,nclasses-1} (for binary classification possible values are 0 and 1 (not -1)). 
  Possible dbname are:
  2d
  letters
  breast
  usps
  mnist
  usps0 ... usps9
  nist0 ... usps9
  The dbname can optionally be followed by :size in which case only the 'size'
  first elements of trainset and testset will be kept.
*/

inline string loadClassificationDatasetHelp()
{
    return "  Preprogrammed datasets are: \n"
        "    2d \n"
        "    letters \n"
        "    breast \n"
        "    usps \n"
        "    mnist \n"
        "    usps0 ... usps9 \n"
        "    nist0 ... usps9 \n"
        "    They can optionally be followed by :size in which case only the 'size' \n"
        "    first rows will be kept. \n";
}

inline string loadUCIDatasetsHelp()
{
    return 
        "In order to access the UCI datasets the dataset name must start with UCI_. The possible\n"
        "dataset names are:\n"
        "    UCI_annealing\n"
        "    UCI_heart-disease_ID=va\n"
        "    UCI_heart-disease_ID=cleveland\n"
        "    UCI_heart-disease_ID=hungarian\n"
        "    UCI_heart-disease_ID=switzerland\n"
        "    UCI_housing\n"
        "    UCI_image\n"
        "    UCI_ionosphere\n"
        "    UCI_iris\n"
        "    UCI_iris_ID=bezdekIris\n"
        "    UCI_isolet_ID=1+2+3+4\n"
        "    UCI_isolet_ID=5\n"
        "    UCI_monks-problems_ID=monks-1\n"
        "    UCI_monks-problems_ID=monks-2\n"
        "    UCI_monks-problems_ID=monks-3\n"
        "    UCI_mushroom\n"
        "    UCI_musk_ID=clean1\n"
        "    UCI_musk_ID=clean2\n"
        "    UCI_page-blocks\n"
        "    UCI_pima-indians-diabetes\n"
        "    UCI_solar-flare_ID=data1\n"
        "    UCI_solar-flare_ID=data2\n"
        "    UCI_statlog_ID=german\n"
        "    UCI_statlog_ID=australian\n"
        "    UCI_statlog_ID=heart\n"
        "    UCI_statlog_ID=satimage\n"
        "    UCI_statlog_ID=segment\n"
        "    UCI_statlog_ID=vehicle\n"
        "    UCI_statlog_ID=shuttle\n"
        "    UCI_thyroid-disease_ID=allbp\n"
        "    UCI_thyroid-disease_ID=allhyper\n"
        "    UCI_thyroid-disease_ID=allhypo\n"
        "    UCI_thyroid-disease_ID=allrep\n"
        "    UCI_thyroid-disease_ID=ann\n"
        "    UCI_thyroid-disease_ID=dis\n"
        "    UCI_thyroid-disease_ID=sick\n"
        "    UCI_thyroid-disease_ID=hypothyroid\n"
        "    UCI_thyroid-disease_ID=new-thyroid\n"
        "    UCI_thyroid-disease_ID=sick-euthyroid\n"
        "    UCI_thyroid-disease_ID=thyroid0387\n"
        "    UCI_abalone\n"
        "    UCI_adult\n"
        "    UCI_covtype\n"
        "    UCI_internet_ads\n"
        "    UCI_nursery\n"
        "    UCI_pendigits\n"
        "    UCI_spambase\n"
        "    UCI_yeast\n" 
        "In order to access the UCI KDD datasets the dataset name must start with UCI_KDD_. The possible\n"
        "dataset names are:\n"
        "    UCI_KDD_corel_ID=ColorMoments\n"
        "    UCI_KDD_corel_ID=ColorHistogram\n"
        "    UCI_KDD_corel_ID=CoocTexture\n"
        "    UCI_KDD_corel_ID=LayoutHistogram\n"
        "    UCI_KDD_insurance-bench\n";
}

void loadClassificationDataset(const string& dbname, int& inputsize, int& nclasses, VMat& trainset, VMat& testset, bool normalizeinputs, VMat& allset);


} // end of namespace PLearn


#endif


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
