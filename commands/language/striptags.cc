#include <iostream.h>
#include <fstream.h>
#include <string>
#include <algorithm>
#include <plearn/base/general.h>
#include <plearn/base/stringutils.h>    //!< For split.

using namespace PLearn;

int main(int argc, char** argv)
{
#ifdef USE_EXCEPTIONS
    try {
#endif
    // TODO: Should be specifiable via the command line
    bool skip_dot_info = true;

    string line;
    while (!cin.eof()) {
        getline(cin, line, '\n');
        if (line == "")
            continue;
        vector<string> tokens = split(line, "/");
        if (tokens.size() == 2) {
            if ((skip_dot_info == false) || (tokens[0][0] != '.'))
                cout << tokens[0] << endl;
        } else {
            cout << "..." << endl;
        }
    }
#ifdef USE_EXCEPTIONS
    } catch (const PLearnError &e) {
        cout << "FATAL ERROR: " << e.message() << endl;
    }
#endif
    return 0;
}
