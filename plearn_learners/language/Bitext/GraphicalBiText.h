#include "Learner.h"
#include "WordNetOntology.h"
#include "random.h"
#include "TMat_maths.h"
#include <time.h>
#include "ProbSparseMatrix.h"
#include "SubVMatrix.h"
#include "/u/kermorvc/LisaPLearn/UserExp/larocheh/WSD/learning/TextSenseSequenceVMatrix.h"
#include "SelectColumnsVMatrix.h"
#include "DiskVMatrix.h"
#include "ConcatRowsVMatrix.h"
#include "SmoothedProbSparseMatrix.h"
#include "ProbVector.h"
#include "Splitter.h"


#define DISCOUNT_MASS 0.7
//#define PROB_PREC 0.0001 is defined in SmoothedProbMatrix
#define MAX_EM_IT 10
#define EM_THRES 0.001
#define INIT_ALPHA 0.80
#define INIT_P_A 0.1
#define DEF_INTERP 0.5

namespace PLearn {
using namespace std;

// Load from file into VMat
 VMat loadToVMat(string file,string name, int window, int n_examples);


class GraphicalBiText : public Learner {
  

  public :
  // Disambiguation model
  // window size used for disambiguation
  int  window_size;
  int n_train_examples; // number of train example in semcor
  int n_test_examples; // number of test example in semcor
  int n_epoch; // number of epoch in EM-graphical model learning algorithm
  string source_path; // path to the ontology
  string source_voc; // path to source vocabulary
  string target_voc; // path to target vocabulary
  string train_file; // Bitext training file
  string valid_file; // Bitext validation file
  string key_file; // key file for senseval test set
  string sensemap_file; // sensmap file for coarse sense 
  int sensemap_level; // level of sense grouping 1=all grouped 99 = all separated
  string semcor_train_path;// Path to semcor vmat
  string semcor_valid_path;// Path to semcor vmat
  string semcor_valid2_path;// Path to semcor vmat
  string semcor_test_path;// Path to semcor vmat
  string senseval2_train_path;// Path to Senseval2 train set VMat

  
  // Data 
  VMat wsd_train ;
  VMat wsd_valid ;
  VMat wsd_valid2;
  VMat wsd_test ;
  VMat senseval2_train;
    
  private :
    
    // Bitext Model
  // Sense table : P(S)
  ProbVector pS;
  // Probability mass in node c : P(C)
  ProbVector pMC;
  Vec pC;
  // Probability mass of the subtree rooted at c
  Vec pTC;
  // Probability of stoping in c
  Vec pA;
  Vec nA;

  // target_voc - Sense table :  P(F|S)
  ProbSparseMatrix nFS;
  ProbSparseMatrix pFS;
  // source_Voc - Sense table : P(E|S)
  ProbSparseMatrix nES;// Part of the graphical model
  ProbSparseMatrix pES;

  // Common nodes structure
  // this structure stores the deepest common nodes for each (source,taget) word couple
   TMat<Set> commNode;
   //TMat<map<int,int> > commNode;
  map<int, Set> sens_to_conceptAncestors;

  // target word -> senses map; the equivalent for the source words is in WordNet
  map<int, Set> target_word_to_senses;
  
  // Independant bitext model
  // target_voc proba
  ProbVector pF;
  // source_Voc proba
  ProbVector pE;
  
  // Joint probability bitext model P(E,F)
  ProbSparseMatrix nEF;
  ProbSparseMatrix pEF;
 
  // For Entropy computation
  ProbSparseMatrix nSE;
  ProbSparseMatrix pSE;
  ProbSparseMatrix nSEbi;
  ProbSparseMatrix pSEbi;
  Vec KL;
  // store if we should use bitext estimated model for this word 
   map<int,bool> BiSelect;

   //Sense mapping for coarse sense 
   map<string,string> sensemap;
   map <int,int> nodemap;
   map <int,int> node_level;


  // size of the input data for disambiguation (VMat)
  int n_fields;

  ProbVector pEbase;
  ProbVector pSbase;
  ProbVector pSupbi;
  Vec nS;
  // Context proba
  ProbVector pH;
  ProbVector pHbase;
  ProbVector pHupbi;

  ProbSparseMatrix nESbase;
  ProbSparseMatrix nESupbi;
  SmoothedProbSparseMatrix pESbase;// Estimated on Semcor
  SmoothedProbSparseMatrix pESupbi;
  
// context _Voc - Sense table : P(H|S)
  ProbSparseMatrix nHS;// Estimated on semcor
  SmoothedProbSparseMatrix pHS;
  ProbSparseMatrix nHSupbi;// Updated on bitexts
  SmoothedProbSparseMatrix pHSupbi;

  // Ontology
  WordNetOntology ontology;
  int source_wsd_voc_size;
  int sense_size;
  int ss_size;
  
  // Bitext 
  // Source Vocabulary
  map<int, string> source_id_to_word;
  map<string, int> source_word_to_id;
  int source_voc_size;

  // Target Vocabulary
  map<int, string> target_id_to_word;
  map<string, int> target_word_to_id;
  map<int,real> target_id_to_proba;
  int target_voc_size;

  Set target_wsd_voc;
  int target_wsd_voc_size;
  
  // Bitext Data  
  Vec train_bitext_tgt;
  Vec train_bitext_src;
  Vec valid_bitext_tgt;
  Vec valid_bitext_src;


  // Interpolation coefficients
  real alpha_bn;
  real alpha_joint;

  // Checksum variables
  Vec sum_epEC;
  Vec sum_fpFC;
  Vec sum_cpC;


  void compute_likelihood( Vec bitext_src, Vec bitext_tgt,string name, bool update);
  int getDeepestCommonAncestor(int s1, int s2);
  void compute_pTC();
  void compute_pTC(int word);
  void distribute_pS_on_ancestors(int s,real probaToDistribute);
  void compute_node_level();
  void compute_pMC();
  void check_set_pA();
  void printNode(int ss,ostream &out_hie);  
  void update_pWS(ProbSparseMatrix& , int , string);
  real compute_efs_likelihood(int e,int f, int se);
  real compute_BN_likelihood(int e,int f, bool update, real nb);
  void optimize_interp_parameter(Vec tgt,Vec src, string name);
  void loadBitext(string train_file_name,string valid_file_name, bool update_voc);
  void compute_nodemap(int split_level);
  void set_nodemap(int node,int word);
  void print_sensemap();
  void build_();
  void init_WSD();
  void init();
 public:
  
  GraphicalBiText();
  virtual ~GraphicalBiText();
  typedef Learner inherited;
  PLEARN_DECLARE_OBJECT(GraphicalBiText);
  
  
  static void declareOptions(OptionList& ol);
  void build();
   
  void use(const Vec& input, Vec& output) { PLERROR("NaiveBayes does not know 'use', only 'computeOutput'"); }
  void train(VMat training_set);
  void test();

  void train(int n_epoch);
  void senseTagBitext(string name);
  void check_consitency();
  void print(string name);
  void printHierarchy(string name);
  void update_WSD_model(string name);
  void sensetag_valid_bitext(string name);
  void computeKL();
  void loadSensemap(string sensemap_file);
  void compute_train_likelihood(string name);
  void compute_valid_likelihood(string name);

  void test_WSD(VMat  wsd_test, string name, TVec<string> v,bool select, real interp = DEF_INTERP);
  void  setTrainingSet(VMat training_set, bool call_forget);
};
} // end of namespace PLearn

