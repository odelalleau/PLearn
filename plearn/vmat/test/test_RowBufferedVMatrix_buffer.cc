#include <iostream>

#include <plearn/io/fileutils.h>
#include <plearn/vmat/VMat.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/math/TVec.h>

using namespace std;
using namespace PLearn;

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " matrix_filename.pmat" << endl;
        return 1;
    }

    const int row_size = 3;
    double first_insert_data[] = { 20010101, -1, 42 };
    double second_insert_data[] = { 20000813, 37, 49 };

    VMat mat = new FileVMatrix(argv[1], 0, row_size);
    mat->setOption("remove_when_done", "1");
    mat->insertRow(0, Vec(row_size, first_insert_data));
    mat->insertRow(0, Vec(row_size, second_insert_data));

    if (mat->get(0, 0) != 20000813)
        cout << "Test failed!" << endl;

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
