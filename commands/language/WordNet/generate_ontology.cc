#include "WordNetOntology.h"

using namespace PLearn;

int main(int argc, char** argv)
{
//   if (argc < 9 || argc > 10)
//     PLERROR("usage : generate_ontology load|extract voc synset ontology diff_unk print pos_type polysemy_reduction_level [pos]");
//   bool load = (tostring(argv[1]) == "load");
//   string voc_file = tostring(argv[2]);
//   string synset_file = tostring(argv[3]);
//   string ontology_file = tostring(argv[4]);
//   bool diff_unk = tobool(argv[5]);
//   bool print = tobool(argv[6]);
//   int pos_type = toint(argv[7]);
//   int polysemy_reduction_level = toint(argv[8]);

  if (argc != 8)
    PLERROR("usage : generate_ontology {load | extract} voc synset ontology diff_unk print polysemy_reduction_level");
  bool load = (tostring(argv[1]) == "load");
  string voc_file = tostring(argv[2]);
  string synset_file = tostring(argv[3]);
  string ontology_file = tostring(argv[4]);
  bool diff_unk = tobool(argv[5]);
  bool print = tobool(argv[6]);
  int pos_type = 1005;
  int polysemy_reduction_level = toint(argv[7]);
//   string pos_file = "-";
//   if (argc == 10)
//     pos_file = tostring(argv[9]);
  if (load)
  {
    WordNetOntology ontology(voc_file, synset_file, ontology_file, false, true);
    ontology.printStats();
    ontology.detectWordsWithoutOntology();
    if (polysemy_reduction_level != -1)
    {
      ontology.reducePolysemy(polysemy_reduction_level);
      ontology.printStats();
      ontology.detectWordsWithoutOntology();
    }
//     if (pos_file != "-")
//       ontology.loadPredominentSyntacticClasses(pos_file);
    if (print)
      ontology.print(true);

    cout << "extracting word categories" << endl;
    ontology.extractWordCategoriesAtLevel(4, 2);

  } else
  {
    WordNetOntology ontology(voc_file, diff_unk, false, false, pos_type);
    ontology.printStats();
    ontology.detectWordsWithoutOntology();
//     if (pos_file != "-")
//       ontology.extractPredominentSyntacticClasses();
    if (polysemy_reduction_level != -1)
    {
      ontology.reducePolysemy(polysemy_reduction_level);
      ontology.printStats();
      ontology.detectWordsWithoutOntology();
    }
    //ontology.lookForSpecialTags();
    ontology.save(synset_file, ontology_file);
//     if (pos_file != "-")
//       ontology.savePredominentSyntacticClasses(pos_file);
    if (print)
      ontology.print(true);
  }
  return 0;
}
