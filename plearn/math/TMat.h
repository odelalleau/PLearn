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
   * $Id: TMat.h,v 1.19 2003/07/03 23:31:41 plearner Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/TMat.h */

#ifndef TMAT_INC
#define TMAT_INC

#include <iterator>
#include "general.h"
#include "Storage.h"
#include "Range.h"
#include "plstreams.h"
//#include "Array.h"

namespace PLearn <%
using namespace std;

template <class T> class TVec;
template <class T> class TMat;



// **************
// **** TVec ****
// **************

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

    inline TVec(const vector<T> & vec)
      :length_(vec.size()), offset_(0),
       storage(new Storage<T>(vec.size()))
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
      if(storage->length() != length())
      {
        if(storage->usage()>1)
          PLERROR("IN Mat::compact() compact operation not allowed when matrix storage is shared (for obvious reasons)");
        operator=(copy());
      }
    }

      //!  used by Hash  (VERY DIRTY: TO BE REMOVED [Pascal])
      inline operator char*() const { if(isNull()) return 0; else return (char*)data(); }

      inline const size_t byteLength() const { return length()*sizeof(T); }

/*!     Resizes the TVector to a new length
        The underlying storage is never shrunk and
        it is grown only if it is not already big enough
        When grown, extrabytes are allocated to anticipate further grows
*/
      inline void resize(int newlength, int extrabytes=0)
      {
#ifdef BOUNDCHECK
        if (newlength<0 || extrabytes<0)
          PLERROR("IN TVec::resize(int newlength)\nInvalid argument (<0)");
#endif
        if (storage.isNull() && newlength>0)
        {
          offset_ = 0;
          length_ = newlength;
          Storage<T>* s = new Storage<T>(newlength + extrabytes);          
          storage = s;
        }
        else
        {
          if (storage.isNotNull() && (newlength > capacity()))
            storage->resize (offset_ + newlength + extrabytes);
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
    
      
      void deepRead(istream& in, DeepReadMap& old2new)
      {
        readHeader(in, "TVec");
        PLearn::deepRead(in, old2new, length_);
        PLearn::deepRead(in, old2new, offset_);
        PLearn::deepRead(in, old2new, storage);
        readFooter(in, "TVec");
      }

      void deepWrite(ostream& out, DeepWriteSet& already_saved) const
      {
        writeHeader(out, "TVec");
        PLearn::deepWrite(out, already_saved, length_);
        PLearn::deepWrite(out, already_saved, offset_);
        PLearn::deepWrite(out, already_saved, storage);
        writeFooter(out, "TVec");
      }
           
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

    /*! ************
      Deep copying
      ************
      */
    void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

    /*! Notice that deepCopy of a Vec returns a Vec rather than a Vec*. The
      reason for this being that a Vec is already some kind of "smart
      pointer" to an underlying Storage
    */
    TVec<T> deepCopy(map<const void*, void*>& copies) const;


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
        T* v1 = data(); //!<  get data start
        for(int i=0; i<n; i++)
          v1[i] = x[i];
      }

      //!  copy to a C TVec starting at x
      void copyTo(T* x) const
      {
        T* v1 = data(); // get data start
        for(int i=0; i<length(); i++)
          x[i] = v1[i];
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
      
  //!  To allow if(v) statements
  operator bool() const
  { return isNotEmpty(); }
      
  //!  To allow if(!v) statements
  bool operator!() const
  { return isEmpty(); }

      // for compatibility with Array
      TVec<T>* operator->()
      { return this; }

      //! Fills the vector with the given value
      inline void fill(const T& value) const
      { fill_n(data(), length(), value); }

      //! Fills the vector, putting startval in its first element
      //! and increments of step in subsequent elements
      void fill(const T& startval, const T& step)
      {
        iterator it = begin();
        iterator itend = end();
        for(T val=startval; it!=itend; ++it, val+=step)
          *it = val;
      }

      //! same as fill(f)
      inline void operator=(const T& f) const
      { fill(f); }
  
      inline void clear() const
      { if(!isNull()) clear_n(data(),length()); }

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

      int findSorted(T value)
      {
        if(length()==0)
          return 0;
 
        T* v = data();
 
        // WARNING Someone please implement a real dichotomy search some day!
        int i=0;
        while(i<length() && v[i]<value)
          i++;
        return i;
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
      resize(length()+1);
      lastElement() = newval;
    }

    //! for compatibility with Array
    void append(const vector<T>& newvec)
    {
      T* v = data();
      int currentsize = length();
      resize(currentsize + newvec.size());
      for (unsigned int i=0; i<newvec.size(); ++i)
        v[currentsize+i] = newvec[i];
    }

    //! for compatibility with Array
    inline void appendIfNotThereAlready(const T& newval)
    {
      T* v = data();
      for (int i=0;i<length();i++)
        if (newval==v[i]) return;
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
      resize(oldLength+values.length());
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

      inline T& lastElement() const
      { return storage->data[offset_+length()-1]; }

      inline T& firstElement() const
      { return storage->data[offset_]; }

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
        int half = length()/2;
        T* ptr = data();
        for(int i=0; i<half; i++)
          std::swap(ptr[i],ptr[length()-i-1]);
      }

      //!  return a vector with 1's when (*this)[i]==value for all i, 0 otherwise
      TVec<bool> operator==(const T& value)
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
        if (value.length()!=length()) return false;
        T* x=data();
        T* y=value.data();
        for (int i=0;i<length();i++)
          if (x[i]!=y[i]) return false;
        return true;
      }
      bool operator==(TVec<T>& value) const
      {
        if (value.length()!=length()) return false;
        T* x=data();
        T* y=value.data();
        for (int i=0;i<length();i++)
          if (x[i]!=y[i]) return false;
        return true;
      }
      bool operator!=(const TVec<T>& value) const { return !((*this)==value); }

      //!  return true if element is in the TVec and false otherwise.
      bool contains(const T& element) const
      {
        if (length()==0) return false;
        bool contained=false;
        T *v = data(); //!<  get start of data
        for (int i=0; i<length() && !contained; i++)
          if (v[i]==element)
            contained=true;
        return contained;
      }

      //!  return the set of indices whose corresponding values are "element". 
      TVec<T> findIndices(const T& element)
      {
        TVec<T> indices(0);
        T *v = data();
        for (int i=0; i<length(); i++)
          if (v[i]==element)
            indices.append(i);
        return indices;
      }
 
      TVec<T> findIndices(const TVec<T>& elements)
      {
        TVec<T> indices(0);
        T *v = data();
        for (int i=0; i<length(); i++)
          for (int j=0, m=elements.length(); j<m; j++)
            if (v[i] == elements[j])
            {
              indices.append(i);
              break;
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

      TVec<T> find(TVec<T> elements)
      {
        TVec<T> indices(elements.length(),-1);
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

      //!  C++ stream output
      void print(ostream& out = cout) const; //!<  the data is printed on a single row, no newline
      void println(ostream& out = cout) const { print(out); out<<endl; } //!<  same with newline
      void printcol(ostream& out = cout) const; //!<  printed as a column
      void print(ostream& out, const string& separator) const; //!<  each value is printed with the given separator string between them

      void input(istream& in=cin) const;

      void operator<<(const string& datastring) const
      {
        istrstream in(datastring.c_str());
        input(in);
      }

};



// Youhoooo!!! Beaucoup de sueur pour pouvoir ecrire cette ligne... ;p (Pascal)
  typedef TVec<real> Vec;

// **************
// **** TMat ****
// **************

  // class TMatElementIterator: public iterator<forward_iterator_tag, T>

template<class T>
class TMatElementIterator
{
private:
  int width;
  int mod_minus_width;
  T* ptr; // current element pointer
  T* rowend; // after-last element of current row

public:

  typedef forward_iterator_tag iterator_category;
  typedef T value_type;
  typedef int size_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef T& reference;

  inline TMatElementIterator(T* begin, int width, int mod)
    :width(width), mod_minus_width(mod-width), ptr(begin), rowend(begin+width)
  {}

  inline TMatElementIterator<T>& operator++()
  { 
    ++ptr;
    if(ptr==rowend)
    {
      ptr += mod_minus_width;
      rowend = ptr+width; 
    }
    return *this;
  }

  inline TMatElementIterator<T> operator++(int)
  { 
    TMatElementIterator<T> prev(*this);
    ++ptr;
    if(ptr==rowend)
    {
      ptr += mod_minus_width;
      rowend = ptr+width; 
    }
    return prev;
  }

  inline T* operator->() const
  { return ptr; }
  
  inline T& operator*() const
  { return *ptr; }

  inline bool operator==(const TMatElementIterator& other)
  { return ptr==other.ptr; }

  inline bool operator!=(const TMatElementIterator& other)
  { return ptr!=other.ptr; }

};

template <class T>
class TMat
{
  friend class TVec<T>;
  friend class Variable; //!<  for makeShared hack... (to be cleaned)
  friend class VarArray; //!<  for makeShared hack... (to be cleaned)

protected:
  int offset_; /*!<  the displacement to do with respect to storage->data  */
  int mod_; /*!<  the real width() of the matrix  */
  int length_; /*!<  the actual length() of the matrix  */
  int width_; /*!<  the actual width() of the matrix  */
  PP< Storage<T> > storage; /*!<  where the data is really kept  */

public:

  //!  for template compatibility with other types of matrices
  int nrows() const { return length_; }
  int ncols() const { return width_; }

public:

  typedef T value_type;
  typedef int size_type;
  typedef TMatElementIterator<T> iterator; // iterator over elements
  typedef TMatElementIterator<T> const_iterator; // iterator over elements
  typedef T* compact_iterator; // super-efficient iterator over elements but works reliably only for compact matrices
  typedef T* rowelements_iterator; // iterator over elements of a particular row

  TMat<T>()
    :offset_(0), mod_(0), length_(0), width_(0)
  {}

  TMat<T>(int the_length, int the_width)
    :offset_(0), mod_(0), length_(0), width_(0)
  { resize(the_length, the_width); }

  TMat<T>(int the_length, int the_width, const T& init_value)
    :offset_(0), mod_(0), length_(0), width_(0)
  { 
    resize(the_length, the_width); 
    fill(init_value);
  }

  TMat<T>(int the_length, int the_width, T* the_data)
    :offset_(0), mod_(the_width), length_(the_length), width_(the_width), 
     storage(new Storage<T>(the_length*the_width, the_data))
  {}

  TMat<T>(int the_length, int the_width, const TVec<T>& v);

    //!  NOTE: operator= COPIES THE TMat STRUCTURE BUT NOT THE DATA (use operator<< to copy data)
    inline const TMat<T>& operator=(const TMat<T>& other)
    {
      storage = other.storage;
      offset_ = other.offset_;
      mod_ = other.mod_;
      length_ = other.length_;
      width_ = other.width_;
      return *this;
    }

  //! returns an iterator over elements
  inline iterator begin() const
  { return iterator(data(), width_, mod_); }

  inline iterator end() const
  { return iterator(data()+length_*mod_, width_, mod_); }

  //! returns a compact_iterator, which is an iterator over elements, but that works only if the matrix is compact
  inline compact_iterator compact_begin() const
  { 
#ifdef BOUNDCHECK
    if(mod()!=width()) 
      PLERROR("You cannot use a compact iterator to iterate over the elements of a non compact matrix");
#endif
    return data();
  }

  inline compact_iterator compact_end() const
  { return data()+size(); }

  //! returns an iterator over the elements of a particular row
  inline rowelements_iterator rowelements_begin(int rownum) const
  {
#ifdef BOUNDCHECK
    if(rownum<0 || rownum>=length())
      PLERROR("OUT OF RANGE rownum in rowelements_begin");
#endif
    return data()+rownum*mod();
  }

  //! IMPORTANT WARNING: use this only to check reaching the end with 
  //! an iterator obtained through rowelements_begin USING THE *SAME* rownum
  inline rowelements_iterator rowelements_end(int rownum) const
  { return data()+rownum*mod()+width(); }

/*!     Resizes the matrix to a new length() and width()
    Notice that the previous structure of the data in the matrix
    is not preserved if you increase the width() beyond mod().
    The underlying storage is never shrunk, and it is grown only if necessary.
    When grown, it is grown with extrabytes to anticipate further resizes.
*/
  void resize(int newlength, int newwidth, int extrabytes=0)
  {
#ifdef BOUNDCHECK
    if(newlength<0 || newwidth<0)
      PLERROR("IN TMat::resize(int newlength, int newwidth)\nInvalid arguments (<0)");
#endif
    if (newlength==length_ && newwidth==width_) return;
    if(storage.isNull())
    {
      offset_ = 0;
      length_ = newlength;
      width_ = newwidth;
      mod_ = newwidth;
      storage = new Storage<T>(length()*mod());
    }
    else
    {
      if(storage->usage()>1 && newwidth > mod()-offset_%mod())
        PLERROR("IN TMat::resize(int newlength, int newwidth)\nFor safety reasons, increasing the width() beyond mod() - offset_%mod() is not allowed when the storage is shared with others");

      int newsize = offset_+newlength*MAX(mod(),newwidth);
      if(offset_+newsize>storage->length())
        storage->resize(offset_+newsize+extrabytes);
      length_ = newlength;
      width_ = newwidth;
      if(newwidth>mod())
        mod_ = newwidth;
    }
  }

  inline int length() const
  { return length_; }

  inline int width() const
  { return width_; }

  inline int size() const
  { return length_*width_; }

  inline int mod() const
  { return mod_; }

  inline PP< Storage<T> > getStorage() const 
  { return storage; }

  inline bool isSquare() const
  { return length() == width(); }

  bool hasMissing() const
  {
    iterator it = begin();
    iterator itend = end();
    for(; it!=itend; ++it)
      if(is_missing(*it))
        return true;
    return false;
  }

  //!  Returns a pointer to the beginning of the matrix data
  inline T* data() const
  {
#ifdef BOUNDCHECK
    if(storage.isNull())
      PLERROR("IN TMat::data()\nAttempted to get a pointer to the data of an empty matrix");
#endif
    return storage->data+offset_; 
  }

  //!  Returns a pointer to the data beginning of the required row
  inline T* operator[](int rownum) const
  {
#ifdef BOUNDCHECK
    if(rownum<0 || rownum>=length())
      PLERROR("OUT OF BOUND ACCESS IN TMat::operator[](int rownum=%d), length=%d",rownum,length());
#endif
    return storage->data + offset_ + mod()*rownum; 
  }

  inline T* rowdata(int i) const { return (*this)[i]; }

  inline T& operator()(int rownum, int colnum) const
  {
#ifdef BOUNDCHECK
    if(rownum<0 || rownum>=length() || colnum<0 || colnum>=width())
      PLERROR("OUT OF BOUND ACCESS IN TMat::operator()(int rownum, int colnum)"
          " width=%d; length=%d; colnum=%d; rownum=%d;", width(), length(), colnum, rownum);
#endif
    return storage->data[offset_ + mod()*rownum + colnum];
  }

  inline TVec<T> operator()(int rownum) const;


  //! writes the Mat to the PStream:
  //! Note that users should rather use the form out << m;
  void write(PStream& out) const
  {
    T* ptr = 0;
    if(storage)
      ptr = data();

    switch(out.outmode)
    {
      case PStream::raw_ascii:      
      case PStream::pretty_ascii:
        for(int i=0; i<length_; i++, ptr+=mod_)
        {
          for(int j=0; j<width_; j++)
          {
            out << ptr[j];
            out.put('\t');
          }
          out.put('\n');
        }
        break;
        
      case PStream::raw_binary:
        for(int i=0; i<length_; i++, ptr+=mod_)
          binwrite_(out, ptr, width_);
        break;
        
      case PStream::plearn_ascii:
        {
          if(!out.implicit_storage)
          {
            out.write("TMat("); 
            out << length_ << width_ << mod_ << offset_ << storage;
            out.write(")\n");
          }
          else // implicit storage
          {
            out << length_;
            out.put(' ');
            out << width_;
            out.write(" [ \n");
            for(int i=0; i<length_; i++, ptr+=mod_)
            {
              for(int j=0; j<width_; j++)
              {
                out << ptr[j];
                out.put('\t');
              }
              out.put('\n');
            }
            out.write("]\n");
          }
        }
        break;

      case PStream::plearn_binary:
        {
          if(!out.implicit_storage)
          {
            out.write("TMat("); 
            out << length_ << width_ << mod_ << offset_ << storage;
            out.write(")\n");
          }
          else // implicit storage
          {
            unsigned char typecode;
            if(byte_order()==LITTLE_ENDIAN_ORDER)
            {
              out.put(0x14); // 2D little-endian 
              typecode = TypeTraits<T>::little_endian_typecode();
            }
            else
            {
              out.put(0x15); // 2D big-endian
              typecode = TypeTraits<T>::big_endian_typecode();
            }
              
            // write typecode
            out.put(typecode);
              
            // write length and width in raw_binary 
            out.write((char*)&length_, sizeof(length_));
            out.write((char*)&width_, sizeof(width_));
              
            // write the data
            for(int i=0; i<length_; i++, ptr+=mod_)
              binwrite_(out, ptr, width_);
          }
        }
        break;
      
      default:
        PLERROR("In TMat::write(PStream& out)  unknown outmode!!!!!!!!!");
        break;
      }
  }



  //! reads the Mat from the PStream:
  //! Note that users should rather use the form in >> m;
  void read(PStream& in)
  {
    switch(in.inmode)
    {
      case PStream::raw_ascii:
      case PStream::raw_binary:
        {
          T* ptr = data();
          for(int i=0; i<length_; i++, ptr+=mod_)
            for(int j=0; j<width_; j++)
              in >> ptr[j];
        }
        break;

      case PStream::plearn_ascii:
      case PStream::plearn_binary:
        {
          in.skipBlanksAndComments();
          int c = in.peek();
          if(c=='T') // explicit storage
          {
            char word[6];
            // !!!! BUG: For some reason, this hangs!!!
            // in.read(word,5);

            for(int i=0; i<5; i++)
              in.get(word[i]);

            word[5]='\0';
            if(strcmp(word,"TMat(")!=0)
              PLERROR("In operator>>(PStream&, TMat&) '%s' not a proper header for a TMat!",word);
            // v.storage = 0;
            in >> length_ >> width_ >> mod_ >> offset_;
            in >> storage;
            in.skipBlanksAndCommentsAndSeparators();
            int c = in.get(); // skip ')'
            if(c!=')')
              PLERROR("In operator>>(PStream&, TMat&) expected a closing parenthesis, found '%c'",c);
          }
          else // implicit storage
          {
            if(isdigit(c)) // ascii mode with length and width given  
            {
              int l,w;
              in >> l >> w;
              in.skipBlanksAndComments();
              c = in.get();
              if(c!='[')
                PLERROR("Error in TMat::read(PStream& in), expected '[', read '%c'",c);
              in.skipBlanksAndCommentsAndSeparators();
              resize(l,w);
              T* ptr = data();
              for(int i=0; i<length_; i++, ptr+=mod_)
                for(int j=0; j<width_; j++)
                {
                  in.skipBlanksAndCommentsAndSeparators();
                  in >> ptr[j];
                }
              in.skipBlanksAndCommentsAndSeparators();
              c = in.get();
              if(c!=']')
                PLERROR("Error in TMat::read(PStream& in), expected ']', read '%c'",c);
            }
            else if(c==0x14 || c==0x15) // it's a binary 2D sequence
            {
              in.get(); // eat c
              unsigned char typecode = in.get(); 
              int l, w;                  
              in.read((char*)&l,sizeof(l));
              in.read((char*)&w,sizeof(w));
              bool inverted_byte_order = ((c==0x14 && byte_order()==BIG_ENDIAN_ORDER) 
                  || (c==0x15 && byte_order()==LITTLE_ENDIAN_ORDER) );
              if(inverted_byte_order)
              {
                endianswap(&l);
                endianswap(&w);
              }
              resize(l,w);
              T* ptr = data();
              for(int i=0; i<length_; i++, ptr+=mod_)                    
                binread_(in, ptr, width_, typecode);
            }
            else
              PLERROR("In TMat::read(PStream& in) Char with ascii code %d not a proper first character in the header of a TMat!",c);
          }
        }
        break;
      
      default:
        PLERROR("In TMat<T>::read(PStream& in)  unknown inmode!!!!!!!!!");
        break;
      }
  }

    // The following methods are deprecated, and just call corresponding functions.
    // Please call those functions directly in new code
    //void write(ostream& out) const { PLearn::write(out, *this); }
    //void read(istream& in) { PLearn::read(in, *this); }
    void save(const string& filename) const { savePMat(filename, *this); }
    void load(const string& filename) { loadPMat(filename, *this); }
    
  void deepRead(istream& in, DeepReadMap& old2new)
  {
    readHeader(in, "TMat");
    PLearn::deepRead(in, old2new, offset_);
    PLearn::deepRead(in, old2new, mod_);
    PLearn::deepRead(in, old2new, length_);
    PLearn::deepRead(in, old2new, width_);
    PLearn::deepRead(in, old2new, storage);
    readFooter(in, "TMat");
  }

  void deepWrite(ostream& out, DeepWriteSet& already_saved) const
  {
    writeHeader(out, "TMat");
    PLearn::deepWrite(out, already_saved, offset_);
    PLearn::deepWrite(out, already_saved, mod_);
    PLearn::deepWrite(out, already_saved, length_);
    PLearn::deepWrite(out, already_saved, width_);
    PLearn::deepWrite(out, already_saved, storage);
    writeFooter(out, "TMat");
  }

  //!  Returns a TMat that is a column of the matrix
  inline TMat<T> column(int colnum) const
  { return subMatColumns(colnum, 1); }

  inline TMat<T> firstColumn() const
  { return column(0); }

  inline TMat<T> lastColumn() const
  { return column(width()-1); }

  //!  Returns a Mat that is a row of the matrix
  inline TMat<T> row(int row) const
  { return subMatRows(row, 1); }

  inline T& firstElement() const { return *data(); }
  inline T& lastElement() const { return operator()(length-1,width-1); }

  inline TVec<T> firstRow() const { return operator()(0); } 
  inline TVec<T> lastRow() const { return operator()(length_ - 1); }
  inline TVec<T> front() const { return firstRow(); }
  inline TVec<T> back() const { return lastRow(); }
  
  //!  selectColumns(*this,columns,result)
  //!  i.e. return the matrix with specified columns (indices)
  template<class I>
  inline TMat<T> columns(const TVec<I>& columns) const
  {
    TMat<T> result(length(),columns.length());
    selectColumns(*this,columns,result);
    return result;
  }

  //!  selectRows(*this,rows,result)
  //!  i.e. return the matrix with specified rows (indices)
  template<class I>
  inline TMat<T> rows(const TVec<I>& rows) const
  {
    TMat<T> result(rows.length(),width());
    selectRows(*this,rows,result);
    return result;
  }

  inline bool operator==(const TMat<T>& other) const
  {
    if(length() != other.length() || width() != other.width())
      return false;

    iterator it = begin();
    iterator end = end();
    iterator otherIt = other.begin();
    for(; it != end; ++it)
      if(*it == *otherIt)
        ++otherIt;
      else
        return false;

    return true;
  }

  template<class I>
  inline TMat<T> operator()(const TVec<I>& rows, const TVec<I>& columns) const
  {
    TMat<T> result(rows.length(),columns.length());
    select(*this,rows,columns,result);
    return result;
  }

  //!  Returns a sub-matrix that is a rectangular portion of this matrix
  inline TMat<T> subMat(int rowstart, int colstart, int newlength, int newwidth) const
  {
#ifdef BOUNDCHECK
    if(rowstart<0 || newlength<0 || rowstart+newlength>length()
        || colstart<0 || newwidth<0 || colstart+newwidth>width())
      PLERROR("Mat::subMat(int rowstart, int colstart, int newlength, int newwidth) OUT OF BOUNDS"
          "  rowstart=%d colstart=%d newlength=%d newwidth=%d length()=%d width()=%d",
          rowstart, colstart, newlength, newwidth, length(), width());
#endif
    TMat<T> subm = *this;
    subm.length_ = newlength;
    subm.width_ = newwidth;
    subm.offset_ += rowstart*mod() + colstart;
    return subm;
  }

  //!  Returns a sub-matrix that is a range of rows of this matrix
  inline TMat<T> subMatRows(int rowstart, int newlength) const
  {
#ifdef BOUNDCHECK
    if(rowstart<0 || newlength<0 || rowstart+newlength>length())
      PLERROR("TMat::subMatRows(int rowstart, int newlength) OUT OF BOUNDS");
#endif
    TMat<T> subm = *this;
    subm.length_ = newlength;
    subm.offset_ += rowstart*mod();
    return subm;
  }

  //!  Returns a sub-matrix that is a range of columns of this matrix
  inline TMat<T> subMatColumns(int colstart, int newwidth) const
  {
#ifdef BOUNDCHECK
    if(colstart<0 || newwidth<0 || colstart+newwidth>width())
      PLERROR("Mat::subMatColumns(int colstart, int newwidth) OUT OF BOUNDS");
#endif
    TMat<T> subm = *this;
    subm.width_ = newwidth;
    subm.offset_ += colstart;
    return subm;
  }

  //!  returns a newly created copy of this Matrix
  TMat<T> copy() const
  {
    TMat<T> freshcopy(length(),width());
    freshcopy << *this;
    return freshcopy;
  }

  //! copy to a C vector starting at x
  void copyTo(T* x) const
  {
    T* row = data(); // get data start
    int k=0;
    for(int i=0; i<length(); i++,row+=mod())
      for (int j=0;j<width();j++,k++)
        x[k] = row[j];
  }

/*! ************
    Deep copying
    ************
*/

  void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

/*!     Notice that deepCopy of a Mat returns a Mat rather than a Mat*.  The
    reason for this being that a Mat is already some kind of "smart
    pointer" to an underlying Storage
*/
  TMat<T> deepCopy(map<const void*, void*>& copies) const;


  //!  Copy of data
  TVec<T> toVecCopy() const;

  //!  Views same data (not always possible)
  TVec<T> toVec() const;

  bool isNull() const 
  { return storage.isNull(); }

  bool isNotNull() const
  { return storage.isNotNull(); }

  bool isEmpty() const
  { return length_==0; }

  bool isNotEmpty() const
  { return length_!=0; }

  //!  To allow if(m) statements
  inline operator bool() const
  { return isNotEmpty(); }

  //!  To allow if(!m) statements
  inline bool operator!() const
  { return isEmpty(); }

  void fill(const T& value) const
  {
    if(isCompact())
      fill_n(data(),size(),value); 
    else
    {
      int l = length();
      T* ptr = data();
      while(l--)
      {
        fill_n(ptr, width(), value);
        ptr += mod();
      }
    }
  }
  
  inline void operator=(const T& f) const
  { fill(f); }

  inline void clear() const
  { 
    if(isCompact())
      clear_n(data(),size()); 
    else
    {
      int l = length();
      T* ptr = data();
      while(l--)
      {
        clear_n(ptr, width());
        ptr += mod();
      }
    }
  }

  //!  swap the contents of row i and row j
  void swapRows(int i, int j) const
  {
    if(i!=j)
    {
      //T* Mi = rowdata(i);
      //T* Mj = rowdata(j);
      T* Mi = (*this)[i];
      T* Mj = (*this)[j];
      for (int k=0;k<width();k++)
      {
        T tmp = Mi[k];
        Mi[k] = Mj[k];
        Mj[k] = tmp;
      }
    }
  }

  int findRow(const TVec<T>& row) const;

  inline void appendRow(const TVec<T>& newrow);

  //! stl-like push_back and pop_back
  inline void push_back(const TVec<T>& newrow) { appendRow(newrow); }
  inline void pop_back() { length_ -= 1; }


  bool isCompact() const
  { return mod() == width(); }

  bool isSymmetric() const 
  {
    if (!isSquare())
      return false;

    if (length() == 0)
    {
      PLWARNING("at bool TMat::isSymmetric(), the size of the matrix is 0\n");
      return false;
    }

    for (int i = 0; i < length() - 1 ; i++)
      for (int j = i + 1; j < width(); j++)
        if ( (*this)[i][j] != (*this)[j][i] )
          return false;

    return true;
  }

  //!  Makes sure the allocated memory for this matrix is exactly length()*width()
  void compact()
  {
    if(storage->length() != length()*width())
    {
      if(storage->usage()>1)
        PLERROR("IN Mat::compact() compact operation not allowed when matrix storage is shared (for obvious reasons)");
      operator=(copy());
    }
  }

  void transpose()
  {
    if(mod()!=width())
      PLERROR("In transpose() can transpose in place only compact matrices whose mod()==width() (that is often not the case for submatrices");
    for(int i=0; i<length(); i++)
    {
      //T* rowi = rowdata(i);
      T* rowi = (*this)[i];
      T* colielem = rowi+i+mod();
      for(int j=i+1; j<width(); j++, colielem+=mod())
        swap(rowi[j], *colielem);
    }
  }

  void swapUpsideDown() const
  {
    int half = length()/2;
    for(int i=0; i<half; i++)
      swapRows(i, length()-i-1);
  }

  //!  C++ stream output
  void print(ostream& out = cout) const;
  void input(istream& in = cin) const;

  inline void operator<<(const string& datastring) const
  { 
    istrstream in(datastring.c_str());
    input(in); 
  }

};

// Youhoooo!!! Beaucoup de sueur pour pouvoir ecrire cette ligne... ;p (Pascal)
typedef TMat<real> Mat;

// *****************************
// **** Fonctions pour TVec ****
// *****************************
template <class T>
inline TVec<T> deepCopy(const TVec<T> source)
{ 
  CopiesMap copies; //!<  create empty map
  return deepCopy(source, copies);
}

template <class T>
inline TVec<T> deepCopy(const TVec<T> source, CopiesMap copies)
{ 
  return source.deepCopy(copies);
}

template<class T>
void swap( TVec<T>& a, TVec<T>& b)
{ swap_ranges(a.begin(), a.end(), b.begin()); }

//! copy TVec << TVec 
template<class T>
inline void operator<<(const TVec<T>& m1, const TVec<T>& m2)
{
#ifdef BOUNDCHECK
  if(m1.size()!=m2.size())
    PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements (%d != %d)", m1.size(), m2.size());
#endif
  copy(m2.begin(), m2.end(), m1.begin());
}

//! copy TVec << TVec  (different types)
template<class T, class U>
void operator<<(const TVec<T>& m1, const TVec<U>& m2)
{
#ifdef BOUNDCHECK
  if(m1.size()!=m2.size())
    PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
  copy_cast(m2.begin(), m2.end(), m1.begin());
}

//! copy TVec >> TVec
template<class T, class U>
inline void operator>>(const TVec<T>& m1, const TVec<U>& m2)
{ m2 << m1; }

// old .pvec format
template<class T>
void savePVec(const string& filename, const TVec<T>& vec)
{ PLERROR("savePVec only implemented for float and double"); }

template<class T>
void loadPVec(const string& filename, TVec<float>& vec)
{ PLERROR("loadPVec only implemented for float and double"); }


//!  Read and Write from C++ stream:
//!  write saves length and read resizes accordingly
//! (the raw modes don't write any size information)

template <class T> inline PStream &
operator<<(PStream &out, const TVec<T> &v)
{ 
  v.write(out); 
  return out;
}

template <class T> 
PStream & operator>>(PStream &in, TVec<T> &v)
{
  v.read(in);
  return in;
}


template<class T>      
void binwrite(ostream& out, const TVec<T>& v)
{
  int l = v.length();
  PLearn::binwrite(out,l);
  if (l<200000)
    PLearn::binwrite(out,v.data(),l);
  else for (int i=0;i<l;i+=200000)
    PLearn::binwrite(out,&v[i],std::min(200000,l-i));
}

template<class T>
void binread(istream& in, TVec<T>& v)
{
  int l;
  PLearn::binread(in,l);
  v.resize(l);
  if (l<200000)
    PLearn::binread(in,v.data(),l);
  else for (int i=0;i<l;i+=200000)
    PLearn::binread(in,&v[i],std::min(200000,l-i));
}

template<class T>      
void binwrite_double(ostream& out, const TVec<T>& v)
{
  int l = v.length();
  PLearn::binwrite(out,l);
  if (l<200000)
    PLearn::binwrite_double(out,v.data(),l);
  else for (int i=0;i<l;i+=200000)
    PLearn::binwrite_double(out,&v[i],std::min(200000,l-i));
}

template<class T>
void binread_double(istream& in, TVec<T>& v)
{
  int l;
  PLearn::binread(in,l);
  v.resize(l);
  if (l<200000)
    PLearn::binread_double(in,v.data(),l);
  else for (int i=0;i<l;i+=200000)
    PLearn::binread_double(in,&v[i],std::min(200000,l-i));
}


template<class T>
inline void deepWrite(ostream& out, DeepWriteSet& already_saved, const TVec<T>& v) 
{ v.deepWrite(out, already_saved); }

template<class T>
inline void deepRead(istream& in, DeepReadMap& old2new, TVec<T>& v)
{ v.deepRead(in, old2new); }
  
template<class T>
inline ostream& operator<<(ostream& out, const TVec<T>& v)
{ 
  v.print(out);
  return out;
}

template<class T>
inline istream& operator>>(istream& in, const TVec<T>& v)
{ 
  v.input(in);
  return in;
}


/*!   select the elements of the source as specified by the
  vector of indices (between 0 and source.length()-1) into
  the destination vector (which must have the same length()
  as the indices vector).
*/
template<class T, class I>
void selectElements(const TVec<T>& source, const TVec<I>& indices, TVec<T>& destination);

//! put in destination 1's when (*this)[i]==value, 0 otherwise
template<class T>
void elementsEqualTo(const TVec<T>& source, const T& value, const TVec<T>& destination);

template<class T>
TVec<T> concat(const TVec<T>& v1, const TVec<T>& v2);

//template<class T>
//TVec<T> concat(const Array< TVec<T> >& varray);

//! if the element to remove is the first or the last one, 
//! then a submatrix (a view) of m will be returned (for efficiency)
//! otherwise, it is a fresh copy with the element removed.
template<class T>
TVec<T> removeElement(const TVec<T>& v, int elemnum);


// *****************************
// **** Fonctions pour TMat ****
// *****************************

template <class T> inline TMat<T> deepCopy(const TMat<T> source)
{
  CopiesMap copies; //!< create empty map
  return deepCopy(source, copies);
}

template <class T> inline TMat<T>
deepCopy(const TMat<T> source, CopiesMap copies)
{ return source.deepCopy(copies); }


template<class T>
void clear(const TMat<T>& x)
{ 
  if(x.isCompact())
  {
    typename TMat<T>::compact_iterator it = x.compact_begin();
    typename TMat<T>::compact_iterator itend = x.compact_end();
    for(; it!=itend; ++it)
      clear(*it);
  }
  else
  {
    typename TMat<T>::iterator it = x.begin();
    typename TMat<T>::iterator itend = x.end();
    for(; it!=itend; ++it)
      clear(*it);
  }
}

template<class T>
void swap( TMat<T>& a, TMat<T>& b)
{ swap_ranges(a.begin(), a.end(), b.begin()); }

//! copy TMat << TMat 
template<class T>
inline void operator<<(const TMat<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
  if(m1.size()!=m2.size())
    PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
  copy(m2.begin(), m2.end(), m1.begin());
}
  
//! copy TMat << TMat  (different types)
template<class T, class U>
void operator<<(const TMat<T>& m1, const TMat<U>& m2)
{
#ifdef BOUNDCHECK
  if(m1.size()!=m2.size())
    PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
  copy_cast(m2.begin(), m2.end(), m1.begin());
}

//! copy TMat << Tvec 
template<class T>
inline void operator<<(const TMat<T>& m1, const TVec<T>& m2)
{
#ifdef BOUNDCHECK
  if(m1.size()!=m2.size())
    PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements;\t m1.size()= %d;\t m2.size= %d", m1.size(), m2.size());
#endif
  copy(m2.begin(), m2.end(), m1.begin());
}

//! copy TMat << Tvec  (different types)
template<class T, class U>
inline void operator<<(const TMat<T>& m1, const TVec<U>& m2)
{
#ifdef BOUNDCHECK
  if(m1.size()!=m2.size())
    PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
  copy_cast(m2.begin(), m2.end(), m1.begin());
}

//! copy TVec << TMat
template<class T>
inline void operator<<(const TVec<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
  if(m1.size()!=m2.size())
    PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
  copy(m2.begin(), m2.end(), m1.begin());
}

//! copy TVec << TMat  (different types)
template<class T, class U>
inline void operator<<(const TVec<T>& m1, const TMat<U>& m2)
{
#ifdef BOUNDCHECK
  if(m1.size()!=m2.size())
    PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
  copy_cast(m2.begin(), m2.end(), m1.begin());
}

//! copy TMat >> TMat
template<class T, class U>
inline void operator>>(const TMat<T>& m1, const TMat<U>& m2)
{ m2 << m1; }

//! copy TVec >> TMat
template<class T, class U>
inline void operator>>(const TVec<T>& m1, const TMat<U>& m2)
{ m2 << m1; }

//! copy TMat >> Tvec
template<class T, class U>
inline void operator>>(const TMat<T>& m1, const TVec<U>& m2)
{ m2 << m1; }



//! printing a TMat
template <class T>
inline ostream& operator<<(ostream& out, const TMat<T>& m)
{ 
  m.print(out);
  return out;
}

//! inputing a TMat

template <class T>
inline istream& operator>>(istream& in, const TMat<T>& m)
{ 
  m.input(in);
  return in;
}

//!  returns a view of this vector as a single row matrix
template <class T>
inline TMat<T> rowmatrix(const TVec<T>& v)
{ return v.toMat(1,v.length()); }

//!  returns a view of this vector as a single column matrix
template <class T>
inline TMat<T> columnmatrix(const TVec<T>& v)
{ return v.toMat(v.length(),1); }

// select the rows of the source as specified by the
// vector of indices (between 0 and source.length()-1), copied into
// the destination matrix (which must have the same length()
// as the indices vector).
template <class T, class I>
void selectRows(const TMat<T>& source, const TVec<I>& row_indices, TMat<T>& destination);

// select the colums of the source as specified by the
// vector of indices (between 0 and source.length()-1), copied into
// the destination matrix (which must have the same width()
// as the indices vector).
template <class T, class I>
void selectColumns(const TMat<T>& source, const TVec<I>& column_indices, TMat<T>& destination);

// select a submatrix of specified rows and colums of the source with
// two vectors of indices. The elements that are both in the specified rows
// and columns are copied into the destination matrix (which must have the 
// same length() as the row_indices vector, and the same width() as the length()
// of the col_indices vector).
template <class T>
void select(const TMat<T>& source, const TVec<T>& row_indices, const TVec<T>& column_indices, TMat<T>& destination);

/*
//!  Vertical concatenation (all Mats must have the same width())
template<class T>
TMat<T> vconcat(const Array< TMat<T> >& ar);

template<class T>
inline TMat<T> vconcat(const TMat<T>& m1, const TMat<T>& m2) { return vconcat(Array< TMat<T> >(m1,m2)); }

//!  Horizontal concatenation (all Mats must have the same length())
template<class T>
TMat<T> hconcat(const Array< TMat<T> >& ar);

template<class T>
inline TMat<T> hconcat(const TMat<T>& m1, const TMat<T>& m2) { return hconcat(Array< TMat<T> >(m1,m2)); }

//!  This will allow a convenient way of building arrays of Matrices by writing ex: m1&m2&m3
template<class T>
inline Array< TMat<T> > operator&(const TMat<T>& m1, const TMat<T>& m2) { return Array< TMat<T> >(m1,m2); } 
*/

//! returns a new mat which is m with the given row removed
//! if the row to remove is the first or the last one, 
//! then a submatrix (a view) of m will be returned (for efficiency)
//! otherwise, it is a fresh copy with the row removed. 
template<class T>
TMat<T> removeRow(const TMat<T>& m, int rownum);

//! returns a new mat which is m with the given column removed
//! if the column to remove is the first or the last one, 
//! then a submatrix (a view) of m will be returned (for efficiency)
//! otherwise, it is a fresh copy with the column removed. 
template<class T>
TMat<T> removeColumn(const TMat<T>& m, int colnum);


template<class T>
TMat<T> diagonalmatrix(const TVec<T>& v);

// old .pmat format
template<class T>
void savePMat(const string& filename, const TMat<T>& mat)
{ PLERROR("savePMat only implemented for float and double"); }

template<class T>
void loadPMat(const string& filename, TMat<float>& mat)
{ PLERROR("loadPMat only implemented for float and double"); }

//!  Read and Write from C++ stream:
//!  write saves length() and width(), and read resizes accordingly

//!  Read and Write from C++ stream:
//!  write saves length and read resizes accordingly
//! (the raw modes don't write any size information)

template <class T> inline PStream &
operator<<(PStream &out, const TMat<T> &m)
{ 
  m.write(out); 
  return out;
}

template <class T> 
PStream & operator>>(PStream &in, TMat<T> &m)
{
  m.read(in);
  return in;
}

/*^*************************************^*/



/*

template<class T>
void binwrite(ostream& out, const TMat<T>& m)
{
  binwrite(out,m.length());
  binwrite(out,m.width());
  for(int i=0; i<m.length(); i++)
    binwrite(out,m[i],m.width());
}

template<class T>
void binread(istream& in, TMat<T>& m)
{
  int l,w;
  binread(in,l);
  binread(in,w);
  m.resize(l,w);
  for(int i=0; i<l; i++)
    binread(in,m[i],w);
}

template<class T>
void binwrite_double(ostream& out, const TMat<T>& m)
{
  binwrite(out,m.length());
  binwrite(out,m.width());
  for(int i=0; i<m.length(); i++)
    PLearn::binwrite_double(out,m[i],m.width());
}

template<class T>
void binread_double(istream& in, TMat<T>& m)
{
  int l,w;
  binread(in,l);
  binread(in,w);
  m.resize(l,w);
  for(int i=0; i<l; i++)
    PLearn::binread_double(in,m[i],w);
}
*/

template<class T>
inline void deepWrite(ostream& out, DeepWriteSet& already_saved, const TMat<T>& m) 
{ m.deepWrite(out, already_saved); }

template<class T>
inline void deepRead(istream& in, DeepReadMap& old2new, TMat<T>& m)
{ m.deepRead(in, old2new); }


// Type traits (especially type "names" for displaying optionHelp() )

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

template<class T>
class TypeTraits< TMat<T> >
{
public:
  static inline string name()
  { return string("TMat< ") + TypeTraits<T>::name()+" >"; }

  static inline unsigned char little_endian_typecode()
  { return 0xFF; }

  static inline unsigned char big_endian_typecode()
  { return 0xFF; }
};

%> // end of namespace PLearn

#include "TMat_impl.h"

#endif

