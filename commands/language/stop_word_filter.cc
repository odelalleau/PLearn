#include <plearn/base/general.h>
#include <plearn/base/stringutils.h>    //!< For pgetline.

using namespace PLearn;

// usage : stop_word_filter proper_nouns stop_words < in.text > filtered_out.text

set<string> extractWordSet(string file)
{
    set<string> words;
    ifstream in(file.c_str());
    while (!in.eof())
    {
        string line = pgetline(in);
        if (line == "") continue;
        words.insert(line);
    }
    in.close();
    return words;
}

int main(int argc, char** argv)
{
    if (argc != 2)
        PLERROR("usage : stop_word_filter stop_words < in.text > filtered_out.text");
    set<string> stop_words = extractWordSet(argv[1]);
    string word;
    while (true)
    {
        cin >> word;
        if (!cin) break;
        word = lowerstring(word);
        bool is_stop_word = stop_words.find(word) != stop_words.end();
        if (is_stop_word)
            cout << "<stop>" << endl;
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
