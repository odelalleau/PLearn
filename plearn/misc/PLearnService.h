// -*- C++ -*-

// PLearnService.h
//
// Copyright (C) 2005 Pascal Vincent 
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
   * $Id: PLearnService.h,v 1.1 2005/01/07 18:18:14 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file PLearnService.h */


#ifndef PLearnService_INC
#define PLearnService_INC

#include <plearn/base/PP.h>
#include <set>

namespace PLearn {

  class RemotePLearnServer;

class PLearnService: public PPointable
{
public:
  friend class RemotePLearnServer;

  //! Returns single instance of PLearnService
  static PLearnService& instance();

  //! returns the number of available processing ressources
  int availableServers() const;

  //! Attempts to reserve a remote processing ressource. 
  //! If sucessful retrns a pointer to a new RemotePLearnServer
  //! If no server could be successfully reserved, returns 0.
  RemotePLearnServer* newServer();

private:
  PLearnService();

  std::set< RemotePLearnServer* > reserved_servers;

  //! Frees a previously reserved processing ressource.
  /*! This is called automatically by the RemotePLearnServer's destructor
    and is meant to allow you to do some bookkeeping and cleaning
    in the PLearnService object, when the server is freed.
  */
  void freeServer(RemotePLearnServer* remoteserv);

};

} // end of namespace PLearn

#endif
