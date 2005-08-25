// Test that update_heap performs correctly.  This program constructs a
// large list of random numbers.  Then it modifies the numbers at random.
// If the heap invariant is VIOLATED after an update, it fixes it with
// update_heap, and then verifies that the invariant is satisfied again.

#include <assert.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <plearn/misc/heap_utilities.h>
#include <plearn/math/random.h>

const int n = 1000;                          // vector size
const int m = 100000;                        // number of random updates

using namespace std;
using namespace PLearn;

int main()
{
    vector<real> data(n);
    for (int i=0; i<n; ++i)
        data[i] = gaussian_01();
    make_heap(data.begin(), data.end());

    assert(is_valid_heap(data.begin(), data.end()));

    // Destroy heap and reconstruct it
    for (int i=0; i<m; ++i) {
        int j = int(bounded_uniform(0,n));
        assert(j>=0 && j<n);

        data[j] = gaussian_01();
        if (! is_valid_heap(data.begin(), data.end())) {
            update_heap(data.begin(), data.end(), data.begin()+j);
            assert(is_valid_heap(data.begin(), data.end()));
        }
    }

    cout << "Heap condition successfully restored" << endl;
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
