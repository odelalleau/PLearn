
#include "TransposeVariable.h"
#include <plearn/var/Var_utils.h>

namespace PLearn {
using namespace std;


/** TransposeVariable **/

TransposeVariable::TransposeVariable(Variable* v)
    :UnaryVariable(v, v->width(), v->length()), startk(0)
{}


PLEARN_IMPLEMENT_OBJECT(TransposeVariable, "ONE LINE DESCR", "NO HELP");


void TransposeVariable::recomputeSize(int& l, int& w) const
{ l=input->width(); w=input->length(); }








void TransposeVariable::fprop()
{
    if(input->length()==1 || input->width()==1) 
    {
        real* inputdata = input->valuedata;
        for(int k=0; k<nelems(); k++)
            valuedata[k] = inputdata[k];
    }
    else // general case
    {
        real* inputrowdata = input->valuedata;
        int thiskcolstart = 0; // element index of start of column in this var
        for(int i=0; i<width(); i++) // the width() of this var is the length() of the submat
        {
            int thisk = thiskcolstart++;
            for(int j=0; j<length(); j++, thisk+=width()) // the length() of this var is the width() of the submat
                valuedata[thisk] = *inputrowdata++;
        }
    }
}


void TransposeVariable::bprop()
{
    if(input->length()==1 || input->width()==1) 
    {
        real* inputdata = input->gradientdata;
        for(int k=0; k<nelems(); k++)
            inputdata[k] += gradientdata[k];
    }
    else // general case
    {
        real* inputrowdata = input->gradientdata;
        int thiskcolstart = 0; // element index of start of column in this var
        for(int i=0; i<width(); i++) // the width() of this var is the length() of the submat
        {
            int thisk = thiskcolstart++;
            for(int j=0; j<length(); j++, thisk+=width()) // the length() of this var is the width() of the submat
                *inputrowdata++ += gradientdata[thisk];
        }
    }
}


void TransposeVariable::symbolicBprop()
{
    PLERROR("Not implemented yet!");
    /*
      int i = startk/input->width();
      int j = startk%input->width();
      int topextent = i;
      int bottomextent = input->length()-(i+width()); // the width() of this var is the length() of the submat
      int leftextent = j;
      int rightextent = input->width()-(j+length()); // the length() of this var is the width() of the submat
      input->accg(extend(transpose(g),topextent,bottomextent,leftextent,rightextent));
    */
}


void TransposeVariable::rfprop()
{
    PLERROR("Not implemented yet!");
    /*
      if (rValue.length()==0) resizeRValue();
      if(input->length()==1 || input->width()==1) // optimized version for this special case...
      {
      real* inputdata = input->rvaluedata+startk;
      for(int k=0; k<nelems(); k++)
      rvaluedata[k] = inputdata[k];
      }
      else // general case
      {
      real* inputrowdata = input->rvaluedata+startk;
      int thiskcolstart = 0; // element index of start of column in this var
      for(int i=0; i<width(); i++) // the width() of this var is the length() of the submat
      {
      int thisk = thiskcolstart++;
      for(int j=0; j<length(); j++, thisk+=width()) // the length() of this var is the width() of the submat
      rvaluedata[thisk] = inputrowdata[j];
      inputrowdata += input->width();
      }
      }
    */
}



} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
