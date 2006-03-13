
#ifndef CopiesMap_INC
#define CopiesMap_INC

#include <map>
#include <string>
#include <utility>                           //!< for pair
#include "plerror.h"                         //!< For PLWARNING.

//! Macro to define deep copy for types that actually do not require
//! any deep copy (such as int, real, etc.).
//! Since the copy constructor of an array does copy the content of
//! its storage, deep copying an array of such types is not necessary
//! either.
#define NODEEPCOPY(TYPE)                                      \
      inline void deepCopyField(TYPE&, CopiesMap&) {}         \
      inline void deepCopyField(Array<TYPE>&, CopiesMap&) {}

namespace PLearn {
using std::string;
using std::map;
using std::pair;

class PPath;
class VMField;
class VMFieldStat;
template <class T> class Array;

//!  Global typedef to make the map of copied objects (needed by the deep
//!  copy mechanism in Object) more palatable
typedef map<const void*,void*> CopiesMap;

//! Some typedefs to use the NODEEPCOPY macro with.
typedef map<string, float> map_string_float;
typedef map<string, double> map_string_double;
typedef map<double, string> map_double_string;
typedef map<float, string> map_float_string;
typedef map<string, string> map_string_string;
typedef map<float, float> map_float_float;
typedef map<double, double> map_double_double;
typedef map<string, int> map_string_int;
typedef map<int, string> map_int_string;

/*! Support for generic deep copying
    
Deep copying is defined for objects in the following manner:
+ copy constructors should always do a shallow copy.
+ a public method OBJTYPE* deepCopy(CopiesMap& copies) const 
should be defined to allow deepCopying
+ the deepCopy method should be virtual for classes that are designed to be subclassed
Take a close look at the Object class in Object.h to see how this is done.
*/

//! Types that do not require deep copy.
NODEEPCOPY(double)
NODEEPCOPY(float)
NODEEPCOPY(int)
NODEEPCOPY(unsigned int)
NODEEPCOPY(clock_t)
NODEEPCOPY(bool)
NODEEPCOPY(map_string_float)
NODEEPCOPY(map_string_double)
NODEEPCOPY(map_float_string)
NODEEPCOPY(map_double_string)
NODEEPCOPY(map_string_string)
NODEEPCOPY(map_float_float)
NODEEPCOPY(map_double_double)
NODEEPCOPY(map_string_int)
NODEEPCOPY(map_int_string)
NODEEPCOPY(string)
NODEEPCOPY(PPath)
NODEEPCOPY(VMField)
NODEEPCOPY(VMFieldStat)
NODEEPCOPY(FILE*)   //!< There is currently no proper way to deep copy these.

//! Pairs handle deepCopying by distributing it to each element
    template <class T, class U>
inline void deepCopyField(pair<T,U>& p, CopiesMap& copies)
{
    deepCopyField(p.first, copies);
    deepCopyField(p.second, copies);
}
  
//!  Any type not handled below: do nothing
template <class T>
inline void deepCopyField(T&, CopiesMap&)
{
    /*! no op */
    PLWARNING(
        "In CopiesMap.h - deepCopyField not handled for this type. "
        "If it actually doesn't need deep copy, edit CopiesMap.h and add NODEEPCOPY(your_type) to remove this warning."
        );
}

template <class T>
inline void deepCopyField(T*& field, CopiesMap& copies)
{
    if (field)
        field = field->deepCopy(copies);
}

//!  A simple template function that calls the method
template<class T>
T* deepCopy(const T* source, CopiesMap& copies)
{ return source->deepCopy(copies); }

//!  This function simply calls the previous one with an initially empty map
template<class T>
inline T* deepCopy(const T* source)
{ 
    CopiesMap copies; //!<  create empty map
    return deepCopy(source, copies);
}


}

#endif //ndef CopiesMap_INC


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
