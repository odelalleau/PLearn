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
   * $Id: Object.h,v 1.7 2002/10/03 07:35:27 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Object.h */

#ifndef Object_INC
#define Object_INC

#include <map>
#include <set>
#include "general.h"
#include "PP.h"
#include "TypeFactory.h"
#include "Array.h"
#include "stringutils.h"


namespace PLearn <%
using namespace std;

  class OptionBase;  // predeclare for methods in Object
  typedef vector< PP<OptionBase> > OptionList;

/*!     This base class of PLearn objects exposes simple mechanisms for:
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

    // this returns a static OptionList, filled the first time by calling declareOptions
    // It is declared/implemented automatically when you call the macros 
    // DECLARE_NAME_AND_DEEPCOPY and IMPLEMENT_NAME_AND_DEEPCOPY
    virtual OptionList& getOptionList() const;

    //!  A few utility methods to help you in writing the read method
    void readAndCheck(istream& in, const string& value);

    //! Utility method used to send status in call(...) method implementations.
    inline void sendRMIStatus(ostream& out, int status)
    { PLearn::write(out, status); }

  public:
    Object();

  public:
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

/*!       Returns a deep copy of this object copies[xxx] will be used as the
      copy of object xxx (if present), rather than a fresh copy.  Typical
      implementation of deepCopy look as follows:
      
      DerivedClass* DerivedClass::deepCopy(CopiesMap& copies) const
      {
          return implementDeepCopy<DerivedClass>(copies);
      }
      
      Before you implement yourself this method, see the macros
      DECLARE_NAME_AND_DEEPCOPY and IMPLEMENT_NAME_AND_DEEPCOPY that are
      defined below, and that would probably save you some work.
*/
    virtual Object* deepCopy(CopiesMap& copies) const;

    //!  returns name of the class
    virtual string classname() const; 

    //!  returns a bit more informative string about object (default returns
    //!  classname())
    virtual string info() const; 

/*!       Prints a human-readable, short (not necessarily complete)
      description of this object instance (default prints info()).  This
      is what is called by operator<< on Object
*/
    virtual void print(ostream& out) const; 

    //! Returns a help string with the list of build options
    //! for this object (includes inherited options from parent classes)
    /*! Warning: the values printed as "default" are the current instance's values, so they
      correspond to the actual "defaults" only when invoked on an instance freshly constructed
      with the default constructor... */
    string optionHelp() const;

    //! Reads and sets the value for the specified option from the specified stream
    /*! Subclasses should overload this, typically following this pattern:
      void MyClass::readOptionVal(istream& in, const string& optionname)
      {
      if(optionname=="myopt1")
        PLearn::read(in, myopt1);
      else if(optionname=="myopt2")
        PLearn::read(in,myopt2);
      ....
      else // maybe an option of the parent class...
        inherited::readOptionVal(in, optionname);
      } 
    */    
    void readOptionVal(PStream &in, const string &optionname);
    
    //! Writes the value of the specified option to the specified stream
    /*! Subclasses should overload this, following a similar parretn to readOptionVal */
    void writeOptionVal(PStream &out, const string &optionname) const;

    //! returns a string of the names of all options to save 
    //! (optionnames are to be separated by a space, and must be supported by writeOptionVal)
    virtual string getOptionsToSave(OBflag_t option_flag = dft_option_flag) const;

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
    // Pascal: the new version calls readOptionVal, and is no longer virtual
    // Subclasses must not overload this method, but readOptionVal.
    void setOption(const string& optionname, const string& value);

    //! Should return a multiline string with a short description of what this
    //! object does, as well as a description of available build-options
    //! (and their default value)
    virtual string help() const;

/*!       This is a generic method to be able to retrieve the value of an
      option supported by the object (and its derivatives). The option
      value is returned as a string and MUST be converted to the correct type
      before use.
*/
    // Pascal: the new version calls writeOptionVal, and is no longer virtual
    // Subclasses must not overload this method, but writeOptionVal.
    string getOption(const string &optionname) const;
    
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
    //! Should call inherited::build(), then this class's build_()
    /*! This method should be callable again at later times,
      after modifying some option fields to change the "architecture" of
      the object. */
    virtual void build();
    
/*!       The write method should write a complete description of the object to the given
      stream, that should be enough to later reconstruct it.  (a somewhat
      human-readable ascii format is usually preferred).
      The new default version simply calls newwrite(...) which simply
      writes all the "options" declared in declareOptions, so
      there is no need to overload write in subclasses.
      Old classes that still overload write should progressively be moved to 
      the new declareOptions/build mechanism.
*/
    virtual void write(ostream& out) const;

/*!       The read method is the counterpart of the write method. It should be
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
        [ Whenever we write send status integer, the integer must be sent with sendRMIStatus(value),
          This status is also returned by the function. Negative values mean failure of some sort. ]

         1) determine which actual method to call based only on methodname and, possibly, nargs (argument types cannot be used)
            => If no such remote method is defined, send status integer -1 meaning "bad method name"
            => If the number of arguments is invalid, send status integer -2 meaning "invalid number of arguments"
         2) read the narg arguments in sequence with PLearn::read(in_parameters, ...)
            => If an error occurs, send and return status integer -3 meaning "problem while reading arguments"
         3) call the actual associated method
            => If an error occurs, send and return status integer -4 meaning "method call failed"
         4) If all went OK, set status integer to the number of "output results" and send it
         5) write all output results in sequence with PLearn::write(out_results, ...)
         6) return status integer
    */
    virtual int call(const string& methodname, int nargs, istream& in_parameters, ostream& out_results);

    //! Overload this for runnable objects (default method issues a runtime error)
    virtual void run();

    //! For backward compatibility with old saved object
    virtual void oldread(istream& in);


    virtual void deepWrite(ostream& out, DeepWriteSet& already_saved) const;
    virtual void deepRead(istream& in, DeepReadMap& old2new);

/*!       These call read and write so you should not override them in
      subclasses (they are virtual only for backward compatibility, but it
      won't last!).
*/
    virtual void save(const string& filename) const;
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
      if (it != copies.end())
        return static_cast<DerivedClass*>(it->second);
    
      DerivedClass* deep_copy =
        new DerivedClass(dynamic_cast<const DerivedClass&>(*this));
      if (usage() > 1)
        copies[this] = deep_copy;
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

//! Base class for option definitions
class OptionBase: public PPointable
{
public:
  typedef OBflag_t flag_t; // To change type of flag_t, change type of OBflag_t in pl_io.h instead

  //! 'buildoption' an option typically specified before calling the initial build 
  //! (semantically similat to a constructor parameter) ex: the number of hidden units in a neural net
  static const flag_t buildoption;       

  //! 'learntoption' a field whose proper value is computed by the class after construction
  //! (not to be set by the user before build) ex: the weights of a neural net
  static const flag_t learntoption;

  //! 'tuningoption' an option typically set after the initial build, to tune the object
  static const flag_t tuningoption;

  //! Do not include this option in the objet's serialisation (write method skips it)
  static const flag_t nosave; 

protected:
  string optionname_;  // the name of the option
  flag_t flags_; 
  string optiontype_;  // the datatype of the option ("int" ...)
  string defaultval_;  // string representation of the default value (will be printed by optionHelp())
  string description_; // A description of this option

public:

  //! Most of these parameters only serve to provide the user 
  //! with an informative help text. (only optionname and saveit are really important)
  OptionBase(const string& optionname, flag_t flags,
             const string& optiontype, const string& defaultval, 
             const string& description)
    :optionname_(optionname), flags_(flags), 
    optiontype_(optiontype), defaultval_(defaultval),
    description_(description) {}

  virtual void read(Object* o, PStream& in) const = 0;
  virtual void read_and_discard(PStream& in) const = 0;
  virtual void write(const Object* o, PStream& out) const = 0;

  // writes the option into a string instead of a stream
  // (calls write on a string stream) 
  string writeIntoString(const Object* o) const;
  
  virtual Object* getAsObject(Object* o) const = 0;
  virtual const Object* getAsObject(const Object* o) const = 0;
  virtual Object *getIndexedObject(Object *o, int i) const = 0;
  virtual const Object *getIndexedObject(const Object *o, int i) const = 0;    
  
  //! Returns the name of the class in to which this field belongs
  virtual string optionHolderClassName(const Object* o) const = 0;

  //! The name of the option (field)
  inline const string& optionname() const { return optionname_; }
  inline bool isOptionNamed(string name) const { return name == optionname(); }
  
  inline const string& optiontype() const { return optiontype_; }
  inline const string& defaultval() const { return defaultval_; }
  inline const string& description() const { return description_; }
  inline flag_t flags() const { return flags_; }

  //! change the string representation of the default value
  inline void setDefaultVal(const string& newdefaultval)
  { defaultval_ = newdefaultval; }
};

//! Template class for option definitions
template<class ObjectType, class OptionType, class ConstOptionType>
class Option: public OptionBase
{
protected:
  OptionType ObjectType::*ptr;

public:

  //! Most of these parameters only serve to provide the user 
  //! with an informative help text. (only optionname and saveit are really important)
  Option(const string& optionname, OptionType ObjectType::* member_ptr, 
         flag_t flags, const string& optiontype, const string& defaultval,
         const string& description)
    :OptionBase(optionname, flags, optiontype, defaultval, description),
    ptr(member_ptr) {}
  virtual void read(Object* o, PStream& in) const
  { in >> dynamic_cast<ObjectType*>(o)->*ptr; }
  virtual void read_and_discard(PStream& in) const
  { 
    string dummy;
    smartReadUntilNext(in.rawin(), ";)", dummy);
    in.unget();
    //OptionType op; //dummy object that will be destroyed after read
    //in >> op; //read dummy object
  }
  virtual void write(const Object* o, PStream& out) const
  { out << static_cast<ConstOptionType>(dynamic_cast<const ObjectType *>(o)->*ptr); }

  virtual Object* getAsObject(Object* o) const
  { return toObjectPtr(dynamic_cast<ObjectType*>(o)->*ptr); }

  virtual const Object* getAsObject(const Object* o) const
  { return toObjectPtr(dynamic_cast<const ObjectType*>(o)->*ptr); }

  virtual Object *getIndexedObject(Object *o, int i) const
  { return toIndexedObjectPtr(dynamic_cast<ObjectType*>(o)->*ptr, i); };

  virtual const Object* getIndexedObject(const Object *o, int i) const
  { return toIndexedObjectPtr(dynamic_cast<const ObjectType*>(o)->*ptr, i); };

  virtual string optionHolderClassName(const Object* o) const
  { return dynamic_cast<const ObjectType*>(o)->ObjectType::classname(); }
};

//! For flags, you should specify one of 
//! OptionBase::buildoption, OptionBase::learntoption or OptionBase::tuningoption
//! If the option is not to be serialized, you can additionally specify 
//! OptionBase::nosave
/*! The "type" printed in optionHelp() is given by TypeTraits<OptionType>::name().
    The "default value" printed in optionHelp() will be a serialization of 
    the value of the field in a default constructed instance, (which should be ok in most cases),
    unless you explicitly specify it as the last argument here (It is recomended that you *don't*
    specify it explicitly, unless you really must). */

template<class ObjectType, class OptionType>
inline void declareOption(OptionList& ol,                      //!< the list to which this option should be appended 
                          const string& optionname,            //!< the name of this option
                          OptionType ObjectType::* member_ptr, //!< &YourClass::your_field
                          OptionBase::flag_t flags,            //! see the flags in OptionBase
                          const string& description,           //!< a description of the option
                          const string & defaultval="")        //!< the default value for this option, as set by the default constructor
{ ol.push_back(new Option<ObjectType, OptionType, const OptionType>(optionname, member_ptr, flags, 
                                                                    TypeTraits<OptionType>::name(), 
                                                                    defaultval, description)); }
template<class ObjectType, class OptionType>
inline void declareOption(OptionList& ol, const string& optionname, OptionType *ObjectType::* member_ptr, OptionBase::flag_t flags,
                          const string& description, const string & defaultval="")
{ ol.push_back(new Option<ObjectType, OptionType *, const OptionType *>(optionname, member_ptr, flags, TypeTraits<OptionType *>::name(), 
                                                                        defaultval, description)); }

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

  //!  the old address is read (and write) as an "unsigned long"
  Object* deepReadObject(istream& in, DeepReadMap& old2new);
  inline Object* deepReadObject(istream& in)
  { DeepReadMap old2new; return deepReadObject(in, old2new); }


  inline ostream& operator<<(ostream& out, const Object& obj)
    { obj.print(out); return out; }

  inline void deepRead(istream& in, DeepReadMap& old2new, Object*& ptr)
  {
    unsigned long old_ptr;
    read(in, old_ptr);
    if (old_ptr)
    {
      if (old2new.find(old_ptr) != old2new.end())
        ptr = (Object*)old2new[old_ptr];
      else
      {
        ptr = deepReadObject(in, old2new); 
        old2new[old_ptr] = ptr;
      }
    }
    else
      ptr = 0;
  }

  //!  The following functions are used to help you write the function
  //!  makeDeepCopyFromShallowCopy()

  //!  Any type derived from Object: call makeDeepCopyFromShallowCopy
  template <>
  inline void deepCopyField(Object& field, CopiesMap& copies)
  {
    field.makeDeepCopyFromShallowCopy(copies);
  }


  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, const Object& o)
  { o.deepWrite(out, already_saved); }

  inline void deepRead(istream& in, DeepReadMap& old2new, Object& o) 
  { o.deepRead(in, old2new); }

  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, Object* ptr)
  {
    write(out, (unsigned long)ptr);
    if (ptr)
    {
      if (already_saved.find((void*)ptr) == already_saved.end())  //!<  not found
      {
        already_saved.insert((void*)ptr);
        deepWrite(out, already_saved, *ptr);
      }
    }
  }
 
  inline PStream &operator>>(PStream &in, Object &o)
    { o.newread(in); return in; }
  inline PStream &operator<<(PStream &out, const Object &o)
    { o.newwrite(out); return out; }

  PStream &operator>>(PStream &in, Object * &o);
  PStream &operator<<(PStream &out, const Object * &o);
/*
  inline pl_istream &
  operator>>(pl_istream &in, PP<Object> &o)
  {

      if (Object *obj = readObject(in)) {
          o = dynamic_cast<T *>(obj);
          if (o.isNull())
              PLERROR("Object is of wrong type \"%s\"", obj->classname().c_str());
      } else
          o = 0;
      return in;
  }
*/

/*!     The following macros are meant to help you write the classname() and
    deepCopy() methods in a systematic manner for every class.  Within your
    class definition (in the .H), add the line:
    
    class MyClass : public Object {
        ...
        DECLARE_NAME_AND_DEEPCOPY(MyClass);
    };
    
    In you .CC, add the following line:
    
    IMPLEMENT_NAME_AND_DEEPCOPY(MyClass);
    
    When you do so, you will automatically have a working classname()
    method added to the class, along with a working deepCopy() method.
    *DO NOTE* that the method makeDeepCopyFromShallowCopy is not
    implemented by these macros, and need to be defined manually.
    
    There is an additional subtlety with ABSTRACT CLASSES.
    You should use the macros DECLARE_ABSTRACT_NAME_AND_DEEPCOPY and 
    IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY which will actually compile 
    if the class is abstract; the implementation of the method 
    yields an error, but the return type is correct.
        
    NEW STUFF: TYPE FACTORY-related.  In addition to declaring the
    classname() and deepCopy() methods, the macros are now used to
    automate the declaration of classes derived from Object inside the
    global system TypeMap (see TypeFactory.h).  When you call these macros,
    the class MyClass is registered with the global TypeMap (which is
    required for the disk input/output functions to work properly in
    Object), and if the class is concrete, then you can call the TypeMap
    to instantiate new empty objects given only the name of the class as a
    string.

    CHANGE FROM PASCAL: Abstract classes are no longer declared in the TypeFactory.
    As they could not be instantiated anyway, there was no point in including them,
    and it prevented looping through all classes declared in the TypeFactory 
    to list all Learner subclasses for example. This is now possible.
*/


#define DECLARE_ABSTRACT_NAME(CLASSNAME)                                   \
        public:                                                            \
          string classname() const
#define IMPLEMENT_ABSTRACT_NAME(CLASSNAME)                                 \
	string CLASSNAME::classname() const                                \
          { return #CLASSNAME; }

#define DECLARE_ABSTRACT_DEEPCOPY(CLASSNAME)                               \
        CLASSNAME *deepCopy(CopiesMap &copies) const
#define IMPLEMENT_ABSTRACT_DEEPCOPY(CLASSNAME)                             \
	CLASSNAME* CLASSNAME::deepCopy(CopiesMap& copies) const	           \
	  { PLERROR("deepCopy is not implemented for %s",                  \
              classname().c_str());		                           \
	    return 0; }

#define DECLARE_NAME(CLASSNAME)	                                           \
        DECLARE_ABSTRACT_NAME(CLASSNAME);                                  \
        static Object* _new_instance_for_typemap_();                       \
	      static TypeRegistrar _register_in_typemap_
#define IMPLEMENT_NAME(CLASSNAME)                                          \
        IMPLEMENT_ABSTRACT_NAME(CLASSNAME);                                \
        Object* CLASSNAME::_new_instance_for_typemap_()                    \
          { return new CLASSNAME(); }                                      \
        TypeRegistrar CLASSNAME::_register_in_typemap_(                    \
	      #CLASSNAME, &CLASSNAME::_new_instance_for_typemap_)
#define IMPLEMENT_ABSTRACT_TEMPLATE_NAME(CLASSNAME, TEMPLATETYPE)          \
        template <class TEMPLATETYPE> string                               \
        CLASSNAME<TEMPLATETYPE>::classname() const                         \
          { return #CLASSNAME"<"#TEMPLATETYPE">"; };            
#define IMPLEMENT_TEMPLATE_NAME(CLASSNAME, TEMPLATETYPE)                   \
        IMPLEMENT_ABSTRACT_TEMPLATE_NAME(CLASSNAME, TEMPLATETYPE)          \
        template <class TEMPLATETYPE> Object *                             \
        CLASSNAME<TEMPLATETYPE>::_new_instance_for_typemap_()              \
          { return new CLASSNAME<TEMPLATETYPE>(); }

#define DECLARE_DEEPCOPY(CLASSNAME)                                        \
        DECLARE_ABSTRACT_DEEPCOPY(CLASSNAME)
#define IMPLEMENT_DEEPCOPY(CLASSNAME)                                      \
        CLASSNAME* CLASSNAME::deepCopy(CopiesMap& copies) const	           \
          { return implementDeepCopy<CLASSNAME>(copies); }
#define IMPLEMENT_TEMPLATE_DEEPCOPY(CLASSNAME, TEMPLATETYPE)               \
        template <class TEMPLATETYPE> CLASSNAME<TEMPLATETYPE> *            \
        CLASSNAME<TEMPLATETYPE>::deepCopy(CopiesMap &copies) const         \
          { return implementDeepCopy<CLASSNAME<TEMPLATETYPE> >(copies); }

#define IMPLEMENT_ABSTRACT_TEMPLATE_DEEPCOPY(CLASSNAME, TEMPLATETYPE)      \
        template <class TEMPLATETYPE> CLASSNAME<TEMPLATETYPE> *            \
	CLASSNAME<TEMPLATETYPE>::deepCopy(CopiesMap& copies) const         \
	  { PLERROR("deepCopy is not implemented for %s",                  \
              classname().c_str());		                           \
	    return 0; }

#define DECLARE_OBJECT_PTR(CLASSNAME)                                      \
        inline Object *toObjectPtr(const CLASSNAME &o)                     \
          { return const_cast<CLASSNAME *>(&o); };                         \
        inline PStream &operator>>(PStream &in, CLASSNAME &o)        \
          { o.newread(in); return in; };                                   \
        inline PStream &operator>>(PStream &in, CLASSNAME * &o)      \
          { if (o) o->newread(in);                                         \
            else o = static_cast<CLASSNAME *>(readObject(in));             \
            return in; };                                                  \
        inline PStream &operator<<(PStream &out, const CLASSNAME &o) \
          { o.newwrite(out); return out; };                                \
        inline PStream &operator>>(PStream &in, PP<CLASSNAME> &o)    \
          { Object *ptr = (CLASSNAME *)o;                                  \
            in >> ptr;                                                     \
            o = dynamic_cast<CLASSNAME *>(ptr);                            \
            return in;                                                     \
          };                                                               \
        DECLARE_TYPE_TRAITS(CLASSNAME)

#define DECLARE_OBJECT_PP(PPCLASSNAME, CLASSNAME)                          \
        inline PStream &operator>>(PStream &in, PPCLASSNAME &o)      \
          { Object *ptr;                                                   \
            in >> ptr;                                                     \
            o = dynamic_cast<CLASSNAME *>(ptr);                            \
            return in; };                                                  \
        inline PStream &operator<<(PStream &out, const PPCLASSNAME &o) \
          { out << static_cast<const PP<CLASSNAME> &>(o); return out; };

#define DECLARE_NAME_AND_DEEPCOPY(CLASSNAME)                               \
        DECLARE_NAME(CLASSNAME);                                           \
        DECLARE_DEEPCOPY(CLASSNAME);                                       \
        virtual OptionList& getOptionList() const
#define IMPLEMENT_NAME_AND_DEEPCOPY(CLASSNAME)	                           \
        IMPLEMENT_NAME(CLASSNAME);                                         \
        IMPLEMENT_DEEPCOPY(CLASSNAME);                                     \
        OptionList& CLASSNAME::getOptionList() const                       \
          { static OptionList ol;                                          \
            if(ol.empty())                                                 \
              declareOptions(ol);                                          \
            return ol; }

#define DECLARE_TEMPLATE_NAME_AND_DEEPCOPY(CLASSNAME, TEMPLATETYPE)        \
        DECLARE_NAME_AND_DEEPCOPY(CLASSNAME<TEMPLATETYPE>)
#define IMPLEMENT_TEMPLATE_NAME_AND_DEEPCOPY(CLASSNAME, TEMPLATETYPE)      \
        IMPLEMENT_TEMPLATE_NAME(CLASSNAME, TEMPLATETYPE);                  \
        IMPLEMENT_TEMPLATE_DEEPCOPY(CLASSNAME, TEMPLATETYPE);              \
        template <class TEMPLATETYPE> OptionList &                         \
        CLASSNAME<TEMPLATETYPE>::getOptionList() const                     \
          { static OptionList ol;                                          \
             if (ol.empty())                                               \
               declareOptions(ol);                                         \
             return ol; }

#define DECLARE_ABSTRACT_NAME_AND_DEEPCOPY(CLASSNAME)                      \
        DECLARE_ABSTRACT_NAME(CLASSNAME);                                  \
        DECLARE_ABSTRACT_DEEPCOPY(CLASSNAME)
#define IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY(CLASSNAME)                    \
        IMPLEMENT_ABSTRACT_NAME(CLASSNAME);                                \
        IMPLEMENT_ABSTRACT_DEEPCOPY(CLASSNAME)

#define DECLARE_ABSTRACT_TEMPLATE_NAME_AND_DEEPCOPY(CLASSNAME, TEMPLATETYPE)   \
        DECLARE_ABSTRACT_NAME_AND_DEEPCOPY(CLASSNAME<TEMPLATETYPE>)
#define IMPLEMENT_ABSTRACT_TEMPLATE_NAME_AND_DEEPCOPY(CLASSNAME, TEMPLATETYPE) \
        IMPLEMENT_ABSTRACT_TEMPLATE_NAME(CLASSNAME, TEMPLATETYPE);             \
        IMPLEMENT_ABSTRACT_TEMPLATE_DEEPCOPY(CLASSNAME, TEMPLATETYPE)

// Template objects cannot be registered automatically. You have to register
// each instantiation type explicitly with the following macro.
#define IMPLEMENT_INSTANTIATED_TEMPLATE_NAME(CLASSNAME, TEMPLATETYPE) TypeRegistrar \
    CLASSNAME<TEMPLATETYPE>::_register_in_typemap_(#CLASSNAME"<"#TEMPLATETYPE">",   \
        &CLASSNAME::_new_instance_for_typemap_)

#define IMPLEMENT_INSTANTIATED_ABSTRACT_TEMPLATE_NAME_AND_DEEPCOPY(CLASSNAME, TEMPLATETYPE) \
        template <> string                                                 \
        CLASSNAME<TEMPLATETYPE>::classname() const                         \
          { return #CLASSNAME"<"#TEMPLATETYPE">"; };                       \
        template <> CLASSNAME<TEMPLATETYPE> *                              \
 	CLASSNAME<TEMPLATETYPE>::deepCopy(CopiesMap& copies) const         \
	  { PLERROR("deepCopy is not implemented for %s",                  \
              classname().c_str());		                           \
	    return 0; }
      

%> // end of namespace PLearn

#endif //!<  Object_INC
