
// -*- C++ -*-

// TextFilesVMatrix.h
//
// Copyright (C) 2003-2004 ApSTAT Technologies Inc.
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

/* *******************************************************
 * $Id$
 ******************************************************* */

// Author: Pascal Vincent, Christian Hudon

/*! \file TextFilesVMatrix.cc */
#include "TextFilesVMatrix.h"
#include <plearn/base/PDate.h>
#include <plearn/base/ProgressBar.h>
#include <plearn/base/stringutils.h>
#include <plearn/io/load_and_save.h>
#include <plearn/io/fileutils.h>
#define PL_LOG_MODULE_NAME "TextFilesVMatrix"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;


TextFilesVMatrix::TextFilesVMatrix():
    idxfile(0),
    delimiter("\t"),
    quote_delimiter(""),
    auto_build_map(false),
    auto_extend_map(true),
    build_vmatrix_stringmap(false),
    reorder_fieldspec_from_headers(false),
    partial_match(false)
{}

PLEARN_IMPLEMENT_OBJECT(
    TextFilesVMatrix,
    "Parse and represent a text file as a VMatrix",
    "This VMatrix contains a plethora of options for parsing text files,\n"
    "interpreting the fields (including arbitrary string fields), and\n"
    "representing the result as a numerical VMatrix.  It can be used to parse\n"
    "both SAS and CSV files.\n"
    "\n"
    "The metadatadir option should probably be specified.\n"
    "\n"
    "Internally, the metadata directory contains the following files:\n"
    " - a txtmat.idx binary index file (which will be automatically rebuilt if any of the raw text files is newer)\n"
    " - a txtmat.idx.log file reporting problems encountered while building the .idx file\n"
    "\n"
    "The txtmat.idx file is a binary file structured as follows\n"
    "- 1 byte indicating endianness: 'L' or 'B'\n"
    "- 4 byte int for length (number of data rows in the raw text file)\n"
    "- (unsigned char fileno, int pos) indicating in which raw text file and at what position each row starts\n"
    );


void TextFilesVMatrix::getFileAndPos(int i, unsigned char& fileno, int& pos) const
{
    PLASSERT(idxfile!=0);
    if(i<0 || i>=length())
        PLERROR("TextFilesVMatrix::getFileAndPos out of range row %d (only %d rows)", i, length());
    fseek(idxfile, 5+i*5, SEEK_SET);
    fileno = fgetc(idxfile);
    fread(&pos, sizeof(int), 1, idxfile);
}

int TextFilesVMatrix::getIndexOfTextField(const string& fieldname) const
{
    int n = fieldspec.size();
    for(int i=0; i<n; i++)
        if(fieldspec[i].first==fieldname)
            return i;
    PLERROR("In TextFilesVMatrix::getIndexOfTextField unknown field %s",fieldname.c_str());
    return -1; // to make the compiler happy
}

void TextFilesVMatrix::buildIdx()
{
    perr << "Building the index file. Please be patient..." << endl;

    if(idxfile)
        fclose(idxfile);
    PPath ft=getMetaDataDir()/"txtmat.idx.tmp";
    PPath f=getMetaDataDir()/"txtmat.idx";
    idxfile = fopen(ft.c_str(),"wb");
    FILE* logfile = fopen((getMetaDataDir()/"txtmat.idx.log").c_str(),"a");

    if (! idxfile)
        PLERROR("TextFilesVMatrix::buildIdx: could not open index file '%s'",
                ( getMetaDataDir()/"txtmat.idx").c_str());
    if (! logfile)
        PLERROR("TextFilesVMatrix::buildIdx: could not open log file '%s'",
                (getMetaDataDir()/"txtmat.idx.log").c_str());

    // write endianness
    fputc(byte_order(), idxfile);
    // We don't know length yet,
    length_ = 0;
    fwrite(&length_, 4, 1, idxfile);

    TVec<string> fields;
    char buf[50000];

    int lineno = 0;
    for(unsigned char fileno=0; fileno<txtfiles.length(); fileno++)
    {
        FILE* fi = txtfiles[(int)fileno];
        fseek(fi,0,SEEK_SET);

        int nskip = 0; // number of header lines to skip
        if(!skipheader.isEmpty())
            nskip = skipheader[int(fileno)];

        // read the data rows and build the index
        for(;;)
        {
            long pos_long = ftell(fi);
            if (pos_long > INT_MAX)
                PLERROR("In TextFilesVMatrix::buildIdx - 'pos_long' cannot be "
                        "more than %d", INT_MAX);
            int pos = int(pos_long);
            if(!fgets(buf, sizeof(buf), fi))
                break;

#ifdef CYGWIN_FGETS_BUGFIX
            // Bugfix for CYGWIN carriage return bug.
            // Should be safe to enable in all case, but need to be tester more widely.
            long new_pos = ftell(fi);
            long lbuf = long(strlen(buf));
            if (lbuf+pos != new_pos){
                if(lbuf+1+pos==new_pos && buf[lbuf-1]=='\n' && buf[lbuf-2]!='\r')
                {
                    //bug under windows. fgets return the good string if unix end of lines, but
                    //change the position suppossing the use of \r\n as carrige return.
                    //So if their is only a \n, we are a caractere too far.
                    //if dos end of lines, return \n as end of lines in the strings and put the pos correctly.
                    
                    fseek(fi,-1,SEEK_CUR);
                    
		    //if unix end of lines
                    if(fgetc(fi)!='\n')
                    	fseek(fi,-1,SEEK_CUR);
                }
                //in the eof case?
                else if(lbuf-1+pos==new_pos && buf[lbuf-1]=='\n' && buf[lbuf-2]!='\r')
                    fseek(fi,+1,SEEK_CUR);
                else
                    PLERROR("In TextFilesVMatrix::buildId - The number of characters read "
                            "does not match the position in the file.");
            }
#endif

            buf[sizeof(buf)-1] = '\0';         // ensure null-terminated
            lineno++;
            if(nskip>0)
                --nskip;
            else if(!isBlank(buf))
            {
                fields = splitIntoFields(buf);
                int nf = fields.length();
                if(nf!=fieldspec.size()){
                    fprintf(logfile, "ERROR In file %d line %d: Found %d fields (should be %d):\n %s",fileno,lineno,nf,fieldspec.size(),buf);
                    PLWARNING("In file %d line %d: Found %d fields (should be %d):\n %s",fileno,lineno,nf,fieldspec.size(),buf);
                }
                else  // Row OK! append it to index
                {
                    fputc(fileno, idxfile);
                    fwrite(&pos, 4, 1, idxfile);
                    length_++;
                }
            }
            else
                PLWARNING("In TextFilesVMatrix::buildIdx() - The line %d is blank",lineno);
        } // end of loop over lines of file
    } // end of loop over files

    // Write true length and width
    fseek(idxfile, 1, SEEK_SET);
    fwrite(&length_, 4, 1, idxfile);

    // close files
    fclose(logfile);
    fclose(idxfile);
    mvforce(ft,f);
    perr << "Index file built." << endl;
}

/////////////////////////////
// isValidNonSkipFieldType //
/////////////////////////////
bool TextFilesVMatrix::isValidNonSkipFieldType(const string& ftype) const {
    return (ftype=="auto" || ftype=="num" || ftype=="date" || ftype=="jdate" ||
            ftype=="postal" || ftype=="dollar" || ftype=="dollar-comma" ||
            ftype=="YYYYMM" || ftype=="sas_date" || ftype == "bell_range" ||
            ftype == "char" || ftype=="num-comma" || ftype=="auto-num");
}

void TextFilesVMatrix::setColumnNamesAndWidth()
{
    width_ = 0;
    TVec<string> fnames;
    TVec<string> fnames_header;//field names take in the header of source file
    char buf[50000];


    //select witch delimiter we will use for all the files.
    if(delimiter.size()>1){
        FILE* f = txtfiles[0];
        fseek(f,0,SEEK_SET);
        if(!fgets(buf, sizeof(buf), f))
            PLERROR("In TextFilesVMatrix::setColumnNamesAndWidth() - "
                    "Couldn't read the fields names from file '%s'",
                    txtfilenames[0].c_str());

        string s1 = string(buf);
        if(!fgets(buf, sizeof(buf), f))
            PLERROR("In TextFilesVMatrix::setColumnNamesAndWidth() - "
                    "Couldn't read the fields names from file '%s'",
                    txtfilenames[0].c_str());
        string s2 = string(buf);
        TVec<int> nbs1(delimiter.size());
        TVec<int> nbs2(delimiter.size());
        
        string old_delimiter = delimiter;
        for(uint i=0;i<old_delimiter.size();i++){
            delimiter = old_delimiter[i];
            TVec<string> fields1 = splitIntoFields(s1);
            TVec<string> fields2 = splitIntoFields(s2);
            nbs1[i]=fields1.size();
            nbs2[i]=fields2.size();
        }
        delimiter=old_delimiter;
        for(uint i=0;i<old_delimiter.size();i++){
            if(nbs1[i]==nbs2[i]&& nbs1[i]>0){
                delimiter = old_delimiter[i];
            }
        }
        MODULE_LOG << "Selected delimiter: <" << delimiter << ">" << endl;
        if(delimiter.size()!=1){
            PLERROR("In TextFilesVMatrix::setColumnNamesAndWidth() - We can't"
                    " automatically determine the delimiter to use as the two"
                    " first row don't have a common delimiter with the same"
                    " number of occurence. nbs1=%s, nbs2=%s",
                    tostring(nbs1).c_str(),tostring(nbs2).c_str());
        }
    }
    PLCHECK(delimiter.size()==1);

    //read the fieldnames from the files.
    for(int i=0; i<txtfiles.size(); i++){
        FILE* f = txtfiles[i];
        fseek(f,0,SEEK_SET);
        if(!fgets(buf, sizeof(buf), f))
            PLERROR("In TextFilesVMatrix::setColumnNamesAndWidth() - "
                    "Couldn't read the fields names from file '%s'",
                    txtfilenames[i].c_str());
        fseek(f,0,SEEK_SET);

        TVec<string> fields = splitIntoFields(buf);

        //check that we have the good delimiter
        if(fields.size()==1 && fieldspec.size()>1)
            PLERROR("In TextFilesVMatrix::setColumnNamesAndWidth() -"
                    " We found only 1 column in the first line, but"
                    " their is %d fieldspec. Meaby the delimiter '%s'"
                    " is not the right one. The line is %s",
                    fieldspec.size(),delimiter.c_str(),
                    string(buf).c_str());
        
        if(reorder_fieldspec_from_headers || partial_match){
            fields.append(removeblanks(fields.pop()));
            
            fnames_header.append(fields);
        }
    }
    if(partial_match)
    {
        TVec< pair<string, string> > new_fieldspec;
        TVec<string> no_expended_fields;
        PLCHECK_MSG(reorder_fieldspec_from_headers,
                    "In TextFilesVMatrix::setColumnNamesAndWidth - "
                    "when partial_match is true, reorder_fieldspec_from_headers"
                    " must be true.");
        for(int i=0;i<fieldspec.size();i++)
        {
            bool expended = false;
            string fname=fieldspec[i].first;
            if(fname[fname.size()-1]!='*')
            {
                new_fieldspec.append(fieldspec[i]);
                continue;
            }
            fname.resize(fname.size()-1);//remove the last caracter (*)
            for(int j=0;j<fnames_header.size();j++)
            {
                if(string_begins_with(fnames_header[j],fname))
                {
                    pair<string,string> n=make_pair(fnames_header[j],
                                                    fieldspec[i].second);
//                    perr<<"expanding "<<fieldspec[i] << " to " << n <<endl;
                    
                    new_fieldspec.append(n);
                    expended = true;
                }
            }
            if(!expended)
                no_expended_fields.append(fieldspec[i].first);
        }
        if(no_expended_fields.length()>0){
            NORMAL_LOG<<"In TextFilesVMatrix::setColumnNamesAndWidth - "
                      <<"Did not find any partial match for "
                      <<no_expended_fields.length()<<" spec:";
            for(int i=0;i<no_expended_fields.length();i++)
                NORMAL_LOG<<" "<<no_expended_fields[i];
            NORMAL_LOG<<endl;
        }
            
        fieldspec = new_fieldspec;
    }

    if(reorder_fieldspec_from_headers)
    {
        //check that all field names from the header have a spec
        TVec<string> not_used_fn;
        for(int i=0;i<fnames_header.size();i++)
        {
            string name=fnames_header[i];
            int j=0;
            for(;j<fieldspec.size();j++)
                if(fieldspec[j].first==name)
                    break;
            if(j>=fieldspec.size()){
                if(default_spec!=""){
                    fieldspec.append(make_pair(name,default_spec));
                }else
                    not_used_fn.append(name);
            }
        }
        //check that all fieldspec names are also in the header
        TVec<string> not_used_fs;
        for(int i=0;i<fieldspec.size();i++)
        {
            string name=fieldspec[i].first;
            int j=0;
            for(;j<fnames_header.size();j++)
                if(fnames_header[j]==name)
                    break;
            if(j>=fnames_header.size())
                not_used_fs.append(name);
        }
        //check that we have the good number of fieldspec
        //if partial match is true, we don't want to generate the warning everytime
        if(fnames_header.size()!=fieldspec.size() && !partial_match)
        {
            PLWARNING("In TextFilesVMatrix::setColumnNamesAndWidth() - "
                    "We read %d field names from the header but have %d"
                    "fieldspec",fnames_header.size(),fieldspec.size());
        }

        if(not_used_fs.size()!=0)
            PLWARNING("TextFilesVMatrix::setColumnNamesAndWidth() - "
                      "%d fieldspecs exists for field(s) that are not in the source: %s\n"
                      "They will be skipped.",
                      not_used_fs.length(), tostring(not_used_fs).c_str());
        if(not_used_fn.size()!=0)
            PLWARNING("TextFilesVMatrix::setColumnNamesAndWidth() - "
                      "%d fieldnames in source that don't have fieldspec: %s\n"
                      "They will be skipped.",
                      not_used_fn.length(), tostring(not_used_fn).c_str());
    

        //the new order for fieldspecs
        TVec< pair<string, string> > fs(fnames_header.size());
        for(int i=0;i<fnames_header.size();i++)
        {
            string name=fnames_header[i];
            int j=0;
            for(;j<fieldspec.size();j++)
                if(fieldspec[j].first==name)
                    break;
            if(j>=fieldspec.size())
                fs[i]=pair<string,string>(name,"skip");
            else
                fs[i]=fieldspec[j];
        }
        fieldspec=fs;
    }
    for(int k=0; k<fieldspec.length(); k++)
    {
        string fname = fieldspec[k].first;
        string ftype = fieldspec[k].second;
        if(isValidNonSkipFieldType(ftype))
        {
            // declare the column name
            fnames.push_back(fname);
            colrange.push_back( pair<int,int>(width_,1) );
            ++width_;
        }
        else if(ftype=="skip")
        {
            colrange.push_back( pair<int,int>(width_,0) );
        }
        else
            PLERROR("In TextFilesVMatrix::setColumnNamesAndWidth, Invalid field type specification for field %s: %s",fname.c_str(), ftype.c_str());
    }
    for(int j=0; j<width_; j++)
        declareField(j, fnames[j]);
}

void TextFilesVMatrix::build_()
{
    if (!default_spec.empty() && !reorder_fieldspec_from_headers)
        PLERROR("In TextFilesVMatrix::build_() when the option default_spec is used, reorder_fieldspec_from_headers must be true");
    if (metadatapath != "") {
        PLWARNING("In TextFilesVMatrix::build_() metadatapath option is deprecated. "
                  "You should use metadatadir instead.\n");

        metadatadir = metadatapath;
        setMetaDataDir(metadatapath);
    }
}
////////////////////
// setMetaDataDir //
////////////////////
void TextFilesVMatrix::setMetaDataDir(const PPath& the_metadatadir){
    inherited::setMetaDataDir(the_metadatadir);

    if(getMetaDataDir().empty())
        PLERROR("In TextFilesVMatrix::setMetaDataDir() - We need a metadatadir");
    if(!force_mkdir(getMetaDataDir()))
        PLERROR("In TextFilesVMatrix::setMetaDataDir() - could not create"
                " directory '%s'",
                getMetaDataDir().absolute().c_str());

    for(int i=0;i<txtfilenames.length();i++)
        updateMtime(txtfilenames[i]);

    PPath metadir = getMetaDataDir();
    PPath idxfname = metadir/"txtmat.idx";

    // Now open txtfiles
    int nf = txtfilenames.length();
    txtfiles.resize(nf);
    for(int k=0; k<nf; k++)
    {
        string fnam = txtfilenames[k];
        txtfiles[k] = fopen(fnam.c_str(),"r");
        if(txtfiles[k]==NULL){
            perror("Can't open file");
            PLERROR("In TextFilesVMatrix::setMetaDataDir - Can't open file %s",
                    fnam.c_str());
        }
    }

    setColumnNamesAndWidth();

    // open the index file
    if(!isUpToDate(idxfname) || isemptyFile(idxfname))
        buildIdx(); // (re)build it first!
    idxfile = fopen(idxfname.c_str(),"rb");
    if(fgetc(idxfile) != byte_order())
        PLERROR("In TextFilesVMatrix::setMetaDataDir - Wrong endianness."
                " Remove the index file %s for it to be automatically rebuilt",
                idxfname.c_str());
    fread(&length_, 4, 1, idxfile);

    // Initialize some sizes
    int n = fieldspec.size();
    mapping.resize(n);
    mapfiles.resize(n);
    mapfiles.fill(0);

    // Handle string mapping
    loadMappings();

    if (auto_build_map)
        autoBuildMappings();

    if(build_vmatrix_stringmap)
        buildVMatrixStringMapping();

    // Sanity checking
}


string TextFilesVMatrix::getTextRow(int i) const
{
    unsigned char fileno;
    int pos;
    getFileAndPos(i, fileno, pos);
    FILE* f = txtfiles[(int)fileno];
    fseek(f,pos,SEEK_SET);
    char buf[50000];

    if(!fgets(buf, sizeof(buf), f))
        PLERROR("In TextFilesVMatrix::getTextRow - fgets for row %d returned NULL",i);
    return removenewline(buf);
}

void TextFilesVMatrix::loadMappings()
{
    int n = fieldspec.size();
    for(int k=0; k<n; k++)
    {
        string fname = getMapFilePath(k);
        if (isfile(fname)) {
            updateMtime(fname);
            vector<string> all_lines = getNonBlankLines(loadFileAsString(fname));
            for (size_t i = 0; i < all_lines.size(); i++) {
                string map_line = all_lines[i];
                size_t start_of_string = map_line.find('"');
                size_t end_of_string = map_line.rfind('"');
                string strval = map_line.substr(start_of_string + 1, end_of_string - start_of_string - 1);
                string real_val_str = map_line.substr(end_of_string + 1);
                real real_val;
                if (!pl_isnumber(real_val_str, &real_val))
                    PLERROR("In TextFilesVMatrix::loadMappings - Found a mapping to something that is not a number (%s) in file %s at non-black line %ld", map_line.c_str(), fname.c_str(), long(i));
                mapping[k][strval] = real_val;
            }
        }
    }
}

///////////////////////
// autoBuildMappings //
///////////////////////
void TextFilesVMatrix::autoBuildMappings() {
    // TODO We should somehow check the date of existing mappings to see if they need to be built.
    // For now we just create them if they do not exist yet.

    // First make sure there is no existing mapping.
    int nb_already_exist = 0;
    int nb_type_no_mapping = 0;
    for (int i = 0;  i < mapping.length(); i++) {
        if (!mapping[i].empty())
            nb_already_exist++;
        else if(fieldspec[i].second!="char")//should add auto when it is char that are selected
            nb_type_no_mapping++;
    }
    if(nb_already_exist == 0){
        // Mappings need to be built.
        // We do this by reading the whole data.
        Vec row(width());
        bool auto_extend_map_backup = auto_extend_map;
        auto_extend_map = true;
        ProgressBar pb("Building mappings", length());
        for (int i = 0; i < length(); i++) {
            getRow(i, row);
            pb.update(i + 1);
        }
        auto_extend_map = auto_extend_map_backup;
    }else if (nb_already_exist+nb_type_no_mapping < mapping.length()) {
        for (int i = 0;  i < mapping.length(); i++) 
            if(fieldspec[i].second=="char" && mapping[i].empty())//should add auto when it is char that are selected
                PLWARNING("In TextFilesVMatrix::autoBuildMappings - mapping already existing but not for field %d (%s)",i,fieldspec[i].first.c_str());

        PLWARNING("In TextFilesVMatrix::autoBuildMappings - The existing "
                "mapping is not complete! There are %d fields with build "
                "mapping and there are %d fields that do not need mapping "
                "in a total of %d fields. Erase the mapping directory in "
                "the metadatadir to have it regenerated next time!",
                nb_already_exist,nb_type_no_mapping,mapping.length());
    }//else already build
}

void TextFilesVMatrix::generateMapCounts()
{
    int n = fieldspec.size();
    TVec< hash_map<string, int> > counts(n);
    for(int k=0; k<n; k++)
    {
        if(!mapping[k].empty())
        {
            hash_map<string, real>& mapping_k = mapping[k];
            hash_map<string, int>& counts_k = counts[k];
            hash_map<string, real>::const_iterator it = mapping_k.begin();
            hash_map<string, real>::const_iterator itend = mapping_k.end();
            while(it!=itend)
            {
                counts_k[ it->first ] = 0;
                ++it;
            }
        }
    }

    int l = length();
    ProgressBar pg("Generating counts of mappings",l);
    for(int i=0; i<l; i++)
    {
        TVec<string> fields = getTextFields(i);
        for(int k=0; k<fields.length(); k++)
        {
            if(mapping[k].find(fields[k])!=mapping[k].end())
                ++counts[k][fields[k]];
        }
        pg(i);
    }

    // Save the counts
    for(int k=0; k<n; k++)
    {
        if(!counts[k].empty())
            PLearn::save( getMetaDataDir() / "counts" / fieldspec[k].first+".count", counts[k] );
    }

}

void TextFilesVMatrix::buildVMatrixStringMapping()
{
    int n = fieldspec.size();
    for(int k=0; k<n; k++)
    {
        if(mapping[k].size()>0)
        {
            // get the corresponding VMatrix column range and add the VMatrix mapping
            int colstart = colrange[k].first;
            int ncols = colrange[k].second;
            hash_map<string,real>::const_iterator it = mapping[k].begin();
            hash_map<string,real>::const_iterator itend = mapping[k].end();
            while(it!=itend)
            {
                for(int j=colstart; j<colstart+ncols; j++)
                    addStringMapping(j, it->first, it->second);
                ++it;
            }
        }
    }
}

real TextFilesVMatrix::getMapping(int fieldnum, const string& strval) const
{
    hash_map<string, real>& m = mapping[fieldnum];
    hash_map<string, real>::const_iterator found = m.find(strval);
    if(found!=m.end()) // found it!
        return found->second;

    // strval not found
    if(!auto_extend_map)
        PLERROR("In TextFilesVMatrix::getMapping - No mapping found for field %d (%s) string-value \"%s\" ", fieldnum, fieldspec[fieldnum].first.c_str(), strval.c_str());

    // OK, let's extend the mapping...
    real val = real(-1000 - int(m.size()));
    m[strval] = val;

    if(!mapfiles[fieldnum])
    {
        string fname = getMapFilePath(fieldnum);
        force_mkdir_for_file(fname);
        mapfiles[fieldnum] = fopen(fname.c_str(),"a");
        if(!mapfiles[fieldnum])
            PLERROR("In TextFilesVMatrix::getMapping - Could not open map file %s\n for appending\n",fname.c_str());
    }

    fprintf(mapfiles[fieldnum],"\n\"%s\" %f", strval.c_str(), val);
    return val;
}

TVec<string> TextFilesVMatrix::splitIntoFields(const string& raw_row) const
{
    return split_quoted_delimiter(removeblanks(raw_row), delimiter,quote_delimiter);
}

TVec<string> TextFilesVMatrix::getTextFields(int i) const
{
    string rowi = getTextRow(i);
    TVec<string> fields =  splitIntoFields(rowi);
    if(fields.size() != fieldspec.size())
        PLERROR("In TextFilesVMatrix::getTextFields - In getting fields of row %d, wrong number of fields: %d (should be %d):\n%s\n",i,fields.size(),fieldspec.size(),rowi.c_str());
    for(int k=0; k<fields.size(); k++)
        fields[k] = removeblanks(fields[k]);
    return fields;
}

real TextFilesVMatrix::getPostalEncoding(const string& strval, bool display_warning) const
{
    if(strval=="")
        return MISSING_VALUE;

    char first_char = strval[0];
    int second_digit = strval[1];
    real val = 0;
    if(first_char=='A')
        val = 30 + second_digit;
    else if(first_char=='B')
        val = 40 + second_digit;
    else if(first_char=='C')
        val = 50 + second_digit;
    else if(first_char=='E')
        val = 60 + second_digit;
    else if(first_char=='G')
        val = 0 + second_digit;
    else if(first_char=='H')
        val = 10 + second_digit;
    else if(first_char=='J')
        val = 20 + second_digit;
    else if(first_char=='K')
        val = 70 + second_digit;
    else if(first_char=='L')
        val = 80 + second_digit;
    else if(first_char=='M')
        val = 90 + second_digit;
    else if(first_char=='N')
        val = 100 + second_digit;
    else if(first_char=='P')
        val = 110 + second_digit;
    else if(first_char=='R')
        val = 120 + second_digit;
    else if(first_char=='S')
        val = 130 + second_digit;
    else if(first_char=='T')
        val = 140 + second_digit;
    else if(first_char=='V')
        val = 150 + second_digit;
    else if(first_char=='W')
        val = 160 + second_digit;
    else if(first_char=='X')
        val = 170 + second_digit;
    else if(first_char=='Y')
        val = 180 + second_digit;
    else if(first_char=='0' || first_char=='1' || first_char=='2' || first_char=='3' ||
            first_char=='4' || first_char=='5' || first_char=='6' || first_char=='7' ||
            first_char=='8' || first_char=='9') {
        // That would be a C.P.
        int first_digit = strval[0];
        val = 260 + first_digit * 10 + second_digit;
    }
    else {
        //http://en.wikipedia.org/wiki/Canadian_postal_code
        //No postal code includes the letters D, F, I, O, Q, or U,
        //as the OCR equipment used in automated sorting could easily
        //confuse them with other letters and digits, 
        if (display_warning) {
            string errmsg;
            if(first_char=='D' ||first_char=='F' ||first_char=='I' ||
               first_char=='O' ||first_char=='Q' ||first_char=='U')
                errmsg = "Postal code don't use letters D, F, I, O, Q, or U: ";
            else
                errmsg = "Currently only some postal codes are supported: ";

            errmsg += "can't process " + strval + ", value will be set to 0.";
            PLWARNING(errmsg.c_str());
        }
        val = 0;
    }

    return val;
}

void TextFilesVMatrix::transformStringToValue(int k, string strval, Vec dest) const
{
    strval = removeblanks(strval);
    string fieldname = fieldspec[k].first;
    string fieldtype = fieldspec[k].second;
    real val;

    if(dest.length() != colrange[k].second)
        PLERROR("In TextFilesVMatrix::transformStringToValue, destination vec for field %d should be of length %d, not %d",k,colrange[k].second, dest.length());


    if(fieldtype=="skip")
    {
        // do nothing, simply skip it
        return;
    }
    
    if(strval=="")  // missing
        dest[0] = MISSING_VALUE;
    else if(fieldtype=="auto")
    {
        if(pl_isnumber(strval,&val))
            dest[0] = real(val);
        else
            dest[0] = getMapping(k, strval);
    }
    else if(fieldtype=="auto-num")
    {//We suppose the decimal point is '.'
        string s=strval;
        if(strval[0]=='$')
            s.erase(0,1);
        if(strval[strval.size()-1]=='$')
            s.erase(s.end());
        
        for(unsigned int pos=0; pos<strval.size(); pos++)
            if(s[pos]==',')
                s.erase(pos--,1);
        if(pl_isnumber(s,&val))
            dest[0] = real(val);
        else
            PLERROR("In TextFilesVMatrix::transformStringToValue -"
                    " expedted [$]number[$] as the value for field %d(%s)."
                    " Got %s", k, fieldname.c_str(), strval.c_str());
    }
    else if(fieldtype=="char")
    {
        dest[0] = getMapping(k, strval);
    }
    else if(fieldtype=="num")
    {
        if(pl_isnumber(strval,&val))
            dest[0] = real(val);
        else
            PLERROR("In TextFilesVMatrix::transformStringToValue - expedted a number as the value for field %d(%s). Got %s",k,fieldname.c_str(),strval.c_str());
                
    }
    else if(fieldtype=="date")
    {
        dest[0] = date_to_float(PDate(strval));
    }
    else if(fieldtype=="jdate")
    {
        dest[0] = PDate(strval).toJulianDay();
    }
    else if(fieldtype=="sas_date")
    {
        if(strval == "0")  // missing
            dest[0] = MISSING_VALUE;
        else if(pl_isnumber(strval,&val)) {
            dest[0] = val;
            if (val <= 0) {
                PLERROR("In TextFilesVMatrix::transformStringToValue - "
                        "I didn't know a sas_date could be negative");
            }
        }
        else
            PLERROR("In TextFilesVMatrix::transformStringToValue - "
                    "Error while parsing a sas_date");
    }
    else if(fieldtype=="YYYYMM")
    {
        if(!pl_isnumber(strval) || toint(strval)<197000)
            dest[0] = MISSING_VALUE;
        else
            dest[0] = PDate(strval+"01").toJulianDay();
    }
    else if(fieldtype=="postal")
    {
        dest[0] = getPostalEncoding(strval);
    }
    else if(fieldtype=="dollar" || fieldtype=="dollar-comma")
    {
        char char_torm = ' ';
        if(fieldtype=="dollar-comma")
            char_torm = ',';
        if(strval[0]=='$')
        {
            string s = "";
            for(unsigned int pos=1; pos<strval.size(); pos++)
                if(strval[pos]!=char_torm)
                    s += strval[pos];

            if(pl_isnumber(s,&val))
                dest[0] = real(val);
            else
                PLERROR("In TextFilesVMatrix::transformStringToValue - Goat as value '%s' while parsing field %d (%s) with fieldtype %s",strval.c_str(),k,fieldname.c_str(),fieldtype.c_str());
        }
        else
            PLERROR("In TextFilesVMatrix::transformStringToValue - Got as value '%s' while expecting a value beggining with '$' while parsing field %d (%s) with fieldtype %s",strval.c_str(),k,fieldname.c_str(),fieldtype.c_str());
    }
    else if(fieldtype=="bell_range") {
        if (strval == "Negative Value") {
            // We put an arbitrary negative value since we don't have more info.
            dest[0] = -100;
        } else {
            // A range of the kind "A: $0- 250".
            string s = "";
            unsigned int pos;
            unsigned int end;
            for (pos=0; pos<strval.size() && (strval[pos] == ' ' || !pl_isnumber(strval.substr(pos,1))); pos++) {}
            for (end=pos; end<strval.size() && strval[end] != ' ' && pl_isnumber(strval.substr(end,1)); end++) {
                s += strval[end];
            }
            real number_1,number_2;
            if (!pl_isnumber(s,&number_1) || is_missing(number_1)) {
                PLERROR(("TextFilesVMatrix::transformStringToValue: " + strval +
                         " is not a well formatted Bell range").c_str());
            }
            s = "";
            for (pos=end; pos<strval.size() && (strval[pos] == ' ' || !pl_isnumber(strval.substr(pos,1))); pos++) {}
            for (end=pos; end<strval.size() && strval[end] != ' ' && pl_isnumber(strval.substr(end,1)); end++) {
                s += strval[end];
            }
            if (!pl_isnumber(s,&number_2) || is_missing(number_2)) {
                PLERROR(("TextFilesVMatrix::transformStringToValue: " + strval +
                         " is not a well formatted Bell range").c_str());
            }
            dest[0] = (number_1 + number_2) / (real) 2;
        }
    }
    else if(fieldtype=="num-comma")
    {
        string s="";
        for(uint i=0;i<strval.length();i++)
        {
            if(strval[i]!=',')
                s=s+strval[i];
        }
        if(pl_isnumber(s,&val))
            dest[0] = real(val);
        else
            PLERROR("In TextFilesVMatrix::transformStringToValue - expedted a number as the value for field %d(%s). Got %s",k,fieldname.c_str(),strval.c_str());
                
    }
    else
    {
        PLERROR("TextFilesVMatrix::TextFilesVMatrix::transformStringToValue, Invalid field type specification for field %s: %s",fieldname.c_str(), fieldtype.c_str());
    }
}

void TextFilesVMatrix::getNewRow(int i, const Vec& v) const
{
    TVec<string> fields = getTextFields(i);
    int n = fields.size();

    for(int k=0; k<n; k++)
    {
        string fieldname = fieldspec[k].first;
        string fieldtype = fieldspec[k].second;
        string strval = fields[k];
        Vec dest = v.subVec(colrange[k].first, colrange[k].second);

        try
        { transformStringToValue(k, strval, dest); }
        catch(const PLearnError& e)
        {
            PLERROR("In TextFilesVMatrix, while parsing field %d (%s) of row %d: \n%s",
                    k,fieldname.c_str(),i,e.message().c_str());
        }
    }
}

void TextFilesVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "metadatapath", &TextFilesVMatrix::metadatapath, OptionBase::buildoption,
                  "Path of the metadata directory (in which to store the index, ...)\n"
                  "DEPRECATED: use metadatadir instead.\n");

    declareOption(ol, "txtfilenames", &TextFilesVMatrix::txtfilenames, OptionBase::buildoption,
                  "A list of paths to raw text files containing the records");

    declareOption(ol, "delimiter", &TextFilesVMatrix::delimiter, OptionBase::buildoption,
                  "Delimiter to use to split the fields.  Common delimiters are:\n"
                  "- \"\\t\" : used for SAS files (the default)\n"
                  "- \",\"  : used for CSV files\n"
                  "- \";\"  : used for a variant of CSV files\n"
                  "If more then 1 delimiter, we will select one based on the"
                  " first two line\n");

    declareOption(ol, "quote_delimiter", &TextFilesVMatrix::quote_delimiter, OptionBase::buildoption,
        "The escape character to indicate the delimiter is not considered.\n"
        "For instance, '\"' is frequently used.");

    declareOption(ol, "skipheader", &TextFilesVMatrix::skipheader, OptionBase::buildoption,
                  "An (optional) list of integers, one for each of the txtfilenames,\n"
                  "indicating the number of header lines at the top of the file to be skipped.");

    declareOption(ol, "fieldspec", &TextFilesVMatrix::fieldspec, OptionBase::buildoption,
                  "Specification of field names and types (type indicates how the text field is to be mapped to one or more reals)\n"
                  "Currently supported types: \n"
                  "- skip       : Ignore the content of the field, won't be inserted in the resulting VMat\n"
                  "- auto       : If a numeric value, keep it as is, if not, look it up in the mapping (possibly inserting a new mapping if it's not there) \n"
                  "- auto-num   : take any float value with the decimal separator as the dot. If there is a $\n"
                  "               at the start or end it is removed. If there are commas they are removed.\n"
                  "- num        : numeric value, keep as is\n"
                  "- num-comma  : numeric value where thousands are separeted by comma\n"
                  "- char       : look it up in the mapping (possibly inserting a new mapping if it's not there)\n"
                  "- date       : date of the form 25DEC2003 or 25-dec-2003 or 2003/12/25 or 20031225, will be mapped to float date format 1031225\n"
                  "- jdate      : date of the form 25DEC2003 or 25-dec-2003 or 2003/12/25 or 20031225, will be mapped to *julian* date format\n"
                  "- sas_date   : date used by SAS = number of days since Jan. 1st, 1960 (with 0 = missing)\n"
                  "- YYYYMM     : date of the form YYYYMM (e.g: 200312), will be mapped to the julian date of the corresponding month. Everthing "\
                  "               other than a number or lower than 197000 is considered as nan\n"
                  "- postal     : canadian postal code \n"
                  "- dollar     : strangely formatted field with dollar amount. Format is sth like '$12 003'\n"
                  "- dollar-comma : strangely formatted field with dollar amount. Format is sth like '$12,003'\n"
                  "- bell_range : a range like \"A: $0- 250\", replaced by the average of the two bounds;\n"
                  "               if the \"Negative Value\" string is found, it is replaced by -100\n"
        );

    declareOption(ol, "auto_extend_map", &TextFilesVMatrix::auto_extend_map, OptionBase::buildoption,
                  "If true, new strings for fields of type AUTO will automatically appended to the mapping (in the metadata/mappings/fieldname.map file)");

    declareOption(ol, "auto_build_map", &TextFilesVMatrix::auto_build_map, OptionBase::buildoption,
                  "If true, all mappings will be automatically computed at build time if they do not exist yet\n");

    declareOption(ol, "build_vmatrix_stringmap", &TextFilesVMatrix::build_vmatrix_stringmap,
                  OptionBase::buildoption,
                  "If true, standard vmatrix stringmap will be built from the txtmat specific stringmap");

    declareOption(ol, "reorder_fieldspec_from_headers", 
                  &TextFilesVMatrix::reorder_fieldspec_from_headers,
                  OptionBase::buildoption,
                  "If true, will reorder the fieldspec in the order given "
                  "by the field names taken from txtfilenames.");

    declareOption(ol, "partial_match", 
                  &TextFilesVMatrix::partial_match,
                  OptionBase::buildoption,
                  "If true, will repeatedly expand all fieldspec name ending "
                  "with * to the full name from header."
                  "The expansion is equivalent to the regex 'field_spec_name*'."
                  "The option reorder_fieldspec_from_headers must be true");

    declareOption(ol, "default_spec", 
                  &TextFilesVMatrix::default_spec,
                  OptionBase::buildoption,
                  "If there is no fieldspec for a fieldname, we will use this"
                  "value. reorder_fieldspec_from_headers must be true.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void TextFilesVMatrix::readAndCheckOptionName(PStream& in, const string& optionname, char buf[])
{
    in.skipBlanksAndComments();
    in.readUntil(buf, sizeof(buf), "= ");
    string option = removeblanks(buf);
    in.skipBlanksAndComments();
    char eq = in.get();
    if(option!=optionname || eq!='=')
        PLERROR("In TextFilesVMatrix::readAndCheckOptionName - "
                "Bad syntax in .txtmat file.\n"
                "Expected option %s = ...\n"
                "Read %s %c\n", optionname.c_str(), option.c_str(), eq);
}


// ### Nothing to add here, simply calls build_
void TextFilesVMatrix::build()
{
    inherited::build();
    build_();
}

TextFilesVMatrix::~TextFilesVMatrix()
{
    if(idxfile)
        fclose(idxfile);
    for(int k=0; k<txtfiles.length(); k++)
        fclose(txtfiles[k]);

    for(int k=0; k<mapfiles.size(); k++)
    {
        if(mapfiles[k])
            fclose(mapfiles[k]);
    }
}

void TextFilesVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    idxfile=0;
    txtfiles.resize(0);
    //the map should be already build.
    auto_build_map=false;
    build();
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
