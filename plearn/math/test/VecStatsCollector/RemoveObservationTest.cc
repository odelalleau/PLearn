// -*- C++ -*-

// RemoveObservationTest.cc
//
// Copyright (C) 2006 Christian Dorion
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

// Authors: Christian Dorion

/*! \file RemoveObservationTest.cc */


#include "RemoveObservationTest.h"
#include <plearn/math/VecStatsCollector.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RemoveObservationTest,
    "Test for the remove observation mechanism.",
    ""
);

//////////////////
// RemoveObservationTest //
//////////////////
RemoveObservationTest::RemoveObservationTest()
{}

///////////
// build //
///////////
void RemoveObservationTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RemoveObservationTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(m_windowed_vsc, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);
}

////////////////////
// declareOptions //
////////////////////
void RemoveObservationTest::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void RemoveObservationTest::build_()
{
}

//////////////////
// compareStats //
//////////////////
bool RemoveObservationTest::
compareStats(int t, const VecStatsCollector& batch, const VecStatsCollector& online, const string& stat)
{
    int len = batch.length();
    assert(len==online.length());
    
    Mat batch_stats(1, len, batch.getAllStats(stat));
    Mat online_stats(1, len, online.getAllStats(stat));

    if ( !batch_stats.isEqual(online_stats, 5e-6) )
    {
        cerr << "At time " << t << " " << stat << " differ: " << endl
             << "batch\n  " << batch_stats(0) << endl    
             << "online\n  " << online_stats(0) << endl << endl;
        return true;
    }
    return false;
}

/////////////
// perform //
/////////////
void RemoveObservationTest::perform()
{
    int N = 8;
    int T = 2500;
    
    Vec obs(N);
    VecStatsCollector vsc;    
    vsc.no_removal_warnings = true;
    vsc.build();

    m_windowed_vsc.m_window = 100;
    m_windowed_vsc.no_removal_warnings = true;
    m_windowed_vsc.build();

    for (int t=0; t<T; t++)
    {        
        int half_n = N/2;

        // Online window management
        for (int n=0; n<N; n++)
            obs[n] = (t+1) * pow(10.0, n-half_n);
        m_windowed_vsc.update(obs);

        // Batch window management
        vsc.forget();
        vsc.update(m_windowed_vsc.getObservations());

        bool stop = compareStats(t, vsc, m_windowed_vsc, "N");
        stop = stop || compareStats(t, vsc, m_windowed_vsc, "NMISSING");
        stop = stop || compareStats(t, vsc, m_windowed_vsc, "NNONMISSING");
        stop = stop || compareStats(t, vsc, m_windowed_vsc, "E");
        stop = stop || compareStats(t, vsc, m_windowed_vsc, "V");
        stop = stop || compareStats(t, vsc, m_windowed_vsc, "STDDEV");
        stop = stop || compareStats(t, vsc, m_windowed_vsc, "STDERROR");
        stop = stop || compareStats(t, vsc, m_windowed_vsc, "SKEW");
        stop = stop || compareStats(t, vsc, m_windowed_vsc, "KURT");

        if( stop )
            break;
    }
        
    
    // cout << "E: " << vsc.getAllStats("E") << endl;
    // cout << "V: " << vsc.getAllStats("V") << endl;           
    // cout << "STDDEV: " << vsc.getAllStats("STDDEV") << endl;      
    // cout << "STDERROR: " << vsc.getAllStats("STDERROR") << endl;    
    // cout << "SKEW: " << vsc.getAllStats("SKEW") << endl;        
    // cout << "KURT: " << vsc.getAllStats("KURT") << endl;        
    // cout << "MIN: " << vsc.getAllStats("MIN") << endl;         
    // cout << "MAX: " << vsc.getAllStats("MAX") << endl;
    // cout << "RANGE: " << vsc.getAllStats("RANGE") << endl;       
    // cout << "SUM: " << vsc.getAllStats("SUM") << endl;         
    // cout << "SUMSQ: " << vsc.getAllStats("SUMSQ") << endl;       
    // cout << "FIRST: " << vsc.getAllStats("FIRST") << endl;       
    // cout << "LAST: " << vsc.getAllStats("LAST") << endl;        
    // cout << "N: " << vsc.getAllStats("N") << endl;           
    // cout << "NMISSING: " << vsc.getAllStats("NMISSING") << endl;    
    // cout << "NNONMISSING: " << vsc.getAllStats("NNONMISSING") << endl;        
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
