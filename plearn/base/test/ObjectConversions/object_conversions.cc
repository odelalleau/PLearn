// Test object conversion functions

#include <iostream>

#include <plearn/base/ObjectConversions.h>
#include <plearn/base/Storage.h>
#include <plearn/math/StatsCollector.h>

using namespace std;

namespace PLearn 
{

void notConvertible()
{
  int x = 0;
  int* y = 0;
  Vec v;
  Mat m;
  Array<int> aint;
  TVec<int> vint;
  PPointable* p = 0;
  Storage<int> stor;
  PP<PLearn::Storage<int> > ppstor;

  cout << "*** SHOULD NOT BE CONVERTIBLE: ***" << endl;
  cout << "isConvertibleToObjectPtr(int)        : " << isConvertibleToObjectPtr(x)      << endl;
  cout << "isConvertibleToObjectPtr(int*)       : " << isConvertibleToObjectPtr(y)      << endl;
  cout << "isConvertibleToObjectPtr(Vec)        : " << isConvertibleToObjectPtr(v)      << endl;
  cout << "isConvertibleToObjectPtr(Mat)        : " << isConvertibleToObjectPtr(m)      << endl;
  cout << "isConvertibleToObjectPtr(Array<int>) : " << isConvertibleToObjectPtr(aint)   << endl;
  cout << "isConvertibleToObjectPtr(TVec<int>)  : " << isConvertibleToObjectPtr(vint)   << endl;
  cout << "isConvertibleToObjectPtr(PPointable*): " << isConvertibleToObjectPtr(p)      << endl;
  cout << "isConvertibleToObjectPtr(Storage)    : " << isConvertibleToObjectPtr(stor)   << endl;
  cout << "isConvertibleToObjectPtr(PP<Storage>): " << isConvertibleToObjectPtr(ppstor) << endl;
}

void convertible()
{
  Object* u = 0;
  StatsCollector sc;
  PP<StatsCollector> ppsc = new StatsCollector;
  StatsCollector*    psc  = ppsc;
  const StatsCollector* cpsc = ppsc;
  TVec<StatsCollector> vsc;
  TVec< PP<StatsCollector> > ppvsc;
  int i;

  cout << endl << "*** SHOULD BE CONVERTIBLE: ***" << endl;
  cout << "isConvertibleToObjectPtr(Object*)                    : " << isConvertibleToObjectPtr(u)     << endl;
  cout << "isConvertibleToObjectPtr(StatsCollector)             : " << isConvertibleToObjectPtr(sc)    << endl;
  cout << "isConvertibleToObjectPtr(StatsCollector*)            : " << isConvertibleToObjectPtr(psc)   << endl;
  cout << "isConvertibleToObjectPtr(const StatsCollector*)      : " << isConvertibleToObjectPtr(cpsc)  << endl;
  cout << "isConvertibleToObjectPtr(PP<StatsCollector>)         : " << isConvertibleToObjectPtr(ppsc)  << endl;
  cout << "isConvertibleToObjectPtr(TVec<StatsCollector>)       : " << isConvertibleToObjectPtr(vsc)   << endl;
  cout << "isConvertibleToObjectPtr(TVec< PP<StatsCollector> >) : " << isConvertibleToObjectPtr(ppvsc) << endl;

  cout << endl << "*** TEST CONVERSIONS: ***" << endl;
  cout << "toObjectPtr(Object*)               : " << toObjectPtr(u)    << endl
       << "toObjectPtr(StatsCollector)        : " << toObjectPtr(sc)   << endl
       << "toObjectPtr(StatsCollector) [2]    : " << toObjectPtr(*psc) << endl
       << "toObjectPtr(StatsCollector*)       : " << toObjectPtr(psc)  << endl
       << "toObjectPtr(const StatsCollector*) : " << toObjectPtr(cpsc) << endl
       << "toObjectPtr(PP<StatsCollector>)    : " << toObjectPtr(ppsc) << endl;

  try {
    cout << "toObjectPtr(int)                 : " << toObjectPtr(i) << endl;
  }
  catch (PLearnError e) {
    cout << "... caught error '" << e.message() << "'" << endl;
  }
}

void notIndexable()
{
  int x = 0;
  int* y = 0;
  Vec v;
  Mat m;
  Array<int> aint;
  TVec<int> vint;
  PPointable* p = 0;
  Storage<int> stor;
  PP<PLearn::Storage<int> > ppstor;
  Object* u = 0;
  StatsCollector sc;
  PP<StatsCollector> ppsc = new StatsCollector;
  StatsCollector*    psc  = ppsc;
  const StatsCollector* cpsc = ppsc;

  cout << endl << "*** INDEXABLE SIZE SHOULD BE ZERO: ***" << endl;
  cout << "indexableObjectSize(int)                   : " << indexableObjectSize(x)    << endl
       << "indexableObjectSize(int*)                  : " << indexableObjectSize(y)    << endl
       << "indexableObjectSize(Vec)                   : " << indexableObjectSize(v)    << endl
       << "indexableObjectSize(Array<int>)            : " << indexableObjectSize(aint) << endl
       << "indexableObjectSize(PPointable*)           : " << indexableObjectSize(p)    << endl
       << "indexableObjectSize(Storage<int>)          : " << indexableObjectSize(stor) << endl
       << "indexableObjectSize(Object*)               : " << indexableObjectSize(u)    << endl
       << "indexableObjectSize(StatsCollector)        : " << indexableObjectSize(sc)   << endl
       << "indexableObjectSize(PP<StatsCollector>)    : " << indexableObjectSize(ppsc) << endl
       << "indexableObjectSize(StatsCollector*)       : " << indexableObjectSize(psc)  << endl
       << "indexableObjectSize(const StatsCollector*) : " << indexableObjectSize(cpsc) << endl;
}

void indexable()
{
  TVec<StatsCollector> vsc(5);
  TVec< PP<StatsCollector> > ppvsc;

  for (int i=0 ; i<5 ; ++i)
    ppvsc.push_back(new StatsCollector);

  cout << endl << "*** INDEXABLE SIZE SHOULD BE 5: ***" << endl;
  cout << "indexableObjectSize(TVec<StatsCollector>)       : " << indexableObjectSize(vsc) << endl
       << "indexableObjectSize(TVec< PP<StatsCollector> >) : " << indexableObjectSize(ppvsc) << endl;

  cout << endl << "*** TRY INDEXING: ***" << endl;
  for (int i=0 ; i<5 ; ++i)
    cout << "toIndexedObjectPtr(TVec<StatsCollector>[" << i << "])       : "
         << toIndexedObjectPtr(vsc, i) << endl;
  for (int i=0 ; i<5 ; ++i)
    cout << "toIndexedObjectPtr(TVec< PP<StatsCollector> >[" << i << "]) : "
         << toIndexedObjectPtr(ppvsc, i) << endl;
}



} // end of namespace PLearn



using namespace PLearn;

int main()
{
  notConvertible();
  convertible();
  notIndexable();
  indexable();
  
  return 0;
}
