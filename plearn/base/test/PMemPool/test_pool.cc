#include <plearn/base/PMemPool.h>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;
using namespace PLearn;

struct MyStruct {
  double x[5];
};

const int N = 10000000;
const int POOL_SIZE = 10000;
const float GROWTH = 2.0;

template <bool fast_dealloc>
void alloc_from_pool()
{
  vector<MyStruct*> all_allocated;
  all_allocated.reserve(N);
  PObjectPool<MyStruct> pool(POOL_SIZE, GROWTH, fast_dealloc);

  cout << "Is pool empty? " << pool.empty() << endl;
  
  for (int i=0; i<N; ++i) {
    MyStruct* s = pool.allocate();
    // cout << "Allocating at " << setbase(16) << (void*)s << endl;
    all_allocated.push_back(s);
  }

  cout << "Is pool empty? " << pool.empty() << endl;
  
  for (int i=N/2; i<N; ++i) {
    MyStruct* s = all_allocated[i];
    // cout << "Deallocating at " << setbase(16) << (void*)s << endl;
    pool.deallocate(s);
  }

  cout << "Is pool empty? " << pool.empty() << endl;

  for (int i=0; i<N/2; ++i) {
    MyStruct* s = all_allocated[i];
    // cout << "Deallocating at " << setbase(16) << (void*)s << endl;
    pool.deallocate(s);
  }

  cout << "Is pool empty? " << pool.empty() << endl;
}

void alloc_from_stdalloc()
{
  vector<MyStruct*> all_allocated;
  all_allocated.reserve(N);
  for (int i=0; i<N; ++i) {
    all_allocated.push_back(new MyStruct);
  }
  for (int i=0; i<N; ++i) {
    delete all_allocated[i];
  }
}

int main()
{
  alloc_from_pool<false>();
  alloc_from_pool<true>();
  alloc_from_stdalloc();
};
