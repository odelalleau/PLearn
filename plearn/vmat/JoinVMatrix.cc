// -*- C++ -*-

// PLearn ("A C++ Machine Learning Library")
// Copyright (C) 2002 Julien Keable and Pascal Vincent
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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "JoinVMatrix.h"
#include <plearn/base/stringutils.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(JoinVMatrix, "ONE LINE DESCR", "NO HELP");

JoinVMatrix::JoinVMatrix(VMat mas,VMat sla,TVec<int> mi,TVec<int> si)
    : inherited(mas.length(),mas.width()),master(mas),slave(sla),master_idx(mi),slave_idx(si)
{
    build();
}

void
JoinVMatrix::build()
{
    inherited::build();
    build_();
}

void
JoinVMatrix::build_()
{
    if (master && slave) {
        if(master_idx.size()!=slave_idx.size())
            PLERROR("JoinVMatrix : master and slave field correspondance don't have same dimensions ");

        for(int j=0;j<width();j++)
            declareField(j, master->fieldName(j), VMField::UnknownType);

        temp.resize(slave.width());
        tempkey.resize(master_idx.size());
  
        for(int i=0;i<slave.length();i++) {
            slave->getRow(i,temp);
            for(int j=0;j<slave_idx.size();j++)
                tempkey[j]=temp[slave_idx[j]];
            mp.insert(make_pair(tempkey,i));
        }
    }
}

void
JoinVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "master", &JoinVMatrix::master, OptionBase::buildoption, "");
    declareOption(ol, "slave", &JoinVMatrix::slave, OptionBase::buildoption, "");
    declareOption(ol, "master_idx", &JoinVMatrix::master_idx, OptionBase::buildoption, "");
    declareOption(ol, "slave_idx", &JoinVMatrix::slave_idx, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void JoinVMatrix::addStatField(const string & statis,const string & namefrom,const string & nameto)
{
    width_++;
    int from=slave->fieldIndex(namefrom),to=width()-1;
    if(from==-1)
        PLERROR("Unknown field in JOIN operation : %s",namefrom.c_str());
    declareField(to, nameto, VMField::UnknownType);
  
    if(statis=="COUNT")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::COUNT));
    else if(statis=="NMISSING")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::NMISSING));
    else if(statis=="NNONMISSING")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::NNONMISSING));
    else if(statis=="SUM")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::SUM));
    else if(statis=="SUMSQUARE")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::SUMSQUARE));
    else if(statis=="MEAN")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::MEAN));
    else if(statis=="VARIANCE")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::VARIANCE));
    else if(statis=="MIN")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::MIN));
    else if(statis=="MAX")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::MAX));
    else if(statis=="STDDEV")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::STDDEV));
    else if(statis=="STDERR")
        fld.push_back(JoinFieldStat(from,to,JoinFieldStat::STDERR));
    else PLERROR("Unknown statistic in JOIN operation : %s",statis.c_str());
}

void JoinVMatrix::getNewRow(int idx, const Vec& v) const
{
    real nonmiss;
    master->getRow(idx,v.subVec(0,master.width()));
  
    // build a key used to search in the slave matrix
    for(int j=0;j<master_idx.size();j++)
        tempkey[j]=v[master_idx[j]];
    Maptype::const_iterator it,low,upp; 
    pair<Maptype::const_iterator,Maptype::const_iterator> tit=mp.equal_range(tempkey);
    low=tit.first;
    upp=tit.second;
  
    Vec popo(v.subVec(master.width(),width()-master.width()));

    int sz=(int)fld.size();
    Vec count(sz,0.0),nmissing(sz,0.0),sum(sz,0.0),sumsquare(sz,0.0),min(sz,FLT_MAX),max(sz,-FLT_MAX);
    real val;  
    if(low!=mp.end())
    {
        for(it=low;it!=upp;++it)
        {  
            slave->getRow(it->second,temp);
            for(int i=0;i<sz;i++)
            {
                val=temp[fld[i].from];
                count[i]++;
                if(is_missing(val))nmissing[i]++;
                else
                {
                    sum[i]+=val;
                    sumsquare[i]+=val*val;
                    if(min[i]>val)min[i]=val;
                    if(max[i]<val)max[i]=val;
                }
            }
        }
    }
    for(int i=0;i<sz;i++)
    {
        nonmiss=count[i]-nmissing[i];
        switch(fld[i].stat)
        {
        case JoinFieldStat::COUNT:
            popo[i]=count[i];
            break;
        case JoinFieldStat::NMISSING:
            popo[i]=nmissing[i];
            break;
        case JoinFieldStat::NNONMISSING:
            popo[i]=nonmiss;
            break;
        case JoinFieldStat::SUM:
            popo[i]=sum[i];
            break;
        case JoinFieldStat::SUMSQUARE:
            popo[i]=sumsquare[i];
            break;
        case JoinFieldStat::MEAN:
            popo[i]=sum[i]/count[i];
            break;
        case JoinFieldStat::VARIANCE:
            popo[i]=(sumsquare[i] - sum[i]*sum[i]/nonmiss)/(nonmiss-1); 
            break;
        case JoinFieldStat::STDDEV:
            popo[i]=sqrt((sumsquare[i] - sum[i]*sum[i]/nonmiss)/(nonmiss-1));
            break;
        case JoinFieldStat::STDERR:
            popo[i]=sqrt((sumsquare[i] - sum[i]*sum[i]/nonmiss)/(nonmiss-1)/nonmiss);
            break;
        case JoinFieldStat::MIN:
            popo[i]=min[i];
            break;
        case JoinFieldStat::MAX:
            popo[i]=max[i];
            break;
        default:PLERROR("Unknown statistic in JoinVMatrix!");
        }
    }
}

string JoinVMatrix::getValString(int col, real val) const
{
    if(col<master.width())
        return master->getValString(col,val);
    else 
        return slave->getValString(col,val);
}

real JoinVMatrix::getStringVal(int col, const string & str) const
{
    if(col<master.width())
        return master->getStringVal(col,str);
    else 
        return slave->getStringVal(col,str);
}

const map<string,real>& JoinVMatrix::getStringToRealMapping(int col) const
{
    if(col<master.width())
        return master->getStringToRealMapping(col);
    else 
        return slave->getStringToRealMapping(col);

}

const map<real,string>& JoinVMatrix::getRealToStringMapping(int col) const
{
    if(col<master.width())
        return master->getRealToStringMapping(col);
    else 
        return slave->getRealToStringMapping(col);
}


string JoinVMatrix::getString(int row,int col) const
{
    if(col<master.width())
        return master->getString(row,col);
    else 
        return slave->getString(row,col);
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
