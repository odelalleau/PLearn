/* *******************************************************
   * $Id: PLearn/PLearnLibrary/TestSuite/TMat/Matrices/test_TMat_maths.h
   * AUTHOR: Christian Dorion & Kim Levy
   ******************************************************* */

/*! \file PLearnLibrary/PLearnCore/TestSuite/TMat/Matrices/test_TMat_maths.h */

#ifndef _TEST_TMAT_MATHS_H__
#define _TEST_TMAT_MATHS_H__

#include "t_general.h"
#include "TMat_maths.h"
//#include "Array.h"
#include "TMat/TMat_utils.h"

using namespace PLearn;

#define FILL 10 //to be modified for a constructor argument in TestTMatMath!!!

template <class T>
class TestTMatMaths
{
public:
  TestTMatMaths<T>(string dirName, T max_diff = 1000):
    dirName_(dirName) 
  {
    forbidden.append("CVS");
  }

  TVec<string> forbidden;
  
  bool runTest()
  {
    vector<string> dirList = lsdir( dirName() );
    vector<string>::iterator it = dirList.begin();
    vector<string>::iterator end = dirList.end();
    string nameOfMat;
    for(; it != end; ++it)
      {
        if( forbidden.contains(*it) )
          continue;
        
        nameOfMat = dirName() + tostring("/") + *it; 
        DO_TEST( nameOfMat,  applyOn(nameOfMat) );
      }
    return true;
  }
  
  bool applyOn(const string& fileMatName)
  {
    cout << endl;
    cout << "Loading " << fileMatName << "..." << endl; 
    M = readMatFromFile(fileMatName);
    cout << M << endl;

    TMat<T> res; //(M.length(), M.width());
    
    if( !miscellaneous() ){ return false; } 

    // product or a variation of it fails the test !
    if( !productAndVariations(res) ){ return false; }
    if( !checkCompilation(res) ) { return false; }

    // Functions between line 2004 and 2198 not tested yet
    
    if( !productTransposeAndVariations(res) ){ return false; }
    if( !squareProductTransposeAndVariations(res) ){ return false; }
    if( !transposeProductAndVariations(res) ){ return false; }  
    
    return true;
  }

  bool notYetImplementedTests();
  
private:
  bool checkCompilation(TMat<T>& res);
  bool miscellaneous();
  bool productAndVariations(TMat<T>& res);
  bool productTransposeAndVariations(TMat<T>& res);
  bool squareProductTransposeAndVariations(TMat<T>& res);
  bool transposeProductAndVariations(TMat<T>& res);
  string dirName() { return dirName_; }

  string dirName_;
  T max_dif;

  // The matrix objects that will be used to store the current matrix
  // being tested
  TMat<T> M;
  TVec<T> V;
};


template <class T>
bool 
TestTMatMaths<T>::
miscellaneous()
{
  cout << "sum(const TMat<T>& mat) = " << sum(M) << endl;
  
  T sumsq = sumsquare(M);
  cout << "sumsquare(const TMat<T>& mat) = "  << sumsq << endl;
  T_ASSERT(sum_of_squares(M) == sumsq,
           "Results of sumsquare & sum_of_squares diverge!!!");
  
  cout << "product(const TMat<T>& mat) = " << product(M) << endl;
  
  T mean = mean(M);
  cout << "mean(const TMat<T>& mat) = " << mean << endl;
  cout << "variance(const TMat<T>& mat, T meanval) = " << variance(M, mean) << endl;

  try{
    cout << "geometric_mean(const TMat<T>& mat) = " << geometric_mean(M) << endl;
  }
  catch(const PLearnError& ple){
    cout << ple.message() << endl;
  }

  T val = min(M);
  int i = -1;
  int j = -1;
  int ij = -1;

  cout << "min(const TMat<T>& mat) = " << val << endl;
  argmin(M, i, j);
  T_ASSERT(M(i,j) == val,
           "Logical error in argmin or min!");
  ij = argmin(M);
  T_ASSERT(M.data()[ij] == val,
           "Logical error in argmin or min!");
  
  val = max(M);
  cout << "max(const TMat<T>& mat) = " << val << endl;
  argmax(M, i, j);
  T_ASSERT(M(i,j) == val,
           "Logical error in argmax or max!");
  ij = argmax(M);
  T_ASSERT(M.data()[ij] == val,
           "Logical error in argmin or min!");
  
  

  cout << "makeItSymmetric: " << flush;
  try{
    makeItSymmetric(M, max_dif);
    cout << M << endl;
  }
  catch(const PLearnError& ple){
    cout << ple.message() << endl;
  }
  
  cout << "trace(const TMat<T>& mat) = " << endl;
  try{
    cout << trace(M) << endl << endl;
  }
  catch(const PLearnError& ple){
    cout << ple.message() << endl << endl;
  }

  return true;
}

template <class T>
bool 
TestTMatMaths<T>::
notYetImplementedTests()
{
  cout << "---------------------------------"
       << "NOT IMPLEMENTED YET"
       << "---------------------------------"
       << endl;

  cout << "projectOnOrthogonalSubspace(const TVec<T>& vec, const TMat<T>& orthonormal_subspace)" 
       << endl;

  

  cout << "matRowDotVec(const TMat<T>& mat, int i, const TVec<T> v)" 
       << endl;

  cout << "matColumnDotVec(const TMat<T>& mat, int j, const TVec<T> v)" 
       << endl;

  cout << "product2Transpose(const TMat<T>& m1, const TMat<T>& m2)"
       << endl;

  cout << "product2TransposeAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)"
       << endl;

  cout << "transposeProduct2(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)"
       << endl;

  cout << "transposeProduct2Acc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)"
       << endl;

  cout << "ALL FUNCTIONS FROM rowSum(const TMat<T>& mat, TMat<T>& singlecolumn)\n"
       << "\t TO THE END"
       << endl;

  return true;
}


// IT NOW COMPILES: THIS IS TO BE MOVED!!!!
template <class T>
bool
TestTMatMaths<T>::
checkCompilation(TMat<T>& res)
{
  // MUST BE AN ERROR: no res arg & return type is void!!!
#ifdef CHECK_COMPILATION
  cout << "product2Transpose(const TMat<T>& m1, const TMat<T>& m2) = " << endl;
  try{
    product2Transpose(M, M);
    
    // TBDone in the same bloc:
    //  product2TransposeAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2)
    
    //cout << res << endl;
  }
  catch(const PLearnError& ple){
    cout << ple.message() << endl;
  }
#endif

  return true;
}


template <class T>
bool
TestTMatMaths<T>::
productAndVariations(TMat<T>& res)
{
  TMat<T> compatible;
  selectByLength( dirName(), M.width(), compatible ); 

  res.resize(M.length(), compatible.width());

  cout << "product(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2) = " << endl; // l:2004
  try{
    product(res, M, compatible);
    cout << res << endl;
  }
  catch(const PLearnError& ple){
    cout << ple.message() << endl << endl;
    return false;
  }


  /*
    Here, 'compatible' matrix is being copied to 'compatible.width' column vectors.
    Then, 'M' will be multiplied with each column giving a column of the result matrix,
    which will also be viewed as an array of column. That way, it will be possible to
    compare the results of the previous test, 'res', with the new result, 'resByVec'.
    Mathematically, we must obtain 'res == resByVec'.
  */
  Array< TVec<T> > resByVec( compatible.width() );
  for(int vec=0; vec<compatible.width(); vec++)
    resByVec[vec].resize( M.length() );

  Array< TVec<T> > colVecViewOfCompatible = toColVecArray(compatible);
  cout << "product(const TVec<T>& vec, const TMat<T>& m, const TVec<T>& v) = " << endl;
  for(int j=0; j<compatible.width(); j++)
    {
      try{
        product(resByVec[j], 
                M, 
                colVecViewOfCompatible[j]);
      }
      catch(const PLearnError& ple){
        T_ILLEGAL(ple.message() + tostring("\n\thas no sense: product worked well.") );
      }
    }
  T_ASSERT( colEqual(res, resByVec),
           "Logical error in product");
  cout << "Ok." << endl << endl;


  /*
    Here 'resPlus' is simply a matrix with initial values 'FILL', added to
    'res'. Then the call to product made previously on 'res' is made with 
    'productAcc' on 'resPLus'.
    We shoud have 'res == resPlus'
  */
  TMat<T> resPlus(M.length(), compatible.width(), FILL);
  res += resPlus;

  cout << "productAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2) = " << endl;
  try{
    productAcc(resPlus, M, compatible);
  }
  catch(const PLearnError& ple){
    T_ILLEGAL(ple.message() + tostring("\n\thas no sense: product worked well.") );
  }
  T_ASSERT(res == resPlus,
           "Logical error in productAcc");
  cout << "Ok." << endl << endl;


  /*
    The 'resByVec' is re-initialised with the same 'FILL' value, so that
    comparing 'resPlus' with 'resPlus' will ensure the correctness of productAcc 
  */
  for(int vec=0; vec<compatible.width(); vec++)
    resByVec[vec].fill( FILL );
  
  cout << "productAcc(const TVec<T>& vec, const TMat<T>& m, const TVec<T>& v) = " << endl;
  for(int j=0; j<compatible.width(); j++)
    {
      try{
        productAcc(resByVec[j], 
                M, 
                colVecViewOfCompatible[j]);
      }
      catch(const PLearnError& ple){
        T_ILLEGAL(ple.message() + tostring("\n\thas no sense: product worked well.") );
      }
    }
  T_ASSERT( colEqual(resPlus, resByVec),
           "Logical error in productAcc");
  cout << "Ok." << endl << endl;

  return true;
}

template <class T>
bool
TestTMatMaths<T>::
productTransposeAndVariations(TMat<T>& res)
{
  res.resize(M.length(), M.length());
  cout << "productTranspose(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2) = " << endl;
  try{
    productTranspose(res, M, M);
    cout << res << endl;
    
    cout << "productTransposeAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2):" << endl;
      
    // BUG!!! quand T != int !!!!!!!  ==> FILL
    TMat<T> resAcc(M.length(), M.length(), FILL);
    // To test if the logical of resAcc is respected
    res += resAcc;

    try{
      productTransposeAcc(resAcc, M, M);
      T_ASSERT(res == resAcc,
               "Logical error in productTransposeAcc");
      cout << "Ok." << endl << endl;
    }
    catch(const PLearnError& ple){
      T_ILLEGAL(ple.message() + tostring("\n\thas no sense: productTranspose worked well.") );
    }
  }
  catch(const PLearnError& ple){
    cout << ple.message() << endl << endl;
  }
  
  return true;
}

template <class T>
bool
TestTMatMaths<T>::
squareProductTransposeAndVariations(TMat<T>& res)
{
  //2286
  cout << "squareProductTranspose(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2) = " << endl;
  try{
    squareProductTranspose(res, M, M);
    cout << res << endl;

    cout << "squareProductTransposeAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2):" << endl;

    // BUG!!! quand T != int !!!!!!!  ==> FILL
    TMat<T> resAcc(M.length(), M.length(), FILL);
    // To test if the logical of resAcc is respected
    res += resAcc;

    try{
      squareProductTransposeAcc(resAcc, M, M);
      T_ASSERT(res == resAcc,
               "Logical error in squareProductTransposeAcc");
      cout << "Ok." << endl << endl;
    }
    catch(const PLearnError& ple){
      T_ILLEGAL(ple.message() + tostring("\n\thas no sense: squareProductTranspose worked well.") );
    }
  }
  catch(const PLearnError& ple){
    cout << ple.message() << endl;
  }
  
  return true;
}

template <class T>
bool
TestTMatMaths<T>::
transposeProductAndVariations(TMat<T>& res)
{
  res.resize(M.width(), M.width());
  cout << "transposeProduct(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2) = " << endl;
  try{
    transposeProduct(res, M, M);
    cout << res << endl;
  }
  catch(const PLearnError& ple){
    cout << ple.message() << endl << endl;
    return false;
  }

  
  /*
    Here, 'M' matrix is being copied to 'M.width' column vectors.
    Then, 'M' will be multiplied with each column giving a column of the result matrix,
    which will also be viewed as an array of column. That way, it will be possible to
    compare the results of the previous test, 'res', with the new result, 'resByVec'.
    Mathematically, we must obtain 'res == resByVec'.
  */
  Array< TVec<T> > resByVec( M.width() );
  for(int vec=0; vec<M.width(); vec++)
    resByVec[vec].resize( M.width() );

  Array< TVec<T> > colVecViewOfM = toColVecArray(M);
  cout << "transposeProduct(const TVec<T>& vec, const TMat<T>& m, const TVec<T>& v) = " << endl;
  for(int j=0; j<M.width(); j++)
    {
      try{
        transposeProduct(resByVec[j], 
                         M, 
                         colVecViewOfM[j]);
      }
      catch(const PLearnError& ple){
        T_ILLEGAL(ple.message() + tostring("\n\thas no sense: product worked well.") );
      }
    }
  T_ASSERT( colEqual(res, resByVec),
            "Logical error in product");
  cout << "Ok." << endl << endl;
  


  
  /*
    Here 'resPlus' is simply a matrix with initial values 'FILL', added to
    'res'. Then the call to product made previously on 'res' is made with 
    'transposeProductAcc' on 'resPLus'.
    We shoud have 'res == resPlus'
  */
  TMat<T> resPlus(M.width(), M.width(), FILL);
  res += resPlus;

  cout << "transposeProductAcc(const TMat<T>& mat, const TMat<T>& m1, const TMat<T>& m2):" << endl;
  try{
    transposeProductAcc(resPlus, M, M);
  }
  catch(const PLearnError& ple){
    T_ILLEGAL(ple.message() + tostring("\n\thas no sense: transposeProduct worked well.") );
  }
  T_ASSERT(res == resPlus,
           "Logical error in transposeProductAcc");
  cout << "Ok." << endl << endl;


  /*
    The 'resByVec' is re-initialised with the same 'FILL' value, so that
    comparing 'resPlus' with 'resPlus' will ensure the correctness of productAcc 
  */
  for(int vec=0; vec<M.width(); vec++)
    resByVec[vec].fill( FILL );
  
  cout << "transposeProductAcc(const TVec<T>& vec, const TMat<T>& m, const TVec<T>& v)" << endl;
  for(int j=0; j<M.width(); j++)
    {
      try{
        transposeProductAcc(resByVec[j], 
                            M, 
                            colVecViewOfM[j]);
      }
      catch(const PLearnError& ple){
        T_ILLEGAL(ple.message() + tostring("\n\thas no sense: transposeProduct worked well.") );
      }
    }
  T_ASSERT( colEqual(resPlus, resByVec),
           "Logical error in transposeProductAcc");
  cout << "Ok." << endl << endl;
  
  return true;
}

#endif // ifndef _TEST_TMAT_MATHS_H__

