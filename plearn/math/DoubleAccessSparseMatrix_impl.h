// -*- C++ -*-

// DoubleAccessSparseMatrix.cc
// 
// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Christian Jauvin
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

/* *******************************************************      
 * $Id: DoubleAccessSparseMatrix_impl.h,v 1.6 2004/09/13 19:40:04 kermorvc Exp $ 
 ******************************************************* */

/*! \file DoubleAccessSparseMatrix_impl.h */

namespace PLearn {
using namespace std;

template<class T>
DoubleAccessSparseMatrix<T>::DoubleAccessSparseMatrix(int n_rows, int n_cols, string _name, int _mode, bool _double_access, T _null_elem) : name(_name), mode(_mode), double_access(_double_access), height(n_rows), width(n_cols), null_elem(_null_elem) 
{
  if (mode != ROW_WISE && mode != COLUMN_WISE) PLERROR("mode must be either row-wise or column-wise");

  if (double_access)
  {
    rows.resize(height);
    cols.resize(width);
  } else if (mode == ROW_WISE)
  {
    rows.resize(height);
  } else
  {
    cols.resize(width);
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::resize(int n_rows, int n_cols) 
{
  height = n_rows;
  width = n_cols;
  if (double_access)
  {
    rows.resize(n_rows);
    cols.resize(n_cols);
  } else if (mode == ROW_WISE)
  {
    rows.resize(n_rows);
  } else
  {
    cols.resize(n_cols);
  }
  clear();
}

template <class T>
void DoubleAccessSparseMatrix<T>::clear() 
{
  if (mode == ROW_WISE || double_access)
  {
	// norman: added explicit cast
    int rs = (int)rows.size();
    for (int i = 0; i < rs; i++)
      rows[i].clear();
  }
  if (mode == COLUMN_WISE || double_access)
  {
	// norman: added explicit cast
    int cs = (int)cols.size();
    for (int i = 0; i < cs; i++)
      cols[i].clear();
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::clearRow(int i, bool force_synchro_if_double_accessible)
{
  if (!double_access)
  {
    if (mode == COLUMN_WISE)
      PLERROR("cannot access rows in the column-wise matrix");
    else
      rows[i].clear();
  } else
  {
    if (force_synchro_if_double_accessible)
    {
      for (int j = 0; j < width; j++)
      {
        map<int, T>& col_j = cols[j];
        if (col_j.find(i) != col_j.end())
          col_j.erase(i);
      }
    } else
    {
      PLWARNING("can only clear rows in the row-wise matrix (internal matrices are now out of sync)");
      rows[i].clear();
    }
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::clearCol(int j, bool force_synchro_if_double_accessible)
{
  if (!double_access)
  {
    if (mode == ROW_WISE)
      PLERROR("cannot access columns in the row-wise matrix");
    else if (mode == COLUMN_WISE)
      cols[j].clear();
  } else
  {
    if (force_synchro_if_double_accessible)
    {
      for (int i = 0; i < height; j++)
      {
        map<int, T>& row_i = rows[i];
        if (row_i.find(j) != row_i.end())
          row_i.erase(j);
      }
    } else
    {
      PLWARNING("can only clear columns in the column-wise matrix (internal matrices are now out of sync)");
      cols[j].clear();
    }
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::clearElem(int i, int j)
{
  if (mode == ROW_WISE || double_access)
  {
    map<int, T>& row_i = rows[i];
    if (row_i.find(j) != row_i.end())
      row_i.erase(j);
  }
  if (mode == COLUMN_WISE || double_access)
  {
    map<int, T>& col_j = cols[j];
    if (col_j.find(i) != col_j.end())
      col_j.erase(i);
  }
}

template <class T>
T DoubleAccessSparseMatrix<T>::get(int i, int j) const
{
#ifdef BOUNDCHECK      
  if (i < 0 || i >= height || j < 0 || j >= width)
    PLERROR("out-of-bound access to (%d, %d), dims = (%d, %d)", i, j, height, width);
#endif
  if (mode == ROW_WISE)
  {
    const map<int, T>& row_i = rows[i];
    typename map<int, T>::const_iterator it = row_i.find(j);
    if (it == row_i.end())
      return null_elem;
    return it->second;
  } else
  {
    const map<int, T>& col_j = cols[j];
    typename map<int, T>::const_iterator it = col_j.find(i);
    if (it == col_j.end())
      return null_elem;
    return it->second;
  }
}

template <class T>
bool DoubleAccessSparseMatrix<T>::exists(int i, int j)
{
#ifdef BOUNDCHECK      
  if (i < 0 || i >= height || j < 0 || j >= width)
    PLERROR("out-of-bound access to (%d, %d), dims = (%d, %d)", i, j, height, width);
#endif
  if (mode == ROW_WISE)
  {
    map<int, T>& row_i = rows[i];
    return (row_i.find(j) != row_i.end());
  } else
  {
    map<int, T>& col_j = cols[j];
    return (col_j.find(i) != col_j.end());
  }    
}

template <class T>
void DoubleAccessSparseMatrix<T>::set(int i, int j, T value)
{
#ifdef BOUNDCHECK      
  if (i < 0 || i >= height || j < 0 || j >= width)
    PLERROR("out-of-bound access to (%d, %d), dims = (%d, %d)", i, j, height, width);
#endif
  if (value != null_elem)
  {
    if (mode == ROW_WISE || double_access)
      rows[i][j] = value;
    if (mode == COLUMN_WISE || double_access)
      cols[j][i] = value;
  } else
  {
    clearElem(i, j);
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::incr(int i, int j, T inc)
{
  if (inc != null_elem)
    set(i, j, get(i, j) + inc);
}

/*
template <class T>
void DoubleAccessSparseMatrix<T>::operator=(SMat<T> m)
{
  name = m->getName();
  mode = m->getMode();
  null_elem = m->getNullElem();
  double_access = m->isDoubleAccessible();
  clear();
  resize(m->getHeight(), m->getWidth());
  if (mode == ROW_WISE)
  {
    for (int i = 0; i < height; i++)
    {
      map<int, T>& row_i = m->getRow(i);
      for (typename map<int, T>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        T val = it->second;
        set(i, j, val);
      }
    }
  } else
  {
    for (int j = 0; j < width; j++)
    {
      map<int, T>& col_j = m->getCol(j);
      for (typename map<int, T>::iterator it = col_j.begin(); it != col_j.end(); ++it)
      {
        int i = it->first;
        T val = it->second;
        set(i, j, val);
      }
    }
  }
}
*/

template <class T>
map<int, T>& DoubleAccessSparseMatrix<T>::getRow(int i)
{
  if (mode == ROW_WISE || double_access)
  {
#ifdef BOUNDCHECK
    if (i < 0 || i > height)
      PLERROR("out-of-bound access to row %d, dims = (%d, %d)", i, height, width);
#endif
    return rows[i];
  } else
  {
    PLERROR("cannot access rows in the column-wise matrix");
    return rows[0];
  }
}

template <class T>
const map<int, T>& DoubleAccessSparseMatrix<T>::getCol(int j)const
{
  if (mode == COLUMN_WISE || double_access)
  {
#ifdef BOUNDCHECK
    if (j < 0 || j > width)
      PLERROR("out-of-bound access to column %d, dims = (%d, %d)", j, height, width);
#endif
    return cols[j];
  } else
  {
    PLERROR("cannot access columns in the row-wise matrix");
    return cols[0];
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::addRow(map<int, T>& row)
{
  if (mode == COLUMN_WISE || double_access)
  {
    PLERROR("cannot add row in the column-wise matrix");
  } else
  {
    rows.push_back(row);
    height++;
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::addCol(map<int, T>& col)
{
  if (mode == ROW_WISE || double_access)
  {
    PLERROR("cannot add col in the row-wise matrix");
  } else
  {
    cols.push_back(col);
    width++;
  }
}

template <class T>
int DoubleAccessSparseMatrix<T>::size()
{
  int s = 0;
  if (mode == ROW_WISE)
  {
    for (unsigned int i = 0; i < rows.size(); i++)
	  // norman: added explicit cast
      s += (int)rows[i].size();
  } else
  {
    for (unsigned int j = 0; j < cols.size(); j++)
	  // norman: added explicit cast
      s += (int)cols[j].size();
  }
  return s;
}

template <class T>
T DoubleAccessSparseMatrix<T>::sumRow(int i) 
{
  if (mode == ROW_WISE || double_access)
  {
    T sum = 0;
    map<int, T>& row_i = rows[i];
    for (typename map<int, T>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      sum += it->second;
    return sum;
  } else
  {
    PLERROR("cannot access rows in the column-wise matrix");
    return 0;
  }
}

template <class T>
T DoubleAccessSparseMatrix<T>::sumCol(int j) 
{
  if (mode == COLUMN_WISE || double_access)
  {
    T sum = 0;
    map<int, T>& col_j = cols[j];
    for (typename map<int, T>::iterator it = col_j.begin(); it != col_j.end(); ++it)
      sum += it->second;
    return sum;
  } else
  {
    PLERROR("cannot access columns in the row-wise matrix");
    return 0;
  }
}

template <class T>
T* DoubleAccessSparseMatrix<T>::getAsCompressedVec()
{
  int vector_size = size() * 3;
  T* compressed_vec = NULL;
  int pos = 0;
  if (mode == ROW_WISE || double_access)
  {
    compressed_vec = new T[vector_size];
    for (int i = 0; i < height; i++)
    {
      map<int, T>& row_i = rows[i];
      for (typename map<int, T>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        T value = it->second;
        compressed_vec[pos++] = (T)i;
        compressed_vec[pos++] = (T)j;
        compressed_vec[pos++] = value;
      }
    }
    if (pos != vector_size)
      PLERROR("weird");
  } else if (mode == COLUMN_WISE)
  {
    compressed_vec = new T[vector_size];
    for (int j = 0; j < width; j++)
    {
      map<int, T>& col_j = cols[j];
      for (typename map<int, T>::iterator it = col_j.begin(); it != col_j.end(); ++it)
      {
        int i = it->first;
        T value = it->second;
        compressed_vec[pos++] = (T)i;
        compressed_vec[pos++] = (T)j;
        compressed_vec[pos++] = value;
      }
    }
    if (pos != vector_size)
      PLERROR("weird");
  }
  return compressed_vec;
}

template <class T>
void DoubleAccessSparseMatrix<T>::getAsMaxSizedCompressedVecs(int max_size, vector<pair<T*, int> >& vectors)
{
  if ((max_size % 3) != 0) PLWARNING("dangerous vector size (max_size mod 3 must equal 0)");

  int n_elems = size() * 3;
  int n_vecs = n_elems / max_size;
  int remaining = n_elems % max_size;
  int pos = 0;
  if (remaining > 0) // padding to get sure that last block size (= remaining) is moddable by 3
  {
    n_vecs += 1;
    int mod3 = remaining % 3;
    if (mod3 != 0)
      remaining += (3 - mod3);
  }
  vectors.resize(n_vecs);
  for (int i = 0; i < n_vecs; i++)
  {
    if (i == (n_vecs - 1) && remaining > 0)
    {
      vectors[i].first = new T[remaining];
      vectors[i].second = remaining;
    } else
    {
      vectors[i].first = new T[max_size];
      vectors[i].second = max_size;
    }
  }
  if (mode == ROW_WISE)
  {
    for (int i = 0; i < height; i++)
    {
      map<int, T>& row_i = rows[i];
      for (typename map<int, T>::iterator it = row_i.begin(); it != row_i.end(); ++it)
      {
        int j = it->first;
        T value = it->second;
        vectors[pos / max_size].first[pos++ % max_size] = (T)i;
        vectors[pos / max_size].first[pos++ % max_size] = (T)j;
        vectors[pos / max_size].first[pos++ % max_size] = value;
      }
    }
  } else if (mode == COLUMN_WISE)
  {
    for (int j = 0; j < width; j++)
    {
      map<int, T>& col_j = cols[j];
      for (typename map<int, T>::iterator it = col_j.begin(); it != col_j.end(); ++it)
      {
        int i = it->first;
        T value = it->second;
        vectors[pos / max_size].first[pos++ % max_size] = (T)i;
        vectors[pos / max_size].first[pos++ % max_size] = (T)j;
        vectors[pos / max_size].first[pos++ % max_size] = value;
      }
    }
  }
  while (pos < n_elems) // pad with (null_elem, null_elem, null_elem)
  {
    vectors[pos / max_size].first[pos++ % max_size] = null_elem;
    vectors[pos / max_size].first[pos++ % max_size] = null_elem;
    vectors[pos / max_size].first[pos++ % max_size] = null_elem;
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::addCompressedVec(T* compressed_vec, int n_elems)
{
  if ((n_elems % 3) != 0) PLERROR("n_elems mod 3 must = 0");
  for (int i = 0; i < n_elems; i += 3)
    incr((int)compressed_vec[i], (int)compressed_vec[i + 1], compressed_vec[i + 2]);
}

template <class T>
void DoubleAccessSparseMatrix<T>::setCompressedVec(T* compressed_vec, int n_elems)
{
  if ((n_elems % 3) != 0) PLERROR("n_elems mod 3 must = 0");
  clear();
  for (int i = 0; i < n_elems; i += 3)
    set((int)compressed_vec[i], (int)compressed_vec[i + 1], compressed_vec[i + 2]);
}

template <class T> 
T DoubleAccessSparseMatrix<T>::sumOfElements()
{
  T sum = 0;
  if (mode == ROW_WISE)
  {
    for (int i = 0; i < height; i++)
    {
      map<int, T>& row_i = rows[i];
      for (typename map<int, T>::iterator it = row_i.begin(); it != row_i.end(); ++it)
        sum += it->second;
    }
  } else if (mode == COLUMN_WISE)
  {
    for (int j = 0; j < width; j++)
    {
      map<int, T>& col_j = cols[j];
      for (typename map<int, T>::iterator it = col_j.begin(); it != col_j.end(); ++it)
        sum += it->second;
    }
  }
  return sum;
}

template <class T>
void DoubleAccessSparseMatrix<T>::setDoubleAccessible(bool da)
{
  if (double_access != da)
  {
    double_access = da;
    if (!double_access)
    {
      if (mode == ROW_WISE)
        cols.clear();
      else if (mode == COLUMN_WISE)
        rows.clear();
    } else
    {
      if (mode == ROW_WISE)
      {
        cols.resize(width);
        for (int i = 0; i < height; i++)
        {
          map<int, T>& row_i = rows[i];
          for (typename map<int, T>::iterator it = row_i.begin(); it != row_i.end(); ++it)
          {
            int j = it->first;
            T value = it->second;
            set(i, j, value);
          }
        }
      } else if (mode == COLUMN_WISE)
      {
        rows.resize(height);
        for (int j = 0; j < width; j++)
        {
          map<int, T>& col_j = cols[j];
          for (typename map<int, T>::iterator it = col_j.begin(); it != col_j.end(); ++it)
          {
            int i = it->first;
            T value = it->second;
            set(i, j, value);
          }
        }
      } 
    }
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::setMode(int new_mode)
{
  if (mode != ROW_WISE && mode != COLUMN_WISE) PLERROR("mode must be either row-wise or column-wise");

  if (mode != new_mode)
  {
    mode = new_mode;
    if (mode == ROW_WISE && !double_access)
    {
      rows.resize(height);
      for (int j = 0; j < width; j++)
      {
        map<int, T>& col_j = cols[j];
        for (typename map<int, T>::iterator it = col_j.begin(); it != col_j.end(); ++it)
        {
          int i = it->first;
          T value = it->second;
          set(i, j, value);
        }
      }
      cols.clear();
    } else if (mode == COLUMN_WISE && !double_access)
    {
      cols.resize(width);
      for (int i = 0; i < height; i++)
      {
        map<int, T>& row_i = rows[i];
        for (typename map<int, T>::iterator it = row_i.begin(); it != row_i.end(); ++it)
        {
          int j = it->first;
          T value = it->second;
          set(i, j, value);
        }
      }
      rows.clear();
    }
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::write(PStream& out) const
{
  string class_name = getClassName();
  switch(out.outmode)
  {
  case PStream::raw_ascii :
  case PStream::pretty_ascii :
    PLERROR("raw/pretty_ascii write not implemented in %s", class_name.c_str());
    break;        
  case PStream::raw_binary :
    PLERROR("raw_binary write not implemented in %s", class_name.c_str());
    break;        
  case PStream::plearn_binary :
  case PStream::plearn_ascii :
    out.write(class_name + "(");
    out << rows;
    out << cols;
    out << name;
    out << mode;
    out << double_access;
    out << height;
    out << width;
    out << null_elem;
    out.write(")\n");
    break;
  default:
    PLERROR("unknown outmode in %s::write(PStream& out)", class_name.c_str());
    break;
  }
}

template <class T>
void DoubleAccessSparseMatrix<T>::read(PStream& in)
{
  string class_name = getClassName();
  switch (in.inmode)
  {
  case PStream::raw_ascii :
    PLERROR("raw_ascii read not implemented in %s", class_name.c_str());
    break;
  case PStream::raw_binary :
    PLERROR("raw_binary read not implemented in %s", class_name.c_str());
    break;
  case PStream::plearn_ascii :
  case PStream::plearn_binary :
  {
    in.skipBlanksAndCommentsAndSeparators();
    string word(class_name.size() + 1, ' ');
    for (unsigned int i = 0; i < class_name.size() + 1; i++)
      in.get(word[i]);
    if (word != class_name + "(")
      PLERROR("in %s::(PStream& in), '%s' is not a proper header", class_name.c_str(), word.c_str());
    in.skipBlanksAndCommentsAndSeparators();
    in >> rows;
    in.skipBlanksAndCommentsAndSeparators();
    in >> cols;
    in.skipBlanksAndCommentsAndSeparators();
    in >> name;
    in.skipBlanksAndCommentsAndSeparators();
    in >> mode;
    in.skipBlanksAndCommentsAndSeparators();
    in >> double_access;
    in.skipBlanksAndCommentsAndSeparators();
    in >> height;
    in.skipBlanksAndCommentsAndSeparators();
    in >> width;
    in.skipBlanksAndCommentsAndSeparators();
    in >> null_elem;
    in.skipBlanksAndCommentsAndSeparators();
    int c = in.get();
    if(c != ')')
      PLERROR("in %s::(PStream& in), expected a closing parenthesis, found '%c'", class_name.c_str(), c);
  }
  break;
  default:
    PLERROR("unknown inmode in %s::write(PStream& out)", class_name.c_str());
    break;
  }
}

} // end of namespace PLearn
