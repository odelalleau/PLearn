// -*- C++ -*-

// ConfigParsing.cc
//
// Copyright (C) 2009 Frederic Bastien
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

// Authors: Frederic Bastien

/*! \file ConfigParsing.cc */


#include "ConfigParsing.h"
#include <plearn/vmat/TextFilesVMatrix.h>
#include <plearn/io/openFile.h>
#include <plearn/io/pl_log.h>
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'ConfigParsing' command in the command registry
PLearnCommandRegistry ConfigParsing::reg_(new ConfigParsing);

ConfigParsing::ConfigParsing()
    : PLearnCommand(
        "ConfigParsing",
        ">>>> INSERT A SHORT ONE LINE DESCRIPTION HERE",
        ">>>> INSERT SYNTAX AND \n"
        "FULL DETAILED HELP HERE \n"
        )
{}

//! The actual implementation of the 'ConfigParsing' command
void ConfigParsing::run(const vector<string>& args)
{
/*    args0 = conf/conf.all.csv;
    args1 = conf/1convertCSV0709toPLearn.inc;
    args2 = conf/3b_remove_col.inc;
    args3 = conf/3fix_missing.inc;
    args4 = conf/9dichotomize.inc;
    args5 = conf/global_imputation_specifications.inc;
*/
    PLCHECK(args.size()==6);
    TextFilesVMatrix input = TextFilesVMatrix();
    input.auto_build_map = 0  ;
    input.default_spec="char";
    input.build_vmatrix_stringmap = 1  ;
    input.delimiter = ",;"  ;
    input.quote_delimiter = '"';
    input.skipheader.append(1);
    input.reorder_fieldspec_from_headers=1;
    input.txtfilenames.append(args[0]);
    input.partial_match=1;
    input.setMetaDataDir(args[0]+".metadatadir");
    input.build();
    bool all_uptodate = true;
    for(int i=1;i<=5;i++)
        if(!input.isUpToDate(args[i])){
            all_uptodate = false;
            break;
        }
    if(all_uptodate){
        NORMAL_LOG << "All config file are uptodate. We don't regenerate them."<<endl;
        return;
    }
    PStream f_csv = openFile(PPath(args[1]),PStream::raw_ascii,"w");
    PStream f_remove = openFile(args[2],PStream::raw_ascii,"w");
    PStream f_missing = openFile(args[3],PStream::raw_ascii,"w");
    PStream f_dichotomize = openFile(args[4],PStream::raw_ascii,"w");
    PStream f_imputation = openFile(args[5],PStream::raw_ascii,"w");

    for(int i=0;i<input.length();i++){
        TVec<string> r = input.getTextFields(i);
        char c = r[0][0];
        if(c=='#' || r[0].empty())//comment
            continue;
        if(!r[1].empty()){
            f_csv << (r[0]);
            f_csv << (" : ");
            f_csv << (r[1]) << endl;
        }
        string y = lowerstring(r[2]);//TODO check that this is an accepted command.
        bool remove=false;
        if(y=="y" ||y=="yes"){//comment
            f_remove << (r[0]) << endl;
            remove=true;
        }else if(y=="n" ||y=="no"||y==""){
        }else{
            PLERROR("Unknow value in column C:'%s'",r[2].c_str());
        }
        if(!r[3].empty() && !remove){
            f_missing << (r[0]);
            f_missing << (" : ");
            f_missing << (r[3]);//TODO check that this is an accepted command.
            f_missing << endl;
        }
        if(!r[4].empty() && !remove){
            f_imputation << (r[0]);
            f_imputation << (" : ");
            f_imputation << (r[4]);//TODO check that this is an accepted command.
            f_imputation << endl;
        }
        if(!r[5].empty() && !remove){
            string s = r[0];
            if(s[s.length()-1]=='*')
                s=s.substr(0,s.length()-1);
            f_dichotomize << s <<" : ["<< r[5] << " ]"<<endl;
        }
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
