// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file PLearn/plearn/io/pl_io_deprecated.cc */

#include "pl_io_deprecated.h"
#include <plearn/base/stringutils.h>
#include "pl_streambuf.h"

namespace PLearn {
using namespace std;


void writeHeader(ostream& out, const string& classname, int version)
{ out << "<" << classname << ":" << version << ">\n"; }

void writeFooter(ostream& out, const string& classname)
{ out << "</" << classname << ">\n"; }

int readHeader(PStream& in, const string& classname)
{
    string header;
//#if defined(_MINGW_) || defined(WIN32)
//  in.tellg();   // Don't remove this line under MinGW, it apparently does nothing
//                // but if it's not there, it won't work (Hint: Microsoft conspiracy)
//#endif
    in >> header;
    int headerlen = (int)header.length();
    in.get(); // consume newline
    int classnamelen = (int)classname.length();
    if (   headerlen<classnamelen+2 
           || header[0]!='<' || header.substr(1,classnamelen)!=classname
           || (header[1+classnamelen]!='>' && header[1+classnamelen]!=':') )
        PLERROR("In Object::readHeader WRONG HEADER: %s (SHOULD BE {s:version>)",header.c_str(),classname.c_str());
    if (header[1+classnamelen]==':')
        return toint(header.substr(2+classnamelen, headerlen-classnamelen-2));
    else return 0;
}

void readFooter(PStream& in, const string& classname)
{
    string footer;
    in >> footer;
    string correctfooter = string("</")+classname+">";
    if(footer != correctfooter)
        PLERROR("In Object::readFooter WRONG FOOTER: %s (SHOULD BE %s)",footer.c_str(),correctfooter.c_str());
    in.get(); // consume newline
}


void writeFieldName(ostream& out, const string& fieldname)
{ out << fieldname << ": "; }

bool readFieldName(istream& in, const string& fieldname, bool force)
{ 
    pl_streambuf* buffer = dynamic_cast<pl_streambuf*>(in.rdbuf());
    pl_streammarker fence(buffer);
    string word;
    in >> word;
    if(word != fieldname+":")
    {
        if (force)
            PLERROR("In readFieldName read %s while expected fieldname was %s",word.c_str(),fieldname.c_str());
        else {
            // back-track to before trying to read the field name
            //NOTE: FIX_ME
            // seekmark is done on 'buffer'... which is NOT in's buffer...
            // so the pl_streambuf and the pl_streammarker are useless.
            // It would be an error anyways to set in's buffer to 'buffer':
            // 'buffer' is local to this function and seekmark is done just before
            // return.  suggestion: don't use this function...
            //                        -xsm
            buffer->seekmark(fence);
            return false;
        }
    }
    in.get(); // consume following white space
    return true;

}




// Functions to write a file in any representation

void fwrite_int(FILE *f, const int* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        reverse_int(ptr,n);
        fwrite(ptr,sizeof(int),n,f);
        reverse_int(ptr,n);
    }
    else
        fwrite(ptr,sizeof(int),n,f);
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        fwrite(ptr,sizeof(int),n,f);
    else
    {
        reverse_int(ptr,n);
        fwrite(ptr,sizeof(int),n,f);
        reverse_int(ptr,n);
    }
#endif
}

void fwrite_float(FILE *f, const float* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        reverse_float(ptr,n);
        fwrite(ptr,sizeof(float),n,f);
        reverse_float(ptr,n);
    }
    else
        fwrite(ptr,sizeof(float),n,f);
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        fwrite(ptr,sizeof(float),n,f);
    else
    {
        reverse_float(ptr,n);
        fwrite(ptr,sizeof(float),n,f);
        reverse_float(ptr,n);
    }
#endif
}

void fwrite_float(FILE *f, const double* ptr, int n, bool is_file_bigendian)
{
    float* fptr = new float[n];
    for(int i=0; i<n; i++)
        fptr[i] = float(ptr[i]);
    fwrite_float(f,fptr,n,is_file_bigendian);
    delete[] fptr;
}

void fwrite_double(FILE *f, const double* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        reverse_double(ptr,n);
        fwrite(ptr,sizeof(double),n,f);
        reverse_double(ptr,n);
    }
    else
        fwrite(ptr,sizeof(double),n,f);
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        fwrite(ptr,sizeof(double),n,f);
    else
    {
        reverse_double(ptr,n);
        fwrite(ptr,sizeof(double),n,f);
        reverse_double(ptr,n);
    }
#endif
}

void fwrite_double(FILE *f, const float* ptr, int n, bool is_file_bigendian)
{
    double* dptr = new double[n];
    for(int i=0; i<n; i++)
        dptr[i] = double(ptr[i]);
    fwrite_double(f,dptr,n,is_file_bigendian);
    delete[] dptr;
}

// Functions to read from a file written in any representation

void fread_int(FILE *f, int* ptr, int n, bool is_file_bigendian)
{
    fread(ptr,sizeof(int),n,f);
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        reverse_int(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        reverse_int(ptr,n);
#endif
}

void fread_float(FILE *f, float* ptr, int n, bool is_file_bigendian)
{
    fread(ptr,sizeof(float),n,f);
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        reverse_float(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        reverse_float(ptr,n);
#endif
}

void fread_float(FILE *f, double* ptr, int n, bool is_file_bigendian)
{
    float* fptr = new float[n];
    fread_float(f,fptr,n,is_file_bigendian);
    for(int i=0; i<n; i++)
        ptr[i] = double(fptr[i]);
    delete[] fptr;
}

void fread_double(FILE *f, double* ptr, int n, bool is_file_bigendian)
{
    fread(ptr,sizeof(double),n,f);
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        reverse_double(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        reverse_double(ptr,n);
#endif
}

void fread_double(FILE *f, float* ptr, int n, bool is_file_bigendian)
{
    double* dptr = new double[n];
    fread_double(f,dptr,n,is_file_bigendian);
    for(int i=0; i<n; i++)
        ptr[i] = float(dptr[i]);
    delete[] dptr;
}

void fread_short(FILE *f, unsigned short* ptr, int n, bool is_file_bigendian)
{
    fread(ptr,sizeof(unsigned short),n,f);
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        reverse_ushort(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        reverse_ushort(ptr,n);
#endif
}

void write_int(ostream& out, const int* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        reverse_int(ptr,n);
        out.write((char*)ptr,n*sizeof(int));
        reverse_int(ptr,n);
    }
    else
        out.write((char*)ptr,n*sizeof(int));
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        out.write((char*)ptr,n*sizeof(int));
    else
    {
        reverse_int(ptr,n);
        out.write((char*)ptr,n*sizeof(int));
        reverse_int(ptr,n);
    }
#endif
}

void write_short(ostream& out, const short* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        reverse_short(ptr,n);
        out.write((char*)ptr,n*sizeof(short));
        reverse_short(ptr,n);
    }
    else
        out.write((char*)ptr,n*sizeof(short));
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        out.write((char*)ptr,n*sizeof(short));
    else
    {
        reverse_short(ptr,n);
        out.write((char*)ptr,n*sizeof(short));
        reverse_short(ptr,n);
    }
#endif
}

void write_double(ostream& out, const double* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        reverse_double(ptr,n);
        out.write((char*)ptr,n*sizeof(double));
        reverse_double(ptr,n);
    }
    else
        out.write((char*)ptr,n*sizeof(double));
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        out.write((char*)ptr,n*sizeof(double));
    else
    {
        reverse_double(ptr,n);
        out.write((char*)ptr,n*sizeof(double));
        reverse_double(ptr,n);
    }
#endif
}


void write_float(ostream& out, const float* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        reverse_float(ptr,n);
        out.write((char*)ptr,n*sizeof(float));
        reverse_float(ptr,n);
    }
    else
        out.write((char*)ptr,n*sizeof(float));
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        out.write((char*)ptr,n*sizeof(float));
    else
    {
        reverse_float(ptr,n);
        out.write((char*)ptr,n*sizeof(float));
        reverse_float(ptr,n);
    }
#endif
}


// Functions to read from a file written in any representation

void read_int(istream& in, int* ptr, int n, bool is_file_bigendian)
{
    in.read((char *)ptr,n*sizeof(int));
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        reverse_int(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        reverse_int(ptr,n);
#endif
}

void read_short(istream& in, short* ptr, int n, bool is_file_bigendian)
{
    in.read((char *)ptr,n*sizeof(short));
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        reverse_short(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        reverse_short(ptr,n);
#endif
}

void read_float(istream& in, float* ptr, int n, bool is_file_bigendian)
{
    in.read((char *)ptr,n*sizeof(float));
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        reverse_float(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        reverse_float(ptr,n);
#endif
}

void read_double(istream& in, double* ptr, int n, bool is_file_bigendian)
{
    in.read((char *)ptr,n*sizeof(double));
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        reverse_double(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        reverse_double(ptr,n);
#endif
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
