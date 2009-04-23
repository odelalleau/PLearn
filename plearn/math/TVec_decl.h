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
 * AUTHORS: Pascal Vincent & Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */



#ifndef TVec_decl_INC
#define TVec_decl_INC

#include <algorithm>
#include <iterator>
#include <numeric>
#include <functional>
#include <sstream>

#include <plearn/base/general.h>
#include <plearn/base/Storage.h>
#include <plearn/base/Range.h>
#include <plearn/io/plstreams.h>
#include <plearn/io/PStream.h>
#include <plearn/io/openString.h>

namespace PLearn {
using namespace std;

// predeclarations
template<class T> class TMat;
class Variable;
class VarArray;
class QRchunker;
class QCchunker;

template <class T>
class TVec
{
    friend class TMat<T>;

    friend class Variable; //!<  for makeShared hack... (to be cleaned)
    friend class VarArray; //!<  for makeShared hack... (to be cleaned)

    //!  The following are for hacky interfaces that for some obscure and dirty reason access offset_ directly
    friend class QRchunker;
    friend class QCchunker;

protected:
    int      length_;  /*!<  The length of the data  */
    int      offset_;  /*!<  the displacement to do with respect to storage->data  */
    PP< Storage<T> > storage; /*!<  where the data is really kept  */

public:

    typedef T value_type;
    typedef int size_type;
    typedef T* iterator;
    typedef const T* const_iterator;

    inline iterator begin() const 
    { 
        if (storage.isNull()) return 0;
        return storage->data+offset_; 
    }

    inline iterator end() const 
    { return begin()+length(); }

    // norman: added explicit cast before vec.size()
    //         but beware if you're creating a constructor for storage that takes size_type
    //         because int could be different from size_type!
    inline TVec(const vector<T> & vec)
        :length_((int)vec.size()), offset_(0),
         storage(new Storage<T>((int)vec.size()))
    {
        for(int i=0;i<length_;i++)
            (*this)[i]=vec[i];
    }

    inline TVec() 
        :length_(0),offset_(0)
    {}

    inline explicit TVec(int the_length) 
        :length_(the_length), offset_(0),
         storage(new Storage<T>(the_length))
    {}

    //! Builds a Vec of specified length with all values initialised with the given value 
    inline TVec(int the_length, const T& init_value) 
        :length_(the_length), offset_(0),
         storage(new Storage<T>(the_length))
    { fill(init_value); }

    //!  Builds a Vec containing values ranging from start to stop with step
    //!  e.g., Vec(0,n-1,1) returns a vector of length() n, with 0,1,...n-1.
    //!  creates range (start, start+step, ..., stop)
    TVec(const T& start, const T& stop, const T& step);

    //!  Builds a TVec which data is the_data 
    inline TVec(int the_length, T* the_data)
        :length_(the_length), offset_(0),
         storage(new Storage<T>(the_length, the_data))
    {}

    //!  NOTE: COPY CONSTRUCTOR COPIES THE TVec STRUCTURE BUT NOT THE DATA
    inline TVec(const TVec<T>& other)
        :length_(other.length()), offset_(other.offset()),
         storage(other.storage)
    {}

    //!  NOTE: operator= COPIES THE TVec STRUCTURE BUT NOT THE DATA (use operator<< to copy data)
    inline const TVec<T>& operator=(const TVec<T>& other)
    {
        storage = other.storage;
        length_ = other.length_;
        offset_ = other.offset_;
        return *this;
    }

    operator vector<T>() const
    {    
        int n = length_;
        vector<T> res(n);
        if(n>0)
        {
            T* ptr = data();
            for(int i=0; i<n; i++)
                res[i] = *ptr++;
        }
        return res;
    }

    bool hasMissing() const
    {
        iterator it = begin();
        iterator itend = end();
        for(; it!=itend; ++it)
            if(is_missing(*it))
                return true;
        return false;
    }

    inline int size() const { return length_; }
    inline int length() const { return length_; }
    inline int capacity() const { return storage.isNotNull() ? storage->length()-offset_ : 0; }
    inline int offset() const { return offset_; }

    inline PP< Storage<T> > getStorage() const { return storage; }

    //!  Makes sure the allocated memory for this vector is exactly length()
    void compact()
    {
        if(storage && storage->length() != length())
        {
            if(storage->usage()>1)
                PLERROR("IN Mat::compact() compact operation not allowed when matrix storage is shared (for obvious reasons)");
            operator=(copy());
        }
    }

    //!  used by Hash  (VERY DIRTY: TO BE REMOVED [Pascal])
    inline operator char*() const { if(isNull()) return 0; else return (char*)data(); }

    // norman: removed const. With inline is useless (and .NET doesn't like it)
    // Old code:
    //inline const size_t byteLength() const { return length()*sizeof(T); }
    inline size_t byteLength() const { return length()*sizeof(T); }

    /*!     Resizes the TVector to a new length
      The underlying storage is never shrunk and
      it is grown only if it is not already big enough
      When grown, extra entries are allocated to anticipate further grows
    */
    inline void resize(int newlength, int extra=0)
    {
#ifdef BOUNDCHECK
        if (newlength<0 || extra<0)
            PLERROR("IN TVec::resize(int newlength)\nInvalid argument (<0)");
#endif
        if (newlength == length_ && extra == 0) {
            // No need to do anything.
            return;
        }
        if (storage.isNull() && (newlength>0 || extra>0))
        {
            offset_ = 0;
            length_ = newlength;
            Storage<T>* s = new Storage<T>(newlength + extra);          
            storage = s;
        }
        else
        {
            if (storage.isNotNull() && (newlength > capacity()))
                storage->resize (offset_ + newlength + extra);
            length_ = newlength;
        }
    }

    //! writes the Vec to the PStream:
    //! Note that users should rather use the form out << v;
    void write(PStream& out) const
    {
        const TVec<T>& v = *this; // simple alias
        if(storage && 
           ( out.implicit_storage 
             || out.outmode==PStream::raw_ascii
             || out.outmode==PStream::raw_binary
             || out.outmode==PStream::pretty_ascii ) )
            writeSequence(out,v);
        else if(storage.isNull() && out.outmode!=PStream::plearn_binary)
            out.write("[]\n");
        else // write explicit storage
        {
            out.write("TVec("); 
            out << v.length();
            out << v.offset();
            out << v.getStorage();
            out.write(")\n");
        }
    }

    //! reads the Vec from the PStream:
    //! Note that users should rather use the form in >> v;
    void read(PStream& in)
    {
        TVec<T>& v = *this; // simple alias
        switch(in.inmode)
        {
        case PStream::raw_ascii:
        case PStream::raw_binary:      
            readSequence(in, v);

        case PStream::plearn_ascii:
        case PStream::plearn_binary:
        {
            in.skipBlanksAndComments();
            int c = in.peek();
            if(c!='T') // implicit storage
                readSequence(in,v);
            else // explicit storage
            {
                char word[6];
                // !!!! BUG: For some reason, this hangs!!!
                // in.read(word,5);
                for(int i=0; i<5; i++)
                    in.get(word[i]);
                word[5]='\0';
                if(strcmp(word,"TVec(")!=0)
                    PLERROR("In operator>>(PStream&, TVec&) '%s' not a proper header for a TVec!",word);
                // v.storage = 0;
                in.skipBlanksAndCommentsAndSeparators();
                in >> v.length_;
                in.skipBlanksAndCommentsAndSeparators();
                in >> v.offset_;
                in.skipBlanksAndCommentsAndSeparators();
                in >> v.storage;
                in.skipBlanksAndCommentsAndSeparators();
                int c = in.get(); // skip ')'
                if(c!=')')
                    PLERROR("In operator>>(PStream&, TVec&) expected a closing parenthesis, found '%c'",c);
            }
        }
        break;
      
        default:
            PLERROR("In TVec<T>::read(PStream& in)  unknown inmode!!!!!!!!!");
            break;
        }
    }

    // The following methods are deprecated, and just call corresponding functions.
    // Please call those functions directly in new code
    void save(const string& filename) const { savePVec(filename, *this); }
    void load(const string& filename) { loadPVec(filename, *this); }
    
    //!  Returns a sub-TVector
    TVec<T> subVec(int newstart, int newlength) const
    {
#ifdef BOUNDCHECK
        if(newstart+newlength>length() || newlength<0)
            PLERROR("TVec::subVec(int newstart, int newlength) OUT OF BOUNDS OR <0 length()"
                    " length()=%d; newstart=%d; newlength=%d.", length(), newstart, newlength);
#endif
        TVec<T> subv = *this;
        subv.length_ = newlength;
        subv.offset_ += newstart;
        return subv;
    }

    /**
     *  Modify the current Vec to point to a subset of itself.  This is useful
     *  if we are handed a 'large' Vec, but only want a subset without incurring
     *  the performance hit of a temporary object.
     */
    void subVecSelf(int newstart, int newlength)
    {
#ifdef BOUNDCHECK
        if(newstart+newlength>length() || newlength<0)
            PLERROR("TVec::subVecSelf(int newstart, int newlength) OUT OF BOUNDS OR <0 length()"
                    " length()=%d; newstart=%d; newlength=%d.", length(), newstart, newlength);
#endif
        length_ = newlength;
        offset_ += newstart;
    }

    /*! ************
      Deep copying
      ************
      */
    void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    /*! Notice that deepCopy of a Vec returns a Vec rather than a Vec*. The
      reason for this being that a Vec is already some kind of "smart
      pointer" to an underlying Storage
    */
    TVec<T> deepCopy(CopiesMap& copies) const;


    inline TVec<T> subVec(Range r) { return subVec(r.start, r.length); }
    
    //!  Returns a TVector made up of the two (or more) input TVec
    void concat(const TVec<T>& input1, const TVec<T>& input2)
    {  
        int l1 = input1.length();
        int l2 = input2.length();
        resize(l1+l2);
        for(int i=0;i<l1;i++) (*this)[i] = input1[i];
        for(int i=0;i<l2;i++) (*this)[l1+i] = input2[i];
    }

    void concat(const TVec<T>& input1, const TVec<T>& input2, const TVec<T>& input3)
    {
        int l1 = input1.length();
        int l2 = input2.length();
        int l3 = input3.length();
        resize(l1+l2+l3);
        for(int i=0;i<l1;i++) (*this)[i] = input1[i];
        for(int i=0;i<l2;i++) (*this)[l1+i] = input2[i];
        for(int i=0;i<l3;i++) (*this)[l1+l2+i] = input3[i];
    }

    void concat(const TVec<T>& input1, const TVec<T>& input2, const TVec<T>& input3, const TVec<T>& input4)
    {
        int l1 = input1.length();
        int l2 = input2.length();
        int l3 = input3.length();
        int l4 = input4.length();
        resize(l1+l2+l3+l4);
        for(int i=0;i<l1;i++) (*this)[i] = input1[i];
        for(int i=0;i<l2;i++) (*this)[l1+i] = input2[i];
        for(int i=0;i<l3;i++) (*this)[l1+l2+i] = input3[i];
        for(int i=0;i<l4;i++) (*this)[l1+l2+l3+i] = input4[i];
    }

    //!  The returned TMat will view the same data
    inline TMat<T> toMat(int newlength, int newwidth) const;

    //!  returns a newly created copy of this TVec
    inline TVec<T> copy() const
    {
        TVec<T> freshcopy(length());
        freshcopy << *this;
        return freshcopy;
    }

    //!  copy from a C TVector starting at x of length() n
    void copyFrom(const T* x, int n) const
    {  
#ifdef BOUNDCHECK
        if(n != length())
            PLERROR("IN TVec::copyFrom(T* x, int n)\nVecs do not have the same length()");
#endif
        if (n == 0)
            return; // Nothing to copy.
        T* v1 = data(); //!<  get data start

        // The following was compiled into an inefficient loop.  Modern C++
        // compilers transform 'copy' into memmove whenever possible, so use
        // that.
        //
        // for(int i=0; i<n; i++)
        //    v1[i] = x[i];

        std::copy(x, x+n, v1);
    }

    //!  copy to a C TVec starting at x
    void copyTo(T* x) const
    {
        T* v1 = data(); // get data start
        if (! v1)
            return;

        // The following was compiled into an inefficient loop.  Modern C++
        // compilers transform 'copy' into memmove whenever possible, so use
        // that.
        //
        // for(int i=0; i<length(); i++)
        //     x[i] = v1[i];

        std::copy(v1, v1+length(), x);
    }

    /*!         make the storage point to this address and
      copy current value to it (i.e. without changing
      current contents)
    */
    void makeSharedValue(T* x, int n)
    {
#ifdef BOUNDCHECK
        if(n != length())
            PLERROR("IN TVec::makeSharedValue(T* x, int n)\nn(%d)!=length_(%d)",
                    n,length());
        if(offset_!=0)
            PLERROR("IN TVec::makeSharedValue(T* x, int n)\noffset should be 0.");
#endif
        T* v = data(); //!<  get data start
        for(int i=0; i<n; i++)
            x[i] = v[i];
        storage->pointTo(n,x);
    }

    bool isNull() const 
    { return storage.isNull(); }

    bool isNotNull() const
    { return storage.isNotNull(); }

    bool isEmpty() const
    { return length_==0; }

    bool isNotEmpty() const
    { return length_!=0; }
      
    /*! This method is commented out because it is much too dangerous:
      it allows a TVec to be converted into an int, which can cause some
      very weird bugs that the compiler would have caught otherwise.*/
    /*
    //!  To allow if(v) statements
    operator bool() const
    { return isNotEmpty(); }
    */
      
    //!  To allow if(!v) statements
    bool operator!() const
    { return isEmpty(); }

    // for compatibility with Array
    TVec<T>* operator->()
    { return this; }

    //! Fills the vector with the given value; no-op if vector is null
    inline void fill(const T& value) const
    {
        if (isNotEmpty())
            fill_n(data(), length(), value);
    }

    //! Fills the vector, putting startval in its first element
    //! and increments of step in subsequent elements
    void fill(const T& startval, const T& step)
    {
        iterator it = begin();
        iterator itend = end();
        for(T val=startval; it!=itend; ++it, val+=step)
            *it = val;
    }

    inline void clear() const
    { if(isNotEmpty()) clear_n(data(),length()); }

    //!  inserts element at position (actually between values at position-1 and posiion). Length is increased by 1.
    inline void insert(int position, T value)
    {
#ifdef BOUNDCHECK
        if(position<0 || position>length())
            PLERROR("OUT OF BOUNDS in TVec::insert");
#endif
        resize(length()+1);
        T* v = data();
        for(int i=length()-1; i>position; i--)
            v[i] = v[i-1];
        v[position] = value;
    }

    //!  removes element at position, Length is decreased by 1
    inline void remove(int position)
    {
#ifdef BOUNDCHECK
        if(position<0 || position>=length())
            PLERROR("OUT OF BOUNDS in Vec::remove");
#endif
        T* v = data();
        for(int i=position; i<length()-1; i++)
            v[i] = v[i+1];
        resize(length()-1);
    }

    //! Returns an index vector I so that (*this)(I) returns a sorted version
    //! of this vec in ascending order. If stable is true, will return a stable sort
    TVec<int> sortingPermutation(bool stable = false, bool missing = false) const;
    
    //! Return the first index where the value COULD be.
    //! If mulitiple value present, return the first index
    //! If the value is not present, return the first index with data bigger then the value.
    int findSorted(T value) const
    {
        if (isEmpty())
            return 0;
 
        pair<iterator, iterator> range =
            equal_range(begin(), end(), value);
        
        return int(range.first - begin());
    }
  
    inline void insertSorted(T value, bool uniq)
    {
        int i = findSorted(value);
        if(!uniq || i==length() || (*this)[i]!=value)
            insert(i,value);
    }

    inline void removeSorted(T value)
    {
        int i = findSorted(value);
        if(i<length() && (*this)[i]==value)
            remove(i);
    }


    inline void append(const T& newval)
    {
        //we do this as an speed optimization. I see a 3.5% speed up...
        //g++4.1 don't seam to inline resize event in heavy loop of append.
        //maybe this is the cause of the speed up?
        if (storage.isNotNull() && (length() < capacity())){
            length_++;
        }else
            resize(length()+1, length());
        lastElement() = newval;
    }

    //! for compatibility with Array
    void append(const vector<T>& newvec)
    {
        int currentsize = length();
        if (currentsize + newvec.size() == 0)
            return;
        resize(currentsize + newvec.size(), currentsize + newvec.size());
        T* v = data();
        for (unsigned int i=0; i<newvec.size(); ++i)
            v[currentsize+i] = newvec[i];
    }

    //! For compatibility with Array.
    inline void appendIfNotThereAlready(const T& newval)
    {
        if(length()>0) {
            T* v = data();
            for (int i=0;i<length();i++)
                if (newval==v[i]) return;
        }
        append(newval);
    }

    //! stl compatibility
    inline void push_back(const T& newval)
    { append(newval); }

    inline void pop_back()
    {
        if(length_ <= 0)
            PLERROR("In TVec::pop_back already empty!");
        length_ -= 1; 
    } 

    //! stack interface compatibility
    inline void push(const T& newval)
    { append(newval); }

    inline T pop()
    { T res = lastElement(); pop_back(); return res; }

    inline T& top() const
    { return lastElement(); }

    inline void append(const TVec<T>& values)
    {
        int oldLength = length();
        if (values.length() == 0)
            return;
        resize(oldLength+values.length(), oldLength+values.length());
        T* v = data()+oldLength;
        T* newv = values.data();
        for(int i=0; i<values.length(); i++)
            v[i] = newv[i];
    }

    inline T& operator[](int i) const
    {
#ifdef BOUNDCHECK
        if(i<0 || i>=length())
            PLERROR("OUT OF BOUND ACCESS %d IN TVec(%d)::operator[]",i,length());
#endif
        return storage->data[i+offset_]; 
    }

    // norman: added operator[] for unsigned int
    inline T& operator[](unsigned int i) const
    {
#ifdef BOUNDCHECK
        // norman: added explicit cast
        if(i<0 || i>=(unsigned int)length())
            PLERROR("OUT OF BOUND ACCESS %d IN TVec(%d)::operator[]",i,length());
#endif
        return storage->data[i+offset_]; 
    }

    inline T& lastElement() const
    { 
#ifdef BOUNDCHECK
        if(length()==0)
            PLERROR("TVec::lastElement() - can't access last"
                    " element of TVec as there is 0 element!");
#endif
        return storage->data[offset_+length()-1];
    }

    inline T& firstElement() const
    { 
#ifdef BOUNDCHECK
        if(length()==0)
            PLERROR("TVec::firstElement() - can't access first"
                    " element of TVec as there is 0 element!");
#endif 
        return storage->data[offset_]; 
    }

    inline T& front() const { return firstElement(); }
    inline T& back() const { return lastElement(); }

    // for compatibility with Array
    inline T& first() const { return firstElement(); }
    inline T& last() const { return lastElement(); }


    //! Deprecated: use the select function instead
    template<class I>
    inline void operator()(const TVec<I>& indices, TVec<T>& destination) const
    { selectElements(*this, indices, destination); }

    /*!         select the elements of the source (this) as specified by the
      TVector of indices (between 0 and this->length()-1) into
      the returned TVector (which will have the same length()
      as the indices TVector).
    */
    template<class I>
    inline TVec<T> operator()(const TVec<I>& indices) const
    {
        TVec<T> result(indices.length());
        selectElements(*this, indices, result);
        return result;
    }

    //!  Returns a pointer to the beginning of the TVector data
    inline T* data() const
    {
#ifdef BOUNDCHECK
        if(storage.isNull())
            PLERROR("IN TVec::operator()\nAttempted to get a pointer to the data of an empty TVec");
#endif
        return storage->data+offset_; 
    }

    //!  swaps first and last element, second and second last, etc... (mirror symmetry). 
    inline void swap()
    {
        if (isEmpty()) return;
        int half = length()/2;
        T* ptr = data();
        for(int i=0; i<half; i++)
            std::swap(ptr[i],ptr[length()-i-1]);
    }

    //!  return a vector with 1's when (*this)[i]==value for all i, 0 otherwise
    TVec<bool> operator==(const T& value) const
    {
        TVec<bool> r(length(), false);
        //elementsEqualTo(*this,value,r);
        for (int i=0; i<length(); i++)
        {
            if ((*this)[i] == value) r[i] = true;
        }
        return r;
    }

    //!  return true if (*this)[i]==value[i] for all i, 0 otherwise
    bool operator==(const TVec<T>& value) const
    {
        if (value.isEmpty() && isEmpty()) return true;
        if (value.length()!=length()) return false;
        T* x=data();
        T* y=value.data();
        for (int i=0;i<length();i++)
            if (x[i]!=y[i]) return false;
        return true;
    }
    bool operator!=(const TVec<T>& value) const { return !((*this)==value); }

    //!  same as operator== but dealing with NaN and Inf
    bool isEqual(const TVec<T>& value, bool ignore_missing=false) const
    {
        if (value.isEmpty() && isEmpty()) return true;
        if (value.length() != length())   return false;
        if (!ignore_missing) return ((*this)==value);

        T* x = data();
        T* y = value.data();
        for (int i=0;  i<length();  i++)
        {
            real x_i = x[i];
            real y_i = y[i];

            // For NaN values
            if (isnan(x_i))
            {
                if (isnan(y_i))
                    continue;
                else
                    return false;
            }
            else if (isnan(x_i)) return false;

            // For Inf values
            if (isinf(x_i))
            {
                if (isinf(y_i))
                    continue;
                else
                    return false;
            }
            else if (isinf(x_i)) return false;

            if (x_i != y_i) return false;
        }
        return true;
    }

    //! Return true if 'element' is in the TVec and false otherwise.
    bool contains(const T& element) const
    {
        return find(element) != -1;
    }

    //!  return the set of indices whose corresponding values are "element". 
    TVec<int> findIndices(const T& element)
    {
        TVec<int> indices(0);
        if (!isEmpty())
        {
            T *v = data();
            for (int i=0; i<length(); i++)
                if (v[i]==element)
                    indices.append(i);
        }
        return indices;
    }

    TVec<int> findIndices(const TVec<T>& elements)
    {
        TVec<int> indices(0);
        if (!isEmpty())
        {
            T *v = data();
            for (int i=0; i<length(); i++)
                for (int j=0, m=elements.length(); j<m; j++)
                    if (v[i] == elements[j])
                    {
                        indices.append(i);
                        break;
                    }
        }
        return indices;
    }

    //!  Returns the position of the first occurence of element
    //!  in the vector or -1 if it never occurs
    int find(const T& element, int start=0) const
    {
        if (length()==0) return -1;
        T *v = data();
        for (int i=start; i<length(); i++)
            if(v[i]==element)
                return i;
        return -1;
    }

    TVec<int> find(TVec<T> elements)
    {
        TVec<int> indices(elements.length(),-1);
        if (length()==0) return indices;
        T *v = data();
        for (int i=0, m=elements.length(); i<m; i++)
            for (int j=0; j<length(); j++)
                if (v[j] == elements[i])
                {
                    indices[i] = j;
                    break;
                }
        return indices;
    }

    //! Returns the number of occurrences of "element"
    int count(const T& element)
    {
        int result = 0;
        if (!isEmpty())
        {
            T *v = data();
            for (int i=0; i<length(); i++)
                if (v[i]==element)
                    result++;
        }
        return result;
    }

    int count(const TVec<T>& elements)
    {
        int result = 0;
        if (!isEmpty())
        {
            T *v = data();
            for (int i=0; i<length(); i++)
                for (int j=0, m=elements.length(); j<m; j++)
                    if (v[i]==elements[j])
                    {
                        result++;
                        break;
                    }
        }
        return result;
    }


    //!  C++ stream output
    void print(ostream& out = cout) const; //!<  the data is printed on a single row, no newline
    void println(ostream& out = cout) const { print(out); out<<endl; } //!<  same with newline
    void printcol(ostream& out = cout) const; //!<  printed as a column
    void print(ostream& out, const string& separator) const; //!<  each value is printed with the given separator string between them

    void input(istream& in=cin) const;
    void input(PStream& in) const;

    // calls print with cerr, usefull with gdb (> call obj.debugprint() )
    void debugPrint(){print(cerr);}


    void operator<<(const string& datastring) const
    {
        // istrstream in(datastring.c_str());
        PStream in = openString(datastring,PStream::plearn_ascii);
        input(in);
    }

};

typedef TVec<real> Vec;

//! Same as fill(f) (will only work with Vec, because of a potential conflict
//! with T == string if we wanted to make it generic).
inline void operator<<(const Vec& v, real f)
{ v.fill(f); }

template<class T>
class TypeTraits< TVec<T> >
{
public:
    static inline string name()
    { return string("TVec< ") + TypeTraits<T>::name()+" >"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};


template <class T>
inline int sizeInBytes(const TVec<T>& x) { 
    int n=x.length();
    int s=sizeof(TVec<T>);
    if (n>0) s+=n*sizeInBytes(x[0]); 
    return s;
}

//! Returns a TVec which is a concatenation of v1 and v2
template<class T>
TVec<T> concat(const TVec<T>& v1, const TVec<T>& v2);

//! Returns a TVec which is a concatenation of v1,v2,v3
template<class T>
TVec<T> concat(const TVec<T>& v1, const TVec<T>& v2, const TVec<T>& v3);

//! Returns a TVec which is a concatenation of v1,v2,v3,v4
template<class T>
TVec<T> concat(const TVec<T>& v1, const TVec<T>& v2, const TVec<T>& v3, const TVec<T>& v4);

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
