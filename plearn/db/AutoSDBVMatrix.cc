// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: AutoSDBVMatrix.cc,v 1.6 2004/03/04 14:53:01 tihocan Exp $
   * AUTHOR: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

#include "AutoSDBVMatrix.h"
//#include "stringutils.h"


namespace PLearn {
using namespace std;

AutoSDBVMatrix::AutoSDBVMatrix(const string& dbname)
  :sdb_(extract_filename(dbname), extract_directory(dbname), SDB::readonly, true), string_field_map()
{
  metadatadir = extract_directory(dbname) + extract_filename(dbname) + ".metadata";
  if(!force_mkdir(metadatadir))
    PLWARNING("In AutoSDBVMatrix constructor, could not create directory %s",metadatadir.c_str());

  const Schema& sc = sdb_.getSchema();

  //get string mappings from sdb metadatadir
  getMappings();

  row_ = Row(&sc);
  Schema::const_iterator it = sc.begin();
  Schema::const_iterator itend = sc.end();

  width_= sdb_.width();
  length_ = sdb_.length();

  // resize the string mappings (TODO/WARNING : two string maping systems coexist (not so peacfully), next 2 line are from the newer system but
  // transition is uncompleted
  map_sr = TVec<map<string,real> >(width_);
  map_rs = TVec<map<real,string> >(width_);

  int i=0;
  for(it=sc.begin(); it!=itend; ++it)
    {
      if(it->field_type==DateType)
        declareField(i++, it->name, VMField::Date);
      else
	declareField(i++, it->name, VMField::UnknownType);
    }
}

void AutoSDBVMatrix::getRow(int i, Vec v) const
{
  sdb_.getInRow(i, row_);
  Row::const_iterator it = row_.begin();
  int w = width();
  if(w!=v.length())
    PLERROR("In AutoSDBVMatrix::getRow length of v must be width of VMatrix");

  int j=0;
  while(j<w)
    {
      if(it.isString())
	v[j]= string_field_map.find(it.name())->second[it.asString()];
      else if(it.isMissing())
	v[j] = MISSING_VALUE;
      else if(it.isCharacter())
	v[j] = (real)*(it.asCharacter());
      else
	v[j] = (real)it.toDouble();
      ++j;
      ++it;
    }
}


void AutoSDBVMatrix::getMappings()
{
  const Schema& sc = sdb_.getSchema();
  
  for(Schema::const_iterator it= sc.begin(); it < sc.end(); ++it)
    if(it->field_type == StringType)
      {
	string field_filename= metadatadir + slash + it->name + ".strings";
	real dft_val= MISSING_VALUE;
	//get value for others, if file exists
	if(isfile(field_filename + ".others"))
	  PLearn::load(field_filename + ".others", dft_val);
	//get mapping, if it exists
	string_field_map[it->name]= StringFieldMapping(field_filename, dft_val);
	num2string_map[it->name]= NumToStringMapping(field_filename);
      }
}


} // end of namespace PLearn


