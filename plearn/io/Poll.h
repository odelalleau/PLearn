// -*- C++ -*-

// Poll.h
//
// Copyright (C) 2005 Christian Hudon 
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

// Authors: Christian Hudon

/*! \file Poll.h */


#ifndef Poll_INC
#define Poll_INC

#include <vector>
#include <nspr/prio.h>
#include <plearn/io/PStream.h>
#include <plearn/math/TVec.h>

namespace PLearn {
using namespace std;


/** A class for polled IO with PStreams. Currently supports only
 *  PrPStreamBufs.
 */
class Poll {
    
public:
    void setStreamsToWatch(const vector<PStream>& streams);

    int waitForEvents(int timeout = 0, bool shuffle_events_= false);

    PStream getNextPendingEvent();

protected:
    /// The PStream's to watch for IO.
    vector<PStream> m_streams_to_watch;

    /** The underlying structure used by NSPR's PR_Poll to notify us
        of pending IO. */
    vector<PRPollDesc> m_poll_descriptors;

    /** Counter used to iterate through the m_poll_descriptors
        in getNextPendingEvent */
    unsigned int m_next_unexamined_event;

    int last_n_poll_events; //nb. events for the last PR_Poll; indicates wether out_flags s/b used.
    bool shuffle_events;
    TVec<int> shuffled_index;
};

} // end of namespace PLearn

#endif


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
