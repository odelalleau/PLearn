/* *******************************************************      
   * $Id: test_TVec.h,v 1.1 2002/10/11 13:44:23 dorionc Exp $
   * AUTHORS: Kim Levy & Christian Dorion
   * This file is part of the PLearn library.
   ******************************************************* */

/*! \file PLearnLibrary/TestSuite/test_TVec.h */

#ifndef TEST_TVEC_H
#define TEST_TVEC_H

#include "t_general.h"
#include "Array.h"
#include "TVec.h"

using namespace PLearn;

#define MAX_SIZE 10 // Array's maximum size
#define MAX_LENGTH 40 // Vector's maximum length 
 

class Test_TVec
{
public:
  int arraySize;

  int** consData;
  int**  fillData;  
  int** appendData;

  //! these arrays will contain vectors constructed with the constructor associated to their name
  Array< TVec<int> > emptyCtorArray;
  Array< TVec<int> > lengthCtorArray;
  Array< TVec<int> > valueCtorArray;
  Array< TVec<int> > stepCtorArray;  
  Array< TVec<int> > dataCtorArray; 
  Array< TVec<int> > copyCtorArray;

  //! these arrays will contain a deep copy of the preceding vectors for comparisons
  Array< TVec<int> > swappedEmptyArray;
  Array< TVec<int> > swappedLengthArray;
  Array< TVec<int> > swappedValueArray; 
  Array< TVec<int> > swappedStepArray;
  Array< TVec<int> > swappedDataArray;
  Array< TVec<int> > swappedCopyArray;
  
  Test_TVec(int size = MAX_SIZE): arraySize(size), 
                                  emptyCtorArray(arraySize),
                                  lengthCtorArray(arraySize),
                                  valueCtorArray(arraySize),
                                  stepCtorArray(arraySize),
                                  dataCtorArray(arraySize),
                                  copyCtorArray(arraySize),
                                  
                                  swappedEmptyArray(arraySize),
                                  swappedLengthArray(arraySize),
                                  swappedValueArray(arraySize),
                                  swappedStepArray(arraySize),
                                  swappedDataArray(arraySize),
                                  swappedCopyArray(arraySize)   
  {
    consData = new int*[arraySize];
    fillData = new int*[arraySize];
    appendData = new int*[arraySize];
  }

  ~Test_TVec()
  {
    for(int i=0; i<arraySize; i++)
      {
        delete consData[i];
        delete fillData[i];
        delete appendData[i];
      }

    delete []consData;
    delete []fillData;
    delete []appendData;
  }

  //! constructs different vectors using different constructors
  bool constructors();
  //! writes the vectors in a file, clears all the vectors and re-reads them from the file
  bool read_write() ;
  //! fills and copies vectors in different ways an compares them
  bool fill_copy();
  //! makes a deepCopy of the TVecs for later swapping: called after 'subVec_concat()'
  bool deepCopyForSwap();
  bool subVec_concat();
  bool swapVectors();
  bool insert_remove();
  bool appendElementsAndTVecs(); 

private:
  //! returns a random integer within [1, Max-1]
  int randomInt(int Max = MAX_LENGTH);
  void invariants();
  void invariants(int index);
  //!  verifies that the "swapped" arrays contain the same TVecs as the "Ctor" arrays
  void assertEqual();
  void verifySubVecConcat();
  void verifyVectorSwap();
};
  
#endif // TEST_TVec_H
  
