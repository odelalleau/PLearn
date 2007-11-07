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
 * AUTHOR: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file PLearn/plearn/db/AutoSDBVMatrix.h */

#ifndef AutoSDBVMatrix_INC
#define AutoSDBVMatrix_INC

//#include "general.h"
#include "SimpleDB.h"
#include <plearn/vmat/VMat.h>
#include <plearn/vmat/RowBufferedVMatrix.h>
#include <plearn/io/openFile.h>
//#include <map>

namespace PLearn {
using namespace std;


//class StringFieldMapping; //fwd decl.

class StringFieldMapping
{
public:

    real dft_val;
    mutable hash_map<string, real> mapping;

    StringFieldMapping()
    {}

    StringFieldMapping(string filename_, real dft_val_= MISSING_VALUE)
        :dft_val(dft_val_), mapping()
    {
        /*ifstream*/ PStream f = openFile(filename_.c_str(),PStream::plearn_ascii);
        while(f)
        {
            string s;
            f >> s;
            //PLearn::read(f, s);
            real val;
            f >> val;
            //PLearn::read(f, val);
            if(f) mapping[s]= val;
        }
    }

    real operator[](const string& s) const
    {
        if(mapping.end() == mapping.find(s))
            return dft_val;
        return mapping[s]; 
    }
 
};


class NumToStringMapping
{
public:

    string filename;
    string dft_val;
    mutable hash_map<real, string> mapping;
    mutable bool loaded;

    NumToStringMapping()
        :loaded(false)
    {}

    NumToStringMapping(string filename_, const string& dft_val_= "")
        :filename(filename_), dft_val(dft_val_), mapping(), loaded(false)
    {}

    void read(istream& in) const
    {
        while(in)
        {
            string s;
            PLearn::read(in, s);
            real val;	    
            PLearn::read(in, val);
            if(in) mapping[val]= s;
        }
        loaded= true;
    }

    void load(string filename_= "")
    {
        if(filename_ != "")
            filename= filename_;
        ifstream f(filename.c_str());
        read(f);
    }

    void load() const
    {
        ifstream f(filename.c_str());
        read(f);
    }

    const string& operator[](real x) const
    {
        if(!loaded) load();
        if(mapping.end() == mapping.find(x))
            return dft_val;
        return mapping[x]; 
    }
 
};


//! A VMatrix view of a SimpleDB: columns whose type is string are removed from the view, all others are converted to real (characters to their ascii code, and dates to the float date format: 990324)

class AutoSDBVMatrix: public RowBufferedVMatrix
{
public:
    AutoSDBVMatrix(const string& dbname);


    //! Returns the number of string fields (these will not be used in getRow!)
    inline int nstrings() { return sdb_.width() - width(); }
  
    //! gets mappings for each string field
    void getMappings();

    //! returns the string associated with value val 
    //! for field# col. Or returns "" if no string is associated.
    virtual string getValString(int col, real val) const
    {
        hash_map<string, NumToStringMapping>::const_iterator it= 
            num2string_map.find(fieldName(col));
        if(it != num2string_map.end())
            return it->second[val];
        return "";
    }

protected:

    virtual void getNewRow(int i, const Vec& v) const;  

    SDB sdb_;
    mutable Row row_;
    hash_map<string, StringFieldMapping> string_field_map;
    hash_map<string, NumToStringMapping> num2string_map;
};





} // end of namespace PLearn

#endif


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
