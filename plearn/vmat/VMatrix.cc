// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2004 Rejean Ducharme
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

/********************************************************
 * $Id$
 ******************************************************* */

#include "VMatrix.h"
#include "CompactFileVMatrix.h"
#include "DiskVMatrix.h"
#include "FileVMatrix.h"
#include "SubVMatrix.h"
#include "VMat_computeStats.h"
#include <plearn/base/tostring.h>
#include <plearn/base/lexical_cast.h>
#include <plearn/base/stringutils.h> //!< For pgetline()
#include <plearn/io/fileutils.h>     //!< For isfile() mtime()
#include <plearn/base/tostring.h>     //!< For isfile() mtime()
#include <plearn/io/load_and_save.h>
#include <plearn/math/random.h>      //!< For uniform_multinomial_sample()
#include <plearn/base/RemoteDeclareMethod.h>
#include <nspr/prenv.h>
#include <plearn/math/TMat_maths.h> //!< for dot, powdistance externalProductAcc
#include <plearn/sys/procinfo.h> //!< for getPid, getUser
#include <limits>

namespace PLearn {
using namespace std;

// TODO-PPath   : this class is now PPath compliant
// TODO-PStream : this class is now PStream compliant

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    VMatrix,
    "Base classes for virtual matrices",
    "VMatrix provides an abstraction for a virtual matrix, namely a matrix wherein\n"
    "all element access operations are virtual. This enables a wide variety of\n"
    "matrix-like objects to be implemented, from simple data containers (e.g.\n"
    "MemoryVMatrix), to large-scale matrices that don't fit in memory (e.g.\n"
    "FileVMatrix), to on-the-fly calculations that are implemented through \n"
    "various processing VMatrices.\n"
    "\n"
    "For implementers, a simple class to derive from is RowBufferedVMatrix, which\n"
    "implements most of the functionalities of the abstract VMatrix interface in terms\n"
    "of a few simple virtual functions to be overridden by the user.");

VMatrix::VMatrix(bool call_build_):
    inherited   (call_build_),
    mtime_      (0),
    mtime_update(0),
    length_     (-1),
    width_      (-1),
    inputsize_  (-1),
    targetsize_ (-1),
    weightsize_ (-1),
    extrasize_  (0),
    writable    (false)
{
    lockf_ = PStream();
    if (call_build_)
        build_();
}

VMatrix::VMatrix(int the_length, int the_width, bool call_build_):
    inherited                       (call_build_),
    mtime_                          (0),
    mtime_update                    (0),
    length_                         (the_length),
    width_                          (the_width),
    inputsize_                      (-1),
    targetsize_                     (-1),
    weightsize_                     (-1),
    extrasize_                      (0),
    writable                        (false),
    map_sr(TVec<map<string,real> >  (the_width)),
    map_rs(TVec<map<real,string> >  (the_width)),
    fieldstats                      (0)
{
    lockf_ = PStream();
    if (call_build_)
        build_();
}

////////////////////
// declareOptions //
////////////////////
void VMatrix::declareOptions(OptionList & ol)
{
    declareOption(
        ol, "writable", &VMatrix::writable, OptionBase::buildoption,
        "Are write operations permitted?");

    declareOption(
        ol, "length", &VMatrix::length_, OptionBase::buildoption,
        "Length of the matrix (number of rows)");

    declareOption(
        ol, "width", &VMatrix::width_, OptionBase::buildoption,
        "Width of the matrix (number of columns; -1 indicates this varies\n"
        "from sample to sample...)");

    declareOption(
        ol, "inputsize", &VMatrix::inputsize_, OptionBase::buildoption,
        "Size of input part (-1 if variable or unspecified, 0 if no input)");

    declareOption(
        ol, "targetsize", &VMatrix::targetsize_, OptionBase::buildoption,
        "Size of target part (-1 if variable or unspecified, 0 if no target)");

    declareOption(
        ol, "weightsize", &VMatrix::weightsize_, OptionBase::buildoption,
        "Size of weights (-1 if unspecified, 0 if no weight, 1 for sample\n"
        "weight, >1 currently not supported).");

    declareOption(
        ol, "extrasize", &VMatrix::extrasize_, OptionBase::buildoption,
        "Size of extra fields (additional info). Defaults to 0");

    declareOption(
        ol, "metadatadir", &VMatrix::metadatadir, OptionBase::buildoption,
        "A directory in which to store meta-information for this matrix \n"
        "You don't always have to give this explicitly. For ex. if your \n"
        "VMat is the outer VMatrix in a .vmat file, the metadatadir will \n"
        "automatically be set to name_of_vmat_file.metadata/ \n"
        "And if it is the source inside another VMatrix that sets its \n"
        "metadatadir, it will often be set from that surrounding vmat's metadata.\n");

    declareOption(
        ol, "mtime", &VMatrix::mtime_update, 
        OptionBase::buildoption|OptionBase::nosave,
        "DO NOT play with this if you don't know the implementation!\n"
        "This add a dependency mtime to the gived value.\n"
        "Use -1 to set permanently that we do not know the mtime.");

    declareOption(
        ol, "fieldinfos", &VMatrix::fieldinfos, OptionBase::buildoption,
        "Field infos.\n");

    inherited::declareOptions(ol);
}

void VMatrix::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this
    // different than for declareOptions()
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(
        rmm, "getRow", &VMatrix::getRowVec,
        (BodyDoc("Returns a row of a matrix \n"),
         ArgDoc ("i", "Position of the row to get.\n"),
         RetDoc ("row i vector")));

    declareMethod(
        rmm, "getExample", &VMatrix::remote_getExample,
        (BodyDoc("Returns the input, target and weight parts of a row.\n"),
         ArgDoc ("i", "Position of the row to get.\n"),
         RetDoc ("An (input, target, weight) tuple.")));

    declareMethod(
        rmm, "getExtra", &VMatrix::remote_getExtra,
        (BodyDoc("Returns the extra part of a row.\n"),
         ArgDoc ("i", "Position of the row to get.\n"),
         RetDoc ("Values for extrafields.")));

    declareMethod(
        rmm, "getColumn", &VMatrix::remote_getColumn,
        (BodyDoc("Returns a row of a matrix \n"),
         ArgDoc ("i", "Position of the row to get.\n"),
         RetDoc ("row i vector")));


    declareMethod(
        rmm, "getString", &VMatrix::getString,
        (BodyDoc("Returns an element of a matrix as a string\n"),
         ArgDoc ("i", "Position of the row to get.\n"),
         ArgDoc ("j", "Position of the column to get.\n"),
         RetDoc ("string value")));

    declareMethod(
        rmm, "getMat", &VMatrix::toMat,
        (BodyDoc("Returns the content of the vmat as a Mat\n"),
         RetDoc ("The content of this VMatrix as a Mat")));

    declareMethod(
            rmm, "getLength", &VMatrix::length,
            (BodyDoc("Return length of this VMatrix.\n"),
             RetDoc("The length of this VMatrix.")));

    declareMethod(
        rmm, "declareField", &VMatrix::declareField,
        (BodyDoc("Declares the field infos for a given column (index).\n"),
         ArgDoc ("fieldindex", "The column index.\n"),
         ArgDoc ("fieldname", "The field name of this column.\n"),
         ArgDoc ("fieldtype", "The field type of this column.\n")));

    declareMethod(
        rmm, "declareFieldNames", &VMatrix::declareFieldNames,
        (BodyDoc("Declares the field names.\n"),
         ArgDoc ("fnames", "TVec of field names.\n")));

    declareMethod(
        rmm, "fieldNames", &VMatrix::fieldNames,
        (BodyDoc("Returns the field names.\n"),
         RetDoc ("TVec of field names.\n")));

    declareMethod(
        rmm, "fieldName", &VMatrix::fieldName,
        (BodyDoc("Returns the field name for a given column.\n"),
         ArgDoc ("col", "column index.\n"),
         RetDoc ("Field name.\n")));

     declareMethod(
        rmm, "findFieldIndex", &VMatrix::fieldIndex,
        (BodyDoc("Returns the index of a field, or -1 if the field does not "
                 "exist.\n"),
         ArgDoc ("fname",
             "Field name of the field.\n"),
         RetDoc ("Index of the field (-1 if not found)\n")));

      declareMethod(
        rmm, "getFieldIndex", &VMatrix::remote_getFieldIndex,
        (BodyDoc("Returns the index of a field. "
                 "Throws an error if the field is not found.\n"),
         ArgDoc ("fname_or_num",
             "Field name or index (as a string) of the field.\n"),
         RetDoc ("Index of the field.\n")));
    
    declareMethod(
        rmm, "appendRow", &VMatrix::appendRow,
        (BodyDoc("Appends a row to the VMatrix.\n"),
         ArgDoc ("v", "Vec with values (row) to append.\n")));

    declareMethod(
        rmm, "appendRows", &VMatrix::appendRows,
        (BodyDoc("Appends rows to the VMatrix.\n"),
         ArgDoc ("rows", "A matrix containing the rows to append.\n")));

    declareMethod(
        rmm, "putRow", &VMatrix::putRow,
        (BodyDoc("Store a row into the VMatrix.\n"),
         ArgDoc ("i", "Index of the row being modified.\n"),
         ArgDoc ("v", "Vec with values (row) to store.\n")));

    declareMethod(
        rmm, "saveFieldInfos", &VMatrix::saveFieldInfos,
        (BodyDoc("Saves field names, etc. in metadatadir.\n")));

    declareMethod(
        rmm, "flush", &VMatrix::flush,
        (BodyDoc("Flush mods. to disk.\n")));

    declareMethod(
        rmm, "getBoundingBox", &VMatrix::getBoundingBox,
        (BodyDoc("Returns the (possibly enlarged) bounding box of the data."),
         ArgDoc ("extra_percent", "if non 0, then the box is enlarged in both ends\n"
                 "of every direction by that given percentage"),
         RetDoc ("bounding box as as a vector of (min,max) pairs")));         

    declareMethod(
        rmm, "fill", &VMatrix::fill,
        (BodyDoc("Appends fills the VMatrix with a constant value.\n"),
         ArgDoc ("value", "The fill value.\n")));

    declareMethod(
        rmm, "dot", &VMatrix::dot,
        (BodyDoc("dot product between row i1 and row i2, w/ inputsize first elements."),
         ArgDoc ("i1", "First row to consider."),
         ArgDoc ("i2", "Second row to consider."),
         ArgDoc ("inputsize", "nb. elements to consider."),
         RetDoc ("dot product")));         

    declareMethod(
        rmm, "saveAMAT", &VMatrix::saveAMAT,
        (BodyDoc("Saves this matrix as an .amat file."),
         ArgDoc ("amatfile", "Path of the file to create."),
         ArgDoc ("verbose", "output details?"),
         ArgDoc ("no_header", "save data only"),
         ArgDoc ("save_strings", "save string instead of real values")));

    declareMethod(
        rmm, "savePMAT", &VMatrix::remote_savePMAT,
        (BodyDoc("Saves this matrix as a .pmat file."),
         ArgDoc ("pmatfile", "Path of the file to create.")));

    declareMethod(
        rmm, "savePMAT_float", &VMatrix::remote_savePMAT_float,
        (BodyDoc("Saves this matrix as a .pmat file in float format."),
         ArgDoc ("pmatfile", "Path of the file to create.")));

    declareMethod(
        rmm, "saveDMAT", &VMatrix::saveDMAT,
        (BodyDoc("Saves this matrix as a .dmat directory."),
         ArgDoc ("dmatdir", "Path of the dir to create.")));

    declareMethod(
        rmm, "subMat", &VMatrix::subMat,
        (BodyDoc("Return a sub-matrix from a VMatrix\n"),
         ArgDoc ("i", "start row"),
         ArgDoc ("j", "start col"),
         ArgDoc ("l", "length"),
         ArgDoc ("w", "width"),
         RetDoc ("The sub-VMatrix")));

    declareMethod(
        rmm, "get", &VMatrix::get,
        (BodyDoc("Returns the element at position (i,j)\n"),
         ArgDoc ("i", "row"),
         ArgDoc ("j", "col"),
         RetDoc ("Value at (i,j)")));


    declareMethod(
        rmm, "getStats", &VMatrix::remote_getStats,
        (BodyDoc("Returns the unconditonal statistics for all fields\n"),
         RetDoc ("Stats vector")));

    declareMethod(
        rmm, "defineSizes", &VMatrix::defineSizes,
        (BodyDoc("Define this vmatrix's sizes\n"),
         ArgDoc ("inputsize", "inputsize"),
         ArgDoc ("targetsize", "targetsize"),
         ArgDoc ("weightsize", "weightsize"),
         ArgDoc ("extrasize", "extrasize")));

    declareMethod(
        rmm, "copySizesFrom", &VMatrix::copySizesFrom,
        (BodyDoc("Define this vmatrix's sizes from another vmatrix\n"),
         ArgDoc ("vm", "the other vmatrix")));

    declareMethod(
        rmm, "addStringMapping", static_cast<void (VMatrix::*)(int, string, real)>(&VMatrix::addStringMapping),
        (BodyDoc("Add or replace a string mapping for a column\n"),
         ArgDoc ("col", "column number"),
         ArgDoc ("str", "string value"),
         ArgDoc ("val", "numeric value")));

    declareMethod(
        rmm, "getStringToRealMapping", &VMatrix::getStringToRealMapping,
        (BodyDoc("Get the string->real mapping for a given column.\n"),
         ArgDoc ("col", "column number"),
         RetDoc ("map of string->real")));

    declareMethod(
        rmm, "getRealToStringMapping", &VMatrix::getRealToStringMapping,
        (BodyDoc("Get the real->string mapping for a given column.\n"),
         ArgDoc ("col", "column number"),
         RetDoc ("map of real->string")));

    declareMethod(
        rmm, "setMetaInfoFrom", &VMatrix::setMetaInfoFrom,
        (BodyDoc("Set this vmatrix's meta-info from another vmatrix\n"),
         ArgDoc ("vm", "the other vmatrix")));

}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(get_row,     copies);
    deepCopyField(dotrow_1,    copies);
    deepCopyField(dotrow_2,    copies);
    deepCopyField(field_stats, copies);
    deepCopyField(map_sr,      copies);
    deepCopyField(map_rs,      copies);
    deepCopyField(fieldinfos,  copies);
    deepCopyField(fieldstats,  copies);

    // TODO See if we can deep-copy a PStream (and what it means).
}

/////////////////
// init_map_sr //
/////////////////
void VMatrix::init_map_sr() const
{
    if (map_sr.length()==0 || map_sr.length() != width()) {
        map_sr.resize(width());
        map_rs.resize(width());
    }
}


///////////////////
// getFieldInfos //
///////////////////
Array<VMField>& VMatrix::getFieldInfos() const
{
    if(fieldinfos.size()==0 && hasMetaDataDir())
    {
        PPath fname =  getMetaDataDir() / "fieldnames";
        if(isfile(fname)) // file exists
            loadFieldInfos();
    }

    int ninfos = fieldinfos.size();
    int w = width();
    if(ninfos!=w && w > 0)
    {
        fieldinfos.resize(w);
        for(int j=ninfos; j<w; j++)
            fieldinfos[j] = VMField(tostring(j));
    }

    return fieldinfos;
}

///////////////////
// setFieldInfos //
///////////////////
void VMatrix::setFieldInfos(const Array<VMField>& finfo) const
{
    fieldinfos=finfo;
}

///////////////////
// hasFieldInfos //
///////////////////
bool VMatrix::hasFieldInfos() const
{
    if (fieldinfos.length() != width())
        return false;
    // If there are some field infos, we check them to see whether they are
    // default ones (i.e. 0, 1, ..., width-1), in which case 'false' is
    // returned.
    double x;
    for (int i = 0; i < width(); i++) {
        string name = fieldName(i);
        if (!pl_isnumber(name, &x)  || !is_equal(x, i))
            return true;
    }
    return false;
}

///////////////////////////
// unduplicateFieldNames //
///////////////////////////
void VMatrix::unduplicateFieldNames()
{
    map<string,vector<int> > mp;
    for(int i=0;i<width();i++)
        mp[getFieldInfos(i).name].push_back(i);
    map<string,vector<int> >::iterator it;
    for(it=mp.begin();it!=mp.end();++it)
        if(it->second.size()!=1)
        {
            vector<int> v=it->second;
            for(unsigned int j=0;j<v.size();j++)
                fieldinfos[v[j]].name+="."+tostring(j);
        }
}

////////////////
// fieldNames //
////////////////
TVec<string> VMatrix::fieldNames() const
{
    int d = width();
    if (d < 0)
        return TVec<string>();
    TVec<string> names(d);
    for(int i=0; i<d; i++)
        names[i] = fieldName(i);
    return names;
}

TVec<string> VMatrix::inputFieldNames() const
{ return fieldNames().subVec(0,inputsize_); }

TVec<string> VMatrix::targetFieldNames() const
{ return fieldNames().subVec(inputsize_, targetsize_); }

TVec<string> VMatrix::weightFieldNames() const
{ return fieldNames().subVec(inputsize_+targetsize_, weightsize_); }

TVec<string> VMatrix::extraFieldNames() const
{ return fieldNames().subVec(inputsize_+targetsize_+weightsize_,extrasize_); }

////////////////
// fieldIndex //
////////////////
int VMatrix::fieldIndex(const string& fieldname) const
{
    Array<VMField>& infos = getFieldInfos();
    for(int i=0; i<width(); i++)
        if(infos[i].name==fieldname)
            return i;
    return -1;
}

///////////////////
// getFieldIndex //
///////////////////
int VMatrix::getFieldIndex(const string& fieldname_or_num, bool error) const
{
    int i = fieldIndex(fieldname_or_num);
    if(i==-1 && pl_islong(fieldname_or_num)) {
        i = toint(fieldname_or_num);

        // Now ensure that THE WHOLE FIELD has been converted, because we want
        // to ensure that stuff that starts with a number but contains other
        // things is not silently converted to the starting number
        if (tostring(i) != fieldname_or_num)
            i = -1;
    }
    if ((i < 0 || i >= width()) && error)
        PLERROR("In VMatrix::getFieldIndex - Asked for an invalid column number: '%s'",
                fieldname_or_num.c_str());
    return i;
}

////////////
// build_ //
////////////
void VMatrix::build_()
{
    if(!metadatadir.isEmpty())
        setMetaDataDir(metadatadir); // make sure we perform all necessary operations
    if(mtime_update == time_t(-1))
        updateMtime(0);
    else if(mtime_update!=0)
        updateMtime(mtime_update);
}

///////////
// build //
///////////
void VMatrix::build()
{
    inherited::build();
    build_();
}

////////////////////
// printFieldInfo //
////////////////////
void VMatrix::printFieldInfo(PStream& out, int fieldnum, bool print_binning) const
{
    VMField fi = getFieldInfos(fieldnum);
    StatsCollector& s = getStats(fieldnum);

    out << "Field #" << fieldnum << ":  ";
    out << fi.name << "\t type: ";
    switch(fi.fieldtype)
    {
    case VMField::UnknownType:
        out << "UnknownType\n";
        break;
    case VMField::Continuous:
        out << "Continuous\n";
        break;
    case VMField::DiscrGeneral:
        out << "DiscrGeneral\n";
        break;
    case VMField::DiscrMonotonic:
        out << "DiscrMonotonic\n";
        break;
    case VMField::DiscrFloat:
        out << "DiscrFloat\n";
        break;
    case VMField::Date:
        out << "Date\n";
        break;
    default:
        PLERROR("Can't write name of type");
    }

    map<real,StatsCollectorCounts>::const_iterator it = s.counts.begin();
    map<real,StatsCollectorCounts>::const_iterator countsend = s.counts.end();
    int n_values = 0;
    //some value(FLT_MAX, meaby others) are used for others purpose.
    //We must not cont then.
    while(it!=countsend)
    {
        real val = it->first;
        const StatsCollectorCounts& co = it->second;
        string str = getValString(fieldnum, val);
        if(co.n>0)
            n_values++;
        ++it;
        }
    char plus = ' ';
    if (n_values==s.maxnvalues)
        plus = '+';

    out << "nmissing: " << s.nmissing() << '\n';
    out << "nnonmissing: " << s.nnonmissing() << '\n';
    out << "sum: " << s.sum() << '\n';
    out << "mean: " << s.mean() << '\n';
    out << "stddev: " << s.stddev() << '\n';
    out << "min: " << s.min() << '\n';
    out << "max: " << s.max() << '\n';
    out << "ndiffvalue: " << n_values << plus << '\n';

    if(!s.counts.empty() && print_binning)
    {
        out << "\nCOUNTS: \n";
        map<real,StatsCollectorCounts>::const_iterator it = s.counts.begin();
        map<real,StatsCollectorCounts>::const_iterator countsend = s.counts.end();
        while(it!=countsend)
        {
            real val = it->first;
            const StatsCollectorCounts& co = it->second;
            string str = getValString(fieldnum, val);
            ostringstream os;
            os.setf(ios::left);
            os << "  "          << setw(12) << val
               << "  "          << setw(12) << str
               << "  n="        << setw(10) << co.n
               << "  nbelow="   << setw(10) << co.nbelow
               << "  sumbelow=" << setw(10) << co.sum
               << endl;
            out << os.str();
            ++it;
        }
    }
    out << endl << endl;
}

////////////////////
// printFieldInfo //
////////////////////
void VMatrix::printFieldInfo(PStream& out, const string& fieldname_or_num,
                             bool print_binning) const
{
    printFieldInfo(out, getFieldIndex(fieldname_or_num), print_binning);
}

/////////////////
// printFields //
/////////////////
void VMatrix::printFields(PStream& out) const
{
    for(int j=0; j<width(); j++)
    {
        printFieldInfo(out,j);
        out << "-----------------------------------------------------" << endl;
    }
}

////////////////
// getExample //
////////////////
void VMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
    if(inputsize_<0)
        PLERROR("In VMatrix::getExample, inputsize_ not defined for this vmat");
    input.resize(inputsize_);
    getSubRow(i,0,input);
    if(targetsize_<0)
        PLERROR("In VMatrix::getExample, targetsize_ not defined for this vmat");
    target.resize(targetsize_);
    if (targetsize_ > 0) {
        getSubRow(i,inputsize_,target);
    }

    if(weightsize_==0)
        weight = 1;
    else if(weightsize_<0)
        PLERROR("In VMatrix::getExample, weightsize_ not defined for this vmat");
    else if(weightsize_>1)
        PLERROR("In VMatrix::getExample, weightsize_ >1 not supported by this call");
    else
        weight = get(i,inputsize_+targetsize_);
}

///////////////////////
// remote_getExample //
///////////////////////
boost::tuple<Vec, Vec, real> VMatrix::remote_getExample(int i)
{
    Vec input, target;
    real weight;
    getExample(i, input, target, weight);
    return boost::tuple<Vec, Vec, real>(input, target, weight);
}

/////////////////
// getExamples //
/////////////////
void VMatrix::getExamples(int i_start, int length, Mat& inputs, Mat& targets,
                          Vec& weights, Mat* extras, bool allow_circular)
{
    inputs.resize(length, inputsize());
    targets.resize(length, targetsize());
    weights.resize(length);
    if (extras)
        extras->resize(length, extrasize());
    Vec input, target, extra;
    int total_length = this->length();
    PLASSERT( i_start < total_length );
    for (int k = 0; k < length; k++) {
        input = inputs(k);
        target = targets(k);
        int idx = i_start + k;
        if (allow_circular)
            idx %= total_length;
        PLASSERT( idx >= 0 && idx < total_length );
        getExample(idx, input, target, weights[k]);
        if (extras) {
            extra = (*extras)(k);
            getExtra(idx, extra);
        }
    }
}

//////////////
// getExtra //
//////////////
void VMatrix::getExtra(int i, Vec& extra)
{
    if(inputsize_<0 || targetsize_<0 || weightsize_<0 || extrasize_<0)
        PLERROR("In VMatrix::getExtra, sizes not properly defined for this vmat");

    extra.resize(extrasize_);
    if(extrasize_>0)
        getSubRow(i,inputsize_+targetsize_+weightsize_, extra);
}

Vec VMatrix::remote_getExtra(int i)
{
    Vec extra;
    getExtra(i, extra);
    return extra;
}


//////////////////
// computeStats //
//////////////////
void VMatrix::computeStats()
{
    fieldstats = Array<VMFieldStat>(width());
    Vec row(width());
    for(int i=0; i<length(); i++)
    {
        getRow(i,row);
        for(int j=0; j<width(); j++)
            fieldstats[j].update(row[j]);
    }
}

///////////////
// loadStats //
///////////////
void VMatrix::loadStats(const PPath& filename)
{
    PStream in = openFile(filename, PStream::raw_ascii, "r");
    int nfields;
    in >> nfields;
    if(nfields!=width())
        PLWARNING("In VMatrix::loadStats - nfields differs from VMat width");

    fieldstats.resize(nfields);
    for(int j=0; j<fieldstats.size(); j++)
        fieldstats[j].read(in);
}

///////////////
// saveStats //
///////////////
void VMatrix::saveStats(const PPath& filename) const
{
    PStream out = openFile(filename, PStream::raw_ascii, "w");
    out << fieldstats.size() << endl;
    for(int j=0; j<fieldstats.size(); j++)
    {
        fieldstats[j].write(out);
        out << endl;
    }
}

//////////////////
// declareField //
//////////////////
void VMatrix::declareField(int fieldindex, const string& fieldname, VMField::FieldType fieldtype)
{
    getFieldInfos(fieldindex) = VMField(fieldname,fieldtype);
}

///////////////////////
// declareFieldNames //
///////////////////////
void VMatrix::declareFieldNames(const TVec<string>& fnames)
{
    if(fnames.length()!=width())
        PLERROR("In VMatrix::declareFieldNames length of fnames differs from width() of VMatrix");
    for(int i=0; i<fnames.length(); i++)
        declareField(i,fnames[i]);
}

////////////////////
// saveFieldInfos //
////////////////////
void VMatrix::saveFieldInfos() const
{
    // check if we need to save the fieldinfos
    if(fieldinfos.size() > 0) {
        Array<VMField> current_fieldinfos;
        try{
            current_fieldinfos = getSavedFieldInfos();
        }catch(PLearnError){}
        if (current_fieldinfos != fieldinfos) {

            // Ensure that the metadatadir exists
            if(!force_mkdir(getMetaDataDir()))
                PLERROR("In VMatrix::saveFieldInfos: could not create directory %s",
                        getMetaDataDir().absolute().c_str());

            PPath filename = getMetaDataDir() / "fieldnames";
            PStream out = openFile(filename, PStream::raw_ascii, "w");
            for(int i= 0; i < fieldinfos.length(); ++i)
                out << fieldinfos[i].name << '\t' << fieldinfos[i].fieldtype << endl;
        }
    }

    // check if we need to save the sizes
    int inp, targ, weight, extr;
    bool sizes_exist = getSavedSizes(inp, targ, weight, extr);
    if ((! sizes_exist && (inputsize_ != -1 || targetsize_ != -1 || weightsize_ != -1 || extrasize_ > 0)) ||
        (sizes_exist && (inp != inputsize_ || targ != targetsize_ || weight != weightsize_ || extr!=extrasize_)))
    {
        // Slightly hackish phenomenon :: if the sizes file doesn't previously
        // exist and we cannot write them, THIS IS NOT AN ERROR.  In this case,
        // just catch the error and continue
        try {
            // Ensure that the metadatadir exists
            if(!force_mkdir(getMetaDataDir()))
                PLERROR("In VMatrix::saveFieldInfos: could not create directory %s",
                        getMetaDataDir().absolute().c_str());

            PPath filename = getMetaDataDir() / "sizes";
            PStream out = openFile(filename, PStream::plearn_ascii, "w");
            out << inputsize_ << targetsize_ << weightsize_ << extrasize_ << endl;
        }
        catch (const PLearnError&) {
            if (sizes_exist)
                throw;
        }
    }
}

////////////////////
// loadFieldInfos //
////////////////////
void VMatrix::loadFieldInfos() const
{
    Array<VMField> current_fieldinfos = getSavedFieldInfos();
    setFieldInfos(current_fieldinfos);

    // Update only if they can successfully be read from the saved metadata
    // and they don't already exist in the VMatrix
    int inp, tar, weight, extr;
    if (inputsize_ == -1 && targetsize_ == -1 && weightsize_ == -1 
        && getSavedSizes(inp,tar,weight,extr))
    {
        inputsize_  = inp;
        targetsize_ = tar;
        weightsize_ = weight;
        extrasize_  = extr;
    }
}

////////////////////////
// getSavedFieldInfos //
////////////////////////
Array<VMField> VMatrix::getSavedFieldInfos() const
{
    PPath filename = getMetaDataDir() / "fieldnames";
    if (!isfile(filename)) // no current fieldinfos saved
    {
        Array<VMField> no_fieldinfos(0);
        return no_fieldinfos;
    }
    PStream in = openFile(filename, PStream::raw_ascii, "r");
    int w = width();
    Array<VMField> current_fieldinfos(w);
    for(int i=0; i<w; ++i)
    {
        string line = in.getline();
        vector<string> v(split(line));
        switch(v.size())
        {
        case 1: current_fieldinfos[i] = VMField(v[0]); break;
        case 2: current_fieldinfos[i] = VMField(v[0], VMField::FieldType(toint(v[1]))); break;
        default: PLERROR("In VMatrix::getSavedFieldInfos Format not recognized in file %s.\n"
                         "Each line should be '<name> {<type>}'.\n"
                         "Got: '%s'. Check for a space in <name>",
                         filename.absolute().c_str(),line.c_str());
        }
    }
    return current_fieldinfos;
}

///////////////////
// getSavedSizes //
///////////////////
bool VMatrix::getSavedSizes(int& inputsize, int& targetsize, int& weightsize, int& extrasize) const
{
    PPath filename = getMetaDataDir() / "sizes";
    inputsize = targetsize = weightsize = extrasize = -1;
    if (isfile(filename))
    {
        PStream in = openFile(filename, PStream::plearn_ascii, "r");
        // perr << "In loadFieldInfos() loading sizes from " << filename << endl;
        in >> inputsize >> targetsize >> weightsize;
        in.skipBlanks();
        extrasize = 0;
        if(in.peek()!=EOF)
            in >> extrasize;
        return true;                         // Successfully loaded
    }
    return false;
}


//////////////////////////
// resolveFieldInfoLink //
//////////////////////////
string VMatrix::resolveFieldInfoLink(const PPath& target, const PPath& source)
{
    PPath contents = removeblanks( loadFileAsString(source) );
    if ( contents == source )
        return "ERROR";

    if( isdir(contents) )
    {
        if ( isfile(contents/target+".lnk") )
            return resolveFieldInfoLink(target,contents/target+".lnk");

        else if ( isfile(contents/target) )
            return contents/target;

        else if( isfile(contents/"__default.lnk") )
            return resolveFieldInfoLink(target, contents/"__default.lnk");

        // assume target is there, but file is empty thus inexistant
        else return contents/target;
    }

    else if( contents.extension() == "lnk" )
        return resolveFieldInfoLink(target,contents);

    else return contents;
}

//////////////////////
// getSFIFDirectory //
//////////////////////
PPath VMatrix::getSFIFDirectory() const
{
    PPath meta = getMetaDataDir();
    if (meta.empty())
        PLERROR("%s: cannot have a SFIFDirectory if there is no metadatadir",
                __FUNCTION__);
    return meta / "FieldInfo";
}

/////////////////////
// setSFIFFilename //
/////////////////////
void VMatrix::setSFIFFilename(int col, string ext, const PPath& filepath)
{
    setSFIFFilename(fieldName(col),ext,filepath);
}

void VMatrix::setSFIFFilename(string fieldname, string ext, const PPath& filepath)
{
    PPath target      = makeFileNameValid(fieldname+ext);
    PPath normalfname = getSFIFDirectory() / target;
    PPath normalfname_lnk = normalfname + ".lnk";

    rm(normalfname_lnk);
    if(filepath==normalfname || filepath=="")
    {
        rm(normalfname_lnk);
        return;
    }

    force_mkdir_for_file(normalfname);
    PStream o = openFile(normalfname_lnk, PStream::raw_ascii, "w");
    o<<filepath<<endl;
}

/////////////////////
// getSFIFFilename //
/////////////////////
PPath VMatrix::getSFIFFilename(int col, string ext)
{
    return getSFIFFilename(fieldName(col),ext);
}

PPath VMatrix::getSFIFFilename(string fieldname, string ext)
{
    PPath  target           = makeFileNameValid(fieldname+ext);
    PPath  normalfname      = getSFIFDirectory() / target;
    string defaultlinkfname = getSFIFDirectory() / "__default.lnk";

    if(isfile(normalfname))
        return normalfname;
    else if(isfile(normalfname+".lnk"))
        return resolveFieldInfoLink(target, normalfname+".lnk");
    else if(isfile(defaultlinkfname))
        return resolveFieldInfoLink(target, defaultlinkfname);
    // assume target is here, but file is empty thus inexistant
    else return normalfname;
}

//////////////////
// isSFIFDirect //
//////////////////
bool VMatrix::isSFIFDirect(int col, string ext)
{
    return isSFIFDirect(fieldName(col), ext);
}

bool VMatrix::isSFIFDirect(string fieldname, string ext)
{
    PPath target      = makeFileNameValid(fieldname+ext);
    PPath normalfname = getSFIFDirectory() / target;
    return getSFIFFilename(fieldname,ext) == normalfname;
}

//////////////////////
// addStringMapping //
//////////////////////
void VMatrix::addStringMapping(int col, string str, real val)
{
    init_map_sr();
    map_sr[col][str]=val;
    map_rs[col][val]=str;
}

//////////////////////
// addStringMapping //
//////////////////////
real VMatrix::addStringMapping(int col, string str)
{
    init_map_sr();
    map<string,real>& m = map_sr[col];
    map<string,real>::iterator it = m.find(str);

    real val = 0;
    if(it != m.end()) // str was found in map
        val = it->second;
    else // str not found in map: add a new mapping
    {
        val = - real(m.size()) - 100;
        addStringMapping(col, str, val);
    }
    return val;
}

/////////////////////////////
// removeAllStringMappings //
/////////////////////////////
void VMatrix::removeAllStringMappings()
{
    init_map_sr();
    for(int i=0;i<width();i++)
    {
        map_sr[i].clear();
        map_rs[i].clear();
    }
}

////////////////////////////////
// removeColumnStringMappings //
////////////////////////////////
void VMatrix::removeColumnStringMappings(int c)
{
    init_map_sr();
    map_sr[c].clear();
    map_rs[c].clear();
}

///////////////////////////
// saveAllStringMappings //
///////////////////////////
void VMatrix::saveAllStringMappings()
{
    PPath fname;
    map<string, real> the_map;
    for(int i=0;i<width();i++)
    {
        the_map = getStringToRealMapping(i);
        if (!the_map.empty()) {
            fname = getSFIFFilename(i,".smap");
            saveStringMappings(i, fname, &the_map);
        }
    }
}

////////////////////////
// saveStringMappings //
////////////////////////
void VMatrix::saveStringMappings(int col, const PPath& fname, map<string, real>* str_to_real)
{
    map<string, real> the_map;
    if (!str_to_real) {
        the_map = getStringToRealMapping(col);
        str_to_real = &the_map;
    }
    if(str_to_real->empty())
    {
        rm(fname);
        return;
    }
    force_mkdir_for_file(fname);
    PStream o = openFile(fname, PStream::plearn_ascii, "w");
    for(map<string,real>::iterator it  = str_to_real->begin();
        it != str_to_real->end();   ++it)
        o << it->first << it->second << endl;
}

/////////////////////////
// removeStringMapping //
/////////////////////////
void VMatrix::removeStringMapping(int col, string str)
{
    init_map_sr();
    map<string,real>::iterator sriterator;
    // Check if the mapping actually exists.
    if((sriterator = map_sr[col].find(str)) == map_sr[col].end())
        return;
    real val = map_sr[col][str];
    map_sr[col].erase(sriterator);
    map_rs[col].erase(map_rs[col].find(val));
}

//////////////////////
// setStringMapping //
//////////////////////
void VMatrix::setStringMapping(int col, const map<string,real> & zemap)
{
    init_map_sr();
    map_sr[col]=zemap;
    map_rs[col].clear();
    for(map<string,real>::iterator it = map_sr[col].begin();it!=map_sr[col].end();++it)
        map_rs[col][it->second]=it->first;
}

/////////////////////////
// deleteStringMapping //
/////////////////////////
void VMatrix::deleteStringMapping(int col)
{
    init_map_sr();
    if(col>=map_sr.size() ||
       col>=map_rs.size())
        PLERROR("deleteStringMapping : out of bounds for col=%i in string mapping array (size=%i).\n Current VMatrix\nclass"\
                "is '%s' (or maybe derivated class?). be sure to set\n map_sr(rs) to appropriate sizes as soon as you know the width of the matrix\n"\
                "(in constructor or elsewhere)",col,map_sr.size(),classname().c_str());
    map_sr[col].clear();
    map_rs[col].clear();
}

//////////////////
// getValString //
//////////////////
string VMatrix::getValString(int col, real val) const
{
    if(is_missing(val))
        return "";
    init_map_sr();
    if(map_rs[col].find(val)==map_rs[col].end())
        return "";
    else return map_rs[col][val];
}

//////////////////
// getStringVal //
//////////////////
real VMatrix::getStringVal(int col,const string & str) const
{
    if(map_sr.length()==0 || map_sr[col].find(str)==map_sr[col].end())
        return MISSING_VALUE;
    else return map_sr[col][str];
}

///////////////
// getString //
///////////////
string VMatrix::getString(int row,int col) const
{
    real val = get(row,col);
    string str = getValString(col, val);
    if (str == "")
        // There is no string mapping associated to this value.
        return tostring(val);
    else
        return str;
}

/////////////////////
// getRowAsStrings //
/////////////////////
void VMatrix::getRowAsStrings(int i, TVec<string>& v_str) const {
    v_str.resize(width());
    for (int j = 0; j < width(); j++)
        v_str[j] = getString(i, j);
}


////////////////////////////
// getStringToRealMapping //
////////////////////////////
const map<string,real>& VMatrix::getStringToRealMapping(int col) const {
    init_map_sr();
    return map_sr[col];
}

////////////////////////////
// getRealToStringMapping //
////////////////////////////
const map<real,string>& VMatrix::getRealToStringMapping(int col) const {
    init_map_sr();
    return map_rs[col];
}

////////////////////
// setMetaDataDir //
////////////////////
void VMatrix::setMetaDataDir(const PPath& the_metadatadir)
{
    if (the_metadatadir.isEmpty())
        PLERROR("In VMatrix::setMetaDataDir - Called setMetaDataDir with an empty PPath");
    metadatadir = the_metadatadir.absolute() / "";
    // We do not create the metadata directory here anymore.
    // This is to prevent the proliferation of useless directories.
    // A VMatrix's subclass should now create the metadatadir itself if it needs it.

    // Load string mappings from the metadatadir.
    loadAllStringMappings();
}

///////////////////
// getDictionary //
///////////////////
PP<Dictionary> VMatrix::getDictionary(int col) const
{
    return 0;
}

///////////////
// getValues //
///////////////
void VMatrix::getValues(int row, int col, Vec& values) const 
{ 
    values.resize(0);
}

void VMatrix::getValues(const Vec& input, int col, Vec& values) const 
{ 
    values.resize(0);
}


///////////////////
// copySizesFrom //
///////////////////
void VMatrix::copySizesFrom(const VMat& m) {
    defineSizes(m->inputsize(), m->targetsize(), m->weightsize(), m->extrasize());
}

/////////////////////
// setMetaInfoFrom //
/////////////////////
void VMatrix::setMetaInfoFrom(const VMatrix* vm)
{
    updateMtime(vm->getMtime());

    // copy length and width from vm if not set
    if(length_<0)
        length_ = vm->length();
    if(width_<0)
        width_ = vm->width();

    // Copy sizes from vm if not set and they do not conflict with the width.
    int current_w = max(0, inputsize_) + max(0, targetsize_) +
                    max(0, weightsize_) + max(0, extrasize_);
    int is = vm->inputsize();
    if(inputsize_<0 && is>=0) {
        if (is + current_w <= width_) {
            inputsize_ = is;
            current_w += is;
        }
    }
    int ts = vm->targetsize();
    if(targetsize_<0 && ts>=0) {
        if (ts + current_w <= width_) {
            targetsize_ = ts;
            current_w += ts;
        }
    }
    int ws = vm->weightsize();
    if(weightsize_<0 && ws>=0) {
        if (ws + current_w <= width_) {
            // We must also ensure the total sum of sizes (if available)
            // will match the width. Otherwise we may end up with sizes
            // conflicting with the width.
            if (inputsize_ < 0 || targetsize_ < 0 || extrasize_ < 0 ||
                inputsize_ + targetsize_ + extrasize_ + ws == width_)
            {
                weightsize_ = ws;
                current_w += ws;
            }
        }
    }
    int es = vm->extrasize();
    if(extrasize_<=0 && es>=0) {
        if (es + current_w <= width_) {
            // Same as above.
            if (inputsize_ < 0 || targetsize_ < 0 || weightsize_ < 0 ||
                inputsize_ + targetsize_ + weightsize_ + es == width_)
            {
                extrasize_ = es;
                current_w += es;
            }
        }
    }

    // Fill missing size if possible, also display warning when sizes are not
    // compatible with the width.
    computeMissingSizeValue(false);

    // Copy fieldnames from vm if not set and they look good.
    bool same_fields_as_source =
        (!hasFieldInfos() && (width() == vm->width()) && vm->hasFieldInfos());
    if(same_fields_as_source)
        setFieldInfos(vm->getFieldInfos());

    // Copy string <-> real mappings for fields which have the same name (or for
    // all fields if it looks like the fields are the same as the source).
    TVec<string> fnames = fieldNames();
    for (int i = 0; i < width_; i++) {
        int vm_index = -1;
        if (same_fields_as_source)
            vm_index = i;
        else if (!pl_isnumber(fnames[i]))
            vm_index = vm->fieldIndex(fnames[i]);
        if (vm_index >= 0)
            // The source VMatrix has a field with the same name (which is not a
            // number): we can get its string mapping.
            setStringMapping(i, vm->getStringToRealMapping(vm_index));
    }

    //we save it now in case the program crash
    if(hasMetaDataDir())
        saveFieldInfos();
}

////////////////////
// looksTheSameAs //
////////////////////
bool VMatrix::looksTheSameAs(const VMat& m) {
    return !(
        this->width()      != m->width()
        || this->length()     != m->length()
        || this->inputsize()  != m->inputsize()
        || this->weightsize() != m->weightsize()
        || this->targetsize() != m->targetsize()
        || this->extrasize()  != m->extrasize() );
}

/////////////////////////
// compatibleSizeError //
/////////////////////////
void VMatrix::compatibleSizeError(const VMat& m, const string& extra_msg) {
#define MY_PRINT_ERROR_MST(NAME) PLERROR("In VMatrix::compatibleSizeError " \
        " - in class %s - The matrices are not compatible!\n"               \
        "m1."#NAME"=%d and m2."#NAME"=%d. \n%s",                            \
        classname().c_str(), this->NAME(), m->NAME(), extra_msg.c_str());

    if(this->width()      != m->width())
        MY_PRINT_ERROR_MST(width)
    else if(this->inputsize()  != m->inputsize())
        MY_PRINT_ERROR_MST(inputsize)
    else if(this->weightsize() != m->weightsize())
        MY_PRINT_ERROR_MST(weightsize)
    else if(this->targetsize() != m->targetsize())
        MY_PRINT_ERROR_MST(targetsize)
    else if(this->extrasize()  != m->extrasize() )
        MY_PRINT_ERROR_MST(extrasize)
#undef MY_PRINT_ERROR_MST
}

/////////////////////
// lockMetaDataDir //
/////////////////////
void VMatrix::lockMetaDataDir(time_t max_lock_age, bool verbose) const
{
#ifndef DISABLE_VMATRIX_LOCK
    if(!hasMetaDataDir())
        PLERROR("In VMatrix::lockMetaDataDir() subclass %s -"
                " metadatadir was not set", classname().c_str());
    if(lockf_.good()) // Already locked by this object!
        PLERROR("VMatrix::lockMetaDataDir() subclass %s -"
                " called while already locked by this object.",
                classname().c_str());
    if(!pathexists(metadatadir))
        force_mkdir(metadatadir);

    PPath lockfile = metadatadir / ".lock";
    while (isfile(lockfile) && (max_lock_age == 0 || mtime(lockfile) + max_lock_age > time(0))) {
        // There is a lock file, and it is not older than 'max_lock_age'.
        string bywho;
        try{ 
            PStream st = openFile(lockfile, PStream::raw_ascii, "r", false);
            if(st.good())
                st.read(bywho, streamsize(filesize(lockfile)));
        }
        catch(const PLearnError& e) {
            PLERROR("In VMatrix::lockMetaDataDir - Catching exceptions is"
                    " dangerous in PLearn (memory"
                    " leaks may occur), thus I prefer to stop here. "
                    " Comment this line if you don't care."
                    " The error message is: %s",e.message().c_str());
            bywho = "UNKNOWN (could not read .lock file)" ;
        } catch(...) {
            PLERROR("In VMatrix::lockMetaDataDir - Catching exceptions is dangerous in PLearn (memory "
                    "leaks may occur), thus I prefer to stop here. Comment this line if you don't care.");
            bywho = "UNKNOWN (could not read .lock file)";
        }

        if (verbose)
            perr << "Waiting for .lock in directory " << metadatadir
                 << " created by " << bywho << endl;
        sleep(uniform_multinomial_sample(10) + 1); // Random wait for more safety.
    }
    lockf_ = openFile(lockfile, PStream::raw_ascii, "w");
    string lock_content = "host " + hostname() + ", pid " + tostring(getPid()) + ", user " + getUser();
    lockf_ << lock_content;
    lockf_.flush();
#endif
}

///////////////////////
// unlockMetaDataDir //
///////////////////////
void VMatrix::unlockMetaDataDir() const
{
#ifndef DISABLE_VMATRIX_LOCK
    if(!lockf_)
        PLERROR("In VMatrix::unlockMetaDataDir() was called while no lock is held by this object");
    lockf_ = PStream();   // Release the lock.
    PPath lockfile = metadatadir / ".lock";
    rm(lockfile); // Remove the file.
#endif
}

////////////////////
// getMetaDataDir //
////////////////////
PPath VMatrix::getMetaDataDir() const
{
    // TODO Remove ?
    //  if(!hasMetaDataDir())
    //  PLERROR("In VMatrix::getMetaDataDir(): metadatadir was not set");
    return metadatadir;
}

///////////////////////////
// loadAllStringMappings //
///////////////////////////
void VMatrix::loadAllStringMappings()
{
    if (! hasMetaDataDir() || ! isdir(getSFIFDirectory()))
        return;
    
    for(int i=0;i<width();i++)
        loadStringMapping(i);
}

///////////////////////
// loadStringMapping //
///////////////////////
void VMatrix::loadStringMapping(int col)
{
    if(!hasMetaDataDir())
        return;
    PPath fname = getSFIFFilename(col,".smap");
    init_map_sr();
    string SFIFdir= getSFIFDirectory();
    if(!pathexists(SFIFdir))
        force_mkdir(SFIFdir);
    if(!isfile(fname))
        return;

    deleteStringMapping(col);

    // smap file exists, open it
    PStream f = openFile(fname, PStream::plearn_ascii);

    // TODO Remove ?
#if 0
    string pref;
    f>>pref;
    if(string(pref)!="#SMAP")
        PLERROR( string("File "+fname+" is not a valid String mapping file.\nShould start with #SMAP on first line (this is to prevent inopportunely overwritting another type of file)").c_str());
#endif

    while(f.good())
    {
        string s;
        real val;
        f >> s >> val;
        if(f.good())
        {
            map_sr[col][s]   = val;
            map_rs[col][val] = s;
            f.skipBlanks();
        }
    }
}

////////////////////////////
// copyStringMappingsFrom //
////////////////////////////
void VMatrix::copyStringMappingsFrom(const VMat& source) {
    if (width_ != source->width()) {
        PLERROR("In VMatrix::copyStringMappingsFrom - The source VMatrix doesn't have the same width");
    }
    map_rs.resize(width_);
    map_sr.resize(width_);
    for (int i = 0; i < width_; i++) {
        setStringMapping(i, source->getStringToRealMapping(i));
    }
}

//////////////
// getStats //
//////////////
TVec<StatsCollector> VMatrix::getStats(bool progress_bar) const
{
    if(!field_stats)
        field_stats = getPrecomputedStatsFromFile("stats.psave", 2000, 
                                                  progress_bar);
    return field_stats;
}

/////////////////
// updateMtime //
/////////////////
void VMatrix::updateMtime(time_t t)
{
    if(t>mtime_ && mtime_!=numeric_limits<time_t>::max())
        mtime_=t;
    else if(t==0)
        mtime_=numeric_limits<time_t>::max();
}
void VMatrix::updateMtime(const PPath& p){if(!p.isEmpty())updateMtime(mtime(p));}

void VMatrix::updateMtime(VMat v){if(v)updateMtime(v->getMtime());}

////////////////
// isUpToDate //
////////////////
bool VMatrix::isUpToDate(const PPath& path, bool warning_mtime0,
                         bool warning_older) const
{
    bool exist = isfile(path);
    bool uptodate = false;
    if(exist)
        uptodate = getMtime() < mtime(path);
    if (warning_mtime0 && exist && uptodate && getMtime()==0)
        PLWARNING("In VMatrix::isUpToDate - for class '%s'"
                  " File '%s' will be used, but "
                  "this VMat's last modification time is undefined: we cannot "
                  "be sure the file is up-to-date.",
                  classname().c_str(), path.absolute().c_str());
    if(warning_older && exist && !uptodate)
        PLWARNING("In VMatrix::isUpToDate - for class '%s'"
                  " File '%s' is older than this "
                  "VMat's mtime of %ld, and should not be re-used.",
                  classname().c_str(), path.absolute().c_str(), long(getMtime()));

    return exist && uptodate;
}

////////////////
// isUpToDate //
////////////////
bool VMatrix::isUpToDate(VMat vm, bool warning_mtime0,
                         bool warning_older) const
{
    time_t my_time = getMtime();
    time_t vm_time = vm->getMtime();
    bool uptodate = my_time < vm_time ||
                    my_time == 0      ||
                    vm_time == 0;
    if (warning_mtime0 && uptodate && (my_time == 0 || vm_time == 0))
        PLWARNING("In VMatrix::isUpToDate - for class '%s'"
                  " When comparing the VMats' last modification times, at "
                  "least one was found to be undefined: we cannot be sure "
                  "the VMat is up-to-date.",
                  classname().c_str());
    if(warning_older && !uptodate)
        PLWARNING("In VMatrix::isUpToDate - for class '%s'"
                  " The VMat with mtime of %ld is older than this "
                  "VMat's with mtime of %ld, and should not be re-used.",
                  classname().c_str(), long(vm->getMtime()), long(getMtime()));

    return uptodate;
}

/////////////////////////////////
// getPrecomputedStatsFromFile //
/////////////////////////////////
TVec<StatsCollector> VMatrix::getPrecomputedStatsFromFile(
        const string& filename, int maxnvalues, bool progress_bar) const
{
    TVec<StatsCollector> stats;
    PPath metadatadir = getMetaDataDir();
    PPath statsfile;
    bool uptodate = false;
    if (hasMetaDataDir()) {
        lockMetaDataDir();
        statsfile =  metadatadir / filename;
        uptodate = isUpToDate(statsfile);
    }
    try{
        if (uptodate){
            PLearn::load(statsfile, stats);
            if(stats.length()!=width()){
                uptodate=false;
                PLWARNING("In VMatrix::getPrecomputedStatsFromFile() for class"
                          " %s - The file %s don't have the good number of"
                          " stats. We regenerate it.",
                        classname().c_str(), statsfile.c_str());
            }
        }
        if(!uptodate){
            VMat vm = const_cast<VMatrix*>(this);
            stats = PLearn::computeStats(vm, maxnvalues, progress_bar);
            if(hasMetaDataDir())
                PLearn::save(statsfile, stats);
        }
    }catch(const PLearnError& e){
        if(!metadatadir.isEmpty())
            unlockMetaDataDir();
        //we erase the file if we are creating it
        // as it can be partilly saved.
        if(!uptodate && isfile(statsfile))
            rm(statsfile);
        throw e;
    }
    if (!metadatadir.isEmpty())
        unlockMetaDataDir();
    return stats;
}

/////////////////////
// remote_getStats //
/////////////////////
TVec<PP<StatsCollector> > VMatrix::remote_getStats() const
{
    if(field_p_stats.isEmpty())
    {
        TVec<StatsCollector> st= getStats();
        field_p_stats.resize(st.length());
        CopiesMap cm;
        for(int i= 0; i < st.length(); ++i)
            field_p_stats[i]= st[i].deepCopy(cm);
    }
    return field_p_stats;
}

////////////////////
// getBoundingBox //
////////////////////
TVec< pair<real,real> > VMatrix::getBoundingBox(real extra_percent) const
{
    TVec<StatsCollector> stats = getStats();
    int n = stats.length();
    TVec< pair<real,real> > bbox(n);
    for(int k=0; k<n; k++)
    {
        StatsCollector& st = stats[k];
        bbox[k] = pair<real,real>(st.min()-extra_percent*st.range(), st.max()+extra_percent*st.range());
    }
    return bbox;
}

///////////////
// getRanges //
///////////////
TVec<RealMapping> VMatrix::getRanges()
{
    TVec<RealMapping> ranges;
    PPath rangefile = getMetaDataDir() / "ranges.psave";
    if(isfile(rangefile))
        PLearn::load(rangefile, ranges);
    else
    {
        ranges = computeRanges(getStats(),std::max(10,length()/200),std::max(10,length()/100) );
        PLearn::save(rangefile, ranges);
    }
    return ranges;
}

/////////
// put //
/////////
void VMatrix::put(int i, int j, real value)
{
    PLERROR("In VMatrix::put - Method not implemented for this VMat(%s), please implement.",
            classname().c_str());
}

///////////////
// getColumn //
///////////////
void VMatrix::getColumn(int j, Vec v) const
{
#ifdef BOUNDCHECK
    if(v.length() != length())
        PLERROR("In VMatrix::getColumn - v must have the same length as the VMatrix");
#endif
    for(int i=0; i<v.length(); i++)
        v[i] = get(i,j);
}

Vec VMatrix::remote_getColumn(int i) const
{
    Vec v(length());
    getColumn(i,v);
    return v;
}


///////////////
// getSubRow //
///////////////
void VMatrix::getSubRow(int i, int j, Vec v) const
{
    for(int k=0; k<v.length(); k++)
        v[k] = get(i,j+k);
}

///////////////
// putSubRow //
///////////////
void VMatrix::putSubRow(int i, int j, Vec v)
{
    for(int k=0; k<v.length(); k++)
        put(i, j+k, v[k]);
}

////////////
// getRow //
////////////
void VMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
    if(v.length() != width())
        PLERROR("In VMatrix::getRow(i,v) length of v and width of VMatrix differ");
#endif
    getSubRow(i,0,v);
}

////////////
// putRow //
////////////
void VMatrix::putRow(int i, Vec v)
{
#ifdef BOUNDCHECK
    if(v.length() != width())
        PLERROR("In VMatrix::putRow(i,v) length of v and width of VMatrix differ");
#endif
    putSubRow(i,0,v);
}

//////////
// fill //
//////////
void VMatrix::fill(real value)
{
    Vec v(width(), value);
    for (int i=0; i<length(); i++) putRow(i,v);
}

///////////////
// appendRow //
///////////////
void VMatrix::appendRow(Vec v)
{
    PLERROR("In VMatrix::appendRow - Not implemented by VMatrix subclass '%s'",
            classname().c_str());
}

///////////////
// insertRow //
///////////////
void VMatrix::insertRow(int i, Vec v)
{
    if (i<0 || i>length_)
        PLERROR("In VMatrix::insertRow: row index (%d) outside valid range [%d,%d]", i, 0, length_);
    else if (i == length_)
        appendRow(v);
    else
    {
        appendRow(v); // dummy operation to increase VMat length
        Vec row(width_);
        for (int j=length_-1; j>i; --j)
        {
            getRow(j-1, row);
            putRow(j, row);
        }
        putRow(i, v);
    }
}

///////////
// flush //
///////////
void VMatrix::flush()
{}

////////////////////
// putOrAppendRow //
////////////////////
void VMatrix::putOrAppendRow(int i, Vec v)
{
    if (v.length() != width())
        PLERROR("In putOrAppendRow, Vec to append must have same length (%d) as VMatrix width (%d)", v.length(), width());

    if(i==length())
        appendRow(v);
    else if(i<length())
        putRow(i,v);
    else
        PLERROR("In putOrAppendRow, index %d out of range",i);
}

/////////////////
// forcePutRow //
/////////////////
void VMatrix::forcePutRow(int i, Vec v)
{
    if (v.length() != width())
        PLERROR("In forcePutRow, Vec to append must have same length (%d) as VMatrix width (%d)", v.length(), width());

    if(i<length())
        putRow(i,v);
    else
    {
        Vec emptyrow(width());
        emptyrow.clear();
        while(length()<i)
            appendRow(emptyrow);
        appendRow(v);
    }
}

////////////
// getMat //
////////////
void VMatrix::getMat(int i, int j, Mat m) const
{
#ifdef BOUNDCHECK
    if(i<0 || j<0 || i+m.length()>length() || j+m.width()>width())
        PLERROR("In VMatrix::getMat(i,j,m) OUT OF BOUNDS");
#endif
    for(int ii=0; ii<m.length(); ii++)
    {
        getSubRow(i+ii, j, m(ii));
    }
}

////////////
// putMat //
////////////
void VMatrix::putMat(int i, int j, Mat m)
{
#ifdef BOUNDCHECK
    if(i<0 || j<0 || i+m.length()>length() || j+m.width()>width())
        PLERROR("In VMatrix::putMat(i,j,m) OUT OF BOUNDS");
#endif
    for(int ii=0; ii<m.length(); ii++)
    {
        putSubRow(i+ii, j, m(ii));
    }
}

///////////////
// compacify //
///////////////
void VMatrix::compacify() {}

///////////
// toMat //
///////////
Mat VMatrix::toMat() const
{
    return toMatCopy();
}

Mat VMatrix::toMatCopy() const
{
    Mat m(length(),width());
    getMat(0,0,m);
    return m;
}

////////////
// subMat //
////////////
VMat VMatrix::subMat(int i, int j, int l, int w)
{ return new SubVMatrix(this,i,j,l,w); }

/////////
// dot //
/////////
real VMatrix::dot(int i1, int i2, int inputsize) const
{
    dotrow_1.resize(inputsize);
    dotrow_2.resize(inputsize);
    getSubRow(i1, 0, dotrow_1);
    getSubRow(i2, 0, dotrow_2);
    return PLearn::dot(dotrow_1, dotrow_2);
}

real VMatrix::dot(int i, const Vec& v) const
{
    dotrow_1.resize(v.length());
    getSubRow(i, 0, dotrow_1);
    return PLearn::dot(dotrow_1, v);
}


//////////
// find //
//////////
bool VMatrix::find(const Vec& input, real tolerance, int* i, int i_start) const
{
    get_row.resize(inputsize());
#ifdef BOUNDCHECK
    if (input.length() != inputsize())
        PLERROR("In VMatrix::find - The given vector must be the same size as "
                "inputsize");
#endif
    int n = length();
    for (int j = 0; j < n; j++) {
        int row = (j + i_start) % n;
        getSubRow(row, 0, get_row);
        if (powdistance(input, get_row, 2.0) < tolerance) {
            if (i)
                *i = row;
            return true;
        }
    }
    if (i)
        *i = -1;
    return false;
}

//////////////
// newwrite //
//////////////
void VMatrix::newwrite(PStream& out) const
{
    /*
      switch(out.outmode)
      {
      case PStream::raw_ascii:
      case PStream::pretty_ascii:
      {
      Vec v(width());
      for(int i=0; i<length(); i++) {
      getRow(i,v);
      out << v << endl;
      }
      break;
      }
      default:
      inherited::newwrite(out);
      }
    */
    inherited::newwrite(out);
}

///////
// ~ //
///////
VMatrix::~VMatrix()
{}

//////////
// save //
//////////
void VMatrix::save(const PPath& filename) const
{
    PLDEPRECATED( "This method overloads the Object::save method which is "
                  "deprecated. This method is therefore deprecated and you should call "
                  "directly the savePMAT() method." );

    savePMAT(filename);
}

//////////////
// savePMAT //
//////////////
void VMatrix::savePMAT(const PPath& pmatfile, bool force_float, 
                       bool auto_float) const
{
    if (width() == -1)
        PLERROR("In VMat::save - Saving in a pmat file is only possible for constant width VMats (where width()!=-1)");

    if(force_float && auto_float)
        PLERROR("VMatrix::savePMAT() - force_float an auto_float are incompatible option");

    int nsamples = length();
    PPath pmatfiletmp=pmatfile+".tmp";
    if(auto_float){
#ifdef USEFLOAT
        PLERROR("VMatrix::savePMAT() - auto_float can't reliably select  float or double when compiled in float. Compile it in double.");
#endif
        Vec v(width());
        bool found_not_equal=false;
        for(int i=0;i<length();i++){
            getRow(i,v);
            for(int j=0;j<width();j++){
                if( ((double)((float)(v[j])))!=v[j] ){
                    found_not_equal=true;break;
                }
            }
        }
        if(!found_not_equal){
            force_float=true;
            pout<<"We will store the result matrix in FLOAT format."<<endl;
        }
        else
            pout<<"We will store the result matrix in DOUBLE format."<<endl;
    }
    {
    FileVMatrix m(pmatfiletmp,nsamples,width(),force_float);
    m.setMetaInfoFrom(this);
    // m.setFieldInfos(getFieldInfos());
    // m.copySizesFrom(this);
    Vec tmpvec(width());

    ProgressBar pb(cout, "Saving to pmat", nsamples);

    for(int i=0; i<nsamples; i++)
    { 
        getRow(i,tmpvec);
        m.putRow(i,tmpvec);
        pb(i);
    }
    m.saveFieldInfos();
    m.saveAllStringMappings();
    }// to ensure that m is deleted?
    
    rm(pmatfile);
    force_rmdir(pmatfile+".metadata");
    mv(pmatfiletmp,pmatfile);
    mv(pmatfiletmp+".metadata",pmatfile+".metadata");
}

//////////////
// saveDMAT //
//////////////
void VMatrix::saveDMAT(const PPath& dmatdir) const
{
    force_rmdir(dmatdir);
    DiskVMatrix vm(dmatdir,width());
    vm.setMetaInfoFrom(this);
    // vm.setFieldInfos(getFieldInfos());
    // vm.copySizesFrom(this);
    Vec v(width());

    ProgressBar pb(cout, "Saving to dmat", length());

    for(int i=0;i<length();i++)
    {
        getRow(i,v);
        vm.appendRow(v);
        pb(i);
    }
    vm.saveFieldInfos();
    vm.saveAllStringMappings();
}

//////////////
// saveAMAT //
//////////////
void VMatrix::saveAMAT(const PPath& amatfile, bool verbose, bool no_header, bool save_strings) const
{
    int l = length();
    int w = width();
    PStream out = openFile(amatfile, PStream::raw_ascii, "w");
    if (!no_header) {
        out << "#size: "<< l << ' ' << w << endl;
    }
    if(w>0 && !no_header)
    {
        out << "#: ";
        for(int k=0; k<w; k++)
            //there must not be any space in a field name...
            out << space_to_underscore(fieldName(k)) << ' ';
        out << "\n";
    }
    if(!no_header)
        out << "#sizes: " << inputsize() << ' ' << targetsize() << ' ' << weightsize() << ' ' << extrasize() << endl;

    PP<ProgressBar> pb;
    if (verbose)
        pb = new ProgressBar(cout, "Saving to amat", length());

    if (save_strings) {
        TVec<string> v(w);
        for (int i = 0; i < l; i++) {
            getRowAsStrings(i, v);
            out << v << endl;
            if (verbose)
                pb->update(i+1);
        }

    } else {
        Vec v(w);
        for(int i=0;i<l;i++)
        {
            getRow(i,v);
            for(int j=0; j<w; j++)
                out << v[j] << ' ';
            out << "\n";
            if (verbose)
                pb->update(i + 1);
        }
    }
}

void VMatrix::saveCMAT(const PPath& filename) const
{
    PLWARNING("VMatrix::saveCMAT() - NOT FULLY IMPLEMENTED");

    //calculate the datatype needed
    TVec<StatsCollector> stats = getStats(true);
    int max_bits=0;
    for(int i=0;i<stats.size();i++){
        StatsCollector stat = stats[i];
        if(! stat.isinteger())
            PLERROR("VMatrix::saveCMAT() currently the source need to contain only integer.");
        if(stat.min()>=0){
            int bits=(int)ceil(sqrt(stat.max()));
            if(max_bits<bits)max_bits=bits;
        }else{
            PLERROR("not implemented to store negatif number.");
        }
        
    }
    //example 12000000 u:784:1:8 u:1:1:8
    //write the header
     if(max_bits>8) PLERROR("VMatrix::saveCMAT() currently we convert to cmat with a maximum of 8 bits by fields!");
    if(max_bits > 1 && max_bits<8){
        max_bits=8;
        PLWARNING("VMatrix::saveCMAT() currently when we need less then 8 bits(except for 1), we upgrade to 8 bits.");
    }
    if(max_bits==0){
        PLERROR("VMatrix::saveCMAT() - their was only 0 in the matrix! This is not supported as we don't think this can happen in real case!");
    }
    //write the data
    if(max_bits==8){
        PStream out = openFile(filename, PStream::raw_ascii, "w");
        out<<length()<<" u:"<<width()<<":1:"<<max_bits<<endl;
        Vec v(width());
        for(int i=0;i<length();i++){
            getRow(i,v);
            for(int j=0;j<width();j++){
                out.put((char)v[j]);
            }
        }
    }else if(max_bits==1){
        PStream out = openFile(filename, PStream::raw_ascii, "w");
        int w2=width()%8;
        int w1=width()-w2;
        PLCHECK(w2+w1==width());
        PLCHECK(w1%8==0);
        PLCHECK(w1>0 && w2>=0);
        out<<length()<<" u:"<<w1<<":1:"<<max_bits;
        if(w2!=0)
            out<<" u:"<<w2<<":1:8";
        out<<endl;
        Vec v(width());

        for(int i=0;i<length();i++){
            getRow(i,v);
            int j;
            for(j=0;j<w1;){
                char c=0;
                for(int k=0;k<8;j++,k++){
                    c=c<<1;
                    c|=((bool)v[j]);
                }
                //revert the bits
                char value=c;
                value = (value & 0x0f) << 4 | (value & 0xf0) >> 4;
                value = (value & 0x33) << 2 | (value & 0xcc) >> 2;
                value = (value & 0x55) << 1 | (value & 0xaa) >> 1;
                out.put(value);
            }
            PLCHECK(width()-j==w2);
            for(;j<width();j++){
                out.put((char)v[j]);
            }
        }
    }
    else
        PLERROR("VMatrix::saveCMAT() - %d bits are not supported!",max_bits);

    CompactFileVMatrix m = CompactFileVMatrix(filename);
    m.setMetaDataDir(filename + ".metadata");
    m.setMetaInfoFrom(this);
    m.saveFieldInfos();
    m.saveAllStringMappings();

    pout<<"generated the file " <<filename <<endl;
}
///////////////////
// accumulateXtY //
///////////////////
void VMatrix::accumulateXtY(int X_startcol, int X_ncols, int Y_startcol, int Y_ncols,
                            Mat& result, int startrow, int nrows, int ignore_this_row) const
{
    int endrow = (nrows>0) ?startrow+nrows :length_;
    Vec x(X_ncols);
    Vec y(Y_ncols);
    for(int i=startrow; i<endrow; i++)
        if(i!=ignore_this_row)
        {
            getSubRow(i,X_startcol,x);
            getSubRow(i,Y_startcol,y);
            externalProductAcc(result, x,y);
        }
}

///////////////////
// accumulateXtX //
///////////////////
void VMatrix::accumulateXtX(int X_startcol, int X_ncols,
                            Mat& result, int startrow, int nrows, int ignore_this_row) const
{
    Vec x(X_ncols);
    int endrow = (nrows>0) ?startrow+nrows :length_;
    for(int i=startrow; i<endrow; i++)
        if(i!=ignore_this_row)
        {
            getSubRow(i,X_startcol,x);
            externalProductAcc(result, x,x);
        }
}

///////////////
// getRowVec //
///////////////
Vec VMatrix::getRowVec(int i) const
{
    Vec v(width());
    getRow(i,v);
    return v;
}

////////////////
// appendRows //
////////////////
void VMatrix::appendRows(Mat rows)
{
    for(int i=0; i<rows.length(); i++)
        appendRow(rows(i));
}


//////////////////
// compareStats //
//////////////////
void VMatrix::compareStats(VMat target,
                           real stderror_threshold,
                           real missing_threshold,
                           Vec stderror,
                           Vec missing)
{
    if(target->width()!=width())
        PLERROR("In VecStatsCollector:: compareStats() - This VMatrix has "
                "width %d which differs from the target width of %d",
                width(), target->width());

    for(int i=0;i<width();i++)
    {
        const StatsCollector tstats = target->getStats(i);
        const StatsCollector lstats = getStats(i);

        real tmissing = tstats.nmissing()/tstats.n();
        real lmissing = lstats.nmissing()/lstats.n();
        real terr = sqrt(tmissing*(1-tmissing)+lmissing*(1-lmissing));
        real th_missing = fabs(tmissing-lmissing)/terr;
        if(fast_is_equal(terr,0))
        {
            if(!fast_is_equal(tmissing,0)||!fast_is_equal(lmissing,0))
                PLWARNING("In VMatrix::compareStats - field %d(%s)terr=%f,"
                          " tmissing=%f, lmissing=%f!",i, fieldName(i).c_str(),
                          terr, tmissing, lmissing);
            PLCHECK((fast_is_equal(tmissing,0)||fast_is_equal(tmissing,1))
                    && (fast_is_equal(lmissing,0)||fast_is_equal(lmissing,1)));
        }
        else if(isnan(th_missing))
            PLWARNING("In VMatrix::compareStats - should not happen!");
        
        real tmean = tstats.mean();
        real lmean = lstats.mean();
        real tstderror = sqrt(pow(tstats.stderror(), 2) + 
                              pow(lstats.stderror(), 2));
        real th_stderror = fabs(lmean-tmean)/tstderror;
        if(tstderror==0)
            PLWARNING("In VMatrix::compareStats - field %d(%s) have a"
                      " stderror of 0 for both matrice.",
                      i, fieldName(i).c_str());
        stderror[i]=th_stderror;
        missing[i]=th_missing;
    }
    return;
}

///////////////////////
// maxFieldNamesSize //
///////////////////////
int VMatrix::maxFieldNamesSize() const
{
    uint size_fieldnames=0;
    for(int i=0;i<width();i++)
        if(fieldName(i).size()>size_fieldnames)
            size_fieldnames=fieldName(i).size();
    return size_fieldnames;
}

/////////////////////////////
// computeMissingSizeValue //
/////////////////////////////
void VMatrix::computeMissingSizeValue(bool warn_if_cannot_compute,
                                      bool warn_if_size_mismatch)
{
    int v=min(inputsize_,0) + min(targetsize_,0)
        + min(weightsize_,0) + min(extrasize_,0);

    if (width_ < 0 && v <= -1) {
        if (warn_if_cannot_compute)
            PLWARNING("In VMatrix::computeMissingSizeValue for %s - Cannot "
                      "compute the missing size value when the width is undefined",
                      classname().c_str());
        return;
    }

    if(v < -1){
        if(warn_if_cannot_compute)
            PLWARNING("In VMatrix::computeMissingSizeValue() - in class %s"
                      " more then one of"
                      " inputsize(%d), targetsize(%d), weightsize(%d) and"
                      " extrasize(%d) is unknow so we cannot compute them with"
                      " the width(%d)",
                      classname().c_str(), inputsize_, targetsize_, weightsize_,
                      extrasize_, width_);
        return;
    }else if(v==0 && warn_if_size_mismatch && width_ >= 0 &&
            width_ != inputsize_ + targetsize_ + weightsize_ + extrasize_)
        PLWARNING("In VMatrix::computeMissingSizeValue() for class %s - "
                  "inputsize_(%d) + targetsize_(%d) + weightsize_(%d) + "
                  "extrasize_(%d) != width_(%d) !",
                  classname().c_str(), inputsize_, targetsize_, weightsize_,
                  extrasize_, width_);

    else if(inputsize_<0)
        inputsize_ = width_- targetsize_ - weightsize_ - extrasize_;
    else if(targetsize_ < 0)
        targetsize_ = width_- inputsize_ - weightsize_ - extrasize_;
    else if(weightsize_ < 0)
        weightsize_ = width_- inputsize_ - targetsize_ - extrasize_;
    else if(extrasize_ < 0)
        extrasize_  = width_- inputsize_ - targetsize_ - weightsize_;
}

} // end of namespace PLearn


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
