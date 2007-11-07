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

/*! \file PLearn/plearn/db/SimpleDB.h */

#ifndef SIMPLEDB_H
#define SIMPLEDB_H


//!  From Standard Library
#include <limits.h>			     //!<  for LONG_MAX
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>         //!< for logic_error
#include <typeinfo>

#include <assert.h>

#ifndef WIN32
#include <unistd.h>			     //!<  for lseek, read, write, close
#else
#include <io.h>
#endif

#include <fcntl.h>			     //!<  for open
#include <errno.h>			     //!<  for errno
#include <cstdlib>			     //!<  for ptrdiff_t


//!  From PLearn
#include <plearn/base/stringutils.h>     //!< For slash.
#include <plearn/base/PDate.h>			     //!<  for PDate
#include <plearn/math/Hash.h>
#include <plearn/base/TinyVector.h>

#ifdef WIN32
// norman: potentially dangerous if there is a function called with the same name in this
//         file. Beware!
#define open _open
#define close _close
#define lseek _lseek
#define read _read
#define write _write
#define unlink _unlink
#endif

namespace PLearn {
using namespace std;

//!  Forward-declarations
class Row;
    

//##############################  CLASS  FIELD  ###########################
/*!     
  A schema is simply a vector of field definitions.  A field
  definition is a structure containing a field type and a precision.
  The meaning of precision is always the length in byte of the type.
    
  Type :			What Precision is:
*/
//		StringType		length of string
//					(including null terminator)
//		CharacterType		1 == unsigned char
//		SignedCharType		1 == signed char
//		ShortType		2 == signed short
//		IntType			4 == signed int
//		FloatType		4 == float
//		DoubleType		8 == double
//		DateType		4 == PDate
enum FieldType {
    Unknown = 0,
    StringType,
    CharacterType,
    SignedCharType,
    ShortType,
    IntType,
    FloatType,
    DoubleType,
    DateType
};

struct Field {
    Field() : name(), field_type(Unknown), precision() {}
    Field(string name, FieldType t, int p = 0)
        : name(name), field_type(t)
    {
        //!  We listen to the precision argument solely when the
        //!  field type is a string
        switch(field_type) {
        case Unknown:		precision=0;	break;
        case StringType:	precision=p;	break;
        case CharacterType:	precision=1;	break;
        case SignedCharType:	precision=1;	break;
        case ShortType:		precision=2;	break;
        case IntType:		precision=4;	break;
        case FloatType:		precision=4;	break;
        case DoubleType:	precision=8;	break;
        case DateType:		precision=4;	break;
        default:
            PLERROR("Unknown field type %d with name %s",
                    int(field_type), name.c_str());
        }
        if (sizeof(PDate) != 4)
            PLERROR("A PLearn PDate must have sizeof equal to 4");
    }

    bool operator==(const Field& x) const {
        return name==x.name &&
            field_type == x.field_type &&
            precision == x.precision;
    }
	
    string name;			     //!<  mainly for debugging purposes
    FieldType field_type;
    int precision;
};

//!  A few constants for representing missing values
extern const char MissingString;
extern const unsigned char MissingCharacter;
extern const signed char MissingSignedChar;
extern const short MissingShort;
extern const int MissingInt;
extern const float MissingFloat;
extern const double MissingDouble;
extern const PDate MissingDate;
    

//#############################  CLASS  FIELDPTR  #########################
/*!     
  A FieldPtr is like a pointer-to-member in C++: it represents the
  abstract location of a single field within a Row.  The operation of
  binding a FieldPtr with a Row, yielding a Row::iterator is very
  fast.
    
  It is an error to bind a pointer created w.r.t. given schema to a
  row using an incompatible schema, although this is not checked for
  it would be prohibitively expensive (it's more complex than just
  checking a schema pointer).
*/

class FieldPtr {
    friend class Row;			     //!<  for binding
    friend class Schema;		     //!<  for construction

public:
    //!  The default ctor makes a "null" field ptr
    FieldPtr() : field_index_(-1), offset_(-1) {}

    //!  Using default copy ctor, dtor, and operator=

    //!  Useful accessors
    int field_index() const {
        return field_index_;
    }

    ptrdiff_t offset() const {
        return offset_;
    }
	
    //!  Conversion to bool: true if pointer is non-null
    operator bool() const {
        return field_index_ >= 0;
    }

    //!  Negation: true if pointer is null
    bool operator!() const {
        return field_index_ == -1;
    }

    //!  Comparison operators
    bool operator==(const FieldPtr& x) const {
        return field_index_ == x.field_index_ &&
            offset_ == x.offset_;
    }

    bool operator!=(const FieldPtr& x) const {
        return !(*this == x);
    }

private:
    //!  For use by Schema and Row::iterator
    FieldPtr(int fi, ptrdiff_t o) : field_index_(fi), offset_(o) {}

private:
    int field_index_;		     //!<  field number in schema
    ptrdiff_t offset_;		     //!<  actual byte offset_into row
};


//############################  CLASS  FIELDVALUE  ##########################
/*!     
  A FieldValue is intended to encapsulate all polyphormic operations
  (copy, comparison, conversion to/from string, double, etc.) on the 
  values of a SimpleDB field.  
*/

class FieldValue
{
    friend class FieldRowRef;
    
public:
    //!  Default construction and copy construction
    FieldValue();
    FieldValue(const FieldValue& fv);
    ~FieldValue();

    //!  Constructors from concrete values
    explicit FieldValue(const char*);
    explicit FieldValue(unsigned char);
    explicit FieldValue(signed char);
    explicit FieldValue(short);
    explicit FieldValue(int);
    explicit FieldValue(float);
    explicit FieldValue(double);
    explicit FieldValue(const PDate&);

    //!  Missing value handling
    bool isMissing() const;
    void setMissing();
    
    //!  Conversion to strings and doubles
    string toString() const;
    double toDouble() const;
    PDate  toDate() const;

    operator double() const { return toDouble(); }
    operator float() const { return float(toDouble()); }
    operator int() const { return int(toDouble()); }
    operator string() const { return toString(); }
    operator PDate() const { return toDate(); }
    
    //!  --- Pseudo-polymorphic operations

    //!  Assignment
    FieldValue& operator=(FieldValue);

    //!  Relational (relational operations don't always make sense)
    bool operator==(const FieldValue&) const;
    bool operator< (const FieldValue&) const;

    bool operator!=(const FieldValue& rhs) const {
        return !(*this == rhs);
    }
    
    bool operator<=(const FieldValue& rhs) const {
        return (*this == rhs) || (*this < rhs);
    }
    
    bool operator> (const FieldValue& rhs) const {
        return !(*this <= rhs);
    }
    
    bool operator>=(const FieldValue& rhs) const {
        return !(*this < rhs);
    }

    //!  Arithmetic (again, these don't always make sense)
    FieldValue operator+(const FieldValue&) const;
    FieldValue operator-(const FieldValue&) const;
    FieldValue operator*(const FieldValue&) const;
    FieldValue operator/(const FieldValue&) const;
    
    //!  Swap the guts of this with someone else
    void swap(FieldValue&);

    //!  Some useful type-checking predicates
    bool isString() const {
        return field_type_ == StringType;
    }
    
    bool isIntegral() const {
        return
            field_type_ == CharacterType || field_type_ == SignedCharType ||
            field_type_ == ShortType     || field_type_ == IntType;
    }
    
    bool isFloating() const {
        return field_type_ == FloatType || field_type_ == DoubleType;
    }
    
    bool isDate() const {
        return field_type_ == DateType;
    }
    
private:
    FieldType field_type_;
    int precision_;
    struct DateVal_t {
        short year;
        unsigned char month, day;
    };
    union {
        char* string_val_;
        long long_val_;
        double double_val_;
        struct DateVal_t date_val_;
    };
};

ostream& operator<<(ostream&, const FieldValue&);



//##############################  CLASS  SCHEMA  ############################
/*!     
  A schema is essentially a vector of fields, with a few extensions to
  more easily find individual fields within the schema.
    
  (Impl. note: we really should be using private inheritance here, but
  this is difficult to achieve since then every single function in the
  base class would have to be made visible through using-decl.  In
  addition, special provisions would have to be hacked up for ensuring
  a smooth working of iterators.  In any case, since we don't add any
  data member, an (accidental) destruction of a Schema by vector's
  destructor is in practice harmless.)
*/
    
class Schema : public vector<Field>
{
    typedef vector<Field> inherited;
    typedef Field T;

 public:
    //!  -- Forwarding functions for base-class constructors
    Schema() : inherited() {}
    Schema(size_type n, const T& value) : inherited(n, value) {}
    Schema(int n, const T& value) : inherited(n, value) {}
    Schema(long n, const T& value) : inherited(n, value) {}
    explicit Schema(size_type n) : inherited(n) {}
    Schema(const Schema& x) : inherited(x) {}

#ifdef __STL_MEMBER_TEMPLATES
    template <class InputIterator>
        Schema(InputIterator first, InputIterator last)
        : inherited(first, last) {}
#else /*!< __STL_MEMBER_TEMPLATES */
    Schema(const_iterator first, const_iterator last)
        : inherited(first, last) {}
#endif /*!< __STL_MEMBER_TEMPLATES */

    //!  (Use inherited dtor and op=)
	
    //!  -- Additional functionality specific to Schema

/*!       Find a schema entry ("column") by name, return true if found,
  and return to position, total bytes before in a row, and number
  of bytes in the column
*/
    bool findColumn(const string& name, int& position, int& start,
                    int& precision) const;

    //!  Make a pointer-to-field from a column number (zero-based)
    FieldPtr findColumn(int position) const;

    //!  Make a pointer-to-field from a column name
    FieldPtr findColumn(const string& name) const;

/*!       For convenience, operator() on a string or an integer position
  return findColumn().  This does not conflict with any member
  inherited from vector<>
*/
    FieldPtr operator()(int position) const {
        return findColumn(position);
    }

    FieldPtr operator()(const string& name) const {
        return findColumn(name);
    }
};


  
//#########################  CLASS  SIMPLEDBINDEXKEY  #######################
/*!     
  An IndexKey is used by hash tables for indexing purposes.  It
  *contains* a tiny vector of bytes (cannot be a pointer onto a
  row, since full row objects are fairly short-lived).
*/
  
template <class KeyType>
class SimpleDBIndexKey {
public:
/*!       These is a small semantic clash between what Hash requires
  and the way database rows are implemented.  According to the
  C++ standard, the only type that is guaranteed to map to any
  preconceived notion of "amorphous byte" is the unsigned char
  (assuming 8-bit chars).  Hence, all our internal data
  storage is performed this way.  However, Hash works with
  plain chars, which causes a somewhat annoying compiler
  complaint of possible conversion error.  This is why you see
  the static_cast in operator char*().  
*/
    typedef KeyType ByteArr;
    typedef typename ByteArr::iterator iterator;

    SimpleDBIndexKey() {}		     //!<  default ctor
	    
    explicit SimpleDBIndexKey(size_t len)
        : raw(len, '\0') {}
	    
    SimpleDBIndexKey(const unsigned char* the_raw, size_t len)
        : raw(len, '\0')
    {
        copy(the_raw, the_raw+len, begin());
    }
		
    SimpleDBIndexKey(const ByteArr& the_raw)
        : raw(the_raw) {}

/*!       default copy constructor
  default destructor
  default assignment operator
*/

/*!       REquired interface.  There is one small thing that our dear
  Hash Table class does not account for: operator char*() MUST
  BE CONST! or otherwise, the conversion operator is not
  called properly.
*/
    operator char*() const {
        return (char*)(&raw[0]);
    }

    size_t byteLength() const {
        return raw.size();
    }

    //!  relational operators
    bool operator==(const SimpleDBIndexKey& other) const {
        return raw == other.raw;
    }

    bool operator!=(const SimpleDBIndexKey& other) const {
        return raw != other.raw;
    }

    void resize(size_t len) {
        raw.resize(len);
    }

    typename ByteArr::iterator begin() {
        return raw.begin();
    }

    typename ByteArr::iterator end(){
        return raw.end();
    }

private:
    ByteArr raw;
};

  
  
//#############################  CLASS  SIMPLEDB  #########################
/*!     
  Simple Database
    
  This class permits the representation of a simple quasi-flat-file
  database in an efficient binary format, and enables indexing string
  columns to obtain the rows that match a given string.
*/

template <class KeyType = TinyVector<unsigned char, 8>,
          class QueryResult = TinyVector<unsigned int, 4> >
class SimpleDB
{
    //!  Friend functions and classes
    //!  Template constraints function

public:
    //!  --- Public Type Definitions

    //!  A row number in the database.  Rows are numbered starting with
    //!  0.  InvalidRow is a constant denoting an invalid row number.
    typedef unsigned long RowNumber;
    enum {
        InvalidRow = ULONG_MAX 
    };

    //!  A physical offset_into the database
    typedef unsigned long Offset;

    //!  Whether the user is granted read/write or read-only access
    enum AccessType {
        readwrite = 0,
        readonly  = 1
    };
	
    //!  Maximum size that a single database file can hold; this is
    //!  currently set to 512 MB.
    static const Offset AbsoluteFileLimit = 512ul * 1024ul * 1024ul - 1;
	
    //!  The result of making a query
    typedef QueryResult QueryResult_t;	     //!<  make it available
    static QueryResult EmptyResult;	     //!<  A null query
    
    //!  An index is simply a hash table from IndexKey to QueryResult
    typedef SimpleDBIndexKey<KeyType> IndexKey;
    typedef Hash<IndexKey,QueryResult> Index;
    typedef PP<Index> PIndex;
    
public:

    //!  --- Constructors, etc.
    SimpleDB(string rootname, string path=".", AccessType = readwrite,
             bool verbose=true);
    virtual ~SimpleDB();

	
    //!  --- Functions dealing with database name and location
    string getName() const {
        return name;
    }
    string getPath() const {
        return path;
    }
	
	
    //!  --- Functions dealing with schema representation.
    void setSchema(const Schema& s);

    const Schema& getSchema() const {
        return schema;
    }

    void saveSchema();
    void loadSchema();
      
/*!       Find a column by name, return true if found, and return the
  position, total bytes before in the row, and number of bytes in
  the column
*/
    bool findColumn(string name, int& position, int& start,
                    int& precision) const {
        return schema.findColumn(name,position,start,precision);
    }

    //!  returns the index of the given field inside the Schema, -1 if
    //!  not found
    int indexOfField(const string& fieldname) const {
        return schema(fieldname).field_index();
    }

    //!  --- Functions dealing with simple database queries
    Row getRow(RowNumber) const;
    Row& getInRow(RowNumber, Row&) const;
    RowNumber size() const { return size_; }  //!<  1 beyond maximum row number

    int length() const {
        return int(size());
    }
    int width() const {
        return int(schema.size());
    }

    //!  --- Functions dealing with database modifications
    void addRow(const Row&);		     //!<  add at end of database
    void setRow(const Row&, RowNumber);	     //!<  set a particular row
    void truncateFromRow(RowNumber n);	     //!<  erase all rows from row n
					     //!    (included) until end


    //!  --- Functions dealing with indexing and more complex queries

/*!       Index according to the field having a given name.
  Return TRUE if indexing has been successful.  At the moment,
  indexes are strictly kept in memory; they are not saved to disk.
  Optionally, the contents of a second column can be concatenated
  for indexing purposes.
*/
    bool indexColumn(string columnName,
                     string secondColumn = string(""));

    //!  Clear an index from memory to free up space
    void clearIndex(string columnName);
    
/*!       Find all rows in the database having the sequence of bytes
  contained in lookfor in the column columnName.  (The number of
  bytes to match is determined by the precision of the field type
  of the specified column).  If the column has been indexed
  before, use the index; otherwise, use linear search through the
  database.
*/
    QueryResult findEqual(const unsigned char* lookfor,
                          string columnName,
                          string secondColumn = string(""));

    //!  Always use the index to find "lookfor" 
    const QueryResult& findEqualIndexed(const unsigned char* lookfor,
                                        string columnName,
                                        string secondColumn = string(""));

    //!  Use linear search to find "lookfor"
    QueryResult findEqualLinear(const unsigned char* lookfor,
                                string columnName,
                                string secondColumn = string(""));
	
    //!  Use linear search to find multiple "lookfor"
    typedef vector<const unsigned char*> vuc;
    QueryResult findEqualLinear(const vuc& lookfor,
                                string columnName,
                                string secondColumn = string(""));

    //!  Access the table size multiplier (see description below)
    double tableSizeMultiplier() const {
        return table_size_multiplier;
    }
    void tableSizeMultiplier(double x) {
        table_size_multiplier = x;
    }
    
    //!  (should provide union, intersection, and such operators...)
	
private:
    //!  computes the size_ field (called by constructor)
    void computeSize();

    //!  convert a row from machine-dependent endianness to disk-standard
    //!  little-endian format.
    void memoryToDisk(Row&) const;

    //!  convert a row from disk-standard little-endian format to
    //!  machine-dependent endianness
    void diskToMemory(Row&) const;

/*!       given a row number, seek to the beginning of the row of the
  correct underlying physical file (segment), and return a file
  descriptor to the file; open a new file if necessary
*/
    int seekToRow(RowNumber) const;

    //!  seek to end of database, return FD to the correct file
    int seekToEnd() const;

    //!  open all existing segments in database
    void openAllFiles() const;
	
    //!  close all open file descriptors
    void closeAllFiles() const;

    //!  return the index (NOT FD!) of the last open segment
    inline int lastSegment() const;
	
    //!  return full path of segment i (zero-based)
    string getSegmentPath(int i) const;
	
private:
    //!  -- Physical characteristics of database
    string name;			     //!<  database base name
    string path;			     //!<  database root path
    AccessType access_type;		     //!<  readwrite or readonly
    int access_mask;			     //!<  Unix constants for access_type
    RowNumber size_;			     //!<  cached number of rows


    //!  -- Schema-related
    Schema schema;			     //!<  database schema
    int row_size;			     //!<  length of a row in bytes
    RowNumber max_records_file;		     //!<  maximum number of full rows
					     //!   in a single file
	
    mutable vector<int> allfd;		     //!<  File descriptors of open
					     //!  segments; -1 for not open.

    //!  -- Indexing-related

/*!       The multiplier factor for converting the number of database entries
  into the size of the hash table for indexing.  By default, 1.8, but
  should be considerably less if there are many repeated keys.
*/
    double table_size_multiplier;
    
    //!  There's one possible index per column in the database.  An index
    //!  is made only when indexColumn is called for a particular column.
    vector<PIndex> indexes;

    //!  -- Miscellaneous
    bool verbose;			     //!<  print debugging info to cerr

private:
    //!  For now, disable copy construction and assignment
    SimpleDB(const SimpleDB&);
    void operator=(const SimpleDB&);
};


//###########################  CLASS  ROWITERATOR  ##########################
/*!     
  Traverse database rows.  A row iterator is "nearly" an input iterator.
  It lets the user navigate through a single row in the database.
  Similarly to JDBC, it lets the user convert the current element to the
  types that conform to the current schema.  Furthermore, the
  dereference operator (operator*()) returns a FieldRowRef objects,
  which behaves like a FieldValue, but when assigned to modifies the
  underlying row.  This lets you do "natural" things for any field type,
  such as:
*/
//			*myit1 = *myit2;

class FieldRowRef;
  
class RowIterator {
public:
    //!  -- Requirements from trivial iterator
    RowIterator() : curfield(0), curptr(0), schema(0) { }
    RowIterator(const RowIterator& x) {
        curfield = x.curfield;
        curptr = x.curptr;
        schema = x.schema;
    }
    bool operator==(const RowIterator& x) {
        return
            curfield==x.curfield &&
            curptr  ==x.curptr   &&
            schema  ==x.schema;
    }
    bool operator!=(const RowIterator& x) {
        return !((*this) == x);
    }

    //!  Non-trivial ctor
    RowIterator(int curf, unsigned char* curp, const Schema* sc)
        : curfield(curf), curptr(curp), schema(sc)
    { }

    //!  Default destructor is being used

    //!  Assignment
    RowIterator& operator=(const RowIterator& x) 
    {
        if (&x != this) 
        {
            curfield = x.curfield;
            curptr   = x.curptr;
            schema   = x.schema;
        }
        return *this;
    }
	    
    //!  Copying of value
    void copyFrom(const RowIterator& it)
    {
#ifdef BOUNDCHECK
        if(it.precision()!=precision() || it.getFieldType()!=getFieldType())
            PLERROR("In Row::iterator::copyFrom Source and destination fields not of same type or precision");
#endif
        copy(it.raw(),it.raw()+it.precision(),raw());
    }

    //!  Dereference
    inline FieldRowRef operator*() const;
    
    //!  -- Requirements from input iterator

/*!       Preincrement : the distance of the increment depends on the
  current field size.  And end() iterator has a current field
  number that is 1 beyond the maximum number of field.  A null
  iterator has everything equal to zero.
*/
    RowIterator& operator++() {
        if (schema && curptr && curfield < schema->size()) {
            curptr += (*schema)[curfield].precision;
            ++curfield;
        }
        return *this;
    }

    //!  Post-increment is expressed as a pre-increment
    RowIterator operator++(int) {
        RowIterator x = *this;
        ++(*this);
        return x;
    }

    //!  Permit limited random access from an iterator (positive
    //!  directions only!)
    RowIterator operator[](int i) {
        PLASSERT(i >= 0);
        RowIterator it = *this;
        while (i--)
            ++it;
        return it;
    }

    FieldType getFieldType() const
    { return (*schema)[curfield].field_type; }

    //!  -- Predicates to test whether current element is of a
    //!  certain type.  Always false for null or past-end iterators.

    bool isString() const {
        return schema && curptr && curfield < schema->size() &&
            (*schema)[curfield].field_type == StringType;
    }
	    
    bool isCharacter() const {
        return schema && curptr && curfield < schema->size() &&
            (*schema)[curfield].field_type == CharacterType;
    }
	    
    bool isSignedChar() const {
        return schema && curptr && curfield < schema->size() &&
            (*schema)[curfield].field_type == SignedCharType;
    }
	    
    bool isShort() const {
        return schema && curptr && curfield < schema->size() &&
            (*schema)[curfield].field_type == ShortType;
    }
	    
    bool isInt() const {
        return schema && curptr && curfield < schema->size() &&
            (*schema)[curfield].field_type == IntType;
    }
	    
    bool isFloat() const {
        return schema && curptr && curfield < schema->size() &&
            (*schema)[curfield].field_type == FloatType;
    }
	    
    bool isDouble() const {
        return schema && curptr && curfield < schema->size() &&
            (*schema)[curfield].field_type == DoubleType;
    }

    bool isDate() const {
        return schema && curptr && curfield < schema->size() &&
            (*schema)[curfield].field_type == DateType;
    }
	    
	    
/*!       -- Return a pointer to the CURRENT element, as the correct
  type.  Return 0 element if the wrong type is specified.
  The current element may be modified through this pointer.
*/
	    
    char* asString() {
        bool iss = isString();
        if (iss)
            return reinterpret_cast<char*>(curptr);
        else 
            return 0;
        //return isString()? reinterpret_cast<char*>(curptr) : 0;
    }

    unsigned char* asCharacter() {
        return isCharacter()?
            reinterpret_cast<unsigned char*>(curptr) : 0;
    }

    signed char* asSignedChar() {
        return isSignedChar()?
            reinterpret_cast<signed char*>(curptr) : 0;
    }

    short* asShort() {
        return isShort()? reinterpret_cast<short*>(curptr) : 0;
    }

    int* asInt() {
        return isInt()? reinterpret_cast<int*>(curptr) : 0;
    }

    float* asFloat() {
        return isFloat()? reinterpret_cast<float*>(curptr) : 0;
    }

    double* asDouble() {
        return isDouble()? reinterpret_cast<double*>(curptr) : 0;
    }

    PDate* asDate() {
        return isDate()? reinterpret_cast<PDate*>(curptr) : 0;
    }


/*!       -- CONST VERSIONS.  Return a pointer to the CURRENT element,
  as the correct type.  Return 0 element if the wrong type
  is specified.  The current element may be modified through
  this pointer.
*/
	    
    const char* asString() const {
        return isString()? reinterpret_cast<const char*>(curptr) : 0;
    }

    const unsigned char* asCharacter() const {
        return isCharacter()?
            reinterpret_cast<const unsigned char*>(curptr) : 0;
    }

    const signed char* asSignedChar() const {
        return isSignedChar()?
            reinterpret_cast<const signed char*>(curptr) : 0;
    }

    const short* asShort() const {
        return isShort()? reinterpret_cast<const short*>(curptr) : 0;
    }

    const int* asInt() const {
        return isInt()? reinterpret_cast<const int*>(curptr) : 0;
    }

    const float* asFloat() const {
        return isFloat()? reinterpret_cast<const float*>(curptr) : 0;
    }

    const double* asDouble() const {
        return isDouble()? reinterpret_cast<const double*>(curptr) : 0;
    }

    const PDate* asDate() const {
        return isDate()? reinterpret_cast<const PDate*>(curptr) : 0;
    }


    //!  -- GENERIC CONVERSIONS.  To string and to double, from
    //!  nearly any type of field
    double toDouble() const;
    string toString() const;

	    
    //!  -- The following functions deal with missing values
    bool isMissing() const;
    void setMissing();
	    

    //!  Return the name of the current field, or "" if not currently
    //!  pointing to any valid field
    string name() const {
        return (schema && curfield < schema->size())?
            (*schema)[curfield].name : string("");
    }
	    
    //!  Return the precision of the current field (number of bytes),
    //!  or -1 if not currently pointing to any valid field.
    int precision() const {
        return (schema && curfield < schema->size())?
            (*schema)[curfield].precision : -1;
    }

    //!  Return the size in characters required to print out the
    //!  current field
    int char_width() const;

    //!  Return the physical address of the iterator
    unsigned char* raw() {
        return curptr;
    }

    const unsigned char* raw() const {
        return curptr;
    }

/*!       Make a FieldPtr from the current iterator position
  (this operation is not extremely fast, since we must loop
  from the beginning of the row)
*/
    FieldPtr makePtr() const {
        return schema->findColumn(curfield);
    }

private:
    unsigned curfield;		     //!<  current field number in schema
    unsigned char* curptr;	     //!<  current ptr in rawrow
    const Schema* schema;
};


//###########################  CLASS  FIELDROWREF  ##########################
/*!     
  A FieldRowRef represents a reference to a (polymorphic) field in a
  row.  It can transparently convert itself into a FieldValue, or be
  assigned from a FieldValue.
*/
  
class FieldRowRef
{
public:
    FieldRowRef(const RowIterator& it)
        : it_(it) {}

    //!  Conversion from / assignment into FieldValue
    operator FieldValue() const;
    FieldRowRef& operator=(const FieldValue&);
    FieldRowRef& operator=(const FieldRowRef rhs) {
        return operator=(FieldValue(rhs));
    }

    //!  Taking the address of a FieldRowRef returns a RowIterator
    inline RowIterator operator&() const;
    
private:
    RowIterator it_;			     //!<  of course, this just wraps
					     //!  an iterator
};
  

//###############################  CLASS  ROW  ############################
/*!     
  A database row is fundamentally a fixed-width byte string.  Since we
  don't want to give direct access to it, we define STL iterators that
  let us traverse the structure
*/

class Row {
	
public:
    //!  -- Definition of public nested types
    typedef RowIterator iterator;
    typedef iterator const_iterator;	     //!<  high-class cheating!
    typedef long size_type;		     //!<  number of elements in a row

public:
/*!       -- We are now in a position to define the Row public interface.
  This is fairly simple, since a lot of the work is delegated to
  the iterator.
*/
    Row() : rawrow(), schema(0) { }
    Row(const Row& r) : rawrow(r.rawrow), schema(r.schema) { }

    //!  Construct an empty row given only a schema
    Row(const Schema* s);
	
    //!  Construct a row given a schema and a full vector of bytes.
    //!  The vector is assumed to be in proper endianness.
    Row(const vector<unsigned char>& raw, const Schema* s)
        : rawrow(raw), schema(s) { }
	
	
    //!  Default destructor and assignment operator are being used
	
	
    //!  Container standard STL functions
    iterator begin() {
        return iterator(0, raw(), schema);
    }

    iterator end() {
        if (schema)
            return iterator(schema->size(), raw()+rawrow.size(),
                            schema);
        else
            return iterator(0,0,0);
    }

    //!  Return the size of a complete row in CHARS!
    size_type size() const {
        return (size_type)rawrow.size();
    }
	
    size_type max_size() const {
        return (size_type)rawrow.size();
    }

    bool empty() const {
        return (schema && schema->empty()) || !schema;
    }

    //!  Return the raw data
    const unsigned char* raw() const {
        if (rawrow.size())
            return &rawrow[0];
        else
            return 0;
    }

    unsigned char* raw() {
        if (rawrow.size())
            return &rawrow[0];
        else
            return 0;
    }

    const Schema* getSchema() const {
        return schema;
    }

/*!       A few utility operators.  Note: contrarily to standard
  containers, operator[] does not return a reference to a type,
  but an iterator to the proper position within the row.  This is
  done because of the intrinsic polymorphism of row elements.
  An iterator equal to end() is returned if the field could be
  accessed/found.
*/
    iterator operator[](int fieldNumber);
    iterator operator[](string fieldName);

/*!       Bind a FieldPtr to a row to produce a row iterator (this would
  actually be an opportunity to overload operator->*(), note the
  star, but use restraint and don't do it).  Right now, we define
  binding a null pointer as an error.
*/
    iterator bind(const FieldPtr& p) const {
        if (!p)
            PLERROR("Trying to dereference a null FieldPtr");
        return iterator(p.field_index_,
                        const_cast<unsigned char*>(raw()) + p.offset_,
                        schema);
    }

/*!       This should be called to perform some sanity checking on a row
  before writing it to disk.  This function is marked const, but
  nevertheless makes invisible changes to the object.
*/
    void sanitize() const;

private:
    vector<unsigned char> rawrow;
    const Schema* schema;
};


//#####  Row-related global functions  ####################################

//!  Generic conversions from an iterator
double todouble(const Row::iterator& it);
string tostring(const Row::iterator& it);
    
//!  outputs a single field flushed right in a cell of apropriate width
//!  (as given by field.char_width())
ostream& operator<<(ostream& o, const Row::iterator& field);

//!  outputs all fields in a row, separated by " | "
ostream& operator<<(ostream&, const Row& row);

//!  outputs the given field name in a cell of apropriate size
void printFieldName(ostream& o, const Row::iterator& field);

//!  outputs all field names, separated by " | "
void printFieldNames(ostream& o, const Row& row);


//#####  Miscellaneous Declarations  ########################################
  
//!  A utility typedef for the common case
typedef SimpleDB<> SDB;

//!  Performs a random permutation of all the rows of the SDB
//!  (same algorithm as Mat::shuffle)
void randomShuffleRows(SDB& sdb);

//!  not quite a random shuffle (see implementation)
//!  but more efficient use of disk cache
void halfShuffleRows(SDB& sdb);



//#####  Non-Template Inline Functions  #####################################

FieldRowRef RowIterator::operator*() const
{
    return FieldRowRef(*this);
}

RowIterator FieldRowRef::operator&() const
{
    return it_;
}

  
  
//#####  Implementation of Templates  #######################################

template <class KT, class QR>    
QR SimpleDB<KT,QR>::EmptyResult;



//#####  SimpleDB-related functions  ########################################
    
template <class KT, class QR>   
SimpleDB<KT,QR>::SimpleDB(string rootname, string the_path,
                          AccessType the_access_type, bool the_verbose)
    : name(rootname), path(the_path), access_type(the_access_type),
      access_mask(0), schema(), row_size(), allfd(),
      table_size_multiplier(1.8), indexes(), verbose(the_verbose)
{
    if (path != "")
        path += slash;

    switch (access_type) {
    case readwrite:
        access_mask = O_RDWR;
        break;
    case readonly:
        access_mask = O_RDONLY;
        break;
    }
	
    loadSchema();
    openAllFiles();
    computeSize();
}

template <class KT, class QR>   
SimpleDB<KT,QR>::~SimpleDB()
{
    //!  Upon destroying the database, save the current schema
    closeAllFiles();
    saveSchema();
}

template <class KT, class QR>   
void SimpleDB<KT,QR>::setSchema(const Schema& s)
{
    schema = s;
    Row row(&s);
    row_size = row.size();
    indexes.resize(s.size());

    //!  Compute the maximum size of a single file, taking into account
    //!  the row size.
    if (row_size > 0)
        max_records_file = RowNumber(AbsoluteFileLimit / row_size);
    else
        max_records_file = 0;

    //!  cout << "In setSchema for db " << getName() << " row_size=" << row_size << " max_records_file=" << max_records_file << endl;

    //!  Reopen files with new schema in effect
    closeAllFiles();
    openAllFiles();
    computeSize();
}

template <class KT, class QR>   
void SimpleDB<KT,QR>::saveSchema()
{
    if (access_type==readwrite) {
        string fullpath = path + name + ".ssc";
        ofstream sf(fullpath.c_str(), ios::out);
        Schema::iterator it = schema.begin(), end = schema.end();
        for (; it != end; ++it) {
            sf << it->name << " ";
            switch (it->field_type) {
            case Unknown:
                break;
            case StringType:
                sf << "string " << it->precision << endl;
                break;

            case CharacterType:
                sf << "character" << endl;
                break;
		
            case SignedCharType:
                sf << "signedchar" << endl;
                break;
	    
            case ShortType:
                sf << "short" << endl;
                break;
	    
            case IntType:
                sf << "int" << endl;
                break;
	    
            case FloatType:
                sf << "float" << endl;
                break;
	    
            case DoubleType:
                sf << "double" << endl;
                break;
	    
            case DateType:
                sf << "date" << endl;
                break;
	    
            default:
                PLERROR("Unknown field type in database: %d", it->field_type);
            }
        }
    }
}

template <class KT, class QR>   
void SimpleDB<KT,QR>::loadSchema()
{
    //!  If a schema does not exist, an empty schema is set into the current
    //!  database!
    string fullpath = path + name + ".ssc";
    ifstream sf(fullpath.c_str());
    Schema schema;
    while (sf) {
        string name,type;
        sf >> name >> type;
        if (name.size() == 0 || type.size() == 0)
            break;
        type = lowerstring(type);
        if (type == "string") {
            int length;
            sf >> length;
            schema.push_back(Field(name,StringType,length));
        }
        else if (type == "character")
            schema.push_back(Field(name,CharacterType));
        else if (type == "signedchar")
            schema.push_back(Field(name,SignedCharType));
        else if (type == "short")
            schema.push_back(Field(name,ShortType));
        else if (type == "int")
            schema.push_back(Field(name,IntType));
        else if (type == "float")
            schema.push_back(Field(name,FloatType));
        else if (type == "double")
            schema.push_back(Field(name,DoubleType));
        else if (type == "date")
            schema.push_back(Field(name,DateType));
        else {
            cerr << "Unexpected input type \"" << type
                 << "\" in schema file " << fullpath << endl;
            exit(1);
        }
    }
    setSchema(schema);
}

template <class KT, class QR>   
void SimpleDB<KT,QR>::addRow(const Row& row)
{
    if(row_size != row.size())
        PLERROR("In addRow row_size != row.size() (%d != %d)", row_size, row.size());
    row.sanitize();
    int fd = seekToEnd();
    off_t curpos = lseek(fd, 0L, SEEK_CUR);
    
#ifdef LITTLEENDIAN
    //!  ssize_t does not compile under MinGw (mini sig-win under Windows)
    //ssize_t writtensize = ::write(fd, row.raw(), row_size);
    int writtensize = ::write(fd, row.raw(), row_size);
#endif    
#ifdef BIGENDIAN
    Row newrow(row);
    memoryToDisk(newrow);
    int writtensize = ::write(fd, newrow.raw(), row_size);
#endif

    //!  Handle writing error
    if (writtensize == -1) {
        //!  Preserve database integrity by truncating from the point where we
        //!  should have started writing
#if defined(_MINGW_) || defined(WIN32)
        PLWARNING("could not truncate database file, end may be corrupted!");
#else
        ftruncate(fd, curpos);
#endif
        PLERROR("Could not write to database: %s", strerror(errno));
    }
    else
        size_++; //!<  increment length of db
}

template <class KT, class QR>   
void SimpleDB<KT,QR>::setRow(const Row& row, RowNumber n)
{
#ifdef __INTEL_COMPILER
#pragma warning(disable:186)  // Get rid of compiler warning.
#endif
    if(n<0 || n>=size())
#ifdef __INTEL_COMPILER
#pragma warning(default:186)
#endif
        PLERROR("Out of bounds in SimpleDB::setRow");
    if(row_size != row.size())
        PLERROR("In setRow row_size != row.size() (%d != %d)", row_size, row.size());
    row.sanitize();
    int fd = seekToRow(n);
	
#ifdef LITTLEENDIAN
    int writtensize = ::write(fd, row.raw(), row_size);
#endif
#ifdef BIGENDIAN
    Row newrow(row);
    memoryToDisk(newrow);
    int writtensize = ::write(fd, newrow.raw(), row_size);
#endif

    //!  Handle writing error
    if (writtensize == -1)
        PLERROR("Could not write to database: %s",strerror(errno));
}

  
template <class KT, class QR>   
void SimpleDB<KT,QR>::truncateFromRow(RowNumber n)
{
/*!       We must perform the following: seek to the proper row, truncate the
  current file, unlink all remaining files until end, close
  everything, re-open everything.
*/

    int curfd = seekToRow(n);
    off_t curpos = lseek(curfd, 0L, SEEK_CUR);
    if (ftruncate(curfd, curpos) == -1) {
        PLERROR((string("Could not truncate database at row ") +
                 tostring(n) + ": " + strerror(errno)).c_str());
    }

    vector<int>::iterator found = find(allfd.begin(), allfd.end(), curfd);
    int fromfd = found-allfd.begin() + 1;
    int last = lastSegment();

    closeAllFiles();
    bool allok = true;
    
    for ( ; fromfd <= last; ++fromfd) {
        string path = getSegmentPath(fromfd);
        if(unlink(path.c_str()) == -1) {
            PLWARNING((string("Could not unlink database segment ") + path +
                       ": " + strerror(errno)).c_str());
            allok = false;
        }
    }
    
    if (allok) {
        openAllFiles();
        computeSize();
    }
    else
        PLERROR("Error during truncation");
}

  
template <class KT, class QR>   
Row& SimpleDB<KT,QR>::getInRow(RowNumber n, Row& row) const
{
#ifdef __INTEL_COMPILER
#pragma warning(disable:186)  // Get rid of compiler warning.
#endif
    if(n<0 || n>=size())
#ifdef __INTEL_COMPILER
#pragma warning(default:186)
#endif
        PLERROR("Out of Bounds in SimpleDB::getInRow"); 
    if(row_size != row.size())
        PLERROR("In getInRow row_size!=row_size()");
    int fd = seekToRow(n);

    int size_read = ::read(fd, row.raw(), row_size);
    if (size_read == -1)
        PLERROR("Could not read from database: %s",strerror(errno));
    diskToMemory(row);
    return row;
}

template <class KT, class QR>   
Row SimpleDB<KT,QR>::getRow(RowNumber n) const
{
    Row row(&schema);
    getInRow(n, row);
    return row;
}

template <class KT, class QR>   
void SimpleDB<KT,QR>::computeSize()
{ 
    if(row_size<=0)
        size_ = 0;
    else
    {
        size_ = 0; 
        int i=0;
        int bytesinfile = file_size(getSegmentPath(i++));
        while(bytesinfile>0)
        {
            size_ += bytesinfile/row_size;
            bytesinfile = file_size(getSegmentPath(i++));
        }
    }

    /*! 
    //!  cerr << "computing size" << endl;
    if(row_size<=0) //!<  schema not yet set
    size_ = 0;
    else
    {
    int last = lastSegment();
    int fd = seekToEnd();
    assert (fd != -1 && last >= 0);
    off_t pos = lseek(fd, 0ul, SEEK_CUR);
    PLASSERT(row_size > 0 && pos % row_size == 0);
    size_ = pos / row_size + last * max_records_file;
    }
    */
      
}

template <class KT, class QR>   
void SimpleDB<KT,QR>::memoryToDisk(Row& row) const
{
#ifdef LITTLEENDIAN
    //!  nothing to do
#endif    
#ifdef BIGENDIAN
    Row newr(row);
    Row::iterator it = newr.begin(), end = newr.end();
    for(; it != end; ++it) {
        //!  unknown, strings and simple characters are NOT converted
        if (short* x = it.asShort())
            reverse_short(x,1);
        if (int* x = it.asInt())
            reverse_int(x,1);
        if (float* x = it.asFloat())
            reverse_float(x,1);
        if (double* x = it.asDouble())
            reverse_double(x,1);
        if (PDate* x = it.asDate()) {
            reverse_short(&(x->year),1);
        }
    }
#endif
}

template <class KT, class QR>   
void SimpleDB<KT,QR>::diskToMemory(Row& row) const
{
    memoryToDisk(row);
}


//#####  Physical Splitting Among Multiple Files  #########################

template <class KT, class QR>
int SimpleDB<KT,QR>::seekToRow(RowNumber i) const
{
    //!  cout << "seekToRow " << i << " in db " << getSegmentPath(0) << endl;
      
    if (max_records_file == 0)
        PLERROR("Attempting to seekToRow without schema set");
	
    int segmentNumber = int(i / max_records_file);
    Offset rowInSegment = Offset(i % max_records_file);

    //!  verify that segment is indeed open
    if (segmentNumber > lastSegment()) {
        for (int i = lastSegment()+1; i <= segmentNumber; ++i) {
            int fd = open(getSegmentPath(i).c_str(),
                          access_mask | O_CREAT, 0777);
            if (fd == -1)
                PLERROR("Could not open database segment %d at path %s: %s",
                        i, getSegmentPath(i).c_str(), strerror(errno));
            allfd.push_back(fd);
        }
    }
    if (allfd[segmentNumber] == -1)
        PLERROR("Problem accessing database segment %d at path %s",
                segmentNumber, getSegmentPath(segmentNumber).c_str());

    if (lseek(allfd[segmentNumber],
              rowInSegment * Offset(row_size), SEEK_SET)<0)
        PLERROR("problem in lseek: %s",strerror(errno));
    return allfd[segmentNumber];
}


template <class KT, class QR>
int SimpleDB<KT,QR>::seekToEnd() const
{
    //!  cout << "seekToEnd in db " << getName() << endl;

    if (max_records_file == 0)
        PLERROR("Attempting to seekToEnd without schema set");
	
    int last = lastSegment();
    int fd = allfd[last];
    if (fd == -1)
        PLERROR("Problem accessing database segment %d at path %s",
                last, getSegmentPath(last).c_str());

    off_t pos = lseek(fd, 0ul, SEEK_END);

    //!  There is a slight subtlety here: if the last segment is full, we
    //!  have to seek to the beginning of the next segment
    if (Offset(pos) / Offset(row_size) >= max_records_file)
        fd = seekToRow((last+1)*max_records_file);

    return fd;
}

    
template <class KT, class QR>
void SimpleDB<KT,QR>::openAllFiles() const
{
/*!       Since we don't know in advance how many files are in the
  database, we start by opening segment zero (which always
  exists), and then successively try to open more segments
*/

    closeAllFiles();		     //!<  safeguard
	
    int fd;
    fd = open(getSegmentPath(0).c_str(),
              access_mask | O_CREAT, 0777);
    if (fd == -1)
        PLERROR("Could not open main database segment %s: %s",
                getSegmentPath(0).c_str(), strerror(errno));
    allfd.push_back(fd);

    int index = 1;
    for ( ; ; ++index ) {
        fd = open(getSegmentPath(index).c_str(),
                  access_mask);
        if (fd == -1)
            break;
        else
            allfd.push_back(fd);
    }
}    

    
template <class KT, class QR>
void SimpleDB<KT,QR>::closeAllFiles() const
{
    vector<int>::iterator it=allfd.begin(), end=allfd.end();
    for (; it != end; ++it)
        if (*it != -1) {
            close(*it);
        }
    allfd.clear();
}


template <class KT, class QR>
inline int SimpleDB<KT,QR>::lastSegment() const
{
    return (int)allfd.size() - 1;
}
    
    
template <class KT, class QR>
string SimpleDB<KT,QR>::getSegmentPath(int i) const
{
    string fullpath = path + name;
    if (i >= 1 && i <= 26) {
        string postfix(1, char('a'+i-1));
        fullpath += string("_") + postfix;
    }
    else if (i > 26)
        PLERROR("Too many segments in the database.");
    if(fullpath.find(".sdb")==string::npos)
        fullpath += ".sdb";
    return fullpath;
}
    

//#####  Indexing- and Query-Related Functions  ###########################

template <class KT, class QR>   
bool SimpleDB<KT,QR>::indexColumn(string column_name,
                                  string second_column)
{
    bool has_second_column = (second_column.size() > 0);
    int n,  start_pos,  column_precision =0,
        n2, start_pos2, column_precision2=0;
    if (!findColumn(column_name, n, start_pos, column_precision))
        return false;
    if (has_second_column &&
        !findColumn(second_column, n2, start_pos2, column_precision2))
        return false;

/*!       Add all records to the index.  Create one if necessary.  Make
  initial size a constant times the number of records in the DB,
  as a heuristic.
*/
    RowNumber maxrows = size();
    RowNumber tablesize = RowNumber(table_size_multiplier*maxrows);
    if (maxrows <= 0 || tablesize <= 0) {
        PLWARNING("SimpleDB::indexColumn: cannot index a database of "
                  "zero size.");
        return false;
    }
    if (!indexes[n]) {
        indexes[n] = new Index(tablesize, true);
        indexes[n]->initializeTable((unsigned int)tablesize);
    }
    Index& index = *indexes[n];
    index.resize(tablesize);
    index.flush();
    
    Row currow(&schema);
    IndexKey key(column_precision + column_precision2);
    typename IndexKey::iterator keybegin = key.begin();
    unsigned char* begin1 = currow.raw() + start_pos;
    unsigned char* end1   = begin1 + column_precision;
    unsigned char* begin2 = currow.raw() + start_pos2;
    unsigned char* end2   = begin2 + column_precision2;	
    
    for(RowNumber i=0; i<maxrows; ++i) {
        if (verbose && i % 1000000 == 0) {
            unsigned numclusters, maxcluster;
            index.diagnostics(numclusters, maxcluster);
            cerr << "indexing row " << i
                 << "\t num. clusters=" << numclusters
                 << "\t max. cluster size=" << maxcluster
                 << endl;
        }

        getInRow(i,currow);
	
/*!         If current key already exists, just add current row number;
  otherwise create new query result (the -1 below assumes that
  the first column type is a string;--- should enforce this in
  the future).
*/
        copy(begin1, end1, keybegin);
        if (has_second_column)
            copy(begin2, end2, keybegin+column_precision-1);
        unsigned int addr = index.hashAddress(key);

        if (addr == Hash_UNUSED_TAG) {
/*! 	  Avoid creating a one-element QueryResult and then copy it over.
  We add an EMPTY key/value pair to the hash, table, then access
  the value object (through the normal code to add a new row into
  an existing key, then set the row i into it.
*/
            bool needresize = !index.add(key,QueryResult_t());
            if (needresize) {
                cerr << "Hash table unexpectedly full; exiting..." << endl;
                exit(1);
            }
            addr = index.hashAddress(key);
        }

        //!  Normal pathway: add row i to existing address
        QueryResult_t* qr = index[addr];
        try {
            qr->push_back(i);
        }
        catch (logic_error& e) {
            cerr << "Exception caught during indexing: "
                 << typeid(e).name() << endl
                 << "Containing: " << e.what() << endl;
            throw;
        }	
    }
    return true;
}

template <class KT, class QR>   
void SimpleDB<KT,QR>::clearIndex(string column_name)
{
    int n, start_pos, column_precision=0;
    if (findColumn(column_name, n, start_pos, column_precision))
        indexes[n] = 0;
}
    
template <class KT, class QR>
QR SimpleDB<KT,QR>::findEqual(const unsigned char* lookfor,
                              string column_name, string second_column)
{
    int n, start_pos, column_precision;
    if (!findColumn(column_name, n, start_pos, column_precision))
        return EmptyResult;
    if (indexes[n])
        return findEqualIndexed(lookfor, column_name, second_column);
    else
        return findEqualLinear(lookfor, column_name, second_column);
}
    
    
template <class KT, class QR>   
const QR& SimpleDB<KT,QR>::findEqualIndexed(const unsigned char* lookfor,
                                            string column_name,
                                            string second_column)
{
    bool has_second_column = (second_column.size() > 0);
    int n,  start_pos,  column_precision =0,
        n2, start_pos2, column_precision2=0;
    if (!findColumn(column_name, n, start_pos, column_precision))
        return EmptyResult;
    if (has_second_column &&
        !findColumn(second_column, n2, start_pos2, column_precision2))
        return EmptyResult;

    if (!indexes[n])
        PLERROR("SimpleDB::indexColumn must be done before performing "
                "indexed searches on column %s", column_name.c_str());
	
    Index& index = *indexes[n];
    IndexKey key(lookfor, column_precision+column_precision2);
    unsigned int addr = index.hashAddress(key);
    if (addr == Hash_UNUSED_TAG)
        return EmptyResult;
    else
        return *index[addr];
}


template <class KT, class QR>
QR SimpleDB<KT,QR>::findEqualLinear(const unsigned char* lookfor,
                                    string column_name,
                                    string second_column)
{
    vuc lf(1);
    lf[0] = lookfor;
    return findEqualLinear(lf, column_name, second_column);
}
    
    
template <class KT, class QR>   
QR SimpleDB<KT,QR>::findEqualLinear(
    const vuc& lookfor,
    string column_name, string second_column)
{
    bool has_second_column = (second_column.size() > 0);
    int n,  start_pos,  column_precision =0,
        n2, start_pos2, column_precision2=0;
    if (!findColumn(column_name, n, start_pos, column_precision))
        return EmptyResult;
    if (has_second_column &&
        !findColumn(second_column, n2, start_pos2, column_precision2))
        return EmptyResult;

    QR qr;

    //!  Make a vector of keys from all strings to look for
    vector<IndexKey> key_lookfor(lookfor.size());
    vuc::const_iterator
        lookit, lookbeg = lookfor.begin(), lookend = lookfor.end();
    typename vector<IndexKey>::iterator
        keyit, keybeg = key_lookfor.begin(), keyend = key_lookfor.end();
    size_t len = column_precision+column_precision2;
	
    for (lookit=lookbeg, keyit=keybeg ; lookit != lookend ;
         ++lookit, ++keyit) {
        keyit->resize(len);
        copy(*lookit, *lookit+len, keyit->begin());
    }
	
    IndexKey key_dbrow(column_precision+column_precision2);
    typename IndexKey::iterator keybegin = key_dbrow.begin();

    RowNumber maxrows = size();
    Row currow(&schema);
    unsigned char* begin1 = currow.raw() + start_pos;
    unsigned char* end1   = begin1 + column_precision;
    unsigned char* begin2 = currow.raw() + start_pos2;
    unsigned char* end2   = begin2 + column_precision2;

    //!  Indeed search linearly...
    for (RowNumber i=0; i<maxrows; ++i) {
        if (verbose && i % 1000000 == 0) {
            cerr << "Searching row " << i << endl;
        }

        getInRow(i, currow);

        copy(begin1, end1, keybegin);
        if (has_second_column)
            //!  (the -1 below assumes that the first column type is a
            //!  string;--- should enforce this in the future)
            copy(begin2, end2, keybegin+column_precision-1);

        //!  Look among all keys to lookfor
        for (keyit = keybeg ; keyit != keyend ; ++keyit) 
            if (*keyit == key_dbrow) {
                qr.push_back(i);
                if (verbose)
                    cerr << "Found string \"" << *keyit
                         << "\" at row " << i << endl;
            }
    }
	
    return qr;
}

#ifdef WIN32
#undef open
#undef close
#undef lseek
#undef read
#undef write
#undef unlink
#endif

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
