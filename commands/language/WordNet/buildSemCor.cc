#include "WordNetOntology.h"

using namespace PLearn;

bool startsWith(string& base, string s);
int extractFiles(vector<string>& files, int n_files);
int convertPOS2WNO(string pos);

int main(int argc, char** argv)
{
  if (argc != 7)
    PLERROR("usage : n_files voc synsets ontology out.ttext out.bttext (argc = %d)", argc);
  WordNetOntology ontology;
  set<string> pos_types;
  set<string> ignored_pos_types;
  string line;
  vector<string> files;
  int n_files = toint(argv[1]);
  string voc_file = tostring(argv[2]);
  string synset_file = tostring(argv[3]);
  string ontology_file = tostring(argv[4]);
  int total_lines = extractFiles(files, n_files);
  ifstream input_if;
  ShellProgressBar progress(0, total_lines - 1, "building", 50);
  progress.draw();
  int n = 0;
  ofstream out_ttext(argv[5]);
  ofstream out_bttext(argv[6]);
  int n_unknown_errors = 0;
  int n_human_ambig_errors = 0;
  int n_tagged_words = 0;
  int n_non_tagged_words = 0;

  for (unsigned int i = 0; i < files.size(); i++)
  {
    input_if.open(files[i].c_str());

    //cout << "processing file " << files[i] << endl;

    while (!input_if.eof())
    {
      getline(input_if, line, '\n');
      if (line == "") continue;
      progress.update(n++);
      vector<string> tokens = split(line, " <>");
      bool bos_found = false;
      bool eos_found = false;
      if (tokens[0] == "s") bos_found = true;
      else if (tokens[0] == "/s") eos_found = true;
      else if (tokens[0] != "wf") continue;
      bool done = false;
      string pos = "";
      string lemma = "";
      string wnsn_str = "";
      int wnsn = -1;
      string lexsn = "";
      string word = "";
      int wno_wnsn = -1;
      int word_id = -1;
      bool tagged_word = false;

      if (tokens.size() > 2)
      {
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
          {
            wnsn_str = token.substr(5, ts);
            wnsn = toint(token.substr(5, ts));
          }
          if (startsWith(token, "lexsn="))
            lexsn = token.substr(6, ts);
        }
      }

      if (bos_found)
        word = BOS_TAG;
      else if (eos_found)
        word = EOS_TAG;
      else 
      {
        word = tokens[tokens.size() - 2];
        if (tokens[tokens.size() - 1] != "/wf") PLWARNING("no </wf>");
      }

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

      //cout << actual_word << endl;

      if (!ontology.containsWord(actual_word))
      {
        ontology.extractWord(actual_word, ALL_WN_TYPE, true, true, false);
      }

      bool human_ambig = ((wnsn_str.find(';', 0) != string::npos) && (lexsn.find(';', 0) != string::npos)); 

      word_id = ontology.getWordId(actual_word);
      if (tagged_word && !human_ambig)
      {
        //wno_wnsn = ontology.getWordSenseIdForWnsn(actual_word, convertPOS2WNO(pos), wnsn);
        wno_wnsn = ontology.getWordSenseIdForSenseKey(lemma, lexsn);

        if (wno_wnsn == WNO_ERROR) 
        {
          wno_wnsn = -1;
          n_unknown_errors++;
/*
          PLWARNING("WNO_ERROR catched");
          cout << "lemma = " << lemma << endl;
          cout << "word = " << word << endl;
          cout << "wnsn = " << wnsn << endl;
          cout << "wnsn_str = " << wnsn_str << endl;
          cout << "actual_word = " << actual_word << endl;
          cout << "lexsn = " << lexsn << endl;
          cout << "line = " << line << endl;
*/
        } else
        {
#ifdef CHECK
          Set senses = ontology.getWordSenses(word_id);
          if (!senses.contains(wno_wnsn))
            PLWARNING("weird");
#endif
        }
      } else
      {
//         Set senses = ontology.getWordSenses(word_id);
//         cout << ontology.getWord(word_id) << " : " << senses << endl;
        wno_wnsn = -1;
        if (human_ambig)
          n_human_ambig_errors++;
      }

      if (tagged_word)
      {
        ignored_pos_types.insert(pos);
      } else 
      {
        pos_types.insert(pos);
      }

      if (wnsn == -1) n_non_tagged_words++;
      else n_tagged_words++;

      out_ttext << ontology.getWord(word_id) << " " << word_id << " " << wno_wnsn << endl;
      binwrite(out_bttext, word_id);
      binwrite(out_bttext, wno_wnsn);
    
    }
    input_if.close();  
  }

  out_ttext.close();
  out_bttext.close();

  progress.done();

  ontology.save(voc_file);
  ontology.save(synset_file, ontology_file);

  cout << n_unknown_errors << " unknown errors" << endl;
  cout << n_human_ambig_errors << " human ambiguity errors" << endl;
  cout << n_tagged_words << " tagged words" << endl;
  cout << n_non_tagged_words << " non-tagged words" << endl;

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
  ifstream b("brown1.files");
  string line;
  int total_lines = 0;
  int fc = 0;
  while (!b.eof() && fc < n_files)
  {
    getline(b, line, '\n');
    if (line == "") continue;
    string file = "/u/jauvinc/wordnet-1.6/semcor/brown1/tagfiles/" + line;
    total_lines += ShellProgressBar::getWcAsciiFileLineCount(file);
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
    total_lines += ShellProgressBar::getWcAsciiFileLineCount(file);
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
