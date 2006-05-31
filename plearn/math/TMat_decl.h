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


/*! \file PLearnLibrary/PLearnCore/TMat.h */

#ifndef TMat_decl_INC
#define TMat_decl_INC

#include "TVec_impl.h"

namespace PLearn {
using namespace std;

// predeclarations
template<class T> class TMatElementIterator;
template<class T> class TMatRowsIterator;
template<class T> class TMatColRowsIterator;
template<class T> class TMatRowsAsArraysIterator;


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
    typedef T* rowelements_iterator; // iterator over elements of a
    // particular row
    typedef TMatRowsIterator<T> rows_iterator;
    typedef TMatRowsAsArraysIterator<T> rows_as_arrays_iterator;
    typedef TMatColRowsIterator<T> colrows_iterator;

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
    inline iterator begin() const;
    inline iterator end() const;

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

    //! Return an iterator over all rows of the matrix.  No const version for
    //! now
    TMatRowsIterator<T> rows_begin();
    TMatRowsIterator<T> rows_end();

    //! Return an iterator over all rows of the matrix.  No const version for
    //! now
    TMatRowsAsArraysIterator<T> rows_as_arrays_begin();
    TMatRowsAsArraysIterator<T> rows_as_arrays_end();


    //! Return an iterator over a single column of the matrix. No const
    //! version for now.  In other words, this iterator views a single column
    //! of the matrix AS A VECTOR to iterate on; very useful for STL algorithms.
    TMatColRowsIterator<T> col_begin(int column);

    //! This version is not strictly standards-compliant since the
    //! end-pointer is beyond 1-past-the-end-of-the-array.  But this pointer
    //! is never dereferenced and should work on all reasonable architectures
    TMatColRowsIterator<T> col_end(int column);
  

    /*!     Resizes the matrix to a new length() and width()
      Notice that the previous structure of the data in the matrix
      is not preserved if you increase the width() beyond mod().
      The underlying storage is never shrunk, and it is grown only if necessary.
      When grown, it is grown with extra entries to anticipate further resizes.
      If preserve_content is true then a change of mod_ triggers a COPY
      of the old entries so that their old value remains accessed at the same indices.
    */
    void resize(int new_length, int new_width, int extra=0, bool preserve_content=false)
    {
#ifdef BOUNDCHECK
        if(new_length<0 || new_width<0)
            PLERROR("IN TMat::resize(int new_length, int new_width)\nInvalid arguments (<0)");
#endif
        if (new_length==length_ && new_width==width_) return;
        if(storage.isNull())
        {
            offset_ = 0;
            length_ = new_length;
            width_ = new_width;
            mod_ = new_width;
            storage = new Storage<T>(length()*mod());
        }
        else
        {
            int usage=storage->usage();
            if(usage>1 && new_width > mod()-offset_%mod())
                PLERROR("IN TMat::resize(int new_length, int new_width) - For safety "
                        "reasons, increasing the width() beyond mod()-offset_ modulo "
                        "mod() is not allowed when the storage is shared with others");
            if (preserve_content && size()>0)
            {
                int new_size = new_length*MAX(mod(),new_width);
                int new_offset = usage>1?offset_:0;
                if(new_size>storage->length() || new_width>mod())
                {
                    int extracols=0, extrarows=0;
                    if (extra>min(new_width,new_length))
                    {
                        // if width has increased, bet that it will increase again in the future,
                        // similarly for length,  so allocate the extra as extra mod
                        float l=length_, l1=new_length, w=width_, w1=new_width, x=extra;
                        // Solve the following equations to apportion the extra 
                        // while keeping the same percentage increase in width and length:
                        //   Solve[{x+w1*l1==w2*l2,(w2/w1 - 1)/(l2/l1 - 1) == (w1/w - 1)/(l1/l - 1)},{w2,l2}]
                        // This is a quadratic system which has two solutions: {w2a,l2a} and {w2b,l2b}:
                        float w2a = 
                            w1*(-1 - l1/(l - l1) + w1/w + (l1*w1)/(l*w - l1*w) + 
                                (2*l*(-w + w1)*x)/
                                (2*l*l1*w*w1 - l1*l1*w*w1 - l*l1*w1*w1 + 
                                 sqrt(square(l1*l1*w*w1 - l*l1*w1*w1) + 
                                      4*l*(l - l1)*l1*w*(w - w1)*w1*(l1*w1 + x))));
                        float l2a = -(-l1*l1*w*w1 + l*l1*w1*w1 + 
                                      sqrt(square(l1*l1*w*w1 - l*l1*w1*w1) + 
                                           4*l*(l - l1)*l1*w*(w - w1)*w1*(l1*w1 + x)))/(2.*l*(w - w1)*w1);
                        float w2b =w1*(-1 - l1/(l - l1) + w1/w + (l1*w1)/(l*w - l1*w) - 
                                       (2*l*(-w + w1)*x)/
                                       (-2*l*l1*w*w1 + l1*l1*w*w1 + l*l1*w1*w1 + 
                                        sqrt(square(l1*l1*w*w1 - l*l1*w1*w1) + 
                                             4*l*(l - l1)*l1*w*(w - w1)*w1*(l1*w1 + x))));
                        float l2b = (l1*l1*w*w1 - l*l1*w1*w1 + 
                                     sqrt(square(l1*l1*w*w1 - l*l1*w1*w1) + 
                                          4*l*(l - l1)*l1*w*(w - w1)*w1*(l1*w1 + x)))/(2.*l*(w - w1)*w1);

                        // pick one that is feasible and maximizes the mod
                        if (w2b>w2a && w2b>w1 && l2b>l1)
                        {
                            extracols=int(ceil(w2b-w1));
                            extrarows=int(ceil(l2b-l1));
                        } else if (w2a>w1 && l2a>l1)
                        {
                            extrarows=int(ceil(l2a-l1));
                            extracols=int(ceil(w2a-w1));
                        } else // no valid solution to the system of equation, use a heuristic
                        {
                            extracols = int(ceil(sqrt(real(extra))/new_length));
                            extrarows = int((extra+l1*w1)/(w1+extracols) - l1);
                        }
                    }
                    storage->resizeMat(new_length,new_width,extrarows,extracols,
                                       new_offset,mod_,length_,width_,offset_);
                    mod_ = new_width + extracols;
                }
                offset_ = new_offset;
            }
            else // old code, verbatim
            {
                int new_size = offset_+new_length*MAX(mod(),new_width);
                if(offset_+new_size>storage->length())
                    storage->resize(new_size + extra);
                if(new_width>mod())
                    mod_ = new_width;
            }
            length_ = new_length;
            width_ = new_width;
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

    //! Set a new value for 'mod'. The content of the matrix will be destroyed
    //! (i.e. moved around).
    inline void setMod(int new_mod)
    {
        if (new_mod == mod_)
            // Nothing to do (the new mod is equal to the old one).
            return;
        if (storage.isNull()) {
            mod_ = new_mod;
            return;
        }
        if (storage->usage() > 1)
            PLERROR("In setMod - You cannot change the 'mod' of a matrix "
                    "whose storage is shared");
        if (new_mod > mod())
            resize(length(), new_mod);
        else
            mod_ = new_mod;
    }

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

    inline TVec<T> operator()(int rownum) const
    {
#ifdef BOUNDCHECK
        if(rownum<0 || rownum>=length())
            PLERROR("OUT OF BOUND ACCESS IN TMat_impl::operator()(int rownum)");
#endif
        TVec<T> tv;
        tv.length_ = width();
        tv.offset_ = offset_ + mod()*rownum;
        tv.storage = storage;
        return tv;
    }

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
                    T* ptr = (l>0 && w>0)? data():0;
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
    // void save(const string& filename) const { savePMat(filename, *this); }
    // void load(const string& filename) { loadPMat(filename, *this); }
    
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

    inline bool operator==(const TMat<T>& other) const;
    inline bool isEqual(const TMat<T>& other, real precision=1e-6) const;
    
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

    void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    /*!     Notice that deepCopy of a Mat returns a Mat rather than a Mat*.  The
      reason for this being that a Mat is already some kind of "smart
      pointer" to an underlying Storage
    */
    TMat<T> deepCopy(CopiesMap& copies) const;


    //!  Copy of data
    TVec<T> toVecCopy() const;

    //!  Views same data (not always possible)
    TVec<T> toVec() const;

    bool isNull() const 
    { return storage.isNull(); }

    bool isNotNull() const
    { return storage.isNotNull(); }

    bool isEmpty() const
    { return length_ == 0 || width_ == 0; }

    bool isNotEmpty() const
    { return length_ != 0 && width_ != 0; }

    /*! This method is commented out because it is much too dangerous:
      it allows a TMat to be converted into an int, which can cause some
      very weird bugs that the compiler would have caught otherwise.*/
    /*
    //!  To allow if(m) statements.
    inline operator bool() const
    { return isNotEmpty(); }
    */

    //!  To allow if(!m) statements
    inline bool operator!() const
    { return isEmpty(); }

    void fill(const T& value) const
    {
        if (isNotEmpty()) {
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
    }
  
    inline void operator=(const T& f) const
    { fill(f); }

    inline void clear() const
    { 
        if(isNotEmpty())
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
    }

    //! Swap the content of row i and row j.
    //! Note: a potentially more efficient version can be found in
    //! TMat_maths_impl.h.
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

    //! Swap the content of column i and column j.
    void swapColumns(int i, int j) const
    {
        if (i != j)
        {
            T* Mi = data() + i;
            T* Mj = data() + j;
            int n = length();
            for (int k = 0; k < n; k++) {
                T tmp = *Mi;
                *Mi = *Mj;
                *Mj = tmp;
                Mi += mod();
                Mj += mod();
            }
        }
    }

    int findRow(const TVec<T>& row) const;

    inline void appendRow(const TVec<T>& newrow);

    //! stl-like push_back and pop_back
    inline void push_back(const TVec<T>& newrow) { appendRow(newrow); }
    inline void pop_back() { length_ -= 1; }

    /*!         make the storage point to this address and
      copy current value to it (i.e. without changing
      current contents)
    */

    void makeSharedValue(T* x, int n)
    {
        int m = size();
#ifdef BOUNDCHECK
        if(n != m)
            PLERROR("IN TMat::makeSharedValue(T* x, int n)\nn(%d)!=size(%d)",
                    n,m);
#endif
        T* v = data(); //!<  get data start
        for(int i=0,k=0; i<length_; i++, v+=mod_)
            for (int j=0;j<width_; j++, k++)
                x[k] = v[j];
        storage->pointTo(n,x);
        offset_ = 0;
    }

    bool isCompact() const
    { return mod() == width(); }

    //! Return 'true' iff the matrix is symmetric.
    //! If 'exact_check' is true, it performs a fast exact equality check (which
    //! does not handle 'nan' or 'inf' for instance), otherwise it uses the
    //! approximate and slower 'is_equal' function from pl_math.h.
    //! If 'accept_empty' is set to 'true', then empty matrices will be considered
    //! as symmetric, otherwise a warning will be issued and 'false' will be
    //! returned.
    bool isSymmetric(bool exact_check = true, bool accept_empty = false) const 
    {
        if (!isSquare())
            return false;

        if (isEmpty())
        {
            if (accept_empty)
                return true;
            else {
                PLWARNING("In TMat::isSymmetric - The matrix is empty, considering "
                          "it is not symmetric (use 'accept_empty' if you want to "
                          " allow it)");
                return false;
            }
        }

        int n = length();
        assert( width() == n );

        if (exact_check) {
            for (int i = 0; i < n - 1 ; i++)
                for (int j = i + 1; j < n; j++)
                    if ( !fast_exact_is_equal((*this)[i][j], (*this)[j][i]) )
                        return false;
        } else {
            for (int i = 0; i < n ; i++)
                for (int j = i + 1; j < n; j++)
                    if ( !is_equal((*this)[i][j], (*this)[j][i] ) )
                        return false;
        }

        return true;
    }

    //! Ensure the allocated memory for this matrix is exactly length * width.
    void compact()
    {
        if(storage->length() != length()*width())
        {
            if(storage->usage()>1)
                PLERROR("In TMat<T>::compact() - Compact operation not allowed"
                        " when matrix storage is shared, for obvious reasons");
            operator=(copy());
        }
    }

    //! Swap element (i,j) with element (j,i).
    //! Currently only implemented for square matrices.
    void transpose()
    {
        if (length() != width())
            PLERROR("In TMat<T>::tranpose() - Only implemented for square "
                    "matrices");
        for (int i = 0; i < length(); i++)
        {
            T* rowi = (*this)[i] + i + 1;
            T* colielem = rowi - 1 + mod();
            for(int j = i + 1; j < width(); j++, colielem += mod(), rowi++)
                pl_swap(*rowi, *colielem);
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
    void input(PStream& in) const;

    // calls print with cerr, usefull with gdb (> call obj.debugprint() )
    void debugPrint(){print(cerr);}


    inline void operator<<(const string& datastring) const
    { 
        // istrstream in(datastring.c_str());
        PStream in = openString(datastring,PStream::plearn_ascii);
        input(in); 
    }

};

typedef TMat<real> Mat;


// Type traits (especially type "names" for displaying optionHelp() )

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

template <class T>
inline int sizeInBytes(const TMat<T>& x) { 
    int n=x.size();
    int s=sizeof(TMat<T>);
    if (n>0) s+=n*sizeInBytes(x(0,0)); 
    return s;
}

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
