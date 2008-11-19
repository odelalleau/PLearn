// -*- C++ -*-

// Redirect.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
 * $Id: Redirect.cc 3994 2005-08-25 13:35:03Z chapados $ 
 ******************************************************* */

// Authors: Frédéric Bastien

/*! \file Redirect.cc */


#include "Redirect.h"
#include <plearn/io/openFile.h>
namespace PLearn {
using namespace std;

Redirect::Redirect():
    active(true)
    /* ### Initialize all fields to their default value */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(Redirect,
                        "Allow to redirect the perr and  pout to a file in a plearn script.",
                        "This runnable object will redirect perr or pout when executed.");

void Redirect::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "active", &Redirect::active, OptionBase::buildoption,
                  "Will do the redirect only if true. Else do nothing.");

    declareOption(ol, "what", &Redirect::what, OptionBase::buildoption,
                  "The string perr or pout. Indicated what will be redirected.");

    declareOption(ol, "filename", &Redirect::filename, OptionBase::buildoption,
                  "The file where the PStream will be redirected.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void Redirect::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

// ### Nothing to add here, simply calls build_
void Redirect::build()
{
    inherited::build();
    build_();
}

void Redirect::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(what, copies);
    deepCopyField(filename, copies);
}

/////////
// run //
/////////
void Redirect::run() {
    //do the redirection
    if(!active)
        return;
    if(what=="perr"){
        //the old is closed automatically
        perr=openFile(filename,PStream::raw_ascii,"w",false,true);
    }else if(what=="pout"){
        //the old is closed automatically
        pout=openFile(filename,PStream::raw_ascii,"w",false,true);
    }else
        PLERROR("In Redirect::run() - unknow stream %s to redirect",what.c_str());
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
