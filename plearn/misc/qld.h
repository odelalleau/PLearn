#ifndef qld_INC
#define qld_INC

/**
 * Declaration of the Fortran QL0001 routine
 */

int ql0001_(int *m,      int *me,     int *mmax,
            int *n,      int *nmax,   int *mnn,
            double *c,   double *d,   double *a,
            double *b,   double *xl,  double *xu,
            double *x,   double *u,
            int *iout,   int *ifail,  int *iprint,
            double *war, int *lwar,
            int *iwar,   int *liwar,
            double *eps1);

#endif


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
