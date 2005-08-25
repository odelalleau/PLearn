#include <plearn/base/general.h>
#include <plearn/base/stringutils.h>      //!< For pgetline.
#include <plearn_learners/language/WordNet/WordNetOntology.h>

using namespace PLearn;

int main(int argc, char** argv)
{
    if (argc != 2)
        PLERROR("usage : pos.text < in.text > filtered_out.text");
    string word, pos_str;
    int pos;
    ifstream pos_in(argv[1]);
    while (true)
    {
        cin >> word;
        if (!cin) break;
        if (word == "") continue;
        pos_str = pgetline(pos_in);
        if (!pos_in) break;
        if (pos_str == "") continue;
        //cout << pos_str << endl;
        pos = toint(pos_str);
        word = lowerstring(word);
        if (pos == NUMERIC_TYPE)
            cout << NUMERIC_TAG << endl;
        else if (pos == PROPER_NOUN_TYPE)
            cout << PROPER_NOUN_TAG << endl;
        else 
            cout << word << endl;
    }
    pos_in.close();
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
