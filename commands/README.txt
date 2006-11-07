This directory contains the source code of PLearn "executables" to be compiled with pymake.

If you look at the plearn_*.cc files closely, you will notice that they are
all built in the same manner, simply including a number of things they
need, and invoking a single function plearn_main.
So they only differ in the functionalities they #include. 

They have been grouped by external library dependencies that will be needed to compile and link each of them.

In short:

plearn_noblas   depends only on NSPR, boost  (if you don't have blas, it must be compiled with pymake -noblas plearn_noblas ) 
plearn_lapack   depends on NSPR, boost, BLAS, LAPACK
plearn_curses   depends on NSPR, boost, BLAS, LAPACK, ncurses
plearn_python   depends on NSPR, boost, BLAS, LAPACK, ncurses, python runtime libraries

Since plearn_noblas has the smallest number of requirements, it should be the
easiest to get to compile and link. But several important learning algorithms require LAPACK. 
plearn_lapack will contain the most useful 99\% of PLearn. 

Note that the plearn_python version has not yet successfully been compiled on Mac OS X.

IMPORTANT ADVICE:

If you are only going to be using PLearn with scripts or from within
python, you can simply compile one of the above (with pymake) that contains
enough of what you need and then never think about it anymore.

If however, you are going to write C++ code, I strongly advise you to write
your own mylearn.cc following the same model as the above programs, but
including only the pieces that you need for your project: this will greatly
reduce link time and executable size.

