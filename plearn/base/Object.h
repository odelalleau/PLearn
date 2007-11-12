// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2005 Pascal Vincent and Yoshua Bengio
// Copyright (C) 2002 Frederic Morin
// Copyright (C) 1999-2005 University of Montreal
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.

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


/*! \file Object.h */

#ifndef Object_INC
#define Object_INC

// Python stuff must be included first
#ifdef PL_PYTHON_VERSION 
#  include <plearn/python/PythonIncludes.h>
#  include <plearn/python/PythonObjectWrapper.h>
#endif //ifdef PL_PYTHON_VERSION 

#include <map>
#include <string>
#include "PP.h"
#include "StaticInitializer.h"
#include "TypeFactory.h"
#include "Option.h"
#include "RemoteDeclareMethod.h"
#include <plearn/io/PPath.h>
#include <plearn/io/openString.h>

namespace PLearn {
using namespace std;

/**
 * One of the following macros should be called in every Object subclass.  The
 * DECLARE macro in the class's declaration in the .h, and the corresponding
 * IMPLEMENT macro in the class's definitionin the .cc
 *
 * They automatically declare and define important methods used for the build,
 * help, and serilisation mechanism.
 *
 * The ABSTRACT versions of the macros should be used for "abstract" classes
 * (classes that are only meant to be derived, and are non instantiable because
 * they declare pure virtual methods, with no definition)
 *
 * The IMPLEMENT macros take two extra string arguments (other than the class's
 * type): a short one line description of the class, and a multiline help.
 */

#define PLEARN_DECLARE_OBJECT(CLASSTYPE)                        \
        public:                                                 \
        static  string _classname_();                           \
        virtual string classname() const;                       \
        static  OptionList& _getOptionList_();                  \
        virtual OptionList& getOptionList() const;              \
        virtual OptionMap& getOptionMap() const;                \
        static  RemoteMethodMap& _getRemoteMethodMap_();        \
        virtual RemoteMethodMap& getRemoteMethodMap() const;    \
        static  Object* _new_instance_for_typemap_();           \
        static  bool _isa_(const Object* o);                    \
        virtual CLASSTYPE* deepCopy(CopiesMap &copies) const;   \
        static  void _static_initialize_();                     \
        static  StaticInitializer _static_initializer_;         \
        static  const PPath& declaringFile()                    \
               { static PPath df= __FILE__; return df;}

#define PLEARN_IMPLEMENT_OBJECT(CLASSTYPE, ONELINEDESCR, MULTILINEHELP)                         \
        string CLASSTYPE::_classname_()                                                         \
        {                                                                                       \
            return #CLASSTYPE;                                                                  \
        }                                                                                       \
                                                                                                \
        string CLASSTYPE::classname() const                                                     \
        {                                                                                       \
            return _classname_();                                                               \
        }                                                                                       \
                                                                                                \
        OptionList& CLASSTYPE::_getOptionList_()                                                \
        {                                                                                       \
            static OptionList ol;                                                               \
            if(ol.empty())                                                                      \
                declareOptions(ol);                                                             \
            return ol;                                                                          \
        }                                                                                       \
                                                                                                \
        OptionList& CLASSTYPE::getOptionList() const                                            \
        {                                                                                       \
            return _getOptionList_();                                                           \
        }                                                                                       \
                                                                                                \
        OptionMap& CLASSTYPE::getOptionMap() const                                              \
        {                                                                                       \
            static OptionMap om;                                                                \
            if(om.empty())                                                                      \
            {                                                                                   \
                OptionList& ol= getOptionList();                                                \
                for(OptionList::iterator it= ol.begin(); it != ol.end(); ++it)                  \
                    /*N.B. option map will contain derived class's option*/                     \
                    /*  when it also exists in a base class.             */                     \
                    om.insert(make_pair((*it)->optionname(), *it));                             \
            }                                                                                   \
            return om;                                                                          \
        }                                                                                       \
                                                                                                \
        RemoteMethodMap& CLASSTYPE::_getRemoteMethodMap_()                                      \
        {                                                                                       \
            static bool initialized = false;                                                    \
            static RemoteMethodMap rmm;                                                         \
            if (! initialized) {                                                                \
                declareMethods(rmm);                                                            \
                initialized = true;                                                             \
            }                                                                                   \
            return rmm;                                                                         \
        }                                                                                       \
                                                                                                \
        RemoteMethodMap& CLASSTYPE::getRemoteMethodMap() const                                  \
        {                                                                                       \
            return _getRemoteMethodMap_();                                                      \
        }                                                                                       \
                                                                                                \
        Object* CLASSTYPE::_new_instance_for_typemap_()                                         \
        {                                                                                       \
            return new CLASSTYPE();                                                             \
        }                                                                                       \
                                                                                                \
        bool CLASSTYPE::_isa_(const Object* o)                                                  \
        {                                                                                       \
            return dynamic_cast<const CLASSTYPE*>(o) != 0;                                      \
        }                                                                                       \
                                                                                                \
        CLASSTYPE* CLASSTYPE::deepCopy(CopiesMap& copies) const                                 \
        {                                                                                       \
            CopiesMap::iterator it = copies.find(this);                                         \
            if (it != copies.end())                                                             \
                return static_cast<CLASSTYPE*>(it->second);                                     \
            CLASSTYPE* deep_copy =                                                              \
                new CLASSTYPE(dynamic_cast<const CLASSTYPE&>(*this));                           \
            copies[this] = deep_copy;                                                           \
            deep_copy->makeDeepCopyFromShallowCopy(copies);                                     \
            return deep_copy;                                                                   \
        }                                                                                       \
                                                                                                \
        void CLASSTYPE::_static_initialize_()                                                   \
        {                                                                                       \
            TypeFactory::register_type(#CLASSTYPE,                                              \
                                       inherited::_classname_(),                                \
                                       &CLASSTYPE::_new_instance_for_typemap_,                  \
                                       &CLASSTYPE::_getOptionList_,                             \
                                       &CLASSTYPE::_getRemoteMethodMap_,                        \
                                       &CLASSTYPE::_isa_,                                       \
                                       ONELINEDESCR,                                            \
                                       MULTILINEHELP,                                           \
                                       CLASSTYPE::declaringFile());                             \
        }                                                                                       \
        StaticInitializer CLASSTYPE::_static_initializer_(&CLASSTYPE::_static_initialize_)


#define PLEARN_DECLARE_ABSTRACT_OBJECT(CLASSTYPE)               \
        public:                                                 \
        static string _classname_();                            \
        static OptionList& _getOptionList_();                   \
        static RemoteMethodMap& _getRemoteMethodMap_();         \
        static bool _isa_(const Object* o);                     \
        virtual CLASSTYPE* deepCopy(CopiesMap &copies) const;   \
        static void _static_initialize_();                      \
        static StaticInitializer _static_initializer_;          \
        static  const PPath& declaringFile()                   \
               { static PPath df= __FILE__; return df;}

#define PLEARN_IMPLEMENT_ABSTRACT_OBJECT(CLASSTYPE, ONELINEDESCR, MULTILINEHELP)                \
        string CLASSTYPE::_classname_()                                                         \
        {                                                                                       \
            return #CLASSTYPE;                                                                  \
        }                                                                                       \
                                                                                                \
        OptionList& CLASSTYPE::_getOptionList_()                                                \
        {                                                                                       \
            static OptionList ol;                                                               \
            if(ol.empty())                                                                      \
                declareOptions(ol);                                                             \
            return ol;                                                                          \
        }                                                                                       \
                                                                                                \
        RemoteMethodMap& CLASSTYPE::_getRemoteMethodMap_()                                      \
        {                                                                                       \
            static bool initialized = false;                                                    \
            static RemoteMethodMap rmm;                                                         \
            if (! initialized) {                                                                \
                declareMethods(rmm);                                                            \
                initialized = true;                                                             \
            }                                                                                   \
            return rmm;                                                                         \
        }                                                                                       \
                                                                                                \
        bool CLASSTYPE::_isa_(const Object* o)                                                  \
        {                                                                                       \
            return dynamic_cast<const CLASSTYPE*>(o) != 0;                                      \
        }                                                                                       \
                                                                                                \
        CLASSTYPE* CLASSTYPE::deepCopy(CopiesMap& copies) const                                 \
        {                                                                                       \
            PLERROR("Called virtual method deepCopy of an abstract class. "                     \
                    "This should never happen!");                                               \
            return 0;                                                                           \
        }                                                                                       \
                                                                                                \
        void CLASSTYPE::_static_initialize_()                                                   \
        {                                                                                       \
            TypeFactory::register_type(#CLASSTYPE,                                              \
                                       inherited::_classname_(),                                \
                                       0,                                                       \
                                       &CLASSTYPE::_getOptionList_,                             \
                                       &CLASSTYPE::_getRemoteMethodMap_,                        \
                                       &CLASSTYPE::_isa_,                                       \
                                       ONELINEDESCR,                                            \
                                       MULTILINEHELP,                                           \
                                       CLASSTYPE::declaringFile());                             \
        }                                                                                       \
        StaticInitializer CLASSTYPE::_static_initializer_(&CLASSTYPE::_static_initialize_)


// Now for TEMPLATEs...

/* Ex: For a template class Toto

template<class T, int U> 
class Toto: public Titi<T> {
public:

typedef Titi<T> inherited;
#define TEMPLATE_DEF_Toto      class T, int U
#define TEMPLATE_ARGS_Toto     T,U
#define TEMPLATE_NAME_Toto string("Toto< ") + TypeTraits<T>::name() + ", " + tostring(U) + " >"
PLEARN_DECLARE_TEMPLATE_OBJECT(Toto)

...
};

PLEARN_IMPLEMENT_TEMPLATE_OBJECT(Toto,"One line description","Multi line help")

// Puis au besoin, pour chaque version de template instanciée, il faudra
// peut-être définir le _static_initializer_ (si le compilo n'est pas assez 
// smart pour le faire tout seul depuis la définition du template)
template<> StaticInitializer Toto<int,3>::_static_initializer_(&Toto<int,3>::_static_initialize_);

*/

#define PLEARN_DECLARE_TEMPLATE_OBJECT(CLASSTYPE)                                               \
        public:                                                                                 \
        static  string _classname_();                                                           \
        virtual string classname() const;                                                       \
        static  OptionList& _getOptionList_();                                                  \
        virtual OptionList& getOptionList() const;                                              \
        virtual OptionMap& getOptionMap() const;                                                \
        static  RemoteMethodMap& _getRemoteMethodMap_();                                        \
        virtual RemoteMethodMap& getRemoteMethodMap() const;                                    \
        static  Object* _new_instance_for_typemap_();                                           \
        static  bool _isa_(const Object* o);                                                    \
        virtual CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >* deepCopy(CopiesMap &copies) const;    \
        static  void _static_initialize_();                                                     \
        static  StaticInitializer _static_initializer_;                                         \
        static  const PPath& declaringFile() { static PPath df= __FILE__; return df;}

#define PLEARN_IMPLEMENT_TEMPLATE_OBJECT(CLASSTYPE, ONELINEDESCR, MULTILINEHELP)                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        string CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_classname_()                          \
        {                                                                                       \
            return TEMPLATE_NAME_ ## CLASSTYPE ;                                                \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        string CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::classname() const                      \
        {                                                                                       \
            return _classname_();                                                               \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        OptionList& CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_getOptionList_()                 \
        {                                                                                       \
            static OptionList ol;                                                               \
            if(ol.empty())                                                                      \
              declareOptions(ol);                                                               \
            return ol;                                                                          \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        OptionList& CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::getOptionList() const             \
        {                                                                                       \
            return _getOptionList_();                                                           \
        }                                                                                       \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        OptionMap& CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::getOptionMap() const               \
        {                                                                                       \
            static OptionMap om;                                                                \
            if(om.empty())                                                                      \
            {                                                                                   \
                OptionList& ol= getOptionList();                                                \
                for(OptionList::iterator it= ol.begin(); it != ol.end(); ++it)                  \
                    /*N.B. option map will contain derived class's option*/                     \
                    /*  when it also exists in a base class.             */                     \
                    om.insert(make_pair((*it)->optionname(), *it));                             \
            }                                                                                   \
            return om;                                                                          \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        RemoteMethodMap& CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_getRemoteMethodMap_()       \
        {                                                                                       \
            static bool initialized = false;                                                    \
            static RemoteMethodMap rmm;                                                         \
            if (! initialized) {                                                                \
                declareMethods(rmm);                                                            \
                initialized = true;                                                             \
            }                                                                                   \
            return rmm;                                                                         \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        RemoteMethodMap& CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::getRemoteMethodMap() const   \
        {                                                                                       \
            return _getRemoteMethodMap_();                                                      \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        Object* CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_new_instance_for_typemap_()          \
        {                                                                                       \
            return new CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >();                              \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        bool CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_isa_(const Object* o)                   \
        {                                                                                       \
            return dynamic_cast<const CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >*>(o) != 0;       \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >*                                               \
        CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::deepCopy(CopiesMap& copies) const             \
        {                                                                                       \
            CopiesMap::iterator it = copies.find(this);                                         \
            if (it != copies.end())                                                             \
                return static_cast<CLASSTYPE*>(it->second);                                     \
            CLASSTYPE* deep_copy = new CLASSTYPE(dynamic_cast<const CLASSTYPE&>(*this));        \
            copies[this] = deep_copy;                                                           \
            deep_copy->makeDeepCopyFromShallowCopy(copies);                                     \
            return deep_copy;                                                                   \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        void CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_static_initialize_()                    \
        {                                                                                       \
            TypeFactory::register_type(                                                         \
                CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_classname_(),                        \
                CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::inherited::_classname_(),             \
                &CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_new_instance_for_typemap_,          \
                &CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_getOptionList_,                     \
                &CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_getRemoteMethodMap_,                \
                &CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_isa_,                               \
                ONELINEDESCR,                                                                   \
                MULTILINEHELP,                                                                  \
                CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::declaringFile());                     \
        }                                                                                       \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        StaticInitializer CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::                            \
            _static_initializer_(&CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::                    \
                                 _static_initialize_);

/** Declare a partially specialized class specific to a given class. This is to
 * ensure the 'diff' static method that is called in Option.h is the one
 * specific to this class.
*/
#define DECLARE_SPECIALIZED_DIFF_CLASS(CLASSTYPE)                                               \
        template<class ObjectType>                                                              \
        class DiffTemplate<ObjectType, CLASSTYPE> {                                             \
            public:                                                                             \
                static int diff(const string& refer, const string& other,                       \
                                const Option<ObjectType, CLASSTYPE>* opt,                       \
                                PLearnDiff* diffs)                                              \
                {                                                                               \
                    return PLearn::diff(refer, other, opt, diffs);                              \
                }                                                                               \
        };
 
/**
 *  The following macro should be called just *after* the declaration of an
 *  object subclass. It declares and defines a few inline functions needed for
 *  the serialization of pointers to the newly declared object type and the
 *  comparison (diff) with other objects.
 */

#define DECLARE_OBJECT_PTR(CLASSTYPE)                                                   \
        inline Object *toObjectPtr(const CLASSTYPE &o)                                  \
        {                                                                               \
            return const_cast<CLASSTYPE *>(&o);                                         \
        }                                                                               \
                                                                                        \
        inline PStream &operator>>(PStream &in, CLASSTYPE &o)                           \
        {                                                                               \
            o.newread(in);                                                              \
            return in;                                                                  \
        }                                                                               \
                                                                                        \
        inline PStream &operator>>(PStream &in, CLASSTYPE * &o)                         \
        {                                                                               \
            Object *ptr = o;                                                            \
            in >> ptr;                                                                  \
            o = dynamic_cast<CLASSTYPE *>(ptr);                                         \
            if(ptr!=0 && o==0)                                                          \
              PLERROR("Mismatched classes while reading a pointer: %s is not a %s",     \
                   ptr->classname().c_str(),CLASSTYPE::_classname_().c_str());          \
            return in;                                                                  \
        }                                                                               \
                                                                                        \
        inline PStream &operator<<(PStream &out, const CLASSTYPE &o)                    \
        {                                                                               \
            o.newwrite(out);                                                            \
            return out;                                                                 \
        }                                                                               \
                                                                                        \
        inline PStream &operator>>(PStream &in, PP<CLASSTYPE> &o)                       \
        {                                                                               \
            Object *ptr = (CLASSTYPE *)o;                                               \
            in >> ptr;                                                                  \
            o = dynamic_cast<CLASSTYPE *>(ptr);                                         \
            if(ptr!=0 && o.isNull())                                                    \
              PLERROR("Mismatched classes while reading a PP: %s is not a %s",          \
                   ptr->classname().c_str(),CLASSTYPE::_classname_().c_str());          \
            return in;                                                                  \
        }                                                                               \
                                                                                        \
        template<class ObjectType>                                                      \
        int diff(const string& refer, const string& other,                              \
                 const Option<ObjectType, CLASSTYPE>* opt, PLearnDiff* diffs)           \
        {                                                                               \
            PP<OptionBase> new_opt = new Option<ObjectType, PP<CLASSTYPE> >             \
                (opt->optionname(), 0, 0, "", "", "", opt->level());                    \
            return new_opt->diff(refer, other, diffs);                                  \
        }                                                                               \
        DECLARE_SPECIALIZED_DIFF_CLASS(CLASSTYPE)                                       \
        DECLARE_TYPE_TRAITS(CLASSTYPE)

#define DECLARE_TEMPLATE_OBJECT_PTR(CLASSTYPE)                                                  \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        inline Object *toObjectPtr(const CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  &o)          \
        {                                                                                       \
            return const_cast<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  *>(&o);                 \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        inline PStream &operator>>(PStream &in, CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  &o)   \
        {                                                                                       \
            o.newread(in); return in;                                                           \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        inline PStream &operator>>(PStream &in, CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  * &o) \
        {                                                                                       \
            if (o)                                                                              \
                o->newread(in);                                                                 \
            else                                                                                \
                o = static_cast<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  *>(readObject(in));   \
            return in;                                                                          \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        inline PStream &                                                                        \
        operator<<(PStream &out, const CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  &o)            \
        {                                                                                       \
            o.newwrite(out);                                                                    \
            return out;                                                                         \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        inline PStream&                                                                         \
        operator>>(PStream &in, PP<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE > > &o)               \
        {                                                                                       \
            Object *ptr = (CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  *)o;                       \
            in >> ptr;                                                                          \
            o = dynamic_cast<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  *>(ptr);                 \
            return in;                                                                          \
        }                                                                                       \
                                                                                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE >                                                 \
        class TypeTraits< CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE > >                            \
        {                                                                                       \
        public:                                                                                 \
            static inline string name()                                                         \
            { return CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_classname_(); }                 \
                                                                                                \
            static inline unsigned char little_endian_typecode()                                \
            { return 0xFF; }                                                                    \
                                                                                                \
            static inline unsigned char big_endian_typecode()                                   \
            { return 0xFF; }                                                                    \
        } 



/**
 *  The following macro should be called after the declaration of a new
 *  SmartPointer derived class. It will declare a number of inline functions
 *  used to serialize the new smart pointer type
 */

#define DECLARE_OBJECT_PP_SERIALIZE(PPCLASSTYPE, CLASSTYPE)             \
        inline PStream &operator>>(PStream &in, PPCLASSTYPE &o)         \
          { Object *ptr = 0;                                            \
            in >> ptr;                                                  \
            o = dynamic_cast<CLASSTYPE *>(ptr);                         \
            return in; }                                                \
        inline PStream &operator<<(PStream &out, const PPCLASSTYPE &o)  \
          { out << static_cast<const PP<CLASSTYPE> &>(o); return out; } \
        DECLARE_TYPE_TRAITS(PPCLASSTYPE)

#ifdef PL_PYTHON_VERSION 
#define DECLARE_OBJECT_PP(PPCLASSTYPE, CLASSTYPE)                       \
        template<> struct ConvertFromPyObject<PPCLASSTYPE>              \
        {                                                               \
            static PPCLASSTYPE convert(PyObject* o,                     \
                                       bool print_traceback)            \
            { return PPCLASSTYPE(ConvertFromPyObject<PP<CLASSTYPE> >    \
                                  ::convert(o, print_traceback)); }     \
        };                                                              \
        template<> struct ConvertToPyObject<PPCLASSTYPE>                \
        {                                                               \
            static PyObject* newPyObject(const PPCLASSTYPE& x)          \
            {return ConvertToPyObject<PP<CLASSTYPE> >::newPyObject(x);} \
        };                                                              \
        DECLARE_OBJECT_PP_SERIALIZE(PPCLASSTYPE, CLASSTYPE)
#else //def PL_PYTHON_VERSION 
#define DECLARE_OBJECT_PP(PPCLASSTYPE, CLASSTYPE)                       \
        DECLARE_OBJECT_PP_SERIALIZE(PPCLASSTYPE, CLASSTYPE)
#endif //def PL_PYTHON_VERSION 


//#####  PLearn::Object  ######################################################

/**
 *  @class Object
 *  @brief Object is the base class of all high level PLearn objects.
 *
 *  @par It exposes simple mechanisms for:
 *
 *  @li automatic memory management (through reference counting and smart pointers)
 *  @li serialization (read, write, save, load)
 *  @li runtime type information (classname)
 *  @li displaying (info, print)
 *  @li deep copying (deepCopy)
 *  @li remote method calling mechanism (call(), declareMethods())
 *  @li a generic way of setting options (setOption) when not knowing the
 *      exact type of the Object and a generic build() method (the combination
 *      of the two allows to change the object structure and rebuild it at
 *      runtime)
 *
 *  NOTES ON THE PLEARN OBJECT MODEL.  All "significant" PLearn classes inherit
 *  from this class, PLearn::Object, as their ultimate base.  Each PLearn class
 *  defines a number of "options", which are data fields that are reflected in
 *  the serialized representation of the object.  Hence, the set of options
 *  describes the capabilities of an object from the viewpoint of the script
 *  writer.  The meaning of each option is documented within each class.
 *
 *  Moreover, all options are given a set of flags.  The meaning of each flag
 *  is as follows:
 *
 *  @li \c buildoption : an option typically specified before calling the
 *  initial build (semantically similar to a constructor parameter), for
 *  instance the number of hidden units in a neural net.
 *
 *  @li \c learntoption : n option whose proper value is computed by the class
 *  after construction (not to be set by the user before build) and potentially
 *  complex operations such as learning over a training set.  Example: the
 *  (trained) weights of a neural net.
 *
 *  @li \c tuningoption : an option typically set after the initial build, to tune
 *  the object
 *
 *  @li \c nosave : when set, this flag requests the option not to be saved in
 *  the object serialisation; mostly used when we must have several options
 *  that map to the same physical data field, for script backward
 *  compatibility.
 *
 *  @li \c nonparentable : when this flag is set, the option does not lead to a
 *  parenting relationship in the "ParentableObject" sense.  In other words,
 *  the object pointed to by this option does not get its parent() backpointer
 *  set to this. (See the ParentableObject class for details).
 *
 *  @li \c nontraversable : when this flag is set, the option is not traversed
 *  by the ObjectGraphIterator class (and ipso facto by related functions, such
 *  as memfun_broadcast. (See the ObjectGraphIterator class for details).
 */
class Object: public PPointable
{
    typedef Object inherited;

public:
    //#####  Public Interface  ################################################
    
    PLEARN_DECLARE_OBJECT(Object);

    /**
     *  Default Constructor.
     *
     *  SUBCLASS WRITING: Note: all subclasses should define a default
     *  constructor (one that can be called without arguments), whose main role
     *  is to give a reasonable default value to all build options (see
     *  declareOptions).  Completing the actual building of the object is left
     *  to the build_() and build() methods (see below).
     */
    Object(bool call_build_ = false);

    //! Virtual Destructor
    virtual ~Object();

    // We rely on the default compiler-generated copy constructor and
    // assignment operator.

    /**
     *  Post-constructor.  The normal implementation should call simply
     *  inherited::build(), then this class's build_().  This method should be
     *  callable again at later times, after modifying some option fields to
     *  change the "architecture" of the object.
     */
    virtual void build();
    
    /**
     *  Does the necessary operations to transform a shallow copy (this) into a
     *  deep copy by deep-copying all the members that need to be.  This needs
     *  to be overridden by every class that adds "complex" data members to the
     *  class, such as \c Vec, \c Mat, \c PP<Something>, etc.  Typical
     *  implementation:
     *
     *  @code
     *  void CLASS_OF_THIS::makeDeepCopyFromShallowCopy(CopiesMap& copies)
     *  {
     *      inherited::makeDeepCopyFromShallowCopy(copies);
     *      deepCopyField(complex_data_member1, copies);
     *      deepCopyField(complex_data_member2, copies);
     *      ...
     *  }
     *  @endcode
     *
     *  @param copies  A map used by the deep-copy mechanism to keep track of
     *                 already-copied objects.
     */
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    /**
     *  Returns a bit more informative string about object (default returns
     *  classname())
     *
     *  @return Information about the object
     */
    virtual string info() const; 

    /**
     * Returns a string representation of the object
     */
    virtual string asString() const; 

    //#####  Options-Related Functions  #######################################
    
    /**
     *  Reads and sets the value for the specified option from the specified
     *  stream.  Fully-qualified options syntax of the form
     *  "option[i].suboption.etc" is supported.
     *
     *  @param in          PStream from which to read the new option value
     *  @param optionname  Name of the option to read from the stream
     */
    void readOptionVal(PStream &in, const string &optionname);
    
    /**
     *  Writes the value of the specified option to the specified stream.
     *  Fully-qualified options syntax of the form "option[i].suboption.etc" is
     *  supported.
     *
     *  @param out         PStream into which write the option value
     *  @param optionname  Name of the option to write out to the stream
     */
    void writeOptionVal(PStream &out, const string &optionname) const;

    /**
     *  Parses a fully qualified option name into the following parts:
     *
     *  - The actual object it refers to (may not be \c this)
     *  - The iterator corresponding to the option within the object's
     *    OptionList.
     *  - The index number (or string) in the case of an indexed option,
     *    or "" if the option is not indexed.
     *
     *  @param[in]  optionname    Name of the option to parse
     *  @param[out] final_object  Pointer to the ultimate object within which
     *                            the option is contained
     *  @param[out] option_iter   Iterator within the \c final_object
     *                            OptionList pointing to the Option object
     *  @param[out] option_index  If we are accessing an indexed option, this
     *                            is the index (which is a string, to allow
     *                            eventual indexing of maps); otherwise ""
     *  @return                   If \c true, the option was found; if \c false
     *                            the option was not found; all output parameters
     *                            contain undefined values.
     *
     *  Note: this function implements the basic mechanism of readOptionVal and
     *  writeOptionVal; the latter functions should be re-implemented in terms
     *  of the present function.
     */
    bool parseOptionName(const string& optionname,
                         Object*& final_object,
                         OptionList::iterator& option_iter,
                         string& option_index);

    /**
     *  Const overload of parseOptionName.
     */
    bool parseOptionName(const string& optionname,
                         const Object*& final_object,
                         OptionList::iterator& option_iter,
                         string& option_index) const;

    /**
     *  Returns a string of the names of all options to save.
     *  (optionnames are to be separated by a space, and must be supported by
     *  writeOptionVal)
     *
     *  @return List of options that don't have the \c OptionBase::nosave flag,
     *          separated by spaces.
     */
    virtual string getOptionsToSave() const;

    /**
     *  Returns a string of the names of all options to transmit when
     *  talking to remote PLearn hosts (see getOptionsToSave())
     */
    virtual string getOptionsToRemoteTransmit() const;

    /**
     *  Set an option (a data field) into an object.  This is a generic method
     *  to be able to set an option in an object in the most generic manner
     *  value is a string representation of the value to be set. It should only
     *  be called for initial construction or reloading of an object, prior to
     *  calling build(); To modify the options of an already built object, call
     *  changeOptions or changeOption instead.  If no option with that name
     *  exists, it causes an "Unknown option" runtime error.
     *
     *  The implementation calls \c readOptionVal() on a stringstream built
     *  from \c value.
     *
     *  @param optionname  Name of option to set
     *  @param value       String representation of the value to set the option
     *                     to.  Note that the actual value set into the option
     *                     is converted from the string representation into a
     *                     genuine C++ object acceptable for the option type.
     */
    void setOption(const string& optionname, const string& value);

#ifdef PL_PYTHON_VERSION 
    void setOptionFromPython(const string& optionname, const PythonObjectWrapper& value);
#endif //def PL_PYTHON_VERSION 

    /**
     * Returns true if the object has an option with the given name. 
     *  @param optionname  Name of option to get
     *  @return            bool indicating whether the option exists.
     */
    bool hasOption(const string &optionname) const;

    /**
     *  Return the string representation of an object's option (data field).
     *  This is a generic method to be able to retrieve the value of an
     *  option supported by the object (and its derivatives). The option
     *  value is returned as a string and MUST be converted to the correct type
     *  before use.
     *
     *  The implementation calls writeOptionVal into a string stream.
     *
     *  @param optionname  Name of option to get
     *  @return            String representation of the option value
     */
    string getOption(const string &optionname) const;

    /**
     *  Set multiple options simultaneously into the object.  This method
     *  should be used, rather than \c setOption(), when modifying some options
     *  of an already built object.  The default version simply calls \c
     *  setOption(), but subclasses should override it to execute any code
     *  required to put the object in a consistent state.  If the set of
     *  options would put the object in an inconsistent state, a runtime error
     *  should be issued.
     *
     *  @param name_value  Map of optionname:optionvalue pairs to set into the
     *                     object
     */
    virtual void changeOptions(const map<string,string>& name_value);

    //! Non-virtual method calls virtual \c changeOptions()
    void changeOption(const string& optionname, const string& value);


    //#####  Input/Output-Related Functions  ##################################

    /**
     *  The default implementation serializes the object in the new format:
     *
     *  @code
     *  Classname(optionname=optionval; optionname=optionval; ...)
     *  @endcode
     *
     *  Subclasses may override this method to provide different outputs
     *  depending on \c out's mode (\c plearn_ascii, \c raw_ascii, ...).
     *
     *  @param in  Stream onto which serialize the object
     */
    virtual void newwrite(PStream& out) const;

    /**
     *  The default implementation reads and builds an object in the new format:
     *
     *  @code
     *  Classname(optionname=optionval; optionname=optionval; ...)
     *  @endcode
     *
     *  @param in  Stream from which read the object
     */
    void newread(PStream& in);


    //#####  Remote Method Invocation  ########################################

    /**
     *  The call method is the standard way to allow for remote method
     *  invocation on instances of your class.  This should result in reading
     *  \c nargs input parameters from \c io, call the appropriate method, and
     *  send results to \c io.  A "Remote-callable method" is typically
     *  associated with an actual methods of your class, but it will usually
     *  differ in its "calling" conventions: its "name", number or input
     *  arguments, and number and nature of output results may differ.
     *
     *  EASIEST WAY TO IMPLEMENT REMOTE CALLING: override the declareMethods()
     *  function (see below) in your derived class, and for each method that
     *  you wish to be remote-callable, call a declareMethod() function to
     *  declare its existence and attach some documentation.  This is very
     *  similar in flavor to declareOptions()/declareOption().
     * 
     *  OLD-FASHIONED IMPLEMENTATION.  Here is the "traditional version" of
     *  what this function should do in a derived class.  For most derived
     *  classes that use the declareMethod mechanism, it is no longer necessary
     *  to override this method.
     *  <ol>
     *  <li> Determine from the methodname what actual method to call.
     *       If the given methodname is none of those supported by your call method,
     *       call the parent's "call".
     *       Ex: <tt>inherited::call(methodname, nargs, io)</tt>
     *  <li> The number of arguments nargs may also influence what version of the
     *       method you want to call
     *  <li> read the narg arguments from io Ex: io >> age >> length >> n; 
     *  <li> call the actual associated method
     *  <li> call <tt>prepareToSendResults(io, nres)</tt> where nres is the
     *       number of result parameters. 
     *  <li> send the nres result parameters to io Ex: io << res1 << res2 <<res3;
     *  <li> call <tt>io.flush()</tt>
     *  </ol>
     * 
     *  If anything goes wrong during the process (bad arguments, etc...)
     *  simply call PLERROR with a meaningful message.
     *
     *  @param methodname  Name of method to call
     *  @param nargs       Number of arguments passed to the method
     *  @param io          Stream expected to contain \c nargs input arguments
     *                     and on which will be written the returned object
     */
    virtual void call(const string& methodname, int nargs, PStream& io);

    //! Must be called by the call method prior to sending results.
    static void prepareToSendResults(PStream& out, int nres);

    /**
     *  Override this for runnable objects (default method issues a runtime
     *  error).  Runnable objects are objects that can be used as *THE* object
     *  of a .plearn script.  The run() method specifies what they should do
     *  when executed.
     */
    virtual void run();


    //#####  Deprecated Interfaces  ###########################################

    /**
     *  Write the object to a C++ \c ostream.
     *
     *  The write method should write a complete description of the object to
     *  the given stream, that should be enough to later reconstruct it.  (a
     *  somewhat human-readable ascii format is usually preferred).  The new
     *  default version simply calls newwrite(...) which simply writes all the
     *  "options" declared in declareOptions, so there is no need to overload
     *  write in subclasses.  Old classes that still override write should
     *  progressively be moved to the new declareOptions/build mechanism.
     *
     *  @deprecated  Use the declareOption / build mechanism instead, that provides
     *               automatic serialization
     */
    virtual void write(ostream& out) const;

    /**
     *  Read the object from a C++ \c istream.
     *
     *  The read method is the counterpart of the write method. It should be
     *  able to reconstruct an object that has been previously written with the
     *  write method. The current implementation automatically decides whether
     *  to call newread() (which is based on the new declareOptions/build
     *  mechanism) or oldread() for backward compatibility (if the header is of
     *  the form <ClassName>).
     *
     *  @deprecated  Use the declareOption / build mechanism instead, that
     *               provides automatic serialization
     */
    virtual void read(istream& in);

    //! @deprecated  For backward compatibility with old saved object
    virtual void oldread(istream& in);

    /**
     *  @deprecated It simply calls the generic PLearn save function (that can
     *  save any PLearn object): PLearn::save(filename, *this) So you should
     *  call PLearn::save directly (it's defined in plearn/io/load_and_save.h).
     */
    virtual void save(const PPath& filename) const;

    /**
     *  @deprecated It simply calls the generic PLearn load function (that can
     *  load any PLearn object): PLearn::load(filename, *this) So you should
     *  call PLearn::load directly (it's defined in plearn/io/load_and_save.h).
     */
    virtual void load(const PPath& filename);

    /**
     * deepCopyNoMap: same as deepCopy, but starts with an empty CopiesMap
     * and returns an Object*.
     */
    Object* deepCopyNoMap();

#ifdef PL_PYTHON_VERSION 
    /**
     * pyDeepCopy: deep copy called from python
     */
    tuple<Object*, CopiesMap> pyDeepCopy(CopiesMap& copies);
#endif //def PL_PYTHON_VERSION 

protected:
    //#####  Protected Member Functions  ######################################

    /**
     *  Declare options (data fields) for the class.  Redefine this in
     *  subclasses: call \c declareOption(...) for each option, and then call
     *  \c inherited::declareOptions(options).  Please call the \c inherited
     *  method AT THE END to get the options listed in a consistent order (from
     *  most recently defined to least recently defined).
     *
     *  @code
     *  static void MyDerivedClass::declareOptions(OptionList& ol)
     *  {
     *      declareOption(ol, "inputsize", &MyObject::inputsize_,
     *                    OptionBase::buildoption,
     *                    "The size of the input; it must be provided");
     *      declareOption(ol, "weights", &MyObject::weights,
     *                    OptionBase::learntoption,
     *                    "The learned model weights");
     *      inherited::declareOptions(ol);
     *  }
     *  @endcode
     *
     *  @param ol  List of options that is progressively being constructed for
     *             the current class.
     */
    static void declareOptions(OptionList& ol) { }

    /**
     *  Declare methods that are intended to be remote-callable.  If you use
     *  this mechanism, you don't usually need to override the call method in
     *  derived classes.  The typical form of this method is as follows:
     *
     *  @code
     *  static void MyDerivedClass::declareMethods(RemoteMethodMap& rmm)
     *  {
     *      // Insert a backpointer to inherited methods; note that this
     *      // mechanism is different from that of declareOptions()
     *      rmm.inherited(inherited::_getRemoteMethodMap_());
     *
     *      // Mind the extra pair of parenthesis around the docstrings.
     *      // They are necessary for proper construction of documentation.
     *      declareMethod(rmm, "method1", &MyDerivedClass::method1,
     *                    (BodyDoc("Main documentation for the method"),
     *                     ArgDoc ("arg1_name", "Documentation for argument1"),
     *                     ArgDoc ("arg2_name", "Documentation for argument2"),
     *                     // ... other ArgDoc here ...
     *                     RetDoc ("Documentation for the return value")));
     *
     *      // Other calls to declareMethod() as appropriate to expose the
     *      // public remote-callable interface
     *  }
     *  @endcode
     *
     *  IMPORTANT REMARKS:
     * 
     *  The types of methods that can be directly declared in this manner is
     *  restricted to methods that don't have any "result-arguments" (and are
     *  either void, or *return* their result).  But in C/C++ it is customary
     *  to implement "multiple results" by passing them as "result-arguments"
     *  to the call. You can't use declareMethod on such a method: you'll first
     *  have to write a wrapper method of the correct form that you can declare
     *  with declareMethod. To *return* multiple results, you should actually
     *  return a *tuple*.
     *
     *  Ex: if you have a method of class PLearner with 2 "result arguments"
     *  like:
     *
     *  @code
     *  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
     *                                     Vec& output, Vec& costs) const;
     *  @endcode
     *
     *  you can't declare it directly with declareMethod, so you'll have to
     *  write a wrapper-method that you can declare, like the following:
     *
     *  @code
     *  tuple<Vec,Vec> PLearner::remote_computeOutputAndCosts(const Vec& input, 
     *                                                        const Vec& target) const
     *  {
     *    Vec output;
     *    Vec costs;
     *    computeOutputAndCosts(input,target,output,costs);
     *    return make_tuple(output, costs);
     *  }
     *  @endcode
     *
     *  The policy is to name such wapper methods destined for the remote
     *  method mechanism by prepending the suffix remote_, and usually to keep
     *  them private and non-virtual.
     *
     *  Note that from the calling-convention perspective of a C++ process
     *  remote-calling such a tuple-returning method, the results will be
     *  received as "multiple results" corresponding to the elements of the
     *  tuple, rather than as a "single result" of type tuple. If instead you
     *  *really* want your tuple to be received as a single tuple then you should
     *  return a tuple of your tuple.
     *
     *  Also beware, if you have several C++ methods with the same name,
     *  overloaded for different types of arguments, and you want to make them
     *  all remote callable, you should declare them with *different*
     *  corresponding string names in declareMethods. Indeed, the remote method
     *  mechanism can only distinguish methods based on their string name and
     *  number of arguments, but not on the types of the arguments.
     *
     *  @param ol  RemoteMethodMap to be constructed for the current class.
     */
    static void declareMethods(RemoteMethodMap& rmm);

private:
    //#####  Private Member Functions  ########################################

    /**
     *  Object-specific post-constructor.  This method should be redefined in
     *  subclasses and do the actual building of the object according to
     *  previously set option fields.  Constructors can just set option fields,
     *  and then call build_.  This method is NOT virtual, and will typically
     *  be called only from three places: a constructor, the public virtual \c
     *  build() method, and possibly the public virtual read method (which
     *  calls its parent's read).  \c build_() can assume that its parent's \c
     *  build_() has already been called.
     */
    void build_();

    //! Version of save that's called by Remote Method Invocation. Our
    //! convention is to have such methods start with the remote_ prefix.
    void remote_save(const string& filepath, const string& io_formatting) const;
};


//! Calls a function previously declared with the declareFunction mechanism.
//! This is used by the remote function call mechanism, and is the
//! equivalent of method Object::call but for global functions and static methods.
void callFunction(const string& funcname, int nargs, PStream& io);


//#####  Inline Implementations  ##############################################

/**
 *  This function builds an object from its representation in the stream.
 *  It understands several representations:
 *
 *  @li The <ObjectClass> ... </ObjectClass> type of representation 
 *      as is typically produced by write() serialization methods and functions.
 *      This will call the object's read() method.
 *  @li The ObjectClass( optionname=optionvalue; ... ; optionname=optionvalue )
 *      type of representation (typical form for human input), will result in 
 *      appropriate calls of the object's setOption() followed by its build().
 *  @li load( filepath ) will call loadObject
 */
Object *readObject(PStream &in, unsigned int id = UINT_MAX);
inline Object *readObject(istream &in_)
{
    PStream in(&in_);
    return readObject(in);
}

//! Loads an object from the given file (no macro-preprocessing is performed)
Object* loadObject(const PPath &filename);

//! Same as loadObject but first performs macro-processing on the file
//! vars may be initialised with the values of some variables
//! and upon return it will also contain newly $DEFINED variables 
Object* macroLoadObject(const PPath &filename, map<string,string>& vars);

//! same as previous, but no need to pass a variables map
Object* macroLoadObject(const PPath &filename);

//! Creates a new object according to the given representation.
//! This actually calls readObject on a PStream obtained with 
//! openString,  so anything
//! understandable by readObject can be used here
inline Object* newObject(const string& representation)
{
    PStream in = openString(representation, PStream::plearn_ascii);
    return readObject(in);
}

Object* newObjectFromClassname(const string& classname);

Object* remote_deepCopy(Object* source);

inline PStream &operator>>(PStream &in, Object &o)
{
    o.newread(in);
    return in;
}

inline PStream &operator<<(PStream &out, const Object &o)
{
    o.newwrite(out);
    return out;
}


//! This takes precedence over the template definitions for a template type T in PStream.h
PStream &operator>>(PStream &in, Object * &o);

} // end of namespace PLearn

//! Useful function for debugging inside gdb:
extern "C" void printobj(PLearn::Object* p);

#endif //!<  Object_INC


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
