// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Pascal Vincent and Julien Keable
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
   * $Id: VMatLanguage.cc,v 1.12 2003/10/08 23:01:57 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "VMatLanguage.h"
#include "stringutils.h"
#include "fileutils.h"
#include "getDataSet.h"
#include "PDate.h"

namespace PLearn <%
using namespace std;
  
  bool VMatLanguage::output_preproc=false;

  // returns oldest modification date of a file containing VPL code, searching recursively every
  // file placed after a INCLUDE token
  time_t getDateOfCode(const string& codefile)
  {
    time_t latest = mtime(codefile);
    string token;  
    ifstream in(codefile.c_str());
    if(in.bad())
      PLERROR("Cannot open file : %s",codefile.c_str());
  
    in >> token;
    while(!in.eof())
      {
        if(token=="INCLUDE")
          {
            in >> token;
            time_t t=getDateOfCode(token);
            if(t>latest)
              latest=t;
          }
        in >> token;
      }
    return latest;
  }

  map<string, int> VMatLanguage::opcodes;

  // this function (that really should be sliced to to smaller pieces someday) takes raw VPL code and 
  // returns the preprocessed sourcecode along with the defines and the fieldnames it generated
  void VMatLanguage::preprocess(istream& in, map<string, string>& defines,  string& processed_sourcecode, vector<string>& fieldnames)
  {
    // PStream pout(&cout);
    // pout << defines << endl;
    char buf[500];
    string token;
    unsigned int spos;
    map<string,string>::iterator pos;
    while(in >> token)
      {
        pos=defines.find(token);

        // are we sitting on a mapping declaration?
        if(token[0]=='{')
          {
            //skip mapping to avoid brackets conflicts with fieldcopy macro syntax
            char car;
            processed_sourcecode+=token;
            // if the token is only a part of the mapping...
            if(token.find("}")==string::npos)
              {
                // just eat till the end of the mapping
                while((car=in.get())!='}' && !in.eof())
                  processed_sourcecode+=car;
                processed_sourcecode+="}";
              }
          }
        // did we find a fieldName declaration?
        // format is either :myField or :myField:a:b
        else if(token[0]==':')
          {
            if(isBlank(token.substr(1)))
              PLERROR("Found a ':' with no fieldname. Do not put a whitespace after the ':'");
            vector<string> parts=split(token,":");
            if(parts.size()==3)
              {
                int a=toint(parts[1]);
                int b=0;
                // let the chance for the second interval boundary to be a "DEFINE"
                // this is used with onehot and @myfield.ranges10.nbins
                // ie: @myfield.onehot10 :myfieldonehot:0:@myfield.ranges10.nbins 
                if(pl_isnumber(parts[2]))
                  b=toint(parts[2]);
                else 
                  {
                    if(defines.find(parts[2])!=defines.end())
                      b=toint(defines[parts[2]]);
                    else 
                      PLERROR("found a undefined non-numeric boundary in multifield declaration : '%s'",parts[2].c_str());
                  }

                for(int i=a;i<=b;i++)
                  fieldnames.push_back(parts[0]+tostring(i));
              }
            else if (parts.size()==1)
              fieldnames.push_back(token.substr(1));
            else PLERROR("Strange fieldname format (multiple declaration format is :label:0:10");
          }
        // did we find a fieldcopy macro?
        else if(token[0]=='[')
          {
            vector<string> parts=split(token.substr(1),":]");

            // fieldcopy macro type is [start,end]
            // fields can be refered to as %number or @name
            if(parts.size()==2)
              {
                string astr=parts[0].substr(1);
                string bstr=parts[1].substr(1);
                int a=-1,b=-1;
                
                if(parts[0][0]=='@')
                  {
                    for(int i=0;i<vmsource.width();i++)
                      if(vmsource->fieldName(i)==astr){a=i;break;}
                  }
                else if(parts[0][0]=='%')
                  a=toint(parts[0].substr(1));
                else PLERROR("fieldcopy macro syntax is : [start:end] EG: [@year:%6]. 'end' must be after 'start'.. OR [field] to copy a single field");
                
                if(parts[1][0]=='@')
                  {
                    for(int i=0;i<vmsource.width();i++)
                      if(vmsource->fieldName(i)==bstr){b=i;break;}
                  }
                else if(parts[1][0]=='%')
                  b=toint(parts[1].substr(1));
                else PLERROR("fieldcopy macro syntax is : [start:end] EG: [@year:%6]. 'end' must be after 'start'.. OR [field] to copy a single field");
                
                if(a>b)
                  PLERROR("In copyfield macro, you have specified a start field that is after the end field. Eg : [%10:%5]");
                if(a==-1)
                  PLERROR("In copyfield macro, unknown field :%s",astr.c_str());
                if(b==-1)
                  PLERROR("In copyfield macro, unknown field :%s",astr.c_str());

                for(int i=a;i<=b;i++)
                  {
                    processed_sourcecode+=string("%")+tostring(i)+ " ";
                    fieldnames.push_back(vmsource->fieldName(i));
                  }
              }
            // fieldcopy macro type is [field]
            else if(parts.size()==1)
            {
              string astr=parts[0].substr(1);
              int a=-1;
              if(parts[0][0]=='@')
              {
                for(int i=0;i<vmsource.width();i++)
                  if(vmsource->fieldName(i)==astr){a=i;break;}
              }
              else if(parts[0][0]=='%')
                a=toint(parts[0].substr(1));
              else PLERROR("fieldcopy macro syntax is : [start:end] EG: [@year:%6]. 'end' must be after 'start'.. OR [field] to copy a single field");
              if(a==-1)
                PLERROR("In copyfield macro, unknown field :%s",astr.c_str());
              processed_sourcecode+=string("%")+tostring(a)+ " ";
              fieldnames.push_back(vmsource->fieldName(a));
            }
            else PLERROR("Strange fieldcopy format. e.g : [%0:%5]. Found parts %s",join(parts," ").c_str());
          }

        // did we find a comment?
        else if(token[0]=='#')
          skipRestOfLine(in);

        // include declaration
        else if(token=="INCLUDE")
          { 
            in >> token;
            // Try to be intelligent and find out if the file belongs directly to another .?mat (the case of a 
            // stats file for example) and warn if the file is out of date

            // Mhhh.. is this still pertinent? This "stats" and "bins" thing is semi-standard I think
            unsigned int idx_meta  =  token.find(".metadata");
            unsigned int idx_stats =  token.find("stats.");
            unsigned int idx_bins  =  token.find("bins.");
            if(idx_meta!=string::npos && (idx_stats!=string::npos || idx_bins!=string::npos))
              {
                string file=token.substr(0,idx_meta);
                if(getDataSetDate(file) > mtime(token))
                  PLWARNING("File %s seems out of date with parent matrix %s",token.c_str(),file.c_str());
              }
            
            ifstream incfile(token.c_str());
            if(incfile.bad())
              PLERROR("Cannot open file %s\n",token.c_str());
            // process recursively this included file
            // **POSSIBLE DRAWBACK : defines done in this file will be used in the next recursion level
            preprocess(incfile,defines, processed_sourcecode,fieldnames);
	    
          }
        // define declaration
        else if(token=="DEFINE")
          {
            in >> token;
            in.getline(buf,500);
            defines[token.c_str()]=string(buf);
          }
        else if(pos!=defines.end())
          {
            // the token is a macro (define) so we process it recursively until it's stable
            // (necessary since the define macro can use defines recursively)
            string oldstr=pos->second,newstr;
            bool unstable=true;
            while(unstable)
              {
                istrstream strm(oldstr.c_str());
                newstr="";
                preprocess(strm,defines,newstr,fieldnames);
                if(removeblanks(oldstr)==removeblanks(newstr))
                  unstable=false;
                oldstr=newstr;
              }
            processed_sourcecode+=newstr + " ";
          }
        // did we find a reference to a string value of a VMatrix that has overloaded getStringVal(..) e.g.:StrTableVMatrix
        // In VPL, you can push on the stack the value of a string according to the string map of a particular column
        // e.g. : to push value of string "WBush" from field MostSuspectAmericanPresidents, write @MostSuspectsAmericanPresidents."WBush"
        else if ((token[0]=='@' || token[0]=='%') && token[token.length()-1]=='"' && (spos=token.find(".\""))!=string::npos)
          
          {
            string colname=token.substr(1,spos-1);
            string str=token.substr(spos+2,token.length()-spos-3);
            // do we have a named field reference?
            if(token[0]=='@')
              {
                pos=defines.find(string("@")+colname);                
                if(pos==defines.end())
                  PLERROR("unknown field : %s",colname.c_str());
                colname=pos->second.substr(1);
              }
            int colnum=toint(colname);
            real r=vmsource->getStringVal(colnum,str);
            if(is_missing(r))
              PLERROR("%s : %s is not a known string for this field",token.c_str(),str.c_str());
            processed_sourcecode+=tostring(r)+" ";
          }
        else processed_sourcecode+=token + " ";
      }
  }

  //  generate bytecode from preprocessed sourcecode
  void VMatLanguage::generateCode(istream& processed_sourcecode)
  {
    char car;
    string token;
    map<string,int>::iterator pos;
    car=peekAfterSkipBlanks(processed_sourcecode);
    while(!processed_sourcecode.eof())
      {
        if (car=='{')
          {
            int mapnum = mappings.size();
            mappings.resize(mapnum+1);
            mappings[mapnum].read(processed_sourcecode);
            program.append(opcodes["__applymapping"]);
            program.append(mapnum);
          }
        else 
          {
            processed_sourcecode>>token;
            if( pl_isnumber(token))
              // assume we have a float
              {
                float zefloat=tofloat(token);
                program.append(opcodes["__insertconstant"]);
                program.append(*(int*)&zefloat);
              }
            else if (token[0]=='%')
              {
                vector<string> parts=split(token,":");
                if (parts.size()==1) // expecting e.g. %10 for column 10
                  {
                    program.append(opcodes["__getfieldval"]);
                    int val=toint(token.substr(1));
                    program.append(val);
                  }
                else if (parts.size()==2) // expecting e.g. %10-%20 for columns 10 to 20 inclusive
                  {
                    program.append(opcodes["__getfieldsrange"]);
                    int a=toint(parts[0].substr(1));
                    int b=toint(parts[1].substr(1));
                    program.append(a);
                    program.append(b);
                  }
              }
            else
              {
                pos=opcodes.find(token);
                if(pos!=opcodes.end())
                  program.append(pos->second);
                else PLERROR("Undefined keyword : %s",token.c_str());
              }
          }
        car=peekAfterSkipBlanks(processed_sourcecode);
      }
  }

  void VMatLanguage::generateCode(const string& processed_sourcecode)
  {
    istrstream in(processed_sourcecode.c_str());
    generateCode(in);
  }

  //void declareField(int fieldindex, const string& fieldname, VMField::FieldType fieldtype=VMField::UnknownType)

  void VMatLanguage::compileString(const string & code, vector<string>& fieldnames)
  {
    istrstream in(code.c_str());
    compileStream(in,fieldnames);
  }

  void VMatLanguage::compileFile(const string & filename, vector<string>& fieldnames)
  {
    ifstream in(filename.c_str());
    if(in.bad())
      PLERROR("Cannot open file %s",filename.c_str());
    compileStream(in,fieldnames);
  }

  void VMatLanguage::compileStream(istream & in, vector<string>& fieldnames)
  {
    map<string,string> defines;
    string processed_sourcecode;

    program.resize(0);
    mappings.resize(0);
    
    // first, warn user if a fieldname appears twice or more in the source matrix
    for(int i=0;i<vmsource.width();i++)
      {
        if(defines.find(string("@")+vmsource->fieldName(i)) != defines.end())
          PLERROR("fieldname %s is duplicate in processed matrix",(string("@")+vmsource->fieldName(i)).c_str());
        defines[string("@")+vmsource->fieldName(i)]=string("%")+tostring(i);
      }

    // the filednames parameter is an output vector in which we put the fieldnames of the final VMat
    fieldnames.clear();
    preprocess(in, defines, processed_sourcecode, fieldnames);

    if(output_preproc)
      {
        cerr<<"Preprocessed code:"<<endl<<processed_sourcecode<<endl;
        cerr<<"FieldNames : "<<endl<<fieldnames<<endl;
      }
    generateCode(processed_sourcecode);
  }

  PLEARN_IMPLEMENT_OBJECT(VMatLanguage, "ONE LINE DESCR", "NO HELP");

  //! builds the map if it does not already exist
  void VMatLanguage::build_opcodes_map()
  {
    if(opcodes.empty())
      {
        opcodes["__insertconstant"] = 0; // followed by a floating point number (4 bytes, just like int) 
        opcodes["__getfieldval"] = 1; // followed by field# 
        opcodes["__applymapping"] = 2; // followed by mapping#
        opcodes["pop"] = 3;
        opcodes["dup"] = 4;
        opcodes["exch"] = 5;
        opcodes["onehot"] = 6;
        opcodes["+"]  = 7;
        opcodes["-"] = 8;
        opcodes["*"] = 9;
        opcodes["/"] = 10;
        opcodes["=="] = 11;
        opcodes["!="] = 12;
        opcodes[">"] = 13;
        opcodes[">="] = 14;
        opcodes["<"] = 15;
        opcodes["<="] = 16;
        opcodes["and"] = 17;
        opcodes["or"] = 18;
        opcodes["not"] = 19;
        opcodes["ifelse"] = 20;
        opcodes["fabs"] = 21;
        opcodes["rint"] = 22;
        opcodes["floor"] = 23;
        opcodes["ceil"] = 24;
        opcodes["log"] = 25;
        opcodes["exp"] = 26;
        opcodes["rowindex"] = 27; // pushes the rownum on the stack
        opcodes["isnan"] = 28; 
        opcodes["year"] = 29; // from format CYYMMDD -> YYYY
        opcodes["month"] = 30; // CYYMMDD -> MM
        opcodes["day"] = 31;  // CYYMMDD -> DD
        opcodes["daydiff"] = 32; // nb. days 
        opcodes["monthdiff"] = 33; // continuous: nb. days / (365.25/12)
        opcodes["yeardiff"] = 34; // continuous: nb. days / 365.25
        opcodes["year_month_day"] = 35; // CYYMMDD -> YYYY MM DD 
        opcodes["todate"] = 36; // YYYY MM DD -> CYYMMDD  
        opcodes["dayofweek"] = 37; // from CYYMMDD -> [0..6] (0=monday; 6=sunday)
        opcodes["today"] = 38; // today's date CYYMMDD
        opcodes["date2julian"] = 39; // CYYMMDD -> nb. days
        opcodes["julian2date"] = 40; // nb. days -> CYYMMDD  
        opcodes["min"] = 41; // a b -> (a<b? a : b)
        opcodes["max"] = 42; // a b -> (a<b? b : a)
        opcodes["sqrt"] = 43; 
        opcodes["^"] = 44; 
        opcodes["mod"] = 45;
        opcodes["vecscalmul"] = 46; // x1 ... xn n alpha --> (x1*alpha) ... (xn*alpha)
        opcodes["__getfieldsrange"] = 47; // %N:%M pushes field %N up to %M. M must be >= N.
      }
  }

  void VMatLanguage::run(int rowindex, const Vec& result) const
  {
    real a,b,c;
    
    pstack.resize(0);
    TVec<int>::iterator pptr = program.begin();
    TVec<int>::iterator pptrend = program.end();
    Vec myvec(vmsource.width());
    vmsource->getRow(rowindex,myvec);
    real* pfieldvalues = myvec.data();
    while(pptr!=pptrend)
      {
        int op = *pptr++;
        switch(op)
          {
          case 0: // insertconstant
            pstack.push(*((float*)pptr++));
            break;
          case 1: // getfieldval
            //if(*pptr > fieldvalues.width()) PLERROR("Tried to acces an out of bound field in VPL code");
            pstack.push(pfieldvalues[*pptr++]);
            break;
          case 2: // applymapping
            pstack.push(mappings[*pptr++].map(pstack.pop()));
            break;
          case 3: // pop
            pstack.pop();
            break;
          case 4: // dup
            pstack.push(pstack.top());
            break;
          case 5: // exch
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(b);
            pstack.push(a);
            break;
          case 6: // onehot
            {
              int nclasses = int(pstack.pop());
              int index = int(pstack.pop()); 
              for(int i=0; i<nclasses; i++)
                pstack.push(i==index ?1 :0);
            }
            break;
          case 7: // +
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(a+b);
            break;
          case 8: // -
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(a-b);
            break;
          case 9: // *
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(a*b);
            break;
          case 10: // /
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(a/b);
            break;
          case 11: // ==
            b = pstack.pop();
            a = pstack.pop();
            pstack.push( ((float)a==(float)b) ?1 :0);
            break;
          case 12: // !=
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(((float)a!=(float)b) ?1 :0);
            break;
          case 13: // >
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(((float)a>(float)b) ?1 :0);
            break;
          case 14: // >=
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(((float)a>=(float)b) ?1 :0);
            break;
          case 15: // <
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(((float)a<(float)b) ?1 :0);
            break;
          case 16: // <=
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(((float)a<=(float)b) ?1 :0);
            break;
          case 17: // and
            b = pstack.pop();
            a = pstack.pop();
            pstack.push((a&&b) ?1 :0);
            break;
          case 18: // or
            b = pstack.pop();
            a = pstack.pop();
            pstack.push((a||b) ?1 :0);
            break;
          case 19: // not
            pstack.push((pstack.pop()==0) ?1 :0);
            break;
          case 20: // ifelse
            c = pstack.pop();
            b = pstack.pop();
            a = pstack.pop();
            pstack.push((a!=0)?b:c);
            break;
          case 21: // fabs
            pstack.push(fabs(pstack.pop()));
            break;
          case 22: // rint
            pstack.push(rint(pstack.pop()));
            break;
          case 23: // floor
            pstack.push(floor(pstack.pop()));
            break;
          case 24: // ceil
            pstack.push(ceil(pstack.pop()));
            break;
          case 25: // log
            pstack.push(log(pstack.pop()));
            break;
          case 26: // exp
            pstack.push(exp(pstack.pop()));
            break;
          case 27: // rowindex
            pstack.push(real(rowindex));
            break;
          case 28: // isnan
            pstack.push(isnan(pstack.pop())?1:0);
            break;
          case 29: //year
            pstack.push(float_to_date(pstack.pop()).year);
            break;
          case 30: //month
            pstack.push(float_to_date(pstack.pop()).month);
            break;
          case 31: //day
            pstack.push(float_to_date(pstack.pop()).day);
            break;
          case 32: //daydiff
            b= pstack.pop();
            a= pstack.pop();
            if (!isnan(a) && !isnan(b)) {
              pstack.push(float_to_date(a)-float_to_date(b));
            }
            else {
              pstack.push(MISSING_VALUE);
            }
            break;
          case 33: //monthdiff
            b= pstack.pop();
            a= pstack.pop();
            pstack.push((float_to_date(a)-float_to_date(b))*(12.0/365.25));
            break;
          case 34: //yeardiff
            b= pstack.pop();
            a= pstack.pop();
            pstack.push((float_to_date(a)-float_to_date(b))/365.25);
            break;
          case 35: //year_month_day
            {
              PDate d(float_to_date(pstack.pop()));
              pstack.push(d.year);
              pstack.push(d.month);
              pstack.push(d.day);
            }
            break;
          case 36: //todate
            c = pstack.pop();
            b = pstack.pop();
            a = pstack.pop();
            pstack.push(date_to_float(PDate((int)a, (int)b, (int)c)));
            break;
          case 37: //dayofweek
            pstack.push(float_to_date(pstack.pop()).dayOfWeek());
            break;
          case 38: //today
            pstack.push(date_to_float(PDate::today()));
            break;
          case 39: //date2julian
            pstack.push(float_to_date(pstack.pop()).toJulianDay());
            break;
          case 40: //julian2date
            pstack.push(date_to_float(PDate((int)pstack.pop())));
            break;
          case 41: //min
            a= pstack.pop();
            b= pstack.pop();
            pstack.push(a<b? a : b);
            break;
          case 42: //max
            a= pstack.pop();
            b= pstack.pop();
            pstack.push(a<b? b : a);
            break;
          case 43: // sqrt
            pstack.push(sqrt(pstack.pop()));
            break;
          case 44: // ^
            a= pstack.pop();
            b= pstack.pop();
            pstack.push(pow(a,b));
            break;
          case 45: // mod
            a= pstack.pop();
            b= pstack.pop();
            pstack.push((int)b % (int)a);
            break;
          case 46: // vecscalmul
            {
              a = pstack.pop(); // n
              b = pstack.pop(); // alpha
              int start = int(pstack.length()-a);
              for (int i=0;i<a;i++)
                pstack[start+i] *= b;
              break;
            }
          case 47: // __getfieldsrange         %M:%N       pushes fields %N to %M inclusively on the stack
            {
              int M = *pptr++; 
              int N = *pptr++; 
              for (int i=M;i<=N;i++)
                pstack.push(pfieldvalues[i]);
              break;
            }
          default:
            PLERROR("BUG IN PreproInterpretor::run while running program: invalid opcode: %d", op);
          }
      }
    // copy to result vec.
    //for(int i=0;i<pstack.size();i++)
    //  cout<<pstack[i]<<" ";
    //cout<<endl;
    if (pstack.length() > result.length())
      PLERROR("Parsing VMatLanguage: left with %d too many items on the stack!",
              pstack.length()-result.length());
    if (pstack.length() < result.length())
      PLERROR("Parsing VMatLanguage: left with %d missing items on the stack!",
              result.length()-pstack.length());
    pstack >> result;

  };
  
  void  PreprocessingVMatrix::getRow(int i, Vec v) const
  {
    program.run(i,v);
  }

PLEARN_IMPLEMENT_OBJECT(PreprocessingVMatrix, "ONE LINE DESCR", "NO HELP");

%> // end of namespace PLearn
