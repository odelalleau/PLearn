// -*- C++ -*-

// ProcessingVMatrix.cc
//
// Copyright (C) 2003 Pascal Vincent 
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
   * $Id: ProcessingVMatrix.cc,v 1.11 2005/04/28 23:56:17 chapados Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file ProcessingVMatrix.cc */


#include "ProcessingVMatrix.h"

namespace PLearn {
using namespace std;


ProcessingVMatrix::ProcessingVMatrix()
  :inherited()
  /* ### Initialise all fields to their default value */
{
  // ...

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

PLEARN_IMPLEMENT_OBJECT(
  ProcessingVMatrix,
  "VMatrix whose rows are processed using a VPL script",
  "VPL (VMat Processing Language) is a home brewed mini-language in postfix\n"
  "notation. As of today, it is used is the {PRE,POST}FILTERING and\n"
  "PROCESSING sections of a .vmat file. It supports INCLUDEs instructions\n"
  "and DEFINEs (dumb named string constants). It can handle reals as well\n"
  "as dates (format is: CYYMMDD, where C is 0 (1900-1999) or 1\n"
  "(2000-2099). For more info, you can look at PLearnCore/VMatLanguage.*.\n"
  "\n"
  "A VPL code snippet is always applied to the row of a VMatrix, and can\n"
  "only refer to data of that row (in the state it was before any\n"
  "processing.) The result of the execution will be a vector which is the\n"
  "execution stack at code termination, defining the row of same index in\n"
  "the resulting matrix.\n"
  "\n"
  "When you use VPL in a PROCESSING section, each field you declare must\n"
  "have its associated fieldname declaration. The compiler will ensure that\n"
  "the size of the result vector and the number of declared fieldnames\n"
  "match. This doesn't apply in the filtering sections, where you don't\n"
  "declare fieldnames, since the result is always a single value.\n"
  "\n"
  "To declare a fieldname, use a colon with the name immediately after. To\n"
  "batch-declare fieldnames, use eg. :myfield:1:10. This will declare\n"
  "fields myfield1 up to myfield10.\n"
  "\n"
  "There are two notations to refer to a field value: the @ symbol followed\n"
  "by the fieldname, or % followed by the field number.\n"
  "\n"
  "To batch-copy fields, use the following syntax : [field1:fieldn] (fields\n"
  "can be in @ or % notation).\n"
  "\n"
  "Here's a real-life example of a VPL program:\n"
  "\n"
  "    @lease_indicator 88 == 1 0 ifelse :lease_indicator\n"
  "    @rate_class 1 - 7 onehot :rate_class:0:6\n"
  "    @collision_deductible { 2->1; 4->2; 5->3; 6->4; 7->5;\n"
  "       [8 8]->6; MISSING->0; OTHER->0 }\n"
  "      7 onehot :collision_deductible:0:6\n"
  "    @roadstar_indicator 89 == 1 0 ifelse :roadstar_indicator\n"
  "\n"
  "In the following, the syntax\n"
  "\n"
  "    a b c -> f(a,b,c)\n"
  "\n"
  "means that (a,b,c) in that order (i.e. 'a' bottommost and 'c' top-of-stack)\n"
  "are taken from the stack, and the result f(a,b,c) is pushed on the stack\n"
  "\n"
  "List of valid VPL operators:\n"
  "\n"
  " _ pop            : pop last element from stack\n"
  " _ dup            : duplicates last element on the stack\n"
  " _ exch           : exchanges the two top-most elements on the stack\n"
  " _ onehot         : index nclasses --> one-hot representation of index\n"
  " _ +              : a b   -->  a + b\n"
  " _ -              : a b   -->  a - b\n"
  " _ *              : a b   -->  a * b\n"
  " _ /              : a b   -->  a / b\n"
  " _ neg            : a     -->  -a\n"
  " _ ==             : a b   -->  a == b\n"
  " _ !=             : a b   -->  a != b\n"
  " _ >              : a b   -->  a >  b\n"
  " _ >=             : a b   -->  a >= b\n"
  " _ <              : a b   -->  a <  b\n"
  " _ <=             : a b   -->  a <= b\n"
  " _ and            : a b   -->  a && b\n"
  " _ or             : a b   -->  a || b\n"
  " _ not            : a     -->  !a\n"
  " _ ifelse         : a b c -->  (a != 0? b : c)\n"
  " _ fabs           : a     -->  fabs(a)\n"
  " _ rint           : a     -->  rint(a)   ; round to closest int\n"
  " _ floor          : a     -->  floor(a)\n"
  " _ ceil           : a     -->  ceil(a)\n"
  " _ log            : a     -->  log(a)    ; natural log\n"
  " _ exp            : a     -->  exp(a)    ; e^a\n"
  " _ rowindex       : pushes the row number in the VMat on the stack\n"
  " _ isnan          : true if missing value\n"
  " _ missing        : pushes a missing value\n"
  " _ year           : CYYMMDD --> YYYY\n"
  " _ month          : CYYMMDD --> MM\n"
  " _ day            : CYYMMDD --> DD\n"
  " _ daydiff        : nb. days\n"
  " _ monthdiff      : continuous: nb. days / (365.25/12)\n"
  " _ yeardiff       : continuous: nb. days / 365.25\n"
  " _ year_month_day : CYYMMDD      --> YYYY MM DD\n"
  " _ todate         : YYYY MM DD   --> CYYMMDD\n"
  " _ dayofweek      : from CYYMMDD --> [0..6] (0=monday  6=sunday)\n"
  " _ today          : todays date CYYMMDD\n"
  " _ date2julian    : CYYMMDD      --> nb. days\n"
  " _ julian2date    : nb. days     --> CYYMMDD\n"
  " _ min            : b a  -->  (a<b? a : b)\n"
  " _ max            : b a  -->  (a<b? b : a)\n"
  " _ sqrt           : a    -->  sqrt(a)    ; square root\n"
  " _ ^              : b a  -->  pow(a,b)   ; a^b\n"
  " _ modulo         : b a  -->  int(b) % int(a)\n"
  " _ vecscalmul     : x1 ... xn n alpha  -->  (x1*alpha) ... (xn*alpha)\n"
  " _ select         : v0 v1 v2 v3 ... vn-1 n i  -->  vi  \n"
  " _ length         : the length of the currently processed column.\n"
  " _ sign           : a  -->  sign(a)  (0 -1 or +1)\n"
  " _ get            : pos  -->  value_of_stack_at_pos\n"
  "                    (if pos is negative then it's relative to stacke end\n"
  "                    ex: -1 get will get the previous element of stack)\n"
  " _ memput         : a mempos  -->    (a is saved in memory position mempos)\n"
  " _ memget         : mempos    --> a  (gets 'a' from memory in position mempos)\n"
  " _ sumabs         : v0 v1 v2 ... vn  -->  sum_i |vi|\n"
  "                    (no pop, and starts from the beginning of the stack)\n"
  );

void ProcessingVMatrix::getNewRow(int i, const Vec& v) const
{
  program.run(i,v);
}

void ProcessingVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "prg", &ProcessingVMatrix::prg, OptionBase::buildoption,
                "The VPL code to be applied to each row of the vmat");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void ProcessingVMatrix::build_()
{
  vector<string> fieldnames;
  program.setSource(source);
  program.compileString(prg,fieldnames); 
  int nfields = (int)fieldnames.size();
  width_ = nfields;

  fieldinfos.resize(nfields);
  for(int j=0; j<nfields; j++)
    fieldinfos[j] = VMField(fieldnames[j]);

  setMetaInfoFromSource();
  sourcevec.resize(source->width());
}

void ProcessingVMatrix::setMetaDataDir(const PPath& the_metadatadir)
{
  inherited::setMetaDataDir(the_metadatadir);
  length_ = source->length();
}

// ### Nothing to add here, simply calls build_
void ProcessingVMatrix::build()
{
  inherited::build();
  build_();
}

void ProcessingVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn

