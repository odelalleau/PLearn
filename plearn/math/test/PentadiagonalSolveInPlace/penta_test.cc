#include <iostream>
#include <plearn/math/TMat_maths.h>
#include <plearn/math/BandedSolvers.h>

using namespace std;
using namespace PLearn;

int main()
{
  try {
    const int n = 10;
    Mat A(n,n);
    Vec a(n),b(n),c(n),y(n);
    for (int i=0; i<n; ++i) {
      A(i,i) = 50+1.5*i;
      a[i] = 50+1.5*i;
      if (i+1<n) {
        A(i,i+1) = 4+2*i;
        A(i+1,i) = 4+2*i;
        b[i] = 4+2*i;
      }
      if (i+2<n) {
        A(i,i+2) = i;
        A(i+2,i) = i;
        c[i] = i;
      }
      y[i] = 2*i+1;
    }

    cout << "A matrix is" << endl;
    cout << A << endl;
    cout << "y vector is " << endl;
    cout << y << endl;
    cout << "Solution found by Cholesky" << endl;
    Vec x = choleskySolve(A, y);
    cout << x << endl;

    cout << "Solution found by Pentadiagonal solver" << endl;
    PentadiagonalSolveInPlace(y, a, b, c);
    cout << y << endl;
  }
  catch (PLearnError e) {
    cout << "PLearnError: " << e.message() << endl;
  }
}
