#ifndef PROBABILITY_SPARSE_MATRIX_H
#define PROBABILITY_SPARSE_MATRIX_H

#include "RowMapSparseMatrix.h"
#include "Object.h"
#include "Set.h"

#define NUMWIDTH 10

namespace PLearn {

//typedef map<int, real>::const_iterator RowIterator;
typedef map<int, real> SparseVec;
typedef const map<int, real> ConstSparseVec;

// Represent a sparse probability matrix P(Y=y|X=x).
// On the non-filled entries, there are two possible values: zero and
// "undefined". The latter would be correct if no instance
// of X=x has ever been seen. This is represented in output as -1
// (or a warning is raised) but does not take space internally 
// (it occurs when the whole column x is empty). 
// Unlike RowMapSparseMatrix, this class clearly
// distinguishes between a read access and a write access
// to avoid inefficient creation of elements when reading.
class ProbabilitySparseMatrix : public Object
{
private:
  RowMapSparseMatrix<real> x2y;
  RowMapSparseMatrix<real> y2x;
public:

  Set Y;
  Set X;

  bool raise_error;
  string name;

  ProbabilitySparseMatrix(int ny=0, int nx=0, string pname = "pYX") : raise_error(true)
    {
      resize(ny,nx);
      name = pname;
    }
  void resize(int ny, int nx) { 
    x2y.resize(nx,ny); 
    y2x.resize(ny,nx);
  }
  int nx() { return x2y.length(); }
  int ny() { return y2x.length(); }

  void rename(string new_name) { name = new_name; }

  Set computeX()
  {
    X.clear();
    for (int xx = 0; xx < nx(); xx++)
    {
      if (sumPYx(xx) > 0)
      {
        X.insert(xx);
      }
    }
    return X;
  }

  Set computeY()
  {
    Y.clear();
    for (int yy = 0; yy < ny(); yy++)
    {
      if (sumPyX(yy) > 0)
      {
        Y.insert(yy);
      }
    }
    return Y;
  }

  real get(int y,int x) { 
    map<int,real>& mx = x2y(x);
    if (mx.size()==0)
    {
      if (raise_error)
        //PLERROR("trying to access an invalid probability at P(%d|%d) in %s",y,x, name);
        PLWARNING("trying to access an invalid probability at P(%d|%d) in %s",y,x, name.c_str());
      return 0; 
    }
    if (mx.find(y)!=mx.end()) 
      return mx[y]; 
    return 0;
  }

  bool exists(int y, int x) { 
    map<int,real>& mx = x2y(x);
    if (mx.size()==0) return false;
    return mx.find(y)!=mx.end();
  }

  // most access are for reading, allow operator() for convenience
  real operator()(int y, int x) { return get(y,x); }
  void incr(int y, int x, real increment=1) 
    { 
      if (increment!=0)
      {
        real current_value = 0;
        map<int,real>& mx = x2y(x);
        if (mx.size()!=0)
        {
          map<int,real>::const_iterator it = mx.find(y);
          if (it!=mx.end())
            current_value = it->second;
        }
        set(y,x,current_value+increment); 
      }
    }
  const map<int,real>& getPYx(int x, bool dont_raise_error=false)  // const is to force user to call set for writing
    { 
      const map<int,real>& PYx = x2y(x);
      if (raise_error && !dont_raise_error && PYx.size()==0)
        PLERROR("ProbabilitySparseMatrix::getPyx: accessing an empty column at X=%d",x);
      return PYx;
    }
  const map<int,real>& getPyX(int y)   // const is to force user to call set for writing
    { 
      return y2x(y); 
    }

  map<int,real> getPYxCopy(int x, bool dont_raise_error=false) 
    { 
      map<int,real> PYx = x2y(x);
      if (raise_error && !dont_raise_error && PYx.size()==0)
        PLERROR("ProbabilitySparseMatrix::getPyx: accessing an empty column at X=%d",x);
      return PYx;
    }

  map<int,real> getPyXCopy(int y)
    { 
      map<int, real> pyX = y2x(y);
      return pyX; 
    }

  void setPYx(int x, const map<int, real>& pYx)
    {
      for (map<int, real>::const_iterator it = pYx.begin(); it != pYx.end(); ++it)
        set(it->first, x, it->second);
    }

  void setPyX(int y, const map<int, real>& pyX)
    {
      for (map<int, real>::const_iterator it = pyX.begin(); it != pyX.end(); ++it)
        set(y, it->first, it->second);
    }

  void set(int y,int x,real v, bool dont_warn_for_zero = false) { 
    if (v!=0)
    {
      x2y(x,y)=v;
      y2x(y,x)=v;
    } else 
    {
      if (!dont_warn_for_zero)
        PLWARNING("setting something to 0 in ProbabilitySparseMatrix");
      map<int,real>& PYx = x2y(x);
      if (PYx.find(y)!=PYx.end())
      {
        PYx.erase(y);
        map<int,real>& PyX = y2x(y);
        PyX.erase(x);
      }
    }
  }

  void removeElem(int y, int x)
  {
    set(y, x, 0.0, true);
  }

  real sumPYx(int x, Set Y)
  {
    real sum_pYx = 0.0;
    map<int, real>& col = x2y(x);
    for (map<int, real>::const_iterator yit = col.begin(); yit != col.end(); ++yit)
    {
      int y = yit->first;
      if (Y.contains(y))
        sum_pYx += yit->second;
    }
    return sum_pYx;
  }
  
  real sumPyX(int y, Set X)
  {
    real sum_pyX = 0.0;
    map<int, real>& row = y2x(y);
    for (map<int, real>::const_iterator xit = row.begin(); xit != row.end(); ++xit)
    {
      int x = xit->first;
      if (X.contains(x))
        sum_pyX += xit->second;
    }
    return sum_pyX;
  }

  real sumPYx(int x)
  {
    real sum_pYx = 0.0;
    map<int, real>& col = x2y(x);
    for (map<int, real>::const_iterator yit = col.begin(); yit != col.end(); ++yit)
    {
      sum_pYx += yit->second;
    }
    return sum_pYx;
  }
  
  real sumPyX(int y)
  {
    real sum_pyX = 0.0;
    map<int, real>& row = y2x(y);
    for (map<int, real>::const_iterator xit = row.begin(); xit != row.end(); ++xit)
    {
      sum_pyX += xit->second;
    }
    return sum_pyX;
  }

  // DEPRECATED
  // put existing elements to 0 without removing them
  void clearElements() {
    for (int x=0;x<nx();x++)
    {
      map<int,real>& r = x2y(x);
      for (map<int,real>::iterator it = r.begin(); it!=r.end(); ++it)
      {
        it->second=0;
        y2x(it->first,x)=0;
      }
    }
  }

  // release all elements in the maps
  void clear() { x2y.clear(); y2x.clear(); }

  void removeRow(int y, Set X) {
    y2x(y).clear();
    for (SetIterator it=X.begin();it!=X.end();++it)
    {
      int x = *it;
      map<int,real>& pYx = x2y(x);
      if (pYx.size()>0)
        pYx.erase(y);
    }
  }

  void removeRow(int y) 
  {
    map<int, real>& row = y2x(y);
    for (map<int, real>::iterator it = row.begin(); it != row.end(); ++it)
    {
      int x = it->first;
      map<int,real>& Yx = x2y(x);
      if (Yx.size()>0)
        Yx.erase(y);
    }
    row.clear();
  }

  void removeColumn(int x)
  {
    map<int, real>& col = x2y(x);
    for (map<int, real>::iterator it = col.begin(); it != col.end(); ++it)
    {
      int y = it->first;
      map<int,real>& yX = y2x(y);
      if (yX.size()>0)
        yX.erase(x);
    }
    col.clear();
  }

  int size()
  {
    if (x2y.size() != y2x.size())
      PLWARNING("x2y and y2x sizes dont match");
    return y2x.size();
  }

  void removeExtra(ProbabilitySparseMatrix& m)
  {
    //for (SetIterator yit = Y.begin(); yit != Y.end(); ++yit)
    int _ny = ny();
    Set x_to_remove;
    for (int y = 0; y < _ny; y++)
    {
      //int y = *yit;
      ConstSparseVec& yX = getPyX(y);
      x_to_remove.clear();
      for (SparseVec::const_iterator xit = yX.begin(); xit != yX.end(); ++xit)
      {
        int x = xit->first;
        if (!m.exists(y, x))
          x_to_remove.insert(x);
      }
      for (SetIterator xit = x_to_remove.begin(); xit != x_to_remove.end(); ++xit)
      {
        int x = *xit;
        removeElem(y, x);
      }
    }
  }

  void fullPrint()
  {
    cout << "y2x" << endl;
    for (int y = 0; y < ny(); y++)
    {
      for (int x = 0; x < nx(); x++)
      {
        cout << y2x(y, x) << " ";
      }
      cout << endl;
    }
    cout << "x2y" << endl;
    for (int x = 0; x < nx(); x++)
    {
      for (int y = 0; y < ny(); y++)
      {
        cout << x2y(x, y) << " ";
      }
      cout << endl;
    }
  }

  void save(string filename)
  {
    y2x.save(filename + ".y2x");
    x2y.save(filename + ".x2y");
  }

  void load(string filename)
  {
    y2x.load(filename + ".y2x");
    x2y.load(filename + ".x2y");
  }

  real* getAsFullVector()
  {
    // a vector of triples : (row, col, value)
    int vector_size = y2x.size() * 3;
    real* full_vector = new real[vector_size];
    int pos = 0;
    for (int i = 0; i < ny(); i++)
    {
      map<int, real>& row_i = y2x(i);
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        real value = it->second;
        full_vector[pos++] = (real)i;
        full_vector[pos++] = (real)j;
        full_vector[pos++] = value;
      }
    }
    if (pos != vector_size)
      PLERROR("weird");
    return full_vector;
  }

  void add(real* full_vector, int n_elems)
  {
    for (int i = 0; i < n_elems; i += 3)
      incr((int)full_vector[i], (int)full_vector[i + 1], full_vector[i + 2]);
  }

  void set(real* full_vector, int n_elems)
  {
    clear();
    for (int i = 0; i < n_elems; i += 3)
      set((int)full_vector[i], (int)full_vector[i + 1], full_vector[i + 2]);
  }

  real sumOfElements()
  {
    real sum = 0.0;
    for (int i = 0; i < ny(); i++)
    {
      map<int, real>& row_i = y2x(i);
      for (map<int, real>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        sum += get(i, j);
      }
    }
    return sum;
  }

};

void samePos(ProbabilitySparseMatrix& m1, ProbabilitySparseMatrix& m2, string m1name, string m2name)
{
  for (SetIterator yit = m1.Y.begin(); yit != m1.Y.end(); ++yit)
  {
    int y = *yit;
    const map<int, real>& yX = m1.getPyX(y);
    for (map<int, real>::const_iterator it = yX.begin(); it != yX.end(); ++it)
    {
      int x = it->first;
      if (!m2.exists(y, x))
        PLERROR("in samePos, %s contains an element that is not present in %s", m1name.c_str(), m2name.c_str());
    }
  }
}

void check_prob(ProbabilitySparseMatrix& pYX, string Yname, string Xname)
{
  bool failed = false;
  real sum_pY = 0.0;
  Set X = pYX.X;
  for (SetIterator x_it = X.begin(); x_it!=X.end(); ++x_it)
  {
    int x = *x_it;
    const map<int,real>& pYx = pYX.getPYx(x);
    sum_pY = 0.0;
    for (map<int,real>::const_iterator y_it=pYx.begin();y_it!=pYx.end();++y_it)
    {
      sum_pY += y_it->second;
    }
    if (fabs(sum_pY - 1.0) > 1e-4)
    {
      failed = true;
      break;
    }
  }
  if (failed)
    PLERROR("check_prob failed for %s -> %s (sum of a column = %g)", Xname.c_str(), Yname.c_str(), sum_pY);
}

void check_prob(Set Y, const map<int, real>& pYx)
{
  real sum_y=0;
  for (map<int,real>::const_iterator y_it=pYx.begin();y_it!=pYx.end();++y_it)
    if (Y.contains(y_it->first))
      sum_y += y_it->second;
  if (fabs(sum_y-1)>1e-4 && pYx.size() != 0)
    PLERROR("check_prob failed, sum_y=%g",sum_y);
}

inline ostream& operator<<(ostream& out, ProbabilitySparseMatrix& pyx)
{
  bool re = pyx.raise_error;
  pyx.raise_error = false;
  for (int y = 0; y < pyx.ny(); y++)
  {
    for (int x = 0; x < pyx.nx(); x++)
    {
      out << setw(NUMWIDTH) << pyx(y, x);
    }
    out << endl;
  }
  pyx.raise_error = re;
  return out;
}

inline void print(ostream& out, ProbabilitySparseMatrix& pyx, Set Y, Set X)
{
  for (SetIterator yit = Y.begin(); yit != Y.end(); ++yit)
  {
    int y = *yit;
    for (SetIterator xit = X.begin(); xit != X.end(); ++xit)
    {
      int x = *xit;
      out << setw(NUMWIDTH) << pyx(y, x);
    }
    out << endl;
  }
}

inline void print(ostream& out, RowMapSparseMatrix<real>& m)
{
  for (int i = 0; i < m.length(); i++)
  {
    for (int j = 0; j < m.width(); j++)
    {
      out << setw(NUMWIDTH) << m(i, j);
    }
    out << endl;
  }
}

inline void print(ostream& out, const map<int, real>& vec, int size)
{
  for (int i = 0; i < size; i++)
  {
    map<int, real>::const_iterator vec_it = vec.find(i);
    if (vec_it != vec.end())
      out << setw(NUMWIDTH) << vec_it->second;
    else
      out << setw(NUMWIDTH) << 0;
  }
  out << endl;
}

inline void print(ostream& out, const map<int, real>& vec)
{
  for (map<int, real>::const_iterator it = vec.begin(); it != vec.end(); ++it)
  {
    out << setw(NUMWIDTH) << it->second;
  }
  out << endl;
}

inline void print(ostream& out, const map<int, real>& vec, Set V)
{
  for (SetIterator vit = V.begin(); vit != V.end(); ++vit)
  {
    int v = *vit;
    map<int, real>::const_iterator vec_it = vec.find(v);
    if (vec_it != vec.end())
      out << setw(NUMWIDTH) << vec_it->second;
    else
      out << setw(NUMWIDTH) << 0;
  }
  out << endl;
}

}

#endif
