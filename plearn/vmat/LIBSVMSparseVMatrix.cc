// -*- C++ -*-

// LIBSVMSparseVMatrix.cc
//
// Copyright (C) 2008 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file LIBSVMSparseVMatrix.cc */


#include "LIBSVMSparseVMatrix.h"
#include "plearn/io/openFile.h"
#include "plearn/io/fileutils.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    LIBSVMSparseVMatrix,
    "VMatrix containing data from a libsvm format file",
    "The libsvm format file is particularly useful for sparse inputs.\n"
    "The sparsity information (i.e. indices of non-zero inputs) is provided\n"
    "via the extra fields, and the input given by getExample() only contains\n"
    "the values for non-zero inputs. Since the sparsity can change from one example to\n"
    "another, getExtra() (and getExample()) will adapt the size of the extra (input)\n"
    "fields vector accordingly. However, since it only works for fixed length inputs,\n"
    "getRow() returns an error when called in sparse mode.\n"
    "This class can also be used in \"coarse\" mode (i.e. use_coarse_representation=true),\n"
    "where getRow() and getExample() will then provide inputs in a fixed vector form, with\n"
    "non-zero fields taking their given values and other fields set explicitely to 0.\n"
    "However, in coarse mode, no extra information is given (i.e. extrasize() is 0), since\n"
    "it is implicitly incorporated in the input vector."
    );

LIBSVMSparseVMatrix::LIBSVMSparseVMatrix(): use_coarse_representation(false)
{
}

LIBSVMSparseVMatrix::LIBSVMSparseVMatrix(PPath filename, bool use_coarse_representation):
    libsvm_file(filename),
    use_coarse_representation(use_coarse_representation)
{
    build();
}

void LIBSVMSparseVMatrix::getNewRow(int i, const Vec& v) const
{
    if( !use_coarse_representation )
        PLERROR("In LIBSVMSparseVMatrix::getNewRow(): not compatible with sparse representations. Use use_coarse_representation=true.");
    real* in = libsvm_input_data[i].data();
    real* ex = libsvm_extra_data[i].data();
    v.clear();
    for( int j=0; j<libsvm_input_data[i].length(); j++ )
        v[(int)round(ex[j])] = in[j];

    v[inputsize_] = libsvm_target_data[i];
}

void LIBSVMSparseVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "class_strings", &LIBSVMSparseVMatrix::class_strings,
                  OptionBase::buildoption,
                  "Strings associated to the different classes. If not present we suppose classes are int.\n");

    declareOption(ol, "libsvm_file", &LIBSVMSparseVMatrix::libsvm_file,
                  OptionBase::buildoption,
                  "File name of libsvm data.\n");

    declareOption(ol, "use_coarse_representation", &LIBSVMSparseVMatrix::use_coarse_representation,
                  OptionBase::buildoption,
                  "Indication that a coarse (i.e. fixed length, filled with 0's) representation\n"
                  "of the data in the .libsvm file should be used.\n");

    //declareOption(ol, "libsvm_input_data", 
    //              &LIBSVMSparseVMatrix::libsvm_input_data,
    //              OptionBase::learntoption,
    //              "Input data.\n");
    //
    //declareOption(ol, "libsvm_extra_data", 
    //              &LIBSVMSparseVMatrix::libsvm_extra_data,
    //              OptionBase::learntoption,
    //              "Extra data.\n");
    //
    //declareOption(ol, "libsvm_target_data", 
    //              &LIBSVMSparseVMatrix::libsvm_target_data,
    //              OptionBase::learntoption,
    //              "Target data.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LIBSVMSparseVMatrix::build_()
{

    if(libsvm_file.isEmpty())
        return;

    // Read data
    PStream libsvm_stream = openFile(libsvm_file, PStream::raw_ascii);
    updateMtime(libsvm_file);
    libsvm_stream.skipBlanks();
    int input_index = 0;
    int largest_input_index = -1;
    int target_index = 0;
    int n_inputs = 0;
    string line;
    vector<string> tokens; 
    length_ = 0;
    width_ = -1;
    libsvm_input_data.resize(0);
    libsvm_extra_data.resize(0);
    libsvm_target_data.resize(0);
    while(!libsvm_stream.eof())
    {
        libsvm_stream.getline(line);
        line = removeblanks(line);
        libsvm_stream.skipBlanks();
        tokens = split(line,": ");
        
        // Get target
        target_index = class_strings.find(tokens[0]);
        if( target_index < 0){
            double d;
            if(pl_isnumber(tokens[0],&d) && ((double)((int)d))==d)
                target_index=(int)d;
            else
                PLERROR("In LIBSVMSparseVMatrix::build_(): target %s unknown and not an int",
                        tokens[0].c_str());
        }
        if( (tokens.size()-1)%2 != 0 )
            PLERROR("In LIBSVMSparseVMatrix::build_(): line %s has incompatible "
                    "format", line.c_str()); 
        libsvm_target_data.push_back(target_index);

        n_inputs = (tokens.size()-1)/2;
        Vec input_vec(n_inputs);
        Vec extra_vec(n_inputs);
        // Get inputs
        for( int i=0; i<n_inputs; i++)
        {
            input_index = toint(tokens[2*i+1])-1;
            extra_vec[i] = input_index;
            if( input_index > largest_input_index )
                largest_input_index = input_index;
            input_vec[i] = toreal(tokens[2*i+2]);
        }
        libsvm_input_data.push_back(input_vec);
        libsvm_extra_data.push_back(extra_vec);
        length_++;
    }

    // Set sizes
    if( inputsize_ < 0 ) inputsize_ = largest_input_index+1;
    if( targetsize_ < 0 ) targetsize_ = 1;
    if( weightsize_ < 0 ) weightsize_ = 0;
    if( use_coarse_representation )
        extrasize_ = 0;
    else
        extrasize_ = largest_input_index+1;
    if( width_ < 0 ) width_ = inputsize_ + targetsize_ + weightsize_;
}
 

// ### Nothing to add here, simply calls build_
void LIBSVMSparseVMatrix::build()
{
    inherited::build();
    build_();
}

void LIBSVMSparseVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(class_strings, copies);
    deepCopyField(libsvm_file, copies);
    deepCopyField(libsvm_input_data, copies);
    deepCopyField(libsvm_extra_data, copies);
    deepCopyField(libsvm_target_data, copies);
}

void LIBSVMSparseVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
    if( use_coarse_representation )
        inherited::getExample(i,input,target,weight);
    else
    {
        if( i>= length_ || i < 0 )
            PLERROR("In LIBSVMSparseVMatrix::getExample(): row index should "
                    "be between 0 and length_-1");
        input.resize(libsvm_input_data[i].length());
        input << libsvm_input_data[i];
        target.resize(targetsize_);
        target[0] = libsvm_target_data[i];
        weight = 1;
    }
}

void LIBSVMSparseVMatrix::getExamples(int i_start, int length, Mat& inputs, Mat& targets,
                          Vec& weights, Mat* extras, bool allow_circular)
{
    PLERROR("In LIBSVMSparseVMatrix::getExamples(): not compatible with "
            "sparse inputs");    
}

void LIBSVMSparseVMatrix::getExtra(int i, Vec& extra)
{
    if( use_coarse_representation )
        extra.resize(0);
    else
    {
        if( i>= length_ || i < 0 )
            PLERROR("In LIBSVMSparseVMatrix::getExample(): row index should "
                    "be between 0 and length_-1");
        extra.resize(libsvm_extra_data[i].length());
        extra << libsvm_extra_data[i];
    }
}

VMatrixExtensionRegistrar* LIBSVMSparseVMatrix::extension_registrar =
    new VMatrixExtensionRegistrar(
        "libsvm",
        &LIBSVMSparseVMatrix::instantiateFromPPath,
        "libsvm format(good for sparce input).");

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
