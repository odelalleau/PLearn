PYSTE_FILES = $(wildcard *.pyste)
CPP_FILES = $(addprefix pylearn/_, $(addsuffix .cpp, $(basename $(PYSTE_FILES))))

pylearn.so: $(CPP_FILES) | gen_pymake_buildenv gen_main
	cd pylearn && pymake -safeopt -so pylearn.cc
	ln -sf pylearn/libpylearn.so pylearn.so
	chmod -x pylearn.so

gen_pymake_buildenv: $(CPP_FILES:.cpp=.h)
	touch pylearn/pylearn.cc.new
	for f in $(PYSTE_FILES:.pyste=.h); do echo >>pylearn/pylearn.cc.new \#include \"_$$f\"; done
	echo >>pylearn/pylearn.cc.new \#include \"_main.h\"
	(cd pylearn; if cmp pylearn.cc.new pylearn.cc >/dev/null; then rm pylearn.cc.new; else cp pylearn.cc.new pylearn.cc; fi)

gen_main: pylearn/_main.h
	pyste --module=pylearn --multiple --out=pylearn --generate-main $(PYSTE_FILES)

pylearn/%.h: FORCE
	test -f $@ || touch $@

FORCE:

pylearn/_%.cpp: %.pyste
	pyste --multiple --out=pylearn --cache-dir=pyste.cache -I `pwd`/.. -D USE_EXCEPTIONS -D USEDOUBLE $<
	touch $@

install:
	cp pylearn.so ../scripts/plearn_modules
	strip ../scripts/plearn_modules/pylearn.so

clean:
	-rm -rf OBJS pylearn/OBJS pylearn.so pylearn

realclean: clean
	-rm -rf pyste.cache

.PHONY: gen_pymake_buildenv gen_main
