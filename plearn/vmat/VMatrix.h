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
 * $Id$
 ******************************************************* */


/*! \file VMatrix.h */

#ifndef VMatrix_INC
#define VMatrix_INC

//#include <cstdlib>
#include <plearn/base/Object.h>
#include <plearn/base/PP.h>
#include <plearn/math/TMat.h>
#include <plearn/math/StatsCollector.h>
#include "VMField.h"
#include <plearn/dict/Dictionary.h>
#include <plearn/io/PPath.h>

#include <map>

namespace PLearn {
using namespace std;

class VMat;

/**
 *  Base classes for virtual matrices
 *
 *  VMatrix provides an abstraction for a virtual matrix, namely a matrix
 *  wherein all element access operations are virtual. This enables a wide
 *  variety of matrix-like objects to be implemented, from simple data
 *  containers (e.g.  MemoryVMatrix), to large-scale matrices that don't fit in
 *  memory (e.g.  FileVMatrix), to on-the-fly calculations that are implemented
 *  through various processing VMatrices.
 *  
 *  For implementers, a simple class to derive from is RowBufferedVMatrix,
 *  which implements most of the functionalities of the abstract VMatrix
 *  interface in terms of a few simple virtual functions to be overridden by
 *  the user.
 */
class VMatrix: public Object
{
    typedef Object inherited;
    friend class VMat;

    /// .lock file in metadatadir
    mutable PStream lockf_; 

    /// Used in the 'find' method to store a row.
    mutable Vec get_row;

    /// Used in the default dot(i,j) method to store the i-th and j-th rows.
    mutable Vec dotrow_1;
    mutable Vec dotrow_2;
    time_t mtime_;          ///< Time of "last modification" of data files.
    time_t mtime_update;    

protected:

    mutable int length_;    ///< Length of the VMatrix.
    int width_;             ///< Width of the VMatrix.

    /// For training/testing data sets we assume each row is composed of 4
    /// parts: an input part, a target part, and a weight part.  These fields
    /// give those parts' lengths.
    mutable int inputsize_;
    mutable int targetsize_;
    mutable int weightsize_;
    mutable int extrasize_;

    /// Are write operations tolerated?
    bool writable;

    /// Path of directory that will contain meta information on this dataset
    /// (fieldnames, cached statistics, etc...) and possibly the data itself.
    PPath metadatadir;

    /// Statistics for each field.
    mutable TVec<StatsCollector> field_stats;  ///< stats[i] contains stats for field #i
    mutable TVec<PP<StatsCollector> > field_p_stats; //same, for remote calls

    /// The string mapping for each field, in both directions.
    mutable TVec<map<string,real> > map_sr;
    mutable TVec<map<real,string> > map_rs;

private:
    /// This does the actual building.
    void build_();

protected:
    /// Declare this class' options.
    static void declareOptions(OptionList & ol);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

public:
    // @TODO Move to protected / private if we don't want to use it directly
    mutable Array<VMField> fieldinfos;
    Array<VMFieldStat> fieldstats;

public:
    //#####  PLearn Object Protocol  ##########################################

    /// Default constructor.
    VMatrix(bool call_build_ = false);

    VMatrix(int the_length, int the_width, bool call_build_ = false);

    virtual ~VMatrix();

    /// Simply calls inherited::build() then build_().
    virtual void build();

    PLEARN_DECLARE_ABSTRACT_OBJECT(VMatrix);
    void makeDeepCopyFromShallowCopy(CopiesMap& copies);


    //#####  Field Types and Field Names  #####################################
    
    /// Set field information. It is a 'const' method because it is called
    /// from other 'const' methods.
    void setFieldInfos(const Array<VMField>& finfo) const;
    
    /// Returns true if fieldinfos have been set.
    bool hasFieldInfos() const;

    /// If no fieldnames have been set, will set default field names to "0",
    /// "1", "2", ... i.e. their column index.
    Array<VMField>& getFieldInfos() const;

    /// Read the fieldnames from the metadatadir.  Does not modify any internal
    /// members.
    Array<VMField> getSavedFieldInfos() const;

    /// Return the fieldinfos for a given column
    VMField& getFieldInfos(int fieldindex) const
    {
        return getFieldInfos()[fieldindex];
    }

    /// Declare the fieldinfos (in particular the field name) for a given
    /// column
    void declareField(int fieldindex, const string& fieldname,
                      VMField::FieldType fieldtype=VMField::UnknownType);

    /// Declare all fieldinfos
    void declareFieldNames(const TVec<string>& fnames);

    /// Returns the column index corresponding to a fieldname
    /// or -1 if the name was not found.
    int fieldIndex(const string& fieldname) const;

    /**
     *  This first calls fieldIndex to try and get the index corresponding to
     *  the given string. If this fails, the given string is assumed to hold
     *  the numerical index, and its conversion to int will be returned (or a
     *  PLERROR issued if this fails, unless 'throw_error' is set to false, in
     *  which case -1 is returned instead).
     */
    int getFieldIndex(const string& fieldname_or_num,
                      bool throw_error = true) const;

    //! Remote version of 'getFieldIndex'.
    int remote_getFieldIndex(const string& fieldname_or_num) const
    {
        return getFieldIndex(fieldname_or_num);
    }

    /// Return the field name at a given index
    string fieldName(int fieldindex) const
    {
        return getFieldInfos(fieldindex).name;
    }

    /// Returns the vector of field names.
    TVec<string> fieldNames() const;

    /// Add a numeric suffix to duplicated fieldNames (eg: field.1 field.2 etc..).
    void unduplicateFieldNames(); 

    /// Returns the names of the input fields (if any)
    virtual TVec<string> inputFieldNames() const;

    /// Returns the names of the target fields (if any)
    virtual TVec<string> targetFieldNames() const;

    /// Returns the names of the weight fields (if any)
    virtual TVec<string> weightFieldNames() const;

    /// Returns the names of the extra fields (if any)
    virtual TVec<string> extraFieldNames() const;

    VMField::FieldType fieldType(int fieldindex) const
    {
        return getFieldInfos(fieldindex).fieldtype;
    } 

    VMField::FieldType fieldType(const string& fieldname) const
    {
        return fieldType(fieldIndex(fieldname));
    } 

    const VMFieldStat& fieldStat(int j) const
    {
        return fieldstats[j];
    } 

    const VMFieldStat& fieldStat(const string& fieldname) const
    {
        return fieldStat(fieldIndex(fieldname));
    }

    void printFields(PStream& out) const;
    void printFieldInfo(PStream& out, int fieldnum, bool print_binning = false) const;
    void printFieldInfo(PStream& out, const string& fieldname_or_num,
                        bool print_binning = false) const;

    /// Loads/saves from/to the metadatadir/fieldnames file.
    void saveFieldInfos() const;
    void loadFieldInfos() const;

    
    //#####  Meta Data  #######################################################

    /**
     *  Sets all meta info (length_, width_, inputsize_, targetsize_,
     *  weightsize_, extrasize_, fieldnames, ...) that is not already set, by
     *  copying it from the source's Vmat vm. Modification time is also set to
     *  the latest of the current mtime of this vmat and of the mtime of the
     *  source.  Sizes will be copied only if they are consistent with this
     *  VMat's width.
     */
    virtual void setMetaInfoFrom(const VMatrix* vm);

    /// Return true iif it looks like the same matrix, i.e. it has same sizes,
    /// width and length.
    bool looksTheSameAs(const VMat& m);

    //! Generate a PLERROR iff 'm' does not look like the same matrix,
    //! i.e. it does not have same sizes and width.
    //! If an 'extra_msg' is provided, this message is appended to the error
    //! message displayed when there is a size mismatch.
    void compatibleSizeError(const VMat& m, const string& extra_msg = "");

    /**
     *  This should be called by the build method of every VMatrix that has a
     *  metadatadir.  It will create said directory if it doesn's already
     *  exist.  Throws a PLERROR if called with an empty string.
     */
    virtual void setMetaDataDir(const PPath& the_metadatadir);

    /// Returns true if a metadatadir was set
    bool hasMetaDataDir() const { return !metadatadir.isEmpty(); }

    /// Throws a PLERROR if no metadatadir was set.
    PPath getMetaDataDir() const;

    /**
     *  Locks the metadata directory by creating a .lock file inside it.  If
     *  such a file already exists, it is interpreted as being locked by some
     *  other process: this process will print to cerr that it is waiting for a
     *  lock on that directory, and will block and wait until the existing
     *  .lock is removed before recreating its own.  Throws a PLearnError if
     *  called and metadatadir is not set, or lock is already held by this
     *  object (i.e. this->lockMetaDataDir has already been called previously
     *  and no unlockMetaDataDir() was called).  If the 'max_lock_age' option
     *  is given a value > 0, then the lock file will be ignored (and replaced
     *  by our own lock file) as soon as its modification date becomes older
     *  than 'max_lock_age' (in seconds).
     *
     *  The 'verbose' option can be set to false to prevent useless output.
     */
    void lockMetaDataDir(time_t max_lock_age = 0, bool verbose = true) const;

    /// Removes the .lock file inside the metadatadir.
    /// It will throw a PLearnError if this object did not hold the lock.
    void unlockMetaDataDir() const;

    /**
     *  This method overloads the Object::save method which is deprecated. This
     *  method is therefore deprecated and you should call directly the
     *  savePMAT() method.
     *
     *  @deprecated Use savePMAT() instead.
     */
    virtual void save(const PPath& filename) const;

    /// Save the VMatrix in PMat format
    virtual void savePMAT(const PPath& pmatfile,
                          const bool force_float=false) const;

    /// Save the VMatrix in DMat format
    virtual void saveDMAT(const PPath& dmatdir) const;

    /**
     *  Save the content of the matrix in the AMAT ASCII format into a file.
     *  If 'no_header' is set to 'true', then the AMAT header won't be saved,
     *  which can be useful to export data to other applications.  If
     *  'save_strings' is set to 'true', then the string mappings will be used
     *  so as to save strings where they exist (instead of saving the
     *  corresponding real value).
     */
    virtual void saveAMAT(const PPath& amatfile, bool verbose = true,
                          bool no_header = false, bool save_strings = false) const;

    /// Return true if the matrix is writable, i.e. if put()-like member
    /// functions can succeed.
    inline bool isWritable() const { return writable; }

    /**
     *  This function (used with .vmat datasets), is used to return the
     *  filename of fieldInfo files (string maps (.smap) and notes (.notes)).
     *  It recursively navigates through links until it finds a suitable file
     *  (.smap or .notes) Idea : a .metadata/FieldInfo can contain one of these
     *  files : (the order show here is the one used by the function to
     *  searches the file)
     * 
     *  fieldName.smap.lnk : containing the actual path+target OR another .lnk
     *  file
     *
     *  fieldName.smap : the target (the actual string map or comment file)
     *  __default.lnk : contains another FieldInfo directory to look for target
     *  (typically the
     * 
     *  ** Note 1: that target is assumed to be an inexistant file in the directory
     *  where none of the previous 3 can be found (since the file exists only
     *  when non-empty)

     *  ** Note 2: source may not be target
     */
    string resolveFieldInfoLink(const PPath& target, const PPath& source);

    /**
     *  Return the time of "last modification" associated with this matrix The
     *  result returned is typically based on mtime of the files contianing
     *  this matrix's data when the object is constructed.  mtime_ defaults to
     *  0, so that's what will be returned by default, if the time was never
     *  set by a call to updateMtime(t) or setMtime(t)(see below).
     */
    inline time_t getMtime() const { return mtime_==numeric_limits<time_t>::max()? 0: mtime_; }

    /**
     *  Update the "last modification" time for this matrix.
     *  this should be called by the constructor for all dependence,
     *  file or other VMatrix, VMat
     *  This fonction remember the time that is the more recent. If
     *  a dependence have a mtime of 0, getMtime() will always return 0
     *  as if a dependence have an unknow mtime, we should have an unknow mtime
     */
    void updateMtime(time_t t);

    void updateMtime(const PPath& p);

    void updateMtime(VMat v);

    /**
     *  Preferably use updateMtime()!
     *  Sets the "last modification" time for this matrix,
     *  this should be called by the constructor to reflect the mtime of the
     *  disk files.
     *  @see updateMtime 
     */
    inline void setMtime(time_t t) { mtime_ = t; }

    //! Return 'true' iff 'file' was last modified after this VMat, or this
    //! VMat's last modification time is undefined (set to 0).
    //! If 'warning_mtime0' is 'true', then a warning will be issued when the
    //! file exists and this VMat's last modification time is undefined.
    //! If 'warning_older' is 'true', then a warning will be issued when the
    //! file exists and it is older than this VMat's last modification time.
    bool isUpToDate(const PPath& file, bool warning_mtime0 = true,
                    bool warning_older = false) const;

    //! Return 'true' iff 'vm' was last modified after this VMat, this VMat's
    //! last modification time is undefined (set to 0), or vm's last
    //! modification time is undefined.
    //! If 'warning_mtime0' is 'true', then a warning will be issued when one
    //! of the VMats' last modification time is undefined.
    //! If 'warning_older' is 'true', then a warning will be issued when 'vm'
    //! is older than this VMat.
    bool isUpToDate(VMat vm, bool warning_mtime0 = true,
                    bool warning_older = false) const;

    //#####  Matrix Sizes  ####################################################

    /// Return the number of columns in the VMatrix
    inline int width() const
    {
#ifdef BOUNDCHECK
        if (!this)
            PLERROR("VMatrix::width() This object has pointer this=NULL");
#endif
        return width_;
    }

    /// Return the number of rows in the VMatrix
    inline int length() const
    {
#ifdef BOUNDCHECK
        if (!this)
            PLERROR("VMatrix::length() This object has pointer this=NULL");
#endif
        return length_;
    }

    /// Define the input, target and weight sizes.
    inline void defineSizes(int inputsize, int targetsize, int weightsize=0, int extrasize=0)
    {
        inputsize_  = inputsize;
        targetsize_ = targetsize;
        weightsize_ = weightsize;
        extrasize_ = extrasize;
    }

    /// Copy the values of inputsize, targetsize and weightsize from the source
    /// matrix m.
    void copySizesFrom(const VMat& m);

    /**
     *  Read the saved sizes from the metadatadir. If the "sizes" file does not
     *  exist, return false.  If it exists but the format is wrong, generate a
     *  PLerror.  If everything looks clean, the 3 arguments are set to the
     *  sizes and return true.
     */
    bool getSavedSizes(int& inputsize, int& targetsize, int& weightsize, int& extrasize) const;

    /// Input size accessor
    inline int inputsize() const { return inputsize_; }

    /// Target size accessor
    inline int targetsize() const { return targetsize_; }

    /// Weight size accessor
    inline int weightsize() const { return weightsize_; }

    /// Extra size accessor
    inline int extrasize() const { return extrasize_; }

    /// Return true if VMatrix has a weight column
    inline bool hasWeights() const { return weightsize_>0; }

    
    //#####  Numerical Data Access  ###########################################

    /**
     *  Default version calls getSubRow based on inputsize_ targetsize_
     *  weightsize_ But exotic subclasses may construct, input, target and
     *  weight however they please.  If not a weighted matrix, weight should be
     *  set to default value 1.
     */
    virtual void getExample(int i, Vec& input, Vec& target, real& weight);

    //! Remote version of getExample.
    boost::tuple<Vec, Vec, real> remote_getExample(int i);

    //! Obtain a subset of 'length' examples, starting from 'i_start'.
    //! The 'extra' matrix is provided as a pointer so that it can be omitted
    //! without significant overhead.
    //! If the 'allow_circular' boolean parameter is set to 'true', then one
    //! may ask for a subset that goes beyond this VMat's length: in such a
    //! case, the rest of the subset will be filled with data found at the
    //! beginning of this VMat.
    void getExamples(int i_start, int length, Mat& inputs, Mat& targets,
                     Vec& weights, Mat* extra = NULL,
                     bool allow_circular = false);

    /**
     *  Complements the getExample method, fetching the the extrasize_ "extra"
     *  fields expected to appear after the input, target and weight fields
     *  Default version calls getSubRow based on inputsize_ targetsize_
     *  weightsize_ and extrasize_
     */
    virtual void getExtra(int i, Vec& extra);
    Vec remote_getExtra(int i);

    /// This method must be implemented in all subclasses
    virtual real get(int i, int j) const = 0; ///<  Returns element (i,j).

    /// This method must be implemented in all subclasses of writable matrices
    virtual void put(int i, int j, real value); ///< Sets element (i,j) to value.

    /**
     *  It is suggested that this method be implemented in subclasses to speed
     *  up accesses (default version repeatedly calls get(i,j) which may have a
     *  significant overhead).  Fills v with the subrow i lying between columns
     *  j (inclusive) and j+v.length() (exclusive).
     */
    virtual void getSubRow(int i, int j, Vec v) const;

    /**
     *  It is suggested that this method be implemented in subclasses of writable
     *  matrices to speed up accesses (default version repeatedly calls
     *  put(i,j,value) which may have a significant overhead)
     */
    virtual void putSubRow(int i, int j, Vec v);

    /// This method must be implemented for matrices that are allowed to grow.
    virtual void appendRow(Vec v);

    /// This method must be implemented for matrices that are allowed to grow.
    virtual void insertRow(int i, Vec v);

    /// For matrices stored on disk, this should flush all pending buffered write operations.
    virtual void flush();

    /// Will call putRow if i<length() and appendRow if i==length().
    void putOrAppendRow(int i, Vec v);

    /// Will call putRow if i<length().  if i>= length(), it will call
    /// appendRow with 0 filled rows as many times as necessary before it can
    /// append row i.
    void forcePutRow(int i, Vec v);

    /// These methods do not usually need to be overridden in subclasses
    /// (default versions call getSubRow, which should do just fine)
    virtual void getRow(int i, Vec v) const; ///< Copies row i into v (which must have appropriate length equal to the VMat's width).

    virtual void putRow(int i, Vec v);
    virtual void fill(real value);

    /// Copies the submatrix starting at i,j into m (which must have
    /// appropriate length and width).
    virtual void getMat(int i, int j, Mat m) const;

    /// Copies matrix m at position i,j of this VMat.
    virtual void putMat(int i, int j, Mat m);

    /// Copies column i into v (which must have appropriate length equal to the
    /// VMat's length).
    virtual void getColumn(int i, Vec v) const;

    //! remote version of getColumn: return newly alloc'd vec
    Vec remote_getColumn(int i) const;

    /**
     *  Return true iff the input vector is in this VMat (we compare only the
     *  input part).  If the parameter 'i' is provided, it will be filled with
     *  the index of the corresponding data point, or with -1 if it does not
     *  exist in this VMat.  The 'tolerance' parameter indicates the maximum
     *  squared distance between two points to consider them as equal.
     *  'i_start' specifies the row where the search begins.
     */
    bool find(const Vec& input, real tolerance, int* i = 0, int i_start = 0) const;

    /**
     *  Default version returns a SubVMatrix referencing the current VMatrix
     *  however this can be overridden to provide more efficient shortcuts
     *  (see MemoryVMatrix::subMat and SubVMatrix::subMat for examples)
     */
    virtual VMat subMat(int i, int j, int l, int w);


    //#####  Conversions  #####################################################

    /**
     *  Returns a Mat with the same data as this VMat.  The default version of
     *  this method calls toMatCopy().  However this method will typically be
     *  overrided by subclasses (such as MemoryVMatrix) whose internal
     *  representation is already a Mat in order to return this Mat directly to
     *  avoid a new memory allocation and copy of elements. In this case, and
     *  in this case only, modifying the elements of the returned Mat will
     *  logically result in modified elements in the original VMatrix view of
     *  it. If you want to be sure that altering the content of the returned
     *  Mat won't modify the data contained in the VMatrix, you should call
     *  toMatCopy() instead.
     */
    virtual Mat toMat() const;

    /// Returns a Mat with the same data as this VMat.
    /// This method copies the data in a fresh Mat created in memory
    Mat toMatCopy() const;

    /// The default implementation of this method does nothing, but subclasses
    /// may overload it to reallocate memory to exactly what is needed and no
    /// more.
    virtual void compacify();

    /// In case the dimensions of an underlying VMat has changed, recompute it.
    virtual void reset_dimensions() { }

    /**
     *  Conversion to Mat.  WARNING: modifying the content of the returned Mat
     *  may or may not modify the content of the VMatrix, depending on the type
     *  of the VMatrix. If you want to be sure to get a *copy* of the data,
     *  consider calling toMatCopy() instead.
     */
    operator Mat() const { return toMat(); }

    /**
     *  Output the content of the VMat in the stream 'out'.
     *  Overridden to display only the content of the VMat when out's mode
     *  is 'raw_ascii' or 'pretty_ascii' (instead of doing serialization).
     *  @todo Deal with raw_binary too !
     */
    virtual void newwrite(PStream& out) const;


    //#####  Statistics  ######################################################

    virtual void computeStats();
    bool hasStats() const { return fieldstats.size()>0; }
    void saveStats(const PPath& filename) const;
    void loadStats(const PPath& filename);

    /**
     * Returns the unconditional statistics for all fields from the
     * stats.psave file (if the file does not exist, a default version is
     * automatically created).
     */
    TVec<StatsCollector> getStats(bool progress_bar=false) const;

    //! Generic function to obtain the statistics from a given file in the
    //! metadatadir. If this file does not exist, statistics are computed and
    //! saved in this file.
    TVec<StatsCollector> getPrecomputedStatsFromFile(const string& filename,
                                                     int maxnvalues,
                                                     bool progress_bar) const;

    TVec<PP<StatsCollector> > remote_getStats() const;

    StatsCollector& getStats(int fieldnum) const
    { return getStats()[fieldnum]; }

    
    /** Compare the stats of this VMat with the target one.
     * @param target The VMat we compare against
     * @param stderror_threshold The threshold allowed for the standard error
     * @param missing_threshold The threshold allowed for the % of missing
     * @param stderror A measure of difference 
     * @param missing A measure of difference 
     * @return The number of differences that were found
     */
    void compareStats(VMat target,
                      real stderror_threshold ,
                      real missing_threshold,
                      Vec stderror,
                      Vec missing
                      );
                      
    /**
     * @return The size of the longest fieldname
     */
    int maxFieldNamesSize() const;

    /** If only one of inputsize, targetsize, weightsize, extrasize
     *  is unknow while width>=0, we compute its value.
     *  Two warnings may be issued in this method:
     *      1. If 'warn_if_cannot_compute' is true, a warning is issued when
     *         it is not possible to compute a missing size's value (for
     *         instance when there are two missing sizes).
     *      2. If 'warn_if_size_mismatch' is true, a warning is issued when
     *         all sizes are defined but they do not match the width.
     */
    void computeMissingSizeValue(bool warn_if_cannot_compute = true,
                                 bool warn_if_size_mismatch = true);

    /**
     *  Returns the bounding box of the data, as a vector of min:max pairs.  If
     *  extra_percent is non 0, then the box is enlarged in both ends of every
     *  direction by the given percentage (ex: if the data's x lies within
     *  [0,100] and extra_percent is 0.03 then the returned bound pair will be
     *  -3:103 ).
     */
    TVec< pair<real,real> > getBoundingBox(real extra_percent=0.00) const;

    /**
     *  Returns the ranges as defined in the ranges.psave file (for all fields)
     *  (if the ranges.psave file does not exist, a reasonable default version
     *  is created ).
     */
    TVec<RealMapping> getRanges();


    //#####  Special Mathematical Operations  #################################

    /**
     *  Returns the dot product between row i1 and row i2 (considering only the
     *  inputsize first elements).  The default version in VMatrix is somewhat
     *  inefficient, as it repeatedly calls get(i,j) The default version in
     *  RowBufferedVMatrix is a little better as it buffers the 2 Vecs between
     *  calls in case one of them is needed again.  But the real strength of
     *  this method is for specialised and efficient versions in subbclasses.
     *  This method is typically used by SmartKernels so that they can compute
     *  kernel values between input samples efficiently.
     */
    virtual real dot(int i1, int i2, int inputsize) const;

    inline real dot(int i1, int i2) const { return dot(i1,i2,width()); }

    /// Returns the result of the dot product between row i and the given vec
    /// (only v.length() first elements of row i are considered).
    virtual real dot(int i, const Vec& v) const;

    /**
     *  result += transpose(X).Y
     *  where X = this->subMatColumns(X_startcol,X_ncols)
     *  and   Y = this->subMatColumns(Y_startcol,Y_ncols)
     */
    virtual void accumulateXtY(
        int X_startcol, int X_ncols, int Y_startcol, int Y_ncols,
        Mat& result, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;

    /**
     *  A special case of method accumulateXtY
     *  result += transpose(X).X
     *  where X = this->subMatColumns(X_startcol,X_ncols)
     */
    virtual void accumulateXtX(
        int X_startcol, int X_ncols,
        Mat& result, int startrow=0, int nrows=-1, int ignore_this_row=-1) const;

    //#####  String Mappings  #################################################

    /// Make sure string mappings are the right size.
    void init_map_sr() const ;

    /// Save all string mapings (one .smap file for each field).
    void saveAllStringMappings();

    /**
     *  Save a single field's string mapping in file 'fname'.  The
     *  corresponding string -> real mapping can optionally be given in
     *  argument, otherwise it will be obtained through the
     *  getStringToRealMapping() method.
     */
    void saveStringMappings(int col, const PPath& fname,
                            map<string, real>* str_to_real = 0);
    /// Adds a string<->real mapping
    void addStringMapping(int col, string str, real val);

    /**
     *  Adds a string<->real mapping for a new string, if it doesn't already
     *  have one and returns the associated value.  If the string doesn't
     *  already have an associated value, it will be associated with value
     *  -100-number_of_strings_already_in_the_map.
     */
    real addStringMapping(int col, string str);

    /// Remove all string mappings.
    void removeAllStringMappings();

    /// Remove all string mappings of a given field.
    void removeColumnStringMappings(int c);

    /// Removes a single string mapping.
    void removeStringMapping(int col, string str);

    /// overwrite the string<->real mapping with this one (and build the
    /// reverse mapping).
    void setStringMapping(int col, const map<string,real>& zemap);

    /// Deletes string mapping for column i.
    void deleteStringMapping(int col);

    /// Loads the appropriate string map file for column 'col'.
    void loadStringMapping(int col);

    /// Loads the appropriate string map file for every column.  It is virtual
    /// because StrTableVMatrix will need to override it.
    virtual void loadAllStringMappings();

    /// Copy all string mappings from a given VMat.
    void copyStringMappingsFrom(const VMat& source);

    /// Returns the string associated with value val for field# col. Or returns
    /// "" if no string is associated.
    virtual string getValString(int col, real val) const;

    /// Returns the string->real mapping for column 'col'.
    virtual const map<string,real>& getStringToRealMapping(int col) const;

    /// Returns the real->string mapping for column 'col'.
    virtual const map<real,string>& getRealToStringMapping(int col) const;

    /// Returns value associated with a string (or MISSING_VALUE if there's no
    /// association for this string).
    virtual real getStringVal(int col, const string & str) const;

    /// Returns element as a string, even if value doesn't map to a string, in
    /// which case tostring(value) is returned.
    virtual string getString(int row, int col) const;

    /// Copy row i (converted to string values, using string mappings when they
    /// exist) into v.
    virtual void getRowAsStrings(int i, TVec<string>& v_str) const;

    /// Return the Dictionary object for a certain field, or a null pointer if
    /// there isn't one
    virtual PP<Dictionary> getDictionary(int col) const;

    //! Returns the possible values for a certain field in the VMatrix.
    //! For example, if "col" corresponds to the target column, this
    //! function could fill "values" with the class indices of the possible 
    //! target classes for the example at row "row".
    //! The default getValues(...) function gives an empty "values"
    virtual void getValues(int row, int col, Vec& values) const;

    //! Gives the possible values of a certain field (column) given the input.
    virtual void getValues(const Vec& input, int col, Vec& values) const;

    //! returns a given row
    Vec getRowVec(int i) const; 

    //! appends the given rows
    void appendRows(Mat rows);

    //#####  SFIF Files  ######################################################
    
    /**
     *  These 3 functions deal with stringmaps, notes, and binning files (all
     *  three called Special Field Info Files, or 'SFIF') for each field
     *  eventually, I (julien) guess all this info should be wrapped (thus
     *  saved, and loaded) in the VMField class
     * 
     *  SFIFs, are by default located in the directory
     *  MyDataset.{amat,vmat,etc}.metadata/FieldInfo/ and are named
     *  'fieldname'.{smap,notes,binning,...}.  In all 3 functions, the
     *  parameter ext (given **with** the dot) specifies the extension of the
     *  special field info file [smap,notes,binning], and col is the column
     *  index you refer to.
     * 
     *  setSFIFFilename : sets the SFIF with extensions 'ext' to some
     *  'string'. if this string is different from the default filename, the
     *  string is actually placed in a new file called
     *  [dataset].metadata/FieldInfo/fieldname.[ext].lnk if the 'string' is
     *  empty, the default SFIF filename is assumed, which is :
     *  [MyDataset].metadata/FieldInfo/fieldname.[ext]
    */
    void setSFIFFilename(int col, string ext, const PPath& filepath="");
    void setSFIFFilename(string fieldname, string ext, const PPath& filepath="");

    /**
     *  getSFIFFilename :If a '*.vmat' dataset uses fields from another
     *  dataset, how can we keep the field info dependency? To resolve this
     *  issue, a file named __default.lnk containing path 'P' can be placed in
     *  the FieldInfo directory of the .vmat. Here's how the function
     *  getSFIFFilename search for a file : if the default SFIF file doesn't
     *  exist, it will then search for the default filename +'.lnk'.  if the
     *  later neither exists, the __default.lnk file is used if present, and if
     *  not, then an empty (thus inexistent) file (with SFIF default filename)
     *  is assumed.
    */
    PPath getSFIFFilename(int col, string ext);
    PPath getSFIFFilename(string fieldname, string ext);

    /// Return the directory that stores SFIF files.
    PPath getSFIFDirectory() const;
    
    /// isSFIFDirect : tells whether the SFIF filename is the default
    /// filename. (if false, means the field uses the SFIF from another
    /// dataset)
    bool isSFIFDirect(int col, string ext);
    bool isSFIFDirect(string fieldname, string ext);
};

DECLARE_OBJECT_PTR(VMatrix);

/**
 *  NOTE: How to handle exotic cases of data-sets whose input or target are not
 *  standard Vecs: The idea is to still have the getExample build and return
 *  Vecs, but the representation of these Vecs will have a special format,
 *  detected and understood by a specialised Learner or specialised Variables:
 *  Hack: format des Vec compris par un Learner:
 *
 *  Si v[0] == SPECIAL_FORMAT
 *  v[1] indique le format de ce qui suit (v[2] ...):
 *  0 sparse vec de la forme: length nvals i val i val ...
 *  1 pointeur vers un Object de la forme: ptr  (cast du float ou du double)
 *  2 tenseur plein de la forme: rank size_1...size_n val ...
 *  3 tenseur sparse de la forme: rank size_1...size_n nvals i_1...i_n val i1...i_n val ...
 */
#define SPECIAL_FORMAT ((real)3.1e36)


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
