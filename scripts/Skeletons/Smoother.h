#ifndef DERIVEDCLASS_INC
#define DERIVEDCLASS_INC

#include "Smoother.h"

namespace PLearn <%
using namespace std;

class DERIVEDCLASS: public Smoother
{
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

  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  DERIVEDCLASS();


  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:
  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  PLEARN_DECLARE_OBJECT(DERIVEDCLASS);


  /****
   * DERIVEDCLASS methods
   */

 public:
  // The source function is either f(i) = source_function[i] as a function of i
  // or if bin_positions is provided (non-zero length), 
  //    f(x) = source_function[i]
  //      where i is s.t. bin_positions[i]>x>=bin_positions[i+1]
  // the optional bin_positions vector has length 0, or 1 more than source_function.
  // By default (if not provided) the dest_bin_positions are assumed the same as the source bin_positions.
  // Returns integral(smoothed_function).
  virtual real smooth(const Vec& source_function, Vec smoothed_function, 
		      Vec bin_positions = Vec(), Vec dest_bin_positions = Vec()) const;

  //   real smooth(const HistogramCDF& source_cdf, HistogramCDF& dest_cdf);

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(DERIVEDCLASS);
  
%> // end of namespace PLearn

#endif
