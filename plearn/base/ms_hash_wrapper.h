// -*- C++ -*-

// ms_hash_wrapper Works as a wrapper between MS (VS .NET 2003) hashing functions
//                 and GCC plearn (gcc inspired) hashing specializations.
// Copyright (C) 2004 Norman Casagrande
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

// Wrappers for the has functions. See below for instructions.
// IMPORTANT: These macros MUST be outside of any namespace!!!!!!!!!


#ifndef ms_hash_wrapper_H
#define ms_hash_wrapper_H

#if defined(__GNUC__)     // Look below after the macros for the end of this

#if __GNUC__ < 3 
#error GNUC < 3 is not supported!!
#endif 

#  include <ext/hash_set> //to get stl_hash_fun.h ... (template<> class hash)
#  include <ext/hash_map> //to get stl_hash_fun.h ... (template<> class hash)

#	if (__GNUC__ == 3 && __GNUC_MINOR__ == 0) || (defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1000)
//              GCC 3.0 or ICC < 10.0
#		define __NMSPACE__ std
#  else                                            // GCC 3.1 or later
using namespace __gnu_cxx;
#		define __NMSPACE__ __gnu_cxx
#  endif

//using namespace __NMSPACE__;

// Three original cases for hash specialization and declaration:

// In file pl_hash_fun:
// template<>
// struct hash<float>
// {
//   size_t operator()(float x) const { return PLearn::hashval(x); }
// };
// NOW IS: SET_HASH_WITH_FUNCTION

// -------------------------------------------------------------------

// In file pl_hash_fun:
// template<>
// struct hash<string>
// {
//   size_t operator()(const string& __s) const { return hash<const char*>()(__s.c_str()); }
// };
// NOW IS: SET_HASH_WITH_INHERITANCE

// -------------------------------------------------------------------

// In file TMat_maths_impl.h:
// template<class T>
// struct hash<PLearn::TVec<T> >
// {
//   size_t operator()(PLearn::TVec<T> v) const { return hash<T>()(sumsquare(v));} 
// };
// NOW IS: SET_HASH_FUNCTION

// -------------------------------------------------------------------
// -------------------------------------------------------------------
// -------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
//////// GCC Version /////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// WARNING: paramName should be included in hashFunc!	
// Example: 
//     SET_HASH_WITH_FUNCTION(float, val, PLearn::hashval(val))
// --> Will return (see the function operator() below): 
//     size_t operator()(const float& val) const
//     { 
//        return (size_t)(PLearn::hashval(val));
//     } 
#define SET_HASH_WITH_FUNCTION(type, paramName, hashFunc)           \
  namespace __NMSPACE__ {                                           \
	  template<>                                                      \
	  struct hash< type >                                             \
	  {                                                               \
		  size_t operator()(const type& paramName) const                \
		  {                                                             \
			  return (size_t)(hashFunc);                                  \
		  }                                                             \
	  };                                                              \
  }

#define SET_HASH_WITH_FUNCTION_NOCONSTREF(type, paramName, hashFunc) \
  namespace __NMSPACE__ {                                           \
	  template<>                                                      \
	  struct hash< type >                                             \
	  {                                                               \
		  size_t operator()(type paramName) const                       \
		  {                                                             \
			  return (size_t)(hashFunc);                                  \
		  }                                                             \
	  };                                                              \
  }

// Example: 
//     SET_HASH_WITH_INHERITANCE(std::string, const char*, val, val.c_str())
// --> Will return (see the function operator() below): 
//   size_t operator()(const std::string& val) const 
//   { 
//     return hash<const char*>()val.c_str()); 
//   } 
#define SET_HASH_WITH_INHERITANCE(type, originalType, paramName, hashFunc)		\
  namespace __NMSPACE__ {                                                     \
	  template<>                                                                \
	  struct hash< type >                                                       \
	  {                                                                         \
		  size_t operator()(const type& paramName) const                          \
      {                                                                       \
			  return hash< originalType >()(hashFunc);                              \
		  }                                                                       \
	  };                                                                        \
  }


// WARNING: paramName should be included in hashFunc!	
// Example: 
//     SET_HASH_FUNCTION(PLearn::TVec<T>, T, val, sqrt(val))
// --> Will return:
//   template <class T >
//   struct hash< PLearn::TVec<T> >
//   {
//      size_t operator()(const PLearn::TVec<T>& val) const 
//      {								
//         return hash< T >()(sqrt(val));											
//      }	
//   }
// See below: size_t operator()(const type& paramName) const 
#define SET_HASH_FUNCTION(type, templateClass, paramName, hashFunc)  \
  namespace __NMSPACE__ {                                            \
	  template<class templateClass >                                   \
	  struct hash< type >                                              \
	  {                                                                \
		  size_t operator()(const type& paramName) const                 \
		  {                                                              \
			  return hash< templateClass >()(hashFunc);                    \
		  }                                                              \
	  };                                                               \
  }
#endif // __GNUC__

//////////////////////////////////////////////////////////////////////////////
//////// WIN32 Version ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#if (defined(WIN32) || defined(__INTEL_COMPILER)) && !defined(_MINGW_) && !defined(__GNUC__) // MinGW runs under gcc...

#include <hash_set>
#include <hash_map>

// WARNING: paramName should be included in hashFunc!	
// Example: 
//     SET_HASH_WITH_FUNCTION(float, val, PLearn::hashval(val))
// --> Will return (see the function operator() below): 
//     size_t operator()(const float& val) const
//     { 
//        return (size_t)(PLearn::hashval(val));
//     } 

#define SET_HASH_WITH_FUNCTION(type, paramName, hashFunc)     \
  namespace stdext {                                          \
	template<class _Kty>                                        \
	class hash_compare<type, std::less<_Kty> >                  \
	{                                                           \
	public:                                                     \
		enum                                                      \
		{	/* parameters for hash table */                         \
			bucket_size = 4,	/* 0 < bucket_size */                 \
			min_buckets = 8};	/* min_buckets = 2 ^^ N, 0 < N */     \
                                                              \
			hash_compare()                                          \
				: comp()                                              \
			{	/* construct with default comparator */               \
			}                                                       \
                                                              \
			hash_compare(std::less<_Kty> _Pred)                     \
				: comp(_Pred)                                         \
			{	/* construct with _Pred comparator */                 \
			}                                                       \
                                                              \
			size_t operator()(const type& paramName) const          \
			{	/* hash _Keyval to size_t value */                    \
				return (size_t)(hashFunc);                            \
			}                                                       \
                                                              \
			bool operator()(const _Kty& _Keyval1, const _Kty& _Keyval2) const\
			{	/* test if _Keyval1 ordered before _Keyval2 */        \
				return (comp(_Keyval1, _Keyval2));                    \
			}                                                       \
                                                              \
	protected:                                                  \
		std::less<_Kty> comp; /* the comparator object */         \
                                                              \
	};                                                          \
  } // end of namespace stdext

// WARNING: paramName should be included in hashFunc!	
//     SET_HASH_WITH_INHERITANCE(std::string, const char*, val, val.c_str())
// --> Will return (see the function operator() below): 
//   size_t operator()(const std::string& val) const 
//   { 
//     return stdext::hash_compare<const char*>()val.c_str()); 
//   } 
#define SET_HASH_WITH_INHERITANCE(type, originalType, paramName, hashFunc)\
  namespace stdext {                                                      \
  template<class _Kty>                                                    \
  class hash_compare<type, std::less<_Kty> >                              \
  {                                                                       \
  public:                                                                 \
    enum                                                                  \
    {	/* parameters for hash table */                                     \
      bucket_size = 4,	/* 0 < bucket_size */                             \
      min_buckets = 8};	/* min_buckets = 2 ^^ N, 0 < N */                 \
                                                                          \
      hash_compare()                                                      \
        : comp()                                                          \
			{	/* construct with default comparator */ }                         \
                                                                          \
      hash_compare(std::less<_Kty> _Pred)                                 \
        : comp(_Pred)                                                     \
			{	/* construct with _Pred comparator */ }                           \
                                                                          \
      size_t operator()(const type& paramName) const                      \
      {	/* hash _Keyval to size_t value */                                \
        return hash_compare< originalType >()(hashFunc);                  \
      }                                                                   \
                                                                          \
      bool operator()(const _Kty& _Keyval1, const _Kty& _Keyval2) const   \
      {	/* test if _Keyval1 ordered before _Keyval2 */                    \
        return (comp(_Keyval1, _Keyval2));                                \
      }                                                                   \
                                                                          \
  protected:                                                              \
    std::less<_Kty> comp;	/* the comparator object */                     \
                                                                          \
  }; }

// WARNING: paramName should be included in hashFunc!	
// Example: 
//     SET_HASH_FUNCTION(PLearn::TVec<T>, T, val, sqrt(val))
// --> Will return:
//   template <class T >
//   class stdext::hash_compare<PLearn::TVec<T>, std::less< T > >				
//   {
//   //// Some other stuffs...
//      size_t operator()(const PLearn::TVec<T>& val) const 
//      {								
//          return stdext::hash_compare< T >()(sqrt(val)); 
//      }	
//   //// Some other stuffs...
//   }
// See below: size_t operator()(const type& paramName) const 
#define SET_HASH_FUNCTION(type, templateClass, paramName, hashFunc)       \
  namespace stdext {                                                      \
	template<class templateClass >                                          \
	class hash_compare<type, std::less< templateClass > >                   \
	{                                                                       \
	public:                                                                 \
		enum			                                                            \
		{	/* parameters for hash table */			                                \
			bucket_size = 4,	/* 0 < bucket_size */                             \
			min_buckets = 8};	/* min_buckets = 2 ^^ N, 0 < N */                 \
                                                                          \
			hash_compare()                                                      \
				: comp()                                                          \
			{	/* construct with default comparator */                           \
			}                                                                   \
                                                                          \
			hash_compare(std::less< templateClass > _Pred)                      \
				: comp(_Pred)                                                     \
			{	/* construct with _Pred comparator */                             \
			}                                                                   \
                                                                          \
			size_t operator()(const type& paramName) const                      \
			{	/* hash _Keyval to size_t value */                                \
				return hash_compare< templateClass >()(hashFunc);                 \
			}                                                                   \
                                                                          \
			bool operator()(const templateClass& _Keyval1, const templateClass& _Keyval2) const	\
			{	/* test if _Keyval1 ordered before _Keyval2 */                    \
				return (comp(_Keyval1, _Keyval2));                                \
			}                                                                   \
                                                                          \
	protected:                                                              \
		std::less< templateClass > comp;	/* the comparator object */         \
                                                                          \
  }; }

#endif // WIN32 

#endif // ms_hash_wrapper_H


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
