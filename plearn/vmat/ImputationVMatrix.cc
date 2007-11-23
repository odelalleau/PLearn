// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
// Copyright (C) 2003 Olivier Delalleau
// Copyright (C) 2007 Frederic Bastien
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


/* *******************************************************************    
   * $Id: NeighborhoodImputationVMatrix.cc 3658 2005-07-06 20:30:15  Godbout $
   ******************************************************************* */


#include "ImputationVMatrix.h"

namespace PLearn {
using namespace std;

/** ImputationVMatrix **/

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
  ImputationVMatrix,
  "Super-class for VMatrices that replace missing value in another one",
  ""
  );

  ImputationVMatrix::ImputationVMatrix():
    test_level(0)
{
}

ImputationVMatrix::~ImputationVMatrix()
{
}

void ImputationVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "source", &ImputationVMatrix::source, OptionBase::buildoption, 
                "The source VMatrix with missing values that will be filled.\n");
  declareOption(ol, "test_level", &ImputationVMatrix::test_level, OptionBase::buildoption, 
                "The level of test of final matrix. 0 : no test, 1: linear in column or row test, 2: linear in cell\n");

  inherited::declareOptions(ol);
}

void ImputationVMatrix::build()
{
  inherited::build();
  build_();
}

void ImputationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  deepCopyField(source, copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
}

void ImputationVMatrix::build_()
{
}

void ImputationVMatrix::testResultantVMatrix()
{
  TVec<string> source_names(source->width());
  source_names = source->fieldNames();
  
  if(test_level>=1){
    for(int row=0;row<length();row++)
      for(int col=0;col<width();col++){
        real data=get(row,col);
        real sourcedata=source->get(row,col);

        //test if variable not missing are not changed.
        if(!is_missing(sourcedata))
          if(data!=sourcedata){
            PLERROR("ImputationImputations::testResultantVMatrix() data at [%d,%d] are different but the data is not missing",row,col);
          }

        //test if missing variable are replaced by a value not missing.
        if(is_missing(data))
          PLERROR("ImputationImputations::testResultantVMatrix() data at [%d,%d] in the final matrix is missing",row,col);
      }
  }
  if(test_level>=2){ //Must verify if source->getStats is linear
    getStats();
    source->getStats();
    //print the variable that the replacement of missing value change the mean by more then 3 times the stderr
    int nberr = 0;
    for(int col=0;col<width();col++)
      {
        real mean = field_stats[col].mean();
        real smean = source->getStats(col).mean();
        real sstderr = source->getStats(col).stddev()/sqrt(source->getStats(col).nnonmissing());
        real val=(mean-smean)/sstderr;
        if(fabs(val)>3){
          PLWARNING("ImputationImputations::testResultantVMatrix() the variable %d(%s) have a value of %f for abs((mean-sourcemean)/source_stderr)",col,source_names[col].c_str(),val);
          nberr++;
        }
      }
    if(nberr>0)
      PLWARNING("ImputationImputations::testResultantVMatrix() There have been %d variables with the mean after imputation outside of sourcemean +- 3*source_stderr",nberr);
  }
  //    for(int row=0;row<length();row++)
  /*    for(int col=0;col<width();col++)
        {
        StatsCollector sstats=source->getStats(col);
        StatsCollector stats=getStats(col);
        if(sstats.nmissing()!=0){
        real sum=0;
        condmean_variable_file_name = source_metadata + "/" + condmean_dir + "/dir/" + source_names[col] + "/Split0/test1_outputs.pmat";
        
        if (!isfile(condmean_variable_file_name))
        PLERROR("In ImputationVMatrix::A conditional mean file was not found for variable %s", source_names[col].c_str());
        condmean_variable_file = new FileVMatrix(condmean_variable_file_name, false);
        if (condmean_variable_file->length() != source_length)
        PLERROR("In ImputationVMatrix::Source and conditional mean file length are not equal for variable %s", source_names[col].c_str());
        for (int source_row = 0; source_row < source_length; source_row++){
        real rdata = condmean_variable_file->get(source_row, 0);
        real data = get(source_row, col);
        if(!is_missing(rdata))
        sum+=pow(data - rdata, 2.0);
        }
        real mse=sum/stats.nnonmissing();
        real diff=mse/sstats.variance();
        if(diff >0.9){
        perr <<col<<" "<<diff<<endl;
        PLWARNING("ImputationImputations::testresultantVMatrix() the variable %d(%s) have a MSEtreecondmean(%f)/MSEmean(%f) of %f",col,source_names[col].c_str(),mse,sstats.variance(),diff);
        }
        }*/
}
} // end of namespcae PLearn
