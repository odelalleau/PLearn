// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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

#include "RealMapping.h"
#include <algorithm>
#include <plearn/io/fileutils.h>    //!< For peekAfterSkipBlanks.
//#include <sstream>
#include "stringutils.h"  //!< For removeblanks.
#include <plearn/io/PStream.h>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    RealMapping,
    "Mapping between ranges and values.",
    "RealMapping is used as a component of binning operations.  It divides the\n"
    "real line in a set of ranges, and associates each range with a single\n"
    "(usually integer) value.  The ranges are specified as follows:\n"
    "\n"
    "- ]low,high[ : both endpoints are EXCLUDED\n"
    "- [low,high[ : lower endpoint INCLUDED, upper endpoint EXCLUDED\n"
    "- [low,high] : both endpoints are INCLUDED\n"
    "- ]low,high] : lower endpoint EXCLUDED, upper endpoint INCLUDED\n"
    );

////////
// << //
////////
PStream& operator<<(PStream& out, const RealRange& x) 
{ 
    out.put(x.leftbracket);
    out << x.low;
    switch(out.outmode) {
    case PStream::raw_ascii:
    case PStream::pretty_ascii:
        out << ' ';
        break;
    default:
        // Nothing to add.
        break;
    }
    out << x.high;
    out.put(x.rightbracket);
    return out;
}

////////
// >> //
////////
PStream& operator>>(PStream& in, RealRange &x) 
{ 
    in.skipBlanksAndCommentsAndSeparators();
    x.leftbracket = in.get();
    in.skipBlanksAndComments();
    in >> x.low;
    in.skipBlanksAndComments();
    in >> x.high;
    in.skipBlanksAndComments();
    x.rightbracket = in.get();
    x.checkbrackets();
    return in;
}

string RealRange::getString() const 
{
    ostringstream s;
    s << leftbracket << low << ' ' << high << rightbracket;
    return s.str();
//    
//    char s[50];
//    sprintf(s,"%c%f %f%c",leftbracket,low,high,rightbracket);
//    return s;
}  


bool RealRange::contains(real val) const
{ return (val>=low) && (val<=high)
                    && (!fast_exact_is_equal(val, low)  || leftbracket=='[')
                    && (!fast_exact_is_equal(val, high) || rightbracket==']');
}

bool RealRange::operator<(real x) const
{ return high < x || fast_exact_is_equal(high, x) && rightbracket == '['; }

bool RealRange::operator>(real x) const
{ return low > x || fast_exact_is_equal(low, x) && leftbracket == ']'; }

bool RealRange::operator<(const RealRange& x) const
{ return high < x.low || (fast_exact_is_equal(high, x.low) && (rightbracket=='[' || x.leftbracket==']')); }

bool RealRange::operator>(const RealRange& x) const
{ return low > x.high || (fast_exact_is_equal(low, x.high) && (leftbracket==']' || x.rightbracket=='[')); }

bool RealRange::operator==(const RealRange& rr) const
{
    return (fast_exact_is_equal(rr.low, low)  && 
            fast_exact_is_equal(rr.high,high) && 
            rr.leftbracket == leftbracket     && 
            rr.rightbracket == rightbracket);
}


// ==============================================

bool RealMapping::operator==(const RealMapping& rm) const
{
    return (rm.mapping == mapping && 
            is_equal(rm.missing_mapsto, missing_mapsto, 1, 0, 0) &&
            rm.keep_other_as_is == keep_other_as_is &&
            is_equal(rm.other_mapsto, other_mapsto, 1, 0, 0));
}

bool operator<(RealMapping::single_mapping_t a, RealMapping::single_mapping_t b)
{
    return a.first<b.first;
}

real RealMapping::maxMappedToValue()
{
    real max = -9999999;
    mapping_t::iterator it = mapping.begin();
    mapping_t::iterator itend = mapping.end();
    for(; it!=itend; ++it)
        if(it->second > max)
            max = it->second;
    return max;
}    

void RealMapping::declareOptions(OptionList& ol) 
{
    declareOption(
        ol, "mapping", &RealMapping::mapping, OptionBase::buildoption,
        "The mapping");

    declareOption(
        ol, "missing_mapsto", &RealMapping::missing_mapsto, OptionBase::buildoption,
        "Value to which to map missing values (can be missing value)");

    declareOption(
        ol, "keep_other_as_is", &RealMapping::keep_other_as_is,
        OptionBase::buildoption,
        "If true, values not in mapping are left as is, otherwise they're\n"
        "mapped to 'other_mapsto' (default=True)");

    declareOption(
        ol, "other_mapsto", &RealMapping::other_mapsto, OptionBase::buildoption,
        "Value to which to map values not in mapping, if 'keep_other_as_is' is\n"
        "false");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RealMapping::buildOrderedMapping()
{ 
    o_mapping.resize(0);
    for(iterator it = begin();it!=end();++it)
        o_mapping.push_back(*it);
    sort(o_mapping.begin(),o_mapping.end());
}

real RealMapping::map(real val) const
{
    if(is_missing(val))
        return missing_mapsto;
    mapping_t::const_iterator it= mapping.lower_bound(RealRange('[',val,val,']'));
    if(it != mapping.end() && it->first.contains(val))
        return it->second;
    if(keep_other_as_is)
        return val;
    else
        return other_mapsto;      
}


void RealMapping::addMapping(const RealRange& range, real val)
{
    //PStream perr(&cerr);
    //perr << "BEFORE " << mapping << endl;
    //perr << "ADDING " << range << "->" << val << endl;
    mapping[range]= val; 
    //perr << "AFTER " << mapping << endl;
    // cout<<"adding :"<<range<<" = "<<val<<endl;
    // cout<<"MAPPING:"<<*this<<endl;
}
//    { mapping.push_back(make_pair(range,val)); }


int RealMapping::binnumber(real val) const
{
    if(is_missing(val))
        return -2;
    mapping_t::const_iterator it = mapping.begin();
    mapping_t::const_iterator itend = mapping.end();
    for(int num=0; it!=itend; ++it,num++)
        if(it->first.contains(val))
            return num;
    return -1;      
}


void RealMapping::transform(const Vec& v) const
{
    real* p = v.data();
    int l = v.length();
    while(l--)
    {
        *p = map(*p);
        ++p;
    }
}

//////////////
// newwrite //
//////////////
void RealMapping::newwrite(PStream& out) const {
    switch (out.outmode) {
    case PStream::raw_ascii:
        out << "{ ";
        for(mapping_t::const_iterator it= mapping.begin();
            it != mapping.end(); ++it)
            out << it->first << " -> " << it->second << "; ";
        if(!is_missing(missing_mapsto))
            out << "MISSING -> " << missing_mapsto << "; " << endl;
        if(!keep_other_as_is)
            out << "OTHER -> " << other_mapsto;
        out << "} ";
        break;
    default:
        inherited::newwrite(out);
    }
}

// TODO  The following are probably deprecated.

void RealMapping::print(ostream& out) const
{ 
    out << "{ ";
    out.precision(17);
    for(mapping_t::const_iterator it= mapping.begin(); 
	it != mapping.end(); ++it)
        out << it->first << " -> " << it->second << "; ";
    if(!is_missing(missing_mapsto))
        out << "MISSING -> " << missing_mapsto << "; " << endl;
    if(!keep_other_as_is)
        out << "OTHER -> " << other_mapsto;
    out << "} ";
}

void RealMapping::write(ostream& out) const
{ 
    writeHeader(out,"RealMapping",0);
    print(out);
    writeFooter(out,"RealMapping");
}

void RealMapping::read(PStream& in)
{
    clear();//mapping.resize(0);
    missing_mapsto = MISSING_VALUE;
    keep_other_as_is = true;

    in >> ws;//skipBlanks(in);
    char c=in.get();
    bool uses_headers = false;
    
    if(c=='<')
    {
        in.unget();
        int version = readHeader(in,"RealMapping");
        if(version!=0)
            PLERROR("In RealMapping::read reading of version #%d not implemented",version);
        uses_headers = true;
        in >> ws;//skipBlanks(in);
        c = in.get();
    }

    if(c!='{')
        PLERROR("In RealMapping::read, expected a {, found a %c",c);

    while((c=peekAfterSkipBlanks(in))!='}' && !in.eof())
    {
        RealRange r;
            
        if(c==';'){c=in.get();}
        else if(c=='[' || c==']')
        {
            r.read(in);
            in >> ws;//skipBlanks(in);
            if(in.get()!='-' || in.get()!='>')
                PLERROR("Expecting -> after range specification ( range syntax example : ]100 200] -> 10 )");
            real val;
            in >> val;
            addMapping(r,val);
        }
        else if (isdigit(c) || c == '-' || c == '.')
        {
            real val0, val;
            in >> val0 >> ws;
            r= RealRange('[', val0, val0, ']');
            char c0= in.get();
            char c1= in.get();
            if(c0!='-' || c1!='>')
                PLERROR("Expecting '->' in mapping (syntax example : '100 -> 10') got '%c%c' instead",c0,c1);
            in >> val;
            addMapping(r,val);	    
        }
        else 
        {
            string str;
            in.getline(str, '-');
            str=upperstring(removeblanks(str));
            if(str=="MISSING")
            {
                if(in.get()!='>')
                    PLERROR("Expecting -> after MISSING");
                real val;
                in >> val;
                setMappingForMissing(val);
            }
            else if (str=="OTHER")
            {
                if(in.get()!='>')
                    PLERROR("Expecting -> after OTHER");
                real val;
                in >> val;
                setMappingForOther(val);
            }
            else PLERROR("Unexpected string in mapping definition : %s",str.c_str());
        }
    }
    in.get();
    
    if(uses_headers)
    {
        in >> ws;//skipBlanks(in);
        readFooter(in,"RealMapping");        
    }
}


Vec RealMapping::getCutPoints() const
{
    Vec v(length()+1);
    mapping_t::const_iterator it= mapping.begin();
    v[0]= it->first.low;
    for(int i= 1; it != mapping.end(); ++it, ++i)
        v[i]= it->first.high;
    return v;
}

// checks that the values which the 'n' ranges are mapped to span from 0..n-1 and that these values are integers
bool RealMapping::checkConsistency()
{
    mapping_t::const_iterator it= mapping.begin();
    int max = 0;
    for(;it != mapping.end(); ++it)
    {
        if(it->second <0 || it->second - (int)it->second > 0)
            return false;
        if(max < it->second)
            max = (int)it->second;
    }
    TVec<int> v(max+1,0);
    for(it=mapping.begin();it != mapping.end(); ++it)
    {
        if(v[(int)it->second] != 0)
            return false;
        v[(int)it->second]=1;
    }
    for(int i=0;i<max;i++)
        if(v[i]==0)
            return false;
    return true;  
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
