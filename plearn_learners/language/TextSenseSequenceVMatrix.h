/*! \file TextSenseSequenceVMatrix.h */

#ifndef TextSenseSequenceVMatrix_INC
#define TextSenseSequenceVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "WordNetOntology.h"
#include "DiskVMatrix.h"
#include "VMat.h"
#include "WordNetOntology.h"
#include "random.h"

namespace PLearn <%
using namespace std;

//! This class handles a sequence of words/sense tag/POS triplets to present it as target words and their context
class TextSenseSequenceVMatrix: public RowBufferedVMatrix
{
protected:
  // *********************
  // * protected options *
  // *********************

  //! The VMatrix containing the sequence of words or lemmas, with their POS and WordNet (optional) tags
  VMat dvm;
  //! The number of context words
  int window_size;  // window_size = 2*n_context
  //! Indication that at less some of the words or lemmas are semantically disambiguated
  bool is_supervised_data;
  //! The vector containing the forbidden POS of the words given in the context of a target word
  TVec<int> res_pos;
  //! Indication that examples can be randomly generated using random synonym replacements
  bool rand_syn;
  //! Probability of a word given it has some sense
  TVec<TVec<pair<int, real> > > word_given_sense_priors;
  //! Ontology of the sense tagging
  WordNetOntology *wno;
  //! Index of the current row
  mutable int my_current_row_index;
  //! Elements of the current row
  mutable Vec my_current_row;

public:

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor. After setting all options individually, build() should be called.
  TextSenseSequenceVMatrix();
  //!
  /*!
    \param that_dvm the sequence of words/lemmas
    \param that_window_size the number of context words/lemmas
    \param that_res_pos the forbidden POS for the context words
    \param that_rand_syn indication that the user allow the random generation of contexts and target words using synonyms
    \param that_wno the ontology used as a sense inventory
   */
  TextSenseSequenceVMatrix(VMat that_dvm, int that_window_size, TVec<int> that_res_pos = TVec<int>(0), bool that_rand_syn = false, WordNetOntology *that_wno = NULL)
    :inherited(that_dvm->length(), 3*(that_window_size+1)),dvm(that_dvm),window_size(that_window_size), is_supervised_data(dvm->width()==3), res_pos(that_res_pos), rand_syn(that_rand_syn), wno(that_wno), my_current_row_index(-3*(that_window_size+1)), my_current_row(3*(that_window_size+1))
  /* ### Initialise all fields to their default value */
{
  build_();
}

  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();
  
  //! This permutes randomly the words (target and context) with one of their corresponding synonym.
  void permute(Vec v) const;

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:
  //!  This is the only method requiring implementation
  virtual void getNewRow(int i, Vec& v) const;

  //! This restricts the extraction of the context to the words that don't have their POS in res_pos and returns the position of the next non-overlapping context
  int getRestrictedRow(int i, Vec v) const;

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Sets the ontology
  void setOntology(WordNetOntology *that_wno){wno = that_wno;}

  //! Sets the number of context words
  void setWindowSize(int that_window_size){window_size = that_window_size;}

  //! Sets the VMatrix of word/sense_tag/POS sequence
  void setWordSequence(VMat that_dvm){dvm = that_dvm; is_supervised_data = that_dvm->width()==3; }

  //! Sets the activation/desactivation of the random generation of contexts and target words
  void setRandomGeneration(bool that_rand_syn){rand_syn = that_rand_syn;}

  //! Sets the vector of forbidden POS for the context words
  void setRestrictedPOS(TVec<int> that_res_pos){res_pos = that_res_pos;}


  typedef RowBufferedVMatrix inherited;
  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(TextSenseSequenceVMatrix);

};

%> // end of namespace PLearn
#endif
