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
   * $Id: databases.h,v 1.5 2004/07/08 21:31:13 tihocan Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file Databases/databases.h */

#ifndef DATABASES_INC
#define DATABASES_INC

#include "UCISpecification.h"
#include "VMat.h"
//#include "NistDB.h"

namespace PLearn {
using namespace std;


//!  This will input a 2d binary classification problem (launches a java applet)
Mat input2dSet(const string& filename="data2d.amat");

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
void loadUCI(VMat& trainset, VMat& testset, VMat& allset, string db_spec, string id, bool normalize);
//! Load a specific UCI dataset in the given VMatrix.
void loadUCISet(VMat& data, string file, PP<UCISpecification> uci_spec, bool normalize);


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

void loadClassificationDataset(const string& dbname, int& inputsize, int& nclasses, VMat& trainset, VMat& testset, bool normalizeinputs, VMat& allset);


} // end of namespace PLearn


#endif







