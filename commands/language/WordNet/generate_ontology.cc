#include "WordNetOntology.h"

using namespace PLearn;

int main(int argc, char** argv)
{
  if (argc != 7)
    PLERROR("usage : generate_ontology {load | extract} voc synset ontology diff_unk print");
  bool load = (tostring(argv[1]) == "load");
  string voc_file = tostring(argv[2]);
  string synset_file = tostring(argv[3]);
  string ontology_file = tostring(argv[4]);
  bool diff_unk = tobool(argv[5]);
  bool print = tobool(argv[6]);
  int pos_type = 1005;
  WordNetOntology* ontology;
  if (load)
  {
    ontology = new WordNetOntology(voc_file, synset_file, ontology_file, false, true);
  } else
  {
    ontology = new WordNetOntology(voc_file, diff_unk, false, true, pos_type);
    ontology->save(synset_file, ontology_file);
  }

  ontology->detectWordsWithoutOntology();

  //ontology->extractWordHighLevelSenses(1, 1, 0, 0, 0);

  if (print)
    ontology->print(true);

  ontology->printStats();

  return 0;
}
