// -*- C++ -*-
// SDBVMat.cc: Implementation of VMat/SDB Interface
//
// Copyright (C) 2000 Nicolas Chapados
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

#include "TMat_maths.h"			     // for binary_search
#include "stringutils.h"
#include "SDBVMat.h"

// norman: profiler deactivated for win32
#ifndef WIN32
#include "Profiler.h"
#endif

namespace PLearn {
using namespace std;


//#####  SDBVMOutputCoder  ####################################################

SDBVMOutputCoder::SDBVMOutputCoder(SDBVMOutputCoding oc,
				   real missing_values_mapping)
  : output_coding_(oc), num_classes_(0),
    missing_values_mapping_(missing_values_mapping)
{}

SDBVMOutputCoder::~SDBVMOutputCoder()
{}

SDBVMOutputCoding SDBVMOutputCoder::getOutputCoding() const
{
  return output_coding_;
}

void SDBVMOutputCoder::setNumClasses(int n_classes)
{
  num_classes_ = n_classes;
}

int SDBVMOutputCoder::getNumClasses() const
{
  return num_classes_;
}

void SDBVMOutputCoder::setMissingValuesMapping(real missing_values_mapping)
{
  missing_values_mapping_ = missing_values_mapping;
}

real SDBVMOutputCoder::getMissingValuesMapping() const
{
  return missing_values_mapping_;
}

void SDBVMOutputCoder::setOutput(real output_value, const Vec& output_field) const
{
  if (is_missing(output_value))
    output_value = missing_values_mapping_;
  int output_int = int(output_value);
    
  switch (output_coding_) {
  case SDBVMNumeric:
    output_field[0] = output_value;
    break;

  case SDBVMOneHot:
    // Additional coding: if we get a MISSING_VALUE at this point, fill
    // in the output_field with MISSING_VALUEs.
    if (is_missing(output_value))
      output_field.fill(MISSING_VALUE);
    else {
      if (output_value < 0 || output_value >= num_classes_)
	PLERROR("In SDBVMOutputCoder::setOutput: "
	      "Output value not in the range [0,%d]", num_classes_-1);

      output_field.fill(0.0);
      output_field[output_int] = 1.0;
    }
    break;

  case SDBVMOneHotMinus1:
    // Additional coding: if we get a MISSING_VALUE at this point, fill
    // in the output_field with MISSING_VALUEs.
    if (is_missing(output_value))
      output_field.fill(MISSING_VALUE);
    else {
      
      /*NOTE: this used to be a BUG!!! the following line was
	moved up from within 'if (output_int > 0)'.
	when output_int was ==0, output_field would stay as is,
	but should have been filled w/ zeros.
      */
      output_field.fill(0.0);
      if (output_value < 0 || output_value >= num_classes_)
	PLERROR("In SDBVMOutputCoder::setOutput: "
	      "Output value not in the range [0,%d]", num_classes_-1);
      if (output_int > 0) {
	--output_int;
	//output_field.fill(0.0);
	output_field[output_int] = 1.0;
      }
    }
    break;

  default:
    PLERROR("In SDBVMOutputCoder::setOutput: "
	  "Unknown coding type: %d", int(output_coding_));
  }
}

int SDBVMOutputCoder::fieldWidth() const
{
  switch (output_coding_) {
  case SDBVMNumeric:
    return 1;

  case SDBVMOneHot:
    if (num_classes_ == 0)
      PLERROR("In SDBVMOutputCoder::fieldWidth: "
	    "number of output classes not specified");
    return num_classes_;

  case SDBVMOneHotMinus1:
    if (num_classes_ == 0)
      PLERROR("In SDBVMOutputCoder::fieldWidth: "
	    "number of output classes not specified");
    return num_classes_ - 1;

  default:
    PLERROR("In SDBVMOutputCoder::fieldWidth: "
	  "Unknown coding type: %d", int(output_coding_));
  }
  return 0;
}

template <class Mapping>
int SDBVMOutputCoder::getNumClasses(const Mapping& mapping,
				    real other_values_mapping,
				    real missing_values_mapping)
{
  typename Mapping::const_iterator it = mapping.begin(), end = mapping.end();
  int max_of_map = INT_MIN;
  bool all_int = true;

  // Find the maximum of the mapped values
  for (; it != end; ++it) {
    if (all_int && it->second >= 0 && it->second == int(it->second))
      max_of_map = std::max(max_of_map, int(it->second));
    else
      all_int = false;
  }

  return handleOtherAndMissing(all_int, max_of_map,
			       other_values_mapping, missing_values_mapping);
}

int SDBVMOutputCoder::handleOtherAndMissing(bool all_int,
					    int candidate_max,
					    real other_values_mapping,
					    real missing_values_mapping)
{
  // Handle other_values and missing_values mappings.  Causes for
  // dismissal include: infinites, NaN's, non-integers
  if (all_int && finite(other_values_mapping) &&
      other_values_mapping == int(other_values_mapping))
    candidate_max = std::max(candidate_max, int(other_values_mapping));
  else
    // if other_values_mapping is MISSING_VALUE, leave candidate_max as
    // is and don't disable one-hot coding
    if (!is_missing(other_values_mapping))
      all_int = false;

  if (all_int && finite(missing_values_mapping) &&
      missing_values_mapping == int(missing_values_mapping))
    candidate_max = std::max(candidate_max, int(missing_values_mapping));
  else
    // if other_values_mapping is MISSING_VALUE, leave candidate_max as
    // is and don't disable one-hot coding
    if (!is_missing(missing_values_mapping))
      all_int = false;
    
  return all_int? candidate_max+1 : 0;
}  


//#####  SDBVMatrix  ########################################################

SDBVMatrix::SDBVMatrix(const string& dbname, bool detect_missing)
    : sdb_(dbname, "", SDB::readonly), detect_missing_(detect_missing)
{
    row_ = Row(&sdb_.getSchema());
    length_ = sdb_.size();
    width_  = 0;
    if (sdb_.hasStats())
	sdb_.loadStats();
    else {
	sdb_.computeStats();
	sdb_.saveStats();
    }
}

void SDBVMatrix::appendField(const string& name, SDBVMField* new_field)
{
  int fieldwidth = new_field->fieldWidth();
  vector<string> fieldnames = split(name);
  if(fieldwidth>1 && int(fieldnames.size())==fieldwidth)
  {
    for(unsigned int i=0; i<fieldnames.size(); i++)
    {
      width_++;
      declareField(width_-1,fieldnames[i],new_field->fieldType());
    }
  }
  else
  {
    for(int k=0; k<fieldwidth; k++)
    {
      width_++;
      declareField(width_-1,name,new_field->fieldType());
    }
  }

  fields_.push_back(PSDBVMField(new_field));
  current_row.resize(width_);
}

void SDBVMatrix::getRow(int i, Vec v) const
{
  //prof.start("getrow");
  //prof.start("getrow1");
  sdb_.getInRow(i, row_);
    //prof.end("getrow1");
  FieldsVector::const_iterator it = fields_.begin(), end = fields_.end();
  int curpos=0, curwidth;
  for (int f=0; it != end; ++it, curpos+=curwidth, f++) {
    curwidth = (*it)->fieldWidth();
    Vec output = v.subVec(curpos,curwidth);
      //prof.start("getrow2");
    (*it)->convertField(sdb_, row_, output);
      //prof.end("getrow2");
    if (detect_missing_ && output.hasMissing())
      PLWARNING("SDBVMatrix::getRow(%d,v) has missing value for %d-th field (columns %d-%d)",
          i,f,curpos,curpos+curwidth-1);
    }
  //prof.end("getrow");
}


//#####  SDBVMField  ########################################################

SDBVMOutputCoding
SDBVMField::getOutputCoding() const
{
    return SDBVMUnknownCoding;
}

void SDBVMField::convertMissing(const Vec& output) const
{
  for (int i=0;i<output.length();i++)
    if (is_missing(output[i]))
      output[i]=missing_values_mapping_;
}


//#####  SDBVMFieldAsIs  ####################################################

void SDBVMFieldAsIs::convertField(const SDBWithStats& sdb, const Row& row,
				    const Vec& output) const
{
    output[0] = real(source_.getValue(sdb,row));
    convertMissing(output);
}

int SDBVMFieldAsIs::fieldWidth() const
{
    return 1;
}




//#####  SDBVMFieldNormalize  ###############################################

void SDBVMFieldNormalize::convertField(const SDBWithStats& sdb,
					 const Row& row, const Vec& output) const
{
    real x = source_.getValue(sdb,row);
    const FieldStat& stat = source_.getFieldStat(sdb,row);
    output[0] = (x - stat.mean()) / stat.stddev();
    convertMissing(output);
}

int SDBVMFieldNormalize::fieldWidth() const
{
    return 1;
}




//#####  SDBVMFieldDivSigma  ################################################

void SDBVMFieldDivSigma::convertField(const SDBWithStats& sdb,
					const Row& row, const Vec& output) const
{
    real x = source_.getValue(sdb,row);
    const FieldStat& stat = source_.getFieldStat(sdb,row);
    output[0] = x / stat.stddev();
    convertMissing(output);
}

int SDBVMFieldDivSigma::fieldWidth() const
{
    return 1;
}



//#####  SDBVMFieldAffine  ##################################################

void SDBVMFieldAffine::convertField(const SDBWithStats& sdb,
				      const Row& row, const Vec& output) const
{
  real v = source_.getValue(sdb,row); 
  //if (is_missing(v))
  //  output[0] = MISSING_VALUE;
  //else
  output[0] = a_ * v + b_;
  convertMissing(output);
}

int SDBVMFieldAffine::fieldWidth() const
{
    return 1;
}




//#####  SDBVMFieldPosAffine  ###############################################

void SDBVMFieldPosAffine::convertField(const SDBWithStats& sdb,
					 const Row& row, const Vec& output) const
{
    output[0] = a_ * std::max(real(source_.getValue(sdb,row)), real(0.)) + b_;
    convertMissing(output);
}

int SDBVMFieldPosAffine::fieldWidth() const
{
    return 1;
}



//#####  SDBVMFieldSignedPower  #############################################
 
void SDBVMFieldSignedPower::convertField(const SDBWithStats& sdb,
           const Row& row, const Vec& output) const
{
  real x = source_.getValue(sdb,row);
  real sign_x = x>=0.0 ? 1.0 : -1.0;
  output[0] = sign_x * mypow(x*sign_x, a_);
  convertMissing(output);
}
 
int SDBVMFieldSignedPower::fieldWidth() const
{
  return 1;
}



//#####  SDBVMFieldFunc1  ###################################################

void SDBVMFieldFunc1::convertField(const SDBWithStats& sdb,
				     const Row& row, const Vec& output) const
{
    Vec input(1);
    input[0] = source_.getValue(sdb,row);
    output << func_(input);
    convertMissing(output);
}

int SDBVMFieldFunc1::fieldWidth() const
{
    return func_->outputsize;
}




//#####  SDBVMFieldFunc2  ###################################################

void SDBVMFieldFunc2::convertField(const SDBWithStats& sdb,
				     const Row& row, const Vec& output) const
{
    Vec input1(1), input2(1);
    input1[0] = real(source1_.getValue(sdb,row));
    input2[0] = real(source2_.getValue(sdb,row));
    output[0] = func_(input1, input2);
    convertMissing(output);
}

int SDBVMFieldFunc2::fieldWidth() const
{
    return 1;
}




//#####  SDBVMFieldDate  ####################################################

void SDBVMFieldDate::convertField(const SDBWithStats& sdb,
                                  const Row& row, const Vec& output) const
{
  real realval = source_.getValue(sdb,row);
  if(is_missing(realval)) {
    output[0] = missing_values_mapping_;
    output[1] = missing_values_mapping_;
    output[2] = missing_values_mapping_;
  }
  else {
    PDate d = float_to_date(realval);
    output[0] = real(d.year);
    output[1] = real(d.month);
    output[2] = real(d.day);
  }
}

int SDBVMFieldDate::fieldWidth() const
{
  return 3;
}




//#####  SDBVMFieldDay  #####################################################

void SDBVMFieldDay::convertField(const SDBWithStats& sdb,
				   const Row& row, const Vec& output) const
{
    real realval = source_.getValue(sdb,row);
    PDate d = float_to_date(realval);
    // compute a normalized day ranging approximately in [-1,1]
    // for (1980 - 2000).
    output[0] = ((d.year-1990)*365+(d.month-1)*30+(d.day-1))/3650.0;
    convertMissing(output);
}

int SDBVMFieldDay::fieldWidth() const
{
    return 1;
}



//#####  SDBVMFieldMonths  #####################################################

void SDBVMFieldMonths::convertField(const SDBWithStats& sdb,
                                    const Row& row, const Vec& output) const
{
    real realval = source_.getValue(sdb,row);
    PDate d = float_to_date(realval);
    output[0] = d.year*12 + (d.month-1);
    convertMissing(output);
}

int SDBVMFieldMonths::fieldWidth() const
{
    return 1;
}


//#####  SDBVMFieldDateDiff  ###################################################

void SDBVMFieldDateDiff::convertField(const SDBWithStats& sdb,
                                      const Row& row, const Vec& output) const
{
  // WARNING.  Convoluted logic ahead.  Should fix this.

  FieldValue v1 = source1_.getValue(sdb,row);
  if (!v1.isMissing() && !date1_threshold_.isMissing() &&
      v1 <= date1_threshold_)
    v1.setMissing();
  
  PDate d1 = v1.toDate();

  FieldValue v2;
  PDate d2 = refdate;
  output[0] = MISSING_VALUE;  // default value

  if(!d1.isMissing() && refdate.isMissing())
  {
    v2 = source2_.getValue(sdb,row);
    if (!v2.isMissing() && !date2_threshold_.isMissing() &&
        v2 <= date2_threshold_)
      v2.setMissing();
    
    if(v2.isDate())
      d2 = v2.toDate();
    else if(v2.isIntegral()) {
      if (!v2.isMissing()) {
        int value2 = int(v2);
        switch(unit) {
        case 'D':
          output[0] = d1.toJulianDay() - value2;
          break;
        case 'M':
          output[0] = d1.month - value2;
          break;
        case 'Y':
          output[0] = d1.year - value2;
          break;
        default:
          PLERROR("In SDBVMFieldDateDiff: unrecognized unit %c", unit);
        }
        return;
      }
    }
    else 
      PLERROR("In SDBVMFieldDateDiff convertField: second field is neither "
            "a date nor an integer type!");
  }

  // Hack for consistency: (We found a date with year==-32764 (sdb select policy_start_date,drv_last_suspension_end,drv_suspension_days FROM icbc3:2530858:1 ) )
  if(!d1.isMissing() && d1.year<1900)
    d1.setMissing();
  if(!d2.isMissing() && d2.year<1900)
    d2.setMissing();
  if(!d1.isMissing() && !d2.isMissing())
  {
    switch(unit)
    {
    case 'D':
      output[0] = d1-d2;
      break;
    case 'M':
      output[0] = (d1.year*12+d1.month) - (d2.year*12+d2.month);
      break;
    case 'Y':
      output[0] = d1.year - d2.year;
      break;
    default:
      PLERROR("In SDBVMFieldDateDiff: unrecognized unit %c", unit);
    }
  }
}

int SDBVMFieldDateDiff::fieldWidth() const
{
    return 1;
}


//#####  SDBVMFieldDiscrete  ##################################################

SDBVMFieldDiscrete::SDBVMFieldDiscrete(SDBVMSource source, int num_classes,
    real missing_values_mapping, SDBVMOutputCoding oc, VMField::FieldType ft)
: inherited(source,missing_values_mapping,ft), num_classes_(num_classes),
  output_coder_(new SDBVMOutputCoder(oc, missing_values_mapping))
{
  output_coder_->setNumClasses(num_classes);
}

// Version of the constructor that takes an OutputCoder object
SDBVMFieldDiscrete::SDBVMFieldDiscrete(SDBVMSource source, SDBVMOutputCoder* oc,
    int num_classes, real missing_values_mapping, VMField::FieldType ft)
: inherited(source, missing_values_mapping,ft),
  num_classes_(num_classes), output_coder_(oc)
{
  output_coder_->setNumClasses(num_classes);
}

void SDBVMFieldDiscrete::convertField(const SDBWithStats& sdb, const Row& row,
    const Vec& output) const
{
  real value = getDiscreteValue(sdb, row);
  output_coder_->setOutput(value, output);
}

int SDBVMFieldDiscrete::fieldWidth() const
{
  return output_coder_->fieldWidth();
}

SDBVMOutputCoding SDBVMFieldDiscrete::getOutputCoding() const
{
  return output_coder_->getOutputCoding();
}

void SDBVMFieldDiscrete::setNumClasses(int num_classes)
{
  num_classes_ = num_classes;
  output_coder_->setNumClasses(num_classes);
}


//#####  SDBVMFieldDateGreater  ###################################################

void SDBVMFieldDateGreater::convertField(const SDBWithStats& sdb,
				     const Row& row, const Vec& output) const
{
    PDate d = source_.getValue(sdb,row);

    if (d>ref)
      output[0]=1;
    else 
      output[0]=0;
}

int SDBVMFieldDateGreater::fieldWidth() const
{
    return 1;
}

real SDBVMFieldDateGreater::getDiscreteValue(const SDBWithStats& sdb, const
					     Row& row) const
{
  // WARNING: IS THIS CORRECT (Pascal?)
  FieldValue v = source_.getValue(sdb,row);
  if(v.isMissing())
    return missing_values_mapping_;
  return v.toDate()>ref ?1 :0;
}

//#####  SDBVMFieldCodeAsIs  ################################################

real SDBVMFieldCodeAsIs::getDiscreteValue(const SDBWithStats& sdb,
					  const Row& row) const
{
  FieldValue v = source_.getValue(sdb,row);
  return v.isMissing() ?missing_values_mapping_ :real(v);
}


//#####  SDBVMFieldRemapReals  ##############################################

SDBVMFieldRemapReals::RealMap
SDBVMFieldRemapReals::getRealMapping(const string& mappings)
{
    RealMap real_mapping;
    
    if(!mappings.empty()) {
      istrstream in(mappings.c_str());
      real realkey, value;

      for(;;) {
        in >> realkey >> value;
        if (!in)
          break;
        real_mapping[realkey] = value;
      }
    }
    return real_mapping;
}

SDBVMFieldRemapReals::SDBVMFieldRemapReals(SDBVMSource source,
					   const string& mappings,
					   real other_values_mapping,
					   real missing_values_mapping,
					   SDBVMOutputCoding oc,
					   VMField::FieldType ft)
    : inherited(source, 0, missing_values_mapping, oc, ft),
      real_mapping_(getRealMapping(mappings)),
      other_values_mapping_(other_values_mapping)
{
  // Set the base-class num_classes_
  setNumClasses(SDBVMOutputCoder::getNumClasses(
    real_mapping_, other_values_mapping, missing_values_mapping));
}

SDBVMFieldRemapReals::SDBVMFieldRemapReals(SDBVMSource source,
					   const FieldStat& field_stat,
					   real other_values_mapping,
					   real missing_values_mapping,
					   SDBVMOutputCoding oc,
					   VMField::FieldType ft)
    : inherited(source, 0, missing_values_mapping, oc, ft),
      other_values_mapping_(other_values_mapping)
{
    map<string,int>::iterator it = field_stat.symbolid.begin(),
	end = field_stat.symbolid.end();
    for( ; it != end; ++it)
	real_mapping_[real(todouble(it->first))] = it->second;
    
    // Set the base-class num_classes_
    setNumClasses(SDBVMOutputCoder::getNumClasses(
      real_mapping_, other_values_mapping, missing_values_mapping));
}

real SDBVMFieldRemapReals::getDiscreteValue(const SDBWithStats& sdb,
					    const Row& row) const
{
  FieldValue v = source_.getValue(sdb,row);
  if(v.isMissing())
    return missing_values_mapping_;

  real realval = real(v);
  RealMap::const_iterator found = real_mapping_.find(realval);
  if (found != real_mapping_.end())
    realval = found->second;
  else
    realval = other_values_mapping_;
  return realval;
}


//#####  NGSDBVMFieldRemapStrings  ############################################

SDBVMFieldRemapStrings::StringMap
SDBVMFieldRemapStrings::getStringMapping(const string& mappings)
{
    StringMap string_mapping;
    
    if(!mappings.empty()) {
        istrstream in(mappings.c_str());
	string stringkey;
	real value;

        for(;;) {
            in >> stringkey >> value;
            if (!in)
		break;
            string_mapping[stringkey] = value;
	}
    }
    return string_mapping;
}

SDBVMFieldRemapStrings::SDBVMFieldRemapStrings(SDBVMSource source,
					       const string& mappings,
					       real other_values_mapping,
					       real missing_values_mapping,
					       SDBVMOutputCoding oc,
					       VMField::FieldType ft)
    : inherited(source, 0, missing_values_mapping, oc, ft),
      string_mapping_(getStringMapping(mappings)),
      other_values_mapping_(other_values_mapping)
{
    // Set the base-class num_classes_
    setNumClasses(SDBVMOutputCoder::getNumClasses(
      string_mapping_, other_values_mapping, missing_values_mapping));
}

SDBVMFieldRemapStrings::SDBVMFieldRemapStrings(SDBVMSource source,
					       const FieldStat& field_stat,
					       real other_values_mapping,
					       real missing_values_mapping,
					       SDBVMOutputCoding oc,
					       VMField::FieldType ft)
    : inherited(source, 0, missing_values_mapping, oc, ft),
      other_values_mapping_(other_values_mapping)
{
    map<string,int>::iterator it = field_stat.symbolid.begin(),
	end = field_stat.symbolid.end();
    for( ; it != end; ++it)
	string_mapping_[it->first] = it->second;
    
    // Set the base-class num_classes_
    setNumClasses(SDBVMOutputCoder::getNumClasses(
      string_mapping_, other_values_mapping, missing_values_mapping));
}

real SDBVMFieldRemapStrings::getDiscreteValue(const SDBWithStats& sdb,
					      const Row& row) const
{
  real realval;
  FieldValue v = source_.getValue(sdb,row);
  if(v.isMissing())
    /*NOTE: this used to be a BUG!!! the following used to be
      realval = missing_values_mapping_;
      but realval was overriden with other_values_mapping_
      a few lines later... and never returned
     */
    return missing_values_mapping_;
  string s = space_to_underscore(string(v));
  StringMap::const_iterator found = string_mapping_.find(s);
  if (found != string_mapping_.end())
    realval = found->second;
  else
    realval = other_values_mapping_;
  return realval;
}


//#####  NGSDBVMFieldRemapIntervals  ##########################################

void
SDBVMFieldRemapIntervals::getIntervals(const string& mappings,
				       bool& all_int,    real& max_of_map,
				       Vec& intervals_x, Vec& intervals_y)
{
    all_int = true;
    max_of_map = -FLT_MAX;
    istrstream in(mappings.c_str());
    real xi, yi;
    intervals_x.resize(10);
    intervals_y.resize(11);
    
    int i;
    for(i=0; ; ++i) {
	in >> yi;
	if(!in)
	    PLERROR("In NGSDBVMFieldRemapIntervals::getIntervals: "
		  "mappings should have an odd number of elements, found %d",
		  2*i);
	intervals_y[i] = yi;
	if (all_int && yi >= 0 && yi == int(yi))
	    max_of_map = std::max(max_of_map, yi);
	else
	    all_int = false;

	in >> xi;
	if(!in)
	    break;
	if (i>0 && intervals_x[i-1]>=xi)
	    PLERROR("In NGSDBVMFieldRemapIntervals::getIntervals: "
		  "mappings needs x_{i-1}<x_i, found x[%d]=%f, x[%d]=%f",
		  i-1,intervals_x[i-1],i,xi);
	intervals_x[i] = xi;
	if (intervals_x.length()==i+1) {
	    intervals_x.resize(2*i);
	    intervals_y.resize(2*i+1);
	}
    }

    /*NOTE: this used to be a BUG!!! the following used to be:
      intervals_x.resize(i-1);
      intervals_y.resize(i);
      which made both vectors 1 too short... 
      ...no value would ever fall in the last class
     */
    intervals_x.resize(i);
    intervals_y.resize(i+1);
}

SDBVMFieldRemapIntervals::SDBVMFieldRemapIntervals(SDBVMSource source,
						   const string& mappings,
						   real other_values_mapping,
						   real missing_values_mapping,
						   SDBVMOutputCoding oc,
						   VMField::FieldType ft)
  : inherited(source, 0, missing_values_mapping, oc, ft),
    other_values_mapping_(other_values_mapping)
{
  real max_of_map;
  bool all_int;
  getIntervals(mappings, all_int, max_of_map, intervals_x_, intervals_y_);

  // Initialize base-class member
  setNumClasses(SDBVMOutputCoder::handleOtherAndMissing(
    all_int, int(max_of_map), other_values_mapping, missing_values_mapping));
}

real SDBVMFieldRemapIntervals::getDiscreteValue(const SDBWithStats& sdb,
						const Row& row) const
{
  FieldValue v = source_.getValue(sdb,row);
  if(v.isMissing()) 
    return missing_values_mapping_;
  else
    return intervals_y_[1+int(binary_search(intervals_x_,real(v)))];
}


//#####  SDBVMFieldMultiDiscrete  #############################################

SDBVMFieldMultiDiscrete::SDBVMFieldMultiDiscrete(const FieldArray& fields,
						 real missing_values_mapping,
						 SDBVMOutputCoding oc,
						 VMField::FieldType ft)
  : inherited(FieldPtr() /* "null pointer" */, 0,
	      missing_values_mapping, oc, ft)
{
  setFields(fields);
}

void SDBVMFieldMultiDiscrete::setFields(const FieldArray& fields)
{
  fields_ = fields;
  int n = fields.size();
  field_multipliers_.resize(n);
  setNumClasses(0);

  // Compute all partial products from i to n
  if (n>0) {
    int prod = 1;
    field_multipliers_[n-1] = 1;
    for (int i=n-2; i>=0; --i) {
      prod *= fields[i+1]->getNumClasses();
      field_multipliers_[i] = prod;
    }
    prod *= fields[0]->getNumClasses();
    setNumClasses(prod);
  }
}

real SDBVMFieldMultiDiscrete::getDiscreteValue(const SDBWithStats& sdb,
					       const Row& row) const
{
  int index = 0;
  int n = fields_.size();
  for (int i=0; i<n; ++i) {
    real value = fields_[i]->getDiscreteValue(sdb,row);
    if (value != int(value) || value < 0)
      PLERROR("SDBVMFieldMultiDiscrete::getDiscreteValue: negative or "
	    "non-integer value %f returned for field %d", value, i);
    index += int(value)*int(field_multipliers_[i]);
  }
  return real(index);
}

//  int SDBVMFieldMultiDiscrete::fieldWidth() const
//  {
//    return 1;
//  }

  

//#####  SDBVMFieldICBCTargets  ###############################################

SDBVMFieldICBCTargets::SDBVMFieldICBCTargets(Schema schema, bool use_roadstar,
    bool add_claims_sum_column, bool rescale_by_interval,
    bool rescale_by_start_date, Mat& start_date_rescaling_values, const string& targetname)
  : inherited(0,VMField::Continuous), use_roadstar_(use_roadstar),
  add_claims_sum_column_(add_claims_sum_column),
  rescale_by_interval_(rescale_by_interval),
  rescale_by_start_date_(rescale_by_start_date),
  start_date_rescaling_values_(start_date_rescaling_values),
  targetname_(targetname),
  start_date_(schema("policy_start_date")),
  end_date_(schema("policy_end_date")),
  bodily_injury_incurred_((targetname_=="ALL" || targetname_=="bodily_injury_incurred") ?
                          schema("bodily_injury_incurred") : FieldPtr()),
  property_damage_incurred_((targetname_=="ALL" || targetname_=="sum_all_but_BI" 
                             || targetname_=="property_damage_incurred") ?
                            schema("property_damage_incurred") : FieldPtr()),
  accident_death_incurred_((targetname_=="ALL" || targetname_=="sum_all_but_BI" 
                            || targetname_=="accident_death_incurred") ?
                           schema("accident_death_incurred") : FieldPtr()),
  collision_lou_incurred_((targetname_=="ALL" || targetname_=="sum_all_but_BI"
                           || targetname_=="collision_lou_incurred") ?
                          schema("collision_lou_incurred") : FieldPtr()),
  comprehensive_incurred_((targetname_=="ALL" || targetname_=="sum_all_but_BI"
                           || targetname_=="comprehensive_incurred") ?
                          schema("comprehensive_incurred") : FieldPtr()),
  bodily_injury_count_((targetname_=="ALLcounts" || targetname_=="bodily_injury_count") ?
		       schema("bodily_injury_count") : FieldPtr()),
  property_damage_count_((targetname_=="ALLcounts" || targetname_=="all_counts_but_BI"
			  || targetname_=="property_damage_count") ?
			 schema("property_damage_count") : FieldPtr()),
  accident_death_count_((targetname_=="ALLcounts" || targetname_=="all_counts_but_BI"
			 || targetname_=="accident_death_count") ?
			schema("accident_death_count") : FieldPtr()),
  collision_lou_count_((targetname_=="ALLcounts" || targetname_=="all_counts_but_BI"
			|| targetname_=="collision_lou_count") ?
		       schema("collision_lou_count") : FieldPtr()),
  comprehensive_count_((targetname_=="ALLcounts" || targetname_=="all_counts_but_BI"
			|| targetname_=="comprehensive_count") ?
		       schema("comprehensive_count") : FieldPtr()),
  bodily_injury_severity_((targetname_=="ALLseverities" || targetname_=="bodily_injury_severity") ?
		       schema("bodily_injury_severity") : FieldPtr()),
  property_damage_severity_((targetname_=="ALLseverities" || targetname_=="all_severities_but_BI"
			  || targetname_=="property_damage_severity") ?
			 schema("property_damage_severity") : FieldPtr()),
  accident_death_severity_((targetname_=="ALLseverities" || targetname_=="all_severities_but_BI"
			 || targetname_=="accident_death_severity") ?
			schema("accident_death_severity") : FieldPtr()),
  collision_lou_severity_((targetname_=="ALLseverities" || targetname_=="all_severities_but_BI"
			|| targetname_=="collision_lou_severity") ?
		       schema("collision_lou_severity") : FieldPtr()),
  comprehensive_severity_((targetname_=="ALLseverities" || targetname_=="all_severities_but_BI"
			|| targetname_=="comprehensive_severity") ?
		       schema("comprehensive_severity") : FieldPtr())
  // roadstar_incurred_((use_roadstar && (targetname_ == "ALL" || targetname == "roadstar_incurred")) ?
  //                   schema("roadstar_incurred") : FieldPtr())

{
  reference_start_date_year_ = rescale_by_start_date_ ? (int) start_date_rescaling_values_[0][0] : 0;
  reference_start_date_month_ = rescale_by_start_date_ ? (int) start_date_rescaling_values_[0][1] : 0;
}

void SDBVMFieldICBCTargets::convertField(const SDBWithStats& sdb,
                                         const Row& row, const Vec& output) const
{
  int i = 0;
  PDate start_date = start_date_.getValue(sdb,row).toDate();
  PDate end_date = end_date_.getValue(sdb,row).toDate();
  // ******* BIG HACK TO GET AROUND A NASTY INCOMPREHENSIBLE BUG ****
  // (with the value read from file at this point not corresponding
  // to the value as seen otherwise in the file for the date of certain records)
  if (start_date.year<1970) start_date.year = end_date.year-1;
  // ****************************************************************
  real normalization = 0.001;
  real duration = (end_date - start_date)/365.25;

  if (rescale_by_interval_) {
    if (is_missing(duration) || duration<=0)
    {
      cout << "start_date = " << start_date << endl;
      cout << "end_date = " << end_date << endl;
      PLERROR("SDBVMFieldICBCTargets: incorrect dates");
    }
    normalization = 0.001/duration;
  }
  if (rescale_by_start_date_) {
    int row_index = (start_date.year - reference_start_date_year_ - 1) * 12
      + 12 - reference_start_date_month_ + start_date.month;
    if (targetname_=="ALL" || targetname_=="bodily_injury_incurred")
    {
      if (start_date_rescaling_values_[row_index][2] == 0)
        PLERROR("Trying to divide by zero");
      output[i++] = real(bodily_injury_incurred_.getValue(sdb,row))*normalization /
        start_date_rescaling_values_[row_index][2];
    }
    if (targetname_=="ALL" || targetname_=="property_damage_incurred")
    {
      if (start_date_rescaling_values_[row_index][3] == 0)
        PLERROR("Trying to divide by zero");
      output[i++] = real(property_damage_incurred_.getValue(sdb,row))*normalization /
        start_date_rescaling_values_[row_index][3];
    }
    if (targetname_=="ALL" || targetname_=="accident_death_incurred")
    {
      if (start_date_rescaling_values_[row_index][4] == 0)
        PLERROR("Trying to divide by zero");
      output[i++] = real(accident_death_incurred_.getValue(sdb,row))*normalization /
        start_date_rescaling_values_[row_index][4];
    }
    if (targetname_=="ALL" || targetname_=="collision_lou_incurred")
    {
      if (start_date_rescaling_values_[row_index][5] == 0)
        PLERROR("Trying to divide by zero");
      output[i++] = real(collision_lou_incurred_.getValue(sdb,row))*normalization /
        start_date_rescaling_values_[row_index][5];
    }
    if (targetname_=="ALL" || targetname_=="comprehensive_incurred")
    {
      if (start_date_rescaling_values_[row_index][6] == 0)
        PLERROR("Trying to divide by zero");
      output[i++] = real(comprehensive_incurred_.getValue(sdb,row))*normalization /
        start_date_rescaling_values_[row_index][6];
    }
    if (targetname_=="sum_all_but_BI")
    {
      if (start_date_rescaling_values_[row_index][3] == 0 ||
          start_date_rescaling_values_[row_index][4] == 0 ||
          start_date_rescaling_values_[row_index][5] == 0 ||
          start_date_rescaling_values_[row_index][6] == 0)
        PLERROR("Trying to divide by zero");
      output[i++] = (real(property_damage_incurred_.getValue(sdb,row))/
                     start_date_rescaling_values_[row_index][3] +
                     real(accident_death_incurred_.getValue(sdb,row))/
                     start_date_rescaling_values_[row_index][4] +
                     real(collision_lou_incurred_.getValue(sdb,row))/
                     start_date_rescaling_values_[row_index][5] +
                     real(comprehensive_incurred_.getValue(sdb,row))/
                     start_date_rescaling_values_[row_index][6])*normalization;
    }
  }
  else { //  no rescale by start date
    if (targetname_=="ALL" || targetname_=="bodily_injury_incurred")
      output[i++] = real(bodily_injury_incurred_.getValue(sdb,row))*normalization; 
    if (targetname_=="ALL" || targetname_=="property_damage_incurred")
      output[i++] = real(property_damage_incurred_.getValue(sdb,row))*normalization; 
    if (targetname_=="ALL" || targetname_=="accident_death_incurred")
      output[i++] = real(accident_death_incurred_.getValue(sdb,row))*normalization; 
    if (targetname_=="ALL" || targetname_=="collision_lou_incurred")
      output[i++] = real(collision_lou_incurred_.getValue(sdb,row))*normalization; 
    if (targetname_=="ALL" || targetname_=="comprehensive_incurred")
      output[i++] = real(comprehensive_incurred_.getValue(sdb,row))*normalization; 
    if (targetname_=="sum_all_but_BI")
      output[i++] = (real(property_damage_incurred_.getValue(sdb,row))+
                     real(accident_death_incurred_.getValue(sdb,row))+
                     real(collision_lou_incurred_.getValue(sdb,row))+
                     real(comprehensive_incurred_.getValue(sdb,row)))*normalization;

    //counts
    if (targetname_=="ALLcounts" || targetname_=="bodily_injury_count")
      output[i++] = real(bodily_injury_count_.getValue(sdb,row));
    if (targetname_=="ALLcounts" || targetname_=="all_counts_but_BI" || targetname_=="property_damage_count")
      output[i++] = real(property_damage_count_.getValue(sdb,row));
    if (targetname_=="ALLcounts" || targetname_=="all_counts_but_BI" || targetname_=="accident_death_count")
      output[i++] = real(accident_death_count_.getValue(sdb,row));
    if (targetname_=="ALLcounts" || targetname_=="all_counts_but_BI" || targetname_=="collision_lou_count")
      output[i++] = real(collision_lou_count_.getValue(sdb,row));
    if (targetname_=="ALLcounts" || targetname_=="all_counts_but_BI" || targetname_=="comprehensive_count")
      output[i++] = real(comprehensive_count_.getValue(sdb,row));


    //severities
    if (targetname_=="ALLseverities" || targetname_=="bodily_injury_severity")
      {
        int n = int(real(bodily_injury_count_.getValue(sdb,row)));
	output[i++] = n>0? real(bodily_injury_incurred_.getValue(sdb,row)) / n: 0;
      }
    if (targetname_=="ALLseverities" || targetname_=="all_severities_but_BI" || targetname_=="property_damage_severity")
      {
        int n = int(real(property_damage_count_.getValue(sdb,row)));
	output[i++] = n>0? real(property_damage_incurred_.getValue(sdb,row)) / n: 0;
      }
    if (targetname_=="ALLseverities" || targetname_=="all_severities_but_BI" || targetname_=="accident_death_severity")
      {
        int n = int(real(accident_death_count_.getValue(sdb,row)));
	output[i++] = n>0? real(accident_death_incurred_.getValue(sdb,row)) / n: 0;
      }
    if (targetname_=="ALLseverities" || targetname_=="all_severities_but_BI" || targetname_=="collision_lou_severity")
      {
        int n = int(real(collision_lou_count_.getValue(sdb,row)));
	output[i++] = n>0? real(collision_lou_incurred_.getValue(sdb,row)) / n: 0;
      }
    if (targetname_=="ALLseverities" || targetname_=="all_severities_but_BI" || targetname_=="comprehensive_severity")
      {
        int n = int(real(comprehensive_count_.getValue(sdb,row)));
	output[i++] = n>0? real(comprehensive_incurred_.getValue(sdb,row)) / n: 0;
      }

    //severity weights
    if (targetname_=="ALLseverities")
	output[i++] = real(bodily_injury_count_.getValue(sdb,row))>0? 1 : 0;
    if (targetname_=="ALLseverities" || targetname_=="all_severities_but_BI")
      {
	output[i++] = real(property_damage_count_.getValue(sdb,row))>0? 1 : 0;
	output[i++] = real(accident_death_count_.getValue(sdb,row))>0? 1 : 0;
	output[i++] = real(collision_lou_count_.getValue(sdb,row))>0? 1 : 0;
	output[i++] = real(comprehensive_count_.getValue(sdb,row))>0? 1 : 0;
      }

  }
  if (add_claims_sum_column_) {
    if (targetname_=="ALL")
      output[i] = output[0] + output[1] + output[2] + output[3] + output[4];
    else
      output[i] = output[0];
    i++;
  }
  if (rescale_by_interval_)
    output[i] = duration; // weight that should be given to squared loss
  // REPLACE MISSING VALUES BY ZEROS
  convertMissing(output);
}


//#####  SDBVMFieldHasClaim  ###############################################

void SDBVMFieldHasClaim::convertField(const SDBWithStats& sdb,
                                         const Row& row, Vec& output) const
{
  real a,b,c,d,e,f;  
  a = real(row.bind(bodily_injury_incurred_).toDouble());
  b = real(row.bind(property_damage_incurred_).toDouble());
  c = real(row.bind(accident_death_incurred_).toDouble());
  d = real(row.bind(collision_lou_incurred_).toDouble());
  e = real(row.bind(comprehensive_incurred_).toDouble());
  f = real(row.bind(roadstar_incurred_).toDouble());
  output[0] = (a!=0) || (b!=0) || (c!=0) || (d!=0) || (e!=0) || (f!=0);
  // REPLACE MISSING VALUES BY ZEROS
  convertMissing(output);
}

//#####  SDBVMFieldSumClaims  ###############################################

void SDBVMFieldSumClaims::convertField(const SDBWithStats& sdb,
                                         const Row& row, Vec& output) const
{
  real a,b,c,d,e,f;  
  a = real(row.bind(bodily_injury_incurred_).toDouble());
  b = real(row.bind(property_damage_incurred_).toDouble());
  c = real(row.bind(accident_death_incurred_).toDouble());
  d = real(row.bind(collision_lou_incurred_).toDouble());
  e = real(row.bind(comprehensive_incurred_).toDouble());
  f = real(row.bind(roadstar_incurred_).toDouble());
  output[0] = a+b+c+d+e+f;
  // REPLACE MISSING VALUES BY ZEROS
  convertMissing(output);
}


int ICBCpartition(const Vec& claims, real threshold)
{
  bool flag_big = 0;
  bool flag_pos = 0;
  bool flag_neg = 0;

  for (int j=0; j<claims.length(); j++)
  {
    if (claims[j]>threshold)  {flag_big=1;}
    else if (claims[j]>0) {flag_pos=1;}
    else if (claims[j]<0) {flag_neg=1;}
  }

  if (flag_big) return 3;
  else if (flag_pos) return 2;
  else if (flag_neg) return 0;
  else return 1;
}


//#####  SDBVMFieldICBCClassification  ###############################################

SDBVMFieldICBCClassification::SDBVMFieldICBCClassification(Schema schema, const string& fieldname,const string& tmap_file)
  : inherited(0),
    bodily_injury_incurred_(schema("bodily_injury_incurred")),
    property_damage_incurred_(schema("property_damage_incurred")),
    accident_death_incurred_(schema("accident_death_incurred")),
    collision_lou_incurred_(schema("collision_lou_incurred")),
    comprehensive_incurred_(schema("comprehensive_incurred")),
    policy_start_date_(schema("policy_start_date")),
    fieldname_(fieldname)
{

  // is this used anyway? threshold seems to be hardcoded in ConvertField
  if (fieldname == "")
    threshold = 10000;
  else if (fieldname == "condprob3")
    threshold = 10000;
  else if (fieldname == "bodily_injury_incurred")
    threshold = 50000;
  else if (fieldname == "property_damage_incurred")
    threshold = 4000;
  else if (fieldname == "accident_death_incurred")
    threshold = 12000;
  else if (fieldname == "collision_lou_incurred")
    threshold = 5000;
  else if (fieldname == "comprehensive_incurred")
    threshold = 1000;
  else if (fieldname == "sum_all_but_BI")
    threshold = 10000;
  else
    PLERROR("Bad field fieldname");
}

void SDBVMFieldICBCClassification::convertField(const SDBWithStats& sdb,
    const Row& row, const Vec& output) const
{
  int threshold = 10000;
  Vec claims(1);
  if (fieldname_ == "")
  { 
    claims.resize(5);
    claims[0] = real(bodily_injury_incurred_.getValue(sdb,row));
    claims[1] = real(property_damage_incurred_.getValue(sdb,row));
    claims[2] = real(accident_death_incurred_.getValue(sdb,row));
    claims[3] = real(collision_lou_incurred_.getValue(sdb,row));
    claims[4] = real(comprehensive_incurred_.getValue(sdb,row));
  }
  // ugly hack
  else if (fieldname_ == "condprob3")
  { 
    claims[0] = real(property_damage_incurred_.getValue(sdb,row))
    + real(accident_death_incurred_.getValue(sdb,row))
    + real(collision_lou_incurred_.getValue(sdb,row))
    + real(comprehensive_incurred_.getValue(sdb,row));
    output[0]=claims[0]<=0?0:1;

    return;
  }
  
  else if (fieldname_ == "bodily_injury_incurred")
    claims[0] = real(bodily_injury_incurred_.getValue(sdb,row));
  else if (fieldname_ == "property_damage_incurred")
    claims[0] = real(property_damage_incurred_.getValue(sdb,row));
  else if (fieldname_ == "accident_death_incurred")
    claims[0] = real(accident_death_incurred_.getValue(sdb,row));
  else if (fieldname_ == "collision_lou_incurred")
    claims[0] = real(collision_lou_incurred_.getValue(sdb,row));
  else if (fieldname_ == "comprehensive_incurred")
    claims[0] = real(comprehensive_incurred_.getValue(sdb,row));
  else if (fieldname_ == "sum_all_but_BI")
    claims[0] = real(property_damage_incurred_.getValue(sdb,row)) +
      real(accident_death_incurred_.getValue(sdb,row)) +
      real(collision_lou_incurred_.getValue(sdb,row)) +
      real(comprehensive_incurred_.getValue(sdb,row));
  output[0] = ICBCpartition(claims, threshold);

}













} // end of namespace PLearn
