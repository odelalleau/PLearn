// -*- C++ -*-

// HTMLUtils.cc
//
// Copyright (C) 2004-2006 Nicolas Chapados 
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies, inc.
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

// Authors: Xavier Saint-Mleux

/*! \file HTMLUtils.cc */


#include "HTMLUtils.h"

#include <plearn/base/stringutils.h>
#include <plearn/base/tostring.h>
#include <boost/regex.hpp>
#include <commands/PLearnCommands/plearn_main.h>
#include <plearn/base/TypeFactory.h>
#include <plearn/base/OptionBase.h>

namespace PLearn {
using namespace std;

string HTMLUtils::quote(string s)
{
    search_replace(s, "&", "&amp;");
    search_replace(s, "<", "&lt;");
    search_replace(s, ">", "&gt;");
    search_replace(s, "\"", "&quot;");
    return s;
}

string HTMLUtils::highlight_known_classes(string typestr)
{
    vector<string> tokens = split(typestr, " \t\n\r<>,.';:\"");
    set<string> replaced; // Carry out replacements for a given token only once
    const TypeMap& type_map = TypeFactory::instance().getTypeMap();
    vector<string>::size_type n=tokens.size();
    for (unsigned int i=0; i<n ; ++i) {
        TypeMap::const_iterator it = type_map.find(tokens[i]);
        if (it != type_map.end() && replaced.find(tokens[i]) == replaced.end()) {
            replaced.insert(tokens[i]);
      
            // ensure we only match whole words with the regular expression
            const boost::regex e("\\<" + tokens[i] + "\\>");
            const string repl_str("<a href=\"class_$&.html\\?level="
                                  + tostring(OptionBase::getCurrentOptionLevel()) 
                                  +"\">$&</a>");
            typestr = regex_replace(typestr, e, repl_str, boost::match_default | boost::format_default);
        }
    }
    return typestr;
}

string HTMLUtils::format_free_text(string text)
{
    // sort of DWIM HTML formatting for free-text; cannot use split since it
    // eats up consecutive delimiters
    text = removeblanks(text);
    size_t curpos = 0, curnl = text.find('\n');
    bool ul_active = false;
    bool start_paragraph = false;
    string finallines;
    for ( ; curpos != string::npos ;
          curpos = curnl+(curnl!=string::npos), curnl = text.find('\n', curpos) ) {
        string curline = text.substr(curpos, curnl-curpos);

        // step 1: check if the line starts with a '-': if so, start a new <UL>
        // if not in one, or extend one if so
        if (removeblanks(curline).substr(0,1) == "-" ||
            removeblanks(curline).substr(0,1) == "*" )
        {
            curline = removeblanks(curline).substr(1);
            if (! ul_active)
                curline = "<ul><li>" + curline;
            else
                curline = "<li>" + curline;
            start_paragraph = false;
            ul_active = true;
        }

        // otherwise, a line that starts with some whitespace within a list
        // just extends the previous <li> :: don't touch it
        else if (ul_active && (curline == "" ||
                               curline.substr(0,1) == " " ||
                               curline.substr(0,1) == "\t")) {
            /* do nothing */
        }

        // otherwise, normal processing
        else {
            // any line that is empty or starts with some whitespace gets its own <br>
            if (removeblanks(curline) == "") {
                // Don't start new paragraph right away; wait until we
                // encounter some text that's neither a <ul> or a <pre>
                start_paragraph = true;
                curline = "";
            }
            else if (curline[0] == ' ' || curline[0] == '\t') {
                start_paragraph = false;
                curline = "<pre>" + curline + "</pre>";
            }

            // if we were processing a list, close it first
            if (ul_active) {
                curline = "</ul>" + curline;
                ul_active = 0;
            }
        }

        if (!curline.empty() && start_paragraph) {
            finallines += "<p>";
            start_paragraph = false;
        }
        
        finallines += curline + "\n";
    }

    // Close any pending open blocks
    if (ul_active)
        finallines += "</ul>\n";
  
    // Finally join the lines
    return make_http_hyperlinks(finallines);
}
string HTMLUtils::make_http_hyperlinks(string text)
{
    // Find elements of the form XYZ://x.y.z/a/b/c and make them into
    // hyperlink. An issue is to determine when
    static const char* recognized_protocols[] = 
        { "http://", "https://", "ftp://", "mailto:" };        // for now...
    static const vector<string> protocols_vector(
        recognized_protocols,
        recognized_protocols + sizeof(recognized_protocols) / sizeof(recognized_protocols[0]));

    // Match everything that starts with the recognized protocol up to the
    // following whitespace, excluding trailing punctuation if any.
    // Make sure the URL is NOT enclosed in quotes
    static const boost::regex e( string("(?!\")") + "(" +
                                 "(?:" + join(protocols_vector, "|") + ")" +
                                 "\\S+(?:\\w|/)" +
                                 ")" + "(?!\")" + "([[:punct:]]*\\s|$)");

    const string repl_str("<a href=\"$1\?level=" 
                          + tostring(OptionBase::getCurrentOptionLevel())
                          +"\">$1</a>$2");
    text = regex_replace(text, e, repl_str, boost::match_default | boost::format_default);
    return text;
}

string HTMLUtils::generated_by()
{
    time_t curtime = time(NULL);
    struct tm *broken_down_time = localtime(&curtime);
    const int SIZE = 100;
    char time_buffer[SIZE];
    strftime(time_buffer,SIZE,"%Y/%m/%d %H:%M:%S",broken_down_time);

    return string("<p>&nbsp;</p><address>Generated on " ) +
        time_buffer + " by " + version_string() + "</address>";
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
