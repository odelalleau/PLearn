
#include "general.h"
#include "stringutils.h"
#include "fileutils.h"

#ifdef WIN32 // WIN32 est definie par VC++ 
#  include <windows.h> 
#else 
#  include <sys/time.h> 
#  include <unistd.h> 
#endif 

#ifndef Tgeneral_INC
#define Tgeneral_INC

#define REL_FEQUAL(a,b) ( FABS((a)-(b)) < (FABS(a)*FLOAT_THRESHOLD) )


// voir où et comment définir le CHECK_COMPILATION flag ... 

/*typedef bool (*TestFunction)(); 

inline void doTest(string name, TestFunction TF)
{
  cout << "Step:\t " << name << flush;
  
  cout << "Doing ..." << endl;
  
  bool result = TF();
  
  if(result){
	cout << "PASSED" << endl;
  }		
  else{
	cout << "**FAIL**" << endl;
  }	
}*/

bool __result = false;	//!< Maybe not clean, but it doesn't work otherwise
#define DO_TEST(__name, __testFunction)\
__result = false;\
cout << endl;\
cout << "Step:\t " << __name << endl;\
cout << "Doing ..." << endl << endl;\
__result = __testFunction;\
if(__result){ cout << "PASSED" << endl; }\
else{ cout << "****FAIL****" << endl; }\
cout << endl << endl;

/*
#ifdef USE_EXCEPTIONS // then it isn't defined in plerror.h ...
inline void send_file_line(char* file,int line) 
{ cerr<<"At "<<file<<":"<<line; }
#endif
*/

#define T_ERR(_Type__, _The_Message__)\
cerr << "\n============================================================" << endl;\
cerr << "" 		<< _Type__ << " " << flush;\
send_file_line(__FILE__, __LINE__);\
cerr << endl << _The_Message__ << endl;\
cerr << "============================================================\n" << endl;\
exit(1);

#define T_ILLEGAL(__Message_)\
T_ERR("T_ILLEGAL", __Message_);

#define T_PRECONDITION(__Test_, __Message_) \
if( !(__Test_) ){\
T_ERR("T_PRECONDITION", (__Message_) );}

#define T_POSTCONDITION(__Test_, __Message_)\
if( !(__Test_) ){\
T_ERR("T_POSTCONDITION", (__Message_) );}

#define T_ASSERT(__Test_, __Message_)\
if( !(__Test_) ){\
T_ERR("T_ASSERT", (__Message_) )};



template <class T>
string
array2string(T* array, int array_size)
{
  string s("[ ");
  for(int i=0; i<array_size; i++){
	s.append( tostring(array[i]) );
	s.append( " " );
  }
  s.append( "]" );
  return s;
}

template <class T>
string
ptr_array2string(T* array, int array_size)
{
  string s("[ ");
  for(int i=0; i<array_size; i++){
    s.append( tostring(*array[i]) );
    s.append( " " );
  }
  s.append( "]" );
  return s;
}

template <class T>
void
print_array(ostream& out, T* array, int array_size)
{
  out << array2string(array, array_size) << flush; 
}

template <class T>
void
print_ptr_array(ostream& out, T* array, int array_size)
{
  out << ptr_array2string(array, array_size) << flush; 
}


/*! 
  Returns currentTime in ms since that famous date ... 
  \warning MS version may be incorrect
*/ 
inline long current_time() 
{ 
#ifdef WIN32 
  // tentative de ma part ici, je ne suis pas certains que ca 
  // fonctionne; j'ai pris ca du code source de Qt, dans qdatetime.cpp 
  SYSTEMTIME t; 
  GetLocalTime( &t ); 
  return 3600000 * t.wHour + 60000 * t.wMinute + 1000 * t.wSecond + t.wMilliseconds; 
#else 
  struct timeval tv; 
  gettimeofday( &tv, NULL ); 
  return tv.tv_sec * 1000 + tv.tv_usec / 1000; 
#endif
}

template <class T>
inline string getStringFrom(const T& x)
{
  return tostring(x);
}

inline string raw_input(string msg, istream& in = cin)
{
  cout << msg << flush;

  char* line = new char[128];
  in.getline(line, 128);
  string the_line(line);
  delete line;
  return the_line;
}

#endif
