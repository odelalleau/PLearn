// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
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

#include "VMatLanguage.h"
#include <plearn/base/PDate.h>
#include <plearn/base/stringutils.h>
#include <plearn/base/plerror.h>
#include <plearn/db/getDataSet.h>
#include <plearn/io/fileutils.h>
#include <plearn/io/openFile.h>
#include <plearn/misc/Calendar.h>
#include <plearn/math/pl_erf.h>

namespace PLearn {
using namespace std;

bool VMatLanguage::output_preproc=false;


PLEARN_IMPLEMENT_OBJECT(VMatLanguage,
                        "This class implements the VPL mini-language.",
                        "VPL (VMat Processing Language) is a home brewed mini-language in postfix\n"
                        "notation. As of today, it is used is the {PRE,POST}FILTERING and\n"
                        "PROCESSING sections of a .vmat file. It supports INCLUDEs instructions\n"
                        "and DEFINEs (dumb named string constants). It can handle reals as well\n"
                        "as dates (format is: CYYMMDD, where C is 0 (1900-1999) or 1\n"
                        "(2000-2099). For more info, you can look at PLearnCore/VMatLanguage.*.\n"
                        "\n"
                        "A VPL code snippet is always applied to the row of a VMatrix, and can\n"
                        "only refer to data of that row (in the state it was before any\n"
                        "processing.) The result of the execution will be a vector which is the\n"
                        "execution stack at code termination, defining the row of same index in\n"
                        "the resulting matrix.\n"
                        "\n"
                        "When you use VPL in a PROCESSING section, each field you declare must\n"
                        "have its associated fieldname declaration. The compiler will ensure that\n"
                        "the size of the result vector and the number of declared fieldnames\n"
                        "match. This doesn't apply in the filtering sections, where you don't\n"
                        "declare fieldnames, since the result is always a single value.\n"
                        "\n"
                        "To declare a fieldname, use a colon with the name immediately after. To\n"
                        "batch-declare fieldnames, use eg. :myfield:1:10. This will declare\n"
                        "fields myfield1 up to myfield10.\n"
                        "\n"
                        "There are two notations to refer to a field value: the @ symbol followed\n"
                        "by the fieldname, or % followed by the field number (with valid indices.\n"
                        "going from 0 to num_fields-1).\n"
                        "\n"
                        "To batch-copy fields, use the following syntax: [field1:fieldn], which copies\n"
                        "all the fields from field1 up to and including fieldn (fields\n"
                        "can be in @ or % notation, with the keyword 'END' denoting the last field).\n"
                        "The fields can also be transformed with a VPL program using the syntax:\n"
                        "[field1:fieldn:vpl_code], where vpl_code can be any VPL code, for example\n"
                        "for a 0.5 thresholding: 0.5 < 0 1 ifelse. To copy a single field, use [field].\n"
                        "There is also a special feature available only for single field copies: if you\n"
                        "use the syntax [field?], then VPL will not produce an error if the field cannot\n"
                        "be found. END-N(N an integer) will select a field conting from the END.\n"
                        "\n"
                        "Here's a real-life example of a VPL program:\n"
                        "\n"
                        "    @lease_indicator 88 == 1 0 ifelse :lease_indicator\n"
                        "    @rate_class 1 - 7 onehot :rate_class:0:6\n"
                        "    @collision_deductible { 2->1; 4->2; 5->3; 6->4; 7->5;\n"
                        "       [8 8]->6; MISSING->0; OTHER->0 }\n"
                        "      7 onehot :collision_deductible:0:6\n"
                        "    @roadstar_indicator 89 == 1 0 ifelse :roadstar_indicator\n"
                        "\n"
                        "In the following, the syntax\n"
                        "\n"
                        "    a b c -> f(a,b,c)\n"
                        "\n"
                        "means that (a,b,c) in that order (i.e. 'a' bottommost and 'c' top-of-stack)\n"
                        "are taken from the stack, and the result f(a,b,c) is pushed on the stack\n"
                        "\n"
                        "List of valid VPL operators:\n"
                        "\n"
                        " _ pop            : pop last element from stack\n"
                        " _ dup            : duplicates last element on the stack\n"
                        " _ exch           : exchanges the two top-most elements on the stack\n"
                        " _ onehot         : index nclasses --> one-hot representation of index\n"
                        " _ gausshot       : index nclasses sigma --> smooth 'one-hot' representation of\n"
                        "                    index using a gaussian of width sigma.  Maximum value remains 1;\n"
                        "                    useful if there is some locality structure in the classes\n"
                        " _ thermometer    : index nclasses --> thermometer representation of index\n"
                        " _ +              : a b   -->  a + b\n"
                        " _ -              : a b   -->  a - b\n"
                        " _ *              : a b   -->  a * b\n"
                        " _ /              : a b   -->  a / b\n"
                        " _ neg            : a     -->  -a\n"
                        " _ ==             : a b   -->  a == b\n"
                        " _ !=             : a b   -->  a != b\n"
                        " _ >              : a b   -->  a >  b\n"
                        " _ >=             : a b   -->  a >= b\n"
                        " _ <              : a b   -->  a <  b\n"
                        " _ <=             : a b   -->  a <= b\n"
                        " _ and            : a b   -->  a && b\n"
                        " _ or             : a b   -->  a || b\n"
                        " _ not            : a     -->  !a\n"
                        " _ ifelse         : a b c -->  (a != 0? b : c)\n"
                        " _ fabs           : a     -->  fabs(a)\n"
                        " _ rint           : a     -->  rint(a)   ; round to closest int\n"
                        " _ floor          : a     -->  floor(a)\n"
                        " _ ceil           : a     -->  ceil(a)\n"
                        " _ log            : a     -->  log(a)    ; natural log\n"
                        " _ exp            : a     -->  exp(a)    ; e^a\n"
                        " _ sigmoid        : a     -->  sigmoid(a); sigmoid\n"
                        " _ cos            : a     -->  cos(a)    ; cosinus\n"
                        " _ rowindex       : pushes the row number in the VMat on the stack\n"
                        " _ isnan          : true if missing value\n"
                        " _ missing        : pushes a missing value\n"
                        " _ year           : CYYMMDD --> YYYY\n"
                        " _ month          : CYYMMDD --> MM\n"
                        " _ day            : CYYMMDD --> DD\n"
                        " _ daydiff        : CYYMMDD_a CYYMMDD_b --> (CYYMMDD_a - CYMMDD_b) in nb. of days\n"
                        " _ monthdiff      : continuous: nb. days / (365.25/12)\n"
                        " _ yeardiff       : continuous: nb. days / 365.25\n"
                        " _ year_month_day : CYYMMDD      --> YYYY MM DD\n"
                        " _ todate         : YYYY MM DD   --> CYYMMDD\n"
                        " _ dayofweek      : from CYYMMDD --> [0..6] (0=monday  6=sunday)\n"
                        " _ today          : todays date CYYMMDD\n"
                        " _ date2julian    : CYYMMDD      --> nb. days (JDate)\n"
                        " _ julian2date    : nb. days     --> CYYMMDD\n"
                        " _ weeknumber     : CYYMMDD      --> week number in the year between 0 and 52 incl.\n"
                        "                                     (ISO 8601 minus 1)\n"
                        " _ dayofyear      : CYYMMDD      --> number of days since january 1 of year CYY \n"
                        " _ nextincal      : CYYMMDD cal# --> next CYYMMDD ON OR AFTER given jdate within\n"
                        "                                     global calendar 'cal#'; global calendar name\n"
                        "                                     should be a string repr. of the integer cal#\n"
                        "                                     If not found, return 0\n"
                        " _ previncal      : CYYMMDD cal# --> previous CYYMMDD ON OR BEFORE given jdatewithin\n"
                        "                                     global calendar 'cal#'; global calendar name\n"
                        "                                     should be a string repr. of the integer cal#\n"
                        "                                     If not found, return 0\n"
                        " _ min            : b a  -->  (a<b? a : b)\n"
                        " _ max            : b a  -->  (a<b? b : a)\n"
                        " _ sqrt           : a    -->  sqrt(a)    ; square root\n"
                        " _ ^              : a b  -->  pow(a,b)   ; a^b\n"
                        " _ mod            : b a  -->  int(b) % int(a)\n"
                        " _ vecscalmul     : x1 ... xn alpha n  -->  (x1*alpha) ... (xn*alpha)\n"
                        " _ select         : v0 v1 v2 v3 ... vn-1 n i  -->  vi  \n"
                        " _ length         : the length of the currently processed column.\n"
                        " _ sign           : a  -->  sign(a)  (0 -1 or +1)\n"
                        " _ get            : pos  -->  value_of_stack_at_pos\n"
                        "                    (if pos is negative then it's relative to stacke end\n"
                        "                    ex: -1 get will get the previous element of stack)\n"
                        " _ memput         : a mempos  -->    (a is saved in memory position mempos)\n"
                        " _ memget         : mempos    --> a  (gets 'a' from memory in position mempos)\n"
                        " _ sumabs         : v0 v1 v2 ... vn  -->  sum_i |vi|\n"
                        "                    (no pop, and starts from the beginning of the stack)\n"
                        " _ varproduct     : a0 a1 ... an n+1 b0 b1 ... bm m+1 ... num_vars -> res0 ..."
                        "                    (product of encoded variables)\n"
                        " _ erf            : a     -->  erf(a)    ; the error function erf\n"
    );

//////////////////
// VMatLanguage //
//////////////////
VMatLanguage::VMatLanguage(VMat vmsrc)
{
    setSource(vmsrc);
    build_();
}

// returns oldest modification date of a file containing VPL code, searching recursively every
// file placed after a INCLUDE token
time_t getDateOfCode(const string& codefile)
{
    time_t latest = mtime(codefile);
    string token;
    ifstream in(codefile.c_str());
    if(in.bad())
        PLERROR("Cannot open file : %s",codefile.c_str());

    in >> token;
    while(!in.eof())
    {
        if(token=="INCLUDE")
        {
            in >> token;
            time_t t=getDateOfCode(token);
            if(t>latest)
                latest=t;
        }
        in >> token;
    }
    return latest;
}

map<string, int> VMatLanguage::opcodes;

void
VMatLanguage::build()
{
    inherited::build();
    build_();
}

void
VMatLanguage::build_()
{
    build_opcodes_map();
}

void
VMatLanguage::declareOptions(OptionList &ol)
{
    declareOption(ol, "sourcecode", &VMatLanguage::sourcecode, OptionBase::buildoption,
                  "The VPL sourcecode of the program.");
    declareOption(ol, "outputfieldnames", &VMatLanguage::outputfieldnames, OptionBase::learntoption,
                  "The output fieldnames produced by the program");
    declareOption(ol, "vmsource", &VMatLanguage::vmsource, OptionBase::learntoption,
                  "The VMat that was set by setSource");
    declareOption(ol, "srcfieldnames", &VMatLanguage::srcfieldnames, OptionBase::learntoption,
                  "The fieldnames that were set by setSourceFieldNames");
    declareOption(ol, "program", &VMatLanguage::program, OptionBase::learntoption,
                  "The opcodes of the compiled program");
    declareOption(ol, "mappings", &VMatLanguage::mappings, OptionBase::learntoption,
                  "The mappings of the compiled program");

    inherited::declareOptions(ol);
}

void VMatLanguage::setSource(VMat the_source)
{
    vmsource = the_source;
    // Set field names from the source VMat if it has field names, otherwise
    // set each field name to "".
    TVec<string> fnames = vmsource->fieldNames();
    if (fnames.isEmpty())
        fnames = TVec<string>(vmsource->width());
    setSourceFieldNames(fnames);
    program.resize(0);
}

void VMatLanguage::setSourceFieldNames(TVec<string> the_srcfieldnames)
{ 
    srcfieldnames = the_srcfieldnames; 
    myvec.resize(srcfieldnames.length());
}

//! Make it an empty program by clearing outputfieldnames, program, mappings
void VMatLanguage::clear()
{
    outputfieldnames.resize(0);
    program.resize(0);
    mappings.resize(0);
}

////////////////
// preprocess //
////////////////
void VMatLanguage::preprocess(PStream& in, map<string, string>& defines,
                              string& processed_sourcecode, vector<string>& fieldnames)
{
    string token;
    size_t spos;
    map<string, string>::iterator pos;
    while (in.good())
    {
        in >> token;
        pos = defines.find(token);

        // Are we sitting on a mapping declaration?
        if (token[0] == '{')
        {
            // Skip mapping to avoid brackets conflicts with fieldcopy macro
            // syntax
            processed_sourcecode += token;
            // If the token is only a part of the mapping...
            if (token.find("}") == string::npos)
            {
                char car;
                // Just eat till the end of the mapping
                while ((car = in.get()) != '}' && !in.eof())
                    processed_sourcecode += car;
                processed_sourcecode += "}";
            }
        }
        // Did we find a fieldName declaration? format is either :myField or
        // :myField:a:b
        else if (token[0] == ':')
        {
            if (isBlank(token.substr(1)))
                PLERROR("Found a ':' with no fieldname. Do not put a whitespace after the ':'");
            vector<string> parts = split(token, ":");
            if (parts.size() == 3)
            {
                int a = toint(parts[1]);
                int b = 0;
                // Let the chance for the second interval boundary to be a "DEFINE".
                // This is used with onehot and @myfield.ranges10.nbins
                // ie: @myfield.onehot10 :myfieldonehot:0:@myfield.ranges10.nbins
                if (pl_isnumber(parts[2]))
                    b = toint(parts[2]);
                else
                {
                    if (defines.find(parts[2]) != defines.end())
                        b = toint(defines[parts[2]]);
                    else
                        PLERROR("found a undefined non-numeric boundary in multifield declaration : '%s'",
                                parts[2].c_str());
                }

                for (int i = a; i <= b; i++)
                    fieldnames.push_back(parts[0] + tostring(i));
            }
            else if (parts.size() == 1)
                fieldnames.push_back(token.substr(1));
            else PLERROR("Strange fieldname format (multiple declaration format is :label:0:10");
        }
        // Did we find a fieldcopy macro?
        else if (token[0] == '[')
        {
            if (token[token.size() - 1] != ']') {
                // First read until the brackets are closed.
                string end_of_token;
                in.smartReadUntilNext("]", end_of_token, false, false);
                in.get(); // Read the ']' character.
                token += end_of_token + ']';
            }
	    vector<string> parts = split(token.substr(1, token.size()-2), ":");

            // fieldcopy macro type is [start:end] or [start:end:vpl_code]
            // fields can be refered to as %number or @name
	    if (parts.size() == 2 || parts.size() == 3)
            {
                string astr=parts[0].substr(1);
                string bstr=parts[1].substr(1);
                bool code_to_perform = (parts.size() == 3);
                string performed_code;
                if (code_to_perform)
                    performed_code = parts[2];

                int a=-1;
                int b=-1;

                if (parts[0][0] == '@')
                {
                    for (int i = 0; i < srcfieldnames.length(); i++)
                        if (srcfieldnames[i] == astr)
                        {
                            a = i;
                            break;
                        }
                }
                else if (parts[0][0] == '%')
                    a = toint(parts[0].substr(1));
                else if (parts[0] == "END")
                    // Keyword indicating we go till the end.
                    a = srcfieldnames.length() - 1;
                else
                    PLERROR("fieldcopy macro syntax is : [start:end] EG: [@year:%%6]. 'end' must be after 'start'.. OR [field] to copy a single field (%s)",
                        token.c_str());

                if (parts[1][0] == '@')
                {
                    for (int i = 0; i < srcfieldnames.length(); i++)
                        if (srcfieldnames[i] == bstr)
                        {
                            b = i;
                            break;
                        }
                }
                else if (parts[1][0] == '%')
                    b = toint(parts[1].substr(1));
                else if (parts[1] == "END")
                    // Keyword indicating we go till the end.
                    b = srcfieldnames.length() - 1;
                
                else if (parts[1].substr(0,3) == "END")
                {
                    // Keyword indicating we go till the end.
                    int v = toint(parts[1].substr(3));
                    PLCHECK(v<0);
                    b = srcfieldnames.length() - 1 + v;
                }
                else
                    PLERROR("fieldcopy macro syntax is : [start:end] EG: [@year:%%6]. 'end' must be after 'start'.. OR [field] to copy a single field (%s)",
                            token.c_str());

                if (a > b)
                    PLERROR("In copyfield macro '%s', you have specified a "
                            "start field (%d) that is after the end field "
                            "(%d). E.g.: [%%10:%%5]", token.c_str(), a, b);
                if (a == -1)
                    PLERROR("In copyfield macro, unknown field : '%s'", astr.c_str());
                if (b == -1)
                    PLERROR("In copyfield macro, unknown field : '%s'", astr.c_str());

                for (int i = a; i <= b; i++)
                {
                    processed_sourcecode+=string("%")+tostring(i)+ " ";
                    if (code_to_perform)
                        processed_sourcecode += performed_code + " ";
                    if (i >= srcfieldnames.length())
                        PLERROR("In VMatLanguage::preprocess - Asked field number %d, but there "
                                "are only %d fields available", i, srcfieldnames.length());
                    fieldnames.push_back(srcfieldnames[i]);
                }
            }
            else if (parts.size() == 1)
                // fieldcopy macro type is [field]
            {
                bool ignore_if_missing = false;
                if (parts[0][parts[0].size()-1] == '?') {
                    ignore_if_missing = true;
                    // Remove ending '?' character.
                    parts[0] = parts[0].substr(0, parts[0].size()-1);
                }
                
                string astr = parts[0].substr(1);
                int a = -1;
                if (parts[0][0] == '@')
                {
                    for (int i = 0;i < srcfieldnames.length(); i++)
                        if (srcfieldnames[i] == astr)
                        {
                            a = i;
                            break;
                        }
                }
                else if (parts[0][0] == '%')
                    a = toint(parts[0].substr(1));
                else if (parts[0] == "END")
                    // Keyword indicating we go till the end.
                    a = srcfieldnames.length() - 1;
                else
                    PLERROR("fieldcopy macro syntax is : [start:end] EG: [@year:%%6]. 'end' must be after 'start'.. OR [field] to copy a single field (%s)",
                            token.c_str());

                if (a == -1) {
                    if (!ignore_if_missing)
                        PLERROR("In copyfield macro, unknown field: '%s'", astr.c_str());
                }
                else {
                    processed_sourcecode += string("%") + tostring(a) + " ";
                    if (a >= srcfieldnames.length())
                        PLERROR("In VMatLanguage::preprocess - Asked field number %d, but there "
                                "are only %d fields available", a, srcfieldnames.length());
                    fieldnames.push_back(srcfieldnames[a]);
                }
            }
            else PLERROR("Strange fieldcopy format. e.g : [%%0:%%5]. Found parts '%s'",join(parts," ").c_str());
        }

        // did we find a comment?
        else if (token[0] == '#')
            skipRestOfLine(in);

        // include declaration
        else if (token=="INCLUDE")
        {
            in >> token;
            // Try to be intelligent and find out if the file belongs directly
            // to another .?mat (the case of a stats file for example) and warn
            // if the file is out of date

            // Mhhh.. is this still pertinent? This "stats" and "bins" thing is
            // semi-standard I think
            size_t idx_meta  =  token.find(".metadata");
            size_t idx_stats =  token.find("stats.");
            size_t idx_bins  =  token.find("bins.");
            if (idx_meta != string::npos && (idx_stats != string::npos || idx_bins != string::npos))
            {
                string file = token.substr(0, idx_meta);
                if (getDataSetDate(file) > mtime(token))
                    PLWARNING("File %s seems out of date with parent matrix %s",
                              token.c_str(), file.c_str());
            }

            PStream incfile = openFile(token, PStream::raw_ascii, "r");
            // process recursively this included file
            // **POSSIBLE DRAWBACK : defines done in this file will be used in the next recursion level
            preprocess(incfile, defines, processed_sourcecode, fieldnames);
	
        }
        // define declaration
        else if (token == "DEFINE")
        {
            in >> token;
            string str_buf;
            in.getline(str_buf);
            defines[token.c_str()] = str_buf;
        }
        else if (pos != defines.end())
        {
            // the token is a macro (define) so we process it recursively until
            // it's stable (necessary since the define macro can use defines
            // recursively)
            string oldstr = pos->second;
            string newstr;
            bool unstable = true;
            while (unstable)
            {
                PStream strm = openString(oldstr, PStream::raw_ascii);
                newstr = "";
                preprocess(strm, defines, newstr, fieldnames);
                if (removeblanks(oldstr) == removeblanks(newstr))
                    unstable = false;
                oldstr = newstr;
            }
            processed_sourcecode += newstr + " ";
        }
        // Did we find a reference to a string value of a VMatrix that has
        // overloaded getStringVal(..) e.g.:StrTableVMatrix? In VPL, you can
        // push on the stack the value of a string according to the string map
        // of a particular column e.g. : to push value of string "WBush" from
        // field MostSuspectAmericanPresidents, write
        // @MostSuspectsAmericanPresidents."WBush"
        else if ((token[0]=='@' || token[0]=='%') &&
                 token[token.length()-1]=='"' &&
                 (spos=token.find(".\""))!=string::npos)
        {
            string colname = token.substr(1, spos - 1);
            const string str = token.substr(spos + 2, token.length() - spos - 3);
            // Do we have a named field reference?
            if (token[0]=='@')
            {
                pos = defines.find(string("@") + colname);
                if (pos == defines.end())
                    PLERROR("unknown field : '%s'",colname.c_str());
                colname = pos->second.substr(1);
            }
            const int colnum = toint(colname);
            const real r = vmsource->getStringVal(colnum, str);
            if (is_missing(r))
                PLERROR("String '%s' is not a known string for the field '%s'",
                        str.c_str(), token.c_str());
            processed_sourcecode+=tostring(r)+" ";
        }
        else
            processed_sourcecode += token + " ";
    }
}

void VMatLanguage::generateCode(PStream& processed_sourcecode)
{
    char car;
    string token;
    map<string,int>::iterator pos;
    car = peekAfterSkipBlanks(processed_sourcecode);
    while(!processed_sourcecode.eof())
    {
        if (car=='{')
        {
            int mapnum = mappings.size();
            mappings.resize(mapnum+1);
            mappings[mapnum].read(processed_sourcecode);
            program.append(opcodes["__applymapping"]);
            program.append(mapnum);
        }
        else
        {
            processed_sourcecode>>token;
            if( pl_isnumber(token))
                // assume we have a float
            {
                float zefloat=tofloat(token);
                program.append(opcodes["__insertconstant"]);
                program.append(*(int*)&zefloat);
            }
            else if (token[0]=='%')
            {
                vector<string> parts=split(token,":");
                if (parts.size()==1) // expecting e.g. %10 for column 10
                {
                    program.append(opcodes["__getfieldval"]);
                    int val=toint(token.substr(1));
                    program.append(val);
                }
                else if (parts.size()==2) // expecting e.g. %10-%20 for columns 10 to 20 inclusive
                {
                    program.append(opcodes["__getfieldsrange"]);
                    int a=toint(parts[0].substr(1));
                    int b=toint(parts[1].substr(1));
                    program.append(a);
                    program.append(b);
                }
            }
            else
            {
                pos=opcodes.find(token);
                if(pos!=opcodes.end())
                    program.append(pos->second);
                else PLERROR("Undefined keyword : '%s'",token.c_str());
            }
        }
        car=peekAfterSkipBlanks(processed_sourcecode);
    }
}

void VMatLanguage::generateCode(const string& processed_sourcecode)
{
    PStream in = openString(processed_sourcecode, PStream::raw_ascii);
    generateCode(in);
}

//void declareField(int fieldindex, const string& fieldname, VMField::FieldType fieldtype=VMField::UnknownType)

void VMatLanguage::compileString(const string & code, vector<string>& fieldnames)
{
    PStream in = openString(code, PStream::raw_ascii);
    compileStream(in,fieldnames);
}

void VMatLanguage::compileString(const string & code, TVec<string>& fieldnames)
{
    vector<string> names;
    compileString(code, names);
    fieldnames.resize(int(names.size()));
    for(unsigned int i=0; i< names.size(); i++)
        fieldnames[i] = names[i];
}

void VMatLanguage::compileFile(const PPath& filename, vector<string>& fieldnames)
{
    PStream in = openFile(filename, PStream::raw_ascii, "r");
    compileStream(in,fieldnames);
}

void VMatLanguage::compileStream(PStream & in, vector<string>& fieldnames)
{
    map<string,string> defines;
    string processed_sourcecode;

    program.resize(0);
    mappings.resize(0);

    // first, warn user if a fieldname appears twice or more in the source matrix
    string fname;
    for(int i=0;i<srcfieldnames.length();i++)
    {
        fname = srcfieldnames[i];
        if (!fname.empty())
        {
            fname = string("@") + fname;
            if(defines.find(fname) != defines.end())
                PLERROR("fieldname '%s' is duplicate in processed matrix", fname.c_str());
            defines[fname]=string("%")+tostring(i);
        }
    }

    // the filednames parameter is an output vector in which we put the fieldnames of the final VMat
    fieldnames.clear();
    preprocess(in, defines, processed_sourcecode, fieldnames);
    outputfieldnames.resize(int(fieldnames.size()));
    for(unsigned int k=0; k < fieldnames.size(); k++)
        outputfieldnames[k] = fieldnames[k];

    if(output_preproc)
    {
        perr<<"Preprocessed code:"<<endl<<processed_sourcecode<<endl;
        perr<<"FieldNames : "<<endl<<fieldnames<<endl;
    }
    generateCode(processed_sourcecode);
}



void VMatLanguage::getOutputFieldNamesFromString(const string & code, vector<string>& fieldnames)
{
    PStream in = openString(code, PStream::raw_ascii);
    getOutputFieldNamesFromStream(in, fieldnames);
}

void VMatLanguage::getOutputFieldNamesFromString(const string & code, TVec<string>& fieldnames)
{
    vector<string> names;
    getOutputFieldNamesFromString(code, names);
    fieldnames.resize(int(names.size()));
    for(unsigned int i=0; i< names.size(); i++)
        fieldnames[i] = names[i];
}

void VMatLanguage::getOutputFieldNamesFromStream(PStream &in, vector<string>& fieldnames)
{

    map<string,string> defines;
    string processed_sourcecode;
    fieldnames.clear();
    staticPreprocess(in, defines, processed_sourcecode, fieldnames);
}

void VMatLanguage::staticPreprocess(PStream& in, map<string, string>& defines,
                                    string& processed_sourcecode, vector<string>& fieldnames)
{
    string token;
    map<string,string>::iterator pos;
    while(in.good())
    {
        in >> token;
        pos=defines.find(token);

        // are we sitting on a mapping declaration?
        if(token[0]=='{')
        {
            //skip mapping to avoid brackets conflicts with fieldcopy macro syntax
            char car;
            processed_sourcecode+=token;
            // if the token is only a part of the mapping...
            if(token.find("}")==string::npos)
            {
                // just eat till the end of the mapping
                while((car=in.get())!='}' && !in.eof())
                    processed_sourcecode+=car;
                processed_sourcecode+="}";
            }
        }
        // did we find a fieldName declaration?
        // format is either :myField or :myField:a:b
        else if(token[0]==':')
        {
            if(isBlank(token.substr(1)))
                PLERROR("Found a ':' with no fieldname. Do not put a whitespace after the ':'");
            vector<string> parts=split(token,":");
            if(parts.size()==3)
            {
                int a=toint(parts[1]);
                int b=0;
                // let the chance for the second interval boundary to be a "DEFINE"
                // this is used with onehot and @myfield.ranges10.nbins
                // ie: @myfield.onehot10 :myfieldonehot:0:@myfield.ranges10.nbins
                if(pl_isnumber(parts[2]))
                    b=toint(parts[2]);
                else
                {
                    if(defines.find(parts[2])!=defines.end())
                        b=toint(defines[parts[2]]);
                    else
                        PLERROR("Found an undefined non-numeric boundary in multifield declaration : '%s'",parts[2].c_str());
                }

                for(int i=a;i<=b;i++)
                    fieldnames.push_back(parts[0]+tostring(i));
            }
            else if (parts.size()==1)
                fieldnames.push_back(token.substr(1));
            else PLERROR("Strange fieldname format (multiple declaration format is :label:0:10");
        }
        // Did we find a fieldcopy macro?
        else if(token[0]=='[')
            PLERROR("fieldcopy macro not supported in VMatLanguage::staticPreprocess");
        // did we find a comment?
        else if(token[0]=='#')
            skipRestOfLine(in);
        // include declaration
        else if(token=="INCLUDE")
        {
            in >> token;
            // Try to be intelligent and find out if the file belongs directly to another .?mat (the case of a
            // stats file for example) and warn if the file is out of date

            // Mhhh.. is this still pertinent? This "stats" and "bins" thing is semi-standard I think
            size_t idx_meta  =  token.find(".metadata");
            size_t idx_stats =  token.find("stats.");
            size_t idx_bins  =  token.find("bins.");
            if(idx_meta!=string::npos && (idx_stats!=string::npos || idx_bins!=string::npos))
            {
                string file=token.substr(0,idx_meta);
                if(getDataSetDate(file) > mtime(token))
                    PLWARNING("File %s seems out of date with parent matrix %s",token.c_str(),file.c_str());
            }

            PStream incfile = openFile(token, PStream::raw_ascii, "r");
            // process recursively this included file
            // **POSSIBLE DRAWBACK : defines done in this file will be used in the next recursion level
            staticPreprocess(incfile,defines, processed_sourcecode,fieldnames);
	
        }
        // define declaration
        else if(token=="DEFINE")
        {
            in >> token;
            string str_buf;
            in.getline(str_buf);
            defines[token.c_str()] = str_buf;
        }
        else if(pos!=defines.end())
        {
            // the token is a macro (define) so we process it recursively until it's stable
            // (necessary since the define macro can use defines recursively)
            string oldstr=pos->second,newstr;
            bool unstable=true;
            while(unstable)
            {
                PStream strm = openString(oldstr, PStream::raw_ascii);
                newstr="";
                staticPreprocess(strm,defines,newstr,fieldnames);
                if(removeblanks(oldstr)==removeblanks(newstr))
                    unstable=false;
                oldstr=newstr;
            }
            processed_sourcecode+=newstr + " ";
        }
        // did we find a reference to a string value of a VMatrix that has overloaded getStringVal(..) e.g.:StrTableVMatrix
        // In VPL, you can push on the stack the value of a string according to the string map of a particular column
        // e.g. : to push value of string "WBush" from field MostSuspectAmericanPresidents, write @MostSuspectsAmericanPresidents."WBush"
        else if ((token[0]=='@' || token[0]=='%') && token[token.length()-1]=='"' && token.find(".\"")!=string::npos)
            PLERROR("string values not supported in VMatLanguage::staticPreprocess");
        else processed_sourcecode+=token + " ";
    }
}

//! builds the map if it does not already exist
void VMatLanguage::build_opcodes_map()
{
    if(opcodes.empty())
    {
        opcodes["__insertconstant"] = 0; // followed by a floating point number (4 bytes, just like int)
        opcodes["__getfieldval"] = 1; // followed by field#
        opcodes["__applymapping"] = 2; // followed by mapping#
        opcodes["pop"] = 3;
        opcodes["dup"] = 4;
        opcodes["exch"] = 5;
        opcodes["onehot"] = 6;
        opcodes["+"]  = 7;
        opcodes["-"] = 8;
        opcodes["*"] = 9;
        opcodes["/"] = 10;
        opcodes["=="] = 11;
        opcodes["!="] = 12;
        opcodes[">"] = 13;
        opcodes[">="] = 14;
        opcodes["<"] = 15;
        opcodes["<="] = 16;
        opcodes["and"] = 17;
        opcodes["or"] = 18;
        opcodes["not"] = 19;
        opcodes["ifelse"] = 20;
        opcodes["fabs"] = 21;
        opcodes["rint"] = 22;
        opcodes["floor"] = 23;
        opcodes["ceil"] = 24;
        opcodes["log"] = 25;
        opcodes["exp"] = 26;
        opcodes["rowindex"] = 27; // pushes the rownum on the stack
        opcodes["isnan"] = 28;
        opcodes["year"] = 29; // from format CYYMMDD -> YYYY
        opcodes["month"] = 30; // CYYMMDD -> MM
        opcodes["day"] = 31;  // CYYMMDD -> DD
        opcodes["daydiff"] = 32; // nb. days
        opcodes["monthdiff"] = 33; // continuous: nb. days / (365.25/12)
        opcodes["yeardiff"] = 34; // continuous: nb. days / 365.25
        opcodes["year_month_day"] = 35; // CYYMMDD -> YYYY MM DD
        opcodes["todate"] = 36; // YYYY MM DD -> CYYMMDD
        opcodes["dayofweek"] = 37; // from CYYMMDD -> [0..6] (0=monday; 6=sunday)
        opcodes["today"] = 38; // today's date CYYMMDD
        opcodes["date2julian"] = 39; // CYYMMDD -> nb. days
        opcodes["julian2date"] = 40; // nb. days -> CYYMMDD
        opcodes["min"] = 41; // a b -> (a<b? a : b)
        opcodes["max"] = 42; // a b -> (a<b? b : a)
        opcodes["sqrt"] = 43;
        opcodes["^"] = 44;
        opcodes["mod"] = 45;
        opcodes["vecscalmul"] = 46; // x1 ... xn alpha n --> (x1*alpha) ... (xn*alpha)
        opcodes["__getfieldsrange"] = 47; // %N:%M pushes field %N up to %M. M must be >= N.
        opcodes["select"] = 48; // v0 v1 v2 v3 ... vn-1 n i --> vi
        opcodes["length"] = 49; // the length of the currently processed column.
        opcodes["sign"]   = 50; // a --> sign(a)  (0 -1 or +1)
        opcodes["get"]    = 51; // pos --> value_of_stack_at_pos (if pos is negative then it's relative to stacke end ex: -1 get will get the previous element of stack)
        opcodes["memput"] = 52; // a mempos -->    ( a is saved in memory position mempos)
        opcodes["memget"] = 53; // mempos --> val  ( gets val from memory in position mempos)
        opcodes["neg"]    = 54; // a --> -a
        opcodes["missing"] = 55;  // a missing value
        opcodes["sumabs"] = 56;  // v0 v1 v2 ... vn --> sum_i |vi| (no pop, and starts from the beginning of the stack)
        opcodes["weeknumber"] = 57;  // CYYMMDD -> week number in the year between 0 and 52 incl. (ISO 8601 minus 1)
        opcodes["dayofyear"] = 58;  // CYYMMDD ->  number of days since january 1 of year CYY
        opcodes["nextincal"] = 59;  // cal# JDate -> next jdate on or after given jdate in global calendar cal#
        opcodes["previncal"] = 60;  // cal# JDate -> previous jdate on or before given jdate in global calendar cal#
        opcodes["gausshot"]  = 61;  // index nclasses sigma --> smooth one-hot
        opcodes["sigmoid"]  = 62;   // a -> sigmoid(a)
        opcodes["cos"]  = 63;   // a -> cos(a)
        opcodes["varproduct"] = 64; // a0 a1 ... an n+1 b0 b1 ... bm m+1 ... num_vars -> res0 ... (product of encoded variables)
        opcodes["thermometer"] = 65; // index nclasses -> thermometer encoding
        opcodes["erf"] = 66;
    }
}

void VMatLanguage::run(const Vec& srcvec, const Vec& result, int rowindex) const
{
    if (program.length() == 0 && sourcecode != "")
    {
        TVec<string> outnames;
        const_cast<VMatLanguage*>(this)->compileString(sourcecode, outnames);
    }
    
    
    if (srcvec.length()!=srcfieldnames.length())
        PLERROR("In VMatLanguage::run, srcvec should have length %d, not %d.",srcfieldnames.length(),srcvec.length());
    
    pstack.resize(0);
    TVec<int>::iterator pptr = program.begin();
    const TVec<int>::iterator pptrend = program.end();
    real* pfieldvalues = srcvec.data();
    real a,b,c;    

    while (pptr != pptrend)
    {
        const int op = *pptr++;
        switch(op)
        {
        case 0: // insertconstant
            pstack.push(*((float*)pptr++));
            break;
        case 1: // getfieldval
            // XXX Question: why is the next PLERROR line commented? Is if made
            // obsolete by another bound check earlier in the code? Or is it
            // temporarily disabled? If the former, please *delete* the line,
            // together with this comment. If the latter, please reenable the
            // line or replace this comment by one explaining what needs to be
            // done before the line can be reenabled. (Bound checking in
            // general is a good idea...)
            //
            //if(*pptr > fieldvalues.width()) PLERROR("Tried to acces an out of bound field in VPL code");
            pstack.push(pfieldvalues[*pptr++]);
            break;
        case 2: // applymapping
            pstack.push(mappings[*pptr++].map(pstack.pop()));
            break;
        case 3: // pop
            pstack.pop();
            break;
        case 4: // dup
            pstack.push(pstack.top());
            break;
        case 5: // exch
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(b);
            pstack.push(a);
            break;
        case 6: // onehot
        {
            const int nclasses = int(pstack.pop());
            const int index = int(pstack.pop());
            for (int i = 0; i < nclasses; i++)
                pstack.push(i == index ? 1 : 0);
            break;
        }
        case 7: // +
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(a+b);
            break;
        case 8: // -
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(a-b);
            break;
        case 9: // *
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(a*b);
            break;
        case 10: // /
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(a/b);
            break;
        case 11: // ==
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(fast_exact_is_equal((float)a, (float)b) ?1 :0);
            break;
        case 12: // !=
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(!fast_exact_is_equal((float)a, (float)b) ?1 :0);
            break;
        case 13: // >
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(((float)a>(float)b) ?1 :0);
            break;
        case 14: // >=
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(((float)a>=(float)b) ?1 :0);
            break;
        case 15: // <
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(((float)a<(float)b) ?1 :0);
            break;
        case 16: // <=
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(((float)a<=(float)b) ?1 :0);
            break;
        case 17: // and
            b = pstack.pop();
            a = pstack.pop();
            pstack.push((!fast_exact_is_equal(a, 0) &&
                         !fast_exact_is_equal(b, 0)) ?1 :0);
            break;
        case 18: // or
            b = pstack.pop();
            a = pstack.pop();
            pstack.push((!fast_exact_is_equal(a, 0) ||
                         !fast_exact_is_equal(b, 0)) ?1 :0);
            break;
        case 19: // not
            pstack.push(fast_exact_is_equal(pstack.pop(), 0) ?1 :0);
            break;
        case 20: // ifelse
            c = pstack.pop();
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(!fast_exact_is_equal(a, 0)?b:c);
            break;
        case 21: // fabs
            pstack.push(fabs(pstack.pop()));
            break;
        case 22: // rint
            pstack.push(rint(pstack.pop()));
            break;
        case 23: // floor
            pstack.push(floor(pstack.pop()));
            break;
        case 24: // ceil
            pstack.push(ceil(pstack.pop()));
            break;
        case 25: // log
            pstack.push(pl_log(pstack.pop()));
            break;
        case 26: // exp
            pstack.push(exp(pstack.pop()));
            break;
        case 27: // rowindex
            pstack.push(real(rowindex));
            break;
        case 28: // isnan
            pstack.push(isnan(pstack.pop())?1:0);
            break;
        case 29: //year
            pstack.push(float_to_date(pstack.pop()).year);
            break;
        case 30: //month
            pstack.push(float_to_date(pstack.pop()).month);
            break;
        case 31: //day
            pstack.push(float_to_date(pstack.pop()).day);
            break;
        case 32: //daydiff
            b= pstack.pop();
            a= pstack.pop();
            if (!isnan(a) && !isnan(b)) {
                pstack.push(float_to_date(a)-float_to_date(b));
            }
            else {
                pstack.push(MISSING_VALUE);
            }
            break;
        case 33: //monthdiff
            b= pstack.pop();
            a= pstack.pop();
            pstack.push((float_to_date(a)-float_to_date(b))*(12.0/365.25));
            break;
        case 34: //yeardiff
            b= pstack.pop();
            a= pstack.pop();
            if (is_missing(a) || is_missing(b))
                pstack.push(MISSING_VALUE);
            else
                pstack.push((float_to_date(a)-float_to_date(b))/365.25);
            break;
        case 35: //year_month_day
        {
            PDate d(float_to_date(pstack.pop()));
            pstack.push(d.year);
            pstack.push(d.month);
            pstack.push(d.day);
            break;
        }
        case 36: //todate
            c = pstack.pop();
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(date_to_float(PDate((int)a, (int)b, (int)c)));
            break;
        case 37: //dayofweek
            pstack.push(float_to_date(pstack.pop()).dayOfWeek());
            break;
        case 38: //today
            pstack.push(date_to_float(PDate::today()));
            break;
        case 39: //date2julian
            pstack.push(float_to_date(pstack.pop()).toJulianDay());
            break;
        case 40: //julian2date
            pstack.push(date_to_float(PDate((int)pstack.pop())));
            break;
        case 41: //min
            a= pstack.pop();
            b= pstack.pop();
            pstack.push(a<b? a : b);
            break;
        case 42: //max
            a= pstack.pop();
            b= pstack.pop();
            pstack.push(a<b? b : a);
            break;
        case 43: // sqrt
            pstack.push(sqrt(pstack.pop()));
            break;
        case 44: // ^
            b= pstack.pop();
            a= pstack.pop();
            pstack.push(pow(a,b));
            break;
        case 45: // mod
            a= pstack.pop();
            b= pstack.pop();
            pstack.push((int)b % (int)a);
            break;
        case 46: // vecscalmul
        {
            a = pstack.pop(); // n
            b = pstack.pop(); // alpha
            int start = int(pstack.length()-a);
            for (int i=0;i<a;i++)
                pstack[start+i] *= b;
            break;
        }
        case 47: // __getfieldsrange         %M:%N       pushes fields %N to %M inclusively on the stack
        {
            int M = *pptr++;
            int N = *pptr++;
            for (int i=M;i<=N;i++)
                pstack.push(pfieldvalues[i]);
            break;
        }
        case 48: // select:    v0 v1 v2 v3 ... vn-1 n i --> vi
        {
            int i = (int)pstack.pop();
            int n = (int)pstack.pop();
            a = MISSING_VALUE;
            while(--n>=0)
            {
                if(n==i)
                    a = pstack.pop();
                else
                    pstack.pop();
            }
            pstack.push(a);
            break;
        }
        case 49: // length
        {
            pstack.push(srcvec.length());
            break;
        }
        case 50: // sign
        {
            a = pstack.pop();
            if(a>0)
                pstack.push(1.);
            else if(a<0)
                pstack.push(-1.);
            else
                pstack.push(0.);
            break;
        }
        case 51: // get
        {
            const int i = int(pstack.pop());
            if (i >= 0)
                pstack.push(pstack[i]);
            else
                pstack.push(pstack.length() + i);
            break;
        }
        case 52: // memput
        {
            const int i = int(pstack.pop());
            a = pstack.pop();
            if (mem.size()<i+1)
                mem.resize(i+1);
            mem[i] = a;
            break;
        }
        case 53: // memget
        {
            const int i = int(pstack.pop());
            pstack.push(mem[i]);
            break;
        }
        case 54: // neg
            pstack.push(-pstack.pop());
            break;
        case 55: // missing
            pstack.push(MISSING_VALUE);
            break;
        case 56: // sumabs
        {
            real sumabs = 0;
            for (int i = 0; i < pstack.length(); i++)
                sumabs += fabs(pstack[i]);
            pstack.push(sumabs);
            break;
        }
        case 57: // weeknumber
            pstack.push(float_to_date(pstack.pop()).weekNumber());
            break;
        case 58: // dayofyear
            pstack.push(float_to_date(pstack.pop()).dayOfYear());
            break;
        case 59: // nextincal
        {
            const string cal_name = tostring(pstack.pop());
            PDate d = float_to_date(pstack.pop());
            JTime date = d.toJulianDay();
            const Calendar* cal = Calendar::getGlobalCalendar(cal_name);
            if (cal)
            {
                JTime next = cal->calendarTimeOnOrAfter(date);
                if(next<0)
                    PLERROR("VMatLanguage :: attempting 'nextincal' for date '%s' on "
                            "calendar '%s' but no next-date found",
                            d.info().c_str(), cal_name.c_str());
                else
                    pstack.push(date_to_float(PDate((int)next)));
            }
            else
                PLERROR("Global calendar '%s' does not exist", cal_name.c_str());
            break;
        }
        case 60: // previncal
        {
            const string cal_name = tostring(pstack.pop());
            PDate d = float_to_date(pstack.pop());
            JTime date = d.toJulianDay();
            const Calendar* cal = Calendar::getGlobalCalendar(cal_name);
            if (cal)
            {
                JTime next = cal->calendarTimeOnOrBefore(date);
                if(next<0)
                    PLERROR("VMatLanguage :: attempting 'previncal' for date '%s' on "
                            "calendar '%s' but no previous-date found",
                            d.info().c_str(), cal_name.c_str());
                else
                    pstack.push(date_to_float(PDate((int)next)));
            }
            else
                PLERROR("Global calendar '%s' does not exist", cal_name.c_str());
            break;
        }
        case 61: // gausshot
        {
            const real sigma = pstack.pop();
            const int nclasses = int(pstack.pop());
            const int index = int(pstack.pop());
            for (int i = 0; i < nclasses; i++) {
                const real diff_index = i - index;
                pstack.push(exp(- diff_index*diff_index / sigma));
            }
            break;
        }
        case 62: // sigmoid
        {
            pstack.push(sigmoid(pstack.pop()));
            break;
        }
        case 63: // cos
        {
            pstack.push(cos(pstack.pop()));
            break;
        }
        case 64: // varproduct
        {
            const real num_vars_real = pstack.pop();
            if (num_vars_real <= 0)
                PLERROR("VMatLanguage: varproduct: num_vars must be a strictly positive number.");
            const int num_vars = (int)num_vars_real;
            if (num_vars != num_vars_real)
                PLERROR("VMatLanguage: varproduct: num_vars must be an integer.");
            TVec<Vec> vars(num_vars);
            int result_size = 1;

            // Store all the encoded variables on the stack into "vars"
            for (int i = num_vars - 1; i >= 0; i--) {
                real var_size_real = pstack.pop();
                if (var_size_real <= 0)
                    PLERROR("VMatLanguage: varproduct: var_size must be a strictly positive number.");
                int var_size = (int)var_size_real;
                if (var_size != var_size_real)
                    PLERROR("VMatLanguage: varproduct: var_size must be an integer.");

                result_size *= var_size;
                vars[i].resize(var_size);
                for (int j = var_size - 1; j >= 0 ; j--)
                    vars[i][j] = pstack.pop();
            }

            Vec res(result_size, 1);
            int length_of_run = 1;
            int gap_between_runs = 1;

            // Accumulate variable product into "res" variable.
            for (int var_index = num_vars - 1; var_index >= 0; var_index--) {
                const int current_var_size = vars[var_index].size();
                gap_between_runs *= current_var_size;
                int start_dest_index = 0;

                for (int var_data_index = 0; var_data_index < current_var_size; var_data_index++) {
                    int dest_index = start_dest_index;
                    while (dest_index < result_size) {
                        int start_of_run = dest_index;
                        while (dest_index - start_of_run < length_of_run) {
                            res[dest_index++] *= vars[var_index][var_data_index];
                        }
                        dest_index += gap_between_runs - 1;
                    }
                    start_dest_index += length_of_run;
                }
                length_of_run = gap_between_runs;
            }

            // Put the result onto the stack
            for (int i = 0; i < result_size; i++)
                pstack.push(res[i]);
            break;
        }
        case 65: // thermometer
        {
            const int nclasses = int(pstack.pop());
            const int index = int(pstack.pop());
            for (int i = 0; i < nclasses; i++)
                pstack.push(i > index ? 1 : 0);
            
            break;
        }        
        case 66: // erf
            pstack.push(pl_erf(pstack.pop()));
            break;
        default:
            PLASSERT_MSG(false, "BUG IN VMatLanguage::run while running program: unexpected opcode: " +
                         tostring(op));
        }
    }

    if (pstack.length() > result.length())
        PLERROR("Parsing VMatLanguage: left with %d too many items on the stack!",
                pstack.length()-result.length());
    if (pstack.length() < result.length())
        PLERROR("Parsing VMatLanguage: left with %d missing items on the stack!",
                result.length()-pstack.length());

    pstack >> result;
}

void VMatLanguage::run(int rowindex, const Vec& result) const
{
    vmsource->getRow(rowindex,myvec);
    run(myvec, result, rowindex);
}

void VMatLanguage::setMemory(const Vec& new_mem) const
{
    mem.resize(new_mem.size());
    mem << new_mem;
}

void VMatLanguage::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(vmsource, copies);
    deepCopyField(srcfieldnames, copies);
    deepCopyField(outputfieldnames, copies);
    deepCopyField(program, copies);
    deepCopyField(mappings, copies);
    deepCopyField(pstack, copies);
    deepCopyField(myvec, copies);
    deepCopyField(mem, copies);
}



void  PreprocessingVMatrix::getNewRow(int i, const Vec& v) const
{
    program->run(i,v);
}

PLEARN_IMPLEMENT_OBJECT(PreprocessingVMatrix, "DEPRECATED: use ProcessingVMatrix instead", "NO HELP");


PreprocessingVMatrix::PreprocessingVMatrix(VMat the_source, const string& program_string)
    : source(the_source), program(new VMatLanguage(the_source))
{
    program->compileString(program_string,fieldnames);
    build();
}

void
PreprocessingVMatrix::build()
{
    inherited::build();
    build_();
}

void
PreprocessingVMatrix::build_()
{
    if (source) {
        fieldinfos.resize((int)fieldnames.size());
        for(unsigned int j=0; j<fieldnames.size(); j++)
            fieldinfos[j] = VMField(fieldnames[j]);

        sourcevec.resize(source->width());
        width_ = (int)fieldnames.size();
        length_ = source.length();
    }
}

void
PreprocessingVMatrix::declareOptions(OptionList &ol)
{
    inherited::declareOptions(ol);
    declareOption(ol, "source", &PreprocessingVMatrix::source, OptionBase::buildoption, "");
    declareOption(ol, "program", &PreprocessingVMatrix::program, OptionBase::buildoption, "");
    declareOption(ol, "fieldnames", &PreprocessingVMatrix::fieldnames, OptionBase::buildoption, "");
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
