CC=g++
CFLAGS=-Wall -fPIC -O3
LIBCBLAS=~bastienf/.NOBACKUP/CBLAS/lib/${OSARCH}/libcblas.a

LIBBLAS=-lblas ${LIBCBLAS}
#LIBGOTO=-lgoto -lgfortran ${LIBCBLAS}
LIBGOTO=-lgoto -lgfortran -I~/PLearn
LIBATLAS=-lcblas -lf77blas -latlas -lg2c
LIBMKL=-lmkl -lvml ${LIBCBLAS}
LIBMKLPT=-lmkl -lvml -lguide -lpthread ${LIBCBLAS}

ACMLBASE=/u/lisa/local/acml-3-6-1beta/gfortran64
ACMLBASE=/u/lisa/local/acml-3-6-0/gfortran32
LIBACML= ${LIBCBLAS} ${ACMLBASE}/lib/libacml.a -lgfortran
LIBACMLPT= ${LIBCBLAS} ${ACMLBASE}_mp/lib/libacml_mp.a -lg2c -lgfortran
LIBNVIDIA=-L/u/bastienf/NVIDIA_CUDA_SDK/cuda/lib -L/u/bastienf/NVIDIA_CUDA_SDK/lib/ -I/u/bastienf/NVIDIA_CUDA_SDK/cuda/include/ -lcuda -lcudart -lGL -lGLU -lcublas 
#-l cutil

all: xgemm

xgemm: xgemm-atlas xgemm-blas xgemm-goto xgemm-mkl xgemm-nvidia xgemm-cxgemm xgemm-sgemm-goto xgemm-dgemm-goto xgemm-sgemm-blas xgemm-dgemm-blas
	true
#xgemm-blas-compare xgemm-goto-compare xgemm-nvidia-compare

xgemm-sgemm: xgemm-sgemm-acml xgemm-sgemm-atlas xgemm-sgemm-blas xgemm-sgemm-c xgemm-sgemm-goto xgemm-sgemm-mkl xgemm-sgemm-nvidia

xgemm-atlas: xgemm-sgemm-atlas xgemm-dgemm-atlas

xgemm-blas: xgemm-sgemm-blas xgemm-dgemm-blas

xgemm-goto: xgemm-sgemm-goto xgemm-dgemm-goto

xgemm-mkl: xgemm-sgemm-mkl xgemm-dgemm-mkl

xgemm-nvidia: xgemm-sgemm-nvidia

xgemm-cxgemm: xgemm-sgemm-c xgemm-dgemm-c

xgemm-sgemm-acml: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEFLOAT $< ${LIBACML}

xgemm-sgemm-acmlpt: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEFLOAT $< -fopenmp ${LIBACMLPT}

xgemm-sgemm-atlas: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEFLOAT $< ${LIBATLAS}

xgemm-dgemm-atlas: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEDOUBLE $< ${LIBATLAS}

xgemm-sgemm-blas: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEFLOAT $< ${LIBBLAS}

xgemm-dgemm-blas: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEDOUBLE $< ${LIBBLAS}

xgemm-sgemm-blas-compare: xgemm.c
	${CC} ${CFLAGS} -o $@ $< ${LIBBLAS} -DCOMPARE

xgemm-sgemm-c: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEFLOAT $< -DCXGEMM 

xgemm-dgemm-c: xgemm.c
	${CC} ${CFLAGS} -o $@ $< -DCXGEMM -DUSEDOUBLE

xgemm-sgemm-goto: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEFLOAT $< ${LIBGOTO}

xgemm-dgemm-goto: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEDOUBLE $< ${LIBGOTO}

xgemm-sgemm-goto-compare: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEFLOAT $< -DCOMPARE ${LIBGOTO}

xgemm-sgemm-mkl: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEDOUBLE $< ${LIBMKL}

xgemm-dgemm-mkl: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEFLOAT $< ${LIBMKL}

xgemm-sgemm-mklpt: xgemm.c
	${CC} ${CFLAGS} -o $@ -DUSEFLOAT $< ${LIBMKLPT}

xgemm-sgemm-nvidia: xgemm.c
	${CC} ${CFLAGS} -m32 -o $@ -DUSEFLOAT $<  ${LIBNVIDIA} -DNVIDIA

xgemm-sgemm-nvidia-compare: xgemm.c
	${CC} ${CFLAGS} -m32 -o $@ -DUSEFLOAT $< ${LIBNVIDIA} -DNVIDIA -DCOMPARE

xgemm-sgemm-nvidia-64: xgemm.c
	${CC} ${CFLAGS} -m64 -o $@ -DUSEFLOAT $<  ${LIBNVIDIA} -DNVIDIA

xgemm-sgemm-nvidia-compare-64: xgemm.c
	${CC} ${CFLAGS} -m64 -o $@ -DUSEFLOAT $< ${LIBNVIDIA} -DNVIDIA -DCOMPARE

xgemv: xgemv-blas xgemv-blas-compare xgemv-cxgemm xgemv-goto xgemv-goto-compare xgemv-nvidia xgemv-nvidia-compare xgemv-sgemv-goto xgemv-dgemv-goto xgemv-sgemv-blas xgemv-dgemv-blas
	true

clean-xgemv: 
	rm -f xgemv xgemv-blas xgemv-blas-compare xgemv-cxgemm xgemv-goto xgemv-goto-compare xgemv-sgemv-goto xgemv-dgemv-goto xgemv-sgemv-blas xgemv-dgemv-blas

xgemv-blas: xgemv.c
	${CC} ${CFLAGS} -o $@ $< ${LIBBLAS}

xgemv-blas-compare: xgemv.c
	${CC} ${CFLAGS} -o $@ $< ${LIBBLAS} -DCOMPARE

xgemv-goto: xgemv.c
	${CC} ${CFLAGS} -o $@ $< ${LIBGOTO}

xgemv-goto-compare: xgemv.c
	${CC} ${CFLAGS} -o $@ $< ${LIBGOTO} -DCOMPARE

xgemv-cxgemm: xgemv.c
	${CC} ${CFLAGS} -o $@ $< ${LIBBLAS} -DCXGEMV

xgemv-nvidia: xgemv.c
	${CC} ${CFLAGS} -m32 -o $@ $< ${LIBNVIDIA} -DNVIDIA

xgemv-nvidia-compare: xgemv.c
	${CC} ${CFLAGS} -m32 -o $@ $< ${LIBNVIDIA} -DNVIDIA -DCOMPARE

xgemv-sgemv-goto: xgemv.c
	${CC} ${CFLAGS} -o $@ $< ${LIBGOTO}

xgemv-dgemv-goto: xgemv.c
	${CC} ${CFLAGS} -o $@ -DUSEDOUBLE $< ${LIBGOTO}

xgemv-sgemv-blas: xgemv.c
	${CC} ${CFLAGS} -o $@ $< ${LIBBLAS}

xgemv-dgemv-blas: xgemv.c
	${CC} ${CFLAGS} -o $@ -DUSEDOUBLE $< ${LIBBLAS}

clean: clean-xgemv
	rm -f xgemm-blas-compare xgemm-goto-compare xgemm-nvidia-compare xgemm-blas xgemm-goto xgemm-nvidia xgemm-cxgemm
	rm -f xgemm-sgemm-goto xgemm-dgemm-goto xgemm-sgemm-blas xgemm-dgemm-blas
	rm -f xgemm-acml xgemm-acmlpt xgemm-atlas xgemm-mkl xgemm-mklpt xgemv-nvidia xgemv-nvidia-compare


