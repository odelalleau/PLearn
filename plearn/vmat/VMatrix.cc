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

#include <sstream>
#include <iomanip>
#include "VMatrix.h"
#include "DiskVMatrix.h"
#include "FileVMatrix.h"
#include "SubVMatrix.h"
#include "VMat_computeStats.h"
#include <plearn/base/tostring.h>
#include <plearn/base/lexical_cast.h>
#include <plearn/base/stringutils.h> //!< For pgetline()
#include <plearn/io/fileutils.h>     //!< For isfile()
#include <plearn/io/load_and_save.h>
#include <plearn/math/random.h>      //!< For uniform_multinomial_sample()

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
    length_     (-1),
    width_      (-1),
    mtime_      (0),
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
    length_                         (the_length),
    width_                          (the_width),
    mtime_                          (0),
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
    declareOption(ol, "writable", &VMatrix::writable, OptionBase::buildoption, "Are write operations permitted?");
    declareOption(ol, "length", &VMatrix::length_, OptionBase::buildoption,
                  "Length of the matrix (number of rows)");
    declareOption(ol, "width", &VMatrix::width_, OptionBase::buildoption,
                  "Width of the matrix (number of columns; -1 indicates this varies from sample to sample...)");
    declareOption(ol, "inputsize", &VMatrix::inputsize_, OptionBase::buildoption,
                  "Size of input part (-1 if variable or unspecified, 0 if no input)");
    declareOption(ol, "targetsize", &VMatrix::targetsize_, OptionBase::buildoption,
                  "Size of target part (-1 if variable or unspecified, 0 if no target)");
    declareOption(ol, "weightsize", &VMatrix::weightsize_, OptionBase::buildoption,
                  "Size of weights (-1 if unspecified, 0 if no weight, 1 for sample weight, >1 currently not supported).");
    declareOption(ol, "extrasize", &VMatrix::extrasize_, OptionBase::buildoption,
                  "Size of extra fields (additional info). Defaults to 0");
    declareOption(ol, "metadatadir", &VMatrix::metadatadir, OptionBase::buildoption,
                  "A directory in which to store meta-information for this matrix \n"
                  "You don't always have to give this explicitly. For ex. if your \n"
                  "VMat is the outer VMatrix in a .vmat file, the metadatadir will \n"
                  "automatically be set to name_of_vmat_file.metadata/ \n"
                  "And if it is the source inside another VMatrix that sets its \n"
                  "metadatadir, it will often be set from that surrounding vmat's metadata.\n");
    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(get_row, copies);
    deepCopyField(dotrow_1, copies);
    deepCopyField(dotrow_2, copies);
    deepCopyField(field_stats, copies);
    deepCopyField(map_sr, copies);
    deepCopyField(map_rs, copies);
    deepCopyField(fieldinfos, copies);
    deepCopyField(fieldstats, copies);
    // TODO See if we can deep-copy a PStream (and what it means).
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
int VMatrix::getFieldIndex(const string& fieldname_or_num) const
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
    if (i < 0 || i >= width())
        PLERROR("In VMatrix::getFieldIndex - Asked for an invalid column number");
    return i;
}

////////////
// build_ //
////////////
void VMatrix::build_()
{
    if(!metadatadir.isEmpty())
        setMetaDataDir(metadatadir); // make sure we perform all necessary operations
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

    out << "nmissing: " << s.nmissing() << '\n';
    out << "nnonmissing: " << s.nnonmissing() << '\n';
    out << "sum: " << s.sum() << '\n';
    out << "mean: " << s.mean() << '\n';
    out << "stddev: " << s.stddev() << '\n';
    out << "min: " << s.min() << '\n';
    out << "max: " << s.max() << '\n';

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

void VMatrix::getExtra(int i, Vec& extra)
{
    if(inputsize_<0 || targetsize_<0 || weightsize_<0 || extrasize_<0)
        PLERROR("In VMatrix::getExtra, sizes not properly defined for this vmat");

    extra.resize(extrasize_);
    if(extrasize_>0)
        getSubRow(i,inputsize_+targetsize_+weightsize_, extra);
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
        Array<VMField> current_fieldinfos = getSavedFieldInfos();
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
    if ((! sizes_exist && inputsize_ != -1 && targetsize_ != -1 && weightsize_ != -1) ||
        (inp != inputsize_ || targ != targetsize_ || weight != weightsize_ || extr!=extrasize_))
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
    if (getSavedSizes(inp,tar,weight,extr) &&
        inputsize_ == -1 && targetsize_ == -1 && weightsize_ == -1)
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
        vector<string> v(split(in.getline()));
        switch(v.size())
        {
        case 1: current_fieldinfos[i] = VMField(v[0]); break;
        case 2: current_fieldinfos[i] = VMField(v[0], VMField::FieldType(toint(v[1]))); break;
        default: PLERROR("In VMatrix::getSavedFieldInfos Format not recognized in file %s.\n"
                         "Each line should be '<name> {<type>}'.", filename.absolute().c_str());
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
    PPath normalfname = getMetaDataDir() / "FieldInfo" / target;
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
    PPath  normalfname      = getMetaDataDir() / "FieldInfo" / target;
    string defaultlinkfname = getMetaDataDir() / "FieldInfo" / "__default.lnk";

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
    PPath normalfname = getMetaDataDir() / "FieldInfo" / target;
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
        val = - m.size() - 100;
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
    static string str;
    real val = get(row,col);
    str = getValString(col, val);
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
    setMtime(max(getMtime(),vm->getMtime()));

    // copy length and width from vm if not set
    if(length_<0)
        length_ = vm->length();
    if(width_<0)
        width_ = vm->width();

    // Copy sizes from vm if not set and they do not conflict with the width.
    if(inputsize_<0) {
        int is = vm->inputsize();
        if (is <= width_)
            inputsize_ = is;
    }
    if(targetsize_<0) {
        int ts = vm->targetsize();
        if (ts + (inputsize_ > 0 ? inputsize_ : 0) <= width_)
            targetsize_ = ts;
    }
    if(weightsize_<0) {
        int ws = vm->weightsize();
        if (ws + (inputsize_ > 0 ? inputsize_ : 0) + (targetsize_ > 0 ? targetsize_ : 0) <= width_)
            weightsize_ = ws;
    }
    if(extrasize_<=0) {
        int es = vm->extrasize();
        if (es + (inputsize_ > 0 ? inputsize_ : 0) + (targetsize_ > 0 ? targetsize_ : 0) + (weightsize_ > 0 ? weightsize_ : 0) <= width_)
            extrasize_ = es;
    }

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

/////////////
// getHost //
/////////////
string getHost()
{
    return "TODO";
}

////////////
// getPid //
////////////
int getPid()
{
    return -999;
}

/////////////
// getUser //
/////////////
string getUser()
{
    return "TODO";
}

/////////////////////
// lockMetaDataDir //
/////////////////////
void VMatrix::lockMetaDataDir(time_t max_lock_age, bool verbose) const
{
    if(!hasMetaDataDir())
        PLERROR("In VMatrix::lockMetaDataDir(): metadatadir was not set");
    if(lockf_) // Already locked by this object!
        PLERROR("VMatrix::lockMetaDataDir() called while already locked by this object.");
    if(!pathexists(metadatadir))
        force_mkdir(metadatadir);

    PPath lockfile = metadatadir / ".lock";
    while (isfile(lockfile) && (max_lock_age == 0 || mtime(lockfile) + max_lock_age > time(0))) {
        // There is a lock file, and it is not older than 'max_lock_age'.
        string bywho;
        try{ bywho = loadFileAsString(lockfile); }
        catch(...) {
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
    string lock_content = "host " + getHost() + ", pid " + tostring(getPid()) + ", user " + getUser();
    lockf_ << lock_content;
    lockf_.flush();
}

///////////////////////
// unlockMetaDataDir //
///////////////////////
void VMatrix::unlockMetaDataDir() const
{
    if(!lockf_)
        PLERROR("In VMatrix::unlockMetaDataDir() was called while no lock is held by this object");
    lockf_ = PStream();   // Release the lock.
    PPath lockfile = metadatadir / ".lock";
    rm(lockfile); // Remove the file.
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
    force_mkdir( getMetaDataDir() / "FieldInfo" );

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

    while(f)
    {
        string s;
        real val;
        f >> s >> val;
        if(f)
        {
            map_sr[col][s]   = val;
            map_rs[col][val] = s;
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
TVec<StatsCollector> VMatrix::getStats() const
{
    if(!field_stats)
    {
        PPath statsfile = getMetaDataDir() / "stats.psave";
        if (isfile(statsfile) && getMtime()<mtime(statsfile))
        {
            if(getMtime()==0)
                PLWARNING("Warning: using a saved stat file (%s) but mtime is 0.\n(cannot be sure file is up to date)",statsfile.absolute().c_str());
            PLearn::load(statsfile, field_stats);
        }
        else
        {
            VMat vm = const_cast<VMatrix*>(this);
            field_stats = PLearn::computeStats(vm, 2000);
            PLearn::save(statsfile, field_stats);
        }
    }
    return field_stats;
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
    PLERROR("In VMatrix::put - Method not implemented for this VMat, please implement.");
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
    PLERROR("This method (appendRow) not implemented by VMatrix subclass!");
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
void VMatrix::savePMAT(const PPath& pmatfile) const
{
    if (width() == -1)
        PLERROR("In VMat::save - Saving in a pmat file is only possible for constant width VMats (where width()!=-1)");

    int nsamples = length();

    FileVMatrix m(pmatfile,nsamples,width());
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

    ProgressBar* pb = 0;
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
    if (pb)
        delete pb;
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
