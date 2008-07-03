// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal

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

#include "VarArray.h"
#include "NaryVariable.h"
//#include <plearn/display/DisplayUtils.h>
#include "VarArrayElementVariable.h"

namespace PLearn {
using namespace std;

//////////////
// VarArray //
//////////////
VarArray::VarArray()
    : Array<Var>(0,10) 
{}

VarArray::VarArray(int n, int n_extra)
    : Array<Var>(n,n_extra) 
{}

VarArray::VarArray(int n, int initial_length, int n_extra)
    : Array<Var>(n,n_extra) 
{
    iterator array = data();
    for (int i=0;i<n;i++)
        array[i]=Var(initial_length);
}

VarArray::VarArray(int n, int initial_length, int initial_width, int n_extra)
    : Array<Var>(n,n_extra) 
{
    iterator array = data();
    for (int i=0;i<n;i++)
        array[i]=Var(initial_length,initial_width);
}

VarArray::VarArray(const Var& v)
    : Array<Var>(1,10)
{ (*this)[0] = v; }

VarArray::VarArray(const Var& v1, const Var& v2)
    : Array<Var>(2,10)
{ 
    (*this)[0] = v1; 
    (*this)[1] = v2; 
}

VarArray::VarArray(Variable*  v)
    : Array<Var>(1,10)
{ (*this)[0] = Var(v); }

VarArray::VarArray(Variable*  v1, Variable*  v2)
    : Array<Var>(2,10)
{ 
    (*this)[0] = Var(v1); 
    (*this)[1] = Var(v2); 
}

VarArray::VarArray(Variable*  v1, Variable*  v2, Variable* v3):
    Array<Var>(3)
{ 
    (*this)[0] = Var(v1); 
    (*this)[1] = Var(v2); 
    (*this)[2] = Var(v3); 
}
// This is really EXTERN!  Don't try to define it...
//template<>
//extern void deepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies); // a cause d'une bug du compilateur
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VarArray::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    for(int i=0; i<size(); i++)
        varDeepCopyField((*this)[i], copies);
}

////////////////////
// copyValuesFrom //
////////////////////
void VarArray::copyValuesFrom(const VarArray& from)
{
    if (size()!=from.size())
        PLERROR("VarArray::copyValuesFrom(a): a does not have the same size (%d) as this (%d)\n",
                from.size(),size());
    for (int i=0;i<size();i++)
        (*this)[i]->value << from[i]->value;
}

//////////////
// copyFrom //
//////////////
void VarArray::copyFrom(const Vec& datavec)
{
    copyFrom(datavec.data(),datavec.length());
}

void VarArray::copyFrom(const real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real* value = v->valuedata;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::copyFrom total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                value[j] = *data++;
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::copyFrom total length of all Vars in the array (%d) differs from length of argument (%d)",
                ncopied,n);
}

void VarArray::makeSharedValue(real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            int vlength = v->nelems();
            v->makeSharedValue(data,vlength);
            data += vlength;
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::makeSharedValue total length of all Vars in the array (%d) differs from length of argument (%d)",
                ncopied,n);
}

void VarArray::makeSharedValue(PP<Storage<real> > storage, int offset_) 
{
    iterator array = data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            int vlength = v->nelems();
            v->makeSharedValue(storage,offset_+ncopied);
            ncopied += vlength;
        }
    }
}

void VarArray::makeSharedGradient(Vec& v, int offset_)
{
    makeSharedGradient( v.storage, v.offset_+offset_);
}

void VarArray::makeSharedRValue(Vec& v, int offset_)
{
    makeSharedRValue( v.storage, v.offset_+offset_);
}

void VarArray::makeSharedGradient(PP<Storage<real> > storage, int offset_) 
{
    iterator array = data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            int vlength = v->nelems();
            v->makeSharedGradient(storage,offset_+ncopied);
            ncopied += vlength;
        }
    }
}

void VarArray::makeSharedValue(Vec& v, int offset_) 
{ 
    if (v.length() < nelems()+offset_)
        PLERROR("VarArray::makeSharedValue(Vec,int): vector too short (%d < %d + %d)",
                v.length(),nelems(),offset_);
    makeSharedValue(v.storage,offset_); 
}

void VarArray::makeSharedGradient(real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            int vlength = v->nelems();
            v->makeSharedGradient(data,vlength);
            data += vlength;
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::makeSharedGradient total length of all Vars in the array (%d) differs from length of argument (%d)",
                ncopied,n);
}

void VarArray::copyTo(const Vec& datavec) const
{
    copyTo(datavec.data(), datavec.length());
}

void VarArray::accumulateTo(const Vec& datavec) const
{
    accumulateTo(datavec.data(), datavec.length());
}

void VarArray::copyTo(real* data, int n) const
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real* value = v->valuedata;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::copyTo total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                *data++ = value[j];
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::copyTo total length of all Vars in the array differs from length of argument");
}

void VarArray::accumulateTo(real* data, int n) const
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real* value = v->valuedata;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::copyTo total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                *data++ += value[j];
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::copyTo total length of all Vars in the array differs from length of argument");
}

void VarArray::copyGradientFrom(const Vec& datavec)
{
    copyGradientFrom(datavec.data(),datavec.length());
}

void VarArray::copyGradientFrom(const Array<Vec>& datavec)
{
    iterator array = this->data();
    if (size()!=datavec.size())
        PLERROR("VarArray::copyGradientFrom has argument of size %d, expected %d",datavec.size(),size());
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        real* data = datavec[i].data();
        int n = datavec[i].length();
        if (!v.isNull())
        {
            real* value = v->gradientdata;
            int vlength = v->nelems();
            if(vlength!=n)
                PLERROR("IN VarArray::copyGradientFrom length of -th Var in the array differs from length of %d-th argument",i);
            for(int j=0; j<vlength; j++)
                value[j] = *data++;
        }
    }
}

void VarArray::copyGradientTo(const Array<Vec>& datavec)
{
    iterator array = this->data();
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        real* data = datavec[i].data();
        int n = datavec[i].length();
        if (!v.isNull())
        {
            real* value = v->gradientdata;
            int vlength = v->nelems();
            if(vlength!=n)
                PLERROR("IN VarArray::copyGradientFrom length of -th Var in the array differs from length of %d-th argument",i);
            for(int j=0; j<vlength; j++)
                *data++ = value[j];
        }
    }
}

void VarArray::accumulateGradientFrom(const Vec& datavec)
{
    accumulateGradientFrom(datavec.data(),datavec.length());
}

void VarArray::copyGradientFrom(const real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real* value = v->gradientdata;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::copyGradientFrom total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                value[j] = *data++;
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::copyGradientFrom total length of all Vars in the array differs from length of argument");
}

void VarArray::accumulateGradientFrom(const real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real* value = v->gradientdata;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::accumulateGradientFrom total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                value[j] += *data++;
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::accumulateGradientFrom total length of all Vars in the array differs from length of argument");
}

void VarArray::copyGradientTo(const Vec& datavec)
{
    copyGradientTo(datavec.data(), datavec.length());
}

void VarArray::copyGradientTo(real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real* value = v->gradientdata;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::copyGradientTo total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                *data++ = value[j];
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::copyGradientTo total length of all Vars in the array differs from length of argument");
}

void VarArray::accumulateGradientTo(const Vec& datavec)
{
    accumulateGradientTo(datavec.data(), datavec.length());
}

void VarArray::accumulateGradientTo(real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real* value = v->gradientdata;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::accumulateGradientTo total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                *data++ += value[j];
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::accumulateGradientTo total length of all Vars in the array differs from length of argument");
}

void VarArray::copyMinValueTo(const Vec& datavec)
{
    copyMinValueTo(datavec.data(), datavec.length());
}

void VarArray::copyMinValueTo(real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real value = v->min_value;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::copyMinValueTo total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                *data++ = value;
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::copyMinValueTo total length of all Vars in the array differs from length of argument");
}

void VarArray::copyMaxValueTo(const Vec& datavec)
{
    copyMaxValueTo(datavec.data(), datavec.length());
}

void VarArray::copyMaxValueTo(real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real value = v->max_value;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::copyMaxValueTo total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                *data++ = value;
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::copyMaxValueTo total length of all Vars in the array differs from length of argument");
}

void VarArray::makeSharedRValue(PP<Storage<real> > storage, int offset_) 
{
    iterator array = data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            v->resizeRValue();
            int vlength = v->nelems();
            v->makeSharedRValue(storage,offset_+ncopied);
            ncopied += vlength;
        }
    }
}

void VarArray::copyRValueTo(real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            real* value = v->rvaluedata;
            if (!value)
                PLERROR("VarArray::copyRValueTo: empty Var in the array (number %d)",i);
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::copyRValueTo total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                *data++ = value[j];
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::copyRValueTo total length of all Vars in the array differs from length of argument");
}

void VarArray::copyRValueFrom(const real* data, int n)
{
    iterator array = this->data();
    int ncopied=0; // number of elements copied so far
    for(int i=0; i<size(); i++)
    {
        Var& v = array[i];
        if (!v.isNull())
        {
            v->resizeRValue();
            real* value = v->rvaluedata;
            int vlength = v->nelems();
            if(ncopied+vlength>n)
                PLERROR("IN VarArray::copyRValueFrom total length of all Vars in the array exceeds length of argument");
            for(int j=0; j<vlength; j++)
                value[j] = *data++;
            ncopied += vlength;
        }
    }
    if(ncopied!=n)
        PLERROR("IN VarArray::copyRValueFrom total length of all Vars in the array differs from length of argument");
}

void VarArray::copyRValueTo(const Vec& datavec)
{
    copyRValueTo(datavec.data(), datavec.length());
}

void VarArray::copyRValueFrom(const Vec& datavec)
{
    copyRValueFrom(datavec.data(),datavec.length());
}

int VarArray::nelems() const
{
    iterator array = data();
    int totallength = 0;
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            totallength += array[i]->nelems();
    return totallength;
}

void VarArray::resizeRValue()
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->resizeRValue();
}

int VarArray::sumOfLengths() const
{
    iterator array = data();
    int totallength = 0;
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            totallength += array[i]->length();
    return totallength;
}

int VarArray::sumOfWidths() const
{
    iterator array = data();
    int totalwidth = 0;
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            totalwidth += array[i]->width();
    return totalwidth;
}

int VarArray::maxWidth() const
{
    iterator array = data();
    int maxwidth = 0;
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
        {
            int w = array[i]->width();
            if (w>maxwidth)
                maxwidth=w;
        }
    return maxwidth;
}

int VarArray::maxLength() const
{
    iterator array = data();
    int maxlength = 0;
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
        {
            int l = array[i]->length();
            if (l>maxlength)
                maxlength=l;
        }
    return maxlength;
}

VarArray VarArray::nonNull() const
{
    VarArray results(0, size());

    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            results.append(array[i]);

    return results;
}
VarArray& VarArray::subVarArray(int start,int len) const
{
    int max_position=start+len-1;
    if (max_position>size())               
        PLERROR("Error in :subVarArray(int start,int len), start+len>=nelems");
    VarArray* results=new VarArray(0, len);
  
    iterator array = data();
    for(int i=start; i<start+len; i++)
        if (!array[i].isNull())
            results->append(array[i]);
  
    return *results;
} 
void VarArray::copyFrom(int start,int len,const VarArray& from)
{
    int max_position=start+len-1;
    if (max_position>size())
        PLERROR("VarArray is to short");
    iterator array = data();
    for(int i=0; i<len; i++)
        if (!from[i].isNull())
            array[i+start]=from[i];
}
void VarArray::setMark() const
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->setMark();
}

void VarArray::clearMark() const
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->clearMark();
}

void VarArray::markPath() const
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull()){
            array[i]->markPath();
            //cout<<"mark :"<<i<<" "<<array[i]->getName()<<endl;
        }
}

void VarArray::buildPath(VarArray& path) const
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->buildPath(path);
}

void VarArray::fprop()
{
    iterator array = data();
    for(int i=0; i<size(); i++)
    {
        if (!array[i].isNull())
            array[i]->fprop();
    }
}

void VarArray::sizeprop()
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->sizeprop();
}

void VarArray::sizefprop()
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->sizefprop();
}

void VarArray::bprop()
{
    iterator array = data();
    for(int i=size()-1; i>=0; i--)
        if (!array[i].isNull())
            array[i]->bprop();
}

void VarArray::bbprop()
{
    iterator array = data();
    for(int i=size()-1; i>=0; i--)
        if (!array[i].isNull())
            array[i]->bbprop();
}

void VarArray::rfprop()
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->rfprop();
}

// This one is to be a little smarter, when handling SumOfs and the like...
// Now work properly if VarArray has size 0
void VarArray::fbprop()
{
    iterator array = data();
    for(int i=0; i<size()-1; i++)
        if (!array[i].isNull())
            array[i]->fprop();

    if (size() > 0) {
        last()->fbprop();

#ifdef BOUNDCHECK
        if (last()->gradient.hasMissing())
            PLERROR("VarArray::fbprop has NaN gradient");
#endif    
        for(int i=size()-2; i>=0; i--)
            if (!array[i].isNull())
            {
#ifdef BOUNDCHECK
                if (array[i]->gradient.hasMissing())
                    PLERROR("VarArray::fbprop has NaN gradient");
#endif    
                array[i]->bprop();
            }
    }
}

void VarArray::sizefbprop()
{
    iterator array = data();
    for(int i=0; i<size()-1; i++)
        if (!array[i].isNull())
            array[i]->sizefprop();

    if (size() > 0) {
        {
            last()->sizeprop();
            last()->fbprop();
        }

#ifdef BOUNDCHECK
        if (last()->gradient.hasMissing())
            PLERROR("VarArray::fbprop has NaN gradient");
#endif    
        for(int i=size()-2; i>=0; i--)
            if (!array[i].isNull())
            {
#ifdef BOUNDCHECK
                if (array[i]->gradient.hasMissing())
                    PLERROR("VarArray::fbprop has NaN gradient");
#endif    
                array[i]->bprop();
            }
    }
}

void VarArray::fbbprop()
{
    iterator array = data();
    for(int i=0; i<size()-1; i++)
        if (!array[i].isNull())
            array[i]->fprop();

    if (size() > 0) {
        last()->fbbprop();

        for(int i=size()-2; i>=0; i--)
            if (!array[i].isNull())
            {
                array[i]->bprop();
                array[i]->bbprop();
            }
    }
}


void VarArray::symbolicBprop()
{
    iterator array = data();
    for(int i=size()-1; i>=0; i--)
        if (!array[i].isNull())
            array[i]->symbolicBprop();
}

VarArray VarArray::symbolicGradient()
{
    iterator array = data();
    VarArray symbolic_gradients(size());
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            symbolic_gradients[i] = array[i]->g;  
    return symbolic_gradients;
}

void VarArray::clearSymbolicGradient()
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->clearSymbolicGradient();  
}
 
void VarArray::fillGradient(real value)
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->fillGradient(value);
}

void VarArray::clearGradient()
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->clearGradient();
}

void VarArray::clearDiagHessian()
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->clearDiagHessian();
}


void VarArray::setDontBpropHere(bool val)
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->setDontBpropHere(val);
}

bool VarArray::update(real step_size, Vec direction, real coeff, real b)
{
    bool hit = false;
    int pos=0;
    iterator array = data();
    for(int i=0; i<size(); pos+=array[i++]->nelems())
        if (!array[i].isNull())
            hit = hit || 
                array[i]->update(step_size,direction.subVec(pos,array[i]->nelems()), coeff, b);
    return hit;
}

bool VarArray::update(Vec step_sizes, Vec direction, Vec coeffs)
{
    bool hit = false;
    int pos = 0;
    iterator array = data();
    for (int i=0; i<size(); pos+=array[i++]->nelems()) {
        if (!array[i].isNull()) {
            hit = hit ||
                array[i]->update(
                    step_sizes.subVec(pos, array[i]->nelems()), 
                    direction.subVec(pos, array[i]->nelems()),
                    coeffs[i]);
        }
    }
    return hit;
}

bool VarArray::update(Vec step_sizes, Vec direction, real coeff, real b)
{
    bool hit = false;
    int pos = 0;
    iterator array = data();
    for (int i=0; i<size(); pos+=array[i++]->nelems()) {
        if (!array[i].isNull()) {
            hit = hit ||
                array[i]->update(
                    step_sizes.subVec(pos, array[i]->nelems()), 
                    direction.subVec(pos, array[i]->nelems()),
                    coeff, b);
        }
    }
    return hit;
}

real VarArray::maxUpdate(Vec direction)
{
    real max_step_size = FLT_MAX;
    int pos=0;
    iterator array = data();
    for(int i=0; i<size(); pos+=array[i++]->nelems())
        if (!array[i].isNull())
            max_step_size = MIN(max_step_size,
                                array[i]->maxUpdate(direction.subVec(pos,array[i]->nelems())));
    return max_step_size;
}

bool VarArray::update(real step_size, bool clear)
{
    bool hit = false;
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            hit = hit || array[i]->update(step_size,clear);
    return hit;
}

void VarArray::updateWithWeightDecay(real step_size, real weight_decay, bool L1, bool clear)
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->updateWithWeightDecay(step_size,weight_decay,L1,clear);
    return;
}

bool VarArray::update(Vec new_value)
{
    bool hit = false;
    int pos=0;
    iterator array = data();
    for(int i=0; i<size(); pos+=array[i++]->nelems())
        if (!array[i].isNull())
            hit = hit || 
                array[i]->update(new_value.subVec(pos,array[i]->nelems()));
    return hit;
}

void VarArray::read(istream& in)
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->read(in);
}

void VarArray::write(ostream& out) const
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->write(out);
}

Var VarArray::operator[](Var index)
{
    return new VarArrayElementVariable(*this, index);
}

VarArray VarArray::sources() const
{
    VarArray a;
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            a &= array[i]->sources();
    return a;
}

VarArray VarArray::ancestors() const
{
    VarArray a;
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            a &= array[i]->ancestors();
    return a;
}

void VarArray::unmarkAncestors() const
{
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
            array[i]->unmarkAncestors();
}

VarArray VarArray::parents() const
{
    setMark();
    VarArray all_parents;
    VarArray parents_i; 
    iterator array = data();
    for(int i=0; i<size(); i++)
        if (!array[i].isNull())
        {
            parents_i = array[i]->parents();
            if(parents_i.size()>0)
            {
                parents_i.setMark();
                all_parents.append(parents_i);
            }
        }
    clearMark();
    all_parents.clearMark();
    return all_parents;
}
void VarArray::printNames()const
{
    for(int i=0;i<this->size();i++)
        cout<<i<<" : "<<(*this)[i]->getName()<<endl;
}

/** The function that computes a propagation path **/

// from inputs to outputs
VarArray propagationPath(const VarArray& inputs, const VarArray& outputs)
{
    if(outputs.size()==0)
        return VarArray(0,0);

    VarArray proppath; 
    inputs.setMark(); // sets the mark for all inputs
    outputs.markPath(); // sets the mark along all paths going from inputs to outputs
    inputs.clearMark(); // since we don't want the inputs in the update path
    outputs.buildPath(proppath); // appends to proppath every marked item that leads to one of the outputs
    // and clears the marks at the same time.
    return proppath;
}
// from inputs to outputs passing by parameters_to_optimize
// IMPORTANT this is not working properly //FP
/*
  VarArray propagationPath(const VarArray& inputs, const VarArray& parameters_to_optimize,const VarArray& outputs)
  {
  //VarArray test=propagationPath(inputs,outputs);
  //cout<<"parameters_to_optimize :"<<endl<<parameters_to_optimize<<endl; 
  //cout<<"propagationPath(inputs,parameters_to_optimize)"<<endl;
  VarArray path1=propagationPath(inputs,parameters_to_optimize);
  //cout<<"propagationPath(parameters_to_optimize,outputs)"<<endl;
  VarArray path2=propagationPath(parameters_to_optimize,outputs);
  //cout<<"end of propagationPath(parameters_to_optimize,outputs)"<<endl;
  //cout<<"path2"<<endl<<path2<<endl;
  //displayVarGraph(inputs);
  //displayVarGraph(parameters_to_optimize);
  
  //displayVarGraph(test,true);
  //displayVarGraph(path1,true);
  //displayVarGraph(path2,true);
  return path1&path2;
  }*/

// from all sources to outputs
VarArray propagationPath(const VarArray& outputs)
{
    if(outputs.size()==0)
        return VarArray(0,0);
    VarArray all_sources = outputs.sources();
    outputs.unmarkAncestors();
    return propagationPath(all_sources,outputs);
}

// from all sources to all direct non-inputs parents of the path inputs-->outputs
VarArray propagationPathToParentsOfPath(const VarArray& inputs, const VarArray& outputs)
{
    VarArray parents = nonInputParentsOfPath(inputs, outputs);
    // WARNING: with this way of proceeding, any SourceVariable that 
    // is a direct parent is currently included in parents, and will
    // thus be included in the propagation path computed below.
    // Calling fprop on a SourceVariable should not harm, but we usually
    // avoided this in previous code (because it is useless). Now the
    // question of SampleSourceVariables remains to be solved... (see TODO.txt)
    // For now we keep it like this. But maybe later we should find a way
    // to exclude the unnecessary source variables (and include the maybe necessary
    // SampleSourceVariables) in the returned path... [Pascal]
    if(parents.size()==0)
        return VarArray(0,0);
    return propagationPath(parents);
}


VarArray nonInputParentsOfPath(VarArray inputs, VarArray outputs)
{
    //cout<<"start nonInputParentsOfPath(...)"<<endl;
    VarArray proppath = propagationPath(inputs, outputs);
    inputs.setMark();
    VarArray non_input_parents = proppath.parents();
    inputs.clearMark();
    //cout<<"stop nonInputParentsOfPath(...)"<<endl;
    return non_input_parents;
}

// returns all sources that influence the given vars
VarArray allSources(const VarArray& v)
{
    VarArray result;
    v.unmarkAncestors();
    result = v.sources();
    v.unmarkAncestors();
    return result;
}

// returns all variables of a that are not in b
VarArray operator-(const VarArray& a, const VarArray& b)
{
    VarArray result;
    int i,j;
    for(i=0; i<a.size(); i++)
    {
        Var v = a[i];
        for(j=0; j<b.size(); j++)
            if(b[j]==v)
                break;
        if(j>=b.size()) // v not found in b
            result.append(v);
    }
    return result;
}

// returns all sources that influence outputs except those that influence it only through inputs
VarArray nonInputSources(const VarArray& inputs, const VarArray& outputs)
{
    VarArray result;
    outputs.unmarkAncestors();
    inputs.setMark();
    result = outputs.sources();
    outputs.unmarkAncestors();
    return result;
}

void operator<<(VarArray& ar, const Array<Vec>& values)
{
    int n = ar.size();
    if(values.size()!=n)
        PLERROR("In operator<<(VarArray&, const Array<Vec>&) sizes of arrays differ (VarArray:%d Array<Vec>:%d)",ar.size(),values.size());
    for(int k=0; k<n; k++)
    {
        Vec& ar_v = ar[k]->value;
        Vec& v = values[k];
        if(ar_v.size() != v.size())
            PLERROR("In operator<<(VarArray&, const Array<Vec>&) sizes of var array and vector differ.  "
                    "(VarArray length:%d, in Array<Vec>, Vec length:%d)",ar_v.size(),v.size());
        ar_v << v;
    }
}

void operator>>(VarArray& ar, const Array<Vec>& values)
{
    int n = ar.size();
    if(values.size()!=n)
        PLERROR("In operator<<(VarArray&, const Array<Vec>&) sizes of arrays differ (VarArray:%d Array<Vec>:%d)",ar.size(),values.size());
    for(int k=0; k<n; k++)
    {
        Vec& ar_v = ar[k]->value;
        Vec& v = values[k];
        if(ar_v.size() != v.size())
            PLERROR("In operator<<(VarArray&, const Array<Vec>&) sizes of var array and vector differ.  "
                    "(VarArray length:%d, in Array<Vec>, Vec length:%d)",ar_v.size(),v.size());
        ar_v >> v;
    }
}

void printInfo(VarArray& a) { a.printInfo(); }

void printInfo(VarArray inputs, const Var& output,bool show_gradients)
{
    inputs.setMark();
    output->markPath();
    VarArray proppath;
    output->buildPath(proppath);
    if (show_gradients)
    {
        // Warning: we should probably clear the gradients along the proppath before doing this
        proppath.fbprop();
    }
    else
        proppath.fprop();
    proppath.printInfo(show_gradients);
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
