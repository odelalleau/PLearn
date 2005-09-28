// Ensure that the Python function PyCFunction_NewEx works correctly

#include <plearn/python/PythonCodeSnippet.h>
#include <iostream>

using namespace PLearn;
using namespace std;

string python_code =
"import sys\n"
"\n"
"def trampoline_function_call(x):\n"
"    y = injected_c_function(x)\n"
"    print >>sys.stderr, 'The C function returned the charming value',y\n"
"\n"
"def trampoline_method_call(x):\n"
"    y = injected_c_method1(x)\n"
"    z = injected_c_method2(x)\n"
"    print >>sys.stderr, 'The C method 1 returned the charming value',y\n"
"    print >>sys.stderr, 'The C method 2 returned the charming value',z\n"
;

PythonObjectWrapper basic_function(const TVec<PythonObjectWrapper>& args)
{
    cout << "basic_function called with arg[0]="
         << args[0].as<int>() << endl;
    return PythonObjectWrapper(42);
}


struct X
{
    X(int value) : i(value) { }
  
    int i;
    PythonObjectWrapper f(const TVec<PythonObjectWrapper>& args);
    PythonObjectWrapper g(const TVec<PythonObjectWrapper>& args) const;
};

PythonObjectWrapper X::f(const TVec<PythonObjectWrapper>& args)
{
    cout << "X::f() called with i=" << i << " and arg[0]="
         << args[0].as<int>() << endl;
    return PythonObjectWrapper(1337);
}

PythonObjectWrapper X::g(const TVec<PythonObjectWrapper>& args) const
{
    cout << "X::g() called with i=" << i << " and arg[0]="
         << args[0].as<int>() << endl;
    return PythonObjectWrapper(1337*2);
}


int main()
{
    PP<PythonCodeSnippet> python = new PythonCodeSnippet(python_code);
    python->build();

    X x(13370);
    
    python->inject("injected_c_function", basic_function);
    python->inject("injected_c_function2", basic_function);
    python->inject("injected_c_method1", &x, &X::f);
    python->inject("injected_c_method2", &x, &X::g);

    python->call("trampoline_function_call", 64);
    python->call("trampoline_method_call",   128);

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
 
