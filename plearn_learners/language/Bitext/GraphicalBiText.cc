#include "GraphicalBiText.h"



namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(GraphicalBiText, "Probabilistically tag a bitext (english-other language) with senses from WordNet", "NO HELP");

GraphicalBiText::GraphicalBiText() // DEFAULT VALUES FOR ALL OPTIONS
  :
window_size(3),
n_epoch(5),
source_path("/u/larocheh/myUserExp/WSD/features/world3"),
semcor_train_path("/u/kermorvc/Data/Semcor/semcor1.7/1/train_corpus_all_wn17"),
semcor_valid_path("/u/kermorvc/Data/Semcor/semcor1.7/1/valid1_corpus_all_wn17"),
semcor_valid2_path("/u/kermorvc/Data/Semcor/semcor1.7/1/valid2_corpus_all_wn17"),
semcor_test_path("/u/kermorvc/Data/Semcor/semcor1.7/1/test_corpus_all_wn17"),
senseval2_train_path("/u/kermorvc/Data/Senseval/english-lex-sample/train/eng-lex_world3")
{

}

GraphicalBiText::~GraphicalBiText() 
{

}

void GraphicalBiText::declareOptions(OptionList& ol)
{
  declareOption(ol, "window_size", & GraphicalBiText::window_size, OptionBase::buildoption,
                "   size of the context window for disambiguation (same on the rigth side and on the left side)\n");
  declareOption(ol, "n_epoch", & GraphicalBiText::n_epoch, OptionBase::buildoption,
                "   number of iterations in the EM learning algorithm\n");
  declareOption(ol, "source_path", & GraphicalBiText::source_path, OptionBase::buildoption,
                "   path to the ontology\n");
  declareOption(ol, "source_voc", & GraphicalBiText::source_voc, OptionBase::buildoption,
                "   path to the source language vocabulary\n");
  declareOption(ol, "target_voc", & GraphicalBiText::target_voc, OptionBase::buildoption,
                "   path to the target language vocabulary\n");
  declareOption(ol, "train_file", & GraphicalBiText::train_file, OptionBase::buildoption,
                "   path to the bitext training file\n");
  declareOption(ol, "valid_file", & GraphicalBiText::valid_file, OptionBase::buildoption,
                "   path to the bitext validation file\n");
  declareOption(ol, "sensemap_file", & GraphicalBiText::sensemap_file, OptionBase::buildoption,
                "   path to the sensemap file  for coarse senses\n");
  declareOption(ol, "semcor_train_path", & GraphicalBiText::semcor_train_path, OptionBase::buildoption,
                "   path to the semcor training VMat file\n");
  declareOption(ol, "semcor_valid_path", & GraphicalBiText::semcor_valid_path, OptionBase::buildoption,
		"   path to the semcor validation VMat file\n");
  declareOption(ol, "semcor_valid2_path", & GraphicalBiText::semcor_valid2_path, OptionBase::buildoption,
		"   path to a second semcor validation VMat file\n");
  declareOption(ol, "semcor_test_path", & GraphicalBiText::semcor_test_path, OptionBase::buildoption,
                "   path to the semcor testing VMat file\n");
  
}

 void GraphicalBiText::build()
{
  inherited::build();
  build_();
  
}

 void GraphicalBiText::build_()
{

  // Used to read files
  string line;
  vector<string> tokens;
  string word;
  int id;
  SetIterator sit;


  alpha_bn= INIT_ALPHA;
  alpha_joint = INIT_ALPHA;

    // Load Ontology
  string wsd_voc = source_path+".voc";
  string synset_file = source_path+".synsets";
  string ontology_file = source_path+".ontology";
  string sense_key_file = source_path+".sense_key";

  ontology = WordNetOntology(wsd_voc, synset_file, ontology_file, sense_key_file,false, false);
  //  ontology = WordNetOntology(source_voc,1,1,1);
  // BOUH : dirty patch...
  ontology.fillTempWordToSensesTVecMap();



  // Warning : original sense and synset sizes are kept even if the ontology is pruned later
  // since these ss_id are used in VMat
  source_wsd_voc_size = ontology.getVocSize();
  source_voc_size =  ontology.getVocSize();
  sense_size = ontology.getSenseSize();
  ss_size = ontology.getMaxSynsetId() + 1;
  cout << "|original ontology voc| = " << source_wsd_voc_size << endl;
  cout << "|original sense| = " << sense_size << endl;
  cout << "|original synset| = " << ss_size << endl;

  // Store original  voc in internal source voc structure
  // This voc comes with the ontology
  Set all_words = ontology.getAllWords();
  for (sit = all_words.begin(); sit != all_words.end(); ++sit){
    word = ontology.getWord(*sit);
    id = ontology.getWordId(word);
    source_word_to_id[word]=id;
    source_id_to_word[id]=word;
  }

  cout << "| source voc | = "<< source_word_to_id.size()<< endl;
  //Load WSD language vocabulary
  // only these words are to be disambiguated
  ifstream if_voc(source_voc.c_str());
  if (!if_voc) PLERROR("can't open %s", source_voc.c_str());
  Set words_to_be_kept;
  int oov_id = ontology.getWordId(OOV_TAG);

  int wn_id;
  while (!if_voc.eof()){
    getline(if_voc, line, '\n');
     if (line == "") continue;
     if (line[0] == '#' && line[1] == '#') continue;
     tokens = split(line, " ");
     if (tokens.size() != 1) PLERROR("target vocabulary file format error (line = '%s')", line.c_str());
     wn_id = ontology.getWordId(tostring(tokens[0]));
     if (wn_id==oov_id && tostring(tokens[0])!=OOV_TAG){
       PLWARNING("word to disambiguate is not in the ontology %s", line.c_str());
     }else{
       words_to_be_kept.insert(wn_id);
     }
  }
  if_voc.close();
  // Remove unwanted words
  for (sit = all_words.begin(); sit != all_words.end(); ++sit){
    if( words_to_be_kept.find(*sit)== words_to_be_kept.end()){
      // remove
      ontology.removeWord(*sit);
    }
  }
  ontology.removeNonReachableSynsets(); 

  cout << "|pruned ontology voc| = " << ontology.getVocSize()<<endl;
  cout << "|pruned sense| = " <<  ontology.getSenseSize() << endl;
  cout << "|pruned synset| = " << ontology.getMaxSynsetId() + 1 << endl;
  
  
  //Load WSD target language vocabulary
  
  ifstream if_tvoc(target_voc.c_str());
  if (!if_tvoc) PLERROR("can't open %s", target_voc.c_str());
  int next_id=0;
  while (!if_tvoc.eof()) {
    getline(if_tvoc, line, '\n');
    if (line == "") continue;
    if (line[0] == '#' && line[1] == '#') continue;
    tokens = split(line, " ");
    if (tokens.size() != 1) PLERROR("target vocabulary file format error (line = '%s')", line.c_str());
    target_id_to_word[next_id]=tostring(tokens[0]);
    target_word_to_id[tostring(tokens[0])]=next_id;
    target_wsd_voc.insert(next_id);
    next_id++;
  }
  if_tvoc.close();
  // Add OV if necessary
  if (target_word_to_id.find(OOV_TAG)==target_word_to_id.end()){   
    target_word_to_id[OOV_TAG]=next_id;
    cout << " add OOV to target vocabulary " << endl;
    next_id++;
  }
  target_wsd_voc_size = target_wsd_voc.size();
  target_voc_size = target_wsd_voc_size;
  cout << "|WSD target voc| = " <<target_wsd_voc_size<<endl;
  
  
  loadBitext(train_file, valid_file,0);
  cout << "|target voc| = " <<  target_voc_size<<endl;
  cout << "|source voc| = " <<  source_voc_size<<endl;
  

  // common node structure allocation
  commNode.resize(source_wsd_voc_size,target_wsd_voc_size);


  
  // Probability matrices allocations 
  
  // Debuging variables
  sum_epEC.resize(ss_size);
  sum_fpFC.resize(ss_size);
  sum_cpC.resize(ss_size);
  
  pMC.resize(ss_size);
  pC.resize(ss_size);
  pTC.resize(ss_size);
  pA.resize(ss_size);
  nA.resize(ss_size);
  
  // Sense table : P(S)
  pS.resize(ss_size);
  pSbase.resize(ss_size);
  pSupbi.resize(ss_size);
  nS.resize(ss_size);
  // Src voc : P(E)
  pEbase.resize(source_wsd_voc_size);
  pE.resize(source_wsd_voc_size);
  // Context
  pH.resize(source_voc_size);
  pHbase.resize(source_voc_size);
  pHupbi.resize(source_voc_size);
  // Target vc : P(F)
  pF.resize(target_wsd_voc_size);

  // Graphical model variables
  // target_voc - Sense table :  P(F|S)
  nFS.resize(target_wsd_voc_size,ss_size);nFS.setName("nFS");nFS.setMode(COLUMN_WISE);
  pFS.resize(target_wsd_voc_size,ss_size);pFS.setName("pFS");pFS.setMode(COLUMN_WISE);

  // source_Voc - Sense table : P(E|S)
  nES.resize(source_wsd_voc_size,ss_size);nES.setName("nES");nES.setMode(COLUMN_WISE);
  pES.resize(source_wsd_voc_size,ss_size);pES.setName("pES");pES.setMode(COLUMN_WISE);


  // Entropy computation
  nSE.resize(ss_size,source_wsd_voc_size);nSE.setName("nSE");nSE.setMode(COLUMN_WISE);
  pSE.resize(ss_size,source_wsd_voc_size);pSE.setName("pSE");pSE.setMode(COLUMN_WISE);
  nSEbi.resize(ss_size,source_wsd_voc_size);nSEbi.setName("nSE");nSEbi.setMode(COLUMN_WISE);
  pSEbi.resize(ss_size,source_wsd_voc_size);pSEbi.setName("pSE");pSEbi.setMode(COLUMN_WISE);
  KL.resize(source_wsd_voc_size);
  BiSelect.clear();
    
  // NaiveBayes model
  pESbase.resize(source_wsd_voc_size,ss_size);pESbase.setName("pESbase");pESbase.setMode(COLUMN_WISE);
  pESupbi.resize(source_wsd_voc_size,ss_size);pESupbi.setName("pESupbi");pESupbi.setMode(COLUMN_WISE);
  nESbase.resize(source_wsd_voc_size,ss_size);nESbase.setName("nESbase");nESbase.setMode(COLUMN_WISE);
  nESupbi.resize(source_wsd_voc_size,ss_size);nESupbi.setName("nESupbi");nESupbi.setMode(COLUMN_WISE);
  
  // context - Sense table : P(H|S)
  nHS.resize(source_voc_size,ss_size);nHS.setName("nHS");nHS.setMode(COLUMN_WISE);
  pHS.resize(source_voc_size,ss_size);pHS.setName("pHS");pHS.setMode(COLUMN_WISE);
  nHSupbi.resize(source_voc_size,ss_size);nHSupbi.setName("nHSupbi");nHSupbi.setMode(COLUMN_WISE);
  pHSupbi.resize(source_voc_size,ss_size);pHSupbi.setName("pHSupbi");pHSupbi.setMode(COLUMN_WISE);

  pEF.resize(source_wsd_voc_size,target_wsd_voc_size);pEF.setMode(COLUMN_WISE);
  nEF.resize(source_wsd_voc_size,target_wsd_voc_size);nEF.setMode(COLUMN_WISE);


  init();
  init_WSD();
}

void GraphicalBiText::loadBitext(string train_file_name, string valid_file_name,bool update_voc)
{
  ifstream ifs1,ifs2;
  int if1_nb_lines=0;
  int if2_nb_lines=0;
  
  int nb_line;
  string line;
  vector<string> tokens;
  ShellProgressBar progress;
  string src_word,src_stem_word,tgt_word;
  int src_word_id,tgt_word_id;  
  int tgt_next_id =  target_voc_size;
  //  int src_next_id =  source_voc_size;

  // Train file
  ifs1.open(train_file_name.c_str());
  if (!ifs1) PLERROR("load_bitext : can't open %s", train_file_name.c_str());
  if1_nb_lines = ShellProgressBar::getAsciiFileLineCount(train_file_name);
  train_bitext_tgt.resize(if1_nb_lines);
  train_bitext_src.resize(if1_nb_lines);

  // Valid file
  ifs2.open(valid_file_name.c_str());
  if (!ifs2) PLERROR("load_bitext : can't open %s", valid_file_name.c_str());
  if2_nb_lines = ShellProgressBar::getAsciiFileLineCount(valid_file_name);
  valid_bitext_tgt.resize(if2_nb_lines);
  valid_bitext_src.resize(if2_nb_lines);
  
  // Load train
  progress.set(0, if1_nb_lines, "Loading "+train_file_name, 50);  
  progress.init();
  progress.draw();
  nb_line = 0;
  while (!ifs1.eof()) {
    getline(ifs1, line, '\n');
    if (line == "") continue;
    if (line[0] == '#' && line[1] == '#') continue;
    tokens = split(line, " ");
    // file must be in GIZA++ format : tgt_word src_word 
    if (tokens.size() != 2) PLERROR("format error : file %s (line = '%s')", train_file_name.c_str(),line.c_str());
    tgt_word = tostring(tokens[0]);
    src_word = tostring(tokens[1]);   
    if(update_voc){
      if (   target_word_to_id.find(tgt_word) ==  target_word_to_id.end()){
	target_id_to_word[tgt_next_id]=tgt_word;
	target_word_to_id[tgt_word]=tgt_next_id;
	tgt_word_id = tgt_next_id;
	tgt_next_id++;
      }else{
	tgt_word_id= target_word_to_id[tgt_word];
      }
      if (source_word_to_id.find(src_word) ==  source_word_to_id.end()){
	// Do not update src voc
	//source_id_to_word[src_next_id]=src_word;
	//source_word_to_id[src_word]=src_next_id;
	//src_word_id = src_next_id;
	//src_next_id++;
	src_word_id=source_word_to_id[OOV_TAG];
      }else{
	src_word_id=source_word_to_id[src_word];
      }
    }else{
      if ( target_word_to_id.find(tgt_word) ==  target_word_to_id.end()){
	tgt_word_id=target_word_to_id[OOV_TAG];
      }else{
	tgt_word_id= target_word_to_id[tgt_word];
      }
      if ( source_word_to_id.find(src_word) ==  source_word_to_id.end()){
	src_word_id=source_word_to_id[OOV_TAG];
      }else
	src_word_id=source_word_to_id[src_word];
    }
    train_bitext_tgt[nb_line]=tgt_word_id;
    train_bitext_src[nb_line]=src_word_id;
    nb_line++;
    progress.update(nb_line);
  }
  progress.done();
  if (update_voc){
    target_voc_size = tgt_next_id;
  }
  
  // do not update valid voc
  update_voc = false;

  // Load valid
  progress.set(0, if2_nb_lines, "Loading "+valid_file_name, 50);  
  progress.init();
  progress.draw();
  nb_line = 0;
  while (!ifs2.eof()) {
    getline(ifs2, line, '\n');
    if (line == "") continue;
    if (line[0] == '#' && line[1] == '#') continue;
    tokens = split(line, " ");
    // file must be in GIZA++ format : tgt_word src_word 
    if (tokens.size() != 2) PLERROR("format error : file %s (line = '%s')", valid_file_name.c_str(),line.c_str());
    tgt_word = tostring(tokens[0]);
    src_word = tostring(tokens[1]);   
    if(update_voc){
      if (   target_word_to_id.find(tgt_word) ==  target_word_to_id.end()){
	target_id_to_word[tgt_next_id]=tgt_word;
	target_word_to_id[tgt_word]=tgt_next_id;
	tgt_word_id = tgt_next_id;
	tgt_next_id++;
      }else{
	tgt_word_id= target_word_to_id[tgt_word];
      }
      if (source_word_to_id.find(src_word) ==  source_word_to_id.end()){
	// Do not update src voc
	//source_id_to_word[src_next_id]=src_word;
	//source_word_to_id[src_word]=src_next_id;
	//src_word_id = src_next_id;
	//src_next_id++;
	src_word_id=source_word_to_id[OOV_TAG];
      }else{
	src_word_id=source_word_to_id[src_word];
      }
    }else{
      if ( target_word_to_id.find(tgt_word) ==  target_word_to_id.end()){
	tgt_word_id=target_word_to_id[OOV_TAG];
      }else{
	tgt_word_id= target_word_to_id[tgt_word];
      }
      if ( source_word_to_id.find(src_word) ==  source_word_to_id.end()){
	src_word_id=source_word_to_id[OOV_TAG];
      }else
	src_word_id=source_word_to_id[src_word];
    }
    valid_bitext_tgt[nb_line]=tgt_word_id;
    valid_bitext_src[nb_line]=src_word_id;
    nb_line++;
    progress.update(nb_line);
  }
  if (update_voc){
    target_voc_size = tgt_next_id;
  }
  progress.done();
  


}

void GraphicalBiText::init_WSD()
{
  int i,e,s,si,pos,k,h;
  string skey;
  n_fields = 6 * window_size+3;
  int oov_id = ontology.getWordId(OOV_TAG);
  Vec row_data;
  row_data.resize(n_fields);
  for (i = 0; i < wsd_train.length(); i++){
    wsd_train->getRow(i, row_data);
    e = (int)row_data[n_fields-3];
    si = (int) row_data[n_fields-2];
    // map the sense
    s = si;
    skey = ontology.getSenseKey(e,si);   
    if (si>0 && sensemap.find(skey)!=sensemap.end())s=ontology.getSynsetIDForSenseKey(e, sensemap[skey]);
    pos = (int)row_data[n_fields-1];

    // only consider supervised examples and words in the disambiguation model
    if (e<0 || e == oov_id)continue;  
    if (s>0 && ontology.isWord(e)&&ontology.isSense(s)){
      if (pos!=NOUN_TYPE)continue;
      //if (pos!=VERB_TYPE)continue;
      // Naive Bayes model
      nESbase.incr(e,s);
      pSbase[s]++;
      pEbase[e]++;
   
      if(window_size!=0){
	// consider the context
	for (k = 0; k < 2 * window_size; k++){
	  h = (int)row_data[3*k];
	  if (h<0 || h == oov_id)continue;
	  nHS.incr(h,s);
	  pH[h]++;
	  pHbase[h]++;
	}
      }  
    }
  }
  
  // Naive Bayes model
  pEbase.smoothNormalize("pEbase");
  pESbase.normalizeCondBackoff(nESbase,0.1,pEbase,false,false);
  pESupbi.normalizeCondBackoff(nESbase,0.1,pEbase,false,false);
  pSbase.smoothNormalize("pSbase",0.1);
  
  
  if(window_size!=0){
    pH.smoothNormalize("pH");
    //pHS.normalizeCond(nHS,false);
    pHS.normalizeCondBackoff(nHS, 0.1,pH,false,false);
    pHSupbi.normalizeCondBackoff(nHS,0.1,pH,false,false);
    //pHSupbi.normalizeCond(nHS,false);
  }
  
}

VMat loadToVMat(string file,string name, int window, int n_examples)
{
  // open disk vmat
  VMat dvm = new DiskVMatrix(file);
  // extract a subset if wanted
  VMat sub_dvm = new SubVMatrix(dvm, 0, 0, (n_examples < 0 ? dvm->length() : n_examples) , dvm->width());
  // load into memory mat
  Mat m(sub_dvm.length(), sub_dvm.width());
  ShellProgressBar progress(0, m.length()-1, "Loading "+name, 50);
  progress.draw();
  for(int i=0; i<m.length(); i++){
    sub_dvm->getRow(i,m(i));
    progress.update(i);
  }
  progress.done();
  cout << m.length() << " lines found"<<endl;
  // transform int vmat
  VMat vm(m);
  VMat tvm = new TextSenseSequenceVMatrix(vm, 2*window);
  return tvm;
}


void GraphicalBiText::init()
{

  n_fields = 6 * window_size+3;
  string skey;
  string line;
  vector<string> tokens;
  string src_word,src_stem_word,tgt_word;
  int src_word_id,tgt_word_id;
  int c,s,si,e,f,i,j,k,h,pos;
  map <int,int> nb_translation;
  int oov_id = ontology.getWordId(OOV_TAG);
  int nbMap=0;
  Set src_senses,ss_anc;
  SetIterator sit,ssit; 
  Vec row_data;
  row_data.resize(n_fields);
  ShellProgressBar progress;
  float maxp=0;
  float p;
  int maxs=0;
  pES.clear();

  
  // Load SemCor
  wsd_train =  loadToVMat  (semcor_train_path,"Semcor_train",window_size,-1);
  //wsd_train =  loadToVMat (senseval2_train_path,"Senseval_train",window_size,-1);
  wsd_valid =  loadToVMat  (semcor_valid_path,"Semcor_valid",window_size,-1);
  wsd_valid2 =  loadToVMat  (semcor_valid2_path,"Semcor_valid2",window_size,-1);
   wsd_test =  loadToVMat  (semcor_test_path,"Semcor_test",window_size,-1);
   //wsd_test =  loadToVMat  (semcor_train_path,"Semcor_train",window_size,-1);
  // load Senseval2
  senseval2_train =  loadToVMat (senseval2_train_path,"Senseval_train",window_size,-1);

  TVec < set<int> > f_possible_senses(target_wsd_voc_size);
  Vec symscore(ss_size);
  cout << "|train| = " <<  wsd_train.length()<< endl;
  
  // Read two times : 1st to build sensemap, 2nd to initialize the model */
  for (i = 0; i < wsd_train.length(); i++){
    wsd_train->getRow(i, row_data);
    e = (int)row_data[n_fields-3];
    s = (int) row_data[n_fields-2];
    pos = (int)row_data[n_fields-1];
   
    // only consider supervised examples and words in the disambiguation model
    if (e<0 || e == oov_id)continue;  
    if (s>0 && ontology.isWord(e)&&ontology.isSense(s)){
      if (pos!=NOUN_TYPE)continue;
      //      if (pos!=VERB_TYPE)continue;
      nSE.incr(s,e);
      pS[s]++;
    }
  }
  pS.normalize();
  pSE.normalizeCond(nSE, false);

  sensemap.clear();
  if(sensemap_level>0)compute_nodemap(sensemap_level);
  //print_sensemap();

  pS.clear();
  nSE.clear();

  for (i = 0; i < wsd_train.length(); i++){
    wsd_train->getRow(i, row_data);
    e = (int)row_data[n_fields-3];
    si = (int) row_data[n_fields-2];
    // map the sense
    s = si;
    skey = ontology.getSenseKey(e,si);   
    if (sensemap_level>0 && si>0 && sensemap.find(skey)!=sensemap.end()){
      s=ontology.getSynsetIDForSenseKey(e, sensemap[skey]);
      nbMap++;
    }

    pos = (int)row_data[n_fields-1];
    // only consider supervised examples and words in the disambiguation model
    if (e<0 || e == oov_id)continue;  
    if (s>0 && ontology.isWord(e)&&ontology.isSense(s)){
      if (pos!=NOUN_TYPE)continue;
      //      if (pos!=VERB_TYPE)continue;
      nES.incr(e, s);
      pS[s]++;
      nSE.incr(s,e);
    }
  }
  cout <<"INIT "<<nbMap<<" mapping done"<<endl;
  // Normalize
  
  pES.normalizeCond(nES, false);
  pS.normalize();
  pSE.normalizeCond(nSE, false);

  // Compute pTC
  // must be after pS.normalize and before compute commNode
  compute_pTC();
  
  
  //Initialize  pEF and nFS
  // read from word train bitext file  
  progress.set(0, train_bitext_tgt.size(), "INIT_initialize_nFS_nEF", 50);  
  progress.init();
  progress.draw();
  
  for (i=0;i< train_bitext_tgt.size();i++){    
    tgt_word_id=(int)train_bitext_tgt[i];
    src_word_id=(int)train_bitext_src[i];
    
    // consider only words to be disambiguated
    if(ontology.isWord(src_word_id) && target_wsd_voc.find(tgt_word_id)!=target_wsd_voc.end()){
      
      // nEF
      nEF.incr(src_word_id,tgt_word_id);// update P(E,F)
      pE[src_word_id]++;
      pF[tgt_word_id]++;
      
      //nFS
      // The set of possible senses for F is the same as its translation E
      // P(F|S) is uniformly distributed
      // WARNING : change getWordNounSense after debug
      src_senses = ontology.getWordNounSenses(src_word_id);// see also further NOUN
      //src_senses = ontology.getWordVerbSenses(src_word_id);
      //      src_senses = ontology.getWordSenses(src_word_id);
      maxp=0;
      maxs=0;
      for (sit = src_senses.begin(); sit != src_senses.end(); ++sit){
	real src_sense_proba = pES.get(src_word_id,*sit);

	if (src_sense_proba!=0){
	// First solution
	// all the senses of e are possible for (e,f)
	
	// Uniform distribution
	  nFS.set(tgt_word_id,*sit,1);
	// Same distribution as pES
	//  nFS.incr(tgt_word_id,*sit,src_sense_proba);
	// update target word to senses map
	  target_word_to_senses[tgt_word_id].insert(*sit);
	

	// Second solution : sense selection according to similarity
	  //	  if(f_possible_senses[tgt_word_id].find(*sit)==f_possible_senses[tgt_word_id].end()){
	  // f_possible_senses[tgt_word_id].insert(*sit);
	  //}
	}
	// Third solution : consider only most probable senses of the translation
	// compute most likely sense
	//	p = pES(src_word_id,*sit)*pS[*sit];
	//if(maxp<p){
	// maxp = p;
	// maxs = *sit;
	//}
      }
      //f_possible_senses[tgt_word_id].insert(maxs);
    }
    progress.update(i);
  }
  progress.done();

  

  // Select f senses
  cout << "Init:attach french words"<<endl;
  compute_node_level();
  for ( f = 0; f<target_wsd_voc_size;f++){
    cout<<target_id_to_word[f]<<endl;
   
    if(nEF.sumCol(f)==1){
      // only one translation available for f
      // use most likely sense of this translation
      map<int, real>& col_f = nEF.getCol(f);
      map<int, real>::iterator it = col_f.begin();
      e = it->first;
      maxp=0;
      maxs=0;
      for(set<int>::iterator lit1=f_possible_senses[f].begin(); lit1 != f_possible_senses[f].end(); lit1++){
	s = *lit1;
	p = pES(e,s)*pS[s];
	if(maxp<p){
	  maxp = p;
	  maxs = i;
	}
      }
    }else{
      
      
      symscore.clear();
      for(set<int>::iterator lit1=f_possible_senses[f].begin(); lit1 != f_possible_senses[f].end(); lit1++){
	i = *lit1;
	for(set<int>::iterator lit2=f_possible_senses[f].begin(); lit2 != f_possible_senses[f].end(); lit2++){
	  j = *lit2;
	  if(i==j)continue;
	  c = getDeepestCommonAncestor(i,j);
	  symscore[i]+= -log(pTC[c]);
	  //symscore[i]+=node_level[c]+pTC[c];
	  //	cout <<" i="<<i<<" j="<<j<<" c="<<c<<" sym="<<-log(pTC[c])<<endl;
	  //	  cout <<" i="<<i<<" j="<<j<<" c="<<c<<" sym="<<node_level[c]+pTC[c]<<endl;
	}
      }
      int nb_fr_sense =10;
      for(i=0;i<nb_fr_sense;i++){
	if(symscore.size()!=0){
	  si = argmax(symscore);
	  symscore[si]=0;
	  if(si!=0){
	    cout <<target_id_to_word[f]<<" argmax="<<si<<" ";ontology.printSynset(si);
	    nFS.set(f,si,1);
	    // update target word to senses map
	    target_word_to_senses[f].insert(si);
	  }
	}
      }
    }
  }
  // Normalize
  pFS.normalizeCond(nFS, false);
   

  // Compute commNode
  int deepestComNode;
  int nb_commNode;
  Set e_senses;
  Set f_senses;
  progress.set(0,source_wsd_voc_size*target_wsd_voc_size , "INIT_compute_commNode", 50);  
  progress.init();
  progress.draw();
  i = 0;
  // For each source word
  Set e_words=ontology.getAllWords();
  for (sit = e_words.begin(); sit != e_words.end(); ++sit){
    e = *sit;
    
    e_senses =  ontology.getWordNounSenses(e);
    //e_senses =  ontology.getWordVerbSenses(e);

    // For each target word
    for ( f = 0; f<target_wsd_voc_size;f++){

      f_senses = target_word_to_senses[f];
      // For each sens of the current source word
      for(SetIterator  esit=e_senses.begin(); esit!=e_senses.end();++esit){
	if (pES.get(e,*esit)==0)continue;
	// For each sens of the current target word
	for (SetIterator fsit = f_senses.begin(); fsit != f_senses.end(); ++fsit){
	  if (pFS.get(f,*fsit)==0)continue;
	  deepestComNode = getDeepestCommonAncestor(*esit,*fsit);
	  //cout << "commNode "<< e << " " << *esit <<" " << f << " " << *fsit << " " <<deepestComNode <<endl;
	  commNode(e,f).insert(deepestComNode);
	  //	  nb_commNode =  commNode(e,f).size();commNode(e,f)[nb_commNode]=deepestComNode;
	  sens_to_conceptAncestors[*esit].insert(deepestComNode);
	  sens_to_conceptAncestors[*fsit].insert(deepestComNode);
	  if (pTC[deepestComNode]==0) PLERROR("compute_commNode : pTC[common ancestor]==0");
	  // Init pA
	  pA[deepestComNode]=  INIT_P_A;
        }
      }
      i++;
      progress.update(i);
      
    }
  }
  progress.done();
  
 
 
  check_set_pA();
  compute_pMC();
 
  // Joint model
  pEF.normalizeJoint(nEF);
  // Independant model
  pE.smoothNormalize("pE");
  pF.smoothNormalize("pF");


  check_consitency();
  
  return;
 
}
int GraphicalBiText::getDeepestCommonAncestor(int s1, int s2)
{
  list<int> candidates;
  int cand;
  Node* candNode;
  Node* ss2 ;
  SetIterator it;
  Set s1_ancestors;
  // WARNING : this interpretation of the ontology may not fit all applications
  // if a node is both a sense and a category
  // it is virtually split into two nodes : the sense node and the category node
  // the virtual category node is parent of the virtual sens node
  
  s1_ancestors = ontology.getSynsetAncestors ( s1,-1);
  // if s1 is not a pure sense
  if (pTC[s1]!=0){
    s1_ancestors.insert(s1);
  }
  // if s2 is not a pure sense
  if(pTC[s2]!=0){
    candidates.push_back(s2);
  }

  ss2 =  ontology.getSynset(s2);
  // add s2's parents to candidate list
  for (it = ss2->parents.begin(); it != ss2->parents.end(); ++it){
    candidates.push_back(*it);
  }
  // Breadth first search starting from s2 and going upward the ontology.
  while(!candidates.empty()){
  cand = candidates.front();
  candidates.pop_front();
    if (s1_ancestors.find(cand)!=s1_ancestors.end()){
      return cand;
    }else{
      candNode =  ontology.getSynset(cand);
      // add cand's parents to candidate list
      for (it = candNode->parents.begin(); it != candNode->parents.end(); ++it){
	candidates.push_back(*it);
      }
    }
  }
  PLERROR("No common ancestor for %d and %d",s1,s2);
  return 0;
}

/* Compute the probability of the subtree rooted at each node pTC */
/* pTC(c) = sum_{s\in subtree rooted c}P(s) */  
void GraphicalBiText::compute_pTC()
{
  SetIterator sit;
  pTC.clear();
  Set ss_set=ontology.getAllCategories();
  int s;
  // loop on all synsets which are not pure category
  for (sit = ss_set.begin(); sit != ss_set.end(); ++sit){
    s = *sit;
    if (ontology.isPureCategory(s))continue;
    if (ontology.isPureSense(s)){
      pTC[s]=0;
    }else{
      // this synset is both a sense and a category
      pTC[s]+=pS[s];
    }
    distribute_pS_on_ancestors(s,pS[s]);
  }
}


/* Compute the probability of the subtree rooted at each node pTC */
/* considering only the senses of a given word w                  */
/* pTC(c) = sum_{s of w \in subtree rooted c}P(s)                 */  
void GraphicalBiText::compute_pTC(int word)
{
  SetIterator sit;
  pTC.clear();
  Set w_senses ;
  Set ss_set=ontology.getAllCategories();
  int s;
  // loop on all synsets which are not pure category
  for (sit = ss_set.begin(); sit != ss_set.end(); ++sit){
    s = *sit;
    if (ontology.isPureCategory(s))continue;
    w_senses = ontology.getWordSenses(word);
    if(w_senses.find(s)==w_senses.end())continue;
    if (ontology.isPureSense(s)){
      pTC[s]=0;
    }else{
      // this synset is both a sense and a category
      pTC[s]+=pS[s];
    }
    distribute_pS_on_ancestors(s,pS[s]);
  }
}



void GraphicalBiText::distribute_pS_on_ancestors(int s,real probaToDistribute)
{
  real proba;
  Set ss_anc;
  SetIterator sit;
  ss_anc = ontology.getSynsetParents(s);
  // if the node has more than one parent, distribute equally the proba on each of them
  proba =  probaToDistribute/ss_anc.size();
  for ( sit = ss_anc.begin(); sit != ss_anc.end(); ++sit){
    pTC[*sit]+=proba;
    distribute_pS_on_ancestors(*sit,proba);
  }
}


// void GraphicalBiText::compute_pTC_old()
// {
//   real sum_cT;
//   SetIterator sit;
//   pTC.clear();
//   Set ss_set=ontology.getAllCategories();
//   // loop on all synsets
//   for (SetIterator ssit = ss_set.begin(); ssit != ss_set.end(); ++ssit){
//     if (ontology.isPureSense(*ssit)){
//        pTC[*ssit]=0;
//     }else {
//       sum_cT=0;
//       Set ss_desc;
//       Node* node = ontology.getSynset(*ssit);
//       ontology.extractStrictDescendants(node, ss_desc, Set());
//       //ontology.extractDescendants(node, ss_desc, Set());
//       // loop on all senses descendant of the current synset
//       for (sit = ss_desc.begin(); sit != ss_desc.end(); ++sit){
// 	if (ontology.isPureCategory(*sit)){
// 	  PLERROR("compute pTC : try to sum on non sense id ");
// 	}else{
// 	  // only senses have pS!=0
// 	  sum_cT+=pS[*sit];
// 	}
//       }
//       if (sum_cT!=0 && pS[*ssit]!=0)sum_cT+=pS[*ssit];
//       pTC[*ssit]=sum_cT;
//     }
//   }
// }
void GraphicalBiText::compute_node_level()
{
  list<int> desc;// descendant list
  SetIterator sit,ssit;
  Set ss_anc;
  Node *node,*node_par;
  bool incomplete;
  int s, max_level,par;
  node = ontology.getSynset(ROOT_SS_ID);
  // add children of the root to the list
  for (sit = node->children.begin(); sit != node->children.end(); ++sit){
    if (pTC[*sit]==0)continue;
    desc.push_back(*sit);
    //cout << " * " << *sit;
  }
  node_level[ROOT_SS_ID]=1;
  for(list<int>::iterator lit=desc.begin(); lit != desc.end(); lit++){
    s = *lit;
    
    if(pMC[s]!=0)continue;
    // no probability in the subtree : this part of the tree is useless
    if (pTC[s]==0)continue;
    // extract parents
    node =  ontology.getSynset(s);
    ss_anc.clear();
    ontology.    extractAncestors(node, ss_anc, 1, 1);
    max_level = 0;
    incomplete=0;
    for (ssit = ss_anc.begin(); ssit != ss_anc.end(); ++ssit){
      par = *ssit;
      if (node_level[par]==0){ PLWARNING("tried to compute level for a node (%d) and level for its parent (%d) is not computed",s,*ssit); 
      incomplete=true;
      break;
      }
      if (node_level[par]>max_level)max_level = node_level[par];
    }
    if(!incomplete){
      node_level[s]=max_level+1;
      node = ontology.getSynset(s);
      // add sense children of s to the list
      for (sit = node->children.begin(); sit != node->children.end(); ++sit){
	if (!ontology.isSynset(*sit))continue;
	desc.push_back(*sit);
	//cout << " * " << *sit;
      }
    }else{
      // will try later
      desc.push_back(s);
    }
  }
}

void GraphicalBiText::compute_pMC()
{
  list<int> desc;// descendant list
  SetIterator sit,ssit;
  Set ss_anc;
  Node *node,*node_par;
  bool incomplete;
  int s,par;
  real proba_mass;
  real sum_pTC_par;
  real check_sum=0;
  // erase previous values
  pMC.clear();
  pC.clear();
  node = ontology.getSynset(ROOT_SS_ID);
  // add children of the root to the list
  for (sit = node->children.begin(); sit != node->children.end(); ++sit){
    if (pTC[*sit]==0)continue;
    desc.push_back(*sit);
    //cout << " * " << *sit;
  }
  // Set pMC and pC for root
  pMC[ROOT_SS_ID]=1;
  pC[ROOT_SS_ID]=pMC[ROOT_SS_ID]*pA[ROOT_SS_ID];
  check_sum= pC[ROOT_SS_ID];
  for(list<int>::iterator lit=desc.begin(); lit != desc.end(); lit++){
    incomplete = false;
    s = *lit;
    //cout << " / " << s;
    // pMC already computed for this node
    // this is possible since the ontology is not a tree but a dag
    if(pMC[s]!=0)continue;
    // no probability in the subtree : this part of the tree is useless
    if (pTC[s]==0)continue;
    // extract parents
    node =  ontology.getSynset(s);
    ss_anc.clear();
    ontology.    extractAncestors(node, ss_anc, 1, 1);
    proba_mass = 0;
    for (ssit = ss_anc.begin(); ssit != ss_anc.end(); ++ssit){
      par = *ssit;
      if (pMC[par]==0){ PLWARNING("tried to compute pMC for a node (%d) and pMC for its parent (%d) is not computed",s,*ssit); ontology.printSynset(*ssit);incomplete=true;break;}
      // Compute sum_{children of parent}pTC(children)
      sum_pTC_par=0;
      node_par = ontology.getSynset(par);
      for (sit = node_par->children.begin(); sit != node_par->children.end(); ++sit){
	sum_pTC_par+=pTC[*sit];
      }
      proba_mass+=pMC[par]*(1.0-pA[par])*pTC[s]/sum_pTC_par;
      //cout << "(" << *ssit << ") " <<pMC[par]*(1.0-pA[par])*pTC[s]/sum_pTC_par << " + "; 
    }
    
    if (incomplete){
      // impossible to compute pMC now : will try later
      pMC[s]=0;
      desc.push_back(s);
      //cout << " * " << s;
    }else{
      node = ontology.getSynset(s);
      // add sense children of s to the list
      for (sit = node->children.begin(); sit != node->children.end(); ++sit){
	if (!ontology.isSynset(*sit))continue;
	desc.push_back(*sit);
	//cout << " * " << *sit;
      }
      pMC[s]=proba_mass;
      pC[s]=pMC[s]*pA[s];
      check_sum+=  pC[s];
      //if(pMC[s]==0)PLERROR("pMC[%d] = 0",s);
      //      cout <<" pMC[" << s<<"]="<<  pMC[s]<<;
    }  
  }
}

bool lessPair ( pair<int,float>& p1,  pair<int,float>& p2)
{
    return p1.second < p2.second;
}


void GraphicalBiText::set_nodemap(int c,int e)
/* go thru the subtree rooted inc           */
/* first seen sense is the common senses        */
/* for all other senses, map it to common sense */
{
  list <int> desc;
  SetIterator sit;
  int s;
  int common_sense=0;
  Node *node;
  Set e_senses = ontology.getWordSenses(e);
  desc.push_back(c);                                                                                                                      
  for(list<int>::iterator lit=desc.begin(); lit != desc.end(); lit++){                                                                             
    s = *lit;      
    if (e_senses.find(s)!=e_senses.end() && pSE(s,e)!=0 ){
      // the current node is both sense and category
      if(common_sense==0){
	// first sense encountered
	common_sense=s;
	sensemap[ontology.getSenseKey(e,s)]= ontology.getSenseKey(e,s); 

      }else{
	//	nodemap[s]=common_sense;
	sensemap[ontology.getSenseKey(e,s)]= ontology.getSenseKey(e,common_sense);     
      }
      cout <<  s<<" "<<pSE(s,e)<< " "<<ontology.getSenseKey(e,s) << " -> "<<  sensemap[ontology.getSenseKey(e,s)]<<endl;
    }
    node = ontology.getSynset(s);                                                                                                                 
    for (sit = node->children.begin(); sit != node->children.end(); ++sit){
      if (!ontology.isSynset(*sit))continue;
      if (pTC[*sit]==0 && (e_senses.find(*sit)==e_senses.end() || pSE(*sit,e)==0))continue; 
      desc.push_back(*sit);  
    }       
  }
}

void GraphicalBiText::print_sensemap()
{
  int e;
  SetIterator sit1,sit;
  cout << "Print_sensemap"<<endl;
 /* for each source word */
  Set e_words=ontology.getAllWords();
  for ( sit1 = e_words.begin(); sit1 != e_words.end(); ++sit1){
    e = *sit1;
    cout <<source_id_to_word[e]<<endl;
    Set e_senses = ontology.getWordSenses(e);
    for (sit = e_senses.begin(); sit != e_senses.end(); ++sit){
      //     if(nodemap.find(*sit)==nodemap.end()){
      //	sensemap[ontology.getSenseKey(e,*sit)]= ontology.getSenseKey(e,*sit);
      //}else{
      //sensemap[ontology.getSenseKey(e,*sit)]= ontology.getSenseKey(e,nodemap[*sit]);
      //}
      //cout <<  *sit<<" "<<pS[*sit]<< " "<<ontology.getSenseKey(e,*sit) << " -> "<<  sensemap[ontology.getSenseKey(e,*sit)]<<endl;
    }
  }
}

void GraphicalBiText::compute_nodemap(int level)
  /* Waning : compute_nodemap uses pTC and erase all previous values */
/* the parameter level defined the granularity of the sense map */
/* the greater the finer */
{
  list<int> desc;// descendant list
  SetIterator sit,ssit,sit1;
  Set ss_anc;
  Set e_senses;
  list<pair<int,float> > split_node;
  Node *node;
  int non_null_child;
  float max_level;
  map <int,float> split_level;
  int s,e;
  cout << "Compute_nodemap"<<endl;


  /* for each source word */
  Set e_words=ontology.getAllWords();
  for (sit1 = e_words.begin(); sit1 != e_words.end(); ++sit1){
    e = *sit1;
    //    cout <<source_id_to_word[e]<<endl;
    compute_pTC(e);
    e_senses = ontology.getWordSenses(e);
    nodemap.clear();
    split_level.clear();
    split_node.clear();
    desc.clear();
    desc.push_back(ROOT_SS_ID); 
    for(list<int>::iterator lit=desc.begin(); lit != desc.end(); lit++){
      s = *lit;
      node = ontology.getSynset(s);
 
      // if the current node is a sense, it is a split node
      if(e_senses.find(s)!=e_senses.end() && pSE(s,e)!=0){
	non_null_child=2;
	
      }else{
	non_null_child=0;
      
	for (sit = node->children.begin(); sit != node->children.end(); ++sit){
	  if (!ontology.isSynset(*sit))continue;
	  if (pTC[*sit]==0 && (pSE(*sit,e)==0 || e_senses.find(*sit)==e_senses.end()))continue;
	  desc.push_back(*sit);
	  non_null_child++;
	}
      }
      // compute the level of the parent
      if(s==ROOT_SS_ID){
	max_level=0;
      }else{
	// compute split_level = max_parent(split_level(parent))+1
	// get parents
	ss_anc.clear();
	max_level =0;
	ontology.extractAncestors(node, ss_anc, 1, 1);
	for (ssit = ss_anc.begin(); ssit != ss_anc.end(); ++ssit){
	  if (split_level[*ssit]>max_level)max_level = split_level[*ssit];
	}
      }
      if(non_null_child>=2){
	// the node is a split node
	split_level[s]=max_level+1.0;
	split_node.push_back(make_pair(s,max_level+1.0));
	//if(e_senses.find(s)!=e_senses.end() && pS[s]!=0){  
	// split_node.push_back(make_pair(s,max_level+2.0));
	//}  
      }else{
	// the node is not a split node
	split_level[s]=max_level;
	//if(e_senses.find(s)!=e_senses.end()){
	  // the current node is a sense
	// split_node.push_back(make_pair(s,max_level));
	//}
      }
      //cout <<s<<" " <<split_level[s]<<endl;
    }
    // Initialize sensemap
    for (sit = e_senses.begin(); sit != e_senses.end(); ++sit){
      sensemap[ontology.getSenseKey(e,*sit)]= ontology.getSenseKey(e,*sit); 
    }
    for(list<pair<int,float> >::iterator lit=split_node.begin(); lit != split_node.end(); lit++){
      //cout << lit->first << " " << lit->second<<endl;
      if(lit->second==level){
	set_nodemap(lit->first,e);
      }
    }
  }
}

void GraphicalBiText::check_set_pA()
{
  // Check wether pA = 1 when it should
  real sum_TC;
  SetIterator sit,ssit;
  Set ss_desc;
  Set ss_set=ontology.getAllCategories();
  Node* node,*childnode;
  // loop on all synsets
  for (sit = ss_set.begin(); sit != ss_set.end(); ++sit){
    // if the node is a pure sens, continue
    if(pTC[*sit]==0)continue;
    
    // compute the sum of the node's children probability
    sum_TC=0;
    node = ontology.getSynset(*sit);
    for (ssit = node->children.begin(); ssit != node->children.end(); ++ssit){
      // loop on all the direct descendant of the current synset
      //sum_TC+=pTC[*ssit];

    
      // this node is a both a category and a sens : it is considered as a virtual category
      if(pTC[*ssit]!=0 && pS[*ssit]!=0)continue;
      
      childnode =  ontology.getSynset(*ssit);
      // if a child node is shared between several parents, it contribute proportionnaly to 
      // it parent probability
      
      sum_TC+=pS[*ssit]/childnode->parents.size();
    }
    // if the node is a both a sense and a category, add its virtual sens children
    if (pTC[*sit]!=0 && pS[*sit]!=0) sum_TC+=pS[*sit];
    

    if (sum_TC!=0)    pA[*sit]=sum_TC/pTC[*sit];
      
 //    if(sum_TC==0){
//       if(pA[*sit]==0)PLERROR("in check_set_pA : loosing probability mass in node %d : pA was null and forced to 1",*sit);
//        pA[*sit]=1;
//     }else{
//       if(pA[*sit]==1)PLERROR("in check_set_pA : loosing probability mass in node %d : pTC!=0 but pA==1",*sit);
//       //pA[*sit]=sum_TC/pTC[*sit];
//     }
    //cout << " pA["<<*sit<<"]="<< pA[*sit]<< " pMC["<<*sit<<"]="<< pMC[*sit];
   
  }
}

void GraphicalBiText::check_consitency()
{
  cout << "Consistency checking :";
  cout << " / pS-1 : "<< sum(pS)-1.0;
  cout << " / pSbase-1 : "<< sum(pSbase)-1.0;
  cout << " / pMC : "<< sum(pMC);
  cout << " / pTC : "<< sum(pTC);
  cout << " / pA : "<< sum(pA);
  cout << " / pC-1 : "<< sum(pC)-1.0;
  cout << " / pF-1 : "<< sum(pF)-1.0;
  cout << " / pE-1 : "<< sum(pE)-1.0;
  cout << " / pH-1 : " << sum(pH)-1.0;
  cout << " / pHupbi-1 : " << sum(pHupbi)-1.0;
  cout << " / pFS : "<<pFS.checkCondProbIntegrity();
  cout << " / pES : "<<pES.checkCondProbIntegrity();
  cout << " / pHSupbi : "<<pHSupbi.checkCondProbIntegrity();
  cout << " / pHS : "<<pHS.checkCondProbIntegrity();
  cout << " / pEF-1 : "<<pEF.sumOfElements() - 1.0 <<endl;

}

void GraphicalBiText::print(string name)
{
  real proba;
  real like_sum=0;
  real efs_sum;
  int e,f,k,s;
  TVec<int> e_senses ;
  SetIterator sit;
  int e_voc_size = ontology.getVocSize();
  string filename = "out_gra"+name;
  ofstream out_gra (filename.c_str());
  if (!out_gra.is_open()){ PLERROR("error printing hierarchy");}

   
  ShellProgressBar progress(0,e_voc_size , "e_f_s_probabilities", 50);  
  progress.init();
  progress.draw();

  Set e_words=ontology.getAllWords();
  for (sit = e_words.begin(); sit != e_words.end(); ++sit){
    e = *sit;
    for ( f = 0; f<target_wsd_voc_size;f++){
      e_senses =  ontology.getSensesForWord(e);
      like_sum+=compute_BN_likelihood(e,f,0,1);
      efs_sum=0;
      for (k = 0; k < e_senses.size(); k++){
	s = e_senses[k];
	proba =  compute_efs_likelihood(e,f,s);
	efs_sum+=proba;
	out_gra <<target_id_to_word[f] << "\t"<<  source_id_to_word[e]<<"\t"<<proba << "\t"<<  ontology.getSenseKey(e,s);
	//	ontology.printSynset(s,out_gra);
      }
      if (efs_sum-1.0>PROB_PREC)PLERROR("print : efs doesn't sum to 1 for (%d,%d)",e,f);
    }
    progress.update(e);
  }
  progress.done();
  cout << " checksum likelihood-1.0 : " <<like_sum-1.0<< endl;
  Set ss_set=ontology.getAllCategories();
  //for (sit = ss_set.begin(); sit != ss_set.end(); ++sit){
  // if (sum_epEC[*sit]!=0)cout <<" epEC["<<*sit<<"]="<<sum_epEC[*sit];
  // if (sum_fpFC[*sit]!=0)cout <<" fpFC["<<*sit<<"]="<<sum_epEC[*sit];
  // }
}
void GraphicalBiText::printHierarchy(string name)
{
  string filename = "/u/kermorvc/HTML/Treebolic/hierarchy"+name+".xml";
  ofstream out_hie (filename.c_str());
  if (!out_hie.is_open()){ PLERROR("error printing hierarchy");}
  
  out_hie <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE treebolic SYSTEM \"Treebolic.dtd\">\n<treebolic toolbar=\"true\" focus-on-hover=\"false\"><tree orientation=\"radial\" expansion=\"0.9\" sweep=\"1.2\" backcolor=\"fffff0\" fontface=\"true\" fontsize=\"20\" fontsizestep=\"2\">\n";
  printNode(ROOT_SS_ID,out_hie);
  out_hie <<"<edges></edges></tree></treebolic>";
  
}

void GraphicalBiText::printNode(int ss,ostream &out_hie)
{
  SetIterator sit,ssit;
  Set word_set;
  int word;
  Node *  node = ontology.getSynset(ss);
  string color;
  if (pTC[ss]==0){
    color="cc0099";
  }else if (pC[ss]==0){
    color="99ffff";
  }else{
    color="0000ff";
  }
  out_hie << "<node id=\""<<ss<<"\" backcolor=\""<<color<<"\" forecolor=\"ffffff\">"<<endl;
  out_hie <<"<label>"<<node->syns<<"</label>"<<endl;
  out_hie<<"<content> pC="<<pC[ss]<<" pMC="<<pMC[ss]<<" pTC=" <<pTC[ss]<<" pA="<<pA[ss]<<" pS="<< pS[ss]<<" ss="<< ss<<node->gloss<<endl;
  out_hie <<"</content>";
  
  // if the node is both sens and category
  if(pS[ss]!=0 && pTC[ss]!=0){
    //create a virtual sens node
      out_hie << "<node id=\""<<ss<<"\" backcolor=\"ff33cc\" forecolor=\"ffffff\">"<<endl;
      out_hie <<"<label>"<<node->syns<<"</label>"<<endl;
      out_hie<<"<content> pS="<< pS[ss]<<" ss="<< ss<<node->gloss<<endl;
      out_hie <<"</content>";
  }    
  // Print Word Children
  word_set = ontology.getSynsetWordDescendants(ss);
  for (ssit = word_set.begin(); ssit != word_set.end(); ++ssit){
    word = *ssit;
    if (pES.get(word,ss)!=0){
      out_hie << "<node id=\"w"<<word<<"\" backcolor=\"ff9050\" forecolor=\"ffffff\">"<<endl;
      out_hie <<"<label> "<< source_id_to_word[word]<<"</label>"<<endl;
      out_hie<<"<content>"<<pES.get(word,ss)<<" id="<<word<<"</content>" <<endl;
      out_hie <<"</node>"<<endl;
    }
  }
  
  // Print Target word children
  for (ssit = target_wsd_voc.begin(); ssit != target_wsd_voc.end(); ++ssit){
    word = *ssit;
    if(pFS.get(word,ss)!=0){
      
      out_hie << "<node id=\"w"<<word<<"\" backcolor=\"00EE00\" forecolor=\"ffffff\">"<<endl;
      out_hie <<"<label> "<< target_id_to_word[word]<<"</label>"<<endl;
      out_hie<<"<content>"<<pFS.get(word,ss)<<" id="<< word <<"</content>" <<endl;
      out_hie <<"</node>"<<endl;
    }
    
  }
  // end of virtual node
  if(pS[ss]!=0 && pTC[ss]!=0){out_hie <<" </node>"<<endl; }
  
  for (sit = node->children.begin(); sit != node->children.end(); ++sit){
    // print only branches whose proba is not null
    if (pTC[*sit]!=0||pS[*sit]!=0){ 
      printNode(*sit,out_hie);
    }
  }
  out_hie <<" </node>"<<endl;
}


void GraphicalBiText::update_WSD_model(string name)
{
  TVec<int> e_senses;
  int i,j,k,h,e,f;
  real proba;
  int nbsent=0;
  SetIterator ssit;
  float thres;
  nHSupbi.clear();
  pHSupbi.clear();
  //Initialize nHSupbi with nHS
  for (int j = 0; j < nHS.getWidth(); j++){
    map<int, real>& col_j = nHS.getCol(j);
    for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
      if( it->second!=0)nHSupbi.set(it->first,j, it->second);
    }
  }
  nESupbi.clear();
  //Initialize nESupbi with nESbase
  for (int j = 0; j < nESbase.getWidth(); j++){
    map<int, real>& col_j = nESbase.getCol(j);
    for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
      if( it->second!=0)nESupbi.set(it->first,j, it->second);
    }
  }
  
  
  pHupbi.clear();
  pHupbi << pHbase;
  // "." denotes the end of the sentence
  int point_index = source_word_to_id[tostring(".")];
 
  string filename = "out_bi"+name;
  ofstream out_bi (filename.c_str());
  if (!out_bi.is_open()){ PLERROR("error while out_bi");}
  ShellProgressBar progress(0, train_bitext_src.size()- 1, "Updating_WSD_model ", 50);  
  progress.init();
  progress.draw();
  
  for(i=0;i<train_bitext_src.size()-1;i++){
    e = (int)train_bitext_src[i];
    f = (int)train_bitext_tgt[i];
    if(e==point_index){
      nbsent++;
      continue;
    }
    // if the current word is to be disambiguated
    // and its translation is considered
    if (ontology.isWord(e)&& target_wsd_voc.find(f)!=target_wsd_voc.end()){
      e_senses = ontology.getSensesForWord(e);
      for (k = 0; k < e_senses.size(); k++){
	int s = e_senses[k];
	// Compute P(s|e,f)
	proba =  compute_efs_likelihood(e,f,s);
	
	out_bi <<target_id_to_word[f] << "\t"<<  source_id_to_word[e]<<"\t"<<sensemap[ontology.getSenseKey(e,s)]<<"\t"<<proba << endl;
	if(proba!=0){
	  // update context proba forward
	  for(j=1;j<=window_size;j++){
	    h = (int)train_bitext_src[i+j];
	    if(h==point_index)break;
	    //update context proba
	    pHupbi[h]++;
	    nHSupbi.incr(h,s,proba);
	  }
	  // update context proba backward
	  for(j=1;j<=window_size;j++){
	    h = (int)train_bitext_src[i-j];
	    if(h==point_index)break;
	    //update context proba
	    pHupbi[h]++;
	    nHSupbi.incr(h,s,proba);
	  }
	}
      }
    }else{
      out_bi <<target_id_to_word[f] << "\t"<<  source_id_to_word[e]<<endl;
    }
    
    progress.update(i);
  }
  progress.done();
  cout<< "Updating WSD model  : "<< nbsent<<" sentences processed" <<endl;
  // Normalize
  pHupbi.smoothNormalize("pHupbi");
  pHSupbi.normalizeCondBackoff(nHSupbi, 0.1,pHupbi,false,false);
  pESupbi.normalizeCondBackoff(nESupbi, 0.1,pEbase,false,false);
}

void GraphicalBiText::senseTagBitext(string name)
{
  TVec<int> e_senses;
  int i,j,k,ie,maxs,e,f,wb,we;
  real proba=0,ps;
  int sent_b,sent_e;
  int nbsent=0;
  SetIterator ssit;
  
  // "." denotes the end of the sentence
  int point_index = source_word_to_id[tostring(".")];
  sent_b = 0;
  sent_e = 0;
  i =0;

  // open out file
  string filename = "out_bi"+name;
  ofstream out_bi (filename.c_str());
  if (!out_bi.is_open()){ PLERROR("error while out_bi");}

  ShellProgressBar progress(0, train_bitext_src.size()- 1, "Updating_WSD_model ", 50);  
  progress.init();
  progress.draw();
  
  
  for(i=0;i<train_bitext_src.size();i++){
    e = (int)train_bitext_src[ie];
    f = (int)train_bitext_tgt[ie];
    
    out_bi <<target_id_to_word[f] << "\t"<<  source_id_to_word[e]<<endl;

    
    if (ontology.isWord( (int)train_bitext_src[i])&& target_wsd_voc.find((int)train_bitext_tgt[i])!=target_wsd_voc.end()){
      e_senses = ontology.getSensesForWord((int)train_bitext_src[i]);
      for (k = 0; k < e_senses.size(); k++){
	int s = e_senses[k];
	// Compute P(s|e,f)
	proba =  compute_efs_likelihood(e,f,s);
	out_bi <<target_id_to_word[f] << "\t"<<  source_id_to_word[e]<<"\t"<<proba << "\t"<<  ontology.getSenseKey(e,s)<<"\t"<<s<< endl; 
      }
    }
    progress.update(i);
  }
  progress.done();
  
}


void GraphicalBiText::sensetag_valid_bitext(string name)
{
  TVec<int> e_senses;
  int i,k,maxs,e,f;
  real proba=0,ps;
  
  string filename = "out_bi"+name;
  ofstream out_bi (filename.c_str());
  if (!out_bi.is_open()){ PLERROR("error while out_bi");}
  
  ShellProgressBar progress(0, valid_bitext_src.size()- 1, "Sensetag_valid_bitext ", 50);  
  progress.init();
  progress.draw();
  
  for (i=0;i<valid_bitext_src.size();i++){
    e = (int)valid_bitext_src[i];
    f = (int)valid_bitext_tgt[i];
    
    if (ontology.isWord(e)&& target_wsd_voc.find(f)!=target_wsd_voc.end()){
      maxs = -1;
      ps = 0;
      e_senses = ontology.getSensesForWord(e);
      for (k = 0; k < e_senses.size(); k++){
	int s = e_senses[k];
	// Compute P(s|e,f)
	proba =  compute_efs_likelihood(e,f,s); 
	//out_bi <<target_id_to_word[f] << "\t"<<  source_id_to_word[e]<<"\t"<<proba << "\t"<<  ontology.getSenseKey(e,s); 
	//ontology.printSynset(s,out_bi);
	if (proba>ps){
	  ps = proba;
	  maxs = s;
	}
	
      }
      out_bi <<target_id_to_word[f] << "\t"<<  source_id_to_word[e]<<"\t"<<ps << endl;//"\t"<<  ontology.getSenseKey(e,maxs)<<endl;
      progress.update(i);
    }
  }
  progress.done();
}


real GraphicalBiText::compute_efs_likelihood(int e,int f, int se)
{
  
  int s,c,i;
  real pws;
  real post;
  real like=0;
  Vec peC;// value of c node modified  src data
  Vec pfC;// value of c node modified  tgt data
  Set ss_anc;
  SetIterator sit,ssit;
  Set ss_adm; // admissible nodes
  set <int>ss_admAnc   ; // admissible ancestors
  Set synsets;


  peC.resize(ss_size);
  pfC.resize(ss_size);

  // Compute likelihood 
  synsets=ontology.getAllCategories();
  //synsets  = ontology.getWordSenses(e);
  for (sit = synsets.begin(); sit != synsets.end(); ++sit){
    s = *sit;
    pws = pES.get(e,s);
    if (pws!=0){
      ss_anc = ontology.getSynsetAncestors(s);
      // if s is not a pure sense add it to its own ancestors
      if (pTC[s]!=0){
	ss_anc.insert(s);
      } 
      // loop on all ancesters
      for ( ssit = ss_anc.begin(); ssit != ss_anc.end(); ++ssit){// go upward following sense ancestors  
	c = *ssit;
	peC[c]+=pws*pS[s];
      }
    }
  }
  
  synsets  = target_word_to_senses[f];
  for (sit = synsets.begin(); sit != synsets.end(); ++sit){
    s = *sit;
    pws = pFS.get(f,s);
    if (pws!=0){
      ss_anc = ontology.getSynsetAncestors(s);
       // if s is not a pure sense
      if (pTC[s]!=0){
	ss_anc.insert(s);
      } 
      
      // loop on all ancesters
      for ( ssit = ss_anc.begin(); ssit != ss_anc.end(); ++ssit){// go upward following sense ancestors  
	c = *ssit;
	pfC[c]+=pws*pS[s];
      }
    }
  }
  ss_adm =  commNode(e,f);
  for( ssit = ss_adm.begin();ssit != ss_adm.end();++ssit){
    c = *ssit;
    //for(i=0;i< commNode(e,f).size();i++){
    //c = commNode(e,f)[i];
    if (peC[c]!=0){
      if (pTC[c]==0){PLERROR("compute_BN_likelihood : division by zero leC/pTC");}
      peC[c]/=pTC[c];
    }
  }
  for( ssit = ss_adm.begin();ssit != ss_adm.end();++ssit){
    c = *ssit;
    //for(i=0;i< commNode(e,f).size();i++){
    //c = commNode(e,f)[i];
    if (pfC[c]!=0){
      if (pTC[c]==0){PLERROR("compute_BN_likelihood : division by zero lfC/pTC");}
      pfC[c]/=pTC[c];
    }
  }
  
  for( ssit = ss_adm.begin();ssit != ss_adm.end();++ssit){
    c = *ssit;
    //for(i=0;i< commNode(e,f).size();i++){
    // c = commNode(e,f)[i];
    //   cout <<" esf sl "<<c<< " " << peC[c]<<" " <<pfC[c]<<" " <<pC[c];
    like+=peC[c]*pfC[c]*pC[c];
  }
  

  // Compute Posterior P(S=se|E=e,F=f)
  post=0;
  if (like!=0){  
    ss_anc = ontology.getSynsetAncestors(se);
    // if se is not a pure sense
    if (pTC[se]!=0){
      ss_anc.insert(se);
    } 
    ss_adm =  commNode(e,f);
    set_intersection(ss_anc.begin(),ss_anc.end(),ss_adm.begin(),ss_adm.end(),inserter( ss_admAnc, ss_admAnc.begin() ));
    pws = pES.get(e,se);
    if (pws!=0){
      // loop on all admissible ancestors
      for ( ssit = ss_admAnc.begin(); ssit != ss_admAnc.end(); ++ssit){// go upward following sense ancestors  
	c = *ssit;
	//for (i=0;i< commNode(e,f).size();i++){
	//c = commNode(e,f)[i];
	if(ss_anc.find(c)==ss_anc.end())continue;
	post += pC[c]*pws*pS[se]/pTC[c]*pfC[c]/like;
	//cout <<" esf post "<<c<<" pC=" << pC[c]<<" pES="<<pws<<" ps="<<pS[se]<<" pTC="<<pTC[c]<<" pfC=" <<pfC[c]<<" p="<<pC[c]*pws*pS[se]/pTC[c]*pfC[c]/like<<" cum="<<post<<endl;
      }
    }
  }
  //cout <<" posterior "<<source_id_to_word[e]<<" "<<target_id_to_word[f]<<" " <<se<<" = "<<post <<" like "<<like<<endl;
  return post;
}
void GraphicalBiText::test_WSD(VMat test_set, string name, TVec<string> v,bool select, real interp)
{
  
  
  int e,s,target,pos,smax,smaxb,smaxs,h;
  real nb_supervised=0;
  real nb_correct=0;
  real nb_single=0;
  real nb_unknown=0;
  real nb_undef=0;
  real nb_correctb=0;
  real nb_undefb=0;
  real nb_corrects=0;
  real nb_correctrandom=0;
  real nb_correctwn=0;
    real nb_undefs=0;
  real max,maxb,maxs,p,pupbi,ps,q,qb;
  int nbMap=0;

  // Vec for detailed scores
  Vec dMatch( source_wsd_voc_size);
  Vec dMatchBi(source_wsd_voc_size);
  Vec dMatchStup(source_wsd_voc_size);
  Vec dNumber(source_wsd_voc_size);
  if(!select){
    BiSelect.clear();
  }
  if(select)cout <<"WSD_number_BiSelected "<<BiSelect.size()<<endl;
  

  Set source_words;
  SetIterator ssit;


  real context_coeff;
  TVec<int> e_senses;
  int e_senses_size;
  int oov_id = ontology.getWordId(OOV_TAG);
  string skey;
  int i,j,k;
  ShellProgressBar progress;
  
  string diff;
  int test_set_size = test_set->length();
  cout << "WSD_"+name+" size  = " << test_set_size << endl;
  

  progress.set(0,  test_set_size, "Predict "+name+" senses", 50);
  progress.init(); 
  progress.draw();  
#ifdef PRINT_WSD 
  string filename = "out_wsd"+name;
  ofstream out_wsd (filename.c_str());
  if (!out_wsd.is_open()){ PLERROR("error while opening out_wsd");}
#endif
  Vec row_data;
  row_data.resize(n_fields);
  for (i = 0; i < test_set_size; i++){
    // get data for one example from test set
    test_set->getRow(i, row_data);
    if (row_data.size() != n_fields) PLERROR("row_data[%d].size = %d, but n_fields = %d", i, row_data.size(), n_fields);
    e = (int)row_data[n_fields-3];
#ifdef PRINT_WSD 
    out_wsd <<source_id_to_word[e]<<" ";
#endif
    // consider only words in the ontology vocabulary.
    if (!ontology.isWord(e))continue; 
    
    s = (int) row_data[n_fields-2];
    // map the sense
    skey = ontology.getSenseKey(e,s);
    if (sensemap_level>0 && s>0 && sensemap.find(skey)!=sensemap.end()){
      nbMap++;
      target=ontology.getSynsetIDForSenseKey(e,sensemap[skey]);
      //cout << "mapping"<<s <<" "<<skey<<" " << sensemap[skey]<<" " <<ontology.getSynsetIDForSenseKey(e, sensemap[skey])<<endl;
    }else{
      target = s;
    }
    pos = (int)row_data[n_fields-1];
    if (pos!=NOUN_TYPE)continue;
#ifdef PRINT_WSD 
    out_wsd <<" tar="<<target<<" pos="<<pos<<endl;
#endif


    if (target>=0){
      //out_wsd <<source_id_to_word[e] <<" ts="<<target<<" "<<pos;
      // Reduce the number of possible senses using POS
      if (1){
	switch (pos){
	case NOUN_TYPE:
	  e_senses = ontology.temp_word_to_noun_senses[e];
	  break;
	case VERB_TYPE:
	  e_senses = ontology.temp_word_to_verb_senses[e];
	  break;
	case ADJ_TYPE:
	  e_senses = ontology.temp_word_to_adj_senses[e];
	  break;
	case ADV_TYPE:
	  e_senses = ontology.temp_word_to_adv_senses[e];
	  break;
	case UNDEFINED_TYPE:
	  e_senses = ontology.getSensesForWord(e);
	  break;
	default:
	  PLERROR("weird in train, pos = %d", pos);
	}
      } else{
	e_senses = ontology.getSensesForWord(e);
      }
      e_senses_size = e_senses.size();
      if (e_senses_size==0){
	// trying  to disambiguate an unknown word
	nb_unknown ++;
	v[(int)nb_supervised] = "-1";
	nb_supervised++;
	continue;
      }
      
      
      if (e_senses_size==1){
	nb_single++;
	v[(int)nb_supervised] = ontology.getSenseKey(e,e_senses[0] );
	dNumber[e]++;
	
	nb_supervised++;
	continue;
      }

      // Real polysemous case
      maxb = -FLT_MAX;
      max=-FLT_MAX;
      maxs=maxb;
      smax=-1;
      smaxb=-1;
      smaxs = smaxb;
      
      for (j = 0; j < e_senses_size; j++){
	int s = e_senses[j];
	p = log(pESbase.get(e,s))+log(pSbase[s]);
	pupbi = p;
	ps =p;
#ifdef PRINT_WSD 
	out_wsd << "pES="<<pES.get(e,s)<<" pS="<<pSbase[s];//ontology.printSynset(s,out_wsd);      
#endif
	if(window_size!=0){
	  // Context coefficient : weight the influence of the context
	  context_coeff = 1.0/(2*window_size);
	  // consider the context
	  for (k = 0; k < 2 * window_size; k++){
	    h = (int)row_data[3*k];
#ifdef PRINT_WSD 
	    out_wsd <<"/"<< source_id_to_word[h];
#endif
	    if (h==oov_id)continue;
	    // Default naive bayes
	    q=pHS.get(h,s);
	    qb=pHSupbi.get(h,s);
	    if(qb>1)PLERROR("qb>1 %f",qb);
	    // if (q!=0 && !isnan(q)){
	    p += context_coeff*(log(q));
	    //}
	    pupbi +=context_coeff*(interp*log(qb)+(1.0-interp)*log(q));
#ifdef PRINT_WSD 
	    out_wsd <<","<<q<<","<<qb;
#endif	   
	  }
	}
#ifdef PRINT_WSD 
	out_wsd << " s="<< s <<" p="<<p<<" pupbi="<<pupbi<<endl;
#endif
	if (p>max){max=p;smax = s;}
	if (pupbi>maxb){maxb=pupbi;smaxb = s;}
	if (ps>maxs){maxs=ps;smaxs = s;}
      }
      //out_wsd <<endl;

      // Naive Bayes
      if (max==-FLT_MAX){
	nb_undef++;
	// No sense predicted : use first in ontology (a kind of more likely a priori)
	smax =  e_senses[0];
      }
      if (target==smax){
	nb_correct++;
	dMatch[e]++;
      }

      // Stupid Bayes
      if (maxs==-FLT_MAX){
   	nb_undefs++;
	smaxs =  e_senses[0];
      }
      if (target==smaxs){
	nb_corrects++;
	dMatchStup[e]++;
      }
      // StupidWordNet
      smaxs =  e_senses[0];
      if (target==smaxs){
	nb_correctwn++;
      }
      // Random
      smaxs = e_senses[(int)floor(rand()/(RAND_MAX+1.0)*(float)e_senses.size())];
     
      //     smaxs = floor(bounded_uniform(0,e_senses.size()));
      if (target==smaxs){
	nb_correctrandom++;
      }
      // Bitext 
      if (maxb==-FLT_MAX){
	nb_undefb++;
	// No sense predicted : use first in ontology (a kind of more likely a priori)
	smaxb =  e_senses[0];
      }
      // Use model selection
      if (select){
	if(BiSelect.find(e)==BiSelect.end())smaxb = smax;

      }
      if (target==smaxb){
	nb_correctb++;
	dMatchBi[e]++;
      }
      v[(int)nb_supervised] = ontology.getSenseKey(e, smaxb);
#ifdef PRINT_WSD 
      out_wsd <<" best " <<source_id_to_word[e]<< " e=" << e <<" tar="<<target<<" hyp="<<smaxb<<" "<< ontology.getSenseKey(e, smaxb)<<endl;
#endif      
      dNumber[e]++;
      nb_supervised++;
    
    }
#ifdef PRINT_WSD 
    out_wsd << endl;
#endif   
    progress.update(i);
  }
  progress.done();
  
#ifdef PRINT_WSD 
  // open out_answers file
   filename = "out_score_"+name;
  ofstream out_score (filename.c_str());
  if (!out_score.is_open()){ PLERROR("error while opening out_score");}
  
  source_words = ontology.getAllWords();
  for (ssit = source_words.begin(); ssit != source_words.end(); ++ssit){
    e = *ssit;
    if (dNumber[e]==0)continue;
    if(dMatch[e]<dMatchBi[e]){diff="+";}else{diff="-";}
    out_score <<diff<<"\t"<<source_id_to_word[e]<<"\t"<<dNumber[e]<<"\t"<<dMatch[e]<<"\t"<<dMatchBi[e]<<"\t"<<dMatchStup[e]<<endl;
    if(!select && dMatch[e]<dMatchBi[e])BiSelect[e]=true;
  }
  out_score.close();
#endif     

  cout <<"WSD "<<nbMap<<" mapping done"<<endl;
  cout <<"WSD "+name+" Random correct :"<<nb_correctrandom<<" / "<<nb_supervised<< " = " << nb_correctrandom/nb_supervised*100 <<endl;
  cout <<"WSD "+name+" StupidWordNet correct :"<<nb_correctwn<<" / "<<nb_supervised<< " = " << nb_correctwn/nb_supervised*100   <<endl;
  cout <<"WSD "+name+" StupidBayes correct :"<<nb_corrects<<" / "<<nb_supervised<< " = " << nb_corrects/nb_supervised*100  << " % - " << nb_undefs << " undefined"  <<endl;
  cout <<"WSD "+name+" NaiveBayes correct :"<<nb_correct<<" / "<<nb_supervised<< " = " << nb_correct/nb_supervised*100  << " % - " << nb_undef << " undefined"  <<endl;
  cout <<"WSD "+name+" Bitext correct :"<< nb_correctb<<" / "<<nb_supervised<< " = " << nb_correctb/nb_supervised*100  << " % - " << nb_undefb << " undefined  - " <<nb_single<< " single sense words "<< nb_unknown << " unknown words " <<endl;
#ifdef PRINT_WSD  
  out_wsd.close();
#endif 
}

real GraphicalBiText::compute_BN_likelihood(int e,int f, bool update, real nb)
{
  // nb is used to update the model with nb times the same observed data
  int s,c,se,i;
  real p,pws;
  real like=0;
  real post,sumpost;

  Vec peC;
  Vec pfC;
  Set ss_anc;
  SetIterator sit,ssit;
  peC.resize(ss_size);
  pfC.resize(ss_size);
  Set ss_adm;// admissible nodes
  set <int>ss_admAnc   ; // admissible ancestors
   ss_adm =  commNode(e,f);
  
  Set synsets=ontology.getAllCategories();
  //  Set synsets  = ontology.getWordSenses(e);
  for (sit = synsets.begin(); sit != synsets.end(); ++sit){
    s = *sit;
    pws = pES.get(e,s);
    if (pws!=0){
      ss_anc = ontology.getSynsetAncestors(s);
      // if s is not a pure sense add it to its own ancestors
      if (pTC[s]!=0){
	ss_anc.insert(s);
      } 
      // loop on all ancesters
      for ( ssit = ss_anc.begin(); ssit != ss_anc.end(); ++ssit){// go upward following sense ancestors  
	c = *ssit;
	peC[c]+=pws*pS[s];
      }
    }
  }
  synsets.clear();
  ss_anc.clear();
  synsets  = target_word_to_senses[f];

  for (sit = synsets.begin(); sit != synsets.end(); ++sit){
    s = *sit;
    pws = pFS.get(f,s);
    if (pws!=0){
      ss_anc = ontology.getSynsetAncestors(s);
      // if s is not a pure sense add it to its own ancestors
      if (pTC[s]!=0){
	ss_anc.insert(s);
      } 
      // loop on all ancesters
      for ( ssit = ss_anc.begin(); ssit != ss_anc.end(); ++ssit){// go upward following sense ancestors  
	c = *ssit;
	pfC[c]+=pws*pS[s];
      }
    }
  }
  
  for( ssit = ss_adm.begin();ssit != ss_adm.end();++ssit){
    c = *ssit;
    //for(i=0;i< commNode(e,f).size();i++){
    //c = commNode(e,f)[i];
    if (peC[c]!=0){
      if (pTC[c]==0){PLERROR("compute_BN_likelihood : division by zero leC/pTC");}
      peC[c]/=pTC[c];
    }
  }

  for( ssit = ss_adm.begin();ssit != ss_adm.end();++ssit){
    c = *ssit;
    //for(i=0;i< commNode(e,f).size();i++){
    //c = commNode(e,f)[i];
    if (pfC[c]!=0){
      if (pTC[c]==0){PLERROR("compute_BN_likelihood : division by zero lfC/pTC");}
      pfC[c]/=pTC[c];
    }
  }
  
  for( ssit = ss_adm.begin();ssit != ss_adm.end();++ssit){
    c = *ssit;
    //for(i=0;i< commNode(e,f).size();i++){
    //c = commNode(e,f)[i];
    //cout <<" sl "<<c<< " " << peC[c]<<" " <<pfC[c]<<" " <<pC[c];
    like+=peC[c]*pfC[c]*pC[c];
    sum_epEC[c]+=peC[c];
    sum_fpFC[c]+=pfC[c];
    
  }
  //  cout <<" like("<<e<<"/"<<source_id_to_word[e]<<","<<f<<"/"<<target_id_to_word[f]<<")="<<like<<endl ;
  if(update){
    if (like!=0){
      real chk_up_pes=0;
      real chk_up_pfs=0;
      real chk_up_pc=0;
      real chk_up_ps=0;
      // Update pA
      
      for( ssit = ss_adm.begin();ssit != ss_adm.end();++ssit){
      	c = *ssit; 
	//for(i=0;i< commNode(e,f).size();i++){
	//c = commNode(e,f)[i];
	p= peC[c]*pfC[c]*pC[c]/like;
	if (p!=0)nA[c]+=nb*p*pA[c];
	chk_up_pc +=nb*p*pA[c];
      }
      if (chk_up_pc-nb>PROB_PREC)PLERROR("compute_BN_likelihood : inconsistent update for chk_pc = %f  instead of %f",chk_up_pc,nb);
      
      for (sit = synsets.begin(); sit != synsets.end(); ++sit){
	s = *sit;
      	ss_anc = ontology.getSynsetAncestors(s);
	// if s is not a pure sense add it to its own ancestors
	if (pTC[s]!=0){
	  ss_anc.insert(s);
	} 
	
	
	ss_admAnc.clear();
	set_intersection(ss_anc.begin(),ss_anc.end(),ss_adm.begin(),ss_adm.end(),inserter( ss_admAnc, ss_admAnc.begin() ));
	
	// Update pES
	pws = pES.get(e,s);
	if (pws!=0){
	  // loop on all admissible ancestors
	  for ( ssit = ss_admAnc.begin(); ssit != ss_admAnc.end(); ++ssit){// go upward following sense ancestors  
	    c = *ssit;
	    //for(i=0;i< commNode(e,f).size();i++){
	    //c = commNode(e,f)[i];
	    if(ss_anc.find(c)==ss_anc.end())continue;
	    p = pC[c]*pws*pS[s]/pTC[c]*pfC[c]/like;
	    
	    if (p!=0){
	      nES.incr(e,s,nb*p);
	      nS[s]+=nb*p;
	      chk_up_pes+=nb*p;
	      chk_up_ps+=nb*p;
	      //cout <<" e ul "<<c<<" pC=" << pC[c]<<" pES="<<pws<<" ps="<<pS[s]<<" pTC="<<pTC[c]<<" pfC=" <<pfC[c]<<" p="<<pC[c]*pws*pS[s]/pTC[c]*pfC[c]/like<<" cum="<< chk_up_pes<<endl;
	    }
	  }
	}
	// Update pFS
	pws = pFS.get(f,s);
	if (pws!=0){
	  // loop on all ancestors
	  for ( ssit = ss_admAnc.begin(); ssit != ss_admAnc.end(); ++ssit){// go upward following sense ancestors  
	    c = *ssit;
	    //for(i=0;i< commNode(e,f).size();i++){
	    //c = commNode(e,f)[i];
	    if(ss_anc.find(c)==ss_anc.end())continue;
	    p = pC[c]*pws*pS[s]/pTC[c]*peC[c]/like;
	    if (p!=0){
	      nFS.incr(f,s,nb*p);
	      nS[s]+=nb*p;
	      //cout <<" f ul "<<c<<" pC=" << pC[c]<<" pFS="<<pws<<" ps="<<pS[s]<<" pTC="<<pTC[c]<<" peC=" <<pfC[c]<<" p="<<pC[c]*pws*pS[s]/pTC[c]*pfC[c]/like<<" cum="<< chk_up_pfs<<endl;
	      chk_up_pfs+=nb*p;
	      chk_up_ps+=nb*p;
	    }
	  }
	}
      }
      if (chk_up_pfs-nb>PROB_PREC || chk_up_pes-nb>PROB_PREC )PLERROR("compute_BN_likelihood : inconsistent update for chk_pES = %f  or chk_pFS = %f instead of %f",chk_up_pes,chk_up_pfs,nb);
      if (chk_up_ps-2*nb>PROB_PREC)PLERROR("compute_BN_likelihood : inconsistent update for chk_ps = %f  instead of %f",chk_up_ps,nb);
      
    }
  }
  
  // Compute Entropy on Bitext
  // Compute Posterior P(S=se|E=e,F=f)
  
  sumpost=0;
  if (like!=0){  
    // For all possibles Senses
    Set e_senses = ontology.getWordSenses(e);
    for (sit = e_senses.begin(); sit != e_senses.end(); ++sit){
      post=0;
      se = *sit;
      ss_anc = ontology.getSynsetAncestors(se);
      // if se is not a pure sense
      if (pTC[se]!=0){
	ss_anc.insert(se);
      } 
         ss_adm =  commNode(e,f);
      ss_admAnc.clear();
      set_intersection(ss_anc.begin(),ss_anc.end(),ss_adm.begin(),ss_adm.end(),inserter( ss_admAnc, ss_admAnc.begin() ));
      pws = pES.get(e,se);
      if (pws!=0){
	// loop on all admissible ancestors
	for ( ssit = ss_admAnc.begin(); ssit != ss_admAnc.end(); ++ssit){// go upward following sense ancestors  
	  c = *ssit;
	  //for(i=0;i< commNode(e,f).size();i++){
	  //c = commNode(e,f)[i];
	  if(ss_anc.find(c)==ss_anc.end())continue;
	  post += pC[c]*pws*pS[se]/pTC[c]*pfC[c]/like;
	  //cout <<" esf post "<<c<<" pC=" << pC[c]<<" pES="<<pws<<" ps="<<pS[se]<<" pTC="<<pTC[c]<<" pfC=" <<pfC[c]<<" p="<<pC[c]*pws*pS[se]/pTC[c]*pfC[c]/like<<" cum="<<post<<endl;
	}
      }
      if(post!=0){
	nSEbi.incr(se,e,post);
	sumpost+=post;
      }
    }
    if (sumpost-1.0>PROB_PREC)PLERROR("Bitext Entropy computation : sum posterior %f != 1.0",sumpost);
  }
  
  return like;
}

void GraphicalBiText::compute_train_likelihood(string name)
{
  compute_likelihood(train_bitext_src,train_bitext_tgt,name,1);
}

void GraphicalBiText::compute_valid_likelihood(string name)
{
  compute_likelihood(valid_bitext_src,valid_bitext_tgt,name,0);
}

void GraphicalBiText::compute_likelihood( Vec bitext_src, Vec bitext_tgt,string name, bool update)
{

  real join_event_number=0;
  real indep_event_number=0;
  real bn_event_number=0;
  real bn_like;
  real indep_like;
  real join_like;
  real join_log_likelihood = 0.0;
  real smoothed_join_log_likelihood = 0.0;
  real indep_log_likelihood =0.0;
  real bn_log_likelihood =0.0;
  real smoothed_bn_log_likelihood =0.0;
  // update variables
  real sum_s,sum_es,sum_fs;
  real up_proba;
  int i;
  int e,f,s,c;
  SetIterator sit,ssit;

  int nb_trans_pairs=0;
  ProbSparseMatrix ef_occur;
  real nb_occu;


  ef_occur.resize(source_wsd_voc_size,target_wsd_voc_size);
  ef_occur.setName("ef_occur");ef_occur.setMode(COLUMN_WISE);

  ofstream out_like ("out_like");
  if (!out_like.is_open()){ PLERROR("error while opening out_like");}
  
  
  join_log_likelihood = 0.0;
  indep_log_likelihood =0.0;
  bn_log_likelihood =0.0;
  join_event_number=0;
  indep_event_number=0;
  bn_event_number=0;
  
  if (update){
    nA.clear();
    nS.clear();
    nES.clear();
    nFS.clear();
    nSEbi.clear();
  }

  // since the likelihood depends only on (e,f), it can computed only once for each (e,f)
  // the updating and global likelihood depends on each (e,f) likelihood and on the frequency 
  // of each (e,f)
  
  ShellProgressBar progress(0,bitext_src.size(), "Computing_likelihood_phase1_"+name, 50);  
  progress.init();
  progress.draw();
  
  for (i=0;i<bitext_src.size() ;i++){    
    e = (int)bitext_src[i];
    f = (int)bitext_tgt[i];
    // Compute likelihod only for words in source_wsd_voc
    if(ontology.isWord(e) && target_wsd_voc.find(f)!=target_wsd_voc.end()){
      ef_occur.incr(e,f);
      nb_trans_pairs++;
    } 
    progress.update(i);
  }
  cout << nb_trans_pairs << " translation_pairs_found"<< endl;
  progress.done();  
  progress.set(0,ef_occur.getWidth(), "Computing_likelihood_phase2_"+name, 50);  
  progress.init();
  progress.draw();
  
  for (int f = 0;  f< ef_occur.getWidth(); f++){
    map<int, real>& col_j = ef_occur.getCol(f);
    for (map<int, real>::iterator it = col_j.begin(); it != col_j.end(); ++it){
      e = (int)it->first;
      nb_occu = it->second;
      // Compute independant proba
      indep_like = pE[e]*pF[f];
      indep_log_likelihood += nb_occu*log(indep_like);
      indep_event_number+= nb_occu;
      
      // compute BN likelihood
      bn_like= compute_BN_likelihood(e,f,update,nb_occu);
      if (bn_like>1.0+PROB_PREC){ PLERROR("Compute_likelihood : BN proba > 1 for %d (%s) %d (%s) ",e,(source_id_to_word[e]).c_str(),f,(target_id_to_word[f]).c_str());}
      if (bn_like!=0){
	bn_log_likelihood += nb_occu*log(bn_like);
	bn_event_number+=nb_occu;    
      }
      smoothed_bn_log_likelihood +=log(alpha_bn*bn_like+(1-alpha_bn)*indep_like);
      
      // Compute Joint proba
      join_like = pEF.get(e,f);
      if (join_like!=0){
	join_log_likelihood +=  nb_occu*log(join_like);
	join_event_number+= nb_occu;
      }
      smoothed_join_log_likelihood +=log(alpha_joint*join_like+(1-alpha_joint)*indep_like);
      
    }
    progress.update(f);
  }
  progress.done();
  

  cout << name+" indep \t/ ll = " << indep_log_likelihood << " \t/ token = " << indep_event_number << " \t/ smoothed : "<< indep_log_likelihood << " \t/ perp = " << safeexp(-indep_log_likelihood / indep_event_number) <<  " \t/ smoothed : " <<safeexp(-indep_log_likelihood / indep_event_number)<<endl;
  cout << name+" joint \t/ ll = " << join_log_likelihood << " \t/ token = " << join_event_number << " \t/ smoothed : "<< smoothed_join_log_likelihood << " \t/ perp = " << safeexp(-join_log_likelihood /join_event_number ) << " \t/ smoothed : " <<safeexp(-smoothed_join_log_likelihood /indep_event_number )<< endl;
  cout << name+" BN \t/ ll = " << bn_log_likelihood << " \t/ token = " << bn_event_number << " \t/ smoothed : " << smoothed_bn_log_likelihood<< " \t/ perp = " << safeexp(-bn_log_likelihood / bn_event_number) << " \t/ smoothed : " <<safeexp(-smoothed_bn_log_likelihood /indep_event_number )<<endl;
  
  
  if (update){
    progress.set(0, ss_size, "Update_pS_pES_pFS", 50);
    progress.init();
    progress.draw();
    
    // Update parameters
    pA.clear();
    pS.clear();
    pES.clear();
    pFS.clear();
    
    
    
    // update pS
    sum_s = sum(nS);
    //cout << "sum nS :" << sum_s<<endl;
    Set synsets=ontology.getAllCategories();
    for (sit = synsets.begin(); sit != synsets.end(); ++sit){
      s = *sit;
      if (nS[s]!=0)pS[s]=nS[s]/sum_s;//+clear
      sum_es = 0;
      Set source_words = ontology.getAllWords();
      for (ssit = source_words.begin(); ssit != source_words.end(); ++ssit){
	e = *ssit;
      	sum_es += nES.get(e,s);
      }
      for (ssit = source_words.begin(); ssit != source_words.end(); ++ssit){
	e = *ssit;
      	up_proba= nES.get(e,s);
	if (up_proba!=0){
	  pES.set(e,s,up_proba/sum_es);
	  //	  cout << " ue " <<up_proba/sum_es ;	
	}
      }
      sum_fs=0;
      for (ssit = target_wsd_voc.begin(); ssit != target_wsd_voc.end(); ++ssit){
	f = *ssit;
	sum_fs += nFS.get(f,s);
      }
      for (ssit = target_wsd_voc.begin(); ssit != target_wsd_voc.end(); ++ssit){
	f = *ssit;
	up_proba = nFS.get(f,s);
	if (up_proba!=0){
	  //cout << " uf "<<up_proba/sum_fs;
	  pFS.set(f,s,up_proba/sum_fs);
	}
      }
      progress.update(s);
    }
    compute_pTC();
    
    // Update pA
    synsets=ontology.getAllCategories();
    for (sit = synsets.begin(); sit != synsets.end(); ++sit){
      c = *sit;
      if(nA[c]!=0){
	pA[c]=nA[c]/bn_event_number;
      }
    }
    compute_pTC();
    check_set_pA();
    compute_pMC();
   
   
    progress.done();
  }
  pSEbi.clear();
  // Entropy computation
  pSEbi.normalizeCond(nSEbi, false);

}

void  GraphicalBiText::computeKL()
{
  int e;
  SetIterator sit;
  Set e_words=ontology.getAllWords();
  real kl,skl;
  for (sit = e_words.begin(); sit != e_words.end(); ++sit){
    e = *sit;
    kl=0;
    if ( pSEbi.sumCol(e)==0 ||  pSE.sumCol(e)==0)continue;
    map<int, real>& col_e = pSE.getCol(e);
    //cout <<"KL\t"<<source_id_to_word[e];
    for (map<int, real>::iterator mit = col_e.begin(); mit != col_e.end(); ++mit){
      //cout << " e="<<e<<" s="<<mit->first<<" bi="<<pSEbi.get(mit->first,e)<<" ref="<<mit->second;
      skl=pSEbi.get(mit->first,e)*safeflog2(pSEbi.get(mit->first,e)/mit->second);
      if (!isnan(skl))kl+=skl;
    }
    //cout << "\t"<<kl<<endl;
    KL[e]=kl;
  }
}



void  GraphicalBiText::loadSensemap(string sensemap_file)
{
  int nbMap=0;
  // Load sensemap file
  cout << "Loading sensemap : ";
  ifstream sensemap_stream(sensemap_file.c_str());
  string line;
  vector<string> tokens;
  if(sensemap_stream.is_open()){
    while(!sensemap_stream.eof()){
      line = pgetline(sensemap_stream);
      if (line=="") continue;
      tokens = split(line, " ");
      if (tokens.size()>1){
	nbMap++;
	sensemap[tokens[0]]=tokens[2];
      }else{
	sensemap[tokens[0]]=tokens[0];
      }
    }
  }
  cout << nbMap << " sense mappings found\n";
  //  for(map<string,string>::iterator  mit=sensemap.begin();mit!=sensemap.end();mit++)cout << mit->first << " -> "<<mit->second<<endl;
}

void GraphicalBiText::train(VMat training_set)
{


  TVec<string> our_answers1(wsd_train.length());
  
  real interp_max = 1;
  real interp_min = 1;
  real interp_step = 0.4;
  //  Bi.print("0");
  //printHierarchy("0");
  //Bi.computeKL();
  for(real interp=interp_min;interp<=interp_max;interp+=interp_step){
    test_WSD(wsd_train, "Semcor_train_set_epoch_0_"+tostring(interp), our_answers1,0,interp);
    test_WSD(wsd_valid, "Semcor_valid1_set_epoch_0_"+tostring(interp), our_answers1,0,interp);
    test_WSD(wsd_valid2,"Semcor_valid2_set_epoch_0_"+tostring(interp),our_answers1,0,interp);
    test_WSD(wsd_test,"Semcor_test_set_epoch_0_"+tostring(interp),our_answers1,0,interp);
    //  test_WSD(senseval2_train,"Senseval2_trainset_epoch_0_"+tostring(interp),our_answers1,0,interp);
    }
  //  Bi.test_WSD(senseval_test, "Senseval_test_set_epoch_0", our_answers,1);
  //string out_name = "out_answer_0";  ofstream out_answer (out_name.c_str());  if (!out_answer.is_open()){ PLERROR("error while opening out_answer");}int k=0;for(int i=0; i<our_answers.size(); i++){    string::size_type pos = headers[i].find_first_of(".");    out_answer << headers[i].substr(0,pos) << " " << headers[i] << " " << our_answers[k] << endl;    k++;  }out_answer.close();
  
  
  for (int i=1;i<n_epoch;i++){
     compute_train_likelihood("Train_set_epoc "+tostring(i));
     //    Bi.computeKL();
     compute_valid_likelihood("Valid_set_epoc "+tostring(i));
     //optimize_interp_parameter(test_tgt,test_src, "Opt valid");
    
     update_WSD_model(tostring(i));
     check_consitency();
     //Bi.print(tostring(i));
     // printHierarchy(tostring(i));
     for(real interp=interp_min;interp<=interp_max;interp+=interp_step){
      test_WSD(wsd_train, "Semcor_train_set_epoch_"+tostring(i)+"_"+tostring(interp), our_answers1,0,interp);
      test_WSD(wsd_valid, "Semcor_valid1_set_epoch_"+tostring(i)+"_"+tostring(interp), our_answers1,0,interp);
      test_WSD(wsd_valid2, "Semcor_valid2_set_epoch_"+tostring(i)+"_"+tostring(interp), our_answers1,0,interp);
      test_WSD(wsd_test,"Semcor_test_set_epoch_"+tostring(i)+"_"+tostring(interp),our_answers1,0,interp);
      // test_WSD(senseval2_train,"Senseval2_train_set_epoch_"+tostring(i)+"_"+tostring(interp),our_answers1,0,interp);
     }
     
     //     Bi.test_WSD(senseval_test, "Senseval_test_set_epoch_"+tostring(i), 
     //     our_answers,1);out_name = "out_answer_"+tostring(i);ofstream out_answer (out_name.c_str());if (!out_answer.is_open()){ PLERROR("error while opening out_answer");}int k=0;for(int j=0; j<our_answers.size(); j++){string::size_type pos = headers[j].find_first_of(".");out_answer << headers[j].substr(0,pos) << " " << headers[j] << " " << our_answers[k] << endl;k++;}out_answer.close();
  }

}

void GraphicalBiText::test()
{

}

void  GraphicalBiText::setTrainingSet(VMat training_set, bool call_forget)
{
  
}


} // end of namespace PLearn

