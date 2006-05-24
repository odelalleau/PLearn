// -*- C++ -*-
 
// tuple_io.h
// Copyright (C) 2006 Pascal Vincent
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

#ifndef pl_tuple_INC
#define pl_tuple_INC

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <plearn/base/TypeTraits.h>
#include <plearn/io/PStream.h>

namespace PLearn {

// *************************************
// inject boost tuples facility in PLearn namespace

using boost::tuples::tuple;
using boost::tuples::make_tuple;
using boost::tuples::tie;
using boost::tuples::get;

// ***********************************************************
// define correpsonding TypeTraits for tuples up to 6 elements

template<class T1>
class TypeTraits< tuple<T1> >
{
public:
    static inline string name()
    { return string("tuple< ") + TypeTraits<T1>::name()+" >"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T1, class T2>
class TypeTraits< tuple<T1,T2> >
{
public:
    static inline string name()
    { 
        return string("tuple< ") 
            + TypeTraits<T1>::name() + ", "
            + TypeTraits<T2>::name() + " >"; 
    }
    
    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T1, class T2, class T3>
class TypeTraits< tuple<T1,T2,T3> >
{
public:
    static inline string name()
    { 
        return string("tuple< ") 
            + TypeTraits<T1>::name() + ", "
            + TypeTraits<T2>::name() + ", "
            + TypeTraits<T3>::name() + " >"; 
    }
    
    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T1, class T2, class T3, class T4>
class TypeTraits< tuple<T1,T2,T3,T4> >
{
public:
    static inline string name()
    { 
        return string("tuple< ") 
            + TypeTraits<T1>::name() + ", "
            + TypeTraits<T2>::name() + ", "
            + TypeTraits<T3>::name() + ", "
            + TypeTraits<T4>::name() + " >"; 
    }
    
    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T1, class T2, class T3, class T4, class T5>
class TypeTraits< tuple<T1,T2,T3,T4,T5> >
{
public:
    static inline string name()
    { 
        return string("tuple< ") 
            + TypeTraits<T1>::name() + ", "
            + TypeTraits<T2>::name() + ", "
            + TypeTraits<T3>::name() + ", "
            + TypeTraits<T4>::name() + ", "
            + TypeTraits<T5>::name() + " >"; 
    }
    
    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T1, class T2, class T3, class T4, class T5, class T6>
class TypeTraits< tuple<T1,T2,T3,T4,T5,T6> >
{
public:
    static inline string name()
    { 
        return string("tuple< ") 
            + TypeTraits<T1>::name() + ", "
            + TypeTraits<T2>::name() + ", "
            + TypeTraits<T3>::name() + ", "
            + TypeTraits<T4>::name() + ", "
            + TypeTraits<T5>::name() + ", "
            + TypeTraits<T6>::name() + " >"; 
    }
    
    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};


// **************************************************************
// define correpsonding serialization for tuples up to 6 elements


template<class T1>
PStream& operator<<(PStream& out, const tuple<T1>& t)
{
    PStream::mode_t oldmode = out.switchToPLearnOutMode();
    out.write("(");
    out << get<0>(t);
    out.write(")");
    out.setOutMode(oldmode);
    return out;
}

template<class T1, class T2>
PStream& operator<<(PStream& out, const tuple<T1,T2>& t)
{
    PStream::mode_t oldmode = out.switchToPLearnOutMode();
    out.write("(");
    out << get<0>(t); out.write(", ");
    out << get<1>(t);
    out.write(")");
    out.setOutMode(oldmode);
    return out;
}

template<class T1, class T2, class T3>
PStream& operator<<(PStream& out, const tuple<T1,T2,T3>& t)
{
    PStream::mode_t oldmode = out.switchToPLearnOutMode();
    out.write("(");
    out << get<0>(t); out.write(", ");
    out << get<1>(t); out.write(", ");
    out << get<2>(t);
    out.write(")");
    out.setOutMode(oldmode);
    return out;
}

template<class T1, class T2, class T3, class T4>
PStream& operator<<(PStream& out, const tuple<T1,T2,T3,T4>& t)
{
    PStream::mode_t oldmode = out.switchToPLearnOutMode();
    out.write("(");
    out << get<0>(t); out.write(", ");
    out << get<1>(t); out.write(", ");
    out << get<2>(t); out.write(", ");
    out << get<3>(t);
    out.write(")");
    out.setOutMode(oldmode);
    return out;
}

template<class T1, class T2, class T3, class T4, class T5>
PStream& operator<<(PStream& out, const tuple<T1,T2,T3,T4,T5>& t)
{
    PStream::mode_t oldmode = out.switchToPLearnOutMode();
    out.write("(");
    out << get<0>(t); out.write(", ");
    out << get<1>(t); out.write(", ");
    out << get<2>(t); out.write(", ");
    out << get<3>(t); out.write(", ");
    out << get<4>(t);
    out.write(")");
    out.setOutMode(oldmode);
    return out;
}

template<class T1, class T2, class T3, class T4, class T5, class T6>
PStream& operator<<(PStream& out, const tuple<T1,T2,T3,T4,T5,T6>& t)
{
    PStream::mode_t oldmode = out.switchToPLearnOutMode();
    out.write("(");
    out << get<0>(t); out.write(", ");
    out << get<1>(t); out.write(", ");
    out << get<2>(t); out.write(", ");
    out << get<3>(t); out.write(", ");
    out << get<4>(t); out.write(", ");
    out << get<5>(t);
    out.write(")");
    out.setOutMode(oldmode);
    return out;
}


// **************************************************************
// define correpsonding de-serialization for tuples up to 6 elements

template<class T1>
PStream& operator>>(PStream& in, tuple<T1>& t)
{    
    in.skipBlanksAndComments(); in.readExpected('('); 
    in.skipBlanksAndComments(); in >> get<0>(t); 
    in.skipBlanksAndComments(); in.readExpected(')');
    return in;
}

template<class T1, class T2>
PStream& operator>>(PStream& in, tuple<T1,T2>& t)
{
    in.skipBlanksAndComments(); in.readExpected('('); 
    in.skipBlanksAndComments(); in >> get<0>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<1>(t); 
    in.skipBlanksAndComments(); in.readExpected(')');
    return in;
}

template<class T1, class T2, class T3>
PStream& operator>>(PStream& in, tuple<T1,T2,T3>& t)
{
    in.skipBlanksAndComments(); in.readExpected('('); 
    in.skipBlanksAndComments(); in >> get<0>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<1>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<2>(t); 
    in.skipBlanksAndComments(); in.readExpected(')');
    return in;
}

template<class T1, class T2, class T3, class T4>
PStream& operator>>(PStream& in, tuple<T1,T2,T3,T4>& t)
{
    in.skipBlanksAndComments(); in.readExpected('('); 
    in.skipBlanksAndComments(); in >> get<0>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<1>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<2>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<3>(t); 
    in.skipBlanksAndComments(); in.readExpected(')');
    return in;
}

template<class T1, class T2, class T3, class T4, class T5>
PStream& operator>>(PStream& in, tuple<T1,T2,T3,T4,T5>& t)
{
    in.skipBlanksAndComments(); in.readExpected('('); 
    in.skipBlanksAndComments(); in >> get<0>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<1>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<2>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<3>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<4>(t); 
    in.skipBlanksAndComments(); in.readExpected(')');
    return in;
}

template<class T1, class T2, class T3, class T4, class T5, class T6>
PStream& operator>>(PStream& in, tuple<T1,T2,T3,T4,T5,T6>& t)
{
    in.skipBlanksAndComments(); in.readExpected('('); 
    in.skipBlanksAndComments(); in >> get<0>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<1>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<2>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<3>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<4>(t); 
    in.skipBlanksAndCommentsAndSeparators(); in >> get<5>(t); 
    in.skipBlanksAndComments(); in.readExpected(')');
    return in;
}

} // namespace PLearn

#endif //ndef pl_tuple_INC


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
