// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */

// This file contains useful functions for string manipulation
// that are used in the PLearn Library


/*! \file ProgressBar.h */

#ifndef ProgressBar_INC
#define ProgressBar_INC

#include <string>
//#include <iostream>
#include "PP.h"
#include <plearn/io/PStream.h>


namespace PLearn {
using namespace std;

class ProgressBar;

//! Base class for pb plugins
class ProgressBarPlugin : public PPointable
{
public:
    ProgressBarPlugin() {}
    virtual ~ProgressBarPlugin() {}
    virtual void addProgressBar(ProgressBar * pb){};
    virtual void killProgressBar(ProgressBar * pb){};
    virtual void update(ProgressBar * pb, uint32_t newpos){};
};


//! Simple plugin for displaying text progress bar
class TextProgressBarPlugin : public ProgressBarPlugin
{
protected:
    PStream out;
public:
    virtual void addProgressBar(ProgressBar * pb);
    virtual void update(ProgressBar * pb, uint32_t newpos);

    TextProgressBarPlugin(ostream& _out);
    TextProgressBarPlugin(PStream& _out);

    //! Displayed width of the text progress bar.  Default=100
    static int width;
};

//! Similar to TextProgressBarPlugin with a different output format 
//! so that remote servers can update progress bars on a client.
class RemoteProgressBarPlugin : public TextProgressBarPlugin
{
public:
    virtual void addProgressBar(ProgressBar* pb);
    virtual void update(ProgressBar* pb, uint32_t newpos);

    RemoteProgressBarPlugin(ostream& _out, unsigned int nticks_= 20);
    RemoteProgressBarPlugin(PStream& _out, unsigned int nticks_= 20);

    virtual void killProgressBar(ProgressBar* pb);

protected:
    static map<ProgressBar*, unsigned int> pb_ids;
    static unsigned int next_pb_id;
    static unsigned int getPBarID(ProgressBar* pb);
    void printTitle(ProgressBar* pb);
    unsigned int nticks;
};

//! Similar to TextProgressBarPlugin with a different output format 
//! so that updates appear on different lines of output.
//! (for logging or when multiple progress bars are used simultaneously)
class LineOutputProgressBarPlugin : public TextProgressBarPlugin
{
public:
    virtual void addProgressBar(ProgressBar* pb);
    virtual void update(ProgressBar* pb, uint32_t newpos);

    LineOutputProgressBarPlugin(ostream& _out, unsigned int nticks_= 100);
    LineOutputProgressBarPlugin(PStream& _out, unsigned int nticks_= 100);

    virtual void killProgressBar(ProgressBar* pb);

protected:
    static string pbInfo(ProgressBar* pb);
    unsigned int nticks;
};

//! Simpler plugin that doesn't display a progress bar at all.  Useful to
//! disable progress bars for operations that are known to be short.
//! Use it as follows:
//!   PP<ProgressBarPlugin> OldPlugin = ProgressBar::getCurrentPlugin();
//!   ProgressBar::setPlugin(new NullProgressBarPlugin);
//!   ... short operations that might otherwise have plugins here ...
//!   ProgressBar::setPlugin(OldPlugin);
struct NullProgressBarPlugin : public ProgressBarPlugin
{ /* all inherited methods are fine... :-) */ };


//! This class will help you display progress of a calculation
//! 
//! Each progressBar you create is connected to the same ProgressBarPlugin object.
//! By default, a  TextProgressBarPlugin that dumps the text in stderr is created and used.
//! 
//! FAQ: 
//! Q #1 : How do I reuse the same progress bar?
//! A #1 : simply call progress_bar(i) again with 'i' from 0..maxpos (The text progress bar plugin will display a new progress bar)
class ProgressBar : public PPointable
{
public:
    string title;
    uint32_t currentpos; // current position
    uint32_t maxpos;

    // creates a new progressbar with the given title and maxpos
    // *** Note, for now, ignore the stream (someday, remove this argument for 
    // every progressBar creation in PLearn)
    ProgressBar(string _title, uint32_t the_maxpos);
    ProgressBar(ostream& _out,string _title, uint32_t the_maxpos);
    ProgressBar(PStream& _out,string _title, uint32_t the_maxpos);

    // moves the progressbar up to position newpos
    void operator()(uint32_t newpos){plugin->update(this,newpos);}

    void update(uint32_t newpos){plugin->update(this,newpos);}
    void updateone(){plugin->update(this,currentpos+1);}
    // this function assumes plugin is always a valid object (it is created statically in the .cc)
    static void setPlugin(PP<ProgressBarPlugin> plugin_) { plugin = plugin_; }
    static PP<ProgressBarPlugin> getCurrentPlugin();

    // Completes and removes the progressBar 
    void close();

    // calls close() if not already done
    ~ProgressBar();
private:
    bool closed;
    static PP<ProgressBarPlugin> plugin;
};


void setProgressBarPlugin(string pb_type);


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
