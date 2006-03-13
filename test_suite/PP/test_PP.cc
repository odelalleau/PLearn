
/* *******************************************************      
   * $Id$
   * AUTHORS: Kim Levy & Christian Dorion
   * This file is part of the PLearn library.
   ******************************************************* */

/*! \file PLearnLibrary/TestSuite/test_PP.cc */
/*! 
  Tests the constructors and methods of PP.cc  
*/
#ifndef TEST_PP_CC
#define TEST_PP_CC

#include "../t_general.h"
#include <plearn/base/PP.h>

#define MAX_SIZE 11

using namespace PLearn;

class ChildA;
class ChildB;

class Parent : public PPointable
{		
public:
  //! integer in the hundreds
  int i_par;
  //! string representing number in the hundreds
  string s_par;
  PP<ChildA> childAPP;
  PP<ChildB> childBPP;
  Parent(int ipp=0, string spp="parent") : i_par(ipp), s_par(spp) {}
  void print(ostream& out) { out << "* Parent:\ti_par= " << i_par << "\ts_par= " << s_par << endl; }
  
  Parent(const Parent& other) : i_par(other.i_par), s_par(other.s_par), 
	childAPP(other.childAPP), childBPP(other.childBPP){}

  Parent* deepCopy(CopiesMap& copies) const
  {
    CopiesMap::iterator it = copies.find(this);
    if(it!=copies.end())  //!<  a copy already exists, so return it
      return (Parent*) it->second;
	
    //!  Otherwise call the copy constructor to obtain a copy
    Parent* dcopy = new Parent(*this);
    dcopy->childAPP = PLearn::deepCopy( childAPP );
    dcopy->childBPP = PLearn::deepCopy( childBPP );
	
    //!  Put the copy in the map
    if (usage() > 1)
      copies[this] = dcopy;
	
    return dcopy;
  } 	
};

class ChildA : public Parent
{
public:
  //! regular integer
  int i_child_a;
  PP<Parent> parentPP;
  PP<ChildB> childBPP; 
  ChildA(int pp=0, string sp="zero_hundred", int cc=0) : Parent(pp, sp), i_child_a(cc) {}
  void print(ostream& out) { out << "* ChildA:\ti_child_a= " << i_child_a << "\t\t\t" << flush; Parent::print(out); }

  ChildA(const ChildA& other) : 
	Parent(other.i_par, other.s_par), 
	parentPP(other.parentPP), childBPP(other.childBPP){}

  ChildA* deepCopy(CopiesMap& copies) const
  {
	CopiesMap::iterator it = copies.find(this);
	if(it!=copies.end())  //!<  a copy already exists, so return it
      return (ChildA*) it->second;

	//!  Otherwise call the copy constructor to obtain a copy
    ChildA* dcopy = new ChildA(*this);
    dcopy->parentPP = PLearn::deepCopy( parentPP );
    dcopy->childBPP = PLearn::deepCopy( childBPP );
	
    //!  Put the copy in the map
    if (usage() > 1)
      copies[this] = dcopy;
	
    return dcopy;
  }
};


class ChildB : public Parent
{
public:
  //! string representing number
  string s_child_b;
  PP<ChildA> childAPP;
  ChildB(string schildb="childB") : s_child_b(schildb) {}
  void print(ostream& out) { out << "* ChildB:\ts_child_b= " << s_child_b << "\t\t" << flush; Parent::print(out); }
  
  ChildB(const ChildB& other) : Parent(other.i_par, other.s_par), childAPP(other.childAPP){}
 
  ChildB* deepCopy(CopiesMap& copies) const
  {
	CopiesMap::iterator it = copies.find(this);
	if(it!=copies.end())  //!<  a copy already exists, so return it
      return (ChildB*) it->second;

	//!  Otherwise call the copy constructor to obtain a copy
    ChildB* dcopy = new ChildB(*this);
	dcopy->childAPP = PLearn::deepCopy( childAPP );
	
    //!  Put the copy in the map
    if (usage() > 1)
      copies[this] = dcopy;
	
    return dcopy;
  }
};

class Other : public PPointable
{
public:
  int i_other;
  string s_other;
  Other(int iother=0, string sother="other") : i_other(iother), s_other(sother) {}
};


class Test_PP
{
public:
  PP<ChildA>* child_a_array;
  PP<ChildB>* child_b_array;
  PP<Parent>* parent_array;
  PP<Other> otherPP; 
  int size_a;
  int size_b;
  int size_p;
  
  Test_PP()
  {
	child_a_array = new PP<ChildA>[MAX_SIZE];
	child_b_array = new PP<ChildB>[MAX_SIZE];
	parent_array = new PP<Parent>[MAX_SIZE];
	otherPP = new Other(99, "ninety-nine");
	cout << endl << "***** STARTING TEST_PP *****" << endl << endl;
	size_a = 0;
	size_b = 0;
	size_p = 0;
  }

  /*! Checking PP constructors 
	and operator-> by accessing a PPointable method(usage()) after each modification
  */  
  bool emptyCons();
  bool copieConsOrdinaryPtr();
  bool copieConsSameTypePP();
  bool copieConsChildPP();
  bool illegalCopieCons();
  
 
  //! Checking PP methods  
  bool conversionOrdinaryPtr(); // tests: PP::operator T*() const
  bool accessPointedObject(); // tests: T& PP:operator*() const
  bool affectationFromOrdPtr(); // tests: void PP::operator=(const T* otherptr)
  bool affectationFromPP(); // tests: void PP::operator=(const PP<T>& other)
  bool tryDeepCopy(); // tests;  inline T* deepCopy(PP<T> source)

private:
  //! "oddIndex" is the number of references on child_a_array[odd], child_b_array[even]
  //! "evenIndex" is the number of references on the PP's object of even indexes
  void invariants(int oddIndex, int evenIndex);
  //! sous-tests de conversionOrdinaryPtr()
  bool conversionOPchildA();
  bool conversionOPchildB();
  bool conversionOPparent();
}; 


// ----------------- tests on constructors --------------------------------------
//! Emtpy constructor test
bool Test_PP::emptyCons()
{  
  //! test for class ChildA
  PP<ChildA> childAPP0;
  child_a_array[0] = childAPP0;
  size_a++;
  cout << "  - Empty constructor verified for class ChildA " << endl;
  
  //! test for class ChildB
  PP<ChildB> childBPP0;
  child_b_array[0] = childBPP0;
  size_b++;
  cout << "  - Empty constructor verified for class ChildB " << endl;

  //! test for class Parent
  PP<Parent> parentPP0;
  parent_array[0] = parentPP0;
  size_p++;
  cout << "  - Empty constructor verified for class Parent " << endl;
  cout << "--- Empty constructor verified ---" << endl << endl;
  return true;
}

//! Copie constructor with ordinary ptr test
bool Test_PP::copieConsOrdinaryPtr()
{
  //! tests for class ChildA
  PP<ChildA> childAPP1 = new ChildA(1, "one_hundred",  100); 
  T_ASSERT(childAPP1->usage()==1 ,
           "Error in call to ->usage() while testing ChildA's \n"
           "copie constructors with ordinary ptr");
  child_a_array[1] = childAPP1;    
  size_a++;
  
  ChildA childA2;
  childA2.i_child_a = 2;
  childA2.i_par = 200;
  childA2.s_par = "two_hundred";
  ChildA* ptr2= new ChildA(childA2);
  PP<ChildA> childAPP2(ptr2);
  T_ASSERT(childAPP2->usage()==1 ,"Error in call to ->usage() while testing ChildA's \ncopie constructors with ordinary ptr");
  child_a_array[2] = childAPP2; 
  size_a++;
  cout << "  - Copie constructor with ordinary pointers verified for class ChildA" << endl;


  //! tests for class ChildB
  PP<ChildB> childBPP1 = new ChildB("one");     
  T_ASSERT(childBPP1->usage()==1 ,"Error in call to ->usage() while testing ChildB's \ncopie constructors with ordinary ptr");
  child_b_array[1] = childBPP1;
  size_b++; 

  ChildB childB2;
  childB2.s_child_b = "two__";
  childB2.i_par = 200;
  childB2.s_par = "two_hundred";
  ChildB* ptr22 = new ChildB(childB2);
  PP<ChildB> childBPP2(ptr22);  
  T_ASSERT(childBPP2->usage()==1 ,"Error in call to ->usage() while testing ChildB's \ncopie constructors with ordinary ptr");
  child_b_array[2] = childBPP2;
  size_b++;
  cout << "  - Copie constructor with ordinary pointers verified for class ChildB" << endl;

  //! tests for class Parent
  PP<Parent> parentPP1 = new Parent(100, "one_hundred" );   
  T_ASSERT(parentPP1->usage()==1 ,"Error in call to ->usage() while testing Parent's \ncopie constructors with ordinary ptr");
  parent_array[1] = parentPP1; 
  size_p++; 

  Parent parent2;
  parent2.i_par=200;
  parent2.s_par="two_hundred";
  Parent* ptr200 = new Parent(parent2);
  PP<Parent> parentPP2(ptr200); 
  T_ASSERT(parentPP2->usage()==1 ,"Error in call to ->usage() while testing Parent's \ncopie constructors with ordinary ptr");
  parent_array[2] = parentPP2; 
  size_p++;
  cout << "  - Copie constructor with ordinary pointers verified for class Parent" << endl;

  cout << "--- Copie constructor with ordinary pointers verified ---" << endl << endl;
  return true;
}

//! Copie constructor with same type of PP
bool Test_PP::copieConsSameTypePP(){
  //! tests for class ChildA
  PP<ChildA> childAPP3(child_a_array[1]);
  T_ASSERT(child_a_array[1]->usage()==2 && childAPP3->usage()==2 ,
		   "Error in call to ->usage() while testing ChildA's \ncopie constructors with same type of PP");
  child_a_array[3] = childAPP3;
  size_a++;

  PP<ChildA> childAPP4(child_a_array[2]);
  T_ASSERT(child_a_array[2]->usage()==2 && childAPP4->usage()==2 ,
		   "Error in call to ->usage() while testing ChildA's \ncopie constructors with same type of PP");
  child_a_array[4] = childAPP4;
  size_a++;
  cout << "  - Copie constructor with same type of PP verified for class ChildA " << endl;

  //! tests for class ChildB
  PP<ChildB> childBPP3(child_b_array[1]);
  T_ASSERT(child_b_array[1]->usage()==2 && childBPP3->usage()==2,
		   "Error in call to ->usage() while testing ChildB's \ncopie constructors with same type of PP");
  child_b_array[3] = childBPP3;
  size_b++;

  PP<ChildB> childBPP4(child_b_array[2]);
  T_ASSERT(child_b_array[2]->usage()==2 && childBPP4->usage()==2,
		   "Error in call to ->usage() while testing ChildB's \ncopie constructors with same type of PP");
  child_b_array[4] = childBPP4;
  size_b++;
  cout << "  - Copie constructor with same type of PP verified for class ChildB" << endl;

  //! tests for class Parent
  PP<Parent> parentPP3(parent_array[1]);
  T_ASSERT(parent_array[1]->usage()==2 && parentPP3->usage()==2,
		   "Error in call to ->usage() while testing Parent's \ncopie constructors with same type of PP");
  parent_array[3] = parentPP3;
  size_p++;

  PP<Parent> parentPP4(parent_array[2]);
  T_ASSERT(parent_array[2]->usage()==2 && parentPP4->usage()==2  ,
		   "Error in call to ->usage() while testing Parent's \ncopie constructors with same type of PP");
  parent_array[4] = parentPP4;
  size_p++;
  cout << "  - Copie constructor with same type of  PP verified for class Parent" << endl;
  cout << "--- Copie constructor with other PPs verified ---" << endl << endl; 
  return true;
}


//! Copie constructor with other(child) type of PP
bool Test_PP::copieConsChildPP()
{
  //! for class Parent tests constructor with existing children
  PP<Parent> parentPP5(child_a_array[1]);
  T_ASSERT(child_a_array[1]->usage()==3 && child_a_array[3]->usage()==3 && parentPP5->usage()==3 ,
		   "Error in call to ->usage() while testing Parent's \ncopie constructors with other type of PP");
  parent_array[5]= parentPP5;
  size_p++;

  PP<Parent> parentPP6(child_b_array[2]);
  T_ASSERT(child_b_array[2]->usage()==3 && child_b_array[4]->usage()==3 && parentPP6->usage()==3 ,
		   "Error in call to ->usage() while testing Parent's \ncopie constructors with other type of PP");
  parent_array[6]= parentPP6;
  size_p++;

  cout << "--- Copie constructor with other PP type verified (for class Parent)---" << endl << endl;
  return true;	
}


//! Illegal constructors
bool Test_PP::illegalCopieCons()
{
#ifdef CHECK_COMPILATION
  //! check compilation for class ChildA: copie constructor with an existing parent
  PP<ChildA> childAPP4(parent_array[2]);
  child_a_array[4] = childAPP4;

  //! check compilation for class ChildB: copie construction with an other child
  PP<ChildA> childAPP4(child_a__array[2]);
  child_a_array[4] = childAPP4;

  //! check compilation for class Parent: copie constructor with an other PP class
  PP<Parent> parentPP4(otherPP);
  parent_array[4] = parentPP4;
  cout << "--- Illegal Copie constructors should not be verified: error has occured!---" << endl << endl;
#endif
  return true;
}
 

// ----------------- tests on methods ----------------------------------------------


//! tests conversion to ordinary ptrs and asserts that the access to the objects is still possible threw ptrs
// tests: PP::operator T*() const
bool Test_PP::conversionOrdinaryPtr()
{
  bool result = conversionOPchildA();
  invariants(3,2);

  result = result && conversionOPchildB();
  invariants(3,2);

  result = result && conversionOPparent();
  invariants(3,2);
 
  cout << "--- Conversion to ordinary ptr verified ---" << endl << endl;
  return true;
}

//! tests the dereferenciation of the PP's by calling print() on the pointed object 
//! ( array's start at [1] because [0] contains an empty PP)
// tests: T& PP:operator*() const
bool Test_PP::accessPointedObject()
{	  
  int i;
  // for ChildA
  cout << "Printing child_a_array:" << endl;
  for( i=1; i< size_a; i++)
	{
	  cout << "child_a_array[" << i << "]: " << flush;
	  (*child_a_array[i]).print(cout);
	}
  
  // for ChildB
  cout << endl << "Printing child_b_array:" << endl;
  for( i=1; i< size_b; i++)
	{
	  cout << "child_b_array[" << i << "]: " << flush;
	  (*child_b_array[i]).print(cout);
	}

  // for Parent
  cout << endl << "Printing parent_array:" << endl;
  for( i=1; i< size_p; i++)
	{
	  cout << "parent_array[" << i << "]: " << flush;
	  (*parent_array[i]).print(cout); 
	}
  cout << endl;
 
  return true;
}

//!  
// tests: void PP::operator=(const T* otherptr)
bool Test_PP::affectationFromOrdPtr()
{
  ChildA* aPtr = new ChildA(400, "four_hundred", 4);
  child_a_array[4] = aPtr;
  T_ASSERT(	child_a_array[4]->i_par==400 && child_a_array[2]->i_par==200, 
			"Error in affectationFromOrdPtr(): wrong ptr links in child_a_array" );
  
  ChildB* bPtr = new ChildB("one__");
  child_b_array[1] = bPtr;
  (*child_b_array[1]).i_par = 100;
  child_b_array[1]->s_par = "one_hundred";
  T_ASSERT( child_b_array[1]->i_par==100 && child_b_array[3]->i_par==300, 
			"Error in affectationFromOrdPtr(): wrong ptr links in child_b_array" );

  Parent* pPtr = new Parent(400, "four_hundred");
  parent_array[4] = pPtr;
  T_ASSERT( parent_array[4]->i_par==400 && parent_array[2]->i_par==200, 
			"Error in affectationFromOrdPtr(): wrong ptr links in parent_array" );
  pPtr = new Parent(300, "three_hundred");
  parent_array[3] = pPtr;
  T_ASSERT( parent_array[1]->i_par==100 && parent_array[3]->i_par==300, 
			"Error in affectationFromOrdPtr(): wrong ptr links in parent_array" );
  pPtr = new Parent(600, "six_hundred");
  parent_array[6]= pPtr;
  T_ASSERT(	child_b_array[2]->i_par!=600 && child_b_array[4]->i_par!=600, 
			"Error in affectationFromOrdPtr(): wrong ptr links between child_b_array and parent_array" );
  
  parent_array[6]= child_b_array[2];


  // after affectationFromOrdPtr()...
  // a: {[1][3]p[5], [2], [4]}  
  // b:{[1], [3], [2][4]p[6]} 
  // p: {[1], [2], [3], [4], [5]a[1][3], [6]b[2][4]}
  accessPointedObject();
  invariants(3,1);
  
  return true;
}


// tests: void PP::operator=(const PP<T>& other)
bool Test_PP::affectationFromPP()  
{
  // for ChildA
  parent_array[7]= new Parent(700, "seven_hundred");
  size_p++;
  parent_array[8]= new Parent(800, "eight_hundred");
  size_p++;
  parent_array[8]= child_a_array[4];
  parent_array[7]= child_a_array[4];
  child_a_array[2] = child_a_array[4];
  T_ASSERT(	child_a_array[2]->i_par==child_a_array[4]->i_par ,
			"1-Error in affectationFromPP(): wrong ptr links in child_a_array"	);
  T_ASSERT(	child_a_array[4]->i_par==parent_array[7]->i_par && parent_array[7]->i_par==parent_array[8]->i_par,
			"2-Error in affectationFromPP(): wrong ptr links in child_a_array"	);
  
  // for ChildB
  parent_array[10]= new Parent(10, "ten_hundred!!");
  parent_array[9]=new Parent(900, "nine_hundred");
  size_p+=2;
  parent_array[10]= child_b_array[3]; 
  parent_array[9]= child_b_array[3];
  child_b_array[1] = child_b_array[3];
  T_ASSERT(	child_b_array[1]->i_par==child_b_array[3]->i_par && child_b_array[1]->i_par==parent_array[9]->i_par
			&& parent_array[9]->i_par==parent_array[10]->i_par,
			"3-Error in affectationFromPP(): wrong ptr links in child_b_array"	);

  // for Parent
  parent_array[4] = parent_array[1];
  parent_array[3] = parent_array[4];
  parent_array[2] = parent_array[4];
  T_ASSERT(	parent_array[1]->i_par==parent_array[2]->i_par,
			"4-Error in affectationFromPP(): wrong ptr links in parent_array"	);
  T_ASSERT( parent_array[2]->i_par==parent_array[3]->i_par && parent_array[3]->i_par==parent_array[4]->i_par,
			"5-Error in affectationFromPP(): wrong ptr links in parent_array"	);
  
  accessPointedObject();
  invariants(3,4);

  return true;
}


/*! Creates a dependance pattern between PP<Parent>, PP<ChildA> and PP<ChilB>,
  copies the root of the pattern by deepCopy and verifies the copie's pattern.
 */
bool Test_PP::tryDeepCopy()
{ 
  /* creating the pattern:
                     :  -parentPP -->                     :  -parentPP -->  
	child_a_array[i]         |         child_a_array[i+1}         |          etc.       && parentPP: ->childBPP
                     :  -childBB  -->                     :  -childBB  -->
  */
  cout << "- creating pattern..." << endl;
  child_a_array[1] = new ChildA(100, "one_hundred", 1);
  child_a_array[2] = new ChildA(300, "three_hundred", 3);

  child_a_array[1]->parentPP = new Parent(200,"two_hundred");
  child_a_array[1]->childBPP = new ChildB("two");
  child_a_array[1]->parentPP->childAPP = child_a_array[2];
  child_a_array[1]->childBPP->childAPP = child_a_array[2];
  child_a_array[1]->parentPP->childBPP = child_a_array[1]->childBPP;
 

  cout << "- deep copying..." << endl;
  //! deep copying the root 
  ChildA* t = PLearn::deepCopy(child_a_array[1]);
  PP<ChildA> root_copy = t;

  cout << "- verifying different levels of the deep copy..." << endl;
  //! 1rst T_ASSERT :asserts that the changes applied to childA through parent are respected within ChildB
  //! 2nd T_ASSERT :asserts that the changes applied to childB through parent are respected within ChildA
  PP<ChildA> root_copy2 = root_copy->parentPP->childAPP;
  root_copy2->i_child_a = 2;
  T_ASSERT(root_copy2->i_child_a == root_copy->childBPP->childAPP->i_child_a,
		   "Error, the copy does not respect the initial map - root_copy1, i_child_a");
  PP<ChildB> root_copy3 = root_copy->parentPP->childBPP;
  root_copy3->s_child_b = "two";
  T_ASSERT(root_copy3->s_child_b == root_copy->childBPP->s_child_b,
			"Error, the copy does not respect the initial map - root copy1, s_child_b");

// 	 int i, j; 
//   for(i=2; i<size_a && i<size_b; i++)
// 	{
// 	  j = 2*(i-1);
// 	  child_a_array[i-1]->parentPP = new Parent(j*100, tostring(j)+(string)"_hundred");
// 	  child_a_array[i-1]->childBPP = new ChildB( tostring(j)+(string)"s");
// 	  child_a_array[i-1]->parentPP->childAPP = child_a_array[i];
// 	  child_a_array[i-1]->childBPP->childAPP = child_a_array[i];
// 	  child_a_array[i-1]->parentPP->childBPP = child_a_array[i-1]->childBPP;
// 	}
  
//   cout << "- deep copying the root..." << endl;
//   //! deep copying the root 
//   ChildA* test = PLearn::deepCopy(child_a_array[3]);
//   PP<ChildA> root_copy = test;

//   cout << "- verifying different levels of the deep copy..." << endl;
//   //! 1rst T_ASSERT :asserts that the changes applied to childA through parent are respected within ChildB
//   //! 2nd T_ASSERT :asserts that the changes applied to childB through parent are respected within ChildA
//   PP<ChildA> root_copy2 = root_copy->parentPP->childAPP;
//   root_copy2->i_child_a = 2;
//   T_ASSERT(root_copy2->i_child_a == root_copy->childBPP->childAPP->i_child_a,
// 	 		"Error, the copy does not respect the initial map - root_copy1, i_child_a");
//   root_copy->parentPP->childBPP->s_child_b = "two";
//   T_ASSERT(root_copy->parentPP->childBPP->s_child_b == root_copy->childBPP->s_child_b,
// 			"Error, the copy does not respect the initial map - root copy1, s_child_b");

//   cout << "- root_copy1 done" << endl;
  
//   PP<ChildA> root_copy3 = root_copy2->parentPP->childAPP;
//   root_copy3->i_child_a = 4;
//   T_ASSERT(root_copy3->i_child_a == root_copy2->childBPP->childAPP->i_child_a,
// 	 		"Error, the copy does not respect the initial map - root_copy2, i_child_a");
//   root_copy2->parentPP->childBPP->s_child_b = "four";
//   T_ASSERT(root_copy->parentPP->childBPP->s_child_b == root_copy->childBPP->s_child_b,
// 			"Error, the copy does not respect the initial map - root copy2, s_child_b");

//   cout << "- root_copy2 done" << endl;
   
//   PP<ChildA> root_copy4 = root_copy3->parentPP->childAPP;
//   root_copy4->i_child_a = 6;
//   T_ASSERT(root_copy4->i_child_a == root_copy3->childBPP->childAPP->i_child_a,
// 	 		"Error, the copy does not respect the initial map - root_copy3, i_child_a");
//   root_copy3->parentPP->childBPP->s_child_b = "six";
//   T_ASSERT(root_copy->parentPP->childBPP->s_child_b == root_copy->childBPP->s_child_b,
// 			"Error, the copy does not respect the initial map - root copy3, s_child_b");

//   cout << "- root_copy3 done" << endl;

  return true;
} 






// ------------ private methods ---------------------------------------------------------

//! Checking invariants
void Test_PP::invariants(int nb1, int nb2)
{
  T_ASSERT(child_a_array[1]->usage()==nb1 && child_a_array[3]->usage()==nb1 && parent_array[5]->usage()==nb1,
		   "1-Error in call to ->usage() in invariants()"); 
  T_ASSERT(child_a_array[2]->usage()==nb2 && child_a_array[4]->usage()==nb2,
		   "2-Error in call to ->usage() in invariants()");
  T_ASSERT(child_b_array[1]->usage()==nb2 && child_b_array[3]->usage()==nb2,
		   "3-Error in call to ->usage() in invariants() " + tostring(child_b_array[1]->usage()));
  T_ASSERT(child_b_array[2]->usage()==nb1 && child_b_array[4]->usage()==nb1 && parent_array[6]->usage()==nb1 ,
		   "4-Error in call to ->usage() in invariants()");
  T_ASSERT(parent_array[1]->usage()==nb2 && parent_array[3]->usage()==nb2,
		   "5-Error in call to ->usage() in invariants()");
  T_ASSERT(parent_array[2]->usage()==nb2 && parent_array[4]->usage()==nb2 ,
		   "6-Error in call to ->usage() in invariants()");
}

//! Checking conversion to the ordinary pointer in class ChildA
bool Test_PP::conversionOPchildA()
{
  string key = "hundred";
  // for class ChildA
  ChildA* a = child_a_array[3];
  int sum = (*a).i_child_a + a->i_par; 
  T_ASSERT(sum==101, "The value of the integer fields in class ChildA are wrong:\n\t\tsum="
		   + tostring(sum) + " ,!=101");
  int pos = (*a).s_par.find (key,0); // one_hundred: "hundred" in 4th pos
  T_ASSERT(pos==4,"The value of the string fields in class ChildA are wrong:\n\t\t pos="
		   + tostring(pos) + " ,!=4");

  //after modifiying the fields
  a->i_child_a=3;
  (*a).i_par=300;
  a->s_par="three_hundred";
  sum = a->i_child_a + (*a).i_par;
  T_ASSERT(sum==303, "The value of the integer fields in class ChildA are wrong:\n\t\tsum="
		   + tostring(sum) + " ,!=303");   
  pos = (*a).s_par.find (key,0); // three_hundred: "hundred" in 6th pos
  T_ASSERT(pos==6,"The value of the string fields in class ChildA are wrong:\n\t\t pos="
		   + tostring(pos) + " ,!=6");
  cout << "  - Conversion to ordinary ptr verified for class ChildA" << endl;
  return true;
}

//! Checking conversion to the ordinary pointer in class ChildB
bool Test_PP::conversionOPchildB()
{
  string key = "hundred";
  // for class ChildB
  ChildB* b = child_b_array[1];
  string key2 = "e";
  int pos = (*b).s_child_b.find(key2,0); // one: "e" in 2nd pos
  T_ASSERT(pos==2,"The value of the string fields in class ChildB are wrong:\n\t\t pos=" 
		   + tostring(pos)+ " ,!=2");
  b->i_par = 100;
  b->s_par = "one_hundred";
  T_ASSERT( b->i_par==100,"The value of the integer fields in class ChildB are wrong:\n\t\ti_par="
			+ tostring(b->i_par)+ " ,!=100"); 
  
  // after modifiying the fields
  b->i_par = 300;
  b->s_par = "three_hundred";
  b->s_child_b = "three";
  T_ASSERT( b->i_par==300,"The value of the integer fields in class ChildB are wrong:\n\t\ti_par=" 
			+ tostring(b->i_par) + " ,!=300");     
  pos = (*b).s_par.find(key2,0); // three_hundred: "hundred" in 6th pos
  T_ASSERT(pos==3,"The value of the string fields in class ChildB are wrong:\n\t\t pos=" 
		   +tostring(pos) + " ,!=3");  
  pos = (*b).s_child_b.find(key2,0); // three_hundred: "e" in 3rd pos
  T_ASSERT(pos==3,"The value of the string fields in class ChildB are wrong:\n\t\t pos=" 
		   + tostring(pos)+ " ,!=3");
  
  cout << "  - Conversion to ordinary ptr verified for class ChildB" << endl;
  return true;
}

//! Checking conversion to the ordinary pointer in class Parent
bool Test_PP::conversionOPparent()
{
  string key = "hundred";
  // for class Parent
  Parent* p = parent_array[5];

  //p->i_par should have value 300 because of it's connection to [3]
  T_ASSERT( p->i_par==300,"The value of the integer fields in class Parent are wrong:\n\t\ti_par=" 
			+ tostring(p->i_par) + " ,!=300");
  int pos = p->s_par.find(key,0); // three_hundred: "hundred" in 6th pos
  T_ASSERT(pos==6,"The value of the string fields in class Parent are wrong:\n\t\tpos=" 
		   + tostring(pos)+ " ,!=6"); 
  
  // after modifiying the fields
  p->s_par="five_hundred";
  p->i_par=500;
  pos = p->s_par.find(key,0); // five_hundred: "hundred" in 5th pos
  T_ASSERT(pos==5,"The value of the string fields in class Parent are wrong:\n\t\tpos=" 
		   + tostring(pos)+ " ,!=5");  
  T_ASSERT( p->i_par==500,"The value of the integer fields in class Parent are wrong:\n\t\ti_par=" 
			+ tostring(p->i_par) + " ,!=500");

  p = child_a_array[3]; //p->i_par should have value 500 because of it's connection to [3]and[5]
  T_ASSERT( p->i_par==500,"The value of the integer fields in class Parent are wrong:\n\t\ti_par=" 
			+ tostring(p->i_par) + " ,!=500");

  p = child_a_array[1]; //p->i_par should have value 500 because of it's connection to [3]and[5]
  T_ASSERT( p->i_par==500,"IciThe value of the integer fields in class Parent are wrong:\n\t\ti_par=" 
			+ tostring(p->i_par) + " ,!=500");
    
  
  cout << "  - Conversion to ordinary ptr verified for class Parent" << endl;
  return true;
}











// --------------- main ------------------------------------------------------

int main(int argc, char** argv)
{
  Test_PP tpp;

  cout << "Launching Test_TVec..." << endl;
  
  DO_TEST("Empty Constructors", tpp.emptyCons());
  DO_TEST("Copie Constructors with Ordinary Pointers", tpp.copieConsOrdinaryPtr());
  DO_TEST("Copie Constructors with same PP type pointers",tpp.copieConsSameTypePP());
  DO_TEST("Copie Constructors with other(child) PP type pointers",tpp.copieConsChildPP());
#ifdef CHECK_COMPILATION
  DO_TEST("Illegal Copie Constructors",tpp.illegalCopieCons());
#endif
  DO_TEST("Conversion to ordinary pointers",tpp.conversionOrdinaryPtr());
  DO_TEST("Access pointed objects",tpp.accessPointedObject());
  DO_TEST("Affectation from ordinary pointers", tpp.affectationFromOrdPtr());
  DO_TEST("Affectation between PP pointers",tpp.affectationFromPP());
  //! is not fonctionnal for the moment
  //DO_TEST("Deep Copy of a PP<childA> root",tpp.tryDeepCopy());

  cout << "Quitting Test_TVec..." << endl;
}

#endif //TEST_PP_CC
