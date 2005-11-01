// Test the object graph traversal functions

#include <string>
#include <iostream>
#include <plearn/base/ObjectGraphIterator.h>
#include <plearn/base/Object.h>
#include <plearn/math/TVec.h>
#include <plearn/io/openString.h>

using std::string;
using std::cout;
using std::endl;

namespace PLearn
{

//#####  X  ###################################################################

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

} // end of namespace PLearn


//#####  main  ################################################################

using namespace PLearn;

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


int main()
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
        return 1;
    }
    catch (...) 
    {
        cerr << "FATAL ERROR: uncaught unknown exception "
             << "(ex: out-of-memory when allocating a matrix)" << endl;
        return 2;
    }

    return 0;
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
  
