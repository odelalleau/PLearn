// -*- C++ -*-

// SimpleDB.cc: Simple Database Representation (implementation)
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

#define _GNU_SOURCE 1

#include "SimpleDB.h"

namespace PLearn <%
using namespace std;
   

  //#####  Static Members  ####################################################

  const char MissingString = '\0';
  const unsigned char MissingCharacter = (unsigned char)SCHAR_MIN;
  const signed char MissingSignedChar = (signed char)SCHAR_MIN;
  const short MissingShort = SHRT_MIN;
  const int MissingInt = INT_MIN;
  const float MissingFloat = MISSING_VALUE;
  const double MissingDouble = MISSING_VALUE;
  const PDate MissingDate;		     // default ctor ==> missing date


  //#####  Schema-related functions  ##########################################

  bool Schema::findColumn(const string& name, int& position, int& start,
			  int& precision) const
  {
    const_iterator it = begin(), end = this->end();
    position = start = precision = 0;

    for (; it != end; start += it->precision, ++it, ++position)
      if (it->name == name) {
	precision = it->precision;
	break;
      }
    return (it == end)? false : true;
  }


  FieldPtr Schema::findColumn(int position) const
  {
    int orig_position = position;
    ptrdiff_t offset_= 0;
    const_iterator it=begin(), end=this->end();
    for (; position && it != end; --position, ++it)
      offset_+= it->precision;
    if (it == end)
      PLERROR("Column %d does not exist in schema",
	    orig_position);
    return FieldPtr(orig_position, offset_);
  }


  FieldPtr Schema::findColumn(const string& name) const
  {
    int position, start, precision;
    bool found = findColumn(name, position, start, precision);
    if (!found)
      PLERROR("Column %s does not exist in schema",
	    name.c_str());
    return FieldPtr(position, start);
  }

  
  //#####  FieldValue  ########################################################

  FieldValue::FieldValue()
    : field_type_(Unknown), precision_(0)
  {}

  FieldValue::FieldValue(const FieldValue& fv)
    : field_type_(fv.field_type_), precision_(fv.precision_)
  {
    switch (field_type_) {
    case Unknown:
      break;

    case StringType:
    {
      int stringlen = strlen(fv.string_val_);
      if (stringlen+1 != precision_)
	PLERROR("Strings in a database field must contain a terminating null");
      string_val_ = new char[precision_];
      strcpy(string_val_, fv.string_val_);
    }
    break;

    case CharacterType:
    case SignedCharType:
    case ShortType:
    case IntType:
      long_val_ = fv.long_val_;
      break;

    case FloatType:
    case DoubleType:
      double_val_ = fv.double_val_;
      break;

    case DateType:
      date_val_ = fv.date_val_;
      break;
    }
  }
  
  FieldValue::~FieldValue()
  {
    switch(field_type_) {
    case StringType:
      delete[] string_val_;

    case Unknown:
    case CharacterType:
    case SignedCharType:
    case ShortType:
    case IntType:
    case FloatType:
    case DoubleType:
    case DateType:
      break;
    }
  }
  
  FieldValue::FieldValue(const char* str)
    : field_type_(StringType), precision_(strlen(str)+1)
  {
    string_val_ = new char[precision_];
    strcpy(string_val_, str);
  }

  FieldValue::FieldValue(unsigned char x)
    : field_type_(CharacterType),
      precision_(Field("",CharacterType).precision),
      long_val_(x)
  {}
  
  FieldValue::FieldValue(signed char x)
    : field_type_(SignedCharType),
      precision_(Field("",SignedCharType).precision),
      long_val_(x)
  {}
  
  FieldValue::FieldValue(short x)
    : field_type_(ShortType),
      precision_(Field("",ShortType).precision),
      long_val_(x)
  {}
  
  FieldValue::FieldValue(int x)
    : field_type_(IntType),
      precision_(Field("",IntType).precision),
      long_val_(x)
  {}
  
  FieldValue::FieldValue(float x)
    : field_type_(FloatType),
      precision_(Field("",FloatType).precision),
      double_val_(x)
  {}
  
  FieldValue::FieldValue(double x)
    : field_type_(DoubleType),
      precision_(Field("",DoubleType).precision),
      double_val_(x)
  {}
  
  FieldValue::FieldValue(const PDate& x)
    : field_type_(DateType),
      precision_(Field("",DateType).precision)
  {
    date_val_.year  = x.year;
    date_val_.month = x.month;
    date_val_.day   = x.day;
  }

  bool FieldValue::isMissing() const
  {
    switch (field_type_) {
    case Unknown:	return true;
    case StringType:	return !string_val_ || string_val_[0] == MissingString;
    case CharacterType:	return (unsigned char)long_val_ == MissingCharacter;
    case SignedCharType:return (signed char)  long_val_ == MissingSignedChar;
    case ShortType:	return (short)        long_val_ == MissingShort;
    case IntType:	return (int)          long_val_ == MissingInt;
    case FloatType:
    case DoubleType:	return isnan(double_val_);
    case DateType:
      return date_val_.year == MissingDate.year &&
	date_val_.month == MissingDate.month &&
	date_val_.day   == MissingDate.day;
    }
    return false;
  }   

  void FieldValue::setMissing()
  {
    switch (field_type_) {
    case Unknown:	break;
    case StringType:	if (string_val_) string_val_[0] = MissingString; break;
    case CharacterType:	long_val_ = long(MissingCharacter);	break;
    case SignedCharType:long_val_ = long(MissingSignedChar);	break;
    case ShortType:	long_val_ = long(MissingShort);		break;
    case IntType:	long_val_ = long(MissingInt);		break;
    case FloatType:
    case DoubleType:	double_val_ = MissingDouble;		break;
    case DateType:
      date_val_.year  = MissingDate.year;
      date_val_.month = MissingDate.month;
      date_val_.day   = MissingDate.day;
      break;
    }
  }
  
  string FieldValue::toString() const
  {
    if (isMissing())
      return "";
    switch (field_type_) {
    case Unknown:	return "";
    case StringType:	return space_to_underscore(string_val_);
    case CharacterType:	return string(1,char(long_val_));
    case SignedCharType:
    case ShortType:
    case IntType:	return tostring(long_val_);
    case FloatType:
    case DoubleType:	return tostring(double_val_);
    case DateType:
      return PDate(date_val_.year, date_val_.month, date_val_.day).info();
    }
    return "";
  }
  
  double FieldValue::toDouble() const
  {
    if (isMissing())
      return MISSING_VALUE;
    switch (field_type_) {
    case Unknown:
      return MISSING_VALUE;
      
    case StringType:
    case CharacterType:
      PLERROR("Cannot convert string or character field to double");
      break;

    case SignedCharType:
    case ShortType:
    case IntType:
      return double(long_val_);

    case FloatType:
    case DoubleType:
      return double_val_;

    case DateType:
      return double(date_to_float(PDate(date_val_.year, date_val_.month,
					date_val_.day)));
    }
    return MISSING_VALUE;
  }

  PDate FieldValue::toDate() const
  {
    switch(field_type_) {
    case DateType:
      return PDate(date_val_.year, date_val_.month, date_val_.day);

    default:
      PLERROR("Cannot convert non-date field type to a date");
    }
    return PDate();
  }  
  
  FieldValue& FieldValue::operator=(FieldValue rhs)
  {
    swap(rhs);
    return *this;
  }

  bool FieldValue::operator==(const FieldValue& rhs) const
  {
    const FieldValue& lhs = *this;
    const FieldType& lhs_type = field_type_;
    const FieldType& rhs_type = rhs.field_type_;

    // Strings
    if (lhs_type == StringType && rhs_type == StringType)
      return !strcmp(lhs.string_val_, rhs.string_val_);
    else if (lhs_type == StringType || rhs_type == StringType)
      PLERROR("A string can be compared for equality only with another string");

    // Dates
    else if (lhs_type == DateType && rhs_type == DateType)
      return
	PDate(lhs.date_val_.year, lhs.date_val_.month, lhs.date_val_.day) ==
	PDate(rhs.date_val_.year, rhs.date_val_.month, rhs.date_val_.day);
    else if (lhs_type == DateType || rhs_type == DateType)
      PLERROR("A date can be compared for equality only with another date");

    // Two integrals
    else if (lhs.isIntegral() && rhs.isIntegral())
      return lhs.long_val_ == rhs.long_val_;

    // Two floating
    else if (lhs.isFloating() && rhs.isFloating())
      return lhs.double_val_ == rhs.double_val_;

    // Cross-numeric
    else if (lhs.isIntegral() && rhs.isFloating())
      return lhs.long_val_ == rhs.double_val_;
    else if (lhs.isFloating() && lhs.isIntegral())
      return lhs.double_val_ == rhs.long_val_;

    // Otherwise, PLERROR(should not happen)
    else
      PLERROR("Unrecognized case in equality testing between FieldValues");

    return false;			     // shut up the compiler
  }

  bool FieldValue::operator<(const FieldValue& rhs) const
  {
    const FieldValue& lhs = *this;
    const FieldType& lhs_type = field_type_;
    const FieldType& rhs_type = rhs.field_type_;

    // Strings
    if (lhs_type == StringType && rhs_type == StringType)
      return strcmp(lhs.string_val_, rhs.string_val_) < 0;
    else if (lhs_type == StringType || rhs_type == StringType)
      PLERROR("A string can be relationally compared only with another string");

    // Dates
    else if (lhs_type == DateType && rhs_type == DateType)
      return
	PDate(lhs.date_val_.year, lhs.date_val_.month, lhs.date_val_.day) <
	PDate(rhs.date_val_.year, rhs.date_val_.month, rhs.date_val_.day);
    else if (lhs_type == DateType || rhs_type == DateType)
      PLERROR("A date can be relationally compared only with another date");

    // Two integrals
    else if (lhs.isIntegral() && rhs.isIntegral())
      return lhs.long_val_ < rhs.long_val_;

    // Two floating
    else if (lhs.isFloating() && rhs.isFloating())
      return lhs.double_val_ < rhs.double_val_;

    // Cross-numeric
    else if (lhs.isIntegral() && rhs.isFloating())
      return lhs.long_val_ < rhs.double_val_;
    else if (lhs.isFloating() && lhs.isIntegral())
      return lhs.double_val_ < rhs.long_val_;

    // Otherwise, PLERROR(should not happen)
    else
      PLERROR("Unrecognized case in relational testing between FieldValues");

    return false;			     // shut up the compiler
  }
  
  FieldValue FieldValue::operator+(const FieldValue& rhs) const
  {
    const FieldValue& lhs = *this;
    const FieldType& lhs_type = field_type_;
    const FieldType& rhs_type = rhs.field_type_;

    // Arithmetic addition is not defined for strings or dates or characters
    if (lhs_type == StringType || rhs_type == StringType)
      PLERROR("Strings cannot be added");
    else if (lhs_type == CharacterType || rhs_type == CharacterType)
      PLERROR("Characters cannot be added");
    else if (lhs_type == DateType || rhs_type == DateType)
      PLERROR("Dates cannot be added");

    // Twice the same type
    else if (lhs.isIntegral() && rhs.isIntegral())
      return FieldValue(int(lhs.long_val_ + rhs.long_val_));
    else if (lhs.isFloating() && rhs.isFloating())
      return FieldValue(double(lhs.double_val_ + rhs.double_val_));

    // Cross-numeric : convert to double
    else if (lhs.isIntegral() && rhs.isFloating())
      return FieldValue(double(lhs.long_val_ + rhs.double_val_));
    else if (lhs.isFloating() && rhs.isIntegral())
      return FieldValue(double(lhs.double_val_ + rhs.long_val_));

    // Otherwise, PLERROR(should not happen)
    else
      PLERROR("Unrecognized case in addition between FieldValues");

    return FieldValue();
  }
  
  FieldValue FieldValue::operator-(const FieldValue& rhs) const
  {
    const FieldValue& lhs = *this;
    const FieldType& lhs_type = field_type_;
    const FieldType& rhs_type = rhs.field_type_;

    // Arithmetic subtraction is not defined for strings or characters
    if (lhs_type == StringType || rhs_type == StringType)
      PLERROR("Strings cannot be subtracted");
    else if (lhs_type == CharacterType || rhs_type == CharacterType)
      PLERROR("Characters cannot be subtracted");

    // For dates, return the number of days between two dates
    else if (lhs_type == DateType && rhs_type == DateType)
      return FieldValue(int(
	PDate(lhs.date_val_.year, lhs.date_val_.month, lhs.date_val_.day) -
	PDate(rhs.date_val_.year, rhs.date_val_.month, rhs.date_val_.day)));
    else if (lhs_type == DateType || rhs_type == DateType)
      PLERROR("A date and a non-date cannot be subtracted");

    // Twice the same type
    else if (lhs.isIntegral() && rhs.isIntegral())
      return FieldValue(int(lhs.long_val_ - rhs.long_val_));
    else if (lhs.isFloating() && rhs.isFloating())
      return FieldValue(double(lhs.double_val_ - rhs.double_val_));

    // Cross-numeric : convert to double
    else if (lhs.isIntegral() && rhs.isFloating())
      return FieldValue(double(lhs.long_val_ - rhs.double_val_));
    else if (lhs.isFloating() && rhs.isIntegral())
      return FieldValue(double(lhs.double_val_ - rhs.long_val_));

    // Otherwise, PLERROR(should not happen)
    else
      PLERROR("Unrecognized case in subtraction between FieldValues");

    return FieldValue();
  }
  
  FieldValue FieldValue::operator*(const FieldValue& rhs) const
  {
    const FieldValue& lhs = *this;
    const FieldType& lhs_type = field_type_;
    const FieldType& rhs_type = rhs.field_type_;

    // Arithmetic addition is not defined for strings or dates or characters
    if (lhs_type == StringType || rhs_type == StringType)
      PLERROR("Strings cannot be multiplied");
    else if (lhs_type == CharacterType || rhs_type == CharacterType)
      PLERROR("Characters cannot be multiplied");
    else if (lhs_type == DateType || rhs_type == DateType)
      PLERROR("Dates cannot be multiplied");

    // Twice the same type
    else if (lhs.isIntegral() && rhs.isIntegral())
      return FieldValue(int(lhs.long_val_ * rhs.long_val_));
    else if (lhs.isFloating() && rhs.isFloating())
      return FieldValue(double(lhs.double_val_ * rhs.double_val_));

    // Cross-numeric : convert to double
    else if (lhs.isIntegral() && rhs.isFloating())
      return FieldValue(double(lhs.long_val_ * rhs.double_val_));
    else if (lhs.isFloating() && rhs.isIntegral())
      return FieldValue(double(lhs.double_val_ * rhs.long_val_));

    // Otherwise, PLERROR(should not happen)
    else
      PLERROR("Unrecognized case in multiplication between FieldValues");

    return FieldValue();
  }
  
  FieldValue FieldValue::operator/(const FieldValue& rhs) const
  {
    const FieldValue& lhs = *this;
    const FieldType& lhs_type = field_type_;
    const FieldType& rhs_type = rhs.field_type_;

    // Arithmetic addition is not defined for strings or dates or characters
    if (lhs_type == StringType || rhs_type == StringType)
      PLERROR("Strings cannot be divided");
    else if (lhs_type == CharacterType || rhs_type == CharacterType)
      PLERROR("Characters cannot be divided");
    else if (lhs_type == DateType || rhs_type == DateType)
      PLERROR("Dates cannot be divided");

    // Twice the same type
    else if (lhs.isIntegral() && rhs.isIntegral())
      return FieldValue(int(lhs.long_val_ / rhs.long_val_));
    else if (lhs.isFloating() && rhs.isFloating())
      return FieldValue(double(lhs.double_val_ / rhs.double_val_));

    // Cross-numeric : convert to double
    else if (lhs.isIntegral() && rhs.isFloating())
      return FieldValue(double(lhs.long_val_ / rhs.double_val_));
    else if (lhs.isFloating() && rhs.isIntegral())
      return FieldValue(double(lhs.double_val_ / rhs.long_val_));

    // Otherwise, PLERROR(should not happen)
    else
      PLERROR("Unrecognized case in division between FieldValues");

    return FieldValue();
  }

  void FieldValue::swap(FieldValue& rhs)
  {
    std::swap(field_type_, rhs.field_type_);
    std::swap(precision_,  rhs.precision_);
    switch(field_type_) {
    case Unknown:	break;
    case StringType:	std::swap(string_val_, rhs.string_val_); break;
    case CharacterType:
    case SignedCharType:
    case ShortType:
    case IntType:	std::swap(long_val_, rhs.long_val_); break;
    case FloatType:
    case DoubleType:	std::swap(double_val_, rhs.double_val_); break;
    case DateType:	std::swap(date_val_, rhs.date_val_); break;
    }
  }

  ostream& operator<<(ostream& os, const FieldValue& ft)
  {
    // quite frankly too simple for now
    return os << ft.toString();
  }
  

  //#####  Row-Iterator-Related functions  ####################################

  bool RowIterator::isMissing() const
  {
    if (const char* x = asString())
      return x[0] == MissingString;
    else if (const unsigned char* x = asCharacter())
      return x[0] == MissingCharacter;
    else if (const signed char* x = asSignedChar())
      return x[0] == MissingSignedChar;
    else if (const short* x = asShort())
      return *x == MissingShort;
    else if (const int* x = asInt())
      return *x == MissingInt;
    else if (const float* x = asFloat())
      return isnan(*x);
    else if (const double* x = asDouble())
      return isnan(*x);
    else if (const PDate* x = asDate())
      return *x == MissingDate;
    else
      return false;
  }

  void RowIterator::setMissing()
  {
    if (char* x = asString())
      *x = MissingString;
    else if (unsigned char* x = asCharacter())
      *x = MissingCharacter;
    else if (signed char* x = asSignedChar())
      *x = MissingSignedChar;
    else if (short* x = asShort())
      *x = MissingShort;
    else if (int* x = asInt())
      *x = MissingInt;
    else if (float* x = asFloat())
      *x = MissingFloat;
    else if (double* x = asDouble())
      *x = MissingDouble;
    else if (PDate* x = asDate())
      *x = MissingDate;
  }
    
  int RowIterator::char_width() const
  {
    int w = 0;
    if (isString())
      w = precision()-1;		     // minus terminating null
    else if (isCharacter())
      w = 1;				     // 'A'
    else if (isSignedChar())
      w = 4;				     // -127
    else if (isShort())
      w = 6;				     // -32767
    else if (isInt())
      w = 11;				     // -2 billion
    else if (isFloat())
      w = 8;				     // -precision + decimal point
    else if (isDouble())
      w = 8;				     // -precision + decimal point
    else if (isDate())
      w = 10;				     // YYYY/MM/DD
    else
      PLERROR("Unknown type for iterator, field %d (%s)",curfield,name().c_str());

    return std::max(int(w),int(name().size()));
  }

  double RowIterator::toDouble() const
  {
    if (isMissing())
      return MISSING_VALUE;
    if (asString())
      PLERROR("Cannot convert string to double");
    if (asCharacter())
      PLERROR("Cannot convert character to double");
    if (const signed char* x = asSignedChar())
      return double(*x);
    if (const short* x = asShort())
      return double(*x);
    if (const int* x = asInt())
      return double(*x);
    if (const float* x = asFloat())
      return double(*x);
    if (const double* x = asDouble())
      return *x;
    if (const PDate* x = asDate())
      return double(date_to_float(*x));    
    return MISSING_VALUE;
  }

  string RowIterator::toString() const
  {
    if (isMissing())
      return "";
    if (const char* x = asString())
      return space_to_underscore(x);
    if (const unsigned char* x = asCharacter())
      return string(1,char(*x));
    if (const signed char* x = asSignedChar())
      return tostring(int(*x));
    if (const short* x = asShort())
      return tostring(*x);
    if (const int* x = asInt())
      return tostring(*x);
    if (const float* x = asFloat())
      return tostring(*x);
    if (const double* x = asDouble())
      return tostring(*x);
    if (const PDate* x = asDate())
      return x->info();
    return "";
  }

  double todouble(const RowIterator& it)
  {
    return it.toDouble();
  }

  string tostring(const RowIterator& it)
  {
    return it.toString();
  }


  //#####  FieldRowRef  #########################################################

  FieldRowRef::operator FieldValue() const
  {
    if (const char* x = it_.asString())
      return FieldValue(x);
    if (const unsigned char* x = it_.asCharacter())
      return FieldValue(*x);
    if (const signed char* x = it_.asSignedChar())
      return FieldValue(*x);
    if (const short* x = it_.asShort())
      return FieldValue(*x);
    if (const int* x = it_.asInt())
      return FieldValue(*x);
    if (const float* x = it_.asFloat())
      return FieldValue(*x);
    if (const double* x = it_.asDouble())
      return FieldValue(*x);
    if (const PDate* x = it_.asDate())
      return FieldValue(*x);
    return FieldValue();
  }

  // This assignment operator is complicated by the fact that the LHS type
  // may not have anything to do with the RHS type.  Appropriate
  // conversions must be enacted.
  FieldRowRef& FieldRowRef::operator=(const FieldValue& rhs)
  {
    // Strings ==> convert anything into string form
    if (char* x = it_.asString()) {
      strncpy(x, rhs.toString().c_str(), it_.precision());
      x[it_.precision()-1] = '\0';
    }
    else if (unsigned char* x = it_.asCharacter()) {
      if (rhs.isIntegral())
	*x = (unsigned char)rhs.long_val_;
      else if (rhs.isFloating())
	*x = (unsigned char)rhs.double_val_;
      else
	PLERROR("Cannot convert a string or a date into an unsigned character");
    }
    else if (signed char* x = it_.asSignedChar()) {
      if (rhs.isIntegral())
	*x = (signed char)rhs.long_val_;
      else if (rhs.isFloating())
	*x = (signed char)rhs.double_val_;
      else
	PLERROR("Cannot convert a string or a date into a signed character");
    }
    else if (short* x = it_.asShort()) {
      if (rhs.isIntegral())
	*x = (short)rhs.long_val_;
      else if (rhs.isFloating())
	*x = (short)rhs.double_val_;
      else
	PLERROR("Cannot convert a string or a date into a short");
    }
    else if (int* x = it_.asInt()) {
      if (rhs.isIntegral())
	*x = (int)rhs.long_val_;
      else if (rhs.isFloating())
	*x = (int)rhs.double_val_;
      else
	PLERROR("Cannot convert a string or a date into an int");
    }
    else if (float* x = it_.asFloat()) {
      if (rhs.isIntegral())
	*x = (float)rhs.long_val_;
      else if (rhs.isFloating())
	*x = (float)rhs.double_val_;
      else
	PLERROR("Cannot convert a string or a date into a float");
    }
    else if (double* x = it_.asDouble()) {
      if (rhs.isIntegral())
	*x = (double)rhs.long_val_;
      else if (rhs.isFloating())
	*x = (double)rhs.double_val_;
      else
	PLERROR("Cannot convert a string or a date into a double");
    }
    else if (PDate* x = it_.asDate()) {
      if (rhs.isDate())
	*x = PDate(rhs.date_val_.year, rhs.date_val_.month,
		   rhs.date_val_.day);
      else
	PLERROR("Cannot convert a non-date into a date");
    }
    else
      PLERROR("Unrecognized case in assignment in FieldRowRef from FieldValue");

    return *this;
  }


  //#####  Row-Related functions  #############################################

  Row::Row(const Schema* s) : schema(s)
  {
    // Compute the total size of a row
    int n=0;
    Schema::const_iterator it = schema->begin(), end = schema->end();
    for ( ; it != end; ++it ) {
      n += it->precision;
    }
    rawrow.resize(n, '\0');		     // zero-initialize it
  }
    
  void Row::sanitize() const
  {
    // The sanitization operation canonicalizes all fields in the row.
    // Should be called before writing it to disk.  This enables indexing
    // and matching operations to find rows quickly simply by comparing
    // byte vectors.  At the moment, the only sanity check is to zero-fill
    // all character strings beyond the initial null until the precision of
    // their field.

    Row* This = const_cast<Row*>(this);
    iterator it = This->begin(), end = This->end();
    for ( ; it != end; ++it ) {
      if (char *x = it.asString()) {
	int prec = it.precision();
	bool clearing = false;
	for ( ; prec; ++x, --prec)
	  if (clearing)
	    *x = '\0';
	  else if (*x == '\0')
	    clearing = true;
      }
    }
  }

  Row::iterator Row::operator[](int fieldNumber)
  {
    iterator it=this->begin(), end=this->end();
    for (; fieldNumber && it != end; --fieldNumber, ++it)
      ;
    return it;
  }

  Row::iterator Row::operator[](string fieldName)
  {
    iterator it=this->begin(), end=this->end();
    Schema::const_iterator scit=schema->begin(), scend=schema->end();
    for(; it != end && scit != scend; ++it, ++scit)
      if (scit->name == fieldName)
	break;
    return it;
  }

  void printFieldName(ostream& o, const Row::iterator& field)
  {
    o.setf(ios::right, ios::adjustfield);
    o.fill(' ');
    o.width(field.char_width());
    o << field.name().c_str();
  }

  void printFieldNames(ostream& o, const Row& rowc)
  {
    Row& row = const_cast<Row&>(rowc);
    Row::const_iterator it = row.begin(), end = row.end();
    
    while(it!=end)
    {
      printFieldName(o,it);
      o << " | ";
      ++it;
    }
    o << endl;
  }

  ostream& operator<<(ostream& o, const Row::iterator& field)
  {
    o.setf(ios::right, ios::adjustfield);
    o.fill(' ');
    o.width(field.char_width());

    // cout << "[" << field.char_width() << "]" << endl;

    if (field.isMissing())
      o << " ";
    else if (const char* x = field.asString())
      o << x;
    else if (const unsigned char* x = field.asCharacter()) 
    {
      if (isprint(*x))
      {
	// couldn't get the formatting using ostream.width() to work so I'm
	// using this...
	o.width(0);
	o << center(string(1,*x),field.char_width()); 
      }
      else 
      {
	o.setf(ios::left, ios::adjustfield);
	o.width(0);
	o << "0x";
	o.width(field.char_width()-2);
	o << hex << int(*x) << dec;
	o.setf(ios::right, ios::adjustfield);
      }
    }
    else if (const signed char* x = field.asSignedChar())
      o << int(*x);
    else if (const short* x = field.asShort())
      o << *x;
    else if (const int* x = field.asInt())
      o << *x;
    else if (const float* x = field.asFloat()) 
    {
      o.setf(ios::fmtflags(0), ios::floatfield);
      o.precision(6);
      o << *x;
    }
    else if (const double* x = field.asDouble()) 
    {
      o.setf(ios::fmtflags(0), ios::floatfield);
      o.precision(6);
      o << *x;
    }
    else if (const PDate* x = field.asDate()) 
    {
      o.width(0);
      o << center(x->info(),field.char_width());
    }
    else
      PLERROR("Unknown field type");

    return o;
  }

  ostream& operator<<(ostream& o, const Row& rowc)
  {
    Row& row = const_cast<Row&>(rowc);
    Row::const_iterator it = row.begin(), end = row.end();
    
    while(it!=end)
    {
      o << it << " | ";
      ++it;
    }
    o << endl;
    return o;
  }

  // [Having a SDB with rows 0,...,n-1]:
  // swap(1,n-1)
  // swap(3,n-3)
  // ...
  // swap(n/2,n-n/2)
  // Not quite a random shuffle, but much more efficient 
  // than randomShuffleRows (better use of cache)
  void halfShuffleRows(SDB& sdb)
  {
    Row rowi(&sdb.getSchema());
    Row rowj(&sdb.getSchema());
    int length = int(sdb.length());
    for(int k=1; k<length/2; k+=2)
    {
      if(k%100000==1)
	cerr << k << endl;
      sdb.getInRow(k,rowi);
      sdb.getInRow(length-k,rowj);
      sdb.setRow(rowi,length-k);
      sdb.setRow(rowj,k);
    }
  }

  // extremely slow for huge databases: cannot use cache in an efficient way
  void randomShuffleRows(SDB& sdb)
  {
    Row rowi(&sdb.getSchema());
    Row rowj(&sdb.getSchema());
    int length = int(sdb.length());
    for(int i=0; i<sdb.length(); i++)
    {
      if(i%1000==0)
	cerr << i << endl;
      int j = i+int(uniform_sample()*(length-i));
      sdb.getInRow(i,rowi);
      sdb.getInRow(j,rowj);
      sdb.setRow(rowi,j);
      sdb.setRow(rowj,i);
    }
  }

%> // end of namespace PLearn
