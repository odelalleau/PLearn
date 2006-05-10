// -*- C++ -*-

// VVMatrix.h
// Copyright (C) 2002 Pascal Vincent and Julien Keable
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
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef VVMatrix_INC
#define VVMatrix_INC

#include "VMat.h"

namespace PLearn {
using namespace std;

//! this class is a wrapper for a .vmat VMatrix.
class VVMatrix: public VMatrix
{

private:

    typedef VMatrix inherited;

protected:

    string code;
    VMat the_mat;

    static void declareOptions(OptionList& ol);

    //! loads a .vmat file and return the VMat object built from it
    // returns the precomputed version if it can
    static VMat createPreproVMat(const string & filename);

    // generate a file (ivfname) containing indexes of rows of 'source' that remain after filtering with
    // the *every* possible step that changes the index of rows (i.e : prefilter, shuffle.. postfiltering)
    // -- Not optimal, since it will first *precompute* if any postfilter is required
    static void generateVMatIndex(VMat source, const string& meta_data_dir,
                                  const string & filename, time_t date_of_code,const string & in,
                                  size_t idx_prefilter, size_t cidx_prefilter,
                                  size_t idx_postfilter, size_t cidx_postfilter,
                                  size_t idx_process, size_t cidx_process,
                                  size_t idx_shuffle, size_t cidx_shuffle,
                                  size_t idx_join, size_t cidx_join);
// returns the result from the join operation
    static void processJoinSection(const vector<string> & code, VMat & tmpsource);
    // returns a 2d-array that contains the structure of the source datasets that will be concatenated
    static vector<vector<string> > extractSourceMatrix(const string & str,const string& filename);
    // generate a file (ivfname) containing indexes of rows of 'source' that remain after filtering with 'code'
    static void generateFilterIndexFile(VMat source, const string & code, const string& ivfname);

public:

    // public build options
    string the_filename;

public:

    PLEARN_DECLARE_OBJECT(VVMatrix);

    virtual void build();

    const string & getCode(){return code;}

    //! get "real" date of .vmat according to its dependencies
    static time_t getDateOfVMat(const string& filename);

    //! returns the source VMat after filtering it with 'code' . ivfname is the name of the index file and
    //! date_of_code is the code last modification date. If existant, index file is used as is, provided
    //! its date is > the latest date of all files (code + all its dependencies)
    static VMat buildFilteredVMatFromVPL(VMat source, const string & code, const string& ivfname, time_t date_of_code);

    //! returns true if the .vmat is precomputed *and* if that precomputed data is more recent than the .vmat file
    bool isPrecomputedAndUpToDate();

    // returns a filename for the precomputed dataset (which you could load for example with getDataSet)
    string getPrecomputedDataName();

    VVMatrix(const string& filename_):the_filename(filename_){build_();}
    VVMatrix(){};

    // ****************************************************
    // POSSIBLE "cache" IMPROVEMENT.. need to check it out with pascal
    // would it be a good idea to systematically wrap "the_mat" with a RowBufferedVMatrix  ?

// string maps are those loaded from the .vmat metadatadir, not those of the source vmatrix anymore
// could be changed..

//   virtual string getValString(int col, real val) const;
//   virtual real getStringVal(int col, const string & str) const;
//   virtual string getString(int row,int col) const;
//   virtual const hash_map<string,real>& getStringToRealMapping(int col) const;

    virtual real get(int i, int j) const {return the_mat->get(i,j);}
    virtual void getSubRow(int i, int j, Vec v) const {the_mat->getSubRow(i,j,v);}

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

private:

    void build_();

};

DECLARE_OBJECT_PTR(VVMatrix);

} // end of namespace PLearn


#endif // VVMatrix_INC


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
