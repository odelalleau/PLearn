/*! \file PLearnLibrary/TestSuite/test_Storage.h */


#ifndef TEST_STORAGE_H
#define TEST_STORAGE_H

#include "t_general.h"
#include "Storage.h"

using namespace PLearn;
  
#define NB_STORAGES 10		
#define NB_OBJ 		  5
#define NB_CTOR	 	  4
#define SERIAL_FILE "serialization.test"
  
class Integer
{
public:
  int intValue;
  inline Integer():intValue(-1000){}
  inline Integer(const int& i): intValue(i){}
  inline void deepWrite( ostream& out, DeepWriteSet& already_saved ) const { PLearn::deepWrite(out, already_saved, intValue); }
  inline void deepRead( istream& in, DeepReadMap& old2new ) { PLearn::deepRead(in, old2new, intValue); }
  inline void doSomething(){ intValue += intValue*intValue; }
  inline bool operator==(const Integer& i) const { return i.intValue == intValue; }
  inline operator string() const { return tostring(intValue); }
  
private:	
  Integer& operator= (Integer i){ return *this; }
};

inline void deepWrite(ostream& out, DeepWriteSet& already_saved, const Integer& i)
{ i.deepWrite(out, already_saved); }
inline void deepWrite(ostream& out, DeepWriteSet& already_saved, Integer* iptr)
{ iptr->deepWrite(out, already_saved); }
inline void deepRead(istream& in, DeepReadMap& old2new, Integer& i)
{ i.deepRead(in, old2new); }
inline void deepRead(istream& in, DeepReadMap& old2new, Integer* iptr)
{ iptr->deepRead(in, old2new); }

inline ostream& operator<<(ostream& out, Integer i) 
{
  out << i.intValue << flush;
  return out;
} 

class Real
{
public:
  real realValue;
  inline Real(int r): realValue(r){}
};

/*!
  Class builded to test the Storage class (Storage.h).
  All boolean test{methods} are steps of the test. Each of these return true if the
  subtest succeded or false if it failed
  Class T must be conform to Integer interface TBModify
*/
template <class T>
class TestStorage
{
 public:
  //! Array of Storage objects
  Storage<T*>** storageFromData;
  Storage<T*>** storageFromPtr;
  Storage<T*>** storageFromStorage;
  Storage<T*>** storageWithPointTo;	
  Storage<T*>** storageFromFile; 	//!< Not implemented yet
  T** objects[NB_STORAGES];
  enum index{ fromPtr, fromStorage, withPT, fromFile };
  
  //! Length of storageObjects array
  int arrayLength;
  int nbObjects;

  //! Takes a seed: to be able to replay a failed test
  TestStorage(long seed){
	srand(seed);
	arrayLength = (int)NB_STORAGES;		//!< will be set like that for now
	nbObjects = (int)NB_OBJ;			//!< will be set like that for now
  
  storageFromData = new Storage<T*>*[arrayLength];
	storageFromPtr = new Storage<T*>*[arrayLength];
	storageFromStorage = new Storage<T*>*[arrayLength];
	storageWithPointTo = new Storage<T*>*[arrayLength];
	//storageFromFile = new Storage<T*>*[arrayLength];

  for(int sto=0; sto < arrayLength; sto++)
    {
      objects[sto] = new T*[nbObjects];
      for(int obj=0; obj<nbObjects; obj++){
        objects[sto][obj] = new T(10*sto + obj);
      }
    } 
  }

  //! Tests all ctor & pointTo method
  bool testCtor();
  
  //! Tests deepCopy procedure
  bool testDeepCopy();
  
  //! Tests length & resize methods. Also tests the mapping
  bool testManagement();
  
  //! Tests deepWrite and deepRead methods
  bool testSerialization();
  
  //! Tests the correctness of Storage objects deallocation
  bool testDeallocation(){return true;}

  ~TestStorage(){
    // Elements of the array were deleted in testDeallocation
    delete[] storageFromData;
    delete[] storageFromPtr;
    delete[] storageFromStorage;
    delete[] storageWithPointTo;
    //delete[] storageFromPtr;
  }

 private:
  void invariants();
  void verifyResize(Storage<T*>** array1, Storage<T*>** array2);
};

/*!
  Testing functions:   
  
  inline Storage(int the_length, T* dataptr)
  inline Storage(const Storage& other)

  inline Storage(int the_length=0)
  inline void pointTo(int the_length, T* dataptr)

  inline Storage(const char* filename, bool readonly)
  
  from Storage class
*/
template <class T>
bool
TestStorage<T>:: 
testCtor()
{
  cout << "Functions being tested:\n"
	   << "\t- Storage(int the_length, T* dataptr)\n"
	   << "\t- Storage(const Storage& other)\n"
	   << "\t- Storage(int the_length=0)\n"
	   << "\t- void pointTo(int the_length, T* dataptr)\n"
	   << "\t- Storage(const char* filename, bool readonly) - NOT YET!!!" << endl;

  int rdm = 0;
  
  for(int sto=0; sto < arrayLength; sto++)
	{
    storageFromData[sto] = new Storage<T*>( nbObjects );
    for(int obj=0; obj < nbObjects; obj++)
      storageFromData[sto]->data[obj] = objects[sto][obj];

	  storageFromPtr[sto] = new Storage<T*>( nbObjects, objects[sto] );
	  T_ASSERT(storageFromPtr[sto]->length() == nbObjects, 
			   (string)"Ctor with length did set length to " + tostring(storageFromPtr[sto]->length())
			   + (string)" instead of nbObjects, which is " + tostring(nbObjects)  					);
	  
	  storageFromStorage[sto] = new Storage<T*>( *storageFromPtr[sto] );
	  
	  rdm = random()%nbObjects;
	  if(rdm != 0)
		storageWithPointTo[sto] = new Storage<T*>( rdm );
	  else	//! To test Storage(int the_length=0) default arg
		storageWithPointTo[sto] = new Storage<T*>();
	  storageWithPointTo[sto]->pointTo(nbObjects, objects[sto]);
	}

  invariants();
  
  return true;		//! must see if we keep getting out whenever there is a problem
  //!					or if we print the ***FAILED*** message. We would have to
  //!					add a "bool result" member...
}

/*!
  TBModified - tests copy: does it really test DEEP copy
*/
template <class T>
bool 
TestStorage<T>:: 
testDeepCopy()
{
  //bla bla bla
}

template <class T>
bool
TestStorage<T>:: 
testManagement()
{
  cout << "Functions being tested:\n"
	   << "\t- void resize(int newlength)\n"
	   << "\t- length()\n"
	   << "\t- Also tests that two storages mapping to the same\n" 
	   << "\tcontents behave correctly after changes" << endl;

  cout << "Growing test..." << flush;
  T_PRECONDITION(storageFromData[0]->length() == nbObjects,
				 "Internal assumption error!"		);
  int sto = -1;
  int newSize = -1;
  for(sto=0; sto < arrayLength; sto++)
	{
	  newSize = rand()%( 2*nbObjects ) + (nbObjects + 1); //!< Then (nbObjects + 1) <= newSize <= (3*nbObjects + 1)
	  
	  storageFromData[sto]->resize( newSize );
	  T_ASSERT( storageFromData[sto]->length() == newSize, 
              (string)"resize on storageFromData[" + tostring(sto) + (string)"didn't affect length");
  }
  cout << "\tdone." << endl;

  
  cout << "Shrinking test..." << flush;
  for(sto=0; sto < arrayLength; sto++)
  {
	  storageFromData[sto]->resize( nbObjects );
	  T_ASSERT( storageFromData[sto]->length() == nbObjects, 
              (string)"resize on storageFromData[" + tostring(sto) + (string)"didn't affect length");
  }  
  cout << "\tdone." << endl;

  invariants();
  
  return true;
}

 
//! Tests deepWrite and deepRead methods
template <class T>
bool 
TestStorage<T>:: 
testSerialization()
{
#if defined(_MINGW_)
  T_PRECONDITION(storageFromStorage[0]->fd<=0 && !storageFromStorage[0]->dont_delete_data, 
				 "Internal assumption error"										); //!<  we are not using a memory-mapped file
#else
  T_PRECONDITION(storageFromStorage[0]->fd<0 && !storageFromStorage[0]->dont_delete_data, 
				 "Internal assumption error"										); //!<  we are not using a memory-mapped file
#endif

  ofstream* outTestFile = new ofstream(SERIAL_FILE);  

  for(int sto=0; sto < arrayLength; sto++)
	{
	  deepWrite(*outTestFile, *storageFromData[sto]);
	  delete storageFromData[sto]; 
	}
  delete outTestFile;
  

  ifstream* inTestFile = new ifstream(SERIAL_FILE);
  for(int sto=0; sto < arrayLength; sto++)
	{
	  storageFromData[sto] = new Storage<T*>(nbObjects);
    for(int obj=0; obj < nbObjects; obj++)
      storageFromData[sto]->data[obj] = new Integer();
	  deepRead(*inTestFile, *storageFromData[sto]);		// CORE DUMP !!!
	}
  delete inTestFile;
  
  invariants();

  return true;
}

template <class T>
void
TestStorage<T>:: 
invariants()
{
  cout << "Testing invariants..." << endl;

  for(int sto=0; sto < arrayLength; sto++)
	{
    int lpd = storageFromData[sto]->length();
	  int lptr = storageFromPtr[sto]->length();
	  int lsto = storageFromStorage[sto]->length();
	  int lpoint = storageWithPointTo[sto]->length();
    
    T_PRECONDITION( lpd == nbObjects, "Internal assumption error");
    
    //! Comparison of length between storageFromPtr & storageFromStorage: 
	  //! 	supposed to be equal! 
	  T_ASSERT( lpd == lptr, 
              (string)"Length error: sto= " + tostring(sto) + (string)", \nstorageFromData[sto]->length() =" +
              tostring(lpd) + (string)", \nstorageFromPtr[sto]->length() =" + tostring(lptr)		);
	  
	  //! Array comparison between storageFromPtr & storageFromStorage
	  for(int obj=0; obj < nbObjects; obj++)
      T_ASSERT( storageFromData[sto]->data[obj] == storageFromPtr[sto]->data[obj],	
                //ptr_array2string(storageFromData[sto]->data[obj], lptr) + (string)"\nis not equal to\n" +
                //ptr_array2string(storageFromPtr[sto]->data[obj], lsto) + (string)"\nsto=" + 
                "sto=" + tostring(sto) + (string)", obj=" + tostring(obj) );
    
    
	  //! Comparison of length between storageFromPtr & storageFromStorage: 
	  //! 	supposed to be equal! 
	  T_ASSERT( lptr == lsto, 
              (string)"Length error: sto= " + tostring(sto) + (string)", \nstorageFromPtr[sto]->length() =" +
              tostring(lptr) + (string)", \nstorageFromStorage[sto]->length() =" + tostring(lsto)		);
	  
	  //! Array comparison between storageFromPtr & storageFromStorage
	  for(int obj=0; obj < nbObjects; obj++)
      T_ASSERT( storageFromPtr[sto]->data[obj] == storageFromStorage[sto]->data[obj],	
              //ptr_array2string(storageFromPtr[sto]->data[obj], lptr) + (string)"\nis not equal to\n" +
              //ptr_array2string(storageFromStorage[sto]->data[obj], lsto) + (string)"\nsto=" + 
              "sto=" + tostring(sto) + (string)", obj=" + tostring(obj) );
	  
	  //! Comparison of length between storageFromPtr & storageWithPointTo: 
	  //! 	supposed to be equal! 
	  T_ASSERT( lptr == lpoint, 
              (string)"Length error: sto= " + tostring(sto) + (string)", \nstorageFromPtr[sto]->length() =" +
              tostring(lptr) + (string)", \nstorageWithPointTo[sto]->length() =" + tostring(lpoint)		);
	  
	  //! Array comparison between storageFromPtr & storageWithPointTo
	  for(int obj=0; obj < nbObjects; obj++)
      T_ASSERT( storageFromPtr[sto]->data[obj] == storageWithPointTo[sto]->data[obj],	
                //ptr_array2string(storageFromPtr[sto]->data[obj], lptr) + (string)"\nis not equal to" +
                //ptr_array2string(storageWithPointTo[sto]->data[obj], lpoint) + (string)"\nsto=" + tostring(sto) +
                "sto=" + tostring(sto) + (string)", obj=" + tostring(obj));
	}
  
  cout << "Invariants ok." << endl;
}

#endif // TEST_STORAGE_H
