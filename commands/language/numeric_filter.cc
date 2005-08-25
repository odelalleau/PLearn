#include <plearn/base/general.h>
#include <plearn/io/TypesNumeriques.h>

using namespace PLearn;

// usage : numeric_filter < in.text > filtered_out.text

int main(int argc, char** argv)
{
    if (argc != 1)
        PLERROR("usage : numeric_filter < in.text > filtered_out.text");
    string word;
    while (true)
    {
        cin >> word;
        if (!cin) break;
        if (looksNumeric(word.c_str()))
            //cout << "<numeric>" << " : " << word << endl;
            cout << "<numeric>" << endl;
        else
            cout << word << endl;
    }
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
