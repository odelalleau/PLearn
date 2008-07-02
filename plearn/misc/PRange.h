// -*- C++ -*-

// PRange.h
//
// Copyright (C) 2004-2005 ApSTAT Technologies Inc.
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

// Authors: Jean-Sébastien Senécal

/*! \file PRange.h */

#ifndef PRange_H
#define PRange_H

namespace PLearn {

/*!
  This template class represents a range of values. A range is simply
  a pair of boundaries lower and upper s.t. lower <= upper. The range
  represented by PRange is always inclusive, i.e. lower and upper are
  part of the range. A PRange may contain just one value (i.e.
  lower == upper) and it may be empty (in this case, lower will be set
  to zero and upper will be set to -1).
*/
template<class T>
class PRange
{
private:
    // Lower bound.
    T lower_;

    // Upper bound.
    T upper_;

public:
    //! Creates an empty range [-1,0].
    PRange() : lower_(0), upper_(-1) { }

    //! Creates a range with one element : [val,val]
    PRange(const T& val) : lower_(val), upper_(val) { }

    //! Creates the range [lower, upper].
    PRange(const T& lower, const T& upper)
    { setBounds(lower,upper); }

    //! Returns true iff "val" is inside range.
    bool contains(const T& val) const
    { return (lower_ <= val && val <= upper_); }

    //! Returns true iff the range is empty.
    bool isEmpty() const { return (lower_ > upper_); }

    //! Returns the lower bound.
    T lower() const { return lower_; }

    //! Returns the upper bound.
    T upper() const { return upper_; }


    //! Sets to the empty range.
    void clear() { lower_ = 0; upper_ = -1; }

    //! Sets to the one-element range [val,val].
    void setValue(const T& val) { lower_ = upper_ = val; }

    //! Changes the bounds.
    void setBounds(const T& lower, const T& upper)
    {
        if (lower > upper)
        { lower_ = 0; upper_ = -1; }
        else
        { lower_ = lower; upper_ = upper; }
    }

    //! Moves the bounds jointly.
    void translate(const T& offset)
    {
        lower_ += offset;
        upper_ += offset;
    }

    //! Move the upper bound only
    void translateUpper(const T& offset) { upper_ += offset; }

    //! Move the lower bound only
    void translateLower(const T& offset) { lower_ += offset; }

    //! Returns the distance (always positive).
    T distance() const { return (max(0,upper_ - lower_ + 1)); }

    //! Returns true iff both ranges are empty or have the same bounds.
    bool operator==(const PRange<T>& r)
    { return (r.lower() == lower_ && r.upper() == upper_); }

    //! Returns true iff the ranges are not equal.
    bool operator!=(const PRange<T>& r) const
    { return (r.lower() != lower_ || r.upper() != upper_); }

    //! Union-assignment operator.
    PRange& operator|=(const PRange<T>& r)
    {
        if (isEmpty())
            return operator=(r);
        else if (!r.isEmpty())
        {
            lower_ = min(lower_, r.lower());
            upper_ = max(upper_, r.upper());
        }
        return (*this);
    }

    //! Intersection-assignment operator.
    PRange& operator&=(const PRange<T>& r)
    {
        if (! isEmpty()){
            if (! r.isEmpty()) {
                lower_ = max(lower_, r.lower());
                upper_ = min(upper_, r.upper());
            }
            else
                clear();
        }
            
        return (*this);
    }
  
};

//! Union operator.
template<class T>
PRange<T> operator|(const PRange<T>& r1, const PRange<T>& r2)
{
    if (r1.isEmpty())
        return r2;
    else if (r2.isEmpty())
        return r1;
    else
        return PRange<T>( min(r1.lower(), r2.lower()), max(r1.upper(), r2.upper()) );
}

//! Intersection operator.
template<class T>
PRange<T> operator&(const PRange<T>& r1, const PRange<T>& r2)
{
    if ( r1.isEmpty() || r2.isEmpty() )
        return PRange<T>();                      // empty range
    else
        return PRange<T>( max(r1.lower(), r2.lower()), min(r1.upper(), r2.upper()) );
}
  
}

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
