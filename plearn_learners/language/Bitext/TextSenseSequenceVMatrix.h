#ifndef TextSenseSequenceVMatrix_INC
#define TextSenseSequenceVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "WordNetOntology.h"
#include "DiskVMatrix.h"
#include "VMat.h"
#include "WordNetOntology.h"
#include "random.h"

namespace PLearn {
using namespace std;

class TextSenseSequenceVMatrix: public RowBufferedVMatrix
{
protected:
  // *********************
  // * protected options *
  // *********************

  VMat dvm;
  int window_size;  // window_size = 2*n_context
  //int oov_tag_id;
  bool is_supervised_data;
  TVec<int> res_pos;
  bool rand_syn;
  TVec<TVec<pair<int, real> > > word_given_sense_priors;
  WordNetOntology *wno;

public:

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  TextSenseSequenceVMatrix();
  TextSenseSequenceVMatrix(VMat that_dvm, int that_window_size, TVec<int> that_res_pos = TVec<int>(0), bool that_rand_syn = false, WordNetOntology *that_wno = NULL)
    :inherited(that_dvm->length(), 3*(that_window_size+1)),dvm(that_dvm),window_size(that_window_size), is_supervised_data(dvm->width()==3), res_pos(that_res_pos), rand_syn(that_rand_syn), wno(that_wno)
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
  virtual void getRow(int i, Vec v) const;

  //! This restricts the extraction of the context to the words that don't have their POS in res_pos and returns the position of the next non-overlapping context
  int getRestrictedRow(int i, Vec v) const;

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  void setOntology(WordNetOntology *that_wno){wno = that_wno;}



  typedef RowBufferedVMatrix inherited;
  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(TextSenseSequenceVMatrix);

};

} // end of namespace PLearn
#endif
