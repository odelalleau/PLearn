%module PLearn
%{
#include "Train.h"
%}

namespace PLearn {
using namespace std;

class Train: public Object
{

private:
  
  typedef Object inherited;

protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  // ************************
  // * public build options *
  // ************************
  PP<PLearner> learner;
  string dataset;
  string psave;

  void run();
  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  // ### Make sure the implementation in the .cc
  // ### initializes all fields to reasonable default values.
  Train();

  
} // end of namespace PLearn
