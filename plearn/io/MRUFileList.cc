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


#include "MRUFileList.h"

namespace PLearn <%

MRUFileList::MRUFileList(int _max_opened_files, ios_base::openmode _mode)
  :max_opened_files(_max_opened_files),tot_miss(0),tot_access(0),mode(_mode)
{}

ofstream * MRUFileList::getFile(string fname)

{
  tot_access++;
  list_typ::iterator it = mru_list.end();
  ofstream * ofile;
  
  // search for the wanted file in the MRU list
  for(list_typ::iterator it2 = mru_list.begin(); it2 != mru_list.end(); ++it2)
    if(it2->first == fname)
    {
      it=it2;
      break;
    }

  // the file is not currently opened. 
  if(it == mru_list.end())
  {
    tot_miss++;
    // If we have already the max of opened files, we close the least recently used
    if((signed)mru_list.size() >= max_opened_files)
    {
      (mru_list.back().second)->close();
      delete (mru_list.back().second);
      mru_list.pop_back();
    }
    ofile = new ofstream(fname.c_str(),mode);
    if(ofile->bad())
    {
      error="Could not open/create file "+fname+".";
      return NULL;
    }
    mru_list.push_front(make_pair(fname,ofile));
    return ofile;
  }
  
  // else, we move the file to the top of the list and return the associated ofstream ptr
  
  ofile=it->second;
  mru_list.erase(it);
  mru_list.push_front(make_pair(fname,ofile));
  return ofile;
}
  
MRUFileList::~MRUFileList()
{
  for(list_typ::iterator it = mru_list.begin(); it != mru_list.end(); ++it)
  {
    (it->second)->close();
    delete it->second;
  }
}

%> //end of namespace PLearn
