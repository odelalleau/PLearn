// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: VMat.h,v 1.2 2002/07/31 01:41:35 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef VMat_INC
#define VMat_INC

#include <cstdlib>
#include "PP.h"
#include "Mat.h"
#include "TMat.h"
#include "VarArray.h"
#include "IntVecFile.h"
#include <map>
#include "StatsCollector.h"
#include "TMat_maths_impl.h"

namespace PLearn <%
using namespace std;

  class Ker;

  //!  a VMField contains a fieldname and a fieldtype
  class VMField
  {
    public:

      enum FieldType 
      {
        UnknownType = 0,
        Continuous,
        DiscrGeneral,
        DiscrMonotonic,
        DiscrFloat,
        Date
      };

      string name;
      FieldType fieldtype;

      VMField(const string& the_name="", FieldType the_fieldtype=UnknownType);
    
    void print(ostream& out) const;
      void write(ostream& out) const;
      void read(istream& in);
  };

inline ostream& operator<<(ostream& out, const VMField& f) { f.print(out); return out; }
/*
  inline void write(ostream& out, const VMField& f) { f.write(out); }
  inline void read(istream& in, VMField& f) { f.read(in); }
*/
inline pl_ostream &operator<<(pl_ostream &out, const VMField &f)
{ f.write(out); return out; };

inline pl_istream &operator>>(pl_istream &in, VMField &f)
{ f.read(in); return in; };

  //!  this class holds simple statistics about a field
  class VMFieldStat
  {
    protected:
      int nmissing_;  //!<  number of missing values
      int nnonmissing_;  //!<  number of non-missing values
      int npositive_; //!<  number of values >0
      int nnegative_; //!<  number of values <0
      double sum_;    //!<  sum of all non missing values
      double sumsquare_; //!<  sum of square of all non missing values
      real min_;       //!<  minimum value
      real max_;       //!<  maximum value
      
      //!  maximum number of different discrete values to keep track of
      int maxndiscrete; 

    public:
      
      //!  counts of discrete values. If the size of counts exceeds maxndiscrete
      //!  maxndiscrete is set to -1, counts is erased, and we stop counting!
      map<real,int> counts;
      
      VMFieldStat(int the_maxndiscrete=255);
      
      int count() const { return nmissing_ + nnonmissing_; } //!<  should be equal to length of VMat
      int nmissing() const { return nmissing_; }
      int nnonmissing() const { return nnonmissing_; }
      int npositive() const { return npositive_; }
      int nnegative() const { return nnegative_; }
      int nzero() const { return nnonmissing_ - (npositive_+nnegative_); }
      real sum() const { return real(sum_); }
      real sumsquare() const { return real(sumsquare_); }
      real min() const { return min_; }
      real max() const { return max_; }
      real mean() const { return real(sum_/nnonmissing_); }
      real variance() const { return real((sumsquare_ - square(sum_)/nnonmissing_)/(nnonmissing_-1)); }
      real stddev() const { return sqrt(variance()); }

      real prob(real value) { return counts[value]/real(nnonmissing()); }

      void update(real val);
      
      void write(ostream& out) const;
      void read(istream& in);
  };
/*
  inline void write(ostream& out, const VMFieldStat& f) { f.write(out); }
  inline void read(istream& in, VMFieldStat& f) { f.read(in); }
*/
inline pl_ostream &operator<<(pl_ostream &out, const VMFieldStat &f)
{ f.write(out); return out; };

inline pl_istream &operator>>(pl_istream &in, VMFieldStat &f)
{ f.read(in); return in; };

/*! ** VMatrix ** */

class VMat;
class Func;

class VMatrix: public Object
{
protected:
  int length_;
  int width_;
  time_t mtime_; // time of "last modification" of files containing the data

  // are write operations tolerated?
  bool writable;

  //! Path of directory (possibly relative to DBDIR) that will contain meta information 
  //! on this dataset (fieldnames, cached statistics, etc...) and possibly the data itself.
  string metadatadir; 

  // contains a short name that can be used as part of a filename for results associated with this dataset.
  string alias_;

  // New set of statistics fields:
  TVec<StatsCollector> field_stats;  //!< stats[i] contains stats for field #i 

public:
  mutable Array<VMField> fieldinfos; // don't use this directly (deprecated...) call getFieldInfos() instead
    Array<VMFieldStat> fieldstats;

  VMatrix()
    :length_(-1), width_(-1), mtime_(0),writable(false)
    {}

  VMatrix(int the_length, int the_width)
    :length_(the_length), width_(the_width), mtime_(0), 
      writable(false),fieldstats(0)
      {}

  static void declareOptions(OptionList & ol);

  Array<VMField>& getFieldInfos() const;
  VMField& getFieldInfos(int fieldindex) const { return getFieldInfos()[fieldindex]; }
  void declareField(int fieldindex, const string& fieldname, VMField::FieldType fieldtype=VMField::UnknownType)
  { getFieldInfos(fieldindex) = VMField(fieldname,fieldtype); }
  int fieldIndex(const string& fieldname) const; //!<  returns -1 if name not found
  string fieldName(int fieldindex) const { return getFieldInfos(fieldindex).name; } 
  void unduplicateFieldNames(); // add a numeric suffix to duplic. fieldNames (eg: field.1 field.2 etc..)

  VMField::FieldType fieldType(int fieldindex) const { return getFieldInfos(fieldindex).fieldtype; } 
  VMField::FieldType fieldType(const string& fieldname) const { return fieldType(fieldIndex(fieldname)); } 
    const VMFieldStat& fieldStat(int j) const { return fieldstats[j]; } 
    const VMFieldStat& fieldStat(const string& fieldname) const { return fieldStat(fieldIndex(fieldname)); }

    void printFields(ostream& out) const;
    string fieldheader(int elementcharwidth=8);

  void saveFieldInfos(const string& filename) const;
  void loadFieldInfos(const string& filename);
  void writeFieldInfos(ostream& out) const;
  void readFieldInfos(istream& in);
  
  //! returns the string associated with value val 
  //! for field# col. Or returns "" if no string is associated.
  virtual string getValString(int col, real val) const;
  
  //! return value associated with a string. Default returns NaN
  virtual real getStringVal(int col, const string & str) const;

    virtual void computeStats(); 
    bool hasStats() const { return fieldstats.size()>0; }
    void writeStats(ostream& out) const;
    void readStats(istream& in);
    void saveStats(const string& filename) const;
    void loadStats(const string& filename);

  void setMetaDataDir(const string& the_metadatadir);
  string getMetaDataDir() const { return metadatadir; }

  //! returns the 'alias' for this dataset. The alias is a short name that 
  //! can be used as part of a filename containing results related to this VMat.
  string getAlias() const { return alias_; }
  void setAlias(const string& the_alias) { alias_ = the_alias; }

  //! returns the unconditonal statistics for all fields from the stats.psave file 
  //! (if the file does not exist, a default version is automatically created)
  TVec<StatsCollector> getStats();
  
  //! returns the ranges as defined in the ranges.psave file (for all fields)
  //! (if the ranges.psave file does not exist, a reasonable default version is created )
  TVec<RealMapping> getRanges();

  //! returns the cooccurence statistics conditioned on the given field
  //! (within the ranges returned by getRanges() )
  //! The results are cached in file stats#.psave (where # stands for the condfield index)
  PP<ConditionalStatsCollector> getConditionalStats(int condfield);

  // default version calls savePMAT
  virtual void save(const string& filename) const;

  virtual void savePMAT(const string& pmatfile) const;
  virtual void saveDMAT(const string& dmatdir) const;
  virtual void saveAMAT(const string& amatfile) const;

  inline int width() const { return width_; }
  inline int length() const { return length_; }

  //! Return the time of "last modification" associated with this matrix
  //! The result returned is typically based on mtime of the files contianing 
  //! this matrix's data when the object is constructed.
  //! mtime_ defaults to 0, so that's what will be returned by default, if 
  //! the time was never set by a call to setMtime(t) (see below).
  inline time_t getMtime() const { return mtime_; }

  //! Sets the "last modification" time  for this matrix
  //! For matrices on disk, this should be called by the constructor
  //! to reflect the mtime of the disk files.
  inline void setMtime(time_t t) { mtime_ = t; }

  //!  This method must be implemented in all subclasses
  virtual real get(int i, int j) const; //!<  returns element (i,j)

  //!  This method must be implemented in all subclasses of writable matrices
  virtual void put(int i, int j, real value); //!<  sets element (i,j) to value

  //!  It is suggested that this method be implemented in subclasses to speed up accesses
  //!  (default version repeatedly calls get(i,j) which may have a significant overhead)
  virtual void getSubRow(int i, int j, Vec v) const; //!<  fills v with the subrow i lying between columns j (inclusive) and j+v.length() (exclusive)

/*!     It is suggested that this method be implemented in subclasses of writable matrices
    to speed up accesses
    (default version repeatedly calls put(i,j,value) which may have a significant overhead)
*/
  virtual void putSubRow(int i, int j, Vec v);
    
  //!  This method must be implemented for matrices that are allowed to grow
  virtual void appendRow(Vec v);

  //!  These methods do not usually need to be overridden in subclasses
  //!  (default versions call getSubRow, which should do just fine)
  virtual void getRow(int i, Vec v) const; //!<  copies row i into v (which must have appropriate length equal to the VMat's width)
  virtual void putRow(int i, Vec v);
  virtual void fill(real value);
  virtual void getMat(int i, int j, Mat m) const; //!<  copies the submatrix starting at i,j into m (which must have appropriate length and width)
  virtual void putMat(int i, int j, Mat m); //!<  copies matrix m at position i,j of this VMat

  //!  copies column i into v (which must have appropriate length equal to the VMat's length)
  virtual void getColumn(int i, Vec v) const;

/*!     returns a Mat with the same data as this VMat
    The default version of this method copies the data in a fresh Mat created in memory
    However this method will typically be overrided by subclasses (such as MemoryVMatrix) 
    whose internal representation is already a Mat in order to return this Mat directly to avoid 
    a new memory allocation and copy of elements. In this case, and in this case only, modifying 
    the elements of the returned Mat will logically result in modified elements in the original 
    VMatrix view of it. 
*/
  virtual Mat toMat() const;

  //!  The default implementation of this method does nothing
  //!  But subclasses may overload it to reallocate memory to exactly what is needed and no more.
  virtual void compacify();

  //!  in case the dimensions of an underlying vmat has changed, recompute it
  virtual void reset_dimensions() {}

/*!     default version returns a SubVMatrix referencing the current VMatrix
    however this can be overridden to provide more efficient shortcuts 
    (see MemoryVMatrix::subMat and SubVMatrix::subMat for examples)
*/
  virtual VMat subMat(int i, int j, int l, int w);

/*!     returns the dot product between row i1 and row i2 (considering only the inputsize first elements).
    The default version in VMatrix is somewhat inefficient, as it repeatedly calls get(i,j)
    The default version in RowBufferedVMatrix is a little better as it buffers the 2 Vecs between calls in case one of them is needed again.
    But the real strength of this method is for specialised and efficient versions in subbclasses. 
    This method is typically used by SmartKernels so that they can compute kernel values between input samples efficiently.
*/
  virtual real dot(int i1, int i2, int inputsize) const;

  inline real dot(int i1, int i2) const { return dot(i1,i2,width()); }

  //!  returns the result of the dot product between row i and the given vec (only v.length() first elements of row i are considered).
  virtual real dot(int i, const Vec& v) const;

  operator Mat() const { return toMat(); }

  //!  Assigns the value of the Vars in the list
  //!  (the total size of all the vars in the list must equal width() )
  virtual void getRow(int i, VarArray& inputs) const;

  void print(ostream& out) const;
  virtual void oldwrite(ostream& out) const;
  virtual void oldread(istream& in);

  DECLARE_NAME_AND_DEEPCOPY(VMatrix);
  void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);


/*!     The following methods can be used in a straightforward manner to compute a variety of useful things:
    Dot products between this vmat and a vector, find the K nearest neighbours to a vector, etc...
    Most methods take an optional last parameter ignore_this_row which may contain the index of a row that
    is to be excluded from the computation (this can be seful for leave-one-out evaluations for instance).
*/

/*!     This will compute for this vmat m a result vector (whose length must be tha same as m's)
    s.t. result[i] = ker( m(i).subVec(v1_startcol,v1_ncols) , v2) 
    i.e. the kernel value betweeen each (sub)row of m and v2
*/
  virtual void evaluateKernel(Ker ker, int v1_startcol, int v1_ncols, 
                              const Vec& v2, const Vec& result, int startrow=0, int nrows=-1) const;

  //!   returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
  virtual real evaluateKernelSum(Ker ker, int v1_startcol, int v1_ncols, 
                                 const Vec& v2, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;

  //!  targetsum := sum_i [ m(i).subVec(t_startcol,t_ncols) * ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
  //!  and returns sum_i [ ker( m(i).subVec(v1_startcol,v1_ncols) , v2) ]
  virtual real evaluateKernelWeightedTargetSum(Ker ker, int v1_startcol, int v1_ncols, const Vec& v2, 
                                               int t_startcol, int t_ncols, Vec& targetsum, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;
  
   
/*!     This will return the Top N kernel evaluated values (between vmat (sub)rows and v2) and their associated row_index.
    Result is returned as a vector of length N of pairs (kernel_value,row_index)
    Results are sorted with largest kernel value first
*/
  virtual TVec< pair<real,int> > evaluateKernelTopN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                    const Vec& v2, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;

  //!  same as evaluateKernelTopN but will look for the N smallest values instead of top values.
  //!  results are sorted with smallest kernel value first
  virtual TVec< pair<real,int> > evaluateKernelBottomN(int N, Ker ker, int v1_startcol, int v1_ncols, 
                                                       const Vec& v2, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;


/*!     result += transpose(X).Y
    Where X = this->subMatColumns(X_startcol,X_ncols)
    and   Y =  this->subMatColumns(Y_startcol,Y_ncols);
*/
  virtual void accumulateXtY(int X_startcol, int X_ncols, int Y_startcol, int Y_ncols, 
                             Mat& result, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;


/*!     A special case of method accumulateXtY
    result += transpose(X).X
    Where X = this->subMatColumns(X_startcol,X_ncols)
*/
  virtual void accumulateXtX(int X_startcol, int X_ncols, 
                             Mat& result, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;

  //!  compute fprop or fbprop of a sumOf operation
  virtual void evaluateSumOfFprop(Func f, Vec& output_result, int nsamples=-1);
  virtual void evaluateSumOfFbprop(Func f, Vec& output_result, Vec& output_gradient, int nsamples=-1);
 
  virtual ~VMatrix();
};

DECLARE_OBJECT_PTR(VMatrix);

class RowBufferedVMatrix: public VMatrix
{
protected:
  mutable int current_row_index;
  mutable Vec current_row;
  mutable int other_row_index; //!<  used by dot
  mutable Vec other_row;

public:
  RowBufferedVMatrix();
  RowBufferedVMatrix(int the_length, int the_width);

  //!  This is the only method requiring implementation
  virtual void getRow(int i, Vec v) const =0;

  //!  These methods are implemented by buffering calls to getRow
  virtual real get(int i, int j) const; //!<  returns element (i,j)
  virtual void getSubRow(int i, int j, Vec v) const; //!<  fills v with the subrow i laying between columns j (inclusive) and j+v.length() (exclusive)

  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;

  DECLARE_ABSTRACT_NAME_AND_DEEPCOPY(RowBufferedVMatrix);
};

class VMat: public PP<VMatrix>
{
public:
  VMat();
  VMat(VMatrix* d);
  VMat(const VMat& d);
  VMat(const Mat& datamat); //!<  Convenience constructor: will use an MemoryVMatrix built from datamat

  int length() const { return ptr->length(); }
  int width() const { return ptr->width(); }

  real operator()(int i, int j) const { return ptr->get(i,j); }
  Vec operator()(int i) const { Vec v(ptr->width()); ptr->getRow(i,v); return v; }
  Vec getColumn(int i) const { Vec v(ptr->length()); ptr->getColumn(i,v); return v; }

  VMat subMat(int i, int j, int l, int w) const { return ptr->subMat(i,j,l,w); }
  VMat subMatRows(int i, int l) const { return ptr->subMat(i,0,l,width()); }
  VMat subMatColumns(int j, int w) const { return ptr->subMat(0,j,length(),w); }

  VMat row(int i) const { return subMatRows(i,1); }
  VMat firstRow() const { return row(0); }
  VMat lastRow() const { return row(length()-1); }
  VMat column(int j) const { return subMatColumns(j,1); }
  VMat firstColumn() const { return column(0); }
  VMat lastColumn() const { return column(width()-1); }
  Mat toMat() const { return ptr->toMat();}  

  //!  Returns a VMatrix made of only the specified rows
  VMat rows(TVec<int> rows_indices) const;
  //!  Returns a VMatrix made of only the specified rows
  VMat rows(Vec rows_indices) const;
  //!  Returns a VMatrix made of only the rows specified in the indexfile (see IntVecFile)
  VMat rows(const string& indexfile) const;

  //!  Returns a VMatrix made of only the specified columns
  VMat columns(TVec<int> columns_indices) const;
  //!  Returns a VMatrix made of only the specified columns
  VMat columns(Vec columns_indices) const;


  operator Mat() const { return ptr->toMat(); }
  inline void save(const string& filename) const { ptr->save(filename); }

  //!  will copy a precomputed version of the whole VMat into memory
  //!  and replace the current pointer to point to the corresponding MemoryVMatrix
  void precompute();
  
/*!     will copy a precomputed version of the whole VMat to the given file
    and replace the current pointer to point to the corresponding FileVMatrix
    For fast access, make sure the file is on a local Disk rather than on 
    a Network Mounted File System.
    If use_existing_file is true, it will use the existing file (from a
    previous precomputation for instance) rather than overwriting it (make
    sure the file indeed contains what you expect!)
*/
  void precompute(const string& pmatfile, bool use_existing_file=false);

  inline void print(ostream& out) const { ptr->print(out); }

  ~VMat();
};

inline pl_istream &operator>>(pl_istream &in, VMat &o)
{ in >> static_cast<PP<VMatrix> &>(o); return in; };

inline pl_ostream &operator<<(pl_ostream &out, const VMat &o)
{ out << static_cast<const PP<VMatrix> &>(o); return out; };

inline void operator<<(const Mat& dest, const VMatrix& src)
{
  if(dest.length()!=src.length() || dest.width()!=src.width())
    PLERROR("In operator<<(const Mat& dest, const VMatrix& src), incompatible dimenasions");
  src.getMat(0,0,dest);
}

inline void operator>>(const VMatrix& src, const Mat& dest)
{ dest << src; }

inline void operator<<(const Mat& dest, const VMat& src)
{ dest << *(VMatrix*)src; }

inline void operator>>(const VMat& src, const Mat& dest)
{ dest << src; }

class MemoryVMatrix: public VMatrix
{
 protected:
  Mat data;

 //!  necessary for the deepcopy mechanism 
 protected:
  MemoryVMatrix() : data(Mat()) {}

 public:
  MemoryVMatrix(const Mat& the_data);
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void getRow(int i, Vec v) const;
  virtual void getColumn(int i, Vec v) const;
  virtual void getMat(int i, int j, Mat m) const;
  virtual void put(int i, int j, real value);
  virtual void putSubRow(int i, int j, Vec v);
  virtual void putRow(int i, Vec v);
  virtual void appendRow(Vec v);
  virtual void fill(real value);
  virtual void putMat(int i, int j, Mat m);
  virtual Mat toMat() const;
  virtual VMat subMat(int i, int j, int l, int w);
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;

  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);

  DECLARE_NAME_AND_DEEPCOPY(MemoryVMatrix);
  void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
  virtual void build() {}  //!<  nothing to do...
};

/*!   Like MemoryVMatrix this class holds the data in memory,
  but it tries to hold it compactly by using single bits
  for binary variables, single bytes for discrete variables
  whose number of possible values is less than 256, and 
  unsigned shorts for the others, using a fixed point
  representation.
*/
class CompactVMatrix : public RowBufferedVMatrix
{
    class SDBVMatrix;

  protected:
    //!  Each row of the matrix holds in order: bits, 1-byte symbols, fixed point numbers
    Storage<unsigned char> data;
    int row_n_bytes; //!<  # of bytes per row
    int n_bits; //!<  number of binary symbols per row
    int n_symbols; //!<  number of 1-byte symbols per row
    int n_fixedpoint; //!<  number of fixed point numbers per row
    int n_variables; //!<  = n_bits + n_symbols + n_fixedpoint
    bool one_hot_encoding; //!<  the 1-byte symbols are converted to one-hot encoding by get
  public:
    TVec<int> n_symbol_values; //!<  for each 1-byte symbol, the number of possible values
    int nbits() { return n_bits; }
    int nsymbols() { return n_symbols; }
    int nfixedpoint() { return n_fixedpoint; }
  protected:
    Vec fixedpoint_min, fixedpoint_max; //!<  the ranges of each number for fixed point encoding
    Vec delta; //!<  (fixedpoint_max-fixedpoint_min)/2^16
    TVec<int> variables_permutation; //!<  this variable is used only when constructed from VMat
/*!                                        and provides the permutation of the original columns
                                       in order to order them into (bits, bytes, fixedpoint)
                                       variables_permutation[new_column]=old_column (not in one-hot code)
*/
    CompactVMatrix() {}

  public:
    int n_last; //!<  used by dotProduct and squareDifference to specify # of last columns to ignore
  protected:
    int normal_width;         //!<  the value of width_ when one_hot_encoding=true
  public:
    void setOneHotMode(bool on=true);

    CompactVMatrix(int the_length, int n_variables, int n_binary, int n_nonbinary_discrete,
                   int n_fixed_point, TVec<int>& n_symbolvalues, Vec& fixed_point_min,
                   Vec& fixed_point_max, bool one_hot_encoding=true);

/*!       Convert a VMat into a CompactVMatrix: this will use the stats
      computed in the fieldstats of the VMatrix (they will be computed if not
      already) to figure out which variables are binary, discrete 
      (and how many symbols), and the ranges of numeric variables.
      THE VMAT DISCRETE VARIABLES MUST NOT BE ALREADY ONE-HOT ENCODED.
      The variables will be permuted according to the permutation vector
      which can be retrieved from the variables_permutation_vector() method.
      By default the last column of the VMat will stay last, thus being coded as 
      fixedpoint (so the permutation information may not be necessary if the
      last column represents a target and all the previous ones some inputs.
      keep_last_variables_last is the number of "last columns" to keep in place.
*/
    CompactVMatrix(VMat m, int keep_last_variables_last=1, bool onehot_encoding=true);

    //!  construct from saved CompactVMatrix

    CompactVMatrix(const string& filename, int nlast=1);

/*!       Create a CompactVMatrix with the same structure as cvcm but
      containing the data in m. Both must obviously have the same width.
      If rescale is true, then the min/max values for fixed-point encoding
      are recomputed. If check==true than this is verified and an error message 
      is thrown if the floating point data are not in the expected ranges (of cvm).
*/
    CompactVMatrix(CompactVMatrix* cvm, VMat m, bool rescale=false, bool check=true);

    //!  append vm to this VMatrix (the rows of vm are concatenated to the current rows of this VMatrix)
    void append(CompactVMatrix* vm);

/*!       create in the elements of row (except the n_last ones) a perturbed
      version of the i-th row of the database. This 
      random perturbation is based on the unconditional
      statistics which should be present in the fieldstats; the
      noise level can be modulated with the noise_level argument
      (a value of 1 will perturb by as much as the noise seen in the
      unconditional statistics). Continuous variables are resampled around
      the current value with sigma = noise_leve * unconditional_sigma.
      Discrete variables are resampled with a distribution that is a mixture:
        (1-noise_level)*(probability mass on all current value)+noise_level*(unconditional distr)
*/
    void perturb(int i, Vec row, real noise_level, int n_last);

/*!       this vector is filled only when the CompactVMatrix was constructed
      from a VMat, and it provides the permutation of the original columns
      to order them into (bits, bytes, fixedpoint)
*/
    TVec<int>& permutation_vector() { return variables_permutation; }
    
    //!  decoding (v may be one-hot depending on one_hot_encoding flag)
    virtual void getRow(int i, Vec v) const;
    //!  encoding (v is not one-hot, and the variables in v are in the "original" order

    //!  return the square difference between row i and row j, excluding n_last columns
    virtual real squareDifference(int i, int j);

    //!  return the dot product of row i with row j, excluding n_last columns
    virtual real dotProduct(int i, int j) const;

    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;


    //!  (i.e. at position i in v we find variable variables_permutation[i] in getRow's result)
    virtual void encodeAndPutRow(int i, Vec v);
    //!  v is possibly one-hot-encoded (according to one_hot_encoding flag) and the
    //!  variables are in the same order as for getRow.
    virtual void putRow(int i, Vec v);
    virtual void putSubRow(int i, int j, Vec v);

    //!  save everything in file (data and auxiliary information), binary format
    virtual void save(const string& filename)
    { Object::save(filename); } //!<  calls write
    virtual void write(ostream& out) const;

    //!  reverse of write, can be used by calling load(string)
    virtual void oldread(istream& in);

    DECLARE_NAME_AND_DEEPCOPY(CompactVMatrix);
    void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
    virtual void build() {}  //!<  nothing to do...

  protected:
    //!  auxiliary 
    int symbols_offset; //!<  where in each row the symbols start
    int fixedpoint_offset; //!<  where in each row the fixed point numbers start
    Vec row_norms; //!<  to cache the norms of the rows for squareDifference method
};

/*!   Like MemoryVMatrix this class holds the data in memory.
  But it is designed to keep a compact representation of sparse matrices,
  keeping for each row only the position and values of the non-zero elements.
  The values are stored as floats regardless whether we ar in USEFLOAT or USEDOUBLE mode.
*/

class SparseVMatrixRow
{
  public:
    int nelements; //!<  number of non zero elements in row 
    int row_startpos; //!<  index of first element of this row in both the positions and the values arrays
    SparseVMatrixRow(): nelements(0), row_startpos(0) {}
};

class SparseVMatrix : public RowBufferedVMatrix
{
protected:
  int nelements; //!<  total number of non-zero elements in the VMatrix  
  unsigned short* positions;
  float* values;
  
  SparseVMatrixRow* rows;

public:

  SparseVMatrix():
    nelements(0),
    positions(0),
    values(0),
    rows(0)
  {}

  //!  This builds a sparse representation in memory of the VMat m passed
  //!  as argument.  The original fieldinfos are copied as-is.
  SparseVMatrix(VMat m);
    
  //!  This reloads a previously saved sparse VMatrix
  SparseVMatrix(const string& filename):
    nelements(0),
    positions(0),
    values(0),
    rows(0)  
  { load(filename); } 

  virtual void getRow(int i, Vec v) const;
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;

  virtual void save(const string& filename)
  { Object::save(filename); } //!<  calls write
  virtual void write(ostream& out) const;
  virtual void oldread(istream& in);
  	
  virtual ~SparseVMatrix();
};

class SubVMatrix: public VMatrix
{
  typedef VMatrix inherited;
protected:
  VMat parent;
  int istart;
  int jstart;

public:
  //! The appropriate VMFields of the parent VMat are copied upon
  //! construction
  SubVMatrix() {};
  SubVMatrix(VMat the_parent, int the_istart, int the_jstart, int the_length, int the_width);
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void getMat(int i, int j, Mat m) const;
  virtual void put(int i, int j, real value);
  virtual void putSubRow(int i, int j, Vec v);
  virtual void putMat(int i, int j, Mat m);
  virtual VMat subMat(int i, int j, int l, int w);

  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;

  virtual void reset_dimensions();

  DECLARE_NAME_AND_DEEPCOPY(SubVMatrix);
  static void declareOptions(OptionList &ol);
  virtual void build();
  void build_();

};

DECLARE_OBJECT_PTR(SubVMatrix);

//!  selects samples from a sub-distribution
//!  according to given vector of indices
class SelectRowsVMatrix: public VMatrix
{
    typedef VMatrix inherited;
protected:
  VMat distr;
  TVec<int> indices;
public:

  //! Also copies the original fieldinfos upon construction
  //! Here the indices will be shared for efficiency. But you should not modify them afterwards!
    SelectRowsVMatrix() {};
  SelectRowsVMatrix(VMat the_distr, TVec<int> the_indices) :
    VMatrix(the_indices.length(),the_distr->width()),
    distr(the_distr),indices(the_indices)
    {
      fieldinfos = the_distr->fieldinfos;
    }

  //! Here the indices will be copied locally into an integer vector
  SelectRowsVMatrix(VMat the_distr, Vec the_indices) :
    VMatrix(the_indices.length(),the_distr->width()),
    distr(the_distr),indices(the_indices.length())
    {
      fieldinfos = the_distr->fieldinfos;
      indices << the_indices; // copy to integer indices
    }
  
    DECLARE_NAME_AND_DEEPCOPY(SelectRowsVMatrix);

    static void declareOptions(OptionList &ol);

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual real getStringVal(int col, const string & str) const;
  virtual string getValString(int col, real val) const;
  virtual void reset_dimensions() { distr->reset_dimensions(); width_=distr->width(); }
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;
};

DECLARE_OBJECT_PTR(SelectRowsVMatrix);

//!  sees an underlying VMat with the specified rows excluded
class RemoveRowsVMatrix: public VMatrix
{
  protected:
    VMat distr;
    Vec indices;

    //!  returns the row number in distr corresponding to i in this VMat
    int getrownum(int i) const; 

  public:
    virtual void reset_dimensions() { distr->reset_dimensions(); width_=distr->width(); }

    //! Copy the original fieldinfos upon construction
    RemoveRowsVMatrix(VMat the_distr, Vec the_indices=Vec() ) :
      VMatrix(the_distr->length()-the_indices.length(), the_distr->width()),
      distr(the_distr), indices(the_indices.copy())
    {
      fieldinfos = the_distr->fieldinfos;
      
      if(indices)
        sortElements(indices);
    }

    //!  the given rownum of the underlying distr will also be excluded
    void remove(int rownum)
    { 
      indices.insertSorted(rownum,true); 
      length_--;
    }

  void unremove(int rownum)
  {
    indices.removeSorted(rownum);
    length_++;
  }

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;
};


/*!   selects samples from a sub-distribution
  according to given vector of indices
  that is stored on disk as an IntVecFile
*/
class SelectRowsFileIndexVMatrix: public VMatrix
{
protected:
  VMat distr;
  IntVecFile indices;
public:
  //! Copy the original fieldinfos upon construction
  SelectRowsFileIndexVMatrix(VMat the_distr, const string& indexfile) :
    VMatrix(0,the_distr->width()),
    distr(the_distr),indices(indexfile) 
    {
      fieldinfos = the_distr->fieldinfos;
      length_ = indices.length();
    }
  
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void getRow(int i, Vec v) const;
  virtual real getStringVal(int col, const string & str) const;
  virtual string getValString(int col, real val) const;
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;
  virtual void reset_dimensions() { distr->reset_dimensions(); width_=distr->width(); }
};

//!  selects variables (columns) from a sub-distribution
//!  according to given vector of indices.  NC: removed the unused field
//!  raw_sample.
class SelectColumnsVMatrix: public VMatrix
{
protected:
  VMat distr;
  TVec<int> indices;
public:
  //! The appropriate fieldinfos are copied upon construction
  //! Here the indices will be shared for efficiency. But you should not modify them afterwards!
  SelectColumnsVMatrix(VMat the_distr, TVec<int> the_indices);

  //! Here the indices will be copied locally into an integer vector
  SelectColumnsVMatrix(VMat the_distr, Vec the_indices);

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void reset_dimensions() 
    { 
      distr->reset_dimensions(); length_=distr->length(); 
      for (int i=0;indices.length();i++)
        if (indices[i]>=distr->width())
          PLERROR("SelectColumnsVMatrix::reset_dimensions, underlying distr not wide enough (%d>=%d)",
                indices[i],distr->width());
    }
};

/*!   VMatrix that can be used to rescale and shift each feature of the
  underlying distribution: x'_i = a_i*(x_i+b_i)
  This can be used to normalize the inputs of a distribution 
  (see the NormalizeInputDistr function in PLearn.h)
*/
class ShiftAndRescaleVMatrix: public VMatrix
{
protected:
  VMat distr;

public:
  //!  x'_i = (x_i+shift_i)*scale_i
  Vec shift; 
  Vec scale; 

  //! For all constructors, the original VMFields are copied upon construction
  ShiftAndRescaleVMatrix(VMat underlying_distr, Vec the_shift, Vec the_scale);
  ShiftAndRescaleVMatrix(VMat underlying_distr, int n_inputs);
  ShiftAndRescaleVMatrix(VMat underlying_distr, int n_inputs, int n_train);

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void reset_dimensions() 
    { 
      distr->reset_dimensions(); 
      length_=distr->length();
      if (width_!=distr->width())
        PLERROR("ShiftAndRescaleVMatrix: can't change width"); 
    }
};

/*!   VMatrix that extends the underlying VMat by appending rows at 
  its top and bottom and columns at its left and right.
  The appended rows/columns are filled with the given fill_value
  This can be used for instance to easily implement the usual trick 
  to include the bias in the weights vectors, by appending a 1 to the inputs.
*/
class ExtendedVMatrix: public RowBufferedVMatrix
{
public:
  VMat distr;
  int top_extent;
  int bottom_extent;
  int left_extent;
  int right_extent;
  real fill_value; 

  //!  Warning: VMFields are NOT YET handled by this constructor
  ExtendedVMatrix(VMat the_distr, 
                  int the_top_extent, int the_bottom_extent, 
                  int the_left_extent, int the_right_extent, 
                  real the_fill_value);

  virtual void getRow(int i, Vec v) const;
  virtual void reset_dimensions() 
    { 
      distr->reset_dimensions(); 
      width_=distr->width()+left_extent+right_extent; 
      length_=distr->length()+top_extent+bottom_extent;
    }
};


/*!  A VecExtendedVMatrix is similar to an ExtendedVMatrix: it extends the
  underlying VMat by appending COLUMNS to its right.  The appended columns
  are filled with a constant vector passed upon construction.  For example,
  if the vector [1,2,3] is passed at construction, then every row of the
  underlying VMat will be extended by 3 columns, containing [1,2,3]
  (constant).
*/

class VecExtendedVMatrix : public RowBufferedVMatrix
{
  typedef RowBufferedVMatrix inherited;

public:
  //! The fieldinfos of the underlying are copied, the extension fieldinfos
  //! are left empty (fill them yourself)
  VecExtendedVMatrix(VMat underlying, Vec extend_data);

  virtual void getRow(int i, Vec v) const;
  virtual void reset_dimensions() {
    underlying_->reset_dimensions();
    width_ = underlying_.width() + extend_data_.length();
    length_ = underlying_.length();
  }

protected:
  VMat underlying_;
  Vec extend_data_;
};


/*!   VMatrix that can be used to uniformize (between a and b)
  each feature in index of the underlying distribution such that:
  P(x') = .5   if  a < x'< b
        =  0   otherwise
  
  We suppose that the original distribution of x, P(x), could be anything,
  and we map "a" with bins[0] and "b" with bins[N-1].
*/
class UniformizeVMatrix: public RowBufferedVMatrix
{
protected:
  VMat distr;
  Mat bins;
  Vec index;
  real a;
  real b;

public:
  //! The original VMFields are copied upon construction
  UniformizeVMatrix(VMat the_distr, Mat the_bins, Vec the_index,
                    real the_a=0.0, real the_b=1.0);

  virtual void getRow(int i, Vec v) const;
  virtual void reset_dimensions()
    { distr->reset_dimensions(); width_=distr->width(); length_=distr->length(); }
};

//!  A VMatrix that exists in a .pmat file (native plearn matrix format, same as for Mat)
class FileVMatrix: public VMatrix
{
  typedef VMatrix inherited;
 protected:
  string filename_;
  FILE* f;
  bool file_is_bigendian;
  bool file_is_float;

 public:
  FileVMatrix() {};
  FileVMatrix(const string& filename); //!<  opens an existing file
  FileVMatrix(const string& filename, int the_length, int the_width); //!<  create a new matrix file

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;

  virtual void put(int i, int j, real value);
  virtual void putSubRow(int i, int j, Vec v);
  virtual void appendRow(Vec v);

  virtual void build();
  void build_();

  DECLARE_NAME_AND_DEEPCOPY(FileVMatrix);

  static void declareOptions(OptionList & ol);

  virtual ~FileVMatrix();
};

DECLARE_OBJECT_PTR(FileVMatrix);

/*!   But it is designed to keep a compact representation.
  It encodes only non zero values, using one byte for small integers,
  and 4-byte floating points for all other values.
  This representation should be reasonably good for both sparse matrices,
  and matrices containing categorical data (represented by small integers)
  possibly with one hot representations.
*/

class VecCompressor
{
protected:

  static inline bool issmallint(real x)
  { int intx = int(x); return floor(x)==x && intx<=127 && intx>=-127; }

  static inline bool is0(real x)
  { return x==0.; }

  static inline bool isI(real x)
  { return x!=0. && issmallint(x); }

  static inline bool isF(real x)
  { return x!=0. && !issmallint(x); }
  
public:
/*!     writes v in a compressed form in the data buffer passed as argument.
    (make sure enough memory is allocated in the data buffer)
    returns a pointer to the one-after-last element written in the data block
*/
  static signed char* compressVec(const Vec& v, signed char* data);

  //!  uncompresses the data of a vector compressed with compressVec
  //!  v must have the correct size.
  static void uncompressVec(signed char* data, const Vec& v);  

  //!  writes v in compressed format to the given stream
  //!  The written data does not contain size info.
  static void writeCompressedVec(ostream& out, const Vec& v);

  //!  reads data of a compressed vector from the given stream
  //!  v must have the right size already (this is not checked!)
  static void readCompressedVec(istream& in, const Vec& v); 

  //!  Returns the number of bytes that will be used to encode a vector of 
  //!  length n in the worst case
  static size_t worstCaseSize(int n)
  { return 2+4*n+n/128; }
};


/*!   Like MemoryVMatrix this class holds the data in memory.
  But it is designed to keep a compact representation.
  Each row is compressed/decompressed through the methods of VecCompressor
  (it's the same encoding of row vectors that is used in the DiskVMatrix)
  This encodes only non zero values, using one byte for small integers,
  and 4-byte floating points for all other values.
  This representation should be reasonably good for both sparse matrices,
  and matrices containing categorical data (represented by small integers)
  possibly with one hot representations.
*/
  class CompressedVMatrix: public RowBufferedVMatrix
  {
  protected:
    int max_length; //!<  maximum number of rows that can be appended to the initially empty matrix

    signed char* data;
    signed char** rowstarts;
    signed char* dataend; //!<  data+memory_alloc
    
    //!  next writing position in data chunk    
    signed char* curpos; 
    
/*!       This initializes the matrix with a length of 0 (but up to the_max_length rows can be appended)
      A memory_alloc of length*(2 + 4*width + width/128) should be enough in every case
      (and much less is needed for matrices containing a lot of 0 or small integers)
*/
    void init(int the_max_length, int the_width, size_t memory_alloc);

  public:

    
/*!       This initializes the matrix with a length of 0 (but up to the_max_length rows can be appended)
      If no memory_alloc value is given, a sufficient default value will be used initially. 
      You can always call reallocateCompactMemory() later to use the minimum memory
*/
    CompressedVMatrix(int the_max_length, int the_width, size_t memory_alloc=0)
    { init(the_max_length, the_width, memory_alloc!=0 ?memory_alloc :the_max_length*VecCompressor::worstCaseSize(the_width)); }
    
/*!       This initializes a matrix from the data of another. 
      If no memory_alloc value is given, a sufficient default value will be used initially. 
      You can always call reallocateCompactMemory() later to use the minimum memory
*/
    CompressedVMatrix(VMat v, size_t memory_alloc=0);

    virtual ~CompressedVMatrix();
    virtual void getRow(int i, Vec v) const;
    virtual void appendRow(Vec v);

    //!  returns the number of bytes allocated for the data 
    size_t memoryAllocated() { return dataend-data; }

    //!  returns the memory actually used for the data
    size_t memoryUsed() { return curpos-data; }    

    //!  This will reallocate the data chunk to be exactly of the needed size, so that no memory will be wasted.
    virtual void compacify();
};

//!  A VMatrix whose (compressed) data resides in a directory and can span several files.
//!  Each row is compressed/decompressed through the methods of VecCompressor
class DiskVMatrix: public RowBufferedVMatrix
{
  typedef RowBufferedVMatrix inherited;
protected:
  mutable fstream * indexf;
  mutable Array<fstream*> dataf;
  bool readwritemode;
  bool freshnewfile;
public:
  string dirname;
  
  DiskVMatrix(){};

/*!     Opens an existing one. If directory does not exist or has missing files, an error is issued.
    If readwrite is true, then the files are opened in read/write mode and appendRow can be called.
    If readwrite is false (the default), then the files are opened in read only mode, and calling appendRow 
    will issue an error.
*/
  DiskVMatrix(const string& the_dirname, bool readwrite=false); 

/*!     Create a new one. 
    If directory already exist an error is issued
    (you may consider calling force_rmdir prior to this.)
    Howver if it is a file then the file is erased and replaced by a new directory
    (this was to allow TmpFilenames to be used with this class).
    Files are opened in read/write mode so appendRow can be called.
*/
  DiskVMatrix(const string& the_dirname, int the_width, bool write_double_as_float=false);

  virtual void getRow(int i, Vec v) const;
  virtual void putRow(int i, Vec v);
  virtual void appendRow(Vec v);

  static void writeRow(ostream& out, const Vec& v)
  { VecCompressor::writeCompressedVec(out,v); }
 
  static void readRow(istream& in, const Vec& v)
  { VecCompressor::readCompressedVec(in, v); }

  virtual void build();
  void build_();

  DECLARE_NAME_AND_DEEPCOPY(DiskVMatrix);

  static void declareOptions(OptionList & ol);
  
  virtual ~DiskVMatrix();
};

DECLARE_OBJECT_PTR(DiskVMatrix);

//!  Silly (but sometimes useful) VMat class that sees the last row as the
//!  first, the second last as the second, etc...
class UpsideDownVMatrix: public VMatrix
{
 protected:
  VMat distr;

 public:
  //! The original VMFields are copied upon construction
  UpsideDownVMatrix(VMat the_distr);
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void put(int i, int j, real value);
  virtual void putSubRow(int i, int j, Vec v);
};

class UniformVMatrix: public VMatrix
{
 protected:
  real minval;
  real maxval;
  
 public:
  UniformVMatrix(real the_minval, real the_maxval, int the_width);    
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
};

//!  Outputs scalar samples (length 1) starting at start, up to end (inclusive) with step. When end is reached it starts over again. 

class RangeVMatrix: public VMatrix
{
 protected:
  real start;
  real end;
  real step;

 public:
  RangeVMatrix(real the_start, real the_end, real the_step=1.0);
  virtual real get(int i, int j) const;
};

class ConcatColumnsVMatrix: public RowBufferedVMatrix
{
  typedef RowBufferedVMatrix inherited;
 protected:
  Array<VMat> array;
  
 public:
  //! The lists of VMFields are appended upon construction.  The case where
  //! some VMat may have some fields and others not is handled properly.
  ConcatColumnsVMatrix(Array<VMat> the_array=Array<VMat>()) : array(the_array) { if (array.size()) build_(); };
  ConcatColumnsVMatrix(VMat d1, VMat d2) : array(d1, d2) { build_(); };
  virtual void getRow(int i, Vec samplevec) const; 
  virtual real getStringVal(int col, const string & str) const;
  virtual string getValString(int col, real val) const;
  virtual void reset_dimensions() { PLERROR("ConcatColumnsVMatrix::reset_dimensions() not implemented"); }
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;

  DECLARE_NAME_AND_DEEPCOPY(ConcatColumnsVMatrix);
  static void declareOptions(OptionList &ol);
  virtual void build();
  void build_();
};

DECLARE_OBJECT_PTR(ConcatColumnsVMatrix);

class ByteMemoryVMatrix: public VMatrix
{
 protected:
  unsigned char* data;
  Vec scale;//!<  sample = (data[i]+offset_[i])*scale[i];
  Vec offset_;

 public:
  ByteMemoryVMatrix(unsigned char* data,int the_length,int the_width, double scaling_factor=1.0, double offset_=0.0);
  ByteMemoryVMatrix(unsigned char* data,int the_length,int the_width, Vec scale);
  ByteMemoryVMatrix(unsigned char* data,int the_length,int the_width, Vec scale, Vec offset_);
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
};

/*!   like PairVMatrix but samples from all the
  pairs in order (traversing all the nxn pairs),
  outputs a Vec that is the concatenation of 
  the i-th row of data1 and j-th row of data2. The
  j-index moves faster than the i-index.
*/
class PairsVMatrix: public RowBufferedVMatrix
{
 protected:
  Mat data1;
  Mat data2;

 public:

  PairsVMatrix(Mat the_data1, Mat the_data2);
  virtual void getRow(int ij, Vec samplevec) const;
  virtual void reset_dimensions() { PLERROR("PairsVMatrix::reset_dimensions() not implemented"); }
};

/*!   This class concatenates several distributions:
  it samples from each of them in sequence, i.e.,
  sampling all the samples from the first distribution,
  then all the samples from the 2nd one, etc...
  This works only for distributions with a finite 
  number of samples (length()!=-1).
*/
class ConcatRowsVMatrix: public VMatrix
{
  typedef VMatrix inherited;
 protected:
  Array<VMat> array;

  //!  returns the index of the correct VMat in the array and the the row
  //!  number in this VMat that correspond to row i in the ConcatRowsVMat
  void getpositions(int i, int& whichvm, int& rowofvm) const; 

 public:
  //! The fields names are copied from the FIRST VMat
  ConcatRowsVMatrix(Array<VMat> the_array = Array<VMat>()) : array(the_array) { if (array.size()) build_(); };
  ConcatRowsVMatrix(VMat d1, VMat d2) : array(d1, d2) { build_(); };

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual real getStringVal(int col, const string & str) const;
  virtual void reset_dimensions() 
    { 
      for (int i=0;i<array.size();i++) array[i]->reset_dimensions(); 
      width_=array[0]->width();
      length_=0;
      for (int i=0;i<array.size();i++) 
        {
          if (array[i]->width()!=width_) 
            PLERROR("ConcatRowsVMatrix: underlying-distr %d has %d width, while 0-th has %d",
                  i,array[i]->width(),width_);
          length_ += array[i]->length();
        }
    }
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;

  DECLARE_NAME_AND_DEEPCOPY(ConcatRowsVMatrix);
  static void declareOptions(OptionList &ol);
  virtual void build();
  void build_();
};

DECLARE_OBJECT_PTR(ConcatRowsVMatrix);

/*!   This class concatenates several (virtual) subVMatrices of the same
  underlying VMatrix. For each sub-vmatrix block, the user
  specifies the starting row and the number of rows in the
  underlying VMatrix.
  The resulting vmatrix sees first all the rows of the
  first sub-vmatrix, then all the rows of the 2nd, etc...
*/
class ConcatRowsSubVMatrix: public VMatrix
{
 protected:
    VMat distr;
    TVec<int> start;
    TVec<int> len;

    void check(); //!<  check ranges are compatible

    //!  returns the index of the correct sub-VMat in the array and the the row
    //!  number in this VMat that correspond to row i in the ConcatRowsVMat
    void getpositions(int i, int& whichvm, int& rowofvm) const; 

  public:
    //!  The field names of the parent VMat are copied upon construction
    ConcatRowsSubVMatrix(VMat the_distr, TVec<int>& the_start, TVec<int>& the_len);
    ConcatRowsSubVMatrix(VMat the_distr, int start1, int len1, int start2, int len2);

    virtual real get(int i, int j) const;
    virtual void getSubRow(int i, int j, Vec v) const;
    virtual void reset_dimensions() 
    { 
      distr->reset_dimensions();
      width_=distr->width();
      length_=0;
      for (int i=0;i<len.length();i++) 
        length_ += len[i];
    }
    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;
};

/*!   This class interleaves several VMats (with consecutive rows
  always coming from a different underlying VMat) thus possibly
  including more than once the rows of the small VMats.
  For example, if VM1.length()==10 and VM2.length()==30 then
  the resulting VM will have 60 rows, and 3 repetitions
  of each row of VM1, with rows taken as follows: 
   VM1.row(0), VM2.row(0), VM1.row(1), VM2.row(1), ..., 
   VM1.row(9), VM2.row(9), VM1.row(0), VM2.row(10), ...
  Note that if VM2.length() is not a multiple of VM1.length()
  some records from VM1 will be repeated once more than others.
*/
class InterleaveVMatrix: public VMatrix
{
  protected:
    Array<VMat> vm;

  public:
  //! The field names are copied from the first VMat in the array
  InterleaveVMatrix(Array<VMat> distributions);

  //! The field names are copied from the first VMat d1
  InterleaveVMatrix(VMat d1, VMat d2);
  
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void reset_dimensions() 
    { 
      for (int i=0;i<vm.size();i++) vm[i]->reset_dimensions(); 
      width_=vm[0]->width();
      int maxl = 0;
      int n=vm.size();
      for (int i=0;i<n;i++) 
        {
          if (vm[i]->width()!=width_) 
            PLERROR("InterleaveVMatrix: underlying-distr %d has %d width, while 0-th has %d",
                  i,vm[i]->width(),width_);
          int l= vm[i]->length();
          if (l>maxl) maxl=l;
        }
      length_=n*maxl;
    }
};


/*!   This VMat is built from another VMat
  Sampling from this VMat will return the corresponding
  sample from the underlying VMat with last element ('target_classnum') replaced
  by a vector of target_values of size nclasses in which only target_values[target_classnum] 
  is set to  hot_value , and all the others are set to cold_value
  In the special case where the VMat is built with nclasses==1, then it is assumed 
  that we have a 2 class classification problem but we are using a single valued target. 
  For this special case only the_cold_value is used as target for classnum 0 
  and the_hot_value is used for classnum 1
*/
class OneHotVMatrix: public RowBufferedVMatrix
{
 protected:
  VMat underlying_distr;
  int nclasses;
  real cold_value;
  real hot_value;

 public:
  //!  (see special case when nclasses==1 desribed above)
  //!  Warning: VMFields are NOT YET handled by this constructor
  OneHotVMatrix(VMat the_underlying_distr, int the_nclasses, real the_cold_value=0.0, real the_host_value=1.0);
  virtual void getRow(int i, Vec samplevec) const;
  virtual void reset_dimensions() 
    { 
      underlying_distr->reset_dimensions(); 
      width_=underlying_distr->width(); 
      length_=underlying_distr->length(); 
    }
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;
};

//!  This VMat is a generalization of OneHotVMatrix where all columns (given
//!  by the Vec index) are mapped, instead of just the last one.
class GeneralizedOneHotVMatrix: public RowBufferedVMatrix
{
 protected:
  VMat distr;
  Vec index;
  Vec nclasses;
  Vec cold_value;
  Vec hot_value;

 public:
  //!  Warning: VMFields are NOT YET handled by this constructor
  GeneralizedOneHotVMatrix(VMat the_distr, Vec the_index, Vec the_nclasses,
                           Vec the_cold_value, Vec the_host_value);

  virtual void getRow(int i, Vec samplevec) const;
  virtual void reset_dimensions() 
    { 
      distr->reset_dimensions(); 
      width_=distr->width(); 
      length_=distr->length(); 
    }
};

/*!   This VMat is built from another VMat and a 'mapping' matrix that indicates how the elements of the 
  last column should be remapped. It's a sort of generalisation of the OneHot VMatrix, which can also
  be used to do grouping of classes...
  There are two available modes with two different associated constructors:
*/

/*!   1) An exhaustive mapping can be specified with the help of a mapping matrix.
     The 'mapping' matrix should specify for each possible value that the last column of the original VMat can take, 
     what value or vector of values it should be replaced with.
     Ex: if you have a VMat containing samples from 6 classes with classnum (0..5) indicated in the last column, 
     and wish to remap classes 1 and 2 to vector (0,1), classes 3 and 4 to vector (1,0) 
     and classes 0 and 5 to vector (0.5, 0.5) you could use the following 'mapping' matrix:
     1  0 1
     2  0 1
     3  1 0
     4  1 0
     0  .5 .5
     5  .5 .5
     In case a value is found in the last column for which no mapping is defined, an error is issued
*/

/*!   2) Alternatively, you can specify a triplet of values (if_equals_value,
  then_value, else_value) in which case the last column value will be
  replaced by (value==if_equals_value ?then_value :else_value)
*/


class RemapLastColumnVMatrix: public RowBufferedVMatrix
{
 protected:
    VMat underlying_distr;

    //!  If this is not empty, then it represents the mapping to apply
    Mat mapping;

/*!       These are used only if mapping is an empty Mat, in which case the
      value in the last column will be replaced by 'then_val' if it is
      equal to 'if_equals_val', otherwise it will be replaced by
      'else_val'
*/
    real if_equals_val;
    real then_val;
    real else_val;

  public:
    //!  full explicit mapping.
    //!  Warning: VMFields are NOT YET handled by this constructor
    RemapLastColumnVMatrix(VMat the_underlying_distr, Mat the_mapping);

    //!  if-then-else mapping.
    //!  Warning: VMFields are NOT YET handled by this constructor
    RemapLastColumnVMatrix(VMat the_underlying_distr, real if_equals_value, real then_value=+1, real else_value=-1);    

    virtual void getRow(int i, Vec samplevec) const;
    virtual void reset_dimensions() 
    { 
      underlying_distr->reset_dimensions(); 
      width_=underlying_distr->width(); 
      length_=underlying_distr->length(); 
    }
};

/*!   This VMat is a concatenation of 2 VMat.  The row of vm2, corresponding
  to the index set by col1 of vm1, is merged with vm1.
  
  for i=1,vm1.length()
    vm.row(i) <- vm1.row(i) + vm2.row(vm1(i,col1))
  
*/
class CrossReferenceVMatrix: public VMatrix
{
 protected:
  VMat vm1;
  int col1;
  VMat vm2;

 public:
  //! The column headings are simply the concatenation of the headings in
  //! the two vmatrices.
  CrossReferenceVMatrix(VMat v1, int c1, VMat v2);
  virtual void getRow(int i, Vec samplevec) const;
  virtual real get(int i, int j) const;
  virtual void reset_dimensions() { PLERROR("CrossReferenceVMatrix::reset_dimensions() not implemented"); }
};

/*!   A distribution to which some dates (or more generally time stamps) have
  been associated to each data row. Special methods then allow
  to take sub-distributions for particular intervals of dates.
  This is an abstract class: subclasses specify what time units
  are available and the semantics of date intervals.
*/
class DatedVMatrix: public VMatrix
{
public:
  DatedVMatrix(int width, int length) : VMatrix(width,length) {}

/*!     this one calls one of subDistrRelative{Years,Months,Days} according
    to wether units=="years", "months", or "days" (or if the first letter
    matches, irrespective of upper/lower case distinctions)
*/
  virtual VMat subDistrRelativeDates(int first, int n, const string& units) = 0;

/*!     this one calls one of subDistrRelative{Years,Months,Days} according
    to wether units=="years", "months", or "days" (or if the first letter
    matches, irrespective of upper/lower case distinctions)
*/
  virtual VMat subDistrAbsoluteUnits(int year, int month, int day, int n_units,
      const string& units) = 0;

  //!  return "size" in the given units (e.g. interval in years, months, etc...)
  virtual int lengthInDates(const string& units) = 0;

  //!  return row position of example whose relative date is the first with
  //!  the given (relative) value, in the given time units
  virtual int positionOfRelativeDate(int first, const string& units) = 0;

  //!  return the number of real fields required to specify a date 
  virtual int nDateFields() = 0;

  //!  copy the date fields for the relative positions starting at the
  //!  given row position for the given number of rows, into the given matrix
  virtual void copyDatesOfRows(int from_row, int n_rows, Mat& dates) = 0;
  
  // added by Julien Keable :
  // returns vector of row at indice 'row'
  // and associated date trough year, month and day
  virtual Vec copyRowDataAndDate(int row, int &year, int &month, int &day)=0;
  virtual void copyDateOfRow(int row, int &year, int &month, int &day)=0;
};
 
/*!   A DatedVMatrix that knows about years, months, and days,
  and for which the relative differences between two dates (e.g. days)
  depends on the number of different values of that unit (e.g. the
  number of days which actually occur).
*/
class YMDDatedVMatrix: public DatedVMatrix
{
public:
  VMat data;
protected:
  Mat years; //!<  single column of years (e.g. 1987), one year per data row
  Mat months; //!<  single column of months (between 1 and 12), one month per data row
  Mat days; //!<  single column of days (between 1 and 31), one day per data row
  Vec pos_of_ith_year; //!<  row index of first row of i-th year in the data
  Vec pos_of_ith_month; //!<  row index of first row of i-th month in the data
  Vec pos_of_ith_day; //!<  row index of first row of i-th month in the data
  Vec day_of_ith_pos; //!<  inverse map of pos_of_ith_day
  Array< TMat<int> > pos_of_date; //!<  dates[year-years(0,0)][month-1][day-1] is the 
                                        //!  position of the absolute (year,month,day) date

  void init();

public:
  //!  THE DATES MUST BE IN INCREASING CHRONOLOGICAL ORDER.
  //!  Warning: VMFields are NOT YET handled by this constructor
  YMDDatedVMatrix(VMat data_, Mat years_, Mat months_, Mat days_);

  //!  alternatively, the given matrix has (year, month, day) in the
  //!  first 3 columns and the rest of the data in the remaining columns.
  //!  Warning: VMFields are NOT YET handled by this constructor
  YMDDatedVMatrix(Mat& YMD_and_data);

  //!  return the number of real fields required to specify a date: here 3 for YMD
  int nDateFields() { return 3; }

/*!     this one calls one of subDistrRelative{Years,Months,Days} according
    to wether units=="years", "months", or "days" (or if the first letter
    matches, irrespective of upper/lower case distinctions)
*/
  VMat subDistrRelativeDates(int first, int n, const string& units);
  VMat subDistrRelativeYears(int first_relative_year, int n_years);
  VMat subDistrRelativeMonths(int first_relative_month, int n_months);
  VMat subDistrRelativeDays(int first_relative_day, int n_days);

  //!  sub-distribution starting at the given date, for the given
  //!  number of occured years, months or days
  VMat subDistrAbsoluteYears(int year, int month, int day, int n_years);
  VMat subDistrAbsoluteMonths(int year, int month, int day, int n_months);
  VMat subDistrAbsoluteDays(int year, int month, int day, int n_days);
  VMat subDistrAbsoluteUnits(int year, int month, int day, int n_units, const string& units);

  //!  return "size" in the given units (e.g. interval in years, months, etc...)
  //!  here e.g. the number of different years (if units=="years").
  int lengthInDates(const string& units);

  //!  return row position of example whose relative date is the first with
  //!  the given (relative) value, in the given time units
  int positionOfRelativeDate(int first, const string& units);

  //!  return row position associated with first sample whose date
  //!  is given
  int positionOfDate(int year, int month, int day);

  //!  copy the date fields for the relative positions starting at the
  //!  given row position for the given number of rows, into the given matrix
  void copyDatesOfRows(int from_row, int n_rows, Mat& dates);
  
  // added by Julien Keable :
  // returns vector of row at indice 'row'
  // and associated date trough year, month and day
  Vec copyRowDataAndDate(int row, int &year, int &month, int &day);
  void copyDateOfRow(int row, int &year, int &month, int &day);

  virtual void reset_dimensions() { PLERROR("YMDDatedVMatrix::reset_dimensions() not implemented"); }
};

/*!   Build an array of distributions which are SubRowDistributions of
  a given distribution, made up of consecutive blocks of approximately
  the same type, but where each block is a sequence of items terminating
  by a special "separator" value.
*/
class SentencesBlocks : public Array<VMat> {
public:
  SentencesBlocks(int n_blocks, VMat d, Vec separator);
};


/*!   *********************************
  * user-friendly VMat interface *
  *********************************
*/

inline Array<VMat> operator&(const VMat& d1, const VMat& d2)
{ return Array<VMat>(d1,d2); }

#define MEAN_ROW 0
#define STDDEV_ROW 1
#define MIN_ROW 2
#define MAX_ROW 3
#define NMISSING_ROW 4
#define NZERO_ROW 5
#define NPOSITIVE_ROW 6
#define NNEGATIVE_ROW 7
#define MEANPOS_ROW 8
#define STDDEVPOS_ROW 9

/*!   The returned Mat is structured as follows:
  row 0: mean
  row 1: stddev
  row 2: min
  row 3: max
  row 4: nmissing
  row 5: nzero     (==0)
  row 6: npositive (>0)
  row 7: nnegative (<0)
  row 8: mean of positive
  row 9: stddev of positive
*/
Mat computeBasicStats(VMat m);

void computeMean(VMat d, Vec& meanvec);
void computeWeightedMean(Vec weights, VMat d, Vec& meanvec);
void computeMeanAndVariance(VMat d, Vec& meanvec, Vec& variancevec);
void computeMeanAndStddev(VMat d, Vec& meanvec, Vec& stddevvec);
void computeMeanAndCovar(VMat d, Vec& meanvec, Mat& covarmat, ostream& logstream=cerr);
void computeWeightedMeanAndCovar(Vec weights, VMat d, Vec& meanvec, Mat& covarmat);

/*!   Computes conditional mean and variance of each target, conditoned on the
  values of categorical integer input feature.  The basic_stats matrix may
  be passed if previously computed (see computeBasicStats) or an empty
  matrix may be passed otherwise, that will compute the basic statistics.
  
  An input feature #i is considered a categorical integer input if its min
  and max (as found in basic_stats) are integers and are not too far
  apart.  For these, the correponding returned array[i] matrix will
  contain max-min+1 rows (one for each integer value between min and max
  inclusive), each row containing the corresponding input value, the
  number of times it occured, and mean and variance for each target.  The
  returned matrix array[i] for input features that are not considered
  categorical integers are empty.
*/
Array<Mat> computeConditionalMeans(VMat trainset, int targetsize, Mat& basic_stats);

/*!   subtracts mean and divide by stddev
  meanvec and stddevvec can be shorter than d.width() 
  (as we probably only want to 'normalize' the 'input' part of the sample, 
  and not the 'output' that is typically present in the last columns)
*/
VMat normalize(VMat d, Vec meanvec, Vec stddevvec);
VMat normalize(VMat d, int inputsize, int ntrain); //!<  Here, mean and stddev are estimated on d.subMat(0,0,ntrain,inputsize)
inline VMat normalize(VMat d, int inputsize) { return normalize(d,inputsize,d.length()); }

/*!   If exclude==false (the default) 
    returns a VMat containing only the rows
    whose column col has a value that belongs 
    to the given set of authorized values
  If exclude==true
    returns a VMat with all the other rows 
    (corresponds to grep -v)
  [MISSING_VALUE is a possible value and is handled correctly]
*/
VMat grep(VMat d, int col, Vec values, bool exclude=false);

//! returns a map mapping all different values appearing in column col to their number of occurences
map< real, int> countOccurencesInColumn(VMat m, int col);

//! returns a map mapping all different values appearing in column col to a vector of the corresponding row indices in the VMat
//! (this proceeds in 2 passes, first calling countOccurencesInColumn to allocate the exact memory)
map< real, TVec<int> > indicesOfOccurencesInColumn(VMat m, int col);

/*!   Same as above, except that the indexes of the rows are stored on disk
  rather than in memory
  (a SelectRowsFileIndexVMatrix is returned rather than a SelectRowsVMatrix)
  BEWARE: If the indexfile already exists, it is *not* recomputed, but used as is.
*/
VMat grep(VMat d, int col, Vec values, const string& indexfile, bool exclude=false);

/*!   returns a VMat that contains only the lines that do not have any MISSING_VALUE
  The indexes of the rows of the original matrix are recorded in the indexfile
  BEWARE: If the indexfile already exists, it is *not* recomputed, but used as is.
*/
VMat filter(VMat d, const string& indexfile);

/*!   Splits the dataset d into a train and a test subset
  If test_fraction is <1.0 then the size of the test subset is set to be ntest = int(test_fraction*d.length())
  If test_fraction is >=1.0 then ntest = int(test_fraction)
  Last argument i allows to get the ith split of a K-fold cross validation
  i = 0 corresponds to having the last ntest samples used as the test set
  i = 1 means having the ntest samples before those as the test set, etc...
*/
void split(VMat d, real test_fraction, VMat& train, VMat& test, int i=0);

//!  Splits the dataset d into 3 subsets 
void split(VMat d, real validation_fraction, real test_fraction, VMat& train, VMat& valid, VMat& test,bool do_shuffle=false);
                                    
/*!   Splits the dataset d into a train and a test subset
  randomly picking which samples should be in each subset.
  (test_fraction has the same meaning a sabove).
  Return the permuted indices (the last ntest of which are the test indices
  and the remainder are the train indices).
*/
Vec randomSplit(VMat d, real test_fraction, VMat& train, VMat& test);

//!  Splits the dataset d into 3 subsets (similar to above)
void randomSplit(VMat d, real validation_fraction, real test_fraction, VMat& train, VMat& valid, VMat& test);

//!  returns a SelectRowsVMatrix that has d's rows shuffled
VMat shuffle(VMat d);

//! returns a SelectRowsVMatrix that has d's rows bootstrapped (sample with replacement
//! and optionally re-ordered). Optionally the repeated rows are eliminated (this is actually
//! done by shuffling and taking the first 2/3 of the rows, so the length() will be always the
//! same).  Note that the default values are fine for "on-line"
//! learning algorithms but does not correspond to the usual "bootstrap".
//! 
VMat bootstrap(VMat d, bool reorder=true, bool norepeat=true);

//!  computes M1'.M2
Mat transposeProduct(VMat m1, VMat m2);

//!  computes M'.M
Mat transposeProduct(VMat m);

/*!   computes the result of the linear regression into theta_t
  Parameters must have the following sizes:
  inputs(l,n)
  outputs(l,m)
  theta_t(n+1,m)
  XtX(n+1,n+1)
  XtY(n+1,m)
  The n+1 is due to the inclusion of the bias terms in the matrices (first row of theta_t)
  If use_precomputed_XtX_XtY is false, then they are computed. Otherwise
  they are used as they are (typically passed precomputed from a previous
  call made with a possibly different weight_decay)
*/
void linearRegression(VMat inputs, VMat outputs, real weight_decay, Mat theta_t, 
                      bool use_precomputed_XtX_XtY, Mat XtX, Mat XtY, int verbose_computation_every=0);

//!  Version that does all the memory allocations of XtX, XtY and theta_t. Returns theta_t
inline Mat linearRegression(VMat inputs, VMat outputs, real weight_decay)
{
  int n = inputs.width()+1;
  int n_outputs = outputs.width();
  Mat XtX(n,n);
  Mat XtY(n,n_outputs);
  Mat theta_t(n,n_outputs);
  linearRegression(inputs, outputs, weight_decay, theta_t, false, XtX, XtY);
  return theta_t;
}

//!  Linear regression where each input point is given a different importance weight (the gammas)
void weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas,
    real weight_decay, Mat theta_t, bool use_precomputed_XtX_XtY, Mat XtX,
    Mat XtY, int verbose_computation_every=0);

/*!   Rebalance the input VMatrix so that each class has a probability 1/nclasses.
  Also, the return VMat class alternates between all classes cyclicly.
  The resulting VMat is a SelectRowsFileIndexVMatrix which has its IntVecFile
  load if filename already exist, or computed if not. 
*/
VMat rebalanceNClasses(VMat inputs, int nclasses, const string& filename);

//!  Rebalance a 2-class VMat such as to keep all the examples of the
//!  dominant class.
void fullyRebalance2Classes(VMat inputs, const string& filename, bool save_indices=true);

//!  Version that does all the memory allocations of XtX, XtY and theta_t. Returns theta_t
inline Mat weightedLinearRegression(VMat inputs, VMat outputs, VMat gammas, real weight_decay)
{
  int n = inputs.width()+1;
  int n_outputs = outputs.width();
  Mat XtX(n,n);
  Mat XtY(n,n_outputs);
  Mat theta_t(n,n_outputs);
  weightedLinearRegression(inputs, outputs, gammas, weight_decay, theta_t, false, XtX, XtY);
  return theta_t;
}

/*!   This VMat is a SelectRowsVMatrix which, given a threshold date,
  keep only the rows earlier (or later) than this date.  The thresdold date
  is given as a YYYYMMDD date, and the date on the original VMatrix are kept
  on 1 column (YYYYMMDD) or 3 (YYYY, MM and DD).
*/
VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
      int yyyymmdd_col);
VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
      int yyyy_col, int mm_col, int dd_col);

inline VMat hconcat(VMat d1, VMat d2)
{ return new ConcatColumnsVMatrix(d1,d2); }

inline VMat hconcat(Array<VMat> ds)
{ return new ConcatColumnsVMatrix(ds); }

inline VMat vconcat(VMat d1, VMat d2)
{ return new ConcatRowsVMatrix(d1,d2); }

inline VMat vconcat(Array<VMat> ds)
{ return new ConcatRowsVMatrix(ds); }

inline VMat removeRows(VMat d, Vec rownums)
{ return new RemoveRowsVMatrix(d,rownums); }

inline VMat removeRow(VMat d, int rownum)
{ return new RemoveRowsVMatrix(d,Vec(1,rownum)); }

inline VMat vrange(real start, real end, real step=1.0)
{ return new RangeVMatrix(start,end,step); }

inline VMat onehot(VMat d, int nclasses, real cold_value=0.0, real hot_value=1.0)
{ return new OneHotVMatrix(d, nclasses, cold_value, hot_value); }

inline VMat remapLastColumn(VMat d, Mat mapping)
{ return new RemapLastColumnVMatrix(d, mapping); }

inline VMat remapLastColumn(VMat d, real if_equals_value, real then_value=1.0, real else_value=-1.0)
{ return new RemapLastColumnVMatrix(d, if_equals_value, then_value, else_value); }

inline ostream& operator<<(ostream& out, const VMat& m)
{ m.print(out); return out; }

template <> void deepCopyField(VMat& field, CopiesMap& copies);

//! Retirns the unconditional statistics of each field
TVec<StatsCollector> computeStats(VMat m, int maxnvalues);

//! returns the cooccurence statistics conditioned on the given field
PP<ConditionalStatsCollector> computeConditionalStats(VMat m, int condfield, TVec<RealMapping> ranges);

VMat loadAsciiAsVMat(const string& filename);

%> // end of namespace PLearn

#endif

