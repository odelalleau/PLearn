#include "general.h"
#include "WordNetOntology.h"
#include "TypesNumeriques.h"
#include "stringutils.h"      //!< For pgetline.

using namespace PLearn;

// usage : full_filter proper_nouns stop_words < in.text > filtered_out.text

set<string> proper_nouns;
set<string> stop_words;
WordNetOntology wn;

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

bool isPunctuation(string& word)
{
  for (unsigned int i = 0; i < word.size(); i++)
  {
    if (isAlpha(word[i])) 
      return false;
  }
  return true;
}

int main(int argc, char** argv)
{
  if (argc != 3)
    PLERROR("usage : full_filter proper_nouns stop_words < in.text > filtered_out.text");
  proper_nouns = extractWordSet(argv[1]);
  //stop_words = extractWordSet(argv[2]);
  string word;
  while (true)
  {
    cin >> word;
    if (!cin) break;
    word = lowerstring(word);
    /*if (stop_words.find(word) != stop_words.end())
      cout << STOP_TAG << endl;
      else*/
    if (looksNumeric(word.c_str()))
      cout << NUMERIC_TAG << endl;
    else if (isPunctuation(word))
      cout << PUNCTUATION_TAG << endl;
    else if ((proper_nouns.find(word) != proper_nouns.end()) && !wn.isInWordNet(word))
      cout << PROPER_NOUN_TAG << endl;
    else
      cout << word << endl;
  }
}
