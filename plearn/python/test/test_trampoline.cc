// Ensure that the Python function PyCFunction_NewEx works correctly

#include <plearn/python/PythonCodeSnippet.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <iostream>

using namespace PLearn;
using namespace std;

string python_code =
"import sys\n"
"\n"
"def trampoline_call(x):\n"
"    y = injected_c_function(x)\n"
"    print >>sys.stderr, 'The C function returned the charming value',y\n"
;

struct X
{
    X(int value) : i(value) { }
  
    int i;
    void f();
};

void X::f()
{
    cout << "X::f() called with i=" << i << endl;
}

typedef boost::function<void ()> XFunction;

PyObject* python_trampoline(PyObject* self, PyObject* args)
{
    XFunction *xfunc = reinterpret_cast<XFunction*>(self);

    // Should parse args here

    (*xfunc)();

    return PyInt_FromLong(64);
}

int main()
{
    PP<PythonCodeSnippet> python = new PythonCodeSnippet(python_code);
    python->build();

    // Build my interesting instance of X
    X x(42);

    // Bind it to a function object
    XFunction xfunc = boost::bind(&X::f, x);

    // Create a Python Function Object
    PyMethodDef py_method;
    py_method.ml_name  = NULL;
    py_method.ml_meth  = python_trampoline;
    py_method.ml_flags = METH_VARARGS;
    py_method.ml_doc   = NULL;
    PyObject* py_funcobj = PyCFunction_NewEx(&py_method,
                                             reinterpret_cast<PyObject*>(&xfunc),
                                             NULL /* module */);

    // Inject into the running python snippet
    python->setGlobalObject("injected_c_function", py_funcobj);

    // And now call our darling
    python->invoke("trampoline_call", 64);

    Py_XDECREF(py_funcobj);
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
 
