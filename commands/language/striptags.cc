#include <iostream.h>
#include <fstream.h>
#include <string>
#include <algorithm>
#include "general.h"

using namespace PLearn;

int main(int argc, char** argv)
{
  string line;
  while (!cin.eof())
  {
    getline(cin, line, '\n');
    if (line == "") continue;
    vector<string> tokens = split(line, "/");
    if (tokens.size() == 2)
    {
      cout << tokens[0] << endl;
    } else
    {
      cout << "..." << endl;
    }
  }
  return 0;
}
