// -*- C++ -*-

// VVMatrix.cc
// Copyright (C) 2002 Pascal Vincent and Julien Keable
// Copyright (C) 2003 Olivier Delalleau
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

#include <plearn/base/Object.h>
#include <plearn/db/getDataSet.h>
#include <plearn/io/IntVecFile.h>
#include <plearn/math/random.h>
#include "ConcatColumnsVMatrix.h"
#include "ConcatRowsVMatrix.h"
#include "DiskVMatrix.h"
#include "FileVMatrix.h"
#include "JoinVMatrix.h"
#include "SelectRowsFileIndexVMatrix.h"
#include "VMatLanguage.h"
#include "VVMatrix.h"

#ifdef WIN32
#include <time.h>
#else
#include <unistd.h>
#endif

#define NEW_SYNTAX_CHAR '@'

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(VVMatrix,
                        "A VMat that reads a '.vmat' file.",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void VVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "filename", &VVMatrix::the_filename, OptionBase::buildoption, "Path to the .vmat file");
    inherited::declareOptions(ol);
}


// vmat genfilterindex source.pmat toto.pvm toto.indexes

void VVMatrix::generateFilterIndexFile(VMat source, const string & code, const string& ivfname)
{
    rm(ivfname);
    IntVecFile ivf(ivfname,true);
    VMatLanguage filt(source);
    vector<string> fn;
    for(int i=0;i<source.width();i++)
        fn.push_back(source->fieldName(i));
    filt.compileString(code,fn);
    Vec bla(1);
    Vec src(source.width());
    ProgressBar pb(cerr,"Filtering",source.length());
    for(int i=0;i<source.length();i++)
    {
        filt.run(i,bla);
        if(!fast_exact_is_equal(bla[0], 0))
            ivf.append(i);
        pb(i);
    }
    ivf.close();
}

VMat VVMatrix::buildFilteredVMatFromVPL(VMat source, const string & code, const string& ivfname, time_t date_of_code)
{
    if(getNonBlankLines(code).empty())
        return source;
    if(!isfile(ivfname) || mtime(ivfname) < date_of_code)
        generateFilterIndexFile(source, code, ivfname);
    IntVecFile ivf(ivfname,true);
    return new SelectRowsFileIndexVMatrix(source, ivfname);
}


vector<vector<string> > VVMatrix::extractSourceMatrix(const string & str,const string& filename)
{
    vector<vector<string> > mstr;
    if (str[0] == NEW_SYNTAX_CHAR) {
        // We are using the new syntax : we don't care about this for now,
        // too bad for the getDate method !
        return mstr;
    }
    vector<string> lines=getNonBlankLines(str);
    for(unsigned int i=0;i<lines.size();i++)
        mstr.push_back(split(lines[i],"|"));

    for(unsigned int i=0;i<mstr.size();i++)
        for(unsigned int j=0;j<mstr[i].size();j++)
        {
            string srcstr = removeblanks(mstr[i][j]);
            if(srcstr[0]!=slash_char)
            {
                string potential_dir = extract_directory(filename);
                size_t p = srcstr.find(":");
                string potential_path = potential_dir + srcstr.substr(0,p);
                if(pathexists(potential_path))
                    srcstr=potential_dir+srcstr;
            }
            mstr[i][j] = srcstr;
        }
    return mstr;
}

time_t VVMatrix::getDateOfVMat(const string& filename)
{
    string in=readFileAndMacroProcess(filename);
    size_t idx_source = in.find("<SOURCES>");
    size_t cidx_source;

    time_t latest = getDateOfCode(filename),tmp;

    if(idx_source!=string::npos)
    {
        idx_source += strlen("<SOURCES>");     // skip beyond
        cidx_source=in.find("</SOURCES>");
        if(cidx_source==string::npos)
            PLERROR("Cannot find closing tag </SOURCES>. File is %s",filename.c_str());
        string sec=in.substr(idx_source,cidx_source-idx_source);
        vector<vector<string> > mstr = extractSourceMatrix(sec,filename);
        // TODO Make it work with the new syntax
        for(unsigned int i=0;i<mstr.size();i++)
            for(unsigned int j=0;j<mstr[i].size();j++)
            {
                vector<string> vecstr;
                vecstr=split(mstr[i][j],":");
                if(vecstr.size()==2 || vecstr.size()==4)
                {
                    // also check for date of possible IntVecFile
                    if(vecstr[1][0]!=slash_char)
                        vecstr[1]=extract_directory(filename)+vecstr[1];
                    if((tmp=mtime(vecstr[1])) > latest)
                        latest=tmp;
                }
                if((tmp=getDataSetDate(vecstr[0])) > latest)
                    latest=tmp;
            }
    }
    return latest;
}

// returns the result from the join operation
void VVMatrix::processJoinSection(const vector<string> & code, VMat & tmpsource)
{
    unsigned int i=0;
    while(i<code.size())
    {
        while(isBlank(code[i]))i++;
        VMat slave=getDataSet(code[i++]);
        vector<string> mess=split(code[i++],"[]");
        if(mess.size()!=3)
            PLERROR("In JOIN section, field correspondance syntax is : [master1,master2] -> [slave1,slave2]");
        vector<string> mfields=split(mess[0],",");
        vector<string> sfields=split(mess[2],",");
        TVec<int> midx(int(mfields.size())), sidx(int(sfields.size()));
        for(unsigned int j=0;j<mfields.size();j++)
            midx[j]=tmpsource->fieldIndex(mfields[j]);
        for(unsigned int j=0;j<sfields.size();j++)
            sidx[j]=slave->fieldIndex(sfields[j]);

        JoinVMatrix * jvm= new JoinVMatrix(tmpsource,slave,midx,sidx);

        // browse field declarations of the <JOIN> section to declare new fields in the resulting JoinVMatrix

        while(i<code.size() && code[i].find(":")!=string::npos)
        {
            vector<string> crunch=split(code[i],":");
            if(crunch.size()!=2)PLERROR("In JOIN section : field declaration section syntax incorrect. EG: MEAN(master1) :newfieldname");
            vector<string> st=split(removeblanks(crunch[0]),"()");
            if(st.size()!=2)PLERROR("In JOIN section : field declaration section syntax incorrect. EG: MEAN(master1) :newfieldname");
            jvm->addStatField(removeblanks(st[0]),removeblanks(st[1]),removeblanks(crunch[1]));
            i++;
        }
        while(i<code.size() && isBlank(code[i]))i++;
        tmpsource=jvm;
    }
}

// generate a file (ivfname) containing indexes of rows of 'source' that remain after filtering with
// the *every* possible step that changes the index of rows (i.e : prefilter, shuffle.. postfiltering)
// -- Not optimal, since it will first *precompute* if any postfilter is required
void VVMatrix::generateVMatIndex(VMat source, const string& meta_data_dir,
                                 const string & filename, time_t date_of_code,const string & in,
                                 size_t idx_prefilter, size_t cidx_prefilter,
                                 size_t idx_postfilter, size_t cidx_postfilter,
                                 size_t idx_process, size_t cidx_process,
                                 size_t idx_shuffle, size_t cidx_shuffle,
                                 size_t idx_join, size_t cidx_join)
{
    VMat tmpsource = source;
    rm(meta_data_dir+slash+"source.indexes");

    if(idx_prefilter!=string::npos)
    {
        idx_prefilter+=11;
        cidx_prefilter=in.find("</PREFILTER>");
        if(cidx_prefilter==string::npos)
            PLERROR("Cannot find closing tag </PREFILTER>. File is %s",filename.c_str());
        string code=in.substr(idx_prefilter,cidx_prefilter-idx_prefilter);
        cout<<"Prefiltering.. "<<endl;
        tmpsource = buildFilteredVMatFromVPL(tmpsource,code,meta_data_dir+slash+"incomplete.source.prefilter.indexes",date_of_code);
    }

    if(idx_join!=string::npos)
    {
        cidx_join=in.find("</JOIN>");
        if(cidx_join==string::npos)
            PLERROR("Cannot find closing tag </JOIN>. File is %s",filename.c_str());
        vector<string> code=split(in.substr(idx_join+6,cidx_join-idx_join-6),"\n");
        processJoinSection(code,tmpsource);
    }

    if(idx_process!=string::npos)
    {
        cidx_process=in.find("</PROCESSING>");
        if(cidx_process==string::npos)
            PLERROR("Cannot find closing tag </PROCESSING>. File is %s",filename.c_str());
        string code=in.substr(idx_process+12,cidx_process-idx_process-12);
        tmpsource = new PreprocessingVMatrix(tmpsource,code);
    }

    if(idx_postfilter!=string::npos)
    {
        idx_postfilter+=12;
        cidx_postfilter=in.find("</POSTFILTER>");
        if(cidx_postfilter==string::npos)
            PLERROR("Cannot find closing tag </POSTFILTER>. File is %s",filename.c_str());
        string code=in.substr(idx_postfilter,cidx_postfilter-idx_postfilter);
        cout<<"Postfiltering.. "<<endl;
        tmpsource = buildFilteredVMatFromVPL(tmpsource,code,meta_data_dir+slash+"incomplete.source.postfilter.indexes",date_of_code);
    }

    // combines pre and postfilter index file in a single one
    if(isfile(meta_data_dir+slash+"incomplete.source.prefilter.indexes"))
        if(isfile(meta_data_dir+slash+"incomplete.source.postfilter.indexes"))
        {
            IntVecFile ivf(meta_data_dir+slash+"source.indexes",true);
            IntVecFile preivf(meta_data_dir+slash+"incomplete.source.prefilter.indexes");
            IntVecFile postivf(meta_data_dir+slash+"incomplete.source.postfilter.indexes");
            for(int i=0;i<postivf.length();i++)
                ivf.append(preivf[postivf[i]]);
        }
        else
            mv(meta_data_dir+slash+"incomplete.source.prefilter.indexes",
               meta_data_dir+slash+"source.indexes");

    else
        if(isfile(meta_data_dir+slash+"incomplete.source.postfilter.indexes"))
            mv(meta_data_dir+slash+"incomplete.source.postfilter.indexes",
               meta_data_dir+slash+"source.indexes");

    if(idx_shuffle!=string::npos)
    {
        idx_shuffle+=9;
        cidx_shuffle=in.find("</SHUFFLE>");
        if(cidx_shuffle==string::npos)
            PLERROR("Cannot find closing tag </SHUFFLE>. File is %s",filename.c_str());
        string seedstr=removeblanks(in.substr(idx_shuffle,cidx_shuffle-idx_shuffle));
        if(seedstr=="")
            manual_seed(clock());
        else
        {
            int seed = toint(seedstr);
            manual_seed(seed);
        }
        // if a source.indexes file exists, shuffle it, otherwise, create it
        TVec<int> idx;
        if(isfile(meta_data_dir+slash+"source.indexes"))
        {
            IntVecFile ivf(meta_data_dir+slash+"source.indexes");
            idx=ivf.getVec();
        }
        else idx=TVec<int>(0,tmpsource->length()-1,1);
        shuffleElements(idx);
        rm(meta_data_dir+slash+"source.indexes");
        IntVecFile newivf(meta_data_dir+slash+"source.indexes",true);
        newivf.append(idx);
    }
    // remove intermediate files
    rm(meta_data_dir+slash+"incomplete.source.prefilter.indexes");
    rm(meta_data_dir+slash+"incomplete.source.postfilter.indexes");
}

bool VVMatrix::isPrecomputedAndUpToDate()
{
    string meta_data_dir=remove_trailing_slash(the_filename)+".metadata";
    if(isfile(meta_data_dir+slash+"precomputed.pmat") &&
       mtime(meta_data_dir+slash+"precomputed.pmat") > getMtime())
        return true;
    if(pathexists(meta_data_dir+slash+"precomputed.dmat"+slash) &&
       mtime(meta_data_dir+slash+"precomputed.dmat"+slash+"indexfile") > getMtime())
        return true;
    return false;
}

string VVMatrix::getPrecomputedDataName()
{
    string meta_data_dir=remove_trailing_slash(the_filename)+".metadata";
    if(isfile(meta_data_dir+slash+"precomputed.pmat"))
        return meta_data_dir+slash+"precomputed.pmat";
    if(pathexists(meta_data_dir+slash+"precomputed.dmat"+slash))
        return meta_data_dir+slash+"precomputed.dmat";
    return "** no precomputed data found **";
}


VMat VVMatrix::createPreproVMat(const string & filename)
{
    string in=readFileAndMacroProcess(filename);
    size_t idx_source     = in.find("<SOURCES>");
    size_t idx_prefilter  = in.find("<PREFILTER>");
    size_t idx_postfilter = in.find("<POSTFILTER>");
    size_t idx_process    = in.find("<PROCESSING>");
    size_t idx_shuffle    = in.find("<SHUFFLE>");
    size_t idx_join       = in.find("<JOIN>");
    size_t idx_precompute = in.find("<PRECOMPUTE>");
    size_t idx_sizes      = in.find("<SIZES>");
    size_t cidx_source    = 0, cidx_prefilter = 0,  cidx_postfilter = 0,
        cidx_process = 0, cidx_shuffle   = 0,  cidx_precompute = 0,
        cidx_join    = 0, cidx_sizes     = 0;
    string precomp;
    VMat source;

    bool olddebugval=VMatLanguage::output_preproc;
    VMatLanguage::output_preproc = in.find("<DEBUG>")!=string::npos;

    if( VMatLanguage::output_preproc )
        cerr<<"DEBUG is on (remove <DEBUG> in "+filename+" to turn it off)"<<endl;

    PPath meta_data_dir = remove_trailing_slash(filename)+".metadata";
    force_mkdir(meta_data_dir);
    time_t date_of_code = getDateOfVMat(filename);

    // remove pollution : all stuff that has possibly been interrupted during computation
    rm ( meta_data_dir / "incomplete.*" );

    bool sizes_spec = false;
    int inputsize = -1;
    int targetsize = -1;
    int weightsize = -1;
    // Check if sizes are specified.
    if(idx_sizes != string::npos)
    {
        sizes_spec = true;
        idx_sizes += 7;
        cidx_sizes = in.find("</SIZES>");
        if(cidx_sizes == string::npos)
            PLERROR("Cannot find closing tag </SIZES>. File is %s",filename.c_str());
        vector<string> sizes = split(in.substr(idx_sizes, cidx_sizes - idx_sizes),"\n");
        if (sizes.size() != 3) {
            PLERROR("You should specify exactly 3 sizes between <SIZES> and </SIZES> (file is %s)", filename.c_str());
        }
        inputsize = toint(sizes[0]);
        targetsize = toint(sizes[1]);
        weightsize = toint(sizes[2]);
    }

    if(isfile(meta_data_dir / "precomputed.pmat"))
    {
        // Precomputed version exist in pmat format.
        // If it seems old, we display a warning (one may still want to use it
        // if he knows the changes made to the vmat code do not alter the data).
        if(mtime(meta_data_dir / "precomputed.pmat") < date_of_code) {
            PLWARNING("In VVMatrix::createPreproVMat - The precomputed data (in %s) is older than current code, you may want to recompute again", meta_data_dir.c_str());
        }
        source = new FileVMatrix(meta_data_dir / "precomputed.pmat");
        source->setMetaDataDir(meta_data_dir);
        source->setMtime(date_of_code);
        source->defineSizes(inputsize, targetsize, weightsize);
        return source;
    }

    if( pathexists(meta_data_dir / "precomputed.dmat") )
    {
        // precomputed version exist in DiskVMatrix format,
        // so we use it *if it is up to date*
        if ( mtime(meta_data_dir/"precomputed.dmat/indexfile") > date_of_code )
        {
            source = new DiskVMatrix(meta_data_dir/"precomputed.dmat");
            source->setMetaDataDir(meta_data_dir);
            source->setMtime(date_of_code);
            source->defineSizes(inputsize, targetsize, weightsize);
            return source;
        }
    }

    // if index_section is true, then this dataset needs a file containing the index of the rows to keep
    bool index_section = idx_prefilter!=string::npos || idx_postfilter!=string::npos || idx_shuffle!=string::npos ;

    // if true, index file lacks or is out of date
    bool must_regen_index = index_section &&
        (!isfile(meta_data_dir/"source.indexes") || date_of_code > mtime(meta_data_dir/"source.indexes"));

    // erase obsolete source.index if necessary
    if(isfile(meta_data_dir/"source.indexes") && !index_section)
        rm (meta_data_dir/"source.indexes");

    if(idx_source!=string::npos)
    {
        // go over tag
        idx_source+=9;
        cidx_source=in.find("</SOURCES>");
        if(cidx_source==string::npos)
            PLERROR("Cannot find closing tag </SOURCES>. File is %s",filename.c_str());
        // 'sec' is the text content of the <SOURCES> section
        string sec=in.substr(idx_source,cidx_source-idx_source);

        if (sec[1] == NEW_SYNTAX_CHAR) {
            // We are using the new syntax
            sec[1] = ' ';  // remove the special character indicating the new syntax
            source = dynamic_cast<VMatrix*>(newObject(sec));
            if(source.isNull()) {
                PLERROR("In VVMatrix::createPreproVMat '%s' is not a valid VMatrix subclass",sec.c_str());
            }
        } else {

            vector<vector<string> > mstr = extractSourceMatrix(sec,filename);
            Array<VMat> vmrows(int(mstr.size()));
            // we need to build a VMat that is the concatenation of the datasets contained in 'mstr'

            for(unsigned int i=0;i<mstr.size();i++)
            {
                Array<VMat> ar( (int)mstr[i].size() );
                for(unsigned int j=0;j<mstr[i].size();j++)
                {
                    vector<string> vecstr;
                    vecstr=split(mstr[i][j],":");
                    ar[j]=getDataSet(vecstr[0]);

                    // handle different cases of dataset specification
                    // legal formats are:
                    // 1- simple dataset filename
                    // 2- intVecFile specification : filename:intVecFile_Filename
                    // 3- range specifiaction : filename:start:length
                    // 4- range specifiaction + intVecFile: filename:intVecFile_Filename:start:length
                    switch(vecstr.size())
                    {
                    case 1: // only dataset name so we do nothing
                        break;
                    case 2: // we have an intVecFile specification
                        // prefix with the path to the current vmat
                        if(vecstr[1][0]!=slash_char)
                            vecstr[1]=extract_directory(filename)+vecstr[1];
                        ar[j]=new SelectRowsFileIndexVMatrix(ar[j],vecstr[1]);
                        break;
                    case 3: // submatRows range specification
                        ar[j]=ar[j].subMatRows(toint(vecstr[1]),toint(vecstr[2]));
                        break;
                    case 4: // intVecFile + submatRows
                        if(vecstr[1][0]!=slash_char)
                            vecstr[1]=extract_directory(filename)+vecstr[1];
                        ar[j]=new SelectRowsFileIndexVMatrix(ar[j],vecstr[1]);
                        ar[j]=ar[j].subMatRows(toint(vecstr[2]),toint(vecstr[3]));
                        break;
                    default:
                        PLERROR("Strange number of semicolumns... Format of source must be something.vmat[:indexfile.index][:start_row:length]. File is %s",filename.c_str());
                        break;
                    }
                }
                // if we have more than one filename in this row, we use hconcat to consolidate 'ar'.
                vmrows[i] = ar.size()==1?ar[0]:hconcat(ar);
                if(vmrows[i].length()==-1)
                    PLERROR("Trying to hconcat matrix with different lengths! File is %s",filename.c_str());
            }

            source = vconcat(vmrows);
            if(mstr.size()==0)
                PLERROR("No source matrix found in <SOURCES> section! File is %s",filename.c_str());
        }
    }
    else PLERROR("Need at least a <SOURCES> section ! File is %s",filename.c_str());

    if(must_regen_index)
        generateVMatIndex(source, meta_data_dir, filename, date_of_code, in, idx_prefilter, cidx_prefilter,
                          idx_postfilter, cidx_postfilter, idx_process, cidx_process,
                          idx_shuffle, cidx_shuffle, idx_join, cidx_join);

    if(idx_join!=string::npos)
    {
        cidx_join=in.find("</JOIN>");
        if(cidx_join==string::npos)
            PLERROR("Cannot find closing tag </JOIN>. File is %s",filename.c_str());
        vector<string> code=split(in.substr(idx_join+6,cidx_join-idx_join-6),"\n");
        processJoinSection(code,source);
    }

    if(idx_process!=string::npos)
    {
        cidx_process=in.find("</PROCESSING>");
        if(cidx_process==string::npos)
            PLERROR("Cannot find closing tag </PROCESSING>. File is %s",filename.c_str());
        string code=in.substr(idx_process+12,cidx_process-idx_process-12);
        source = new PreprocessingVMatrix(source,code);
    }

    // if source.index exists at this point, we need to apply it
    if(isfile(meta_data_dir/"source.indexes"))
        source = new SelectRowsFileIndexVMatrix(source,meta_data_dir/"source.indexes");

    // next lines handle precomputation of matrix
    if(idx_precompute!=string::npos)
    {
        idx_precompute+=12;
        cidx_precompute=in.find("</PRECOMPUTE>");
        if(cidx_precompute==string::npos)
            PLERROR("Cannot find closing tag </PRECOMPUTE>. File is %s",filename.c_str());
        precomp=removeblanks(in.substr(idx_precompute,cidx_precompute-idx_precompute));
        if(precomp ==  "dmat")
        {
            cout << "Rendering DMAT : " << meta_data_dir/"precomputed.dmat" << endl;
            source->saveDMAT(meta_data_dir/"incomplete.precomputed.dmat");
            int cnt=0;
            if (isdir(meta_data_dir/"precomputed.dmat")) {
                while(cnt++ < 5 &&
                      !force_rmdir(meta_data_dir/"precomputed.dmat"))
                {
                    cerr<<"Could not rm -rf '"+meta_data_dir+
                        slash+"precomputed.dmat/'. Maybe 'Stale NFS file handle' curse again. Retrying in 2 sec."
                        <<endl;
                    sleep(2);
                }
            }

            mvforce(meta_data_dir/"incomplete.precomputed.dmat",
                    meta_data_dir/"precomputed.dmat");
            if(pathexists(meta_data_dir/"incomplete.precomputed.dmat.metadata"))
            {
                rm(meta_data_dir/"precomputed.dmat.metadata");
                mvforce(meta_data_dir/"incomplete.precomputed.dmat.metadata",
                        meta_data_dir/"precomputed.dmat.metadata");
            }
            source=new DiskVMatrix(meta_data_dir/"precomputed.dmat");
        }
        else if(precomp ==  "pmat")
        {
            cout<<"Rendering PMAT : "<<meta_data_dir+slash+"precomputed.pmat"<<endl;
            source->savePMAT(meta_data_dir+slash+"incomplete.precomputed.pmat");
            mvforce(meta_data_dir/"incomplete.precomputed.pmat",
                    meta_data_dir/"precomputed.pmat");
            if(pathexists(meta_data_dir/"incomplete.precomputed.pmat.metadata"))
                mvforce(meta_data_dir/"incomplete.precomputed.pmat.metadata",
                        meta_data_dir/"precomputed.pmat.metadata");

            // Save the string mappings.
            source->setMetaDataDir(meta_data_dir);
            source->saveAllStringMappings();
            source = new FileVMatrix(meta_data_dir/"precomputed.pmat");
            source->setMetaDataDir(meta_data_dir);
            source->setMtime(date_of_code);
            source->defineSizes(inputsize, targetsize, weightsize);
            // TODO The 3 lines above are duplicated throughout the code, this is ugly.
        }
        else if(precomp!="")
            PLERROR("Unsupported precomputing format : %s.  File is %s", precomp.c_str(),filename.c_str());
    }

    VMatLanguage::output_preproc=olddebugval;
    source->setMetaDataDir(meta_data_dir);
    source->setMtime(date_of_code);
    if (sizes_spec) {
        source->defineSizes(inputsize, targetsize, weightsize);
    }
    return source;
}

void VVMatrix::build()
{
    inherited::build();
    build_();
}

void VVMatrix::build_()
{
    if (the_filename != "") {
        setMetaDataDir( PPath(the_filename+".metadata").absolute() );
        force_mkdir( getMetaDataDir() );

        code = readFileAndMacroProcess(the_filename);
        if(removeblanks(code)[0]=='<') // old xml-like format
            the_mat = createPreproVMat(the_filename);
        else  // New standard PLearn object description format
        {
            the_mat = dynamic_cast<VMatrix*>(newObject(code));
            if(the_mat.isNull())
                PLERROR("Object described in %s is not a VMatrix subclass",the_filename.c_str());
            the_mat->setMetaDataDir(getMetaDataDir());
        }

        setMtime(the_mat->getMtime());
        length_ = the_mat.length();
        width_ = the_mat.width();

        // Copy the sizes.
        copySizesFrom(the_mat);

        // Copy the string mappings.
        copyStringMappingsFrom(the_mat);

        // Copy the parent field names
        fieldinfos.resize(width_);
        if (the_mat->getFieldInfos().size() > 0)
            for(int j=0; j<width_; j++)
                fieldinfos[j] = the_mat->getFieldInfos()[j];
    }
}

// string maps are those loaded from the .vmat metadatadir, not those of the source vmatrix anymore
// could be changed..

// real VVMatrix::getStringVal(int col, const string & str) const
// {
//   return the_mat->getStringVal(col, str);
// }
// string VVMatrix::getValString(int col, real val) const
// {
//   return the_mat->getValString(col, val);
// }
// string VVMatrix::getString(int row,int col) const
// {
//   return the_mat->getString(row, col);
// }

// const map<string,real>& VVMatrix::getStringToRealMapping(int col) const
// {
//   return the_mat->getStringToRealMapping(col);
// }


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(the_mat, copies);
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
