../scripts/plearn_modules/pylearn.so:	pylearn.cc
	\rm -Rf OBJS
	pymake -so pylearn.cc
	\rm -Rf ../pylearn.so
	cp OBJS/*/libpylearn.so ../scripts/plearn_modules/pylearn.so

