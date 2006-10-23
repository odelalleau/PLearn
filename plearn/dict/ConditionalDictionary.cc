// -*- C++ -*-

// ConditionalDictionary.cc
//
// Copyright (C) 2004 Hugo Larochelle, Christopher Kermorvant
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
 * $Id: ConditionalDictionary.cc 4184 2005-10-04 14:02:12Z larocheh $ 
 ******************************************************* */

// Authors: Hugo Larochelle

/*! \file ConditionalDictionary.cc */


#include "ConditionalDictionary.h"
#include <plearn/base/stringutils.h>
#include "plearn/io/openFile.h"
#include "plearn/io/fileutils.h"

namespace PLearn {
using namespace std;

string ConditionalDictionary::get_option_string(TVec<string> options)
{
    string ret = options[0];
    for(int i=1; i<options.length(); i++)
        ret = ret + " " + options[i];
    if(options_to_lower_case)
        return lowerstring(ret);
    else
        return ret;
}

ConditionalDictionary::ConditionalDictionary()
    : options_to_lower_case(false)
{}
  
PLEARN_IMPLEMENT_OBJECT(ConditionalDictionary,
                        "Dictionary where possible symbols depend on the option fields.",
                        "The mapping between the options and possible symbols is defined\n"
                        "by a mapping file, where each row has the form:\n"
                        "\n"
                        "OPTION_1 ... OPTION_M[\tab]TAG_1 TAG_2 TAG)3 ...\n"
                        "\n"
                        "with [\tab] being the tabulation character. For\n"
                        "instance, a Part Of Speech Dictionary could be\n"
                        "defined using lines like:\n"
                        "\n"
                        "pet[\tab]NN VB VBD\n"
                        "\n"
                        "When the option fields are not found in the mapping,\n"
                        "then the possible values are simply the set of\n"
                        "all possible symbols (TAG fields) of the Dictionary.\n");
  
void ConditionalDictionary::declareOptions(OptionList& ol)
{
    declareOption(ol, "options_to_lower_case", &ConditionalDictionary::options_to_lower_case, OptionBase::buildoption, "Indication that the given options should be put to lower case");
    declareOption(ol, "mapping_file_path", &ConditionalDictionary::mapping_file_path, OptionBase::buildoption, "Path to the mapping file");
    inherited::declareOptions(ol);
}
  
void ConditionalDictionary::build_()
{
    if(options_to_symbols.size() == 0)
    {
        int last_update_mode = update_mode;
        update_mode = UPDATE;
        
        TVec<string> tokens;
        TVec<string> options;
        TVec<string> symbols;
        int id;
        string line; 
        PStream input_stream = openFile(mapping_file_path, PStream::raw_ascii);
        while (!input_stream.eof()){
            getNextNonBlankLine(input_stream, line);
            tokens = split(line, "\t");
            if(tokens.length() != 2)
                PLERROR("In ConditionalDictionary::build_(): line \"%s\" is in bad format",line.c_str());
            options = split(tokens[0]," ");
            symbols = split(tokens[1]," ");
            
            Vec symbols_id = Vec(0); 

            // Insert symbols in dictionaries
            for(int i=0; i<symbols.size(); i++)
            {
                id = getId(symbols[i]);
                if(symbols_id.find(id) < 0)
                    symbols_id.append(id);
            }
            // Insert symbols in map
            tmp_str = get_option_string(options);
            if(options_to_symbols.find(tmp_str) == options_to_symbols.end())
                PLERROR("In ConditionalDictionary::build_(): there is more than one mapping for option \"%s\"",tmp_str.c_str());
            options_to_symbols[tmp_str] = symbols_id;
        }
        
        update_mode = last_update_mode;
    }
}

// ### Nothing to add here, simply calls build_
void ConditionalDictionary::build()
{
    inherited::build();
    build_();
}

void ConditionalDictionary::getValues(TVec<string> options, Vec& values)
{ 
    if(options.length() == 0)
        inherited::getValues(options, values);

    string ret = get_option_string(options);
    if(options_to_symbols.find(ret) != options_to_symbols.end())
        values << options_to_symbols[ret];
    else
        inherited::getValues(options, values);
}

int ConditionalDictionary::size(TVec<string> options){
    if(options.length() == 0)
        return inherited::size();
    else         
    {
        string ret = get_option_string(options);
        if(options_to_symbols.find(ret) != options_to_symbols.end())
            return options_to_symbols[ret].length();
        else
            return inherited::size();
    }
}

void ConditionalDictionary::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);    
    deepCopyField(options_to_symbols, copies);
    deepCopyField(tmp_sym, copies);
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
