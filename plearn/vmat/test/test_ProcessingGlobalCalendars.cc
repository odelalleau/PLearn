// -*- C++ -*-

// test_ProcessingGlobalCalendars.cc
//
// Copyright (C) 2005 Nicolas Chapados
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
 ******************************************************* */

// Authors: Christian Dorion

/*! \file test_ProcessingGlobalCalendars.cc */

#include <plearn/misc/Calendar.h>
#include <plearn/vmat/ProcessingVMatrix.h>
#include <plearn/io/openString.h>

using namespace PLearn;
using namespace std;

string calendar_name = "1";
string vpl_prg =
"@date                                     :date\n"
"1 @date date2julian nextincal julian2date :nextincal\n"
"1 @date date2julian previncal julian2date :previncal\n";

string cal_dates = "[20040101 20040624 20041225]";

string test_dates =
"9 1 [1031231 1040101 1040102 1040623 1040625 1041224 1041225 1041216 1050101]";

int main()
{
    PStream cal_stream = openString(cal_dates, PStream::plearn_ascii);
    PStream test_stream= openString(test_dates,PStream::plearn_ascii);

    Vec cal_timestamps;
    Mat test_dates_mat;
    cal_stream  >> cal_timestamps;
    test_stream >> test_dates_mat;

    // Install global calendar
    PCalendar cal = Calendar::makeCalendar(cal_timestamps);
    Calendar::setGlobalCalendar("1", cal);

    // Create source data
    VMat source_data(test_dates_mat);
    source_data->declareField(0,"date");

    // Create processing vmatrix
    PP<ProcessingVMatrix> ProcVM = new ProcessingVMatrix(source_data,
                                                         vpl_prg);
    ProcVM->saveAMAT("test_ProcessingGlobalCalendars_output.amat");
    return 0;
}


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
