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
   * $Id: Object.cc,v 1.20 2003/08/08 20:45:54 yoshua Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

#include "Object.h"
#include "stringutils.h"
#include "fileutils.h"
#include "TypeFactory.h"
#include <iostream>

namespace PLearn <%
using namespace std;

Object::Object()
{}

PLEARN_IMPLEMENT_OBJECT_METHODS(Object, "Object", Object);   

// by default, do nothing...
void Object::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{}

string Object::optionHelp() 
{ return ""; }

void Object::setOption(const string& optionname, const string& value)
{
    istrstream in_(value.c_str());
    PStream in(&in_);
    in >> option_flags(dft_option_flag | OptionBase::nosave);
    readOptionVal(in, optionname);
}

string Object::help()
{
  return "Base class for PLearn Objects";
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

string Object::info() const { return classname(); }

void Object::print(ostream& out) const
{ 
  PStream pout(&out);
  pout << *this << endl;
  //out << '{' << info() << "} ";
}


void Object::readOptionVal(PStream &in, const string &optionname)
{
  try 
    {
      OptionList &options = getOptionList();
      for (OptionList::iterator it = options.begin(); it != options.end(); ++it) 
        {
          if ((*it)->optionname() == optionname) 
            {
              (*it)->read(this, in);
              return;
            }
        }

      // Found no options matching 'optionname'. First look for brackets. If there
      // are brackets, they must be located before any dot.
      size_t lb_pos = optionname.find('[');
      size_t rb_pos = optionname.find(']');
      size_t dot_pos = optionname.find('.');
      if (rb_pos != string::npos) 
        {
          if (lb_pos == string::npos)
            PLERROR("Object::readOptionVal() - Unmatched brackets");
          string optname = optionname.substr(0, lb_pos);
          if (dot_pos == string::npos || rb_pos < dot_pos) 
            {
              int i = toint(optionname.substr(lb_pos + 1, rb_pos - lb_pos - 1));
              for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
                if ((*it)->optionname() == optname) 
                  {
                    (*it)->getIndexedObject(this, i)->readOptionVal(in, optionname.substr(rb_pos + 2));
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
          for (OptionList::iterator it = options.begin(); it != options.end(); ++it)
            if ((*it)->optionname() == optname) 
              {
                (*it)->getAsObject(this)->readOptionVal(in, optoptname);
                return;
              }
        }
    }
  catch(const PLearnError& e)
    { 
      PLERROR("Problem while attempting to read value of option %s of a %s:\n %s", 
              optionname.c_str(), classname().c_str(), e.message().c_str()); 
    }

  // There are bigger problems in the world but still it isn't always funny
  PLERROR("There is no option named %s in a %s", optionname.c_str(),classname().c_str());
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
  in.getline(cl, '(');
  cl = removeblanks(cl);
  if (cl != classname())
    PLERROR("Object::newread() - Was expecting \"%s\", but read \"%s\"",
            classname().c_str(), cl.c_str());

  in.skipBlanksAndComments();
  int c = in.get();
  if (c != ')') 
    {
      in.unget();
      for (;;) 
        {
          // Read all specified options
          string optionname;
          in.getline(optionname, '=');
          optionname = removeblanks(optionname);
          in.skipBlanksAndComments();

          OptionList &options = getOptionList();
          OptionList::iterator it = find_if(options.begin(), options.end(),
                                            bind2nd(mem_fun(&OptionBase::isOptionNamed), optionname));
          if (it != options.end() && ((*it)->flags() & in.option_flags_in) == 0)
            (*it)->read_and_discard(in);
          else
            readOptionVal(in, optionname);

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

void Object::newwrite(PStream &out) const
{
  vector<string> optnames = split(getOptionsToSave(out.option_flags_out));
  out.write(classname());
  out.write("(\n");
  for (unsigned int i = 0; i < optnames.size(); ++i) 
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


void Object::call(const string& methodname, int nargs, PStream& in_parameters, PStream& out_results)
{
  PLERROR("In Object::call no method named %s supported by this object's call method.", methodname.c_str());
}

void Object::run()
{ PLERROR("Not a runnable Object"); }

void Object::oldread(istream& in)
{ PLERROR("oldread method not implemented for this object"); }

void Object::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{ PLERROR("DEPRECATED deepWrite method not implemented for this object. You should no longer call it anyway. It's DEPRECATED!"); }

void Object::deepRead(istream& in, DeepReadMap& old2new)
{ PLERROR("DEPRECATED deepRead method not implemented for this object. You should no longer call it anyway. It's DEPRECATED!"); }

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
    Object *o=0;
    in.skipBlanksAndCommentsAndSeparators();

    //pl_streambuf* buf = dynamic_cast<pl_streambuf*>(in.rdbuf());
    pl_streammarker fence(in.pl_rdbuf());

    int c = in.peek();
    if (c == '<')  // Old (deprecated) serialization mode 
      {
        in.get(); // Eat '<'
        string cl;
        in.getline(cl, '>');
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
        o->read(in._do_not_use_this_method_rawin_());
      } 
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
        in.pl_rdbuf()->seekmark(fence);
        o->newread(in);
      }
       
    if (id != LONG_MAX)
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
        in.skipBlanksAndCommentsAndSeparators();
        if (id==0)
          x = 0;
        else if (in.peek() == '-') 
          {
            in.get(); // Eat '-'
            char cc = in.get();
            if(cc != '>') // Eat '>'
              PLERROR("In PStream::operator>>(Object*&)  Wrong format.  Expecting \"*%d->\" but got \"*%d-%c\".", id, id, cc);
            in.skipBlanksAndCommentsAndSeparators();
            if(x)
              in >> *x;
            else // x is null
              x = readObject(in, id);
            in.skipBlanksAndCommentsAndSeparators();
            in.copies_map_in[id]= x;
          } 
        else 
          {
            // Find it in map and return ptr;
            map<unsigned int, void *>::iterator it = in.copies_map_in.find(id);
            if (it == in.copies_map_in.end())
              PLERROR("In PStream::operator>>(Object*&) object (ptr) to be read has not been previously defined");
            x= static_cast<Object *>(it->second);
          }
      } 
    else
      {
        x = readObject(in);
        in.skipBlanksAndCommentsAndSeparators();
      }

    return in;
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

%> // end of namespace PLearn


//! Useful function for debugging inside gdb:
extern "C"
{
  void printobj(PLearn::Object* p)
  {
    PLearn::PStream perr(&std::cerr);
    perr << *p << std::endl;
  }
}


