/* PLearnLibrary/TestSuite/TMat/test_TVec.cc */

#include "test_TVec.h"
//#include <iostream.h>

using namespace PLearn;


// ---------- public -------------------------------------------------------------------------------
//! costructs different vectors and fills the arrays of vectors of corresponding type 
bool Test_TVec::constructors()
{
  int vecLength, smallerInt, step;
  for(int j=0; j<arraySize; j++)
    {
      vecLength = randomInt(MAX_LENGTH); 
      smallerInt = randomInt(vecLength);
      step = randomInt(smallerInt); 
      
      emptyCtorArray[j] = TVec<int>();
      T_ASSERT( emptyCtorArray[j].isEmpty(), "The vector at position "+ tostring(j) +" should be empty");
      emptyCtorArray[j].resize(vecLength);
      T_ASSERT( emptyCtorArray[j].isNotEmpty(), "The vector at position "+ tostring(j) +" should not be empty");
      
      lengthCtorArray[j] = TVec<int>(vecLength); 
      T_ASSERT( emptyCtorArray[j].isNotEmpty(), "The vector at position "+ tostring(j) +" should not be empty");
      
      valueCtorArray[j] = TVec<int>(vecLength, smallerInt);
      T_ASSERT( valueCtorArray[j]==smallerInt, "The constructor using length and value is not correct");
      
      stepCtorArray[j] = TVec<int>(smallerInt, smallerInt-1 + step*vecLength, step); 

      consData[j]= new int[vecLength];
      for(int i=0; i< vecLength; i++ )
        consData[j][i]  = randomInt(vecLength);
      
      dataCtorArray[j] = TVec<int>(vecLength, consData[j]);
     
      copyCtorArray[j] = TVec<int>(stepCtorArray[j]);
      //! Checking if the TVecs constructed by making a copy of the "data vectors" are equal to them
      T_ASSERT( stepCtorArray[j]==copyCtorArray[j], "The copy constructor does not return a good copy of the TVec it was given" );
    }
  cout << "TVec have been constructed: \n-empty\n-with only the length\n-with the length and a value\n-with a range and step"
       << "\n-with an array of data\n-with another TVec" << endl;
  invariants(); 
  return true;
}

//! writes the vectors in a file, clears all the vectors and re-reads them from the file
bool Test_TVec::read_write()
{
  filebuf fb;
  fb.open("tvecFile.txt", ios::out);
  ostream os(&fb);
  int j;
  for(j=0; j<arraySize; j++)
    {
      write(os, emptyCtorArray[j]);
      emptyCtorArray[j].clear();
      T_ASSERT( emptyCtorArray[j]==0, "1-The vector at position "+ tostring(j) +" should contain only '0's");
      write(os, lengthCtorArray[j]);
      lengthCtorArray[j].clear();
      T_ASSERT( lengthCtorArray[j]==0, "2-The vector at position "+ tostring(j) +" should contain only '0's");      
      write(os, valueCtorArray[j]);
      valueCtorArray[j].clear();
      T_ASSERT( valueCtorArray[j]==0, "3-The vector at position "+ tostring(j) +" should contain only '0's");
      write(os, stepCtorArray[j]);
      stepCtorArray[j].clear();
      T_ASSERT( stepCtorArray[j]==0, "4-The vector at position "+ tostring(j) +" should contain only '0's");
      write(os, dataCtorArray[j]);
      dataCtorArray[j].clear();
      T_ASSERT( dataCtorArray[j]==0, "5-The vector at position "+ tostring(j) +" should contain only '0's");
      write(os, copyCtorArray[j]);
      copyCtorArray[j].clear();
      T_ASSERT( copyCtorArray[j]==0, "6-The vector at position "+ tostring(j) +" should contain only '0's");
    }
  fb.close();
  cout << "All vectors have been writen in tvecFile.txt and cleared out" << endl; 
  
  fb.open("tvecFile.txt", ios::in);
  istream is(&fb); 
  for(j=0; j<arraySize; j++)
    {
      read(is, lengthCtorArray[j]);          
      read(is, lengthCtorArray[j]);
      read(is, valueCtorArray[j]);
      read(is, stepCtorArray[j]);
      read(is, dataCtorArray[j]);
      read(is, copyCtorArray[j]);
    }
  fb.close();
  cout << "All vectors have been read from tvecFile.txt and reconstructed" << endl;

  invariants();
  return true;
} 

//! 1-fills a vector with a value, fills an array with the same value and copies(copyFrom) it to an other vector, then compares
//! 2-fills with operator and compares
//! 3-fills a vector using a step, fills an array with the same values and copies it to an other vector, then compares
//! 4-uses copyTo(TVec t) and copy() and compares
bool Test_TVec::fill_copy()
{
  TVec<int> vec;
  int rand, step, length, j;
  for(int i=0; i<arraySize; i++)
    { 
      rand = randomInt(); 
      step = randomInt(rand/2);
      length = emptyCtorArray[i].length();
      
      //! 1-fill(const T& value) const and copyFrom(const T* x, int n) const
      emptyCtorArray[i].fill(rand);
      fillData[i] = new int[length];
      for(j=0; j<length; j++)
        fillData[i][j]  = rand;
      
      lengthCtorArray[i].copyFrom(fillData[i], length);
      T_ASSERT(emptyCtorArray[i]==lengthCtorArray[i], "1-Error has occured while filling or copying");
      
      //! 2-operator= ( <==>fill(const T& value )
      emptyCtorArray[i]=rand;
      T_ASSERT(emptyCtorArray[i]==lengthCtorArray[i], "2-Error has occured while using operator=");     
      
      //! 3-fill(const T& startval, const T& step) const 
      emptyCtorArray[i].fill(rand, step);
      j=rand; //initial value
      for(int k=0; k<length; k++)
        {
          fillData[i][k] = j;
          j+=step;
        }
      lengthCtorArray[i].copyFrom(fillData[i], length);
      T_ASSERT(emptyCtorArray[i]==lengthCtorArray[i], "3-Error has occured while filling with a choosen step");
      
      //! 4-copyTo(T* x) const 
      valueCtorArray[i].copyTo(fillData[i]);
      lengthCtorArray[i].copyFrom(fillData[i], length);
      T_ASSERT(valueCtorArray[i]==lengthCtorArray[i], "4-Error has occured while coying into an array");
      
      //! 5-copy()
      vec = lengthCtorArray[i];
      lengthCtorArray[i] = emptyCtorArray[i].copy();
      T_ASSERT(emptyCtorArray[i]==lengthCtorArray[i], "5-Error has occured while filling with a choosen step"); 
      lengthCtorArray[i] = vec;
    }
  invariants();
  return true;
}

//! simply deep-copies the vectors in the different arrays
bool Test_TVec::deepCopyForSwap()
{     
  for(int i=0; i<arraySize; i++)
    {   
      // Creating a copy of the vectors
      swappedEmptyArray[i] = deepCopy(emptyCtorArray[i]);
      swappedLengthArray[i] = deepCopy(lengthCtorArray[i]);
      swappedValueArray[i] = deepCopy(valueCtorArray[i]);
      swappedStepArray[i] = deepCopy(stepCtorArray[i]);
      swappedDataArray[i] = deepCopy(dataCtorArray[i]); 
      swappedCopyArray[i] = deepCopy(copyCtorArray[i]);
    }
  cout <<  "-DeepCopy for swap done" << endl;
  return true;
}

//! exchanges the tail and the head of each vector to test concat and subVec
bool Test_TVec::subVec_concat()
{ 
  int mid = arraySize/2; // if arraySize%2!=0 then the last elem is not considered 
  int length1, length2;
  for(int i=0; i<mid; i++)
    {
      length1 = emptyCtorArray[i].length();
      length2 = emptyCtorArray[i+mid].length(); 
      copyCtorArray[i] = TVec<int>();
      copyCtorArray[i+mid] = TVec<int>();
     
      // LEGENDE:  part1:---- part2:****
      emptyCtorArray[i].concat(emptyCtorArray[i],emptyCtorArray[i+mid]); 
      lengthCtorArray[i].concat(lengthCtorArray[i],lengthCtorArray[i+mid]);
      valueCtorArray[i].concat(valueCtorArray[i],valueCtorArray[i+mid]);
      stepCtorArray[i].concat(stepCtorArray[i],stepCtorArray[i+mid]);
            
      // 1-Replace second half of the vectors by the first part  1234//5678  -> { 1234//1234   -> { 5689//1234 }
      // 2-Keep only the second part in the first half           5678             5678     }
      // 1-resize 
      emptyCtorArray[i+mid].resize( length1 );
      lengthCtorArray[i+mid].resize( length1 );
      valueCtorArray[i+mid].resize( length1 );
      stepCtorArray[i+mid].resize( length1 );
      cout << "-resize1-" <<endl;
      // 1-subVec
      emptyCtorArray[i+mid] << emptyCtorArray[i].subVec(0, length1);
      lengthCtorArray[i+mid] << lengthCtorArray[i].subVec(0, length1);
      valueCtorArray[i+mid] << valueCtorArray[i].subVec(0, length1); 
      stepCtorArray[i+mid] << stepCtorArray[i].subVec(0, length1);
      cout << "-subVec1-" << endl; 
        
 
      // 2-subVec 
      emptyCtorArray[i] = TVec<int>(emptyCtorArray[i].subVec(length1, length2)); 
      lengthCtorArray[i] = TVec<int>(lengthCtorArray[i].subVec(length1, length2));
      valueCtorArray[i] = TVec<int>(valueCtorArray[i].subVec(length1, length2));
      stepCtorArray[i] = TVec<int>(stepCtorArray[i].subVec(length1, length2));      
      T_ASSERT(emptyCtorArray[i].length()==length2, "The subVec is not the appropriate size");
      cout << "-subvec2-" << endl;

      // To insure that all TVecs at the same index have the same length
      dataCtorArray[i] = deepCopy(valueCtorArray[i]);
      dataCtorArray[i+mid] = deepCopy(valueCtorArray[i+mid]);
      copyCtorArray[i] = deepCopy(stepCtorArray[i]);
      copyCtorArray[i+mid] = deepCopy(stepCtorArray[i+mid]);

      swappedDataArray[i+mid] = deepCopy(dataCtorArray[i]); 
      swappedDataArray[i] = deepCopy(dataCtorArray[i+mid]); 
      swappedCopyArray[i+mid] = deepCopy(copyCtorArray[i]);
      swappedCopyArray[i] = deepCopy(copyCtorArray[i+mid]);
      
      invariants(i);      
    }
  cout << "-Construction completed" << endl;
  verifySubVecConcat();
  assertEqual();
  invariants();
  return true;
}

//! swaps vectors and verifies it has been done correctly, then re-swaps to initial position
bool Test_TVec::swapVectors()
{
  for(int i=0; i<arraySize; i++)
    {
      // Swapping two vectors:     { 1234//5678 } -> { 5678//1234 }   
      // 1rst type: swap( TVec<T>& a, TVec<T>& b)
      swap(swappedEmptyArray[i], swappedLengthArray[i]);
    }
  verifyVectorSwap();
  assertEqual();
  invariants();
  return true;
}

//! appends values to a vector while filling an array of the same values, 
//! then appends the array to another vector and compares
bool Test_TVec::appendElementsAndTVecs()
{
  assertEqual();
  int rand, randSize, length;
  TVec<int> newVec;
  for(int i=0; i<arraySize; i++)
    {  
      length = valueCtorArray[i].length();
      randSize = randomInt(length);
      appendData[i] = new int[randSize];
      
      for(int j=0; j<randSize; j++)
        {
          rand = randomInt(length);
          valueCtorArray[i].append(rand);
          appendData[i][j] = rand;
        }
      
      newVec = TVec<int>(randSize, appendData[i]);
      swappedValueArray[i].append(newVec);
      T_ASSERT( swappedValueArray[i]==valueCtorArray[i], "1-Conflicts between 'append' of values one by one and 'append' of a TVec");

     //! taking subvec to assert invariants with the other arrays
      swappedValueArray[i] = swappedValueArray[i].subVec( 0, length );
      valueCtorArray[i] = valueCtorArray[i].subVec( 0, length );
      T_ASSERT( swappedValueArray[i]==valueCtorArray[i], "2-Conflicts between 'append' of values one by one and 'append' of a TVec");
    }
  invariants();  
  assertEqual();
  return true;
} 



//! After this, the "swapped" Arrays and the "Ctor" Arrays will not be the same
bool Test_TVec::insert_remove()
{
  int rand, length;
  for(int i=0; i<arraySize; i++)
    {
      length = emptyCtorArray[i].length();
      rand = randomInt(length); 
      for(int j=0; j<length; j=j+2)
        { 
          emptyCtorArray[i].insert(j, rand);
          emptyCtorArray[i].remove(j+1);
        } 
      T_ASSERT(emptyCtorArray[i]==rand, "Error while inserting and removing a v");
    }

  cout << "-Insertions and removals have been done" << endl;
  invariants();
  return true;
}






// ------------ private ----------------------------------------------------------------------------

void Test_TVec::invariants()
{
  for(int j=0; j<arraySize; j++)
    invariants(j);
    
  cout << "-Invariants have been verified" << endl;
}
void Test_TVec::invariants(int index)
{
  //! Checking length
  T_ASSERT( emptyCtorArray[index].length()==lengthCtorArray[index].length() 
            && lengthCtorArray[index].length()==valueCtorArray[index].length(),
            "1-Vectors at position '" +tostring(index) +"' should be of equal length: "
            +tostring(emptyCtorArray[index].length())
            +"!="+ tostring(lengthCtorArray[index].length())+"!="+ tostring(valueCtorArray[index].length()) );
  T_ASSERT( valueCtorArray[index].length()==dataCtorArray[index].length() 
            && dataCtorArray[index].length()==copyCtorArray[index].length(),
            "2-Vectors at position '" +tostring(index) +"' should be of equal length: "
            +tostring(valueCtorArray[index].length())
                +"!="+ tostring(dataCtorArray[index].length())+"!="+ tostring(copyCtorArray[index].length()) );
  
  
  //! Checking byte length
  T_ASSERT( emptyCtorArray[index].byteLength()==lengthCtorArray[index].byteLength() 
            && lengthCtorArray[index].byteLength()==valueCtorArray[index].byteLength(),
            "3-Vectors at position '" +tostring(index) +"' should be of equal byteLength: "
            +tostring(emptyCtorArray[index].byteLength())
            +"!="+ tostring(lengthCtorArray[index].byteLength())+"!="+ tostring(valueCtorArray[index].byteLength()) );
  T_ASSERT( valueCtorArray[index].byteLength()==dataCtorArray[index].byteLength() 
            && dataCtorArray[index].byteLength()==copyCtorArray[index].byteLength(),
            "4-Vectors at position '" +tostring(index) +"' should be of equal byteLength: "
            +tostring(valueCtorArray[index].byteLength())
            +"!="+ tostring(dataCtorArray[index].byteLength()) +"!="+ tostring(copyCtorArray[index].byteLength()) );
  
      //! Checking offset
  T_ASSERT( emptyCtorArray[index].offset()==lengthCtorArray[index].offset() 
            && lengthCtorArray[index].offset()==valueCtorArray[index].offset(),
            "5-Vectors at position '" +tostring(index) +"' should be of equal offset: "
            +tostring(emptyCtorArray[index].offset())
            +"!="+ tostring(lengthCtorArray[index].offset())+"!="+ tostring(valueCtorArray[index].offset()) );
  T_ASSERT( valueCtorArray[index].offset()==dataCtorArray[index].offset() 
            && dataCtorArray[index].offset()==copyCtorArray[index].offset(),
            "6-Vectors at position '" +tostring(index) +"' should be of equal offset: "
            +tostring(valueCtorArray[index].offset())
            +"!="+tostring( dataCtorArray[index].offset())+"!="+ tostring(copyCtorArray[index].offset()) );
}


int Test_TVec::randomInt(int max)
{
  return random()%max + 1;
}



void Test_TVec::assertEqual()
{  
  for(int i=0; i<arraySize; i++)
    {
      T_ASSERT(swappedEmptyArray[i]==emptyCtorArray[i],"1- Error has occured while manipulating `swapped` arrays and `Ctor` arrays at position: " + tostring(i) );
      T_ASSERT(swappedLengthArray[i]==lengthCtorArray[i],"2- Error has occured while manipulating `swapped` arrays and `Ctor` arrays at position: " + tostring(i) );
      T_ASSERT(swappedValueArray[i]==valueCtorArray[i],"3- Error has occured while manipulating `swapped` arrays and `Ctor` arrays at position: " + tostring(i) );
      T_ASSERT(swappedStepArray[i]==stepCtorArray[i],"4- Error has occured while manipulating `swapped` arrays and `Ctor` arrays  at position: " + tostring(i) );
      T_ASSERT(swappedDataArray[i]==dataCtorArray[i],"5- Error has occured while manipulating `swapped` arrays and `Ctor` arrays  at position: " + tostring(i) );
      T_ASSERT(swappedCopyArray[i]==copyCtorArray[i],"6- Error has occured while manipulating `swapped` arrays and `Ctor` arrays  at position: " + tostring(i) );      
    }
  cout << "-assertEqual done" << endl;
}

void Test_TVec::verifySubVecConcat()
{
  TVec<int> tempVec;
  int mid = arraySize/2;
  for(int i=0; i<mid; i++)
    {
      tempVec = swappedEmptyArray[i];
      swappedEmptyArray[i] = swappedEmptyArray[i+mid];
      swappedEmptyArray[i+mid] = tempVec;

      tempVec = swappedLengthArray[i];
      swappedLengthArray[i] = swappedLengthArray[i+mid];
      swappedLengthArray[i+mid] = tempVec;

      tempVec = swappedValueArray[i];
      swappedValueArray[i] = swappedValueArray[i+mid];
      swappedValueArray[i+mid] = tempVec;

      tempVec = swappedStepArray[i];
      swappedStepArray[i] = swappedStepArray[i+mid];
      swappedStepArray[i+mid] = tempVec;
 
      tempVec = swappedDataArray[i];
      swappedDataArray[i] = swappedDataArray[i+mid];
      swappedDataArray[i+mid] = tempVec;
 
      tempVec = swappedCopyArray[i];
      swappedCopyArray[i] = swappedCopyArray[i+mid];
      swappedCopyArray[i+mid] = tempVec;
    }
  cout << "-Verification completed" << endl;
}


void Test_TVec::verifyVectorSwap()
{  
  for(int i=0; i<arraySize; i++)
    {
      T_ASSERT(swappedEmptyArray[i] == lengthCtorArray[i],"1-The swap of two vectors isn't done correctly at position: " + tostring(i));
      T_ASSERT(swappedLengthArray[i] == emptyCtorArray[i],"2-The swap of two vectors isn't done correctly at position: " + tostring(i));
      swap(swappedLengthArray[i], swappedEmptyArray[i]);
    }
  cout << "-Verification completed" << endl;
}








// -------------------------------------------------------------------------------------------------

int main(int argc, char* argv)
{
  cout << "Creating Test_TVec..." << endl;
  Test_TVec tv;
  
  cout << "Launching Test_TVec..." << endl;
  
  DO_TEST("Testing constructors", tv.constructors());
  DO_TEST("Testing reading and writing", tv.read_write()); 
  DO_TEST("Testing filling and copying", tv.fill_copy());
  DO_TEST("Testing deep copy",tv.deepCopyForSwap());
  DO_TEST("Testing concatenation, creating sub-TVecs and resizing", tv.subVec_concat()); 
  DO_TEST("Testing swap of two vectors", tv.swapVectors());
  DO_TEST("Testing append for elements and TVecs", tv.appendElementsAndTVecs()); 
  DO_TEST("Testing insertion and removal", tv.insert_remove());

  cout << "Quitting Test_TVec..." << endl;
  
  return 0;
}
