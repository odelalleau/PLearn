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

#include "NistDB.h"

namespace PLearn {
using namespace std;


#define DO_RESCALE

NistDB::NistDB(bool train)
    :VMatrix(60000, 28*28+1)
{
    if(train)
    {
        imagef.open(PPath("DBDIR:MNIST/train-images-idx3-ubyte").c_str());
        labelf.open(PPath("DBDIR:MNIST/train-labels-idx1-ubyte").c_str());
        length_ = 60000;
    }
    else
    {
        //      imagef.open(DBDIR"/MNIST/test-images.idx3-ubyte");
        //      labelf.open(DBDIR"/MNIST/test-labels.idx1-ubyte");
        //      length_ = 60000;
        imagef.open(PPath("DBDIR:MNIST/t10k-images-idx3-ubyte").c_str());
        labelf.open(PPath("DBDIR:MNIST/t10k-labels-idx1-ubyte").c_str());
        length_ = 10000;
    }
    if(!imagef)
        PLERROR("In NistDB constructor could not open imagefile for reading");
    if(!labelf)
        PLERROR("In NistDB constructor could not open labelfile for reading");
}
  
real NistDB::get(int i, int j) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j>=width())
        PLERROR("In NistDB::get OUT OF BOUNDS");
#endif
    if(j==width()-1) // then read from labelf
    {
        labelf.seekg(8+i);
        return real(labelf.get());
    }
    else // read from imagef
    {
        imagef.seekg(16+i*(28*28)+j);
#ifdef DO_RESCALE
        return real(imagef.get())/255.0;
#else
        return real(imagef.get());
#endif
    }
}
 
void NistDB::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || j<0 || j+v.length()>width())
        PLERROR("In NistDB::getSubRow OUT OF BOUNDS");
#endif
  
    int npixelstoread = v.length();
    if(j+v.length()==width())
    {
        labelf.seekg(8+i);
        v[v.length()-1] = real(labelf.get());
        npixelstoread--;
    }

    if(j<width()-1)
    {
        imagef.seekg(16+i*(28*28)+j);
#if __GNUC__ < 3 && !defined(WIN32)
        imagef.read(buf, npixelstoread);
#else
        imagef.read(reinterpret_cast<char*>(buf), npixelstoread);
#endif
        for(int k=0; k<npixelstoread; k++)
        {
#ifdef DO_RESCALE
            v[k] = real(buf[k])/255.0;
#else
            v[k] = real(buf[k]);
#endif
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
