/* *******************************************************
   * $Id: /u/lisa/dorionc/PLearn/PLearnLibrary/TestSuite/TMat/TMat_utils.h
   * AUTHOR: Christian Dorion & Kim Levy
   ******************************************************* */

/*! \file PLearnLibrary/TestSuite/TMat/TMat_utils.h */

#ifndef __TMAT_UTILS_H_
#define __TMAT_UTILS_H_

#include "VMat.h"
#include "TMat.h"
#include "Array.h"

using namespace PLearn;
using namespace std; 

#ifdef USENAMESPACE
namespace PLearn {
#endif

#define BIG_INT 10000
  
//! Transforms a TMat into an array of the matrix column vectors (TVec) 
template <class T>
Array< TVec<T> > toColVecArray(const TMat<T>& mat)
{
  int w = mat.width();
  Array< TVec<T> > vecViewOfMat( w );
  for(int col=0; col<w; col++)
    {
      vecViewOfMat[col] = mat.column(col).toVecCopy();
    }
  return vecViewOfMat;
}

//! return true if element per element comparisons of mat and vecMat all return true 
template <class T>
bool colEqual(const TMat<T>& mat, Array< TVec<T> > vecMat)
{
  // Here it's better to start with the width to avoid illegal access 
  //  in the comparison of length
  if( mat.width() != vecMat.size() || mat.length() != vecMat[0].length())
    return false;
  
  TMat<T>::iterator it = mat.begin();
  TMat<T>::iterator end = mat.end();
  int row = 0;
  int col = 0;
  for(; it!=end; ++it)
    {
      if( *it != vecMat[col][row] )
        return false;
      
      ++col;
      if( col == vecMat.size() ){
        col = 0;
        ++row;
      }
    }

  return true;
}
 
//! Transforms a TMat into an array of the matrix row vectors (TVec) -- not tested yet... 
template <class T>
Array< TVec<T> > toRowVecArray(const TMat<T>& mat)
{
  int l = mat.length();
  Array< TVec<T> > vecViewOfMat( l );
  for(int row=0; row<l; row++)
    {
      vecViewOfMat[row] = mat.row(row).toVecCopy();
    }
  return vecViewOfMat;
}

//! return true if element per element comparisons of mat and vecMat all return true -- not tested yet...
template <class T>
bool rowEqual(const TMat<T>& mat, Array< TVec<T> > vecMat)
{
  // Here it's better to start with the width to avoid illegal access 
  //  in the comparison of length
  if( mat.length() != vecMat.size() || mat.width() != vecMat[0].length())
    return false;
  
  TMat<T>::iterator it = mat.begin();
  TMat<T>::iterator end = mat.end();
  int row = 0;
  int col = 0;
  
  for(; it!=end; ++it)
    {
      if( *it != vecMat[row][col] )
        return false;
      
      ++col;
      if( col == vecMat[row].length() ){
        col = 0;
        ++row;
      }
    }

  return true;
}
  
/* *********************
   Loading from files...
   ********************* */

/*! 
  Reads a matrix from an ascii file; format must be length:width on 
  the first line and one row per line after. Ex:
  
  Matrix          File
  | 1 2 3 |       3 3
  | 2 4 5 |  ==>  1 2 3
  | 2 3 3 |       2 4 5
                  2 3 3
*/
TMat<double> readMatFromFile(string filepath)
{
  TMat<double> M;
  loadAscii(filepath, M);
  return M;
}

/*! 
  To Be Modified!!

  Reads a vec from an ascii file; format must be 1:length on 
  the first line and one row per line after. Ex:
  
  Matrix            File
  | 1 2 3 4 |  ==>  1:4
                    1 2 3 4
*/
TVec<double> readVecFromFile(string filepath)
{
  ifstream file(filepath.c_str());
  
  if( !file.good() )
    PLERROR("Error occured in opening %s", filepath.c_str());

  string line;
  
  line = pgetline(file);
  vector<string> v( split(line, ":") );
  T_ASSERT(v.size() == 2, "Format Error In " + filepath);
  
  unsigned int length = toint( v[0] );
  T_ASSERT(length == 1, "Format Error In " + filepath + "\nVector length must be 1!");
  
  unsigned int width = toint( v[1] );

  
  TVec<double> V(width);
  TVec<double>::iterator elemOfV = V.begin();
  
  line = pgetline(file);
  v = split(line);
  T_ASSERT(v.size() == width, "Format Error In " + filepath);
      
  vector<string>::iterator it = v.begin();
  vector<string>::iterator end = v.end();
  for(; it != end; ++it){
    *elemOfV = todouble(*it);
    elemOfV++;
  }

  T_ASSERT(elemOfV == V.end(), "V isn't filled!!!")
  return V;
}

//! Reads the first matrix encountered with the desired length
void selectByLength(const string& dirName, const int& length, TMat<double>& byLength)
{
  string lengthStr = tostring("_") + tostring(length) + tostring("_");

  vector<string> dirList = lsdir(dirName);
  vector<string>::iterator it = dirList.begin();
  vector<string>::iterator end = dirList.end();
  for(; it != end; ++it)
    if(it->find(lengthStr) != string::npos){
      byLength = readMatFromFile(dirName + tostring("/") + *it);
      return;
    }
  T_ILLEGAL("No compatiblex matrix!");
  //return TMat<double>(-1, -1);
}

//! Loading matArray with matrices of the same size
template <class T>
void loadSameSize(const string& dirName, const Array< TMat<T> >& matArray)
{
  vector<string> dirList = lsdir(dirName);

  T_ASSERT(matArray.length() <= int(dirList.size()), 
           "Can't load " + tostring(matArray.length()) + " matrices: there are only " +
           tostring(dirList.size()) + " in " + dirName );
  
  vector<string>::iterator it = dirList.begin();
  int minLength(BIG_INT); 
  int minWidth(BIG_INT);
  for(int m=0; m < matArray.length(); m++)
    {
      if(*it == "CVS")
        ++it;

      matArray[m] = readMatFromFile( dirName + tostring("/") + *it );
      if(matArray[m].length() < minLength) minLength = matArray[m].length();
      if(matArray[m].width() < minWidth) minWidth = matArray[m].width();

      ++it;
    }
  for(int m=0; m < matArray.length(); m++)
    matArray[m].resize(minLength, minWidth);

}

#endif // ifndef __TMAT_UTILS_H_
