// -*- C++ -*-
// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
   * $Id: VMatrix.h,v 1.65 2005/01/20 21:49:12 tihocan Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef VMatrix_INC
#define VMatrix_INC

#include <cstdlib>
#include <map>
#include <plearn/base/PP.h>
#include <plearn/math/TMat.h>
#include <plearn/math/StatsCollector.h>
#include "VMField.h"

#include <plearn/base/stringutils.h>
#include <plearn/math/TMat_maths_impl.h>


namespace PLearn {
using namespace std;

class VMat;

/*! ** VMatrix ** */

class VMatrix: public Object
{
  
private:

  typedef Object inherited;
  friend class VMat;

  mutable FILE* lockf_; //! .lock file in metadatadir

  //! Used in the 'find' method to store a row.
  mutable Vec get_row;

  //! Used in the default dot(i,j) method to store the i-th and j-th rows.
  mutable Vec dotrow_1;
  mutable Vec dotrow_2;

protected:

  int length_;
  int width_;
  time_t mtime_; // time of "last modification" of files containing the data

  // For training/testing data sets we assume each row is composed of 3 parts
  // An input part, a target part, and a weight part
  // These fields give those parts' lengths
  // They are used by method VMat::

  int inputsize_;
  int targetsize_;
  int weightsize_;

  // are write operations tolerated?
  bool writable;

  //! Path of directory (possibly relative to DBDIR) that will contain meta information 
  //! on this dataset (fieldnames, cached statistics, etc...) and possibly the data itself.
  string metadatadir; 

  // [DEPRECATED] contains a short name that can be used as part of a filename for results associated with this dataset.
  string alias_;

  // New set of statistics fields:
  mutable TVec<StatsCollector> field_stats;  //!< stats[i] contains stats for field #i 

  // the string mapping for each fields, in both directions
  mutable TVec<map<string,real> > map_sr; 
  mutable TVec<map<real,string> > map_rs; 
  
private: 

  //! This does the actual building. 
  void build_();

public:

  mutable Array<VMField> fieldinfos; // don't use this directly (deprecated...) call getFieldInfos() instead
  Array<VMFieldStat> fieldstats;

  VMatrix();

  VMatrix(int the_length, int the_width);

  // simply calls inherited::build() then build_() 
  virtual void build();

  static void declareOptions(OptionList & ol);

  void init_map_sr() const { if (map_sr.length()==0) { map_sr.resize(width()); map_rs.resize(width()); } }

  // Data-set info
  // Sample parts sizes
  

  inline void defineSizes(int inputsize, int targetsize, int weightsize=0)
  { inputsize_ = inputsize, targetsize_ = targetsize, weightsize_ = weightsize; }

  //! Copy the values of inputsize, targetsize and weightsize from the source
  //! matrix m.
  void copySizesFrom(const VMat& m);

  //! Sets all meta info (length_, width_, inputsize_, targetsize_, weightsize_,
  //! fieldnames, ...) that is not already set, by copying it from the source's
  //! Vmat vm. Modification time is also set to the latest of the current mtime
  //! of this vmat and of the mtime of the source.
  virtual void setMetaInfoFrom(const VMat& vm);

  //! Return true iif it looks like the same matrix, i.e. it has same sizes, width and length.
  bool looksTheSameAs(const VMat& m);
  
  inline int inputsize() const { return inputsize_; }
  inline int targetsize() const { return targetsize_; }
  inline int weightsize() const { return weightsize_; }
  inline bool hasWeights() const { return weightsize_>0; }

  //! Default version calls getSubRow based on inputsize_ targetsize_ and weightsize_
  //! But exotic subclasses may construct, input, target and weight however they please.
  //! If not a weighted matrix, weight should be set to default value 1.
  virtual void getExample(int i, Vec& input, Vec& target, real& weight);

  /*! NOTE: How to handle exotic cases of data-sets whose input or target are not standard Vecs:
    The idea is to still have the getExample build and return Vecs, but the representation of these Vecs 
    will have a special format, detected and understood by a specialised Learner or specialised Variables:
       Hack: format des Vec compris par un Learner:
           Si v[0] == SPECIAL_FORMAT
              v[1] indique le format de ce qui suit (v[2] ...):
                   0 sparse vec de la forme: length nvals i val i val ...
                   1 pointeur vers un Object de la forme: ptr  (cast du float ou du double)
                   2 tenseur plein de la forme: rank size_1...size_n val ... 
                   3 tenseur sparse de la forme: rank size_1...size_n nvals i_1...i_n val i1...i_n val ...
  */
  #define SPECIAL_FORMAT ((real)3.1e36)

  // Field types...
  void setFieldInfos(const Array<VMField>& finfo);
  //! returns true if fieldinfos have been set
  bool hasFieldInfos() const;
  //! If no fieldnames have been set, will set default field names to "0", "1", "2", ... i.e. their column index.
  Array<VMField>& getFieldInfos() const;
  Array<VMField> getSavedFieldInfos() const;
  VMField& getFieldInfos(int fieldindex) const { return getFieldInfos()[fieldindex]; }
  void declareField(int fieldindex, const string& fieldname, VMField::FieldType fieldtype=VMField::UnknownType);
  void declareFieldNames(TVec<string> fnames);

  //!< returns the column index corresponding to a fieldname
  //!<  or -1 if the name was not found
  int fieldIndex(const string& fieldname) const;

  //!< This first calls fieldIndex to try and get the index corresponding to the given string
  //|< If this fails, the given string is assumed to hold the numerical index, and its
  //!< conversion to int will be returned (or a PLEARNERROR issued if this fails).
  int getFieldIndex(const string& fieldname_or_num) const;

  string fieldName(int fieldindex) const { return getFieldInfos(fieldindex).name; } 
  TVec<string> fieldNames() const; // Returns the vector of field names
  void unduplicateFieldNames(); // add a numeric suffix to duplic. fieldNames (eg: field.1 field.2 etc..)

  VMField::FieldType fieldType(int fieldindex) const { return getFieldInfos(fieldindex).fieldtype; } 
  VMField::FieldType fieldType(const string& fieldname) const { return fieldType(fieldIndex(fieldname)); } 
  const VMFieldStat& fieldStat(int j) const { return fieldstats[j]; } 
  const VMFieldStat& fieldStat(const string& fieldname) const { return fieldStat(fieldIndex(fieldname)); }

  void printFields(ostream& out) const;
  void printFieldInfo(ostream& out, int fieldnum) const;
  void printFieldInfo(ostream& out, const string& fieldname_or_num) const;

  string fieldheader(int elementcharwidth=8);

  // loads/saves from/to the metadatadir/fieldnames file
  void saveFieldInfos() const;
  void loadFieldInfos() const;
  
  // these 3 functions deal with stringmaps, notes, and binning files (all three called special field info files, or 'SFIF')
  // for each field eventually, I (julien) guess all this info should be wrapped (thus saved, and loaded) in the VMField class

  // SFIFs, are by default located in the directory MyDataset.{amat,vmat,etc}.metadata/FieldInfo/ and are named 'fieldname'.{smap,notes,binning,...}. 
  // In all 3 functions, the parameter ext (given **with** the dot) specifies the extension of the special field info file [smap,notes,binning], and col
  // is the column index you refer to.

  // setSFIFFilename : sets the SFIF with extensions 'ext' to some 'string'. if this string is different
  // from the default filename, the string is actually placed in a new file called [dataset].metadata/FieldInfo/fieldname.[ext].lnk
  // if the 'string' is empty, the default SFIF filename is assumed, which is : [MyDataset].metadata/FieldInfo/fieldname.[ext]
  void setSFIFFilename(int col, string ext, string filepath="");
  void setSFIFFilename(string fieldname, string ext, string filepath="");

  // getSFIFFilename :If a '*.vmat' dataset uses fields from another dataset, how can we keep the field info dependency? To resolve 
  // this issue, a file named __default.lnk containing path 'P' can be placed in the FieldInfo directory of the .vmat. Here's how 
  // the function getSFIFFilename search for a file : if the default SFIF file doesn't exist, it will then search for the default filename +'.lnk'. 
  // if the later neither exists, the __default.lnk file is used if present, and if not, then an empty (thus inexistent) file (with
  // SFIF default filename) is assumed. 
  string getSFIFFilename(int col, string ext);
  string getSFIFFilename(string fieldname, string ext);
  
  // isSFIFDirect : tells whether the SFIF filename is the default filename. (if false, means the field uses the SFIF from another dataset)
  bool isSFIFDirect(int col, string ext);
  bool isSFIFDirect(string fieldname, string ext);

  // string mapping stuff
  ///////////////////////

  // save all string mapings (one .smap file for each field)
  void saveAllStringMappings();
  
  // save a single field's string mapping in file 'fname'
  void saveStringMappings(int col,string fname);

  //! adds a string<->real mapping
  void addStringMapping(int col, string str, real val);

  //! adds a string<->real mapping for a new string, if it doesn't already have one and returns the associated value
  //! if the string doesn'a already have an associated value, it will be associated with value -100-number_of_strings_already_in_the_map
  real addStringMapping(int col, string str);

  //! remove all string mappings
  void removeAllStringMappings();

  //! remove all string mappings of a given field 
  void removeColumnStringMappings(int c);

  //! removes a single string mapping
  void removeStringMapping(int col, string str);

  //! overwrite the string<->real mapping with this one (and build the reverse mapping)
  void setStringMapping(int col, const map<string,real>& zemap);

  //! voids string mapping for column i
  void deleteStringMapping(int col);

  //! loads the appropriate string map file for column 'col'
  void loadStringMapping(int col);

  //! loads the appropriate string map file for every column
  void loadAllStringMappings();

  //! Copy all string mappings from a given VMat.
  void copyStringMappingsFrom(VMat source);

  //! returns the string associated with value val 
  //! for field# col. Or returns "" if no string is associated.
  virtual string getValString(int col, real val) const;

  //! returns the string->value mapping for field 'fld'
  virtual const map<string,real>& getStringToRealMapping(int col) const;

  //! returns the value->string mapping for field 'fld'
  virtual const map<real,string>& getRealToStringMapping(int col) const;

  //! returns value associated with a string (or MISSING_VALUE if there's no association for this string)
  virtual real getStringVal(int col, const string & str) const;

  //! returns element as a string, even if value doesn't map to a string, in which case tostring(value) is returned
  virtual string getString(int row, int col) const;

  //! Copy row i (converted to string values, using string mappings when they exist) into v.
  virtual void getRowAsStrings(int i, TVec<string> v_str) const;

  //! Return the dimension of the values for a certain field, -1 if continuous  
  virtual int getDimension(int row, int col) const;

  ////////////////////////
  
  virtual void computeStats(); 
  bool hasStats() const { return fieldstats.size()>0; }
  void saveStats(const string& filename) const;
  void loadStats(const string& filename);

  //! this should be called by the build method of every VMatrix that has a metadatadir
  //! It will create said directory if it doesn's already exist.
  //! Throws a PLERROR if called with an empty string
  virtual void setMetaDataDir(const string& the_metadatadir);

  //! Returns true if a metadatadir was set
  bool hasMetaDataDir() const { return metadatadir!=""; }

  //! Throws a PLERROR if no metadatadir was set.
  string getMetaDataDir() const; 

  //! Locks the metadata directory by creating a .lock file inside it.
  //! If such a file already exists, it is interpreted as being locked by some other process:
  //! this process will print to cerr that it is waiting for a lock on that directory,
  //! and will block and wait until the existing .lock is removed before recreating its own.
  //! Throws a PLearnError if called and metadatadir is not set, or lock is already held by this object 
  //! (i.e. this->lockMetaDataDir has already been called previously and no unlockMetaDataDir() was called).
  //! If the 'max_lock_age' option is given a value > 0, then the lock file will be ignored (and
  //! replaced by our own lock file) as soon as its modification date becomes older than 'max_lock_age'
  //! (in seconds).
  //! The 'verbose' option can be set to false to prevent useless output.
  void lockMetaDataDir(time_t max_lock_age = 0, bool verbose = true) const;

  //! Removes the .lock file inside the metadatadir.
  //! It will throw a PLearnError if this object did not hold the lock.
  void unlockMetaDataDir() const;

  //! returns the 'alias' for this dataset. The alias is a short name that 
  //! can be used as part of a filename containing results related to this VMat.
  string getAlias() const { return alias_; }
  void setAlias(const string& the_alias) { alias_ = the_alias; }

  //! returns the unconditonal statistics for all fields from the stats.psave file 
  //! (if the file does not exist, a default version is automatically created)
  TVec<StatsCollector> getStats() const;
  
  StatsCollector& getStats(int fieldnum) const
  { return getStats()[fieldnum]; }


  //! Returns the bounding box of the data, as a vector of min:max pairs. 
  //! If extra_percent is non 0, then the box is enlarged in both ends of every direction by 
  //! the given percentage (ex: if the data's x lies within [0,100] and extra_percent is 0.03
  //! then the returned bound pair will be -3:103 )
  TVec< pair<real,real> > getBoundingBox(real extra_percent=0.00) const;

  //! returns the ranges as defined in the ranges.psave file (for all fields)
  //! (if the ranges.psave file does not exist, a reasonable default version is created )
  TVec<RealMapping> getRanges();

  //! returns the cooccurence statistics conditioned on the given field
  //! (within the ranges returned by getRanges() )
  //! The results are cached in file stats#.psave (where # stands for the condfield index)
  // PP<ConditionalStatsCollector> getConditionalStats(int condfield);

  // default version calls savePMAT
  virtual void save(const string& filename) const;

  virtual void savePMAT(const string& pmatfile) const;
  virtual void saveDMAT(const string& dmatdir) const;

  //! Save the content of the matrix in the AMAT ASCII format into a file.
  //! If 'no_header' is set to 'true', then the AMAT header won't be saved,
  //! which can be useful to export data to other applications.
  //! If 'save_strings' is set to 'true', then the string mappings will be used
  //! so as to save strings where they exist (instead of saving the corresponding
  //! real value).
  virtual void saveAMAT(const string& amatfile, bool verbose = true,
                        bool no_header = false, bool save_strings = false) const;

  inline int width() const 
  { 
  #ifdef BOUNDCHECK
    if (!this) 
      PLERROR("VMATRIX::width() This object has pointer this=NULL");
  #endif 
    return width_; 
  }
  inline int length() const 
  {
  #ifdef BOUNDCHECK
    if (!this) 
      PLERROR("VMATRIX::length() This object has pointer this=NULL");
  #endif
    return length_;
 }

  inline bool isWritable() const { return writable; }

  // this function (used with .vmat datasets), is used to return the filename of fieldInfo files (string maps (.smap) and notes (.notes))
  // It recursively navigates through links until it finds a suitable file (.smap or .notes)
  // Idea : a .metadata/FieldInfo can contain one of these files :
  // (the order show here is the one used by the function to searches the file)

  // fieldName.smap.lnk : containing the actual path+target OR another .lnk file
  // fieldName.smap : the target (the actual string map or comment file)
  // __default.lnk : contains another FieldInfo directory to look for target (typically the 

  // ** Note 1: that target is assumed to be an inexistant file in the directory where none of the previous 3 can be found (since the file exists only when non-empty)
  // ** Note 2: source may not be target
  string resolveFieldInfoLink(string target, string source);

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

  //! For matrices stored on disk, this should flush all pending buffered write operations
  virtual void flush();

  //! will call putRow if i<length() and appendRow if i==length()
  void putOrAppendRow(int i, Vec v);

  //! will call putRow if i<length()
  //! if i>= length(), it will call appendRow with 0 filled rows as many times as necessary before 
  //! it can append row i 
  void forcePutRow(int i, Vec v);

  //!  These methods do not usually need to be overridden in subclasses
  //!  (default versions call getSubRow, which should do just fine)
  virtual void getRow(int i, Vec v) const; //!<  copies row i into v (which must have appropriate length equal to the VMat's width)

  virtual void putRow(int i, Vec v);
  virtual void fill(real value);
  virtual void getMat(int i, int j, Mat m) const; //!<  copies the submatrix starting at i,j into m (which must have appropriate length and width)
  virtual void putMat(int i, int j, Mat m); //!<  copies matrix m at position i,j of this VMat

  //!  copies column i into v (which must have appropriate length equal to the VMat's length)
  virtual void getColumn(int i, Vec v) const;

  //! Return true iff the input vector is in this VMat (we compare only the input part).
  //! If the parameter 'i' is provided, it will be filled with the index of the
  //! corresponding data point, or with -1 if it does not exist in this VMat.
  //! The 'tolerance' parameter indicates the maximum squared distance between two
  //! points to consider them as equal.
  virtual bool find(const Vec& input, real tolerance, int* i = 0) const;

/*!     returns a Mat with the same data as this VMat
    The default version of this method copies the data in a fresh Mat created in memory
    However this method will typically be overrided by subclasses (such as MemoryVMatrix) 
    whose internal representation is already a Mat in order to return this Mat directly to avoid 
    a new memory allocation and copy of elements. In this case, and in this case only, modifying 
    the elements of the returned Mat will logically result in modified elements in the original 
    VMatrix view of it. 
*/
  virtual Mat toMat() const;

  //!  The default implementation of this method does nothing,
  //!  but subclasses may overload it to reallocate memory to exactly what is needed and no more.
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

  //! Output the content of the VMat in the stream 'out'.
  void print(ostream& out) const;

  // virtual void oldwrite(ostream& out) const;
  // virtual void oldread(istream& in);

  PLEARN_DECLARE_ABSTRACT_OBJECT(VMatrix);
  void makeDeepCopyFromShallowCopy(CopiesMap& copies);


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

  //! Returns the possible values for a certain field in the VMatrix
  virtual Vec getValues(int row, int col) const {return Vec(0);}
 
  //! Gives the possible values of a certain field (column) given the input
  virtual Vec getValues(Vec input, int col) const {return Vec(0);}

  virtual ~VMatrix();
};

DECLARE_OBJECT_PTR(VMatrix);

} // end of namespace PLearn

#endif

