#include "WordNetOntology.h"

using namespace PLearn;

bool startsWith(string& base, string s);
int extractFiles(vector<string>& files, int n_files);
int convertPOS2WNO(string pos);

int main(int argc, char** argv)
{
  WordNetOntology ontology;
  set<string> pos_types;
  set<string> ignored_pos_types;
  string line;
  vector<string> files;
  int n_files = toint(argv[1]);
  int total_lines = extractFiles(files, n_files);
  ifstream input_if;
  ShellProgressBar progress(0, total_lines - 1, "building");
  progress.draw();
  int n = 0;
  ofstream output_of("out.test");
  int n_errors = 0;

  for (unsigned int i = 0; i < files.size(); i++)
  {
    input_if.open(files[i].c_str());

    cout << "processing file " << files[i] << endl;

    while (!input_if.eof())
    {
      getline(input_if, line, '\n');
      if (line == "") continue;
      progress.update(n++);
      vector<string> tokens = split(line, " <>");
      if (tokens[0] != "wf") continue;
      bool done = false;
      string pos = "";
      string lemma = "";
      int wnsn = -1;
      string lexsn = "";
      string word = "";
      int wno_wnsn = -1;
      int word_id = -1;
      bool tagged_word = false;

      for (unsigned int j = 0; j < tokens.size() - 2; j++)
      {
        string token = tokens[j];
        int ts = token.size();
        if (startsWith(token, "cmd="))
          done = (token.substr(4, ts) == "done");
        if (startsWith(token, "pos="))
          pos = token.substr(4, ts);
        if (startsWith(token, "lemma="))
          lemma = token.substr(6, ts);
        if (startsWith(token, "wnsn="))
          wnsn = toint(token.substr(5, ts));
        if (startsWith(token, "lexsn="))
          lexsn = token.substr(6, ts);
      }
      word = tokens[tokens.size() - 2];
      if (tokens[tokens.size() - 1] != "/wf") PLWARNING("no </wf>");

      string actual_word;
      if (lemma != "")
        actual_word = lemma;
      else if (word != "")
        actual_word = word;
      //else PLERROR("no lemma nor word");
      else continue; //ignoring entry

      //tagged_word = (pos != "" && wnsn != -1 && done);
      tagged_word = (pos != "" && lemma != "" && lexsn != "" && done);

      actual_word = lowerstring(actual_word);

      cout << actual_word << endl;

      if (!ontology.containsWord(actual_word))
      {
        ontology.extractWord(actual_word, ALL_TYPE, true, true, false);
      }

      word_id = ontology.getWordId(actual_word);
      if (tagged_word)
      {
        //wno_wnsn = ontology.getWordSenseIdForWnsn(actual_word, convertPOS2WNO(pos), wnsn);
        wno_wnsn = ontology.getWordSenseIdForSenseKey(lemma, lexsn);

        if (wno_wnsn == WNO_ERROR) 
        {
          wno_wnsn = -1;
          n_errors++;
/*
          PLWARNING("WNO_ERROR catched");
          cout << "lemma = " << lemma << endl;
          cout << "word = " << word << endl;
          cout << "actual_word = " << actual_word << endl;
          cout << "lexsn = " << lexsn << endl;
          cout << "line = " << line << endl;
*/
        }
      } else
      {
        wno_wnsn = -1;
      }

      if (tagged_word)
      {
        ignored_pos_types.insert(pos);
      } else 
      {
        pos_types.insert(pos);
      }

      output_of << word_id << " " << wno_wnsn << endl;
    
    }
    
    input_if.close();
  
  }

  progress.done();

  ontology.finalize();

  ontology.save("voc.test");
  ontology.save("s.test", "o.test");

  cout << n_errors << " errors" << endl;

  return 0;
}

bool startsWith(string& base, string s)
{
  if (base.size() < s.size()) return false;
  for (unsigned int i = 0; i < s.size(); i++)
  {
    if (base[i] != s[i]) return false;
  }
  return true;
}

int extractFiles(vector<string>& files, int n_files)
{
/*
  string file = "/u/jauvinc/wordnet-1.6/semcor/brown2/tagfiles/br-e26";
  files.push_back(file);
  return ShellProgressBar::getAsciiFileLineCount(file);
*/

  ifstream b("brown1.files");
  string line;
  int total_lines = 0;
  int fc = 0;
  while (!b.eof() && fc < n_files)
  {
    getline(b, line, '\n');
    if (line == "") continue;
    string file = "/u/jauvinc/wordnet-1.6/semcor/brown1/tagfiles/" + line;
    total_lines += ShellProgressBar::getAsciiFileLineCount(file);
    files.push_back(file);
    fc++;
  }
  b.close();
  b.open("brown2.files");
  while (!b.eof() && fc < n_files)
  {
    getline(b, line, '\n');
    if (line == "") continue;
    string file = "/u/jauvinc/wordnet-1.6/semcor/brown2/tagfiles/" + line;
    total_lines += ShellProgressBar::getAsciiFileLineCount(file);
    files.push_back(file);
    fc++;
  }
  
  cout << "retrieved " << fc << " files" << endl;

  return total_lines;
}

int convertPOS2WNO(string pos)
{
  // JJ NN NNP NNS RB VB VBD VBN
  if (pos == "NN" || pos == "NNP" || pos == "NNS") return NOUN_TYPE;
  else if (pos == "VB" || pos == "VBD" || pos == "VBN") return VERB_TYPE;
  else if (pos == "JJ") return ADJ_TYPE;
  else if (pos == "RB") return ADV_TYPE;
  else return UNDEFINED_TYPE;
}


