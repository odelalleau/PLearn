// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin

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
   * $Id: Object.h,v 1.22 2003/09/17 15:27:29 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Object.h */

#ifndef Object_INC
#define Object_INC

#include <map>
#include <set>
#include "general.h"
#include "PP.h"
#include "StaticInitializer.h"
#include "TypeTraits.h"
#include "Array.h"
#include "stringutils.h"
#include "TypeFactory.h"
#include "OptionBase.h"
#include "Option.h"

namespace PLearn <%
using namespace std;

/*! Object is the base class of all high level PLearn objects. It exposes simple mechanisms for:
    - automatic memory management (through reference counting and smart pointers)
    - serialization (read, write, save, load)
    - runtime type information (classname)
    - displaying (info, print)
    - deep copying (deepCopy)
    - a generic way of setting options (setOption) when not knowing the
    exact type of the Object and a generic build() method (the combination
    of the two allows to change the object structure and rebuild it at
    runtime)
*/

/*! One of the following macros should be called in every Object subclass.
   The DECLARE macro in the class's declaration in the .h, 
   and the corresponding IMPLEMENT macro in the class's definitionin the .cc

   They automatically declare and define important methods used for the build, help, and serilisation mechanism.

   The ABSTRACT versions of the macros should be used for "abstract" classes
   (classes that are only meant to be derived, and are non instantiable
   because they declare pure virtual methods, with no definition)

   The IMPLEMENT macros take two extra string arguments (other than the class's type): a short one line description of the class, and a multiline help.

*/

// ---------------------------------------------------------

#define PLEARN_DECLARE_OBJECT(CLASSTYPE)    \
        public:                                                            \
        static string _classname_();                                       \
        virtual string classname() const;                                  \
        static OptionList& _getOptionList_();                              \
        virtual OptionList& getOptionList() const;                         \
        static Object* _new_instance_for_typemap_();                       \
        static bool _isa_(Object* o);                                      \
        virtual CLASSTYPE* deepCopy(CopiesMap &copies) const;             \
        static void _static_initialize_();                                \
	      static StaticInitializer _static_initializer_;                    

#define PLEARN_IMPLEMENT_OBJECT(CLASSTYPE, ONELINEDESCR, MULTILINEHELP)     \
	      string CLASSTYPE::_classname_()                                       \
          { return #CLASSTYPE; }                                               \
	      string CLASSTYPE::classname() const                                   \
          { return _classname_(); }                                           \
        OptionList& CLASSTYPE::_getOptionList_()                              \
          { static OptionList ol;                                             \
            if(ol.empty())                                                    \
              declareOptions(ol);                                             \
            return ol; }                                                      \
        OptionList& CLASSTYPE::getOptionList() const                          \
          { return _getOptionList_(); }                                       \
        Object* CLASSTYPE::_new_instance_for_typemap_()                       \
          { return new CLASSTYPE(); }                                         \
        bool CLASSTYPE::_isa_(Object* o)                                      \
          { return dynamic_cast<CLASSTYPE*>(o) != 0; }                        \
        CLASSTYPE* CLASSTYPE::deepCopy(CopiesMap& copies) const	              \
          { return implementDeepCopy<CLASSTYPE>(copies); }                    \
        void CLASSTYPE::_static_initialize_()                                 \
          { TypeFactory::register_type( \
            #CLASSTYPE,  \
	          inherited::_classname_(), \
            &CLASSTYPE::_new_instance_for_typemap_, \
            &CLASSTYPE::_getOptionList_, \
            &CLASSTYPE::_isa_,  \
            ONELINEDESCR,   \
            MULTILINEHELP  ); }\
	      StaticInitializer CLASSTYPE::_static_initializer_(&CLASSTYPE::_static_initialize_);               


#define PLEARN_DECLARE_ABSTRACT_OBJECT(CLASSTYPE)    \
        public:                                                            \
        static string _classname_();                                       \
        static OptionList& _getOptionList_();                              \
        static bool _isa_(Object* o);                                      \
        virtual CLASSTYPE* deepCopy(CopiesMap &copies) const;             \
        static void _static_initialize_();                                \
	      static StaticInitializer _static_initializer_;                    

#define PLEARN_IMPLEMENT_ABSTRACT_OBJECT(CLASSTYPE, ONELINEDESCR, MULTILINEHELP)     \
	      string CLASSTYPE::_classname_()                                       \
          { return #CLASSTYPE; }                                               \
        OptionList& CLASSTYPE::_getOptionList_()                              \
          { static OptionList ol;                                             \
            if(ol.empty())                                                    \
              declareOptions(ol);                                             \
            return ol; }                                                      \
        bool CLASSTYPE::_isa_(Object* o)                                      \
          { return dynamic_cast<CLASSTYPE*>(o) != 0; }                         \
        CLASSTYPE* CLASSTYPE::deepCopy(CopiesMap& copies) const	              \
          { PLERROR("Called virtual method deepCopy of an abstract class. This should never happen!"); \
            return 0; } \
        void CLASSTYPE::_static_initialize_()                                 \
          { TypeFactory::register_type( \
            #CLASSTYPE,  \
	          inherited::_classname_(), \
            0, \
            &CLASSTYPE::_getOptionList_, \
            &CLASSTYPE::_isa_, \
            ONELINEDESCR, \
            MULTILINEHELP  ); }\
	      StaticInitializer CLASSTYPE::_static_initializer_(&CLASSTYPE::_static_initialize_);               


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

PLEARN_IMPLEMENT_TEMPLATE_OBJECT(Toto)

// Puis au besoin, pour chaque version de template instanciée, il faudra
// peut-être définir le _static_initializer_ (si le compilo n'est pas assez 
// smart pour le faire tout seul depuis la définition du template)
template<> StaticInitializer Toto<int,3>::_static_initializer_(&Toto<int,3>::_static_initialize_);

*/

#define PLEARN_DECLARE_TEMPLATE_OBJECT(CLASSTYPE)    \
        public:                                                            \
        static string _classname_();                                       \
        virtual string classname() const;                                  \
        static OptionList& _getOptionList_();                              \
        virtual OptionList& getOptionList() const;                         \
        static Object* _new_instance_for_typemap_();                       \
        static bool _isa_(Object* o);                                      \
        virtual CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >* deepCopy(CopiesMap &copies) const; \
        static void _static_initialize_();                                \
	      static StaticInitializer _static_initializer_;                    

#define PLEARN_IMPLEMENT_TEMPLATE_OBJECT(CLASSTYPE, ONELINEDESCR, MULTILINEHELP)     \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
	      string CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_classname_()   \
          { return TEMPLATE_NAME_ ## CLASSTYPE ; }                         \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
	      string CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::classname() const \
          { return _classname_(); }                                           \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        OptionList& CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_getOptionList_()  \
          { static OptionList ol;                                             \
            if(ol.empty())                                                    \
              declareOptions(ol);                                             \
            return ol; }                                                      \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        OptionList& CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::getOptionList() const \
          { return _getOptionList_(); }                                       \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        Object* CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_new_instance_for_typemap_() \
          { return new CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >(); }                   \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        bool CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_isa_(Object* o)                \
          { return dynamic_cast<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >*>(o) != 0; }  \
        CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >* CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::deepCopy(CopiesMap& copies) const \
          { return implementDeepCopy<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE > >(copies); } \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        void CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_static_initialize_()   \
          { TypeFactory::register_type( \
        CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_classname_(), \
	      CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::inherited::_classname_(), \
        &CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_new_instance_for_typemap_, \
        &CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_getOptionList_, \
        &CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_isa_, \
        ONELINEDESCR, \
        MULTILINEHELP  ); }\
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
	      StaticInitializer CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_static_initializer_(&CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_static_initialize_);



/*! The following macro should be called just *after* the declaration of
    an object subclass. It declares and defines a few inline functions needed for
    the serialisation of pointers to the newly declared object type. */

#define DECLARE_OBJECT_PTR(CLASSTYPE)                                      \
        inline Object *toObjectPtr(const CLASSTYPE &o)                     \
          { return const_cast<CLASSTYPE *>(&o); };                         \
        inline PStream &operator>>(PStream &in, CLASSTYPE &o)        \
          { o.newread(in); return in; };                                   \
        inline PStream &operator>>(PStream &in, CLASSTYPE * &o)      \
          { if (o) o->newread(in);                                         \
            else o = static_cast<CLASSTYPE *>(readObject(in));             \
            return in; };                                                  \
        inline PStream &operator<<(PStream &out, const CLASSTYPE &o) \
          { o.newwrite(out); return out; };                                \
        inline PStream &operator>>(PStream &in, PP<CLASSTYPE> &o)    \
          { Object *ptr = (CLASSTYPE *)o;                                  \
            in >> ptr;                                                     \
            o = dynamic_cast<CLASSTYPE *>(ptr);                            \
            return in;                                                     \
          };                                                               \
        DECLARE_TYPE_TRAITS(CLASSTYPE)

#define DECLARE_TEMPLATE_OBJECT_PTR(CLASSTYPE)                                      \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        inline Object *toObjectPtr(const CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  &o)                     \
          { return const_cast<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  *>(&o); };                         \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        inline PStream &operator>>(PStream &in, CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  &o)        \
          { o.newread(in); return in; };                                   \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        inline PStream &operator>>(PStream &in, CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  * &o)      \
          { if (o) o->newread(in);                                         \
            else o = static_cast<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  *>(readObject(in));             \
            return in; };                                                  \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        inline PStream &operator<<(PStream &out, const CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  &o) \
          { o.newwrite(out); return out; };                                \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
        inline PStream &operator>>(PStream &in, PP<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE > > &o)    \
          { Object *ptr = (CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  *)o;                                  \
            in >> ptr;                                                     \
            o = dynamic_cast<CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >  *>(ptr);                            \
            return in;                                                     \
          };                                                               \
        template < TEMPLATE_DEF_ ## CLASSTYPE > \
         class TypeTraits< CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE > >       \
           {                                                    \
             public:                                              \
               static inline string name()                           \
                 { return CLASSTYPE< TEMPLATE_ARGS_ ## CLASSTYPE >::_classname_(); }  \
            static inline unsigned char little_endian_typecode()                    \
             { return 0xFF; }                                                      \
            static inline unsigned char big_endian_typecode()                \
             { return 0xFF; }                                                \
            };



/*! The following macro should be called after the declaration of a 
   new SmartPointer derived class. It will declare a number of inline 
   functions used to serialize the new smart pointer type */

#define DECLARE_OBJECT_PP(PPCLASSTYPE, CLASSTYPE)                          \
        inline PStream &operator>>(PStream &in, PPCLASSTYPE &o)      \
          { Object *ptr = 0;                                               \
            in >> ptr;                                                     \
            o = dynamic_cast<CLASSTYPE *>(ptr);                            \
            return in; };                                                  \
        inline PStream &operator<<(PStream &out, const PPCLASSTYPE &o)    \
          { out << static_cast<const PP<CLASSTYPE> &>(o); return out; };  \
        DECLARE_TYPE_TRAITS(PPCLASSTYPE);


//! The Object class

  class Object: public PPointable
  {
  protected:

    //! redefine this in subclasses: call declareOption(...) for each
    //! option, and then call inherited::declareOptions(options)
    //! ( see the declareOption function further down)
    /*! ex: 
    static void declareOptions(OptionList& ol)
    {
      declareOption(ol, "inputsize", &MyObject::inputsize_, OptionBase::buildoption, "the size of the input\n it must be provided");
      declareOption(ol, "weights", &MyObject::weights, OptionBase::learntoption, "the learnt model weights");
      inherited::declareOptions(ol);
    }
    */

    static void declareOptions(OptionList& ol) {}
    
    // Must be called by the call method prior to dending results. 
    inline void prepareToSendResults(PStream& out, int nres)
    { out << nres; }

  public:

    // hack:
    typedef Object inherited;
    PLEARN_DECLARE_OBJECT(Object);   

    //! SUBCLASS WRITING 
    //! Note: all subclasses should define a default constructor (one that can be called without arguments),
    //! whose main role is to give a reasonable default value to all build options (see declareOptions).
    //! Completing the actual building of the object is left to the build_() and build() methods (see below).
    Object();

    //! redefine this in subclasses
    //! Should return a multiline string with a short description of what this
    //! object does, as well as a description of available build-options
    //! (and their default value)
    static string help();

    


  private:
/*! This method should be redefined in subclasses and 
    do the actual building of the object according to
    previously set option fields.  
    Constructors can just set option fields, and then call build_. 
    This method is NOT virtual, and will typically be called only from three places:
    a constructor, the public virtual build() method, and possibly the public virtual
    read method (which calls its parent's read).
    build_() can assume that it's parent's build_ has already been called.
*/
    void build_();

  public:
    //! Should call simply inherited::build(), then this class's build_()
    /*! This method should be callable again at later times,
      after modifying some option fields to change the "architecture" of
      the object. */
    virtual void build();
    



/*!       Does the necessary operations to transform a shallow copy (this)
      into a deep copy by deep-copying all the members that need to be.
      Typical implementation:
      
      void CLASS_OF_THIS::makeDeepCopyFromShallowCopy(CopiesMap& copies)
      {
        SUPERCLASS_OF_THIS::makeDeepCopyFromShallowCopy(copies);
        member_ptr = member_ptr->deepCopy(copies);
        member_smartptr = member_smartptr->deepCopy(copies);
        member_mat.makeDeepCopyFromShallowCopy(copies);
        member_vec.makeDeepCopyFromShallowCopy(copies);
        ...
      }
*/
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //!  returns a bit more informative string about object (default returns
    //!  classname())
    virtual string info() const; 

/*!       Prints a human-readable, short (not necessarily complete)
      description of this object instance (default prints info()).  This
      is what is called by operator<< on Object
*/
    virtual void print(ostream& out) const; 

    // For temporary backward compatibility. To be removed ultimately. This simply returns an empty string now...
    static string optionHelp();

    //! Reads and sets the value for the specified option from the specified stream
    void readOptionVal(PStream &in, const string &optionname);
    
    //! Writes the value of the specified option to the specified stream
    void writeOptionVal(PStream &out, const string &optionname) const;

    //! returns a string of the names of all options to save 
    //! (optionnames are to be separated by a space, and must be supported by writeOptionVal)
    virtual string getOptionsToSave() const;

    //! Serializes this object in the new format 
    //! Classname(optionname=optionval; optionname=optionval; ...)
    void newwrite(PStream& out) const;


    //! reads and builds an object in the new format 
    //! Classname(optionname=optionval; optionname=optionval; ...)
    void newread(PStream& in);

/*!       This is a generic method to be able to set an option in an object in
      the most generic manner value is a string representation of the
      value to be set.  Object::setOption causes an "Unknown option"
      runtime error.
*/
    // This calls readOptionVal on a stringstream built from value
    void setOption(const string& optionname, const string& value);

/*!       This is a generic method to be able to retrieve the value of an
      option supported by the object (and its derivatives). The option
      value is returned as a string and MUST be converted to the correct type
      before use.
*/
    // This calls writeOptionVal into a string stream
    string getOption(const string &optionname) const;
    
/*!   DEPRECATED (use the declareOption / build_ mecanism instead, that provides automatic serialization)
  The write method should write a complete description of the object to the given
      stream, that should be enough to later reconstruct it.  (a somewhat
      human-readable ascii format is usually preferred).
      The new default version simply calls newwrite(...) which simply
      writes all the "options" declared in declareOptions, so
      there is no need to overload write in subclasses.
      Old classes that still overload write should progressively be moved to 
      the new declareOptions/build mechanism.
*/
    virtual void write(ostream& out) const;

/*!     DEPRECATED (use the declareOption / build_ mecanism instead, that provides automatic serialization)
      The read method is the counterpart of the write method. It should be
      able to reconstruct an object that has been previously written with
      the write method. The current implementation automatically decides whether 
      to call newread() (which is based on the new declareOptions/build mechanism)
      or oldread() for backward compatibility (if the header is of the form <ClassName>).
*/
    virtual void read(istream& in);

    //! The call method is the standard way to allow for remote method invocation on instances of your class.    
    /*! This should result in reading nargs input parameters from in_parameters, 
        call the appropriate method, and send results to out_results. 
        A "Remote-callable method" is typically associated with an actual methods of your class, 
        but it will usually differ in its "calling" conventions: its "name", 
        number or input arguments, and number and nature of output results may differ.

      Here is what such a method should do:
         1) Determine from the methodname what actual method to call.
            If the given methodname is none of those supported by your call method,
            call the parent's "call" Ex: inherited::call(methodname, nargs, in_parameters, out_results)
         2) The number of arguments nargs may also influence what version of the method you want to call
         3) read the narg arguments from in_parameters Ex: in_parameters >> age >> length >> n; 
         4) call the actual associated method
         5) call prepareToSendResults(out_results, nres) where nres is the number of result parameters. 
         6) send the nres result parameters to out_results Ex: out_results << res1 << res2 <<res3;
         7) call out_results.flush()

         If anything goes wrong during the process (bad arguments, etc...) simply call PLERROR 
         with a meaningful message.
    */
    virtual void call(const string& methodname, int nargs, PStream& in_parameters, PStream& out_results);

    //! Overload this for runnable objects 
    //! (default method issues a runtime error)
    //! Runnable objects are objects that can be used
    //! as *THE* object of a .plearn script.
    //! The run() method specifies what they should do when executed. 
    virtual void run();

    //! DEPRECATED For backward compatibility with old saved object
    virtual void oldread(istream& in);


    //! DEPRECATED methods: do not use, ignore them.  
    
    

    //! This method is deprecated. It simply calls the generic PLearn save function
    //! (that can save any PLearn object): PLearn::save(filename, *this) 
    //! So you should call PLearn::save directly (it's defined in pl_io.h).
    virtual void save(const string& filename) const;

    //! This method is deprecated. It simply calls the generic PLearn load function
    //! (that can load any PLearn object): PLearn::load(filename, *this) 
    //! So you should call PLearn::load directly (it's defined in pl_io.h).
    virtual void load(const string& filename);

    virtual ~Object();

  protected:
/*!       This is a 'default' implementation of deepCopy; provided for helping
      derived classes; for some reason I don't understand with the
      compiler (EGCS), this currently has to be inline (NC 2000/12/12)
*/
    template <class DerivedClass>
    DerivedClass* implementDeepCopy(CopiesMap& copies) const
    {
      CopiesMap::iterator it = copies.find(this);
      //cout << "deep copy of object " << this << " of class " << classname();
      if (it != copies.end())
        {
          //cout << "found in the table, so return " << it->second << endl;
          return static_cast<DerivedClass*>(it->second);
        }
    
      DerivedClass* deep_copy =
        new DerivedClass(dynamic_cast<const DerivedClass&>(*this));
      //cout << "not found in the table, so create a new one at " << deep_copy << " and call makedeepcopyfromshallowcopy" << endl;
      if (usage() > 1)
      {
        //cout << "put this->deep_copy in the table" << endl;
        copies[this] = deep_copy;
      } // else cout << "this object does not seem to have multiple pointers to it" << endl;
      deep_copy->makeDeepCopyFromShallowCopy(copies);
      return deep_copy;
    }
  };

  //! The toObjectPtr functions attempt to return a pointer to Object 
  //! (or 0 if the passed argument cannot be considered an Object subclass)

template<class T> inline Object* toObjectPtr(const T* x) // Never to be called stub
{ PLERROR("toObjectPtr() - Unexpected error"); return 0; }

template<class T> inline Object* toObjectPtr(const T& x) // Never to be called stub
{ PLERROR("toObjectPtr() - Unexpected error"); return 0; }


template<> inline Object* toObjectPtr(const Object &x)
{ return const_cast<Object *>(&x); };

template<> inline Object* toObjectPtr(const Object *x)
{ return const_cast<Object *>(x); };


template<class T> inline Object* toObjectPtr(const PP<T>& x)
{ return toObjectPtr(*static_cast<T *>(x)); }

template<class T> Object* toIndexedObjectPtr(const Array<T> &x, int i)
{ return toObjectPtr(static_cast<T &>(x[i])); }

template<class T> Object *toIndexedObjectPtr(const T&, int) // Never to be called stub
{ PLERROR("toIndexedObjectPtr() - Unexpected error"); return 0; };


/*! This function builds an object from its representation in the stream.
    It understands several representations:
    - The <ObjectClass> ... </ObjectClass> type of representation 
      as is typically produced by write() serialization methods and functions.
      This will call the object's read() method.
    - The ObjectClass( optionname=optionvalue; ... ; optionname=optionvalue )
      type of representation (typical form for human input), will result in 
      appropriate calls of the object's setOption() followed by its build().
    - load( filepath ) will call loadObject
*/
  Object *readObject(PStream &in, unsigned int id = LONG_MAX);
  inline Object *readObject(istream &in_)
      { PStream in(&in_); return readObject(in); };

  //!  Same as previously, but takes a filename rather than a istream
  Object *loadObject(const string &filename);

  //! Creates a new object according to the given representation.
  //! This actually calls readObject on an istrstream, so anything
  //! understandable by readObject can be used here
  inline Object* newObject(const string& representation)
  { istrstream in(representation.c_str()); return readObject(in); }

  inline ostream& operator<<(ostream& out, const Object& obj)
    { obj.print(out); return out; }

  //!  The following functions are used to help you write the function
  //!  makeDeepCopyFromShallowCopy()

  //!  Any type derived from Object: call makeDeepCopyFromShallowCopy
  template <>
  inline void deepCopyField(Object& field, CopiesMap& copies)
  {
    field.makeDeepCopyFromShallowCopy(copies);
  }
 
  inline PStream &operator>>(PStream &in, Object &o)
    { o.newread(in); return in; }
  inline PStream &operator<<(PStream &out, const Object &o)
    { o.newwrite(out); return out; }


// This takes precedence over the template definitions for a template type T in PStream.h
  PStream &operator>>(PStream &in, Object * &o);


%> // end of namespace PLearn

//! Useful function for debugging inside gdb:
extern "C"
{
  void printobj(PLearn::Object* p);
}



#endif //!<  Object_INC
