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
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef RealMapping_INC
#define RealMapping_INC

//#include "general.h"
#include "Object.h"
#include <plearn/math/TMat.h>
#include <map>

namespace PLearn {
using namespace std;

//! represents a real range: i.e. one of ]low,high[ ; [low,high[; [low,high]; ]low,high]
class RealRange
{
public:
    real low;
    real high;
    char leftbracket; // either '[' (inclusive left) or ']' (exclusive left)
    char rightbracket; // // either '[' (exclusive right) or ']' (inclusive right)

public:
    RealRange(): // default construvtor
        low(0), high(0), leftbracket(']'), rightbracket('[')
    {}
     

    RealRange(char leftbracket_, real low_, real high_, char rightbracket_):
        low(low_), high(high_), leftbracket(leftbracket_), rightbracket(rightbracket_)
    { checkbrackets(); } 

    real span(){return abs(high-low);}
    
    void checkbrackets() const
    {
        if( (leftbracket!='[' && leftbracket!=']') || (rightbracket!='[' && rightbracket!=']') )
            PLERROR("In RealRange: Brackets must be either '[' or ']'"); 
    }

    void print(ostream& out) const
    { out << leftbracket << low << ' ' << high << rightbracket; }

    void write(ostream& out) const
    { out << leftbracket << low << ' ' << high << rightbracket; }

    void read(PStream& in)
    { in >> leftbracket >> low >> high >> rightbracket; checkbrackets(); }


    string getString() const;
    
    //! Compare RealRange and real:
    //! the relation is either:
    //!   Range `contains` real
    //!   Range < real, if higher bound < real
    //!   Range > real, if lower bound > real
    bool contains(real val) const;
    bool operator<(real x) const;
    bool operator>(real x) const;

/*    inline bool operator<(real x) const
      { return low < x || high == x && rightbracket == '['; }

      inline bool operator>(real x) const
      { return low > x || low == x && leftbracket == ']'; }

*/
    /*! Compare 2 RealRanges:
      This can be used to sort a list of RealRanges, or to build a map indexed by RealRanges.
      Note that ordering of ranges is only properly defined for ranges that do not overlap.
    */
    bool operator<(const RealRange& x) const;
    bool operator>(const RealRange& x) const;
    bool operator==(const RealRange& rr) const;
};

inline void write(ostream& out, const RealRange& range) { range.write(out); }
inline ostream& operator<<(ostream& out, const RealRange& range) { range.print(out); return out; } 
inline void read(PStream& in, RealRange& range) { range.read(in); }

PStream& operator<<(PStream& out, const RealRange& x);
PStream& operator>>(PStream& in, RealRange &x);


/**
 *  Mapping between ranges and values.
 *
 *  RealMapping is used as a component of binning operations.  It divides the
 *  real line in a set of ranges, and associates each range with a single
 *  (usually integer) value.  The ranges are specified as follows:
 *
 *  - ]low,high[ : both endpoints are EXCLUDED
 *  - [low,high[ : lower endpoint INCLUDED, upper endpoint EXCLUDED
 *  - [low,high] : both endpoints are INCLUDED
 *  - ]low,high] : lower endpoint EXCLUDED, upper endpoint INCLUDED
 */
class RealMapping: public Object
{
    typedef Object inherited;

public:
    typedef pair<RealRange, real> single_mapping_t;
    typedef TVec< single_mapping_t > ordered_mapping_t;
    typedef std::map<RealRange, real> mapping_t;
    typedef mapping_t::iterator iterator;
    typedef mapping_t::const_iterator const_iterator;

    //! Defines mapping from real ranges to values
    mapping_t mapping;

    //! Value to which to map missing values (can be missing value)
    real missing_mapsto;

    //! If true, values not in mapping are left as is, otherwise they're
    //! mapped to other_mapsto
    bool keep_other_as_is;

    //! Value to which to map values not inmapping, if keep_other_as_is is
    //! false 
    real other_mapsto;

    /**
     *  o_mapping contains the same mappings as 'mapping', but they are ordered
     *  so that the lower limits of ranges are in ascending order NOTE : before
     *  any access, it must be created with a call to buildOrderedMapping()
     */
    ordered_mapping_t o_mapping; 
    
public:
    PLEARN_DECLARE_OBJECT(RealMapping);
    
    RealMapping()
        :missing_mapsto(MISSING_VALUE),
         keep_other_as_is(true),
         other_mapsto(MISSING_VALUE)
    {}

    int size() const { return (int)mapping.size(); }
    int length() const { return (int)mapping.size(); }

    //! Removes all entries in mapping.  Does not change other params.
    inline void clear() { mapping.clear(); }
    
    void buildOrderedMapping();

    bool checkConsistency();

    void removeMapping(const RealRange& range)
    { 
        mapping_t::iterator it= mapping.find(range);
        if(it != mapping.end())
            mapping.erase(it);
        else
            PLWARNING("In RealMapping::removeMapping  mapping not removed: does not exist.");
    }

    void removeMapping(real x) //remove range where x falls
    {
        mapping_t::iterator it= mapping.lower_bound(RealRange('[',x,x,']'));
        if(it != mapping.end() && it->first.contains(x))
            mapping.erase(it);
        else
            PLWARNING("In RealMapping::removeMapping  mapping not removed: does not exist.");
    }

    void addMapping(const RealRange& range, real val);

    //! Set mapping for missing value (by default it maps to MISSING_VALUE)
    void setMappingForMissing(real what_missing_mapsto)
    { missing_mapsto = what_missing_mapsto; }

    //! Set mapping for any other value (by default it is kept as is)
    void setMappingForOther(real what_other_mapsto)
    { keep_other_as_is=false; other_mapsto = what_other_mapsto; }

    void keepOtherAsIs() 
    { keep_other_as_is=true; }

    // returns the mapped value corresponding to val
    real map(real val) const;

    // returns the number of the bin in which 'val' falls
    int binnumber(real val) const;

    // transforms v by applying the mapping on all its elements
    void transform(const Vec& v) const;

    pair<RealRange, real> lastMapping() 
    { return *(mapping.rbegin()); }
    //    { return mapping.lastElement(); }

    /***
     * map methods "forwarded"
     */

    iterator begin()
    { return mapping.begin(); }
    const_iterator begin() const
    { return mapping.begin(); }
    iterator end()
    { return mapping.end(); }
    const_iterator end() const
    { return mapping.end(); }
    void erase(iterator it) 
    { mapping.erase(it); }

    bool operator==(const RealMapping& rm) const;

    //! Overridden to use the specific real mapping format in raw_ascii mode.
    virtual void newwrite(PStream& out) const;

    virtual void print(ostream& out) const;
    virtual void write(ostream& out) const;
    virtual void read(PStream& in);

    real maxMappedToValue();

    //! If all ranges in the mapping are consecutive, return the cut points between different ranges.
    //! e.g.: [0,1[  [1, 5[  [5, 10]  ]10, 15]--> <0,1,5,10,15>.
    Vec getCutPoints() const;

protected: 
    static void declareOptions(OptionList& ol);
};

DECLARE_OBJECT_PTR(RealMapping);

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
