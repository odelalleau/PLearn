// -*- C++ -*-

// AutoVMatrix.cc
// Copyright (C) 2002 Pascal Vincent
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
 * $Id: AutoVMatrix.cc,v 1.11 2004/08/09 22:16:24 mariusmuja Exp $
 * This file is part of the PLearn library.
 ******************************************************* */


#include "AutoVMatrix.h"
#include <plearn/db/getDataSet.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(AutoVMatrix,
    "Automatically builds an appropriate VMat given its specification.",
    "AutoVMatrix tries to interpret the given 'specification' (it will call getDataSet) and\n"
    "will be a wrapper around the appropriate VMatrix type, simply forwarding calls to it.\n"
    "AutoVMatrix can be used to access the UCI databases.\n");

AutoVMatrix::AutoVMatrix(const string& the_specification)
  :specification(the_specification)
{ build_(); }

void AutoVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "specification", &AutoVMatrix::specification, OptionBase::buildoption,
                "This is any string understood by getDataSet. Typically a file or directory path.\n"
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
                "    UCI_KDD_insurance-bench\n" );
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  // Hide the 'vm' option, that is overwritten at build time anyway.
  redeclareOption(ol, "vm", &AutoVMatrix::vm, OptionBase::nosave, "");
}

void AutoVMatrix::build_()
{
  if(specification=="")
    setVMat(VMat());
  else
    setVMat(getDataSet(specification));
}

void AutoVMatrix::build()
{
  inherited::build();
  build_();
}
 
/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AutoVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies) {
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} 

