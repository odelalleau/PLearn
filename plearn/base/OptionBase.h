// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin
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


/*! \file PLearnLibrary/PLearnCore/Option.h */

#ifndef OptionBase_INC
#define OptionBase_INC

#include <plearn/io/pl_io.h>
#include "PP.h"
#include <plearn/io/PStream.h>        //!< For PStream.
#include <plearn/io/PStream_util.h>   //!< For OBflag_t.
#include <vector>

namespace PLearn {
using std::string;

//! Forward declarations.
class Object;
class PLearnDiff;

//! Base class for option definitions
class OptionBase: public PPointable
{
public:
    //#####  Option Flags  ####################################################

    // === IMPORTANT NOTICE ===
    // If you add new option flags to the following list, don't forget to add
    // a human-readable interpretation within the flagStrings() function.
    // ========================

    //! The flags in the following list are bitwise-ORed to obtain the option
    //! flags
    typedef unsigned int flag_t;

    /**
     *  'buildoption': an option typically specified before calling the initial
     *  build (semantically similar to a constructor parameter), for instance
     *  the number of hidden units in a neural net.
     */
    static const flag_t buildoption;       

    /**
     *  'learntoption': an option whose proper value is computed by the class
     *  after construction (not to be set by the user before build) and
     *  potentially complex operations such as learning over a training set.
     *  Example: the (trained) weights of a neural net.
     */
    static const flag_t learntoption;

    /**
     *  'tuningoption': an option typically set after the initial build, to tune
     *  the object
     */
    static const flag_t tuningoption;

    /**
     *  'nosave': when set, this flag requests the option not to be saved in
     *  the object serialisation; mostly used when we must have several options
     *  that map to the same physical data field, for script backward
     *  compatibility.
     */
    static const flag_t nosave;

    /**
     *  When this flag is set, the option does not lead to a parenting
     *  relationship in the "ParentableObject" sense.  In other words, the
     *  object pointed to by this option does not get its parent() backpointer
     *  set to this.
     */
    static const flag_t nonparentable;

    /**
     *  When this flag is set, the option is not traversed by the
     *  ObjectGraphIterator class (and ipso facto by related functions, such as
     *  memfun_broadcast.
     */
    static const flag_t nontraversable;

    /**
     * When this flag is set, the option is transmitted between servers
     * and clients, even if the nosave flag is also set.
     * All options w/o nosave are transmitted by default.
     */
    static const flag_t remotetransmit;

    /**
     * OptionLevel: How advanced (complicated, seldom used) is this option?
     *   Used to hide options from help text as needed, to ease readability
     *   for a basic, normal, advanced or experimental level of use.
     */
    typedef unsigned int OptionLevel;
    static const OptionLevel basic_level; //!< mandatory options
    static const OptionLevel advanced_level; //!< useful but seldom used
    static const OptionLevel expert_level; //!< more than advanced, not experimental
    static const OptionLevel experimental_level; //!< only a few people understand those
    static const OptionLevel deprecated_level; //!< should not be used anymore
    static const OptionLevel default_level; //!< default for options w/o explicit level

    typedef map<string, OptionLevel> StrToLevelMap;
    typedef map<OptionLevel, string> LevelToStrMap;
    typedef map<string, flag_t> StrToFlagMap;
    typedef map<flag_t, string> FlagToStrMap;
protected:
    static OptionLevel current_option_level_;
    static StrToLevelMap str_to_level;
    static LevelToStrMap level_to_str;

    //! to toggle display of options by option flags
    static flag_t current_flags_;
    static StrToFlagMap str_to_flag;
    static FlagToStrMap flag_to_str;

protected:
    string optionname_;  //!< the name of the option
    flag_t flags_; 
    string optiontype_;  //!< the datatype of the option ("int" ...)
    string defaultval_;  //!< string representation of the default value (will be printed by optionHelp())
    string description_; //!< A description of this option
    OptionLevel level_;  //!< Option level (low=basic, high=advanced)

public:

    //! Most of these parameters only serve to provide the user 
    //! with an informative help text. (only optionname and flags are really important)
    OptionBase(const string& optionname, flag_t flags,
               const string& optiontype, const string& defaultval, 
               const string& description, const OptionLevel& level);

    //#####  Stream Read-Write  ###############################################

    //! Read the option into the specified object from the input stream
    virtual void read(Object* o, PStream& in) const = 0;

    //! Write the option from the specified object to the output stream
    virtual void write(const Object* o, PStream& out) const = 0;

    //! Writes the option into a string instead of a stream
    //! (calls write on a string stream); return the string.
    string writeIntoString(const Object* o) const;

    //! Read into index "i" of the object's option; the index
    //! is a string for generality (i.e. applies to both vectors and maps)
    virtual void readIntoIndex(Object* o, PStream& in, const string& index);

    //! Write from index "i" of the object's option; the index
    //! is a string for generality (i.e. applies to both vectors and maps)
    virtual void writeAtIndex(const Object* o, PStream& out,
                              const string& index) const;


    //#####  Access and Indexing  #############################################

    //! Return as an Object* the option within the specified object; it's an
    //! error to call this function if the option does not refer to an Object&,
    //! Object* or PP<Object>.
    virtual Object* getAsObject(Object* o) const = 0;

    //! Return as an Object* the option within the specified object; it's an
    //! error to call this function if the option does not refer to an Object&,
    //! Object* or PP<Object>.
    virtual const Object* getAsObject(const Object* o) const = 0;

    //! Return as an Object* the i-th item option, which must be either a TVec
    //! or an Array.  It's an error to call this function if the indexed item
    //! does not refer to an Object* or PP<Object>.
    virtual Object *getIndexedObject(Object *o, int i) const = 0;

    //! Return as an Object* the i-th item option, which must be either a TVec
    //! or an Array.  It's an error to call this function if the indexed item
    //! does not refer to an Object* or PP<Object>.
    virtual const Object *getIndexedObject(const Object *o, int i) const = 0;

    //! Return the option contained in the specified object as a raw void*.
    //! Can be used with the optiontype() accessor to cast to an appropriate
    //! type.  Should be used sparingly and only when absolutely necessary.
    virtual void* getAsVoidPtr(Object* o) const = 0;

    //! Return the option contained in the specified object as a raw void*.
    //! Can be used with the optiontype() accessor to cast to an appropriate
    //! type.  Should be used sparingly and only when absolutely necessary.
    virtual const void* getAsVoidPtr(const Object* o) const = 0;


    //#####  Option Information  ##############################################

    //! Should this option be skipped upon reading it?
    //! (i.e. don't read it in any variable)
    virtual bool shouldBeSkipped() const; 

    //! Returns the name of the class in to which this option belongs
    virtual string optionHolderClassName(const Object* o) const = 0;

    //! The name of the option (field)
    inline const string& optionname() const { return optionname_; }

    //! Test the option name.
    //! One cannot use a 'const string&' because it is used by STL.
    inline bool isOptionNamed(string name) const { return name == optionname(); }

    //! Set option name.
    inline void setOptionName(const string& name) { optionname_ = name; }

    //! Option type accessor
    inline const string& optiontype() const { return optiontype_; }

    //! Default value accessor
    inline const string& defaultval() const { return defaultval_; }

    //! Option level accessor
    inline const OptionLevel& level() const { return level_; }
    //! Option level as a string
    inline string levelString() const { return optionLevelToString(level()); }

    //! Change the string representation of the default value.
    inline void setDefaultVal(const string& newdefaultval)
    { defaultval_ = newdefaultval; }

    //! Description accessor
    inline const string& description() const { return description_; }

    //! Flags accessor
    inline flag_t flags() const { return flags_; }

    //! Human-readable accessor of flags
    vector<string> flagStrings() const;
    
    //! Return true if the option can be obtained as an \c Object*
    //! (i.e. \c getAsObject() succeeds) or can be indexed as an \c Object*
    //! (i.e. \c getIndexedObject() succeeds)
    virtual bool isAccessibleAsObject() const = 0;

    /**
     * Return the size of the indexable option, IF it is indexable.
     *
     * If not indexable, return 0.  If indexable, this returns one more
     * than the maximum permissible index in \c getIndexedObject() or
     * \c readIntoIndex() or \c writeAtIndex().
     *
     * @param  o  The object containing the object
     * @return    The number of indexable objects within the option
     */
    virtual int indexableSize(const Object* o) const = 0;
    

    //#####  Miscellaneous  ###################################################

    //! Comparison between two option values.
    virtual int diff(const string& refer, const string& other, PLearnDiff* diffs) const = 0;

    //! get the current option level for help text
    static const OptionLevel& getCurrentOptionLevel() { return current_option_level_; }
    //! set the current option level for help text
    static void setCurrentOptionLevel(const OptionLevel& l) { current_option_level_= l; }
    //! Get map (name->level) of all option levels
    static const StrToLevelMap& getStrToLevelMap();
    //! Get map (level->) of all option levels
    static const LevelToStrMap& getLevelToStrMap();
    //! convert string to OptionLevel
    static OptionLevel optionLevelFromString(const string& s);
    //! convert OptionLevel to string
    static string optionLevelToString(const OptionLevel& l);

    //! get the current option level for help text
    static const flag_t& getCurrentFlags() { return current_flags_; }
    //! set the current option level for help text
    static void setCurrentFlags(const flag_t& f) { current_flags_= f; }
    //! Get map (name->flag) of all possible flags
    static const StrToFlagMap& getStrToFlagMap();
    //! Get map (flag->name) of all possible flags
    static const FlagToStrMap& getFlagToStrMap();

};

typedef std::vector< PP<OptionBase> > OptionList;



} // end of namespace PLearn

#endif //!<  Option_INC


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
