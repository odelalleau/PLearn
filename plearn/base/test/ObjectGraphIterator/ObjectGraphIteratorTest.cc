// -*- C++ -*-

// ObjectGraphIteratorTest.cc
//
// Copyright (C) 2005 Nicolas Chapados
// Copyright (C) 2005 Olivier Delalleau 
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

/* *******************************************************      
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Nicolas Chapados, Olivier Delalleau

/*! \file ObjectGraphIteratorTest.cc */


#include "ObjectGraphIteratorTest.h"

#include <string>
#include <iostream>
#include <plearn/base/ObjectGraphIterator.h>
#include <plearn/base/Object.h>
#include <plearn/math/TVec.h>
#include <plearn/io/openString.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ObjectGraphIteratorTest,
    "Test the object graph traversal functions.",
    ""
);

//////////////////
// ObjectGraphIteratorTest //
//////////////////
ObjectGraphIteratorTest::ObjectGraphIteratorTest() 
    /* ### Initialize all fields to their default value */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

///////////
// build //
///////////
void ObjectGraphIteratorTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ObjectGraphIteratorTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ObjectGraphIteratorTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void ObjectGraphIteratorTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &ObjectGraphIteratorTest::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void ObjectGraphIteratorTest::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

class X : public Object
{
    typedef Object inherited;
    PLEARN_DECLARE_OBJECT(X);

public:
    string name;
    PP<X> child;
    
public:
    X() {}

    static void declareOptions(OptionList& ol)
    {
        declareOption(ol, "name",  &X::name,  OptionBase::buildoption, "");
        declareOption(ol, "child", &X::child, OptionBase::buildoption, "");
    }
    
    virtual void printName()
    {
        cout << "X::printName: " << name << endl;
    }

    virtual void method1(string s)
    {
        cout << "X::method1: name='" << name << "'  arg1='" << s << "'" << endl;
    }
    
    virtual void method2(string s, int i)
    {
        cout << "X::method1: name='" << name
             << "'  arg1='" << s << "'"
             << "  arg2='" << i << "'" << endl;
    }
};

DECLARE_OBJECT_PTR(X);
PLEARN_IMPLEMENT_OBJECT(X, "", "");


//#####  Y  ###################################################################

class Y : public X
{
    typedef X inherited;
    PLEARN_DECLARE_OBJECT(Y);

public:
    Y() {}

    virtual void printName()
    {
        cout << "Y::printName: " << name << endl;
    }
};

DECLARE_OBJECT_PTR(Y);
PLEARN_IMPLEMENT_OBJECT(Y, "", "");


//#####  Z  ###################################################################

class Z : public Object
{
    typedef Object inherited;
    PLEARN_DECLARE_OBJECT(Z);

public:
    string dummy_option1;
    TVec< PP<X> > sub_objects;
    int dummy_option2;
    
public:
    Z() { dummy_option2 = 0; }
    
    static void declareOptions(OptionList& ol)
    {
        declareOption(ol, "dummy_option1", &Z::dummy_option1, OptionBase::buildoption, "");
        declareOption(ol, "sub_objects",   &Z::sub_objects,   OptionBase::buildoption, "");
        declareOption(ol, "dummy_option2", &Z::dummy_option2, OptionBase::buildoption, "");
    }
};

DECLARE_OBJECT_PTR(Z);
PLEARN_IMPLEMENT_OBJECT(Z, "", "");



//#####  main  ################################################################

void iterate(ObjectGraphIterator grit, ObjectGraphIterator grend)
{
    for ( ; grit != grend ; ++grit ) {
        const Object* curobj = *grit;
        cout << "Encountered class \"" << curobj->classname() << "\""
             << " at option \"" << grit.getCurrentOptionName() << "\"" << endl;
        if (const X* x = dynamic_cast<const X*>(curobj)) {
            cout << "... and name is: " << x->name << endl;
        }
    }
}


const char* test_objects[] = {
    "Z(sub_objects = [])",

    "Z(sub_objects = [X(name=\"X1\"),          \n"
    "                 *1->X(name=\"X2\"),      \n"
    "                 Y(name=\"Y3\"),          \n"
    "                 *1,                      \n"
    "                 *2->Y(name=\"Y5\"),      \n"
    "                 X(name=\"X6\",           \n"
    "                   child = X(child = Y(name = \"innerY\"))) \n"
    "                 *2])"
};

const int num_tests = sizeof(test_objects) / sizeof(test_objects[0]);


/////////////
// perform //
/////////////
void ObjectGraphIteratorTest::perform()
{
    try {
        pout.setMode(PStream::plearn_ascii);
        for (int i=0 ; i<num_tests ; ++i) {
            string test_object = test_objects[i];
            cout << endl
                 << "- - - - - - - - - - -" << endl
                 << "Building object structure from:" << endl
                 << test_object << endl;

            PStream strin = openString(test_object, PStream::plearn_ascii);
            PP<Object> o;
            strin >> o;
    
            cout << endl << "Built structure: " << endl;
            pout << o << flush;
            pout.copies_map_out.clear();

            cout << endl << "Now traversing the graph in breadth-first:" << endl;
            iterate(ObjectGraphIterator(o, ObjectGraphIterator::BreadthOrder, true),
                    ObjectGraphIterator());

            cout << endl << "Traversing only Y objects in depth-first:" << endl;
            iterate(ObjectGraphIterator(o, ObjectGraphIterator::DepthPreOrder, true, "Y"),
                    ObjectGraphIterator());

            cout << endl << "Broadcast a call to X::printName()" << endl;
            memfun_broadcast(o, &X::printName);
    
            cout << endl << "Broadcast a call to Y::printName()" << endl;
            memfun_broadcast(o, &Y::printName);

            cout << endl << "Broadcast a breadth-first call to X::method1(\"foo\")" << endl;
            memfun_broadcast(o, &X::method1, string("foo"), ObjectGraphIterator::BreadthOrder);

            cout << endl << "Broadcast a breadth-first call to X::method1(option_name)" << endl;
            memfun_broadcast_optname(o, &X::method1, ObjectGraphIterator::BreadthOrder);

            cout << endl << "Broadcast a breadth-first call to X::method2(\"foo\",42)" << endl;
            memfun_broadcast(o, &X::method2, string("foo"), 42, ObjectGraphIterator::BreadthOrder);
        }
    }
    
    catch(const PLearnError& e)
    {
        cerr << "FATAL ERROR: " << e.message() << endl;
    }
    catch (...) 
    {
        cerr << "FATAL ERROR: uncaught unknown exception "
             << "(ex: out-of-memory when allocating a matrix)" << endl;
    }
}

} // end of namespace PLearn


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
