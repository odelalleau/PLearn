// -*- C++ -*-
// SDBVMat.cc: Implementation of VMat/SDB Interface
//
// Copyright (C) 2000 Nicolas Chapados
// Copyright (C) 2001 Yoshua Bengio
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

/*! \file PLearn/plearn/db/SDBVMat.h */

#ifndef NGSDBVMAT_H
#define NGSDBVMAT_H

#include <plearn/vmat/VMat.h>
#include <plearn/vmat/RowBufferedVMatrix.h>
#include <plearn/var/Func.h>
#include "SDBWithStats.h"

namespace PLearn {
using namespace std;


//#########################  CLASS  SDBVMOUTPUTCODER  #########################
/*!   
  Code a real number into a vector.  Possible codings are one-hot-like
  variations, and, yes, identity.  At the moment, the coding is specified
  by a simple enum, but later could be upgraded to support derived classes
  as well.
  
  This class supports remapping MISSING_VALUEs that are passed to
  setOutput onto some arbitrary real number (including MISSING_VALUE).
  
  One-hot coding supports a special treatment regarding missing values: if
  a MISSING_VALUE is passed to setOutput, and the missing_values_mapping
  leaves it as-is, and one-hot coding is in effect, all the elements of
  the one-hot vector are set to MISSING_VALUE.
*/

enum SDBVMOutputCoding {
    SDBVMUnknownCoding = 0,
    SDBVMNumeric,				     //!<  straight output
    SDBVMOneHot,				     //!<  classic one-hot
    SDBVMOneHotMinus1			     //!<  One-hot vector containing
    //!    all but first element
    //!    (which is skipped)
};

class SDBVMOutputCoder : public PPointable
{
public:
    SDBVMOutputCoder(SDBVMOutputCoding oc = SDBVMNumeric,
                     real missing_values_mapping = MISSING_VALUE);
    virtual ~SDBVMOutputCoder();

    //!  --- State accessors and mutators

    //!  Return current output coding in effect
    virtual SDBVMOutputCoding getOutputCoding() const;
  
    //!  Specify the number of classes if something like one-hot coding is
    //!  desired
    virtual void setNumClasses(int n_classes);

    //!  Return the current number of classes; this can be zero if only
    //!  straight numeric coding is desired
    virtual int getNumClasses() const;

    //!  Specify a mapping for missing values passed to setOutput
    virtual void setMissingValuesMapping(real missing_values_mapping);

    //!  Return current missing values mapping
    virtual real getMissingValuesMapping() const;

  
    //!  --- Conversion operations
  
    //!  Return the field width required by the selected coding and the number
    //!  of classes
    virtual int fieldWidth() const;

    //!  Code the given real value into the vector; obviously, if one-hot-like
    //!  coding is desired, a reasonable integer should be passed
    virtual void setOutput(real output_value, const Vec& output_field) const;

public:
/*!       Utility function to derive the number of classes given a generic
  mapping: This function iterates over all targets of a mapping, and
  notes whether the mapped values are all non-negative integers; if so,
  it returns one more than the largest encountered integer.  Otherwise,
  it returns zero.  If other_values_mapping or missing_values_mapping is
  MISSING_VALUE, the number of classes is not affected.  Note that
  other_values_mapping can be taken into account by this function, but
  is not otherwise handled by this class; this special mapping must be
  handled by other classes.
*/
    template <class Mapping>
    static int getNumClasses(const Mapping& mapping,
                             real other_values_mapping,
                             real missing_values_mapping);

    //!  Called by getNumClasses to handle special mappings
    static int handleOtherAndMissing(bool all_int,
                                     int candidate_max,
                                     real other_values_mapping,
                                     real missing_values_mapping);

protected:
    SDBVMOutputCoding output_coding_;
    int num_classes_;			     //!<  must be >0 for one-hot coding
    real missing_values_mapping_;
};

typedef PP<SDBVMOutputCoder> PSDBVMOutputCoder;


//###########################  CLASS  NGSDBVMFIELD  ###########################
/*!   
  Base class for preprocessing SDB==>VMatrix
  
  This class provides basic functionality for handling generic functional
  transformation between SimpleDB field(s) and VMatrix segments.  (A
  segment is defined as a part of a VMatrix row, consisting of one or more
  columns).
*/

class SDBVMField : public PPointable
{
public:
    //!  Constructor: specifies the mapping for missing values
    SDBVMField(real missing_values_mapping=MISSING_VALUE, VMField::FieldType field_type=VMField::UnknownType)
        : missing_values_mapping_(missing_values_mapping),
          field_type_(field_type) {}
    
/*!       Given a database row, convert the appropriate parts to a
  (preallocated) output vector of the correct width (given by
  fieldWidth).  Replace MISSING_VALUEs by missing_values_mapping.
*/
    virtual void convertField(const SDBWithStats& sdb,
                              const Row& theRow,
                              const Vec& outputField) const = 0;

    //!  Width that the field occupies in the VMat
    virtual int fieldWidth() const = 0;

/*!       Query the kind of mapping performed by each field
  DiscrGeneral: arbitrary discrete values
  DiscrMonotonic: monotonic discrete values
  DiscrFloat: monotonic + some exceptions
*/

    VMField::FieldType fieldType() const { return field_type_; }

    bool isContinuous() const { return field_type_ == VMField::Continuous; }
    bool isDiscrGeneral() const { return field_type_ == VMField::DiscrGeneral; }
    bool isDiscrMonotonic() const { return field_type_ == VMField::DiscrMonotonic; }
    bool isDiscrFloat() const { return field_type_ == VMField::DiscrFloat; }
    bool isDate() const { return field_type_ == VMField::Date; }

    virtual SDBVMOutputCoding getOutputCoding() const;

protected:
    //!  Replace all MISSING_VALUEs in output vector by missing_values_mapping_
    void convertMissing(const Vec& output) const;

protected:
    real missing_values_mapping_;
    VMField::FieldType field_type_;
};

typedef PP<SDBVMField> PSDBVMField;


//!  A SDBVMSource represents a source for a value that can be either
//!  directly a field from a SDB or an already processed SDBVMField

class SDBVMSource
{
protected:
    FieldPtr sdbfieldptr;
    PSDBVMField sdbvmfieldptr;
    Vec output;

public:
    SDBVMSource(FieldPtr the_sdbfieldptr)
        :sdbfieldptr(the_sdbfieldptr) {}

    SDBVMSource(PSDBVMField the_sdbvmfieldptr)
        :sdbvmfieldptr(the_sdbvmfieldptr), 
         output(1)
    {
        if(sdbvmfieldptr->fieldWidth()!=1)
            PLERROR("Can't make a SDBVMSource from a SDBVMField whose width is other than 1");
    }

    //!  to get the value of this source
    FieldValue getValue(const SDBWithStats& sdb, const Row& row) const
    {
        if(sdbfieldptr)
            return *row.bind(sdbfieldptr);
        else
        {
            sdbvmfieldptr->convertField(sdb,row,output);
            return FieldValue(output[0]);
        }
    }

    //!  to get the statistics for this source
    //!  (works only if the source is a FieldPtr, produces an error if it's a PSDBVMField)
    const FieldStat& getFieldStat(const SDBWithStats& sdb, const Row& row) const
    {
        if(!sdbfieldptr)
            PLERROR("works only if the source is a FieldPtr");
        return sdb.getStat(sdbfieldptr.field_index());
    }
};


//###########################  CLASS  NGSDBVMATRIX  ###########################
/*!   
  This is a VMatrix that acts as a front-end to a SimpleDB (actually, an
  SDBWithStats).  You can add as many fields derived from SDBVMField as
  required with the appendField function. 
*/

class SDBVMatrix : public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;
    typedef vector<PSDBVMField> FieldsVector;

public:
    //!  Currently, the constructor assumes that the SDB exists on-disk
    SDBVMatrix(const string& dbname, bool detect_missing=false);

    //!  Use default dtor, copy ctor, op=

    //!  Add a new field to the VMatrix.  The VMatrix assumes ownership of
    //!  the field.
    void appendField(const string& name, SDBVMField* new_field);

    //!  Obtain a list of the added fields
    const FieldsVector& getFields() const {
        return fields_;
    }

    //!  Transform row i of the SimpldDB into a row of the VMatrix
    virtual void getRow(int i, Vec v) const;

    //!  Access underlying sdb
    SDBWithStats& sdb() {
        return sdb_;
    }
    const SDBWithStats& sdb() const {
        return sdb_;
    }
    
protected:
    SDBWithStats sdb_;
    FieldsVector fields_;
    mutable Row row_;
    bool detect_missing_;
};



//#####  VM Fields  ###########################################################

//!  A field that maps exactly 1 SDB field to a VMatrix segment (abstract)
class SDBVMFieldSource1 : public SDBVMField
{
    typedef SDBVMField inherited;
    
public:
    //!  Use, e.g. my_schema("column_name"), to easily get a FieldPtr from a
    //!  symbolic column name
    SDBVMFieldSource1(SDBVMSource source, 
                      real missing_values_mapping=MISSING_VALUE,
                      VMField::FieldType field_type=VMField::UnknownType)
        : inherited(missing_values_mapping,field_type), source_(source) {}

protected:
    SDBVMSource source_;
};

//!  A field that maps exactly 2 SDB fields to a VMatrix segment (abstract)
class SDBVMFieldSource2 : public SDBVMField
{
    typedef SDBVMField inherited;
    
public:
    //!  Use, e.g. my_schema("column_name"), to easily get a FieldPtr from a
    //!  symbolic column name
    SDBVMFieldSource2(SDBVMSource source1, SDBVMSource source2, 
                      real missing_values_mapping=MISSING_VALUE,
                      VMField::FieldType field_type=VMField::UnknownType)
        : inherited(missing_values_mapping,field_type), source1_(source1), source2_(source2) {}

protected:
    SDBVMSource source1_;
    SDBVMSource source2_;
};


//#####  Simple Numerical Conversions  ########################################

//!  Pass through the value within the SDB (after conversion to real of the
//!  underlying SDB type)
class SDBVMFieldAsIs : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
    
public: 
    SDBVMFieldAsIs(SDBVMSource source, real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Continuous) {}
   
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;
};

    
//!  Normalize the field (subtract the mean then divide by standard dev)
class SDBVMFieldNormalize : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
    
public: 
    SDBVMFieldNormalize(SDBVMSource source, 
                        real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Continuous) {}
   
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;
};


//!  Just divide by standard deviation
class SDBVMFieldDivSigma : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
    
public: 
    SDBVMFieldDivSigma(SDBVMSource source, 
                       real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Continuous) {}
   
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;
};


//!  Apply an affine transformation to the field: y = a*x+b
class SDBVMFieldAffine : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
    
public: 
    SDBVMFieldAffine(SDBVMSource source, real a, real b=0.0, 
                     real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Continuous), a_(a), b_(b) {}
   
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;

protected:
    real a_, b_;
};


//!  Take the positive part of the field, followed by affine transformation:
//!  y = a*max(x,0)+b
class SDBVMFieldPosAffine : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
    
public: 
    SDBVMFieldPosAffine(SDBVMSource source, real a, real b=0.0,
                        real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Continuous), a_(a), b_(b) {}
    
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;

protected:
    real a_, b_;
};


//!  Do the following : y = x^a
class SDBVMFieldSignedPower : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
 
public:
    SDBVMFieldSignedPower(SDBVMSource source, real a,
                          real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Continuous),
          a_(a)
    {
        if (a_<=0.0 || a_>=1.0)
            PLERROR("Bad range for a (%f), must be in ]0,1[", a_);
    }
 
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
 
    virtual int fieldWidth() const;
 
protected:
    real a_;
};


/*!   Apply a one-input func to the field: call operator()(const Vec& input)
  of the func, with the input vector set to the single value resulting
  from the conversion of the SDB field to a Real. The vector output by the
  func is kept completely in the output; in other words, fieldWidth() is
  equal to the size of the vector returned by the func.
*/
class SDBVMFieldFunc1 : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
    
public: 
    SDBVMFieldFunc1(SDBVMSource source, Func func, 
                    real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Continuous), func_(func) {}
   
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;

protected:
    mutable Func func_;
};


/*!   Apply a two-input func to the field: call
  operator()(const Vec& input1, const Vec& input2)
  of the func, with the input vectors set to the single value resulting
  from the conversion of each SDB field to a Real. Since the two-argument
  operator() in Func returns a real, the width of this field in the
  VMatrix is 1.
*/
class SDBVMFieldFunc2 : public SDBVMFieldSource2
{
    typedef SDBVMFieldSource2 inherited;
    
public: 
    SDBVMFieldFunc2(SDBVMSource source1, SDBVMSource source2, Func func,
                    real missing_values_mapping=MISSING_VALUE)
        : inherited(source1, source2,missing_values_mapping,VMField::Continuous), func_(func) {}
   
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;

protected:
    mutable Func func_;
};



//#####  Date/Day Processing  #################################################

//!  Convert a date to fill 3 columns in the VMat: YYYY, MM, DD
class SDBVMFieldDate : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
    
public: 
    SDBVMFieldDate(SDBVMSource source, real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Date) {}
   
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;
};

/*!   Convert a date according to the formula:
  ((year - 1990)*365+(month-1)*30+(day-1))/3650
  which is approximately in the range [-1,1] for (1980-2000)
*/
class SDBVMFieldDay : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
    
public: 
    SDBVMFieldDay(SDBVMSource source,real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Continuous) {}
   
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;
};




//!  Computed year*12+(month-1)
class SDBVMFieldMonths : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;
    
public: 
    SDBVMFieldMonths(SDBVMSource source,real missing_values_mapping=MISSING_VALUE)
        : inherited(source,missing_values_mapping,VMField::Continuous) {}
   
    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;
};


//!  difference between two dates ("source1-source2" expressed as an integer number of days, months, or years)
class SDBVMFieldDateDiff : public SDBVMFieldSource2
{
protected:
    PDate refdate;
    FieldValue date1_threshold_;
    FieldValue date2_threshold_;
    char unit; //!<  unit can be 'D' for days, 'M' for months or 'Y' for years

public:
    //!  unit can be 'D' for days, 'M' for months or 'Y' for years
    //!  (result is always an integer number of days, months, or years)

/*!     This will compute difference between dates in source1 and source2 The
  sources must both be dates.  Optionally, two thresholds can be
  specified, such that if a date is below or equal to its respective
  threshold, then the date is considered MISSING_VALUE.  This is useful
  to filter out some obvious outliers.
    
  (TO BE IMPLEMENTED: or one can be a date and the other an integer
  numerical type; in this latter case, the integer is understood as
  beeing expressed in the given unit...)
*/
    SDBVMFieldDateDiff(SDBVMSource source1, SDBVMSource source2,
                       char the_unit = 'D',
                       FieldValue date1_threshold = FieldValue(),
                       FieldValue date2_threshold = FieldValue())
        : SDBVMFieldSource2(source1,source2,MISSING_VALUE,VMField::Continuous),
          refdate(), date1_threshold_(date1_threshold),
          date2_threshold_(date2_threshold), unit(the_unit) {}

    //!  This will compute difference between date in source1 and a fixed refdate
    SDBVMFieldDateDiff(SDBVMSource source1,
                       PDate the_refdate,
                       char the_unit = 'D')
        : SDBVMFieldSource2(source1,source1,MISSING_VALUE,VMField::Continuous),
          refdate(the_refdate),
          date1_threshold_(), date2_threshold_(), unit(the_unit) {}

    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;
};


//#####  Discretes and Mappings  ##############################################

//!  A field that recodes its source field according to an OutputCoder
//!  object.  
class SDBVMFieldDiscrete : public SDBVMFieldSource1
{
    typedef SDBVMFieldSource1 inherited;

public:
/*!       All constructors now specify the number of classes (between 0 and
  n-1), and whether one_hot output coding is desired (true) or direct
  integer codes (false, the default).  By default, num_classes is not
  known at construction time, and must be set by derived classes.  A
  remapping for missing values can also be provided.
*/
    SDBVMFieldDiscrete(SDBVMSource source,
                       int num_classes = 0,
                       real missing_values_mapping = MISSING_VALUE,
                       SDBVMOutputCoding oc = SDBVMNumeric,
                       VMField::FieldType ft = VMField::DiscrGeneral);

    //!  Version of the constructor that takes an OutputCoder object
    SDBVMFieldDiscrete(SDBVMSource source,
                       SDBVMOutputCoder* oc,
                       int num_classes = 0,
                       real missing_values_mapping = MISSING_VALUE,
                       VMField::FieldType ft = VMField::DiscrGeneral);

/*!       This function is implemented as a call to a virtual function
  getDiscreteValue(), followed by a call to setOutput of the
  OutputCoder.
*/
    virtual void convertField(const SDBWithStats& sdb, const Row& row, const Vec& output) const;

    virtual int fieldWidth() const;

    virtual SDBVMOutputCoding getOutputCoding() const;

/*!       This function must be overridden in derived classes to get the
  correctly-mapped discrete value obtained from the SDB.  The semantics
  prescribe missing values to be remapped to missing_values_mapping_.
*/
    virtual real getDiscreteValue(const SDBWithStats& sdb, const Row& row) const = 0;

    //!  Return the number of possible discrete values from 0 to N-1
    int getNumClasses() const { return num_classes_; }

    //!  Call this function to set the number of classes (don't play with
    //!  num_classes_ directly ==> important)
    void setNumClasses(int);

    //!  Set the currently-active output coder (captures ownership of passed
    //!  object)
    void setOutputCoder(SDBVMOutputCoder* oc) { output_coder_ = oc; }
  

protected:
    int num_classes_;			     //!<  must be >0 for one-hot coding
    PSDBVMOutputCoder output_coder_;	     //!<  perform actual formatting
};

typedef PP<SDBVMFieldDiscrete> PSDBVMFieldDiscrete;

//!  verifies if the date within the row is greater than a threshold date
class SDBVMFieldDateGreater : public SDBVMFieldDiscrete
{
    typedef SDBVMFieldDiscrete inherited;

protected:
    PDate ref;
  
public:
    SDBVMFieldDateGreater(SDBVMSource source, PDate the_ref)
        : inherited(source),ref(the_ref)
    {
        setNumClasses(2);		     //!<  true or false
    }

    virtual void convertField(const SDBWithStats& sdb, const Row& row,
                              const Vec& output) const;
    
    virtual int fieldWidth() const;

    virtual real getDiscreteValue(const SDBWithStats& sdb, const Row& row) const;
};

/*!   Code a field coming from the SDB "as-is", without any modifications or
  remapping (except missing-value remapping).  If one-hot coding is
  desired, a number of classes > 0 is required to be specified, and the
  SDB is trusted to only contain integers between 0 and num_classes-1.
  (If a value outside this bound is encountered, an error is reported.)
*/
class SDBVMFieldCodeAsIs : public SDBVMFieldDiscrete
{
    typedef SDBVMFieldDiscrete inherited;

public:
    SDBVMFieldCodeAsIs(SDBVMSource source,
                       int num_classes = 0,
                       real missing_values_mapping = MISSING_VALUE,
                       SDBVMOutputCoding oc = SDBVMNumeric,
                       VMField::FieldType ft = VMField::DiscrGeneral)
        : inherited(source, num_classes, missing_values_mapping, oc, ft) {}

    //!  Perform the actual remapping
    virtual real getDiscreteValue(const SDBWithStats& sdb, const Row& row) const;
};


/*!   A field that remaps a finite set of real values onto discrete integer
  values in the range 0 -- n-1. The mapping provided in the
  constructor has the format:
  "originalvalue1 newvalue1  original2 newvalue2  ... "
  meaning that when the SDB field takes value originalvalueX then
  the VMatrix corresponding column will have the value newvalueX.
  If none of the "original values" is seen then the "other_values_mapping"
  value is obtained. If the SDB contains a MISSING_VALUE then
  the "missing_values_mapping" is given.
*/
class SDBVMFieldRemapReals : public SDBVMFieldDiscrete
{
    typedef SDBVMFieldDiscrete inherited;
    typedef map<real,real> RealMap;

public:
    //!  This constructor accepts an explicitly-specified mappings string
    SDBVMFieldRemapReals(SDBVMSource source,
                         const string& mappings,
                         real other_values_mapping = MISSING_VALUE,
                         real missing_values_mapping = MISSING_VALUE,
                         SDBVMOutputCoding oc = SDBVMNumeric,
                         VMField::FieldType ft = VMField::DiscrGeneral);

/*!       This constructor accepts mappings coming from a FieldStat.  Note
  that the "hasmissing" field of the FieldStat is not automatically
  taken into account, and you must provide a missing_values_mapping
  here by yourself.  The mapping is not modified in any way by the
  presence of missing values.
*/
    SDBVMFieldRemapReals(SDBVMSource source,
                         const FieldStat& field_stat,
                         real other_values_mapping = MISSING_VALUE,
                         real missing_values_mapping = MISSING_VALUE,
                         SDBVMOutputCoding oc = SDBVMNumeric,
                         VMField::FieldType ft = VMField::DiscrGeneral);

    //!  Perform the actual remapping
    virtual real getDiscreteValue(const SDBWithStats& sdb, const Row& row) const;
    
    //!  This parses the remapping string and returns a map.  The maximum
    //!  target value in the map is set into the last (reference) argument.
    static RealMap getRealMapping(const string& mappings);

protected:
    RealMap real_mapping_;
    real other_values_mapping_;
};


/*!   A field that remaps a finite set of strings onto discrete integers.
  The format of "mappings" is a string of the form:
  "string_0 value_0  string_1 value_1  ...  string_N value_N"
  where the string_i cannot contain spaces, and the space is the delimiter
  between strings and values.
  If none of the "string_X" is seen then the "other_values_mapping"
  value is obtained. If the SDB contains a MISSING_VALUE then
  the "missing_values_mapping" is given.
*/
class SDBVMFieldRemapStrings : public SDBVMFieldDiscrete
{
    typedef SDBVMFieldDiscrete inherited;
    typedef map<string,real> StringMap;

public:
    //!  This constructor accepts an explicitly-specified mappings string
    SDBVMFieldRemapStrings(SDBVMSource source,
                           const string& mappings,
                           real other_values_mapping = MISSING_VALUE,
                           real missing_values_mapping = MISSING_VALUE,
                           SDBVMOutputCoding oc = SDBVMNumeric,
                           VMField::FieldType ft = VMField::DiscrGeneral);

/*!       This constructor accepts mappings coming from a FieldStat.  Note
  that the "hasmissing" field of the FieldStat is not automatically
  taken into account, and you must provide a missing_values_mapping
  here by yourself.  The mapping is not modified in any way by the
  presence of missing values.
*/
    SDBVMFieldRemapStrings(SDBVMSource source,
                           const FieldStat& field_stat,
                           real other_values_mapping = MISSING_VALUE,
                           real missing_values_mapping = MISSING_VALUE,
                           SDBVMOutputCoding oc = SDBVMNumeric,
                           VMField::FieldType ft = VMField::DiscrGeneral);

    //!  Perform the actual remapping
    virtual real getDiscreteValue(const SDBWithStats& sdb, const Row& row) const;
    
    //!  This parses the remapping string and returns a map.  The maximum
    //!  target value in the map is set into the last (reference) argument.
    static StringMap getStringMapping(const string& mappings);
    
protected:
    StringMap string_mapping_;
    real other_values_mapping_;
};


/*!   A field that remaps intervals on the reals onto discrete integer values.
  The format of the "mappings" argument is:
  "y_0 x_1 y_1 x_2 y_2 ... x_N y_N",
  where the "x_i" are cut-points for X, with for the value y_i obtained
  when x_i <= X < x_{i+1}, and y_0 obtained for X<x_1, and y_N obtained
  for X >= y_N. It is required that x_i < x_{i+1}.
*/
class SDBVMFieldRemapIntervals : public SDBVMFieldDiscrete
{
    typedef SDBVMFieldDiscrete inherited;

public:
    SDBVMFieldRemapIntervals(SDBVMSource source,
                             const string& mappings,
                             real other_values_mapping = MISSING_VALUE,
                             real missing_values_mapping = MISSING_VALUE,
                             SDBVMOutputCoding oc = SDBVMNumeric,
                             VMField::FieldType ft = VMField::DiscrGeneral);

    //!  Perform the actual remapping
    virtual real getDiscreteValue(const SDBWithStats& sdb, const Row& row) const;
    
/*!       This parses the remapping string and returns a map. Whether the
  target values are all non-negative integers is set in all_int.  The
  maximum target value in the map is set into max_of_map.  The
  intervals are returned in intervals_x and intervals_y
*/
    static void getIntervals(const string& mappings,
                             bool& all_int,    real& max_of_map,
                             Vec& intervals_x, Vec& intervals_y);
    
protected:
    Vec intervals_x_;
    Vec intervals_y_;
    real other_values_mapping_;
};


//#####  Multi Discrete  ######################################################

/*!   A field that maps multiple discrete fields onto a single discrete value.
  The resulting value is determined in a like manner to indexing into a
  multidimensional array.  For example, suppose we have three base fields:
  x1, x2 and x3.  x1 can take 5 values (0 to 4), x2 can take 10 values,
  and x3 can take 2 values. Then the resulting discrete value is:
*/
//	x1*(5*2) + x2*2 + x3
//!  In general, if there are N fields, x_1...x_N, and each can take y_i
//!  values, then the discrete value is:
//	\sum_{i=1}^N x_i \product_{j=i+1}^N y_j
/*!   where \product_{j=N+1}^N is defined to be 1.
  
For convenience, this class inherites from SDBVMFieldDiscrete, but
does not use the inherited source_ member.
*/

typedef Array<PSDBVMFieldDiscrete> FieldArray;

class SDBVMFieldMultiDiscrete : public SDBVMFieldDiscrete
{
    typedef SDBVMFieldDiscrete inherited;

public:
/*!       The constructor accepts an array of discrete fields; this class
  assumes ownership of the fields; the array can be empty, and later set
  with setFields().
*/
    SDBVMFieldMultiDiscrete(const FieldArray& fields,
                            real missing_values_mapping = MISSING_VALUE,
                            SDBVMOutputCoding oc = SDBVMNumeric,
                            VMField::FieldType ft = VMField::DiscrGeneral);

    //!  Accessor and mutator for field array
    const FieldArray& getFields() const {
        return fields_;
    }

    void setFields(const FieldArray& fields);
    //!     virtual int fieldWidth() const;

  
    //!  Compute the "array index" corresponding to the fields
    virtual real getDiscreteValue(const SDBWithStats& sdb, const Row& row) const;

protected:
    FieldArray fields_;
    Vec field_multipliers_;		     //!<  factor by which to multiply
    //!   each variable for indexing
};


//#####  ICBC Stuff  ##########################################################

/*!   This class is specialized for processing an ICBC database with
  the following fields: 
  "policy_start_date";
  "policy_end_date";
  "bodily_injury_incurred";
  "property_damage_incurred";
  "accident_death_incurred";
  "collision_lou_incurred";
  "comprehensive_incurred";
  "roadstar_incurred";
  
  The output has at least 1 value and at most 8 values (number of outputs =
  targetname=="ALL"?5:1 + int(use_roadstar) + int(add_claims_sum_column) + int(rescale_by_interval)).
  targetname can be "ALL" or one of the 5 other targets (excluding "roadstar_incurred"), or sum_all_but_BI.
  There are targetname=="ALL"?5:1 + int(use_roadstar) "*_incurred" targets + possibly one target 
  corresponding to the sum of all the "*_incurred" targets (if add_claims_sum_column==true)
  and possibly one "weight" if rescale_by_interval==true. If targetname=="sum_all_but_BI"
  then there is only one target, which is the sum of all 4 KOLs excluding Bodily Injury.
  
  The weight is computed as follows:
  duration = Year(policy_end_date)-Year(policy_start_date) 
  weight = duration^2
  
  The 5 + int(use_roadstar) targets are computed as follows:
  *_incurred_target = 0.001 * *_incurred;
  if (rescale_by_interval)
  *_incurred_target /= duration
  if (rescale_by_start_date)
  *_incurred_target /= smoothed *_incurred monthly mean for month m and year y
  where m and y are the month and year of the policy start date of the contract.
  (PS: for more information on the smoothed monthly means, read the comments
  in PLearn/Databases/ICBCdatabases.h)
  if is_missing(X_incurred_target) then X_incurred_target = 0
  
  where Year is the date given in units of years (with fractional part),
  so that in the common case where the difference is one year, the weight
  is 1 and there is no normalization of the targets.
  Note that the target outputs are in UNITS OF 1000 DOLLARS PER YEAR.
  
  This "weight" is the weight that should be given to the squared
  loss (f(x)-normalized_target)^2. This comes from the following reasoning:
  
  We are to learn a function f(x) that predicts the incurred amounts
  for ONE YEAR, but the (x,target) pairs in the data may cover more or
  less than one year. So the loss should be
  (f(x)*duration - target)^2
  where duration is in units of years. To make it more "standard" we
  instead consider the weighted squared loss, since
  (f(x)*duration - target)^2 = duration^2 * (f(x) - target/duration)^2
  = w * (f(x) - normalized_target)^2
  
  When add_claims_sum_column==true
  sum of targets = sum of all the *_incurred targets after they have been rescaled
  
  PS: * Year is the date given in units of years (with fractional part),
  so that in the common case where the difference is one year, the weight
  is 1 and there is no normalization of the targets.
  * Note that the target outputs are in UNITS OF 1000 DOLLARS PER YEAR.
  * When rescale_by_interval==true, the "weight" is the weight that should be given 
  to the squared loss (f(x)-normalized_target)^2. This comes from the following reasoning:
  We are to learn a function f(x) that predicts the incurred amounts
  for ONE YEAR, but the (x,target) pairs in the data may cover more or
  less than one year. So the loss should be
  (f(x)*duration - target)^2
  where duration is in units of years. To make it more "standard" we
  instead consider the weighted squared loss, since
  (f(x)*duration - target)^2 = duration^2 * (f(x) - target/duration)^2
  =  w * (f(x) - normalized_target)^2
  
*/
class SDBVMFieldICBCTargets : public SDBVMField
{
    typedef SDBVMField inherited;
    
public:
    SDBVMFieldICBCTargets(Schema schema, bool use_roadstar,
                          bool add_claims_sum_column, bool rescale_by_interval,
                          bool rescale_by_start_date, Mat& start_date_rescaling_values,
                          const string& targetname="ALL");

    virtual void convertField(const SDBWithStats& sdb, const Row& row, const Vec& output) const;

    virtual int fieldWidth() const {
        if (targetname_=="ALL")
            return 5+int(use_roadstar_)+int(add_claims_sum_column_)+int(rescale_by_interval_); 
        else if (targetname_=="sum_all_but_BI")
            return 1;
        else if (targetname_=="ALLcounts")
            return 5;
        else if (targetname_=="all_counts_but_BI")
            return 4;
        else if (targetname_=="ALLseverities")
            return 10;
        else if (targetname_=="all_severities_but_BI")
            return 8;
        else
            return 1+int(use_roadstar_)+int(add_claims_sum_column_)+int(rescale_by_interval_);}

protected:
    bool use_roadstar_;
    bool add_claims_sum_column_;
    bool rescale_by_interval_;
    bool rescale_by_start_date_;
    Mat start_date_rescaling_values_;
    string targetname_;
    int reference_start_date_year_;
    int reference_start_date_month_;
    SDBVMSource start_date_;
    SDBVMSource end_date_;
    SDBVMSource bodily_injury_incurred_;
    SDBVMSource property_damage_incurred_;
    SDBVMSource accident_death_incurred_;
    SDBVMSource collision_lou_incurred_;
    SDBVMSource comprehensive_incurred_;
    //SDBVMSource roadstar_incurred_;
    SDBVMSource bodily_injury_count_;
    SDBVMSource property_damage_count_;
    SDBVMSource accident_death_count_;
    SDBVMSource collision_lou_count_;
    SDBVMSource comprehensive_count_;

    SDBVMSource bodily_injury_severity_;
    SDBVMSource property_damage_severity_;
    SDBVMSource accident_death_severity_;
    SDBVMSource collision_lou_severity_;
    SDBVMSource comprehensive_severity_;
};


class SDBVMFieldHasClaim : public SDBVMField
{
    typedef SDBVMField inherited;
    
public:
    SDBVMFieldHasClaim(Schema schema):
        inherited(0),
        bodily_injury_incurred_(schema("bodily_injury_incurred")),
        property_damage_incurred_(schema("property_damage_incurred")),
        accident_death_incurred_(schema("accident_death_incurred")),
        collision_lou_incurred_(schema("collision_lou_incurred")),
        comprehensive_incurred_(schema("comprehensive_incurred")),
        roadstar_incurred_(schema("roadstar_incurred"))
    {}
    virtual void convertField(const SDBWithStats& sdb, const Row& row, Vec& output) const;
    virtual int fieldWidth() const { return 1;}
  
protected:
    FieldPtr bodily_injury_incurred_;
    FieldPtr property_damage_incurred_;
    FieldPtr accident_death_incurred_;
    FieldPtr collision_lou_incurred_;
    FieldPtr comprehensive_incurred_;
    FieldPtr roadstar_incurred_;
};


class SDBVMFieldSumClaims : public SDBVMField
{
    typedef SDBVMField inherited;
    
public:
    SDBVMFieldSumClaims(Schema schema):
        inherited(0),
        bodily_injury_incurred_(schema("bodily_injury_incurred")),
        property_damage_incurred_(schema("property_damage_incurred")),
        accident_death_incurred_(schema("accident_death_incurred")),
        collision_lou_incurred_(schema("collision_lou_incurred")),
        comprehensive_incurred_(schema("comprehensive_incurred")),
        roadstar_incurred_(schema("roadstar_incurred"))
    {}
    virtual void convertField(const SDBWithStats& sdb,
                              const Row& row, Vec& output) const;
    virtual int fieldWidth() const { return 1;}
  
protected:
    FieldPtr bodily_injury_incurred_;
    FieldPtr property_damage_incurred_;
    FieldPtr accident_death_incurred_;
    FieldPtr collision_lou_incurred_;
    FieldPtr comprehensive_incurred_;
    FieldPtr roadstar_incurred_;
};

class SDBVMFieldICBCClassification : public SDBVMField
{
    typedef SDBVMField inherited;
    
public:
    SDBVMFieldICBCClassification(Schema schema, const string& fieldname="",
                                 const string& tmap_file="");

    virtual void convertField(const SDBWithStats& sdb,
                              const Row& row, const Vec& output) const;

    virtual int fieldWidth() const { return 1; }

protected:
    SDBVMSource bodily_injury_incurred_;
    SDBVMSource property_damage_incurred_;
    SDBVMSource accident_death_incurred_;
    SDBVMSource collision_lou_incurred_;
    SDBVMSource comprehensive_incurred_;
    FieldPtr policy_start_date_;
    string fieldname_;
    int threshold;
};


/*!   Given a vector of claims (representing the claim values for different KOLs)
  and a threshold value, the following function returns the class (integer 
  between 0 and 3) of the claims vector. There are 4 classes:
  - class 0: "neg" claim vector (i.e. at least one claim in the vector < 0 and the others = 0)
  - class 1: "zero" claim vector (i.e. all claims in the vector are 0)
  - class 2: "smallpos" claim vector (i.e. at least one claim in the vector > 0 and all claims < threshold)
  - class 3: "largepos" claim vector (i.e. at least one claim in the vector >= threshold)
*/
int ICBCpartition(const Vec& claims, real threshold);


//#####  Local Aliases Namespace  #############################################

/*!   Since it is extremely tedious to refer to the various fields as,
  e.g. SDBVMFieldRemapIntervals, this namespace provides a means to have
  local aliases at function scope only.  All the local aliases have a name
  of the form FRemapIntervals.  To enable them in a function, use:
  
  void foo()
  {
  using namespace SDBFields;
  ...
  }
*/

namespace SDBFields
{
typedef SDBVMSource			FSource;
typedef SDBVMField			FField;
typedef SDBVMFieldSource1		FSource1;
typedef SDBVMFieldSource2		FSource2;
typedef SDBVMFieldAsIs		FAsIs;
typedef SDBVMFieldNormalize		FNormalize;
typedef SDBVMFieldDivSigma		FDivSigma;
typedef SDBVMFieldAffine		FAffine;
typedef SDBVMFieldPosAffine		FPosAffine;
typedef SDBVMFieldSignedPower FSignedPower;
typedef SDBVMFieldFunc1		FFunc1;
typedef SDBVMFieldFunc2		FFunc2;
typedef SDBVMFieldDate		FDate;
typedef SDBVMFieldDateDiff		FDateDiff;
typedef SDBVMFieldDateGreater		FDateGreater;
typedef SDBVMFieldDay			FDay;
typedef SDBVMFieldDiscrete		FDiscrete;
typedef SDBVMFieldCodeAsIs		FCodeAsIs;
typedef SDBVMFieldRemapReals		FRemapReals;
typedef SDBVMFieldRemapStrings	FRemapStrings;
typedef SDBVMFieldRemapIntervals	FRemapIntervals;
typedef SDBVMFieldMultiDiscrete	FMultiDiscrete;
typedef SDBVMFieldICBCTargets	        FICBCTargets;
typedef SDBVMFieldHasClaim	        FHasClaim;
typedef SDBVMFieldSumClaims	        FSumClaims;
typedef SDBVMFieldICBCClassification	FICBCClassification;
}

} // end of namespace PLearn

#endif //!<  NGSDBVMAT_H


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
