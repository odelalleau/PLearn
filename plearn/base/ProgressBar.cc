// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: ProgressBar.cc,v 1.7 2004/05/14 17:15:37 plearner Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

//#include <iomanip>
#include "ProgressBar.h"
#include "stringutils.h"

#if USING_MPI
#include "PLMPI.h"
#endif //USING_MPI

namespace PLearn {
using namespace std;


  PP<ProgressBarPlugin> ProgressBar::plugin; // = new TextProgressBarPlugin(cerr);

PP<ProgressBarPlugin> ProgressBar::getCurrentPlugin() 
{ 
  if (plugin == NULL)
    plugin = new TextProgressBarPlugin(cerr);
  return plugin; 
}

ProgressBar::ProgressBar(string _title, int the_maxpos)
  :title(_title),currentpos(0), maxpos(the_maxpos),closed(false)
{
  if (plugin == NULL)
    plugin = new TextProgressBarPlugin(cerr);
    
  plugin->addProgressBar(this);
}

ProgressBar::ProgressBar(ostream& _out, string _title, int the_maxpos)
  :title(_title),currentpos(0), maxpos(the_maxpos),closed(false)
{
  if (plugin == NULL)
    plugin = new TextProgressBarPlugin(cerr);

  plugin->addProgressBar(this);
}
ProgressBar::ProgressBar(PStream& _out, string _title, int the_maxpos)
  :title(_title),currentpos(0), maxpos(the_maxpos),closed(false)
{
  if (plugin == NULL)
    plugin = new TextProgressBarPlugin(cerr);

  plugin->addProgressBar(this);
}

ProgressBar::~ProgressBar() 
{
  close();
}

void ProgressBar::close()
{ 
  if(closed)
    return;
  closed=true;
  if(currentpos<maxpos)
    operator()(maxpos); 
  plugin->killProgressBar(this);
}              

TextProgressBarPlugin::TextProgressBarPlugin(ostream& _out)
  :out(&_out)
{
  out.outmode=PStream::raw_ascii;
}

TextProgressBarPlugin::TextProgressBarPlugin(PStream& _out)
  :out(_out)
{
}

void TextProgressBarPlugin::addProgressBar(ProgressBar * pb)
{
#if USING_MPI
  if(PLMPI::rank==0)
  {
#endif
    string fulltitle = string(" ") + pb->title + " (" + tostring(pb->maxpos) + ") ";
    out << "[" + center(fulltitle,100,'-') + "]\n[";
    out.flush();
#if USING_MPI
  }
#endif
}

void TextProgressBarPlugin::update(ProgressBar * pb,int newpos)
{
#if USING_MPI
    if(PLMPI::rank==0)
      {
#endif
        // this handles the case where we reuse the same progress bar
        if(newpos < pb->currentpos)
        {
          pb->currentpos=0;
          string fulltitle = string(" ") + pb->title + " (" + tostring(pb->maxpos) + ") ";
          out << "\n[" + center(fulltitle,100,'-') + "]\n[";
          out.flush();
        }

        if(!pb->maxpos || newpos>pb->maxpos)
          return;
        int ndots = newpos*100 / pb->maxpos - pb->currentpos*100/pb->maxpos;
        while(ndots--)
          out << '.';
        out.flush();
        pb->currentpos = newpos;
        if(pb->currentpos==pb->maxpos)
          {
            out << "]";
            out << endl;
          }
#if USING_MPI
      }
#endif
}


} // end of namespace PLearn




