pylearn.so: pylearn.cpp
	pymake -so pylearn
	ln -sf OBJS/linux-i386__g++_dbg_double_throwerrors_blas/libpylearn.so pylearn.so
	chmod -x pylearn.so

pylearn.cpp: pylearn.pyste
	pyste --cache-dir=pyste.cache -I `pwd`/.. -D USEDOUBLE pylearn.pyste

install:
	cp pylearn.so ../scripts/plearn_modules
	strip ../scripts/plearn_modules/pylearn.so

clean:
	-rm -rf OBJS pylearn.so pylearn.cpp

realclean: clean
	-rm -rf pyste.cache
