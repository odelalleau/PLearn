// -*- C++ -*-

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
   * $Id: Object.cc,v 1.5 2002/09/26 05:06:48 plearner Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

#include "Object.h"
#include "stringutils.h"
#include "fileutils.h"

namespace PLearn <%
using namespace std;

/*
const flag_t OptionBase::buildoption = 1;       
const flag_t OptionBase::learntoption = 1<<1;
const flag_t OptionBase::tuningoption = 1<<2;
const flag_t OptionBase::nosave = 1<<4; 
*/

Object::Object()
{}

Object* Object::deepCopy(map<const void*, void*>& copies) const
{ 
  PLERROR("deepCopy method not implemented for this object"); 
  return 0;
}

// by default, do nothing...
void Object::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{}

string OptionBase::writeIntoString(const Object* o) const
  {
    ostrstream out_;
    PStream out(&out_);
    write(o, out);
    char* buf = out_.str();
    int n = out_.pcount();
    string s(buf,n);
    out_.freeze(false); // return ownership to the stream, so that it may free it...
    return s;
  }

OptionList& Object::getOptionList() const
{
  static OptionList options;
  PLERROR("You must redefine getOptionList() in all instantiable subclasses");
  return options;
}

/*! Warning: the values printed as "default", if they had not a string
representation specified explicitly when calling declareOption(...), will
be printed as the current instance's value.  So they'll correspond to the
actual "defaults" only when invoked on an instance freshly constructed with
the default constructor...  */
string Object::optionHelp(bool buildoptions_only) const
  {
    string res = "OPTIONS FOR CLASS " + classname() + " (including options inherited from  parent classes): \n";
    // res += "%%% A + in the first column indicates that the option is saved by the serialization methods \n";
    // res += "%%% meaning of category: 'B': initial Build option;  'L': Learnt parameter; 'T': for Tuning with later setOption \n\n";
      
    OptionList& options = getOptionList();    
    for( OptionList::iterator it = options.begin(); it!=options.end(); ++it )
      {
        OptionBase::flag_t flags = (*it)->flags();
        if(flags & OptionBase::buildoption || !buildoptions_only)
          {
            string optname = (*it)->optionname();
            if( flags & OptionBase::nosave )
              res += "- ";
            else
              res += "+ ";
            res += optname + ": " + (*it)->optiontype();
            string defaultval = (*it)->defaultval();
            if(defaultval=="")
              defaultval = (*it)->writeIntoString(this);
            res += " (default: " + defaultval + ") ";
            res += " [" + (*it)->optionHolderClassName(this) + "] \n";
            if(!buildoptions_only)
              {
                res += "[ options: ";
                if(flags & OptionBase::buildoption) res += "buildoption ";
                if(flags & OptionBase::learntoption) res += "learntoption ";
                if(flags & OptionBase::tuningoption) res += "tuningoption ";
                if(flags & OptionBase::nosave) res += "nosave ";
                res += "]\n";
              }
            res += (*it)->description() + "\n";
          }
      }
    return res;
  }

void Object::setOption(const string& optionname, const string& value)
{
    istrstream in_(value.c_str());
    PStream in(&in_);
    in >> option_flags(dft_option_flag | OptionBase::nosave);
    readOptionVal(in, optionname);
}

string Object::help() const
{
  return optionHelp();
}

string Object::getOption(const string &optionname) const
{ 
  ostrstream out_;
  PStream out(&out_);
  out << option_flags(dft_option_flag | OptionBase::nosave);
  writeOptionVal(out, optionname);
  char* buf = out_.str();
  int n = out_.pcount();
  string s(buf,n);
  out_.freeze(false); // return ownership to the stream, so that it may free it...
  return removeblanks(s);
}

void Object::build_()
{}

void Object::build()
{}

string Object::classname() const
{ return "Object"; }
// { return string(typeid(*this).name()); } // it would be nice to use this some day, rather than having to redefine classname() in every class

string Object::info() const { return classname(); }

void Object::print(ostream& out) const
{ out << "{" << info() << "}"; }


void Object::readOptionVal(PStream &in, const string &optionname)
{
    OptionList &options = getOptionList();
    for (OptionList::iterator it = options.begin(); it != options.end(); ++it) {
        if ((*it)->optionname() == optionname) {
            (*it)->read(this, in);
            return;
        }
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
        if (dot_pos == string::npos || rb_pos < dot_pos) {
            int i = toint(optionname.substr(lb_pos + 1, rb_pos - lb_pos - 1));
            for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
                if ((*it)->optionname() == optname) {
                    (*it)->getIndexedObject(this, i)->readOptionVal(in, optionname.substr(rb_pos + 2));
                    return;
                }
        }
    } else if (lb_pos != string::npos)
        PLERROR("Object::readOptionVal() - Unmatched brackets");

    // No brackets, look for a dot
    if (dot_pos != string::npos) {
        // Found a dot, assume it's a field with an Object * field
        string optname = optionname.substr(0, dot_pos);
        string optoptname = optionname.substr(dot_pos + 1);
        for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
            if ((*it)->optionname() == optname) {
                (*it)->getAsObject(this)->readOptionVal(in, optoptname);
                return;
            }
    }
    // There are bigger problems in the world but still it isn't always funny
    PLERROR("Object::readOptionVal() - Unknown option \"%s\"", optionname.c_str());
}

void
Object::writeOptionVal(PStream &out, const string &optionname) const
{
    OptionList &options = getOptionList();
    for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
        if ((*it)->optionname() == optionname) {
            (*it)->write(this, out);
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
        if (dot_pos == string::npos || rb_pos < dot_pos) {
            int i = toint(optionname.substr(lb_pos + 1, rb_pos - lb_pos - 1));
            for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
                if ((*it)->optionname() == optname) {
                    (*it)->getIndexedObject(this, i)->writeOptionVal(out, optionname.substr(rb_pos + 2));
                    return;
                }
        }
    } else if (lb_pos != string::npos)
        PLERROR("Object::writeOptionVal() - Unmatched brackets");

    // No brackets, look for a dot
    if (dot_pos != string::npos) {
        // Found a dot, assume it's a field with an Object * field
        string optname = optionname.substr(0, dot_pos);
        string optoptname = optionname.substr(dot_pos + 1);
        for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
            if ((*it)->optionname() == optname) {
                (*it)->getAsObject(this)->writeOptionVal(out, optoptname);
                return;
            }
    }
    // There are bigger problems in the world but still it isn't always funny
    PLERROR("Object::writeOptionVal() - Unknown option \"%s\"", optionname.c_str());    
}







string Object::getOptionsToSave(OBflag_t option_flags) const
{
  string res = "";
  OptionList& options = getOptionList();
  
  for( OptionList::iterator it = options.begin(); it!=options.end(); ++it )
    {
      OptionBase::flag_t flags = (*it)->flags();
      if(!(flags & OptionBase::nosave) && (flags & option_flags))
        res += (*it)->optionname() + " ";
    }
  return res;
}


void Object::newread(PStream &in)
{
    string cl;
    getline(in.rawin(), cl, '(');
    cl = removeblanks(cl);
    if (cl != classname())
        PLERROR("Object::newread() - Was expecting \"%s\", but read \"%s\"",
                classname().c_str(), cl.c_str());

    skipBlanksAndComments(in.rawin());
    if (in.get() != ')') {
        in.unget();
        for (;;) {
            // Read all specified options
            string optionname;
            getline(in.rawin(), optionname, '=');
            optionname = removeblanks(optionname);
            skipBlanksAndComments(in.rawin());

            OptionList &options = getOptionList();
            OptionList::iterator it = find_if(options.begin(), options.end(),
                                              bind2nd(mem_fun(&OptionBase::isOptionNamed), optionname));
            if (it != options.end() && ((*it)->flags() & in.option_flags_in) == 0)
                (*it)->read_and_discard(in);
            else
                readOptionVal(in, optionname);
            skipBlanksAndComments(in.rawin());
            int c = in.get();
            if (c == ')')
                break;
            else if (c == ';') {
                skipBlanksAndComments(in.rawin());
                if (in.peek() == ')') {
                    in.get();
                    break;
                }
            } else
                PLERROR("Object::newread() - Expected field separator ';' "
                        "or end of object ')' after option \"%s\" but read"
                        " %c", optionname.c_str(), c);
        }
    }
    build(); // Build newly read Object
}

void
Object::newwrite(PStream &out) const
{
    vector<string> optnames = split(getOptionsToSave(out.option_flags_out));
    out << raw << classname() << "(\n";
    for (unsigned int i = 0; i < optnames.size(); ++i) {
        out << raw << optnames[i] << " = ";
        writeOptionVal(out, optnames[i]);
        if (i < optnames.size() - 1)
            out << raw << ";\n";
    }
    out << raw << " )\n";
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

int Object::call(const string& methodname, int nargs, istream& in_parameters, ostream& out_results)
{ 
  PLERROR("call method not implemented for this object"); 
  return -1;
}

void Object::run()
{ PLERROR("Not a runnable Object"); }

void Object::oldread(istream& in)
{ PLERROR("oldread method not implemented for this object"); }

void Object::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{ PLERROR("deepWrite method not implemented for this object"); }

void Object::deepRead(istream& in, DeepReadMap& old2new)
{ PLERROR("deepRead method not implemented for this object"); }

void Object::save(const string& filename) const
{ PLearn::save(filename, *this); }

void Object::load(const string& filename)
{ PLearn::load(filename, *this); }

Object::~Object()
{}

Object* deepReadObject(istream& in, DeepReadMap& old2new)
{
  pl_streambuf* buffer = dynamic_cast<pl_streambuf*>(in.rdbuf());
  pl_streammarker fence(buffer);
  while(in && in.get()!='<') ;
  string str_type_name;
  char tmpchar;
  while ((tmpchar = in.get()) !='>')
    str_type_name += tmpchar;
  if (str_type_name=="null") return 0;
  unsigned int p=str_type_name.find(":");
  if (p!=string::npos)
    str_type_name = str_type_name.substr(0,p);
  Object* res = TypeFactory::instance().newObject(str_type_name);
  if (!res)
    PLERROR("Type %s not declared in TypeFactory map (did you do a proper DECLARE_NAME_AND_DEEPCOPY ?)", str_type_name.c_str());
  buffer->seekmark(fence);
  res->deepRead(in, old2new);

  return res;
}

Object* loadObject(const string &filename)
{
    ifstream in_(filename.c_str());
    if (!in_)
        PLERROR("loadObject() - Could not open file \"%s\" for reading", filename.c_str());

    PStream in(&in_);
    Object *o = readObject(in);
    o->build();
    return o;
}


Object* readObject(PStream &in, unsigned int id)
{
    Object *o;
    in >> ws;

    //pl_streambuf* buf = dynamic_cast<pl_streambuf*>(in.rdbuf());
    pl_streammarker fence(in.pl_rdbuf());

    int c = in.peek();
    if (c == '<') {
        in.get(); // Eat '<'
        string cl;
        getline(in.rawin(), cl, '>');
        cl = removeblanks(cl);
        if (cl == "null")
            return 0;
        unsigned int p = cl.find(":");
        if (p != string::npos)
            cl = cl.substr(0, p);
        o = TypeFactory::instance().newObject(cl);
        if (!o)
            PLERROR("readObject() - Type \"%s\" not declared in TypeFactory map (did you do a proper DECLARE_NAME_AND_DEEPCOPY?)", cl.c_str());
        // Go back before the header starts
        in.pl_rdbuf()->seekmark(fence);
        o->read(in.rawin());
    } else if (c == '*') {
        in >> o;
    } else {
        // It must be a Classname(...) or a load(...) kind of definition
        string cl;
        getline(in.rawin(), cl, '(');
        cl = removeblanks(cl);
        if (cl == "load") {
            // It's a load("...")
            string fname;
            getline(in.rawin(), fname, ')');
            fname = removeblanks(fname);
            // TODO: Check if this is really what we want
            //       (ie: We could want to use the options
            //            of 'in' to load the object...)
            o = loadObject(fname);
        } else {
            // It's a Classname(opt1 = ...; ...; optn = ...); --> calls newread()
            o = TypeFactory::instance().newObject(cl);
            if (!o)
                PLERROR("readObject() - Type \"%s\" not declared in TypeFactory map (did you do a proper DECLARE_NAME_AND_DEEPCOPY?)", cl.c_str());
            in.pl_rdbuf()->seekmark(fence);
            o->newread(in);
        }
        if (id != LONG_MAX)
            in.copies_map_in[id] = o;
    }
    return o;
}

PStream& operator<<(PStream &out, const Object * &o)
{
    PLERROR("Not yet implemented");
    return out;
}

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
/*        if (o) {
            o->newread(in);
        }
        else*/
            o = readObject(in);
    }
    return in;
}



//////////////////////////////////////////////////////////
///// Tentative de nouveau systeme de serialisation //////
/////    en cours  (Pascal)                         //////
//////////////////////////////////////////////////////////

/*
void autoRead(istream& in)
{
  // read next token
  if it's </MYCLASSNAME> invoke build() and return  
  otherwise it's an optionname, so invoke readOptionVal

  at the end of it all, call build()
}

*/


%> // end of namespace PLearn

