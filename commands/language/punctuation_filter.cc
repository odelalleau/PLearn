#include <plearn/base/general.h>

using namespace PLearn;

// usage : punctuation_filter < in.text > filtered_out.text

bool isLetter(char c)
{
  return (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
}

bool isDigit(char c)
{
  return (c >= 48 && c <= 57);
}

bool isAlpha(char c)
{
  return (isLetter(c) || isDigit(c));
}

bool isPunctuation(string word)
{
  for (int i = 0; i < word.size(); i++)
  {
    if (isAlpha(word[i])) return false;
  }
  return true;
}

int main(int argc, char** argv)
{
  if (argc != 1)
    PLERROR("usage : punctuation_filter < in.text > filtered_out.text");
  string word;
  while (true)
  {
    cin >> word;
    if (!cin) break;
    if (isPunctuation(word))
      //cout << "<punctuation>" << " : " << word << endl;
      cout << "<punctuation>" << endl;
    else
      cout << word << endl;
  }
}

