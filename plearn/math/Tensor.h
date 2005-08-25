// -*- C++ -*-

// TTensor.h: Definition of a tensor
// Copyright (c) 2002 by Julien Keable and Pascal Vincent

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

#ifndef TENSOR_INC
#define TENSOR_INC

#include <iostream.h>
#include <plearn/base/Storage.h>
#include <plearn/base/TinyVector.h>
#include <vector>

using namespace std;

namespace PLearn {

template <class T>
class TTensor;

typedef TinyVector<int, 7> IVec;

/*
  class: TTensorElementIterator 
  Iterator that iterates over all elements of a TTensor
*/

template<class T>
class TTensorElementIterator
{
private:
    // tensor from which we iterate
    TTensor<T>* tensor;
    IVec position_;
    // the element 'i' in the following vector is the memory leap from the last elem. of dimension i to the next element 
    // computed on creation to accelerate computations in operator++
    IVec stride_minus_width;
    T* ptr; // current element pointer
    // is the iterator past the last tensor element?
    bool end_met;

public:

    inline TTensorElementIterator()
        :tensor(NULL),ptr(NULL),end_met(false)
    {
    }

    inline TTensorElementIterator(TTensor<T>* tensor_, const IVec& pos)
        :tensor(tensor_),position_(pos),end_met(false)
    {
        ptr = tensor->data()+tensor->linearIndex(pos);
        stride_minus_width.resize(tensor->ndims());
        for(int i=0;i<(signed)tensor->ndims()-1;i++)
            stride_minus_width[i] = tensor->stride_[i+1] - tensor->stride_[i] * tensor->width_[i];
    }

    inline TTensorElementIterator<T>& operator++();

    inline T* operator->() const
    { return ptr; }
  
    inline T& operator*() const
    { return *ptr; }

    inline bool operator==(const TTensorElementIterator& other)
    { return ptr==other.ptr; }

    inline bool operator!=(const TTensorElementIterator& other)
    { return ptr!=other.ptr; }
    
    inline IVec position() const {return position_;}

    inline bool end() const {return end_met;}
};

/*
  class TTensorSubTensorIterator :
  this iterator iterates on subTensor of a parent Tensor.
  For exemple, you might have a 3d tensor (x,y,z) and iterate over all (x,y) planes.
  To build such an object, call TTensor::getSubTensorIterator(IVec) with a vector
  containing non-zero values for the dimensions that the subTensors spread on
*/

template<class T>
class TTensorSubTensorIterator
{
private:
    // tensor from which we iterate
    TTensor<T>* tensor;
    IVec position_;
    IVec dimensions_;
    // is the iterator past the last element?
    bool end_met;

public:

    inline TTensorSubTensorIterator(TTensor<T>* tensor_, const IVec& dim)
        :tensor(tensor_),dimensions_(dim),end_met(false)
    {
        position_=IVec((unsigned)tensor->ndims(),0);
        for(int i=0;i<tensor->ndims();i++)
            if(dimensions_[i]!=0)
                position_[i]=-1;
    }

    inline TTensorSubTensorIterator<T>()
        :tensor(NULL),end_met(false){}

    inline IVec position() const {return position_;}

    inline TTensorSubTensorIterator<T>& operator++();

    inline TTensor<T> operator*() const
    {
        IVec from(tensor->ndims()),length(tensor->ndims());
        for(int i=0;i<tensor->ndims();i++)
        {
            if(position_[i]==-1)
            {
                from[i]=0;
                length[i]=tensor->width_[i];
            }
            else
            {
                from[i]=position_[i];
                length[i]=1;
            }
        }
        return tensor->subTensor(from,length);
    }

    inline bool end() const {return end_met;}
};



/*
  class Tensor: 
  A tensor is a n-dimension matrix. The number of dimensions is determined
  at run-time upon object creation. The tensor, as of today, cannot be resized 
  (that needs to be implemented, the default constructor is currently pointless)
  You can extract a subTensor by specifying two vectors : one for the start 
  indices and one for the widths of the subTensor on each dimensions.
  Dimensions with width == 0 are not 'dimensions' anymore. E.g: a(x,y) plane 
  extracted from a 3d tensor (x,y,z) has only 2 dimensions.
*/
 
template <class T>
class TTensor
{
    friend class TTensorElementIterator<T>;
    friend class TTensorSubTensorIterator<T>;
protected:
    int offset_; /*!<  the displacement to do with respect to storage->data  */
    IVec stride_; /*!< the linear memory displacement, for each dimension, to the next element in the same dimension */
    IVec width_; /*!<  the actual widths of each dimensions of the tensor */
    PP< Storage<T> > storage; /*!<  where the data is really kept  */
  
public:
    typedef TTensorElementIterator<T> iterator; // iterator over elements
    typedef TTensorSubTensorIterator<T> subTensorIterator; // iterator over elements
    
    TTensor<T>()
        :offset_(0), stride_(0), width_(0)
    {}

    TTensor<T>(IVec len)
        :offset_(0), stride_(0), width_(len)
    { 
        stride_.resize(len.size());
        for(int i=0;i<ndims();i++)
        {
            stride_[i]=1;
            for(int j=i-1;j>=0;j--)
                stride_[i]*=len[j];
        }
        storage = new Storage<T>(totalElements());
    }

    TTensor<T>(const IVec& len, const T& init_value)
        :offset_(0), stride_(0), width_(len)
    { 
        stride_.resize(len.size());
        for(int i=0;i<ndims();i++)
        {
            stride_[i]=1;
            for(int j=i-1;j>=0;j--)
                stride_[i]*=len[j];
        }
        storage = new Storage<T>(totalElements());
        fill(init_value);
    }
    
    //! don't call this from a subTensor !!
    int resize(const IVec& len)
    {
        width_=len;
        stride_.resize(len.size());
        for(int i=0;i<ndims();i++)
        {
            stride_[i]=1;
            for(int j=i-1;j>=0;j--)
                stride_[i]*=len[j];
        }
        storage = new Storage<T>(totalElements());
        offset_=0;
    }
    
    int ndims() const { return width_.size();}

    IVec width() const {return width_;}
    IVec sizes() const {return width_;}
    int size(int k) const { return width_[k]; }

    int totalElements() const 
    {
        if(ndims()==0)
            return 0;
        int te=1;
        for(int i=0;i<ndims();i++)
            te*=width_[i];
        return te;
    }

    int linearIndex(const IVec& pos) const
    {
        int idx=0;
        for(int i=0;i<ndims();i++)
            idx+=pos[i]*stride_[i];
        return idx;
    }
    
    int linearIndex(const vector<int>& pos) const
    {
        int idx=0;
        for(int i=0;i<ndims();i++)
            idx+=pos[i]*stride_[i];
        return idx;
    }
    
      
    // extract a sub tensor from this one. 'from' is a vector of start indices in each dimension, len is the length of each new dimension
    // Any dimension of length 0 will not be considered as a dimension anymore (we'll keep a slice at the particular position given by from).
    // i.e. : a subTensor extracted from a 3d tensor (x,y,z) with the len of the z dimension set to 0 will be a 2d tensor
    TTensor<T> subTensor(const IVec& from, const IVec& len)
    {
        TTensor<T> subt = *this;
        subt.width_ = len;
        subt.offset_ = linearIndex(from);
        
        IVec idx;
        for(int i=0;i<ndims();i++)
        {
            if(from[i]<0 || from[i]+len[i]>width_[i] || len[i]<0)
                PLERROR("TTensor::subTensor : at index %i : from, len, width = %i %i %i",i,from[i],len[i],width_[i]);
            if(len[i]>0) // skip the 0 dimensions
                idx.push_back(i);      
        }
        // if idx is empty, it is because all lengths are 0: we have a scalar
        if(idx.empty())
            idx.push_back(0);
        subt.selectDimensions(idx);
        return subt;
    }



    // NOTE: (Pascal) This call has been deprecated because the throw_useless_dimension behaviour is unsatisfactory and a potential for nasty bugs. 
    // I've rewritten another subTensor method with a clearer semantic: a len of 1 means keep that dimension (as a single element with that particular value)
    // and a len of 0 means throw away that dimension.
    // extract a sub tensor from this one. 'from' is a vector of start indices in each dimension, len is the length of each new dimension
    // ** NOTE :
    // If 'throw_useless_dimensions' == true, any dimension of length 1 will not be considered as a dimension anymore
    // i.e. : a subTensor extracted from a 3d matrix (x,y,z) with a fixed z value will be a 2d matrix rather than a 3d matrix 
    // with a width of one in the 'z' dimension
    TTensor<T> DEPRECATEDsubTensor(const IVec& from, const IVec& len, bool throw_useless_dimensions=true)
    {
        TTensor<T> subt = *this;
        subt.width_ = len;
        subt.offset_ = linearIndex(from);
        
        IVec idx;
        for(int i=0;i<ndims();i++)
        {
            if(from[i]<0 || from[i]+len[i]>width_[i] || len[i]<0)
                PLERROR("TTensor::subTensor : at index %i : from, len, width = %i %i %i",i,from[i],len[i],width_[i]);
            if(len[i]>1 || !throw_useless_dimensions)
                idx.push_back(i);      
        }
        // if idx is empty, it is because all lengths are 1 since a single element was selected,
        // so we need to create a single dimension.
        if(idx.empty())
            idx.push_back(0);
        subt.selectDimensions(idx);
        return subt;
    }

    TTensor<T> operator[](int i)
    {
        IVec from;
        IVec len;
        from.push_back(i);
        len.push_back(0);
        for(int k=1; k<ndims(); k++)
        {
            from.push_back(0);
            len.push_back(size(k));
        }
        return subTensor(from,len);
    }

    void selectDimensions(const IVec& dim)
    {
        IVec newwidth,newstride;
        for(int i=0;i<(signed)dim.size();i++)
        {
            newwidth.push_back(width_[dim[i]]);
            newstride.push_back(stride_[dim[i]]);
        }
        stride_=newstride;
        width_=newwidth;
    }
   
    T& operator()(const IVec& pos) const {return (*storage)[linearIndex(pos)];}
    T& operator()(const vector<int>& pos) const {return (*storage)[linearIndex(pos)];}

    void fill(const T& val)
    {
        int te=totalElements();
        for(int i=0;i<te;i++)
            (*storage)[i]=val;
    }
    
    inline T* data() const
    {
#ifdef BOUNDCHECK
        if(!storage)
            PLERROR("IN TTensor::data()\nAttempted to get a pointer to the data of an empty matrix");
#endif
        return storage->data+offset_; 
    }

    IVec lastElementPos() const
    {
        IVec v;
        v.resize(ndims());
        for(int i=0;i<ndims();i++)
            v[i]=(width_[i]-1);
        return v;
    }
    
    //! returns an iterator over elements
    inline iterator begin()
    { 
        return iterator(this, IVec((unsigned)ndims(),0));
    }
    
    // note that Tensor iterators know if they're past the end of data by themselves
    // this method is only implemented to provide a standard interface. Check for iterator.end()
    inline iterator end()
    { 
        return iterator(this,lastElementPos());
    }
    
    //! returns an iterator over subTensors
    inline subTensorIterator getSubTensorIterator(const IVec& v)
    { 
        return subTensorIterator(this, v);
    }

};

typedef TTensor<real> Tensor;

template<class T>
inline TTensorElementIterator<T>& TTensorElementIterator<T>::operator++()
{ 
    ptr+=tensor->stride_[0];
    position_[0]++;
    // if we hit the end of the first dimension
    // we need to compute a few things
    if(position_[0]==tensor->width_[0])
    {
        bool found=false;
        int idx=0;
        // find the lowest index of the position vector we can increment
        // a.k.a get the next element of the tensor
        while(!found && idx<(signed int)(tensor->ndims()-1))
        {
            idx++;
            found = position_[idx]<tensor->width_[idx]-1;
        }
        if(found)
        {
            position_[idx]++;
            for(int i=0;i<idx;i++)
            {
                ptr+=stride_minus_width[i];
                position_[i]=0;
            }      
        }
        else 
            // all elements of the tensor have been visited           
            end_met=true;
    }
    return *this;
}
    

template<class T>
inline TTensorSubTensorIterator<T>& TTensorSubTensorIterator<T>::operator++()
{
    bool found=false;
    int idx=-1;
    // find the lowest index of the position vector we can increment
    while(!found && idx<(signed int)(tensor->ndims()-1))
    {
        idx++;
        found = position_[idx]!=-1 && position_[idx]<tensor->width_[idx]-1;
    }
    if(found)
    {
        position_[idx]++;
        for(int i=0;i<idx;i++)
            if(position_[i]!=-1)
                position_[i]=0;
    }
    else 
        // all subTensors of the tensor have been visited           
        end_met=true;
    
    return *this;
}

 
};

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
