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
    PLEARN_DECLARE_OBJECT(Z);

public:
    string dummy_option1;
    TVec< PP<X> > sub_objects;
    int dummy_option2;
    
public:
    Z() {}
    
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

int main()
{
    using namespace PLearn;
    string test_objects =
        "Z(sub_objects = [X(name=\"X1\"),          \n"
        "                 *1->X(name=\"X2\"),      \n"
        "                 Y(name=\"Y3\"),          \n"
        "                 *1,                      \n"
        "                 *2->Y(name=\"Y5\"),      \n"
        "                 X(name=\"X6\",           \n"
        "                   child = X(child = Y(name = \"innerY\"))) \n"
        "                 *2])";

    cout << "Building object structure from:" << endl
         << test_objects << endl;

    PStream strin = openString(test_objects, PStream::plearn_ascii);
    PP<Object> o;
    strin >> o;
    
    pout << "Built structure: " << endl
         << o;

    ObjectGraphIterator ogrit(o, ObjectGraphIterator::BreadthOrder), ogrend;
    for ( ; ogrit != ogrend ; ++ogrit ) {
        const Object* curobj = *ogrit;
        cout << "Encountered class \"" << curobj->classname() << "\"" << endl;
        if (const X* x = dynamic_cast<const X*>(curobj)) {
            cout << "... and name is: " << x->name << endl;
        }
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
  
