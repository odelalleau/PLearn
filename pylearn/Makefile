pylearn.so: pylearn.cpp
	pymake -so pylearn
	ln -sf OBJS/linux-i386__g++_dbg_double_throwerrors_blas/libpylearn.so pylearn.so

pylearn.cpp: pylearn.pyste
	pyste --cache-dir=pyste.cache -I ~/PLearn -D USEDOUBLE pylearn.pyste

install:
	cp pylearn.so ../scripts/plearn_modules
