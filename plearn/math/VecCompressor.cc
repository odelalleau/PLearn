// -*- C++ -*-

// VecCompressor.cc
// Copyright (C) 2001 Pascal Vincent
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
 ******************************************************* */

#include "VecCompressor.h"

namespace PLearn {
using namespace std;

// *******************
// ** VecCompressor **

signed char* VecCompressor::compressVec(const Vec& v, signed char* data)
{
    real* vdata = v.data();
    signed char* ptr = data;

    // mode can be '0' for zeroes, 'F' for floats, 'I' for small integers (signed chars)
    // If mode is '0' abs(count) indicates the number of zeroes, 
    //                a positive sign indicates switch to 'F' mode
    //                a negative sign indicates switch to 'I' mode
    //                a 0 count means insert 127 zeros and stay in zero mode
    // If mode is 'F' abs(count) indicates the number of floats that follow 
    //                a positive sign indicates switch to 'I' mode
    //                a negative sign indicates switch to '0' mode
    //                a 0 count means insert 127 floats and stay in float mode
    // If mode is 'I' abs(count) indicates the number of small integers that follow 
    //                a positive sign indicates switch to 'F' mode
    //                a negative sign indicates switch to '0' mode
    //                a 0 count means insert 127 small integers and stay in 'I' mode

    int l = v.length();

    int i=0;
    real val = vdata[i];

    signed char mode = 'F';
    if(fast_exact_is_equal(val, 0.))
        mode = '0';
    else if(issmallint(val))
        mode = 'I';
    // else 'F'

    int count = 0;
    int istart = 0;
    float fval = 0.;
    signed char* pfval = (signed char*)&fval;

    *ptr++ = mode;

    while(i<l)
    {
        switch(mode)
        {
        case '0':
            istart = i;
            while(i<l && is0(vdata[i]))
                i++;
            count = i - istart;
            while(count>127)
            {
                *ptr++ = 0;
                count -= 127;
            }
            if(i>=l || issmallint(vdata[i]))
            {
                *ptr++ = (signed char)(-count);
                mode = 'I';
            }
            else
            {
                *ptr++ = (signed char)count;
                mode = 'F';
            }
            break;

        case 'I':
            istart = i;
            while(i<l && isI(vdata[i]))
                i++;
            count = i - istart;
            while(count>127)
            {
                *ptr++ = 0;
                int n = 127;
                while(n--)
                    *ptr++ = (signed char)vdata[istart++];
                count -= 127;
            }
            if(i>=l || is0(vdata[i]))
            {
                *ptr++ = (signed char)(-count);
                mode = '0';
            }
            else // next value is a floating point
            {
                *ptr++ = (signed char)count;
                mode = 'F';
            }
            while(count--)
                *ptr++ = (signed char)vdata[istart++];                
            break;

        case 'F':
            istart = i;
            val = vdata[i];
            while(i<l && isF(vdata[i]))
                i++;
            count = i - istart;
            while(count>127)
            {
                *ptr++ = 0;
                int n = 127;
                while(n--)
                {
                    fval = (float)vdata[istart++];
                    *ptr++ = pfval[0];
                    *ptr++ = pfval[1];
                    *ptr++ = pfval[2];
                    *ptr++ = pfval[3];
                }
                count -= 127;
            }
            if(i>=l || is0(vdata[i]))
            {
                *ptr++ = (signed char)(-count);
                mode = '0';
            }
            else
            {
                *ptr++ = (signed char)count;
                mode = 'I';
            }
            while(count--)
            {
                fval = (float)vdata[istart++];
                *ptr++ = pfval[0];
                *ptr++ = pfval[1];
                *ptr++ = pfval[2];
                *ptr++ = pfval[3];
            }
        }
    }
    return ptr;
}

void VecCompressor::uncompressVec(signed char* data, const Vec& v)
{
    // mode can be '0' for zeroes, 'F' for floats, 'I' for small integers (signed chars)
    // If mode is '0' abs(count) indicates the number of zeroes, 
    //                a positive sign indicates switch to 'F' mode
    //                a negative sign indicates switch to 'I' mode
    //                a 0 count means insert 127 zeros and stay in zero mode
    // If mode is 'F' abs(count) indicates the number of floats that follow 
    //                a positive sign indicates switch to 'I' mode
    //                a negative sign indicates switch to '0' mode
    //                a 0 count means insert 127 floats and stay in float mode
    // If mode is 'I' abs(count) indicates the number of small integers that follow 
    //                a positive sign indicates switch to 'F' mode
    //                a negative sign indicates switch to '0' mode
    //                a 0 count means insert 127 small integers and stay in 'I' mode

    real* vptr = v.data();
    real* vptrend = vptr+v.length();
    signed char* ptr = data;
    signed char mode = *ptr++;
    float fval = 0.;
    signed char* pfval = (signed char*)&fval;
    signed char count;
    while(vptr!=vptrend)
    {
        count = *ptr++;
        switch(mode)
        {
        case '0':
            if(count<0)
            {
                mode = 'I';              
                count = -count;
            }
            else if(count>0)
                mode = 'F';
            else
                count = 127;

            while(count--)
                *vptr++ = 0.;
            break;

        case 'I':
            if(count<0)
            {
                mode = '0';
                count = -count;
            }
            else if(count>0)
                mode = 'F';
            else 
                count = 127;

            while(count--)
                *vptr++ = real(*ptr++);
            break;
          
        case 'F':
            if(count<0)
            {
                mode = '0';
                count = -count;
            }
            else if(count>0)
                mode = 'I';
            else 
                count = 127;

            while(count--)
            {
                pfval[0] = *ptr++;
                pfval[1] = *ptr++;
                pfval[2] = *ptr++;
                pfval[3] = *ptr++;
                *vptr++ = real(fval);
            }
            break;

        default:
            PLERROR("Problem in VecCompressor::uncompressVec this should not happen!!! (wrong data format?)");
        }
    }
}

void VecCompressor::writeCompressedVec(ostream& out, const Vec& v)
{
    real* vdata = v.data();

    // mode can be '0' for zeroes, 'F' for floats, 'I' for small integers (signed chars)
    // If mode is '0' abs(count) indicates the number of zeroes, 
    //                a positive sign indicates switch to 'F' mode
    //                a negative sign indicates switch to 'I' mode
    //                a 0 count means insert 127 zeros and stay in zero mode
    // If mode is 'F' abs(count) indicates the number of floats that follow 
    //                a positive sign indicates switch to 'I' mode
    //                a negative sign indicates switch to '0' mode
    //                a 0 count means insert 127 floats and stay in float mode
    // If mode is 'I' abs(count) indicates the number of small integers that follow 
    //                a positive sign indicates switch to 'F' mode
    //                a negative sign indicates switch to '0' mode
    //                a 0 count means insert 127 small integers and stay in 'I' mode

    int l = v.length();

    int i=0;
    real val = vdata[i];

    signed char mode = 'F';
    if(fast_exact_is_equal(val, 0.))
        mode = '0';
    else if(issmallint(val))
        mode = 'I';
    // else 'F'

    int count = 0;
    int istart = 0;
    float fval = 0.;

    write_sbyte(out,mode);

    while(i<l)
    {
        switch(mode)
        {
        case '0':
            istart = i;
            while(i<l && is0(vdata[i]))
                i++;
            count = i - istart;
            while(count>127)
            {
                write_sbyte(out,0);
                count -= 127;
            }
            if(i>=l || issmallint(vdata[i]))
            {
                write_sbyte(out,-count);
                mode = 'I';
            }
            else
            {
                write_sbyte(out,count);
                mode = 'F';
            }
            break;

        case 'I':
            istart = i;
            while(i<l && isI(vdata[i]))
                i++;
            count = i - istart;
            while(count>127)
            {
                write_sbyte(out,0);
                int n = 127;
                while(n--)
                    write_sbyte(out,(signed char)vdata[istart++]);
                count -= 127;
            }
            if(i>=l || is0(vdata[i]))
            {
                write_sbyte(out,-count);
                mode = '0';
            }
            else // next value is a floating point
            {
                write_sbyte(out,count);
                mode = 'F';
            }
            while(count--)
                write_sbyte(out,(signed char)vdata[istart++]);                
            break;

        case 'F':
            istart = i;
            while(i<l && isF(vdata[i]))
                i++;
            count = i - istart;
            while(count>127)
            {
                write_sbyte(out,0);
                int n = 127;
                while(n--)
                {
                    fval = (float)vdata[istart++];
                    out.write((char*)&fval,4);
                }
                count -= 127;
            }
            if(i>=l || is0(vdata[i]))
            {
                write_sbyte(out,-count);
                mode = '0';
            }
            else
            {
                write_sbyte(out,count);
                mode = 'I';
            }
            while(count--)
            {
                fval = (float)vdata[istart++];
                out.write((char*)&fval,4);
            }
        }
    }
}

void VecCompressor::readCompressedVec(istream& in, const Vec& v)
{
    // mode can be '0' for zeroes, 'F' for floats, 'I' for small integers (signed chars)
    // If mode is '0' abs(count) indicates the number of zeroes, 
    //                a positive sign indicates switch to 'F' mode
    //                a negative sign indicates switch to 'I' mode
    //                a 0 count means insert 127 zeros and stay in zero mode
    // If mode is 'F' abs(count) indicates the number of floats that follow 
    //                a positive sign indicates switch to 'I' mode
    //                a negative sign indicates switch to '0' mode
    //                a 0 count means insert 127 floats and stay in float mode
    // If mode is 'I' abs(count) indicates the number of small integers that follow 
    //                a positive sign indicates switch to 'F' mode
    //                a negative sign indicates switch to '0' mode
    //                a 0 count means insert 127 small integers and stay in 'I' mode

    real* vptr = v.data();
    real* vptrend = vptr+v.length();
    signed char mode = read_sbyte(in);
    float fval = 0.;
    signed char count;

    while(vptr!=vptrend)
    {
        count = read_sbyte(in);
        // cerr << int(count) <<  ' ';
        switch(mode)
        {
        case '0':
            if(count<0)
            {
                mode = 'I';              
                count = -count;
            }
            else if(count>0)
                mode = 'F';
            else
                count = 127;

            while(count--)
                *vptr++ = 0.;
            break;

        case 'I':
            if(count<0)
            {
                mode = '0';
                count = -count;
            }
            else if(count>0)
                mode = 'F';
            else 
                count = 127;

            while(count--)
                *vptr++ = real(read_sbyte(in));
            break;
          
        case 'F':
            if(count<0)
            {
                mode = '0';
                count = -count;
            }
            else if(count>0)
                mode = 'I';
            else 
                count = 127;

            while(count--)
            {
                in.read((char*)&fval,4);
                *vptr++ = real(fval);
            }
            break;

        default:
            PLERROR("Problem in VecCompressor::readCompressedVec this should not happen!!! (wrong data format?)");
        }
    }
    // cerr << endl;
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
