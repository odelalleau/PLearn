/* *******************************************************
   * $Id: /u/lisa/dorionc/PLearn/PLearnLibrary/TestSuite/t_utils.h
   * AUTHOR: Christian Dorion & Kim Levy
   ******************************************************* */

/*! \file PLearnLibrary/PLearnCore/TMat.h */

#ifndef _T_UTILS_H__
#define _T_UTILS_H__

#include <plearn/base/Array.h>

#ifdef USENAMESPACE
namespace PLearn {
#endif

template <class ReturnType>
class Lambda
{
protected:
  virtual void toApply() = 0;
  
#if ReturnType != void
  ReturnType returnValue;
#endif

public:
  //Lambda<ReturnType>()  
  //{}

  virtual ReturnType apply()
  {
    toApply();
    
#if ReturnType != void 
    return returnValue;
#endif   
  }
  
  virtual ~Lambda(){}
};
  
  
#ifdef USENAMESPACE
} //  end of namespace PLearn
using namespace PLearn;
#endif

#endif // ifndef _T_UTILS_H__

// .cc
/*
#include "t_utils.h"
void salutation(const string& msg)
{
  cout << msg << endl;
}

int bnj(int k, double j)
{
  cout << "k: " << k << ", j: " << j << endl;
  return int(k*j);
}

class Bnj: public Lambda<int>
{
private:
  Bnj();
  int k; 
  double j;

public:
  Bnj(int k_, double j_): k(k_), j(j_){}

  virtual void toApply()
  {
    returnValue = bnj(k, j);
  }
};

class Salutation: public Lambda<void>
{
private:
  Salutation();
  string m;
public:
  Salutation(string m_): m(m_){}
  virtual void toApply()
  {
    salutation(m);
  }
};

template <class RT>
void doIt(Lambda<RT> f)
{
  f.apply();
}*/
