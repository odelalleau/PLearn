// -*- C++ -*-

// Copyright (c) 2002 by Julien Keable

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

#include <plearn/io/fileutils.h>      //!< For countNonBlankLinesOfFile.
#include "StringTable.h"
#include "stringutils.h"    //!< For left.


namespace PLearn {
using namespace std;


ostream& operator<<(ostream& out,const StringTable& st)
{
  // find out width of each columns
  TVec<size_t> colsiz(st.width(),(size_t)0);
  for(int j=0;j<st.length();j++)
    {
      TVec<string> row=st.data(j);      
      for(int i=0;i<st.width();i++)
        if((size_t)row[i].length() > colsiz[i])
          colsiz[i]=(size_t)row[i].length();
    }
  for(int i=0;i<st.width();i++)
    if(st.fieldnames[i].length() > colsiz[i])
      colsiz[i]=(size_t)st.fieldnames[i].length();

  out<<"#: "; 
  for(int i=0;i<st.width();i++)
    out<<left(st.fieldnames[i],colsiz[i]+3);    
  out<<"\n";

    for(int j=0;j<st.length();j++)
    {
      TVec<string> row=st.data(j);      
      out<<"   ";
      for(int i=0;i<st.width();i++)
        out<<left(row[i],colsiz[i])<<";";
      out<<"\n";
    }

  return out;

}

void StringTable::appendRow(const list<pair<string,string> > &row)
{
  vector<string> vec;
  data.resize(data.length()+1,data.width());
  int rownum=data.length()-1;
  for(list<pair<string,string> >::const_iterator it=row.begin();it!=row.end();++it)
    {
      int colnum;
      map<string,int>::iterator revit=rev_fn.find(it->first);
      if(revit==rev_fn.end())
        {
          colnum=data.width();
          TMat<string> tmp(data.length(),colnum+1);
          tmp.subMatColumns(0,colnum)<<data;
          data=tmp;
          rev_fn[it->first]=colnum;
          fieldnames.push_back(it->first);;
        }
      else colnum=rev_fn[it->first];
      data(rownum,colnum)=it->second;      
    }
}

// This function will go through the list and will add columns as new fields are encountered.
// It will do nothing on existing fields. Its use is mainly to force column order.
// The second string in the pair is ignored.. the type of row is like this for convenience
// ** Note : Do not use if the Table has more than 1 row (because resize won't work)
void StringTable::declareFields(const list<pair<string,string> > & row)
{
  for(list<pair<string,string> >::const_iterator it=row.begin();it!=row.end();++it)
    {
      map<string,int>::iterator revit=rev_fn.find(it->first);
      if(revit==rev_fn.end())
        {
          data.resize(data.length(),data.width()+1);
          rev_fn[it->first]=data.width()-1;
          fieldnames.push_back(it->first);;
        }
    }
}

StringTable::StringTable(){}

StringTable::StringTable(const string & filename)
{
  int nrows= countNonBlankLinesOfFile(filename);
  string str;
  ifstream in(filename.c_str());
  in.ignore(2);
  if(in.peek()==' ')
    in.ignore(1);
  getline(in,str);
  fieldnames=split(str);
  data.resize(nrows, (int)fieldnames.size());
  int rnum=0;
  getline(in,str);
  while(removeblanks(str)!="")
    {
/*
      vector<string> line;
      size_t pos,lpos=0;
      while((pos=str.find(";",lpos))!=string::npos)
      {
        line.push_back(str.substr(lpos,pos-lpos));
        lpos=pos+1;
      }
      line.push_back(str.substr(lpos,str.size()-lpos));
      if(line.size()!=fieldnames.size())
        PLERROR("in row %i : elements (%i)  mismatch number of fields (%i)",rnum,line.size(),fieldnames.size());
*/
      vector<string> line=split(str,";");  
      //line.pop_back(); // last string found is garbage *** NO, not true...!
      if(line.size()!=fieldnames.size())
        PLERROR("in row %i : elements (%i)  mismatch number of fields (%i)",rnum,line.size(),fieldnames.size());

      for(unsigned int i= 0; i < line.size(); ++i)
        data(rnum,i)= line[i];
      ++rnum;
      getline(in,str);
    }
  data.resize(rnum, (int)fieldnames.size());
}


} // end of namespace PLearn
