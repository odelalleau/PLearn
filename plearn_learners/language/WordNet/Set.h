#ifndef Set_H
#define Set_H

#include "TMat.h"

namespace PLearn {
using namespace std;

class PPointableSet : public set<int>, public PPointable
{
public:
  PPointableSet() {}
  virtual ~PPointableSet() {}
};

inline PStream &operator<<(PStream &out, const PPointableSet &pp_set)
{ out << static_cast<const set<int> &>(pp_set); return out; }

inline PStream &operator>>(PStream &in, PPointableSet &pp_set)
{ in >> static_cast<set<int> &>(pp_set); return in; }

typedef PPointableSet::iterator SetIterator;

class Set : public PP<PPointableSet>
{

public:

  Set() : PP<PPointableSet>(new PPointableSet) {}
  Set(PPointableSet* p) : PP<PPointableSet>(p) {}

  bool contains(int elem) { return ptr->find(elem) != ptr->end(); }
  SetIterator find(int elem) { return ptr->find(elem); }
  void insert(int elem) { ptr->insert(elem); }
  int size() { return ptr->size(); }
  bool isEmpty() { return (ptr->size() == 0); }
  void remove(int elem) { ptr->erase(elem); }
  void clear() { ptr->clear(); }
  
  void replace(int old_elem, int new_elem) 
  {
    remove(old_elem);
    insert(new_elem);
  }
  
  void merge(Set s) 
  {
//     Set res;
//     set_union(begin(), end(),
//               s.begin(), s.end(),
//               insert_iterator<PPointableSet>(*res, res.begin()));
//     *ptr = *res;
    for (SetIterator it = s.begin(); it != s.end(); ++it)
      insert(*it);
  }

  void difference(Set s)
  {
    Set res;
    set_difference(begin(), end(),
                   s.begin(), s.end(),
                   insert_iterator<PPointableSet>(*res, res.begin()));
    *ptr = *res;
  }

  void intersection(Set s)
  {
    Set res;
    set_intersection(begin(), end(),
                     s.begin(), s.end(),
                     insert_iterator<PPointableSet>(*res, res.begin()));
    *ptr = *res;
  }

  SetIterator begin() { return ptr->begin(); }
  SetIterator end() { return ptr->end(); }
  
  bool operator==(Set& s) { return *ptr == *s.ptr; }
  bool operator!=(Set& s) { return *ptr != *s.ptr; }

};

inline PPointableSet * newSet()
{ return new PPointableSet; };

inline void merge(Set a, Set b, Set res)
{
  set_union(a.begin(), a.end(),
            b.begin(), b.end(),
            insert_iterator<PPointableSet>(*res, res.begin()));
}

inline void difference(Set a, Set b, Set res)
{
  set_difference(a.begin(), a.end(),
                 b.begin(), b.end(),
                 insert_iterator<PPointableSet>(*res, res.begin()));
}

inline void intersection(Set a, Set b, Set res)
{
  set_intersection(a.begin(), a.end(),
                   b.begin(), b.end(),
                   insert_iterator<PPointableSet>(*res, res.begin()));
}

inline ostream& operator<<(ostream& out, Set s)
{
  for(SetIterator it = s.begin(); it != s.end(); ++it)
  {
    out << *it << " ";
  }
  return out;
}

//include "pl_io.h"
// inline pl_ostream& operator<<(pl_ostream& out, Set& s)
// {
//   int l = s.size(); 
//   out << l;
//   if (out.flags.test(plf_binary)) {
//     for(SetIterator it = s.begin(); it != s.end(); ++it)
//       PLearn::binwrite(out,*it);
//   }
//   else {
//     out << raw << '\n';
//     for(SetIterator it = s.begin(); it != s.end(); ++it)
//       out << *it << " ";
//     out << raw << '\n';
//   }
//   return out;
// }

// inline pl_istream& operator>>(pl_istream& in, Set& s)
// {
//   int l;
//   s.clear();
//   in >> l;
//   int v=0;
//   if (in.flags.test(plf_binary)) {
//     for (int i=0;i<l;i++)
//     {
//       PLearn::binread(in,v);
//       s.insert(v);
//     }
//   }
//   else {
//     for (int i=0;i<l;i++)
//     {
//       in >> v;
//       s.insert(v);
//     }
//   }
// }

}

#endif
