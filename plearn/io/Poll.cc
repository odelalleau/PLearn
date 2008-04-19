// -*- C++ -*-

// Poll.cc
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

/*! \file Poll.cc */


#include "Poll.h"
#include <plearn/base/plerror.h>
#include <plearn/base/PrUtils.h>
#include <plearn/io/PrPStreamBuf.h>
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

void Poll::setStreamsToWatch(const vector<PStream>& streams) {
    m_streams_to_watch.clear();
    m_poll_descriptors.resize(streams.size());

    int i = 0;
    for (vector<PStream>::const_iterator it = streams.begin();
         it != streams.end(); ++it, ++i) 
    {
        PStreamBuf* st = *it;
        PrPStreamBuf* pr_st = dynamic_cast<PrPStreamBuf*>(st);
        if (!pr_st)
            PLERROR("Poll::setStreamsToWatch: only PrPStreamBuf streams supported!");

        m_streams_to_watch.push_back(*it);
        m_poll_descriptors[i].fd = pr_st->in;
        m_poll_descriptors[i].in_flags = PR_POLL_READ;
    }

}

int Poll::waitForEvents(uint32_t timeout, bool shuffle_events_) 
{
    if (m_poll_descriptors.size() == 0)
        PLERROR("Poll::waitforEvents: called with no streams to watch.");

    shuffle_events= shuffle_events_;
    if(shuffle_events)//shuffle index vec if necessary
    {
        shuffled_index= TVec<int>(0, m_poll_descriptors.size()-1, 1);
        shuffleElements(shuffled_index);
    }

    m_next_unexamined_event = 0;

    //first, check for non-empty buffers (ready to read)
    int nevents= 0;
    for(unsigned int i= 0; i < m_poll_descriptors.size(); ++i)
        if(!m_streams_to_watch[i]->inbufEmpty())
            ++nevents;

    if(nevents > 0)//if we already have some events, poll w/ no wait
        timeout= PR_INTERVAL_NO_WAIT;

    //poll underlying streams
    last_n_poll_events= PR_Poll(&m_poll_descriptors[0], PRIntn(m_poll_descriptors.size()), timeout);

    if(last_n_poll_events < 0)
        PLERROR((string("Poll::waitForEvents: poll error: ") + getPrErrorString()).c_str());

    nevents= 0;// now count _all_ events (non-empty buffers + stream polling)
    for(unsigned int i= 0; i < m_poll_descriptors.size(); ++i)
        if ((last_n_poll_events > 0 
             && m_poll_descriptors[i].out_flags & PR_POLL_READ)
            || !m_streams_to_watch[i]->inbufEmpty())
            ++nevents;

    return nevents;
}

PStream Poll::getNextPendingEvent() {
    while (m_next_unexamined_event < m_poll_descriptors.size()) 
    {
        int i = m_next_unexamined_event++;
        if(shuffle_events)
            i= shuffled_index[i];
        if ((last_n_poll_events > 0 
             && m_poll_descriptors[i].out_flags & PR_POLL_READ)
            || !m_streams_to_watch[i]->inbufEmpty())
            return m_streams_to_watch[i];
    }

    PLERROR("Poll::getNextPendingEvent: called with no more pending events!");
    // We never reach this because of the PLERROR. Used to silence
    // a gcc warning.
    return PStream();
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
