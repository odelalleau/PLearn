#include "WordNetOntology.h"

using namespace PLearn;

int main(int argc, char** argv)
{
  if (argc != 3)
    PLERROR("usage : generate_ontology base_name differentiate_unknown_words");
  string base_name = tostring(argv[1]);
  string voc_file = base_name + ".voc";
  string synset_file = base_name + ".synsets";
  string ontology_file = base_name + ".ontology";
  bool diff_unk = tobool(argv[2]);
  WordNetOntology wno(voc_file, diff_unk, false, false);
  wno.save(synset_file, ontology_file);
  wno.printStats();
  return 0;
}
