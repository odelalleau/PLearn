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
   * $Id: Object.cc,v 1.33 2004/09/14 16:04:35 chrish42 Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

#include "Object.h"
#include "stringutils.h"    //!< For removeblanks.
#include <plearn/io/fileutils.h>
#include "TypeFactory.h"
//#include <iostream>

#include <algorithm>

namespace PLearn {
using namespace std;

Object::Object()
{}

PLEARN_IMPLEMENT_OBJECT(Object, "Base class for PLearn Objects", "NO HELP");   

// by default, do nothing...
void Object::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{}

void Object::setOption(const string& optionname, const string& value)
{
    istrstream in_(value.c_str());
    PStream in(&in_);
    readOptionVal(in, optionname);
}

string Object::getOption(const string &optionname) const
{ 
  ostrstream out_;
  PStream out(&out_);
  writeOptionVal(out, optionname);
  char* buf = out_.str();
  int n = out_.pcount();
  string s(buf,n);
  out_.freeze(false); // return ownership to the stream, so that it may free it...
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
      in.putback(c);
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
          // if (it != options.end() && ((*it)->flags() & in.option_flags_in) == 0)
          if (it!=options.end() && (*it)->shouldBeSkipped() )
            (*it)->read_and_discard(in);
          else
            {
              // cerr << "Reading option: " << optionname << endl;
              readOptionVal(in, optionname);
              // cerr << "returned from reading optiion " << optionname << endl;
            }
          in.skipBlanksAndCommentsAndSeparators();
          /*
          in.skipBlanksAndCommentsAndSeparators();
          in.skipBlanksAndCommentsAndSeparators();
          in.skipBlanksAndCommentsAndSeparators();
          in.skipBlanksAndCommentsAndSeparators();
          cerr << "PEEK1: " << in.peek() << endl;
          in.peek(); in.peek(); in.peek();
          cerr << "PEEK2: " << in.peek() << endl;
          */

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
  vector<string> optnames = split(getOptionsToSave());
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


void Object::call(const string& methodname, int nargs, PStream& in_parameters, PStream& out_results)
{
  PLERROR("In Object::call no method named %s supported by this object's call method.", methodname.c_str());
}

void Object::run()
{ PLERROR("Not a runnable Object"); }

void Object::oldread(istream& in)
{ PLERROR("oldread method not implemented for this object"); }

void Object::save(const string& filename) const
{ PLearn::save(filename, *this); }

void Object::load(const string& filename)
{ PLearn::load(filename, *this); }

Object::~Object()
{}


Object* loadObject(const string &filename)
{
#if STREAMBUFVER == 1
  PStream in = openFile(filename, "r", PStream::plearn_ascii);
#else
  ifstream in_(filename.c_str());
  if (!in_)
    PLERROR("loadObject() - Could not open file \"%s\" for reading", filename.c_str());
  PStream in(&in_);
#endif
  Object *o = readObject(in);
  o->build();
  return o;
}

Object* macroLoadObject(const string &filename, map<string, string>& vars)
{
  string script = readFileAndMacroProcess(filename, vars);
  PIStringStream sin(script);
  Object* o = readObject(sin);
  o->build();
  return o;
}
  
Object* macroLoadObject(const string &filename)
{
  map<string, string> vars;
  return macroLoadObject(filename,vars);
}

Object* readObject(PStream &in, unsigned int id)
{
    Object *o=0;
    in.skipBlanksAndCommentsAndSeparators();

    //pl_streambuf* buf = dynamic_cast<pl_streambuf*>(in.rdbuf());
#if STREAMBUFVER == 0
    pl_streammarker fence(in.pl_rdbuf());
#endif

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
#if STREAMBUFVER == 0
        in.pl_rdbuf()->seekmark(fence);
#else
        in.unread(cl+'(');
#endif
        o->newread(in);
      }
       
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

} // end of namespace PLearn


//! Useful function for debugging inside gdb:
extern "C"
{
  void printobj(PLearn::Object* p)
  {
    PLearn::PStream perr(&std::cerr);
    perr << *p;
    perr.endl();
  }
}


