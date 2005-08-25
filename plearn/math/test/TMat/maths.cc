#include <iostream>
#include <plearn/math/TMat_maths.h>
#include <plearn/math/random.h>

using namespace std;
using namespace PLearn;

#define LOOPS  10
#define BOUNDS 10

template <class T>
void simple_maths( TVec<T> vec )
{
    cout << "sign:\n" << sign( vec ) << endl;

    cout << "sum:\n" << sum( vec ) << endl;
    cout << "sum (ignore missing):\n" << sum( vec, true ) << endl;

    cout << "sumabs:\n" << sumabs( vec ) << endl;  
    cout << "sumsquare:\n" << sumsquare( vec ) << endl;
    cout << "sum_of_log:\n" << sum_of_log( vec ) << endl;
    cout << "product:\n" << product( vec ) << endl;

    cout << "mean:\n" << mean( vec ) << endl;
    cout << "mean (ignore missing):\n" << mean( vec, true ) << endl;

    cout << "harmonic_mean:\n" << harmonic_mean( vec ) << endl;
    cout << "harmonic_mean (ignore_missing):\n" << harmonic_mean( vec, true ) << endl;
  
// No negative val accepted
//  cout << "geometric_mean:\n" << geometric_mean( vec ) << endl;
  
    cout << "min:\n" << min( vec ) << endl;
    cout << "argmin:\n" << argmin( vec ) << endl;

    cout << "max:\n" << max( vec ) << endl;
    cout << "argmax:\n" << argmax( vec ) << endl;
  
    cout << "norm:\n" << norm( vec ) << endl;
    cout << "log:\n" << log( vec ) << endl;
    cout << "sqrt:\n" << sqrt( vec ) << endl;
    cout << "tanh:\n" << tanh( vec ) << endl;
    cout << "fasttanh:\n" << fasttanh( vec ) << endl;

// Values are expected to be in [0, 1]
//  cout << "sigmoid:\n" << sigmoid( vec ) << endl;
//  cout << "inverse_sigmoid:\n" << inverse_sigmoid( vec ) << endl;

    cout << "inverted:\n" << inverted( vec ) << endl;
    cout << "logadd:\n" << logadd( vec ) << endl;
    cout << "square:\n" << square( vec ) << endl;
    cout << "squareroot:\n" << squareroot( vec ) << endl;
    cout << "remove_missing:\n" << remove_missing( vec ) << endl;
    cout << "softmax:\n" << softmax( vec ) << endl;
    cout << "exp:\n" << exp( vec ) << endl;
    cout << "nonZeroIndices:\n" << nonZeroIndices( vec ) << endl;
    cout << "median:\n" << median( vec ) << endl;  
}

int main()
{
    try {
        PLWARNING("A lot of the simple maths function behave badly "
                  "when the vec has length 0. TO BE FIXED!!!"
                  "(and then remove the +1 below)");
        for ( int i=0; i < LOOPS; i++ )
        {
            Vec v( i*20 + 1 );
            fill_random_uniform( v, -BOUNDS, +BOUNDS );

            cout << "THE VEC:\n" << v << endl << endl; 
            simple_maths( v );
            cout << "=====================================" << endl << endl;
        }
    }
    catch (PLearnError e) {
        cerr << "FATAL ERROR: " << e.message() << endl;
    }

    return 0;
}


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
