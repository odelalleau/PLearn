// -*- C++ -*-

// Poll.cc
//
// Copyright (C) 2005 Christian Hudon 
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
   * $Id: Poll.cc,v 1.1 2005/01/28 22:36:29 chrish42 Exp $ 
   ******************************************************* */

// Authors: Christian Hudon

/*! \file Poll.cc */


#include "Poll.h"
#include <plearn/base/plerror.h>
#include <plearn/io/PrPStreamBuf.h>

namespace PLearn {
using namespace std;

  void Poll::setStreamsToWatch(const vector<PStream>& streams) {
    m_streams_to_watch.clear();
    m_poll_descriptors.resize(streams.size());

    int i = 0;
    for (vector<PStream>::const_iterator it = streams.begin();
         it != streams.end(); ++it, ++i) {
      PStreamBuf* st = *it;
      PrPStreamBuf* pr_st = dynamic_cast<PrPStreamBuf*>(st);
      if (!pr_st)
        PLERROR("Poll::setStreamsToWatch: only PrPStreamBuf streams supported!");

      m_streams_to_watch.push_back(*it);
      m_poll_descriptors[i].fd = pr_st->out;
      m_poll_descriptors[i].in_flags = PR_POLL_READ;
    }
  }

  int Poll::waitForEvents(int timeout) {
    if (m_poll_descriptors.size() == 0)
      PLERROR("Poll::waitforEvents: called with no streams to watch.");
    
    m_next_unexamined_event = 0;
    return PR_Poll(&m_poll_descriptors[0], m_poll_descriptors.size(),
                   timeout);
  }

  PStream Poll::getNextPendingEvent() {
    while (m_next_unexamined_event < m_poll_descriptors.size()) {
      const int i = m_next_unexamined_event++;
      if (m_poll_descriptors[i].out_flags & PR_POLL_READ) {
        return m_streams_to_watch[i];
      }
    }

    PLERROR("Poll::getNextPendingEvent: called with no more pending events!");
    // We never reach this because of the PLERROR. Used to silence
    // a gcc warning.
    return PStream();
  }


} // end of namespace PLearn
