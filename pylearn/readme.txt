This directory contains the definition/implementation/building-facility for pylearn, the python-plearn interface. It is built using boost-python.

Look at the Makefile 
It calls pymake -so pylearn.cc
In pymake_config_model, I've added a trigger for boost/python.hpp and links with what is needed 
(hopefully, but does not work for Lisa currently, as boost-python is not correctly installed/located).

pymake -so pylearn.cc creates a OBJS/linux-i386__g++_dbg_double_throwerrors_blas/libpylearn.so
But python expects a .so with the *same* basename as that given in BOOST_PYTHON_MODULE(...) i.e. "pylearn.so"

Thus OBJS/linux-i386__g++_dbg_double_throwerrors_blas/libpylearn.so is copied as pylearn.so in 
PLearn/scripts/plearn_modules/ which should be in the PYTHONPATH

All you have to do is then to type 'import pylearn' in python.

-- Pascal
