// -*- C++ -*-

// StatsCommand.cc
//
// Copyright (C) 2008 Frederic Bastien
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

/*! \file StatsCommand.cc */


#include "StatsCommand.h"
#include <plearn/math/StatsCollector.h>
#include <plearn/math/VecStatsCollector.h>
#include <plearn/math/TMat_sort.h>
#include <plearn/io/load_and_save.h>
#include <boost/regex.hpp>
#include <plearn/base/plerror.h>
#include <plearn/base/stringutils.h>
namespace PLearn {
using namespace std;

//! This allows to register the 'StatsCommand' command in the command registry
PLearnCommandRegistry StatsCommand::reg_(new StatsCommand);

StatsCommand::StatsCommand()
    : PLearnCommand(
        "stats",
        "allow to extract some stats from stats file",
        "plearn stats sum_sort <statsfile> --filter=boost_regex --replace=boost_regex=new_value\n"
        "FULL DETAILED HELP HERE \n"
        )
{}

//! The actual implementation of the 'StatsCommand' command
void StatsCommand::run(const vector<string>& args)
{
    if(args[0]=="sum_sort"){
        //plearn Stats sum_sort <statsfile> --filter=boost_regex --replace=boost_regex=new_value
        if(args.size()<2)
            PLERROR("plearn stats sum_sort <statsfile> --filter=boost_regex --replace=boost_regex=new_value");

        //default regex that match all non new line caracter
        boost::regex filter(".*");
        boost::regex replace_re;
        string replace_new = "";

            
        string statsfile = args[1];
        string test_re;
        try{
            for(uint i=2;i<args.size();i++){
                if(args[i].substr(0,9)=="--filter="){
                    test_re=args[i].substr(9);
                    filter.assign(test_re);
                }else if(args[i].substr(0,10)=="--replace="){
                    split_on_first(args[i].substr(10), "=",
                                   test_re, replace_new);
                    replace_re.assign(test_re);
                }else
                    PLERROR("In StatsCommand::run() - unknow parameter '%s'", 
                            args[i].c_str());
            }
        }catch (boost::regex_error& e){
            PLERROR("invalid regular expression: \"%s\"\n %s",
                    test_re.c_str(),
                    e.what());
        }
        TVec<StatsCollector> stats1;
        VecStatsCollector stats;

        PLearn::load(statsfile, stats);
        Mat m(stats.size(),2);
        for(int i = 0;i<stats.size();i++){
            m(i,0)=i;
            m(i,1)=stats.getStats(i).sum();
        }

        pout<<"NAME "<<"SUM"<<endl;
        sortRows(m,1,false);
        for(int i=0;i<m.length();i++){
            string f = stats.getFieldNames()[int(m(i,0))];
            if(boost::regex_match(f, filter)){
                if(replace_re.size()>0)
                    f = boost::regex_replace(f,replace_re,"");
                pout<<f<<" "<<m(i,1)<<endl;
            }
        }
    }else
        PLERROR("Currently only the sub commands sum_sort is supported!");
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
