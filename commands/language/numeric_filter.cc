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
