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

#include "databases.h"
#include <plearn/vmat/ConcatRowsVMatrix.h>
#include <plearn/db/NistDB.h>
#include <plearn/math/random.h>
#include <plearn/vmat/RemapLastColumnVMatrix.h>
#include <plearn/vmat/ShiftAndRescaleVMatrix.h>
#include <plearn/vmat/Splitter.h>
#include <plearn/vmat/VMat_basic_stats.h>
#include <plearn/io/MatIO.h>
#include <plearn/base/stringutils.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;


#define JAVA "java"

Mat input2dSet(const PPath& filename)
{
    Mat data;
    if(!pathexists(filename))
    {
        string systemstring = string(JAVA) + " InputPoints " + filename + " -1 1 -1 1";
        system(systemstring.c_str());
    }
    loadAscii(filename, data);
    shuffleRows(data);
    return data;
}

// normalize training_set validation_set and test_set according to mean and stddev computed on training_set
void normalizeDataSets(Mat& training_set, Mat& validation_set, Mat& test_set)
{
    int inputsize = training_set.width()-1;
    Mat training_inputs = training_set.subMatColumns(0,inputsize);
    Vec meanvec(inputsize);
    Vec stddevvec(inputsize);
    computeMeanAndStddev(training_inputs, meanvec, stddevvec);
    training_inputs -= meanvec;
    training_inputs /= stddevvec;
    Mat validation_inputs = validation_set.subMatColumns(0,inputsize);
    validation_inputs -= meanvec;
    validation_inputs /= stddevvec;
    Mat test_inputs = test_set.subMatColumns(0,inputsize);
    test_inputs -= meanvec;
    test_inputs /= stddevvec;
}
// normalize training_set validation_set and test_set according to mean and stddev computed on training_set
void normalizeDataSets(VMat& training_set, VMat& validation_set, VMat& test_set)
{
    int inputsize = training_set.width()-1;
    Mat training_inputs = training_set.subMatColumns(0,inputsize);
    Vec meanvec(inputsize);
    Vec stddevvec(inputsize);
    computeMeanAndStddev(training_inputs, meanvec, stddevvec);
    training_inputs -= meanvec;
    training_inputs /= stddevvec;
    Mat validation_inputs = validation_set.subMatColumns(0,inputsize);
    validation_inputs -= meanvec;
    validation_inputs /= stddevvec;
    Mat test_inputs = test_set.subMatColumns(0,inputsize);
    test_inputs -= meanvec;
    test_inputs /= stddevvec;
}

// normalize both training_set and test_set according to mean and stddev computed on training_set
void normalizeDataSets(Mat& training_set, Mat& test_set)
{
    int inputsize = training_set.width()-1;
    Mat training_inputs = training_set.subMatColumns(0,inputsize);
    Vec meanvec(inputsize);
    Vec stddevvec(inputsize);
    computeMeanAndStddev(training_inputs, meanvec, stddevvec);
    training_inputs -= meanvec;
    training_inputs /= stddevvec;
    Mat test_inputs = test_set.subMatColumns(0,inputsize);
    test_inputs -= meanvec;
    test_inputs /= stddevvec;
}

void normalizeDataSet(Mat& m) // substract mean, and divide by stddev (these are estimated globally)
{
    Vec meanvec(m.width());
    Vec stddevvec(m.width());
    computeMeanAndStddev(m,meanvec,stddevvec);
    m -= meanvec;
    m /= stddevvec;
}
void splitTrainValidTest(VMat &data_set,VMat &train_set,VMat &valid_set,
			 real valid_fraction,VMat &test_set, real test_fraction, 
			 bool normalize)
{ 
    int nvalid = int((real)data_set.length()*valid_fraction);
    int ntest = int((real)data_set.length()*test_fraction);
    int ntrain = data_set.length()-(nvalid+ntest);

    train_set = data_set.subMatRows(0,ntrain);
    valid_set = data_set.subMatRows(ntrain, nvalid);
    test_set = data_set.subMatRows(ntrain+nvalid,ntest);
    if (normalize){
        VMat train_set_inputs=train_set.subMatColumns(0,data_set.width()-1);
        VMat valid_set_inputs=valid_set.subMatColumns(0,data_set.width()-1); 
        VMat test_set_inputs = test_set.subMatColumns(0,data_set.width()-1);
        normalizeDataSets(train_set_inputs,valid_set_inputs,test_set_inputs);
    }
}
VMat reduceInputSize(real fraction,VMat data)
{
    int n_inputs=data->width()-1;
    int reduce_n_inputs=(int)(fraction*n_inputs);
    cout<<"use "<<reduce_n_inputs<<" of "<<n_inputs<<endl;
    VMat new_data = data.subMatColumns(n_inputs-reduce_n_inputs,1+reduce_n_inputs);
    return new_data;
}
VMat reduceDataSetSize(real fraction,VMat data)
{
    int n_examples=data->length();
    int new_n_examples=(int)(fraction*n_examples);
    return data.subMatRows(0,new_n_examples);
}

// remaps classnums from {0,1} to {-1,+1}
void remapClassnums(VMat& data, real remap_minval_to, real remap_maxval_to)
{
    // Map classnums in last row from 0,1 to -1,1
    int inputsize = data.width()-1;
    for(int i=0; i<data.length(); i++)
    {
        if(data(i,inputsize)<=0.0)
            data->put(i,inputsize,remap_minval_to);
        else
            data->put(i,inputsize,remap_maxval_to);
    }
}

VMat loadBreastCancerWisconsin(bool normalize, bool uniq)
{
    Mat data;
    if(uniq)
        loadAscii("DBDIR:Breast/breast-cancer-wisconsin-uniq.amat",data);
    else
        loadAscii("DBDIR:Breast/breast-cancer-wisconsin.amat",data);
    if(normalize)
    {
        Mat datainput = data.subMatColumns(0,data.width()-1);
        normalizeDataSet(datainput);
    }
    shuffleRows(data);
    return VMat(data);
}

int loadBreastCancer(VMat& training_set, VMat& validation_set, VMat& test_set, int ntrain, int nvalid, bool uniq)
{
    Mat data;
    if(uniq)
        loadAscii("DBDIR:Breast/breast-cancer-wisconsin-uniq.amat",data);
    else
        loadAscii("DBDIR:Breast/breast-cancer-wisconsin.amat",data);
  
    shuffleRows(data);
  
    // split the data into training_set and test_set
    int ntest = data.length()-(ntrain+nvalid);
    Mat training_data = data.subMatRows(0,ntrain);
    Mat validation_data = data.subMatRows(ntrain, nvalid);
    Mat test_data = data.subMatRows(ntrain+nvalid,ntest);
  
    // normalize the inputs
    normalizeDataSets(training_data,validation_data,test_data);

    training_set = VMat(training_data);
    validation_set = VMat(validation_data);
    test_set = VMat(test_data);
    return 2; // 2 classes
}
  
VMat loadPimaIndians(bool normalize)
{
    Mat data = loadUCIMLDB("UCI_MLDB_REP:pima-indians-diabetes/pima-indians-diabetes.data");
    if(normalize)
    {
        Mat datainput = data.subMatColumns(0,data.width()-1);
        normalizeDataSet(datainput);
    }
    shuffleRows(data);
    return VMat(data);
}

VMat loadHousing(bool normalize)
{
    Mat data;
    loadGnuplot("UCI_MLDB_REP:housing/housing.data", data);
    Mat inputs = data.subMatColumns(0,13);
    Mat targets = data.subMatColumns(13,1);
    if (normalize)
    {
        // normalize the inputs
        normalizeDataSet(inputs);
        // put the targets in a nicer range by dividing by 100
        targets *= real(0.01);
    }
    return VMat(data);
}

VMat loadSonar()
{
    Mat data = loadUCIMLDB("UCI_MLDB_REP:undocumented/connectionist-bench/sonar/sonar.all-data");
    shuffleRows(data);
    // no need to normalize
    return VMat(data);
}

VMat loadIonosphere()
{
    Mat data = loadUCIMLDB("UCI_MLDB_REP:ionosphere/ionosphere.data");
    shuffleRows(data);
    // no need to normalize
    return VMat(data);
}

VMat loadDiabetes(bool normalize)
{
    Mat data;
    loadAscii("DBDIR:Diabetes/diabetes.amat",data);

    if(normalize)
    {
        Mat datainput = data.subMatColumns(0,data.width()-1);
        normalizeDataSet(datainput);
    }
    shuffleRows(data);
    return VMat(data);
}

int loadDiabetes(VMat& training_set, VMat& validation_set, VMat& test_set, int ntrain, int nvalid)
{
    Mat data;
    loadAscii("DBDIR:Diabetes/diabetes.amat",data);

    shuffleRows(data);

    // split the data into training_data and test_data
    int ntest = data.length()-(ntrain+nvalid);
    Mat training_data = data.subMatRows(0,ntrain);
    Mat validation_data = data.subMatRows(ntrain, nvalid);
    Mat test_data = data.subMatRows(ntrain+nvalid,ntest);

    // normalize the inputs
    normalizeDataSets(training_data,validation_data,test_data);      

    training_set = VMat(training_data);
    validation_set = VMat(validation_data);
    test_set = VMat(test_data);
    return 2; // 2 classes
}

int loadATT800(VMat& training_set, VMat& test_set)
{
    Mat data;
    loadAscii("DBDIR:ATT800/att800.amat",data);

    // preprocessing the data:
    Mat durations = data.subMatColumns(0,12);
    Mat daytimes = data.subMatColumns(12,24);
    Mat classnums = data.column(36);

    Mat newdata(data.length(), data.width()+2);
    Mat new_total_durations = newdata.column(0);
    Mat new_durations = newdata.subMatColumns(1,12);
    Mat new_total_daytimes = newdata.column(13);
    Mat new_daytimes = newdata.subMatColumns(14,24);
    Mat new_classnums = newdata.column(38);

    new_durations << durations;
    new_daytimes << daytimes;
    new_classnums << classnums;
    for(int i=0; i<data.length(); i++)
    {
        new_total_durations(i,0) = sum(new_durations(i), false);
        if(new_total_durations(i,0) > 0.0)
        {
            Vec new_durations_i = new_durations(i);
            new_durations_i /= new_total_durations(i,0);
        }
        new_total_daytimes(i,0) = sum(new_daytimes(i), false);
        if(new_total_daytimes(i,0) > 0.0)
        {
            Vec new_daytimes_i = new_daytimes(i);
            new_daytimes_i /= new_total_daytimes(i,0);          
        }
    }

    shuffleRows(newdata);      
    Mat training_data = newdata.subMatRows(0,400);
    Mat test_data = newdata.subMatRows(100,185);

    // normalize the new inputs...
    normalizeDataSets(training_data,test_data);      

    training_set = VMat(training_data);
    test_set = VMat(test_data);
    return 2; // 2 classes
}

VMat loadLetters(bool normalize)
{
    Mat letters;
    loadAscii("DBDIR:Letter/letter.amat",letters);

    if(normalize)
    {
        Mat datainput = letters.subMatColumns(0,letters.width()-1);
        normalizeDataSet(datainput);
    }

    return VMat(letters);
}


VMat loadLetters(const char* class0, const char* class1, bool normalize)
{
    int letter_classnum[26];
    for(int l=0; l<26; l++)
        letter_classnum[l] = -1;
    for(unsigned int i=0; i<strlen(class0); i++)
        letter_classnum[class0[i]-'A'] = 0;
    for(unsigned int i=0; i<strlen(class1); i++)
        letter_classnum[class1[i]-'A'] = 1;

    Mat letters;
    loadAscii("DBDIR:Letter/letter.amat",letters);

    int nkeptsamples = 0;
    for(int i=0; i<letters.length(); i++)
        if(letter_classnum[int(letters(i,letters.width()-1))] >= 0)
            nkeptsamples++;

    Mat keptletters(nkeptsamples, letters.width());
    int n = 0;
    for(int i=0; i<letters.length(); i++)
    {
        int classnum = letter_classnum[int(letters(i,letters.width()-1))];
        if(classnum >= 0)
        {
            keptletters(n) << letters(i);
            keptletters(n,keptletters.width()-1) = classnum;
            n++;
        }
    }

    if(normalize)
    {
        Mat datainput = keptletters.subMatColumns(0,keptletters.width()-1);
        normalizeDataSet(datainput);
    }

    return VMat(keptletters);
}

int loadLetters(VMat& training_set, VMat& validation_set, VMat& test_set, char* which_letters, real validation_fraction, real test_fraction, bool do_shuffle)
{
    int letter_classnum[26];
    for(int l=0; l<26; l++)
        letter_classnum[l] = -1;
    int classnum = 0;
    for(unsigned int i=0; i<strlen(which_letters); i++)
        letter_classnum[which_letters[i]-'A'] = classnum++;

    Mat letters;
    loadAscii("DBDIR:Letter/letter.amat",letters);

    Mat keptletters(letters.length(),letters.width());
    int k=0;
    for(int i=0; i<letters.length(); i++)
    {
        int c = letter_classnum[(int)letters(i,letters.width()-1)];
        if(c!=-1)
        {
            keptletters(k) << letters(i);
            keptletters(k,keptletters.width()-1) = c;
            k++;
        }
    }
    keptletters.resize(k,letters.width());

    letters = keptletters.copy();

    // free memory used by keptletters
    keptletters = Mat();
    if (do_shuffle){
        shuffleRows(letters);
    }
    int nvalid = int((real)letters.length()*validation_fraction);
    int ntest = int((real)letters.length()*test_fraction);
    int ntrain = letters.length()-(nvalid+ntest);

    Mat training_data = letters.subMatRows(0,ntrain);
    Mat validation_data = letters.subMatRows(ntrain, nvalid);
    Mat test_data = letters.subMatRows(ntrain+nvalid,ntest);

    // normalize the inputs
    normalizeDataSets(training_data,validation_data,test_data);

    training_set = VMat(training_data);
    validation_set = VMat(validation_data);
    test_set = VMat(test_data);
    return int(strlen(which_letters));
}

VMat loadLetters(int n_letters, bool do_shuffle)
{
    if (n_letters > 26 || n_letters < 1)
        PLERROR("In loadLetters: alphabet is at most 26 letters (and at least 1 letter)!");
    int letter_classnum[26];
    for(int l=0; l<26; l++)
        letter_classnum[l] = -1;
    int classnum = 0;
    int letter = 0;
    for(int i=0; i<n_letters; i++)
        letter_classnum[letter++] = classnum++;

    Mat letters;
    loadAscii("DBDIR:Letter/letter.amat",letters);

    Mat keptletters(letters.length(),letters.width());
    int k=0;
    for(int i=0; i<letters.length(); i++)
    {
        int c = letter_classnum[(int)letters(i,letters.width()-1)];
        if(c!=-1)
        {
            keptletters(k) << letters(i);
            keptletters(k,keptletters.width()-1) = c;
            k++;
        }
    }
    keptletters.resize(k,letters.width());

    letters = keptletters.copy();

    // free memory used by keptletters
    keptletters = Mat();
    if (do_shuffle){
        shuffleRows(letters);
    }
    return VMat(letters);
}

int loadLetters(VMat& training_set, VMat& validation_set, VMat& test_set, int n_letters, real validation_fraction, real test_fraction, bool do_shuffle)
{
    VMat letters=loadLetters(n_letters,do_shuffle);
    int nvalid = int((real)letters.length()*validation_fraction);
    int ntest = int((real)letters.length()*test_fraction);
    int ntrain = letters.length()-(nvalid+ntest);

    Mat training_data = letters.subMatRows(0,ntrain);
    Mat validation_data = letters.subMatRows(ntrain, nvalid);
    Mat test_data = letters.subMatRows(ntrain+nvalid,ntest);

    // normalize the inputs
    normalizeDataSets(training_data,validation_data,test_data);

    training_set = VMat(training_data);
    validation_set = VMat(validation_data);
    test_set = VMat(test_data);
    return n_letters; 
}

void loadCorelDatamat(int classnum, Mat& train, Mat& valid, Mat& test)
{
    int len;
    int width = 16*16*16*2;
    PPath filename;

    // Load train
    {
        filename = "DBDIR:Corel/train/size" + tostring(classnum);
        ifstream sizein(filename.c_str()); // TODO: use a PStream?
        sizein >> len;
        Mat datamat(len, width);

        filename = "DBDIR:Corel/train/histo" + tostring(classnum);
        ifstream datain(filename.c_str());
#ifdef USEFLOAT
        datain.read((char*)datamat.data(), len*width*4);
#ifdef LITTLEENDIAN
        reverse_float(datamat.data(), len*width);
#endif
#else
        PLERROR("In loadCorelDatamat USEDOUBLE case not yet implemented correctly");
#endif
        // Now copy only the useful features
        train.resize(len,width/2);
        for(int i=0; i<train.length(); i++)
            for(int j=0; j<train.width(); j++)
                train(i,j) = datamat(i,2*j);
    }

    // Load valid
    {
        filename = "DBDIR:Corel/valid/size" + tostring(classnum);
        ifstream sizein(filename.c_str());
        sizein >> len;
        Mat datamat(len, width);

        filename = "DBDIR:Corel/valid/histo" + tostring(classnum);
        ifstream datain(filename.c_str());
#ifdef USEFLOAT
        datain.read((char*)datamat.data(), len*width*4);
#ifdef BIGENDIAN
        reverse_float(datamat.data(), len*width);
#endif
#else
        PLERROR("In loadCorelDatamat USEDOUBLE case not yet implemented correctly");
#endif

        // Now copy only the useful features
        valid.resize(len,width/2);
        for(int i=0; i<valid.length(); i++)
            for(int j=0; j<valid.width(); j++)
                valid(i,j) = datamat(i,2*j);
    }

    // Load test
    {
        filename = "DBDIR:Corel/test/size" + tostring(classnum);
        ifstream sizein(filename.c_str());
        sizein >> len;
        Mat datamat(len, width);

        filename = "DBDIR:Corel/test/histo" + tostring(classnum);
        ifstream datain(filename.c_str());
#ifdef USEFLOAT
        datain.read((char*)datamat.data(), len*width*4);
#ifdef BIGENDIAN
        reverse_float(datamat.data(), len*width);
#endif
#else
        PLERROR("In loadCorelDatamat USEDOUBLE case not yet implemented correctly");
#endif

        // Now copy only the useful features
        test.resize(len,width/2);
        for(int i=0; i<test.length(); i++)
            for(int j=0; j<test.width(); j++)
                test(i,j) = datamat(i,2*j);
    }
} 

Mat smoothCorelHisto(Mat& data)
{
    Mat res(data.length(), 7*7*7);
    for(int n=0; n<data.length(); n++)
    {
        real* r = res[n];
        real* d = data[n];
        for(int i=0; i<7; i++)
            for(int j=0; j<7; j++)
                for(int k=0; k<7; k++,r++)
                {
                    *r += 0.15*d[i*2*16*16+j*2*16+k*2];
                    *r += 0.35*d[(i*2+1)*16*16+(j*2+1)*16+k*2+1];
                    *r += 0.35*d[(i*2+2)*16*16+(j*2+2)*16+k*2+2];
                    *r += 0.15*d[(i*2+3)*16*16+(j*2+3)*16+k*2+3];
                }
    }
    return res;
}

void loadCorel(Mat& training_set, Mat& validation_set, Mat& test_set, int negative_class, int positive_class)
{
    // A is the negative class (will have 0 classnums)
    // B is the positive class (will have 1 classnums)

    Mat trainA, validA, testA;
    Mat trainB, validB, testB;

    loadCorelDatamat(negative_class, trainA, validA, testA);
    trainA = smoothCorelHisto(trainA);
    validA = smoothCorelHisto(validA);
    testA = smoothCorelHisto(testA);
    loadCorelDatamat(positive_class, trainB, validB, testB);
    trainB = smoothCorelHisto(trainB);
    validB = smoothCorelHisto(validB);
    testB = smoothCorelHisto(testB);
    int inputsize = trainA.width();

    training_set.resize(trainA.length()+trainB.length(), inputsize+1);  
    Mat trainingAinputs = training_set.subMat(0, 0, trainA.length(), inputsize);
    Mat trainingAclassnums = training_set.subMat(0, inputsize, trainA.length(), 1);
    Mat trainingBinputs = training_set.subMat(trainA.length(), 0, trainB.length(), inputsize);
    Mat trainingBclassnums = training_set.subMat(trainA.length(), inputsize, trainB.length(), 1);
    trainingAinputs << trainA;
    trainingAclassnums.fill(0.0);
    trainingBinputs << trainB;
    trainingBclassnums.fill(1.0);
    shuffleRows(training_set);
  
    validation_set.resize(validA.length()+validB.length(), inputsize+1);  
    Mat validAinputs = validation_set.subMat(0, 0, validA.length(), inputsize);
    Mat validAclassnums = validation_set.subMat(0, inputsize, validA.length(), 1);
    Mat validBinputs = validation_set.subMat(validA.length(), 0, validB.length(), inputsize);
    Mat validBclassnums = validation_set.subMat(validA.length(), inputsize, validB.length(), 1);
    validAinputs << validA;
    validAclassnums.fill(0.0);
    validBinputs << validB;
    validBclassnums.fill(1.0);
    shuffleRows(validation_set);

    test_set.resize(testA.length()+testB.length(), inputsize+1);  
    Mat testAinputs = test_set.subMat(0, 0, testA.length(), inputsize);
    Mat testAclassnums = test_set.subMat(0, inputsize, testA.length(), 1);
    Mat testBinputs = test_set.subMat(testA.length(), 0, testB.length(), inputsize);
    Mat testBclassnums = test_set.subMat(testA.length(), inputsize, testB.length(), 1);
    testAinputs << testA;
    testAclassnums.fill(0.0);
    testBinputs << testB;
    testBclassnums.fill(1.0);
    shuffleRows(test_set);
}

void loadCallxx(int year, VMat& d)
{
    Mat data;
    PPath filename = "DBDIR:Finance/call" + tostring(year) + ".stc.data";
    loadAscii(filename, data);
    d = VMat(data);
}


void loadUSPS(VMat& trainset, VMat& testset, bool use_smooth)
{
    Mat traininputs;
    Mat testinputs;
    Mat traindesired;
    Mat testdesired;

    if(use_smooth)
    {
        traininputs = loadSNMat("DBDIR:usps/train-patterns-smoo.mat");
        testinputs = loadSNMat("DBDIR:usps/test-patterns-smoo.mat");
    }
    else
    {
        traininputs = loadSNMat("DBDIR:usps/ocr16-train.mat");
        testinputs = loadSNMat("DBDIR:usps/ocr16-test.mat");
    }
    //traininputs += 1.0;
    //traininputs /= 2.0;
    //testinputs += 1.0;
    //testinputs /= 2.0;

    traindesired = loadSNMat("DBDIR:usps/train-desired.mat");
    Mat trainclasses(traininputs.length(),1);
    for(int i=0; i<traindesired.length(); i++)
        trainclasses(i,0) = argmax(traindesired(i));

    testdesired = loadSNMat("DBDIR:usps/test-desired.mat");
    Mat testclasses(testinputs.length(),1);
    for(int i=0; i<testdesired.length(); i++)
        testclasses(i,0) = argmax(testdesired(i));

    trainset = hconcat(traininputs,trainclasses);
    testset = hconcat(testinputs,testclasses);
}

VMat loadUSPS(bool use_smooth)
{
    Mat traininputs;
    Mat traindesired;

    if(use_smooth)
        traininputs = loadSNMat("DBDIR:usps/patterns-smoo.mat");
    else
        traininputs = loadSNMat("DBDIR:usps/ocr16.pat");
        
    traininputs += real(1.0);
    traininputs /= real(2.0);

    traindesired = loadSNMat("DBDIR:usps/desired.mat");
    Mat trainclasses(traininputs.length(),1);
    for(int i=0; i<traindesired.length(); i++)
        trainclasses(i,0) = argmax(traindesired(i));

    Mat trainset = hconcat(traininputs,trainclasses);

    return trainset;
}

void loadLetters(int& inputsize, int& nclasses, VMat& trainset, VMat& testset)
{
    Mat letters;
    loadAscii("DBDIR:Letter/letter.amat",letters);
    inputsize = letters.width()-1;
    nclasses = 26;
    trainset = VMat(letters.subMatRows(0,16000));
    testset = VMat(letters.subMatRows(16000,4000));
}

void loadClassificationDataset(const string& datasetname, int& inputsize, int& nclasses, VMat& trainset, VMat& testset, bool normalizeinputs, VMat& allset)
{
    string dbname = datasetname;
    int reduced_size = 0;
    vector<string> dataset_and_size = split(dbname,":");
    if(dataset_and_size.size()==2)
    {
        dbname = dataset_and_size[0];
        reduced_size = toint(dataset_and_size[1]);
    }

    if(dbname=="2d")
    {
        trainset = input2dSet();
        Mat mapping(2,2);
        mapping << string("-1 0 1 1");
        trainset = remapLastColumn(trainset,mapping);
        testset = trainset;
        inputsize = 2;
        nclasses = 2;
    }
    else if(dbname=="letters")
    {
        loadLetters(inputsize, nclasses, trainset, testset);
    }
    else if(dbname=="breast")
    {
        VMat dbname_vm = loadBreastCancerWisconsin();
        inputsize = dbname_vm.width()-1;
        nclasses = 2;
        split(dbname_vm,0.5,trainset,testset);
    }
    else if(dbname=="usps")
    {
        loadUSPS(trainset,testset,true);
        inputsize = trainset.width()-1;
        nclasses = 10;
    }
    else if(dbname=="mnist")
    {
        loadMNIST(trainset,testset);
        inputsize = trainset.width()-1;
        nclasses = 10;
    }
    else if(dbname=="mnist_override")
    {
        loadMNIST(trainset,testset);
        inputsize = trainset.width()-1;
        nclasses = 10;
        Mat m;
        loadPMat("mnist_override.pmat",m);
        if(m.width() != inputsize+1)
            PLERROR("mnist_overrid.pmat is espected to have a width of %d, but has %d",inputsize+1,m.width());
        trainset = VMat(m);
    }
    else if(dbname.length()==5 && dbname.substr(0,4)=="usps" && dbname[4]>='0' && dbname[4]<='9')
    {
        int classnum = dbname[4]-'0';
        loadUSPS(trainset,testset,true);
        inputsize = trainset.width()-1;
        trainset = remapLastColumn(trainset,classnum,1,0);
        testset = remapLastColumn(testset,classnum,1,0);
        nclasses = 2;
    }
    else if(dbname.length()==6 && dbname.substr(0,5)=="mnist" && dbname[5]>='0' && dbname[5]<='9')
    {
        int classnum = dbname[5]-'0';
        loadMNIST(trainset,testset);
        inputsize = trainset.width()-1;
        trainset = remapLastColumn(trainset,classnum,1.,0.);
        testset = remapLastColumn(testset,classnum,1.,0.);
        nclasses = 2;
    }
    else if (dbname.substr(0,4) == "UCI_") {
        string db_spec;
        string type;
        if (dbname.substr(0,8) == "UCI_KDD_") {
            db_spec = dbname.substr(8);
            type = "KDD";
        } else {
            db_spec = dbname.substr(4);
            type = "MLDB";
        }
    
        size_t look_for_id = db_spec.rfind("_ID=");
        string db_dir;
        string id = "";
        if (look_for_id != string::npos) {
            // There is an ID specified.
            db_dir = db_spec.substr(0, look_for_id);
            id = db_spec.substr(look_for_id + 4);
        } else {
            db_dir = db_spec;
        }
        loadUCI(trainset, testset, allset, db_dir, id, normalizeinputs,type);
        inputsize = allset->inputsize();
    
    }
    else
        PLERROR("Unknown dbname %s",dbname.c_str());

    if(reduced_size)
    {
        trainset = trainset.subMatRows(0,reduced_size);
        testset = testset.subMatRows(0,reduced_size);
    }

    if(normalizeinputs)
    {
        Vec meanvec;
        Vec stddevvec;
        computeMeanAndStddev(trainset, meanvec, stddevvec);
        meanvec = meanvec.subVec(0,inputsize);
        stddevvec = stddevvec.subVec(0,inputsize);
        for (int i = 0; i < stddevvec.length(); i++) {
            if (fast_exact_is_equal(stddevvec[i], 0)) {
                // The standard dev is 0, the row must be constant. Since we don't
                // want nans we put 1 instead.
                stddevvec[i] = 1;
            }
        }
        for (int i=0;i<inputsize;i++)
            if (fast_exact_is_equal(stddevvec[i], 0)) stddevvec[i]=1;
        trainset = normalize(trainset,meanvec,stddevvec);
        testset = normalize(testset,meanvec,stddevvec);
    }
}


/////////////
// loadUCI //
/////////////
void loadUCI(VMat& trainset, VMat& testset, VMat& allset, string db_spec, string id, bool &normalize, const string& type) {
    string script_file = db_spec;
    if (id != "") {
        script_file += "_ID=" + id;
    }
    script_file += ".plearn";
    PPath db_dir;
    if (type=="MLDB") {
        db_dir = PPath("UCI_MLDB_REP:") / db_spec;
    } else if (type=="KDD") { // TODO: a PPath protocol for UCI_KDD?
        db_dir = PPath("DBDIR:UCI_KDD") / db_spec;
    } else {
        PLERROR("In loadUCI: Unknown dataset type: %s.",type.c_str());
    }
    Object* obj = PLearn::macroLoadObject(db_dir / script_file);
    PP<UCISpecification> uci_spec = static_cast<UCISpecification*>(obj);
    if (uci_spec->file_train != "") {
        if (uci_spec->format=="UCI") {
            loadUCISet(trainset, db_dir / uci_spec->file_train, uci_spec);
        } else if (uci_spec->format=="AMAT") {
            loadUCIAMat(trainset, db_dir / uci_spec->file_train, uci_spec);
        } else {
            PLERROR("In loadUCI: Format '%s' unsupported",uci_spec->format.c_str());
        }
    }
    if (uci_spec->file_test != "") {
        if (uci_spec->format=="UCI") {
            loadUCISet(testset, db_dir / uci_spec->file_test, uci_spec);
        } else if (uci_spec->format=="AMAT") {
            loadUCIAMat(testset, db_dir / uci_spec->file_test, uci_spec);
        } else {
            PLERROR("In loadUCI: Format '%s' unsupported",uci_spec->format.c_str());
        }
    }
    if (uci_spec->file_all != "") {
        if (uci_spec->format=="UCI") {
            loadUCISet(allset, db_dir / uci_spec->file_all, uci_spec);
        } else if (uci_spec->format=="AMAT") {
            loadUCIAMat(allset, db_dir / uci_spec->file_all, uci_spec);
        } else {
            PLERROR("In loadUCI: Format '%s' unsupported",uci_spec->format.c_str());
        }
    } else {
        allset = vconcat(trainset, testset);
    }
    if (normalize) {
        int is = uci_spec->inputsize;
        if (is == -1)
            is = allset->width() - 1;
        VMat tmp_vmat = new ShiftAndRescaleVMatrix(allset, is, 0, true, 0);
        Mat new_data = tmp_vmat->toMat().subMatColumns(0, is);
        allset->putMat(0, 0, new_data);
        if (trainset && testset) {
            if (allset->length() != trainset->length() + testset->length())
                PLERROR("In loadUCI - The whole dataset should have a length equal to train + test");
            trainset->putMat(0, 0, new_data.subMatRows(0, trainset->length()));
            testset->putMat(0, 0, new_data.subMatRows(trainset->length(), testset->length()));
        } else if (trainset || testset) {
            PLERROR("In loadUCI - There can't be only a train set or only a test set");
        }
        // We don't want to normalize again.
        normalize = false;
    }
}



/////////////////
// loadUCIAMat //
/////////////////
void loadUCIAMat(VMat& data, string file, PP<UCISpecification> uci_spec) 
{
    data = loadAsciiAsVMat(file); 
  
    if (uci_spec->target_is_first) {
        // We need to move the target to the last columns.
        int ts = uci_spec->targetsize;
        if (ts == -1) {
            PLERROR("In loadUCIAMat - We don't know how many columns to move");
        }
        if (uci_spec->weightsize > 0) {
            PLERROR("In loadUCIAMat - Damnit, I don't like weights");
        }
        Vec row;
        Vec target;

        target.resize(ts);
        for (int i = 0; i < data.length(); i++) {
            row = data(i);
            target << row.subVec(0,ts);
            row.subVec(0, data.width() - ts ) << row.subVec(ts, data.width() - ts);
            row.subVec(data.width() - ts , ts) << target;
            data->putRow(i,row);
        }

        // now, move the symbols
        TVec<map<string,real> > sym;
        int is = data.width()-ts;
        sym.resize(ts);
        for (int i=0;i<ts;i++) {
            sym[i] = data->getStringToRealMapping(i);
        }
        for(int i=0;i<is; i++) {
            data->setStringMapping(i, data->getStringToRealMapping(i+ts));
        }
        for(int i=is;i<is+ts;i++) {
            data->setStringMapping(i,sym[i-is]);
        }
    }

    data->defineSizes(uci_spec->inputsize, uci_spec->targetsize, uci_spec->weightsize);
}

////////////////
// loadUCISet //
////////////////
void loadUCISet(VMat& data, PP<UCISpecification> uci_spec) {
    PLASSERT( uci_spec );
    if (!uci_spec->data_all.isEmpty())
        loadUCISet(data, uci_spec->data_all.absolute(), uci_spec);
    else {
        VMat data_train, data_test;
        loadUCISet(data_train, uci_spec->data_train.absolute(), uci_spec);
        loadUCISet(data_test,  uci_spec->data_test.absolute(),  uci_spec);
        data = new ConcatRowsVMatrix(data_train, data_test, true);
    }
}

void loadUCISet(VMat& data, string file, PP<UCISpecification> uci_spec) {
    char *** to_symbols;
    int * to_n_symbols;
    TVec<int> max_in_col;
    TVec<string> header_columns;
    Mat the_data;
    if (uci_spec->header_exists) {
        the_data = loadUCIMLDB(file, &to_symbols, &to_n_symbols, &max_in_col,&header_columns);
    } else {
        the_data = loadUCIMLDB(file, &to_symbols, &to_n_symbols, &max_in_col);
    }
    if (uci_spec->target_is_first) {
        // We need to move the target to the last columns.
        int ts = uci_spec->targetsize;
        if (ts == -1) {
            PLERROR("In loadUCISet - We don't know how many columns to move");
        }
        if (uci_spec->weightsize > 0) {
            PLERROR("In loadUCISet - Damnit, I don't like weights");
        }
        Vec row;
        Vec target;

        target.resize(ts);
        for (int i = 0; i < the_data.length(); i++) {
            row = the_data(i);
            target << row.subVec(0,ts);
            row.subVec(0, the_data.width() - ts ) << row.subVec(ts, the_data.width() - ts);
            row.subVec(the_data.width() - ts , ts) << target;
        }
    }
    data = VMat(the_data);
    data->defineSizes(uci_spec->inputsize, uci_spec->targetsize, uci_spec->weightsize);
 
    if (uci_spec->header_exists) {
        if (uci_spec->header_fields.size()==0) {
      
            if (uci_spec->target_is_first) {
                int ts = uci_spec->targetsize;
                int is = the_data.width()-ts;
                TVec<string> tmp;
                tmp.resize(ts);
                tmp << header_columns.subVec(0,ts);
                header_columns.subVec(0,is) << header_columns.subVec(ts,is);
                header_columns.subVec(is,ts) << tmp;
            }
            data->declareFieldNames(header_columns);
        } else {
            TVec<string> field_names;
            field_names.resize(the_data.width());
            int last = 0;
            int cnt=0;
            for (int i=0; i<uci_spec->header_fields.size(); i++) {
                for (int j=last;j<uci_spec->header_fields[i].first;j++) {
                    field_names[j] = "";
                }
                for (int j=uci_spec->header_fields[i].first;j<=uci_spec->header_fields[i].second;j++) {
                    if (cnt>=header_columns.size()) {
                        PLERROR("In loadUCISet: 'header_fields' setting is incorrect");
                    }
                    field_names[j] = header_columns[cnt++];
                }
                last = uci_spec->header_fields[i].second+1;
            }  
            for (int i=last;i<field_names.size();i++) {
                field_names[i] = "";
            }
            if (uci_spec->target_is_first) {
                int ts = uci_spec->targetsize;
                int is = the_data.width()-ts;
                TVec<string> tmp;
                tmp.resize(ts);
                tmp << field_names.subVec(0,ts);
                field_names.subVec(0,is) << field_names.subVec(ts,is);
                field_names.subVec(is,ts) << tmp;
            }
            data->declareFieldNames(field_names);
        }
    }
  
    // Add symbol mappings
  
    if (uci_spec->target_is_first) {
        int ts = uci_spec->targetsize;
        int is = the_data.width()-ts;
        TVec<char**> tmp_sym(ts);
        TVec<int> tmp_len(ts); 
        for(int i=0;i<ts;i++) {
            tmp_sym[i] = to_symbols[i];
            tmp_len[i] = to_n_symbols[i];
        }
        for (int i=ts;i<is+ts;i++) {
            to_symbols[i-ts] = to_symbols[i];
            to_n_symbols[i-ts] = to_n_symbols[i];
        }
        for(int i=is;i<is+ts;i++) {
            to_symbols[i] = tmp_sym[i-is];
            to_n_symbols[i] = tmp_len[i-is];
        }
    
        tmp_len << max_in_col.subVec(0,ts);
        max_in_col.subVec(0,is) << max_in_col.subVec(ts,is);
        max_in_col.subVec(is,ts) << tmp_len;
    }
    for (int j=0;j<data->width();j++) {
        for (int k=0;k<to_n_symbols[j];k++) {
            data->addStringMapping(j,string(to_symbols[j][k]),real(max_in_col[j]+k+1));
        }
    }

    // Free up the symbols
    for (int i=0; i<data->width(); i++) 
    {
        for (int j=0; j<to_n_symbols[i]; j++)
            free(to_symbols[i][j]);
        free(to_symbols[i]);
    }
    free(to_symbols);
    free(to_n_symbols);

    // Add default 'target' name to the target(s) column(s) if there is no fieldname yet.
    int is = data->inputsize();
    int ts = data->targetsize();
    if (ts == 1) {
        string f_target = data->fieldName(is);
        if (pl_isnumber(f_target) && toint(f_target) == is)
            data->declareField(is, "target");
    } else {
        string f_target_i;
        for (int i = 0; i < ts; i++) {
            f_target_i = data->fieldName(is + i);
            if (pl_isnumber(f_target_i) && toint(f_target_i) == is + i)
                data->declareField(is + i, "target_" + tostring(i));
        }
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
