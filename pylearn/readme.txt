This directory contains pylearn, the python-plearn interface. 

To build it, you need Boost.Python (including Pyste) and gccxml. Type
"make" to build, and "make install" to install.

The Makefile calls pyste to generate the pylearn.cpp file. It then calls
pymake -so pylearn.cpp. In pymake_config_model, a trigger was added for
boost/python.hpp and links with what is needed (hopefully, but does not
work for Lisa currently, as boost-python is not correctly
installed/located).

pymake -so pylearn.cpp creates a
OBJS/linux-i386__g++_dbg_double_throwerrors_blas/libpylearn.so. But python
expects a .so with the *same* basename as that given in
BOOST_PYTHON_MODULE(...) i.e. "pylearn.so" Therefore
OBJS/linux-i386__g++_dbg_double_throwerrors_blas/libpylearn.so is symlinked
to pylearn.so in PLearn/pylearn. The make install target copies it to
PLearn/scripts/plearn_modules which should be in the PYTHONPATH

All you have to do is then to type 'import pylearn' in python.

-- Pascal, Christian
