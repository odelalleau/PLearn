// -*- C++ -*-

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
 * AUTHORS: Pascal Vincent & Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */

#include "Object.h"
#include "stringutils.h"    //!< For removeblanks.
#include <plearn/io/fileutils.h>
#include <plearn/io/pl_log.h>
#include <plearn/io/load_and_save.h>
#include <plearn/io/openFile.h>
#include <plearn/io/openString.h>
#include "TypeFactory.h"
#include "RemoteDeclareMethod.h"
#include <algorithm>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    Object,
    "Base class for all high-level PLearn objects.",
    "Object exposes simple mechanisms for:\n"
    "\n"
    "- automatic memory management (through reference counting and smart pointers)\n"
    "- serialization (read, write, save, load)\n"
    "- runtime type information (classname)\n"
    "- displaying (info, print)\n"
    "- deep copying (deepCopy)\n"
    "- remote method calling mechanism (call(), declareMethods())\n"
    "- a generic way of setting options (setOption) when not knowing the\n"
    "  exact type of the Object and a generic build() method (the combination\n"
    "  of the two allows to change the object structure and rebuild it at\n"
    "  runtime)\n"
    "\n"
    "\n"
    "NOTES ON THE PLEARN OBJECT MODEL.  All \"significant\" PLearn classes inherit\n"
    "from this class, PLearn::Object, as their ultimate base.  Each PLearn class\n"
    "defines a number of \"options\", which are data fields that are reflected in\n"
    "the serialized representation of the object.  Hence, the set of options\n"
    "describes the capabilities of an object from the viewpoint of the script\n"
    "writer.  The meaning of each option is documented within each class.\n"
    "\n"
    "Moreover, all options are given a set of flags.  The meaning of each flag\n"
    "is as follows:\n"
    "\n"
    "- buildoption : an option typically specified before calling the\n"
    "  initial build (semantically similar to a constructor parameter), for\n"
    "  instance the number of hidden units in a neural net.\n"
    "\n"
    "- learntoption : n option whose proper value is computed by the class\n"
    "  after construction (not to be set by the user before build) and potentially\n"
    "  complex operations such as learning over a training set.  Example: the\n"
    "  (trained) weights of a neural net.\n"
    "\n"
    "- tuningoption : an option typically set after the initial build, to tune\n"
    "  the object\n"
    "\n"
    "- nosave : when set, this flag requests the option not to be saved in\n"
    "  the object serialisation; mostly used when we must have several options\n"
    "  that map to the same physical data field, for script backward\n"
    "  compatibility.\n"
    "\n"
    "- nonparentable : when this flag is set, the option does not lead to a\n"
    "  parenting relationship in the \"ParentableObject\" sense.  In other words,\n"
    "  the object pointed to by this option does not get its parent() backpointer\n"
    "  set to this. (See the ParentableObject class for details).\n"
    "\n"
    "- nontraversable : when this flag is set, the option is not traversed\n"
    "  by the ObjectGraphIterator class (and ipso facto by related functions, such\n"
    "  as memfun_broadcast. (See the ObjectGraphIterator class for details).\n"
    );


//#####  Basic PLearn::Object Protocol  #######################################

Object::Object(bool call_build_)
{
    if (call_build_)
        build_();
}

Object::~Object()
{ }

// by default, do nothing...
void Object::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{ }

void Object::build_()
{ }

void Object::build()
{ }

string Object::info() const
{
    return classname();
}

string Object::asString() const
{
    string s;
    PStream out= openString(s, PStream::plearn_ascii, "w");
    out << this;
    out.flush();
    return removeblanks(s);
}

//#####  Option-Manipulation Functions  #######################################

void Object::setOption(const string& optionname, const string& value)
{
    PStream in = openString(value, PStream::plearn_ascii);
    readOptionVal(in, optionname);
}

#ifdef PL_PYTHON_VERSION 
void Object::setOptionFromPython(const string& optionname, const PythonObjectWrapper& value)
{
    OptionMap& om= getOptionMap();
    OptionMap::iterator it= om.find(optionname);
    if(it == om.end())
        PLERROR("%s has no option %s.", classname().c_str(), optionname.c_str());
    it->second->setFromPythonObject(this, value);
}
#endif //def PL_PYTHON_VERSION 

bool Object::hasOption(const string &optionname) const
{ 
    OptionMap& om= getOptionMap();
    OptionMap::iterator it= om.find(optionname);
    return it != om.end();
}

string Object::getOption(const string &optionname) const
{ 
    string s;
    PStream out = openString(s, PStream::plearn_ascii, "w");
    writeOptionVal(out, optionname);
    out.flush();
    return removeblanks(s);
}

void Object::changeOptions(const map<string,string>& name_value)
{
    map<string,string>::const_iterator it = name_value.begin();
    map<string,string>::const_iterator itend = name_value.end();
    while(it!=itend)
    {
        setOption(it->first, it->second);
        ++it;
    }
}

void Object::changeOption(const string& optionname, const string& value)
{
    map<string,string> name_value;
    name_value[optionname] = value;
    changeOptions(name_value);
}


//#####  Object::readOptionVal  ###############################################

void Object::readOptionVal(PStream &in, const string &optionname)
{
    try 
    {

        OptionMap& om= getOptionMap();
        OptionMap::iterator it= om.find(optionname);
        if(it != om.end())
        {
            it->second->read(this, in);
            return;
        }

        // Found no options matching 'optionname'. First look for brackets. If there
        // are brackets, they must be located before any dot.
        size_t lb_pos = optionname.find('[');
        size_t rb_pos = optionname.find(']');
        size_t dot_pos = optionname.find('.');
        if (rb_pos != string::npos) {
            if (lb_pos == string::npos)
                PLERROR("Object::readOptionVal() - Unmatched brackets");
            string optname = optionname.substr(0, lb_pos);

            // Found no dot, or a right bracket before a dot
            if (dot_pos == string::npos || rb_pos < dot_pos) {
                string index = optionname.substr(lb_pos + 1, rb_pos - lb_pos - 1);

                it= om.find(optname);
                if(it != om.end())
                {
                    // There are two cases here: either there is a dot located
                    // immediately after the right bracket, or there is no dot.
                    // If there is a dot, the option HAS to be an Object
                    if (dot_pos != string::npos && dot_pos == rb_pos+1) {
                        int i = toint(index);
                        it->second->getIndexedObject(this, i)->readOptionVal(
                            in, optionname.substr(dot_pos + 1));
                    }
                    else if (dot_pos == string::npos)
                        it->second->readIntoIndex(this, in, index);
                    else
                        PLERROR("Object::readOptionVal() - unknown option format \"%s\"",
                                optionname.c_str());
                    return;
                }
            }
        }
        else if (lb_pos != string::npos)
            PLERROR("Object::readOptionVal() - Unmatched brackets");

        // No brackets, look for a dot
        if (dot_pos != string::npos) 
        {
            // Found a dot, assume it's a field with an Object * field
            string optname = optionname.substr(0, dot_pos);
            string optoptname = optionname.substr(dot_pos + 1);
            it= om.find(optname);
            if(it != om.end())
            {
                it->second->getAsObject(this)->readOptionVal(in, optoptname);
                return;
            }
        }
    }
    catch(const PLearnError& e)
    { 
        PLERROR("Problem while attempting to read value of option \"%s\" of a \"%s\":\n %s", 
                optionname.c_str(), classname().c_str(), e.message().c_str()); 
    }

    // There are bigger problems in the world but still it isn't always funny
    PLERROR("There is no option named \"%s\" in a \"%s\"",
            optionname.c_str(),classname().c_str());
}


//#####  Object::writeOptionVal  ##############################################

void Object::writeOptionVal(PStream &out, const string &optionname) const
{
    OptionMap& om= getOptionMap();
    OptionMap::iterator it= om.find(optionname);
    if(it != om.end())
    {
        it->second->write(this, out);
        return;
    }
    
    // Found no options matching 'optionname'. First look for brackets. If there
    // are brackets, they must be located before any dot.
    size_t lb_pos = optionname.find('[');
    size_t rb_pos = optionname.find(']');
    size_t dot_pos = optionname.find('.');
    if (rb_pos != string::npos) {
        if (lb_pos == string::npos)
            PLERROR("Object::writeOptionVal() - Unmatched brackets");
        string optname = optionname.substr(0, lb_pos);

        // Found no dot, or a right bracket before a dot
        if (dot_pos == string::npos || rb_pos < dot_pos) {
            string index = optionname.substr(lb_pos + 1, rb_pos - lb_pos - 1);
            it= om.find(optname);
            if(it != om.end())
            {
                // There are two cases here: either there is a dot located
                // immediately after the right bracket, or there is no dot.  If
                // there is a dot, the option HAS to be an Object
                if (dot_pos != string::npos && dot_pos == rb_pos+1) {
                    int i = toint(index);
                    it->second->getIndexedObject(this, i)->writeOptionVal(
                        out, optionname.substr(dot_pos + 1));
                }
                else if (dot_pos == string::npos)
                    it->second->writeAtIndex(this, out, index);
                else
                    PLERROR("Object::writeOptionVal() - unknown option format \"%s\"",
                            optionname.c_str());
                return;
            }
        }
    }
    else if (lb_pos != string::npos)
        PLERROR("Object::writeOptionVal() - Unmatched brackets");

    // No brackets, look for a dot
    if (dot_pos != string::npos) {
        // Found a dot, assume it's a field with an Object * field
        string optname = optionname.substr(0, dot_pos);
        string optoptname = optionname.substr(dot_pos + 1);
        it= om.find(optname);
        if(it != om.end())
        {
            it->second->getAsObject(this)->writeOptionVal(out, optoptname);
            return;
        }
    }
    // There are bigger problems in the world but still it isn't always funny
    PLERROR("Object::writeOptionVal() - Unknown option \"%s\"", optionname.c_str());    
}


//#####  Object::parseOptionName  #############################################

bool Object::parseOptionName(const string& optionname,
                             Object*& final_object,
                             OptionList::iterator& option_iter,
                             string& option_index)
{
    OptionList &options = getOptionList();
    for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
        if ((*it)->optionname() == optionname) {
            final_object = this;
            option_iter  = it;
            option_index = "";
            return true;
        }

    // Found no options matching 'optionname'. First look for brackets. If there
    // are brackets, they must be located before any dot.
    size_t lb_pos = optionname.find('[');
    size_t rb_pos = optionname.find(']');
    size_t dot_pos = optionname.find('.');
    if (rb_pos != string::npos) {
        if (lb_pos == string::npos)
            PLERROR("Object::parseOptionName() - Unmatched brackets when parsing option \"%s\"",
                    optionname.c_str());
        string optname = optionname.substr(0, lb_pos);

        // Found no dot, or a right bracket before a dot
        if (dot_pos == string::npos || rb_pos < dot_pos) {
            string index = optionname.substr(lb_pos + 1, rb_pos - lb_pos - 1);
            for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
                if ((*it)->optionname() == optname) {
                    // There are two cases here: either there is a dot located
                    // immediately after the right bracket, or there is no dot.  If
                    // there is a dot, the option HAS to be an Object
                    if (dot_pos != string::npos && dot_pos == rb_pos+1) {
                        int i = toint(index);
                        return (*it)->getIndexedObject(this, i)->
                            parseOptionName(optionname.substr(dot_pos + 1),
                                            final_object, option_iter, option_index);
                    }
                    else if (dot_pos == string::npos) {
                        final_object = this;
                        option_iter  = it;
                        option_index = index;
                        return true;
                    }
                    else {
                        PLERROR("Object::writeOptionVal() - unknown option format \"%s\"",
                                optionname.c_str());
                        return false;        //!< Shut up compiler
                    }
                }
        }
    }
    else if (lb_pos != string::npos)
        PLERROR("Object::writeOptionVal() - Unmatched brackets when parsing option \"%s\"",
                optionname.c_str());

    // No brackets, look for a dot
    if (dot_pos != string::npos) {
        // Found a dot, assume it's a field with an Object * field
        string optname = optionname.substr(0, dot_pos);
        string optoptname = optionname.substr(dot_pos + 1);
        for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
            if ((*it)->optionname() == optname) {
                return (*it)->getAsObject(this)->
                    parseOptionName(optoptname, final_object, option_iter, option_index);
            }
    }

    // Set reasonable defaults for an error condition
    final_object = 0;
    option_iter  = OptionList::iterator();
    option_index = "";
    return false;
}


bool Object::parseOptionName(const string& optionname,
                             const Object*& final_object,
                             OptionList::iterator& option_iter,
                             string& option_index) const
{
    return const_cast<Object*>(this)->parseOptionName(
        optionname, const_cast<Object*&>(final_object),
        option_iter, option_index);
}
    

//#####  Object::getOptionsToSave  ############################################

string Object::getOptionsToSave() const
{
    string res = "";
    OptionList& options = getOptionList();
  
    for( OptionList::iterator it = options.begin(); it!=options.end(); ++it )
    {
        OptionBase::flag_t flags = (*it)->flags();
        if(!(flags & OptionBase::nosave))
            res += (*it)->optionname() + " ";
    }
    return res;
}


//#####  Object::getOptionsToRemoteTransmit  ############################################
string Object::getOptionsToRemoteTransmit() const
{
    string res = "";
    OptionList& options = getOptionList();
  
    for( OptionList::iterator it = options.begin(); it!=options.end(); ++it )
    {
        OptionBase::flag_t flags = (*it)->flags();
        if(!(flags & OptionBase::nosave) || flags & OptionBase::remotetransmit)
            res += (*it)->optionname() + " ";
    }
    return res;
}

//#####  Object::newread  #####################################################

void Object::newread(PStream &in)
{
    PP<Object> dummy_obj = 0; // Used to read skipped options.
    string cl;

    // Allow the use of the pointer syntax for non-pointer instances.
    in.skipBlanksAndComments();
    if ( in.peek() == '*' )  
    {
        const char* errmsg = "In Object::newread(PStream&) Wrong format. "
            "Expecting \"*%d->\" but got \"*%d%c%c\".";
    
        in.get(); // Eat '*'
        unsigned int id;
        in >> id;
        in.skipBlanksAndComments();

        char dash = in.get(); // Eat '-'
        if ( dash != '-' )
        {
            if ( dash == ';' )
                PLERROR("In Object::newread(PStream&): Non pointer objects can be prefixed with dummy "
                        "references ('*%d ->') but these references MUST NOT BE USED afterwards. Just "
                        "read '*%d;'", id, id );
            PLERROR( errmsg, id, id, dash, in.get() );
        }
    
        char cc = in.get();
        if(cc != '>') // Eat '>'
            PLERROR( errmsg, id, id, dash, cc);
        in.skipBlanksAndCommentsAndSeparators();    
    }
    
    in.getline(cl, '(');
    cl = removeblanks(cl);
    if (cl != classname())
        PLERROR("Object::newread() - Was expecting \"%s\", but read \"%s\"",
                classname().c_str(), cl.c_str());

    in.skipBlanksAndComments();
    int c = in.get();
    if (c != ')') 
    {
        in.putback(c);
        for (;;) 
        {
            // Read all specified options
            string optionname;
            in.getline(optionname, '=');
            optionname = removeblanks(optionname);
            in.skipBlanksAndComments();
            OptionMap& om= getOptionMap();
            OptionMap::iterator it= om.find(optionname);
            if(it != om.end() && it->second->shouldBeSkipped())
            {
                // Create a dummy object that will read this option.
                if (!dummy_obj) {
                    // Note that we do not call build on 'dummy_obj'. This is
                    // because some classes may crash when build is called
                    // before setting options (though this is not a desired
                    // behaviour, it can be hard to figure out what is going on
                    // when it crashes here).
                    dummy_obj =
                        TypeFactory::instance().newObject(this->classname());
                }
                dummy_obj->readOptionVal(in, optionname);
            }
            else
            {
                // cerr << "Reading option: " << optionname << endl;
                readOptionVal(in, optionname);
                // cerr << "returned from reading optiion " << optionname << endl;
            }
            in.skipBlanksAndCommentsAndSeparators();

            if (in.peek() == ')') 
            {
                in.get();
                break;
            }
        }
    }
    build(); // Build newly read Object
}


//#####  Object::newwrite  ####################################################

void Object::newwrite(PStream& out) const
{
    vector<string> optnames= split(
        out.remote_plearn_comm?
        getOptionsToRemoteTransmit():
        getOptionsToSave());

    out.write(classname());
    out.write("(\n");
    for (size_t i = 0; i < optnames.size(); ++i) 
    {
        out.write(optnames[i]);
        out.write(" = ");
        writeOptionVal(out, optnames[i]);
        if (i < optnames.size() - 1)
            out.write(";\n");
    }
    out.write(" )\n");
}


void Object::write(ostream& out_) const
{
    PStream out(&out_);
    newwrite(out);
}

void Object::read(istream& in_)
{ 
    in_ >> ws; // skip blanks
    int c = in_.peek();
    if(c=='<') // --> it's an "old-style" <Classname> ... </Classname> kind of definition
        oldread(in_);
    else { // assume it's a "new-style" Classname(...) 
        PStream in(&in_);
        newread(in);
    }
}


//#####  Remote Method Invocation  ############################################

void Object::prepareToSendResults(PStream& out, int nres)
{ 
    DBG_LOG << "PREPARING TO SEND " << nres << " RESULTS." << endl;
    out.write("!R "); out << nres; 
}


void Object::call(const string& methodname, int nargs, PStream& io)
{
    // Look up methodname in the RemoteMethodMap
    RemoteMethodMap& rmm = getRemoteMethodMap();
    if (const RemoteTrampoline* trampoline = rmm.lookup(methodname,nargs))
        trampoline->call(this, nargs, io);
    else
        PLERROR("No method named '%s' taking %d arguments supported by objects of class '%s'.",
                methodname.c_str(), nargs, classname().c_str());
}


// We use an anonymous namespace to ensure the following classes are local to
// this specific translation unit.
// Note that these classes have been moved out of the declareMethods Object
// method as otherwise compilation will fail with Microsoft Visual C++.
namespace {

	// The following are custom trampolines for the very special case where
	// no C++ function can be mapped to these remote functions, since their
	// return type is polymorphic.
	struct ObjectTrampolineGetOption : public RemoteTrampoline
	{
		ObjectTrampolineGetOption(const string& methodname, const RemoteMethodDoc& doc)
			: RemoteTrampoline(methodname, doc)
		{ }

		virtual void call(Object* instance, int nargs, PStream& io) const
		{
			checkNargs(nargs, 1);
			string optionname;
			io >> optionname;
			prepareToSendResults(io, 1);
			instance->writeOptionVal(io, optionname);
			io.flush();
		}

#ifdef PL_PYTHON_VERSION 
                virtual PythonObjectWrapper call(Object* instance, 
                                                 const TVec<PythonObjectWrapper>& args) const
                {
                    checkNargs(args.size(), 1);
                    string optionname= args[0];
                    OptionMap& om= instance->getOptionMap();
                    OptionMap::iterator it= om.find(optionname);
                    if(it != om.end())
                        return it->second->getAsPythonObject(instance);
                    PLERROR("in ObjectTrampolineGetOption::call (python ver.) : "
                            "unknown option: '%s'", optionname.c_str());
                    return PythonObjectWrapper();//gcc: shut up!
                }
#endif //def PL_PYTHON_VERSION 
	};

	struct ObjectTrampolineGetObject : public RemoteTrampoline
	{
		ObjectTrampolineGetObject(const string& methodname, const RemoteMethodDoc& doc)
			: RemoteTrampoline(methodname, doc)
		{ }

		virtual void call(Object* instance, int nargs, PStream& io) const
		{
			checkNargs(nargs, 0);
			prepareToSendResults(io, 1);
			io << *instance;
			io.flush();
		}

#ifdef PL_PYTHON_VERSION 
                virtual PythonObjectWrapper call(Object* instance, 
                                                 const TVec<PythonObjectWrapper>& args) const
                {
			checkNargs(args.size(), 0);
                        return PythonObjectWrapper(instance);
                }
#endif //def PL_PYTHON_VERSION 

	};

} // End of anonymous namespace.

void Object::declareMethods(RemoteMethodMap& rmm)
{
    declareMethod(rmm, "hasOption", &Object::hasOption,
                  (BodyDoc("Checks whether the object has an option with the given name"),
                   ArgDoc ("optionname","The name of the option looked for"),
                   RetDoc ("A bool that is true if the option was found.")));

    declareMethod(rmm, "changeOptions", &Object::changeOptions,
                  (BodyDoc("Change a set of options within the object"),
                   ArgDoc ("option_map",
                           "Set of option-name:option-value pairs to "
                           "modify within the object.")));

    declareMethod(rmm, "getOptionAsString", &Object::getOption,
                  (BodyDoc("Return a given object option in PLearn serialized representation."),
                   ArgDoc ("option_name", "Name of the option to get"),
                   RetDoc ("Object option in string form, or exception if "
                           "the option does not exist.")));

    declareMethod(rmm, "asString", &Object::asString,
                  (BodyDoc("Returns a string representation of this object."),
                   RetDoc ("string representation of the object")));

    declareMethod(rmm, "run", &Object::run,
                  (BodyDoc("Run the given object, if it is runnable; "
                           "raise an exception if it is not.")));

    declareMethod(rmm, "save", &Object::remote_save,
                  (BodyDoc("Save the given object to disk under the designated filename\n"),
                   ArgDoc ("filepath", "Pathname where the object should be saved"),
                   ArgDoc ("io_formatting",
                           "Format in which the object should be saved.  Can be one of:\n"
                           "- \"plearn_ascii\": use the PLearn ASCII (text) serialisation format\n"
                           "- \"plearn_binary\": use the PLearn binary format, which can be\n"
                           "  more efficient for large objects\n")));

    declareMethod(rmm, "build", &Object::build,
                  (BodyDoc("Build newly created object (after setting options).\n")));

    declareMethod(rmm, "classname", &Object::classname,
                  (BodyDoc("Returns the name of the class"),
                   RetDoc ("Class name as string")));

    declareMethod(rmm, "usage", &Object::usage,
                  (BodyDoc("Returns the refcount of this PPointable"),
                   RetDoc ("Number of references")));

    declareMethod(rmm, "deepCopyNoMap", &Object::deepCopyNoMap,
                  (BodyDoc("Returns a deep copy of the object"),
                   RetDoc ("Deep copy.")));

#ifdef PL_PYTHON_VERSION 
    declareMethod(rmm, "pyDeepCopy", &Object::pyDeepCopy,
                  (BodyDoc("Returns a pair containing a deep copy of "
                           "the object and the updated copies map."),
                   ArgDoc ("copies", "The initial copies map"),
                   RetDoc ("Deep copy, copies map")));
#endif //def PL_PYTHON_VERSION 

#ifdef PL_PYTHON_VERSION 
    declareMethod(rmm, "setOptionFromPython", &Object::setOptionFromPython,
                  (BodyDoc("Change an option within the object from a PythonObjectWrapper"),
                   ArgDoc("optionname", "The name of the option to change."),
                   ArgDoc("value","the new value from python")));
#endif //def PL_PYTHON_VERSION 

    rmm.insert(
        "getOption", 1,
        new ObjectTrampolineGetOption(
            "getOption",
            (BodyDoc("Return the option value given its name.  Note that the returned value\n"
                     "is POLYMORPHIC: its type varies according to the actual type of the\n"
                     "option."),
             ArgDoc ("optionname", "Name of the option to get"),
             RetDoc ("Value contained in the option"),
             ArgTypeDoc("string"),
             RetTypeDoc("(polymorphic)"))));

    rmm.insert(
        "getObject", 0,
        new ObjectTrampolineGetObject(
            "getObject",
            (BodyDoc("Return the object itself by value.  Note that the returned value\n"
                     "is POLYMORPHIC: its type varies according to the actual type of the\n"
                     "object."),
             RetDoc ("Object value"),
             RetTypeDoc("(polymorphic)"))));
}


//#####  Serialization and Miscellaneous  #####################################

void Object::run()
{
    PLERROR("%s::run() : Not a runnable Object", this->classname().c_str());
}

void Object::oldread(istream& in)
{
    PLERROR("oldread method not implemented for this object");
}

void Object::save(const PPath& filename) const
{
    PLWARNING("This method is deprecated. It simply calls the generic PLearn save function "
              "(that can save any PLearn object): PLearn::save(filename, *this) "
              "So you should call PLearn::save directly (it's defined in plearn/io/load_and_save.h).");
    PLearn::save(filename, *this);
}

void Object::load(const PPath& filename)
{
    PLWARNING("This method is deprecated. It simply calls the generic PLearn load function "
              "(that can load any PLearn object): PLearn::load(filename, *this) "
              "So you should call PLearn::load directly (it's defined in plearn/io/load_and_save.h).");
    PLearn::load(filename, *this);
}

Object* Object::deepCopyNoMap()
{
    CopiesMap cm;
    return deepCopy(cm);
}

#ifdef PL_PYTHON_VERSION 
tuple<Object*, CopiesMap> Object::pyDeepCopy(CopiesMap& copies)
{
    Object* o= deepCopy(copies);
    return make_tuple(o, copies);
}
#endif //def PL_PYTHON_VERSION 


Object* loadObject(const PPath &filename)
{
    PStream in = openFile(filename, PStream::plearn_ascii, "r");
    Object *o = readObject(in);
    o->build();
    return o;
}

Object* macroLoadObject(const PPath &filename, map<string, string>& vars)
{
    string script = readFileAndMacroProcess(filename, vars);
    PStream sin = openString(script,PStream::plearn_ascii);
    Object* o = readObject(sin);
    o->build();
    return o;
}
  
Object* macroLoadObject(const PPath &filename)
{
    map<string, string> vars;
    return macroLoadObject(filename,vars);
}

Object* readObject(PStream &in, unsigned int id)
{
    Object *o=0;
    in.skipBlanksAndCommentsAndSeparators();

    string head;

    int c = in.peek();
    if (c == '<')  // Old (deprecated) serialization mode 
        PLERROR("Old deprecated serialization mode starting with '<' no longer supported.");
    else if (c == '*') // Pointer to object
    {
        in >> o;
    } 
    else if(c == '`') // back-quote: reference to an object in another file
    {
        in.get(); // skip the opening back-quote
        string fname;
        in.getline(fname,'`');
        fname = removeblanks(fname);
        // TODO: Check if this is really what we want
        //       (ie: We could want to use the options
        //            of 'in' to load the object...)
        o = loadObject(fname);
    }
    else // It must be a Classname(...) kind of definition 
    {
        string cl;
        in.getline(cl, '(');
        cl = removeblanks(cl);
        // It's a Classname(opt1 = ...; ...; optn = ...); --> calls newread()
        o = TypeFactory::instance().newObject(cl);
        if (!o)
            PLERROR("readObject() - Type \"%s\" not declared in TypeFactory map (did you do a proper DECLARE_NAME_AND_DEEPCOPY?)", cl.c_str());
        in.unread(cl+'(');

        // Finally read the guts of the object
        o->newread(in);
    }

#if 0
    // Code that could be used... but need to see if it's useful and not too ugly.
    // See if we actually want an option of this object, instead of the object itself.
    in.skipBlanksAndCommentsAndSeparators();
    while (in.peek() == '.') {
        in.get(); // Skip the dot.
        char ch;
        string option_name;
        while (((ch = in.peek()) >= 'A' && ch <= 'z') || ch == '_' || (ch >= '0' && ch <= '9')) {
            in.get();
            option_name += ch;
        }
        if (option_name == "")
            PLERROR("In readObject - Could not read correctly the option name following a dot");
        o = newObject(o->getOption(option_name));
    }
#endif
       
    if (id != UINT_MAX)
        in.copies_map_in[id] = o;
    return o;
}


PStream& operator>>(PStream& in, Object*& x)
{
    in.skipBlanksAndCommentsAndSeparators();
    if (in.peek() == '*')
    {
        in.get(); // Eat '*'
        unsigned int id;
        in >> id;
        //don't skip blanks before we need to read something else (read might block).
        //in.skipBlanksAndCommentsAndSeparators();
        if (id==0)
            x = 0;
        else
        {
            in.skipBlanksAndCommentsAndSeparators(); 
            if (in.peek() == '-') 
            {
                in.get(); // Eat '-'
                char cc = in.get();
                if(cc != '>') // Eat '>'
                    PLERROR("In PStream::operator>>(Object*&)  Wrong format. "
                            "Expecting \"*%d->\" but got \"*%d-%c\".", id, id, cc);
                in.skipBlanksAndCommentsAndSeparators();
                if(x)
                    in >> *x;
                else // x is null
                    x = readObject(in, id);
                //don't skip blanks before we need to read something else (read might block).
                // in.skipBlanksAndCommentsAndSeparators();
                in.copies_map_in[id]= x;
            } 
            else 
            {
                // Find it in map and return ptr;
                map<unsigned int, void *>::iterator it = in.copies_map_in.find(id);
                if (it == in.copies_map_in.end())
                    PLERROR("In PStream::operator>>(Object*&) object (ptr) to be read with id='%d' "
                            "has not been previously defined", id);
                x= static_cast<Object *>(it->second);
            }
        }
    }
    else
    {
        x = readObject(in);
        //don't skip blanks before we need to read something else (read might block).
        // in.skipBlanksAndCommentsAndSeparators();
    }

    return in;
}

void Object::remote_save(const string& filepath, const string& io_formatting) const
{
    if(io_formatting=="plearn_ascii")
        PLearn::save(filepath, *this, PStream::plearn_ascii);
    else if(io_formatting=="plearn_binary")
        PLearn::save(filepath, *this, PStream::plearn_binary);
    else
        PLERROR("In Object remote method save: invalid io_formatting %s",
                io_formatting.c_str());
}

void callFunction(const string& funcname, int nargs, PStream& io)
{
    // Look up methodname in the RemoteMethodMap
    RemoteMethodMap& rmm = getGlobalFunctionMap();
    if (const RemoteTrampoline* trampoline = rmm.lookup(funcname,nargs))
        trampoline->call(0, nargs, io);
    else
        PLERROR("No function has been registered with name '%s' and %d arguments",
                funcname.c_str(), nargs);
}


/*
  PStream& operator>>(PStream &in, Object * &o)
  {
  if (in.peek() == '*') {
  in.get(); // Eat '*'
  unsigned int id;
  in >> raw >> id;
  if (in.peek() == '-') {
  in.get(); // Eat '-'
  in.get(); // Eat '>'
            // Read object
            // NOTE: Object is added immediately to the map before any build() is called on it.
            o = readObject(in, id);
            } else {
            // Find it in map and return ptr
            map<unsigned int, void *>::iterator it = in.copies_map_in.find(id);
            if (it == in.copies_map_in.end())
            PLERROR("read() - Object to be read has not been previously defined");
            o = static_cast<Object *>(it->second);
            }
            } else {
            o = readObject(in);
            }
            return in;
            }
*/



Object* newObjectFromClassname(const string& classname)
{
    return TypeFactory::instance().newObject(classname);
}

Object* remote_deepCopy(Object* source)
{
    CopiesMap copies;
    return source->deepCopy(copies);
}

BEGIN_DECLARE_REMOTE_FUNCTIONS

    declareFunction("newObject", &newObject,
                    (BodyDoc("Returns PLearn object from a string description.\n"),
                     ArgDoc("representation", 
                            "the string representation of the object"),
                     RetDoc ("newly created object")));

    declareFunction("newObjectFromClassname", &newObjectFromClassname,
                    (BodyDoc("Returns PLearn object from a class name (string.)\n"),
                     ArgDoc("classname", 
                            "the class of the object, as a string"),
                     RetDoc ("newly created object")));

    declareFunction("loadObject", &loadObject,
                    (BodyDoc("Returns PLearn object from a file describing it.\n"),
                     ArgDoc("filename", 
                            "file containing the object to load"),
                     RetDoc ("newly created object")));

    declareFunction("macroLoadObject", static_cast<Object* (*)(const PPath&,map<string,string>&)>(&macroLoadObject),
                    (BodyDoc("Returns PLearn object from a file describing it,"
                             " after macro-processing.\n"),
                     ArgDoc("filename", 
                            "file containing the object to load"),
                     ArgDoc("vars", 
                            "map of vars to values."),
                     RetDoc ("newly created object")));

    declareFunction("deepCopy", &remote_deepCopy,
                    (BodyDoc("Returns deep copy of a PLearn object.\n"),
                     ArgDoc ("source", "object to be deep-copied"),
                     RetDoc ("deep copy of the object")));


END_DECLARE_REMOTE_FUNCTIONS


} // end of namespace PLearn


//! Useful function for debugging inside gdb:
extern "C"
void printobj(PLearn::Object* p)
{
    PLearn::PStream perr(&std::cerr);
    perr << *p;
    perr.endl();
}


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
