// -*- C++ -*-

// LocallyPrecomputedVMatrix.cc
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
   * $Id: LocallyPrecomputedVMatrix.cc,v 1.1 2004/11/18 14:31:45 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file LocallyPrecomputedVMatrix.cc */


#define PLEARN_IMPLEMENT_MY_OBJECT(CLASSTYPE, ONELINEDESCR, MULTILINEHELP)    \
	      string CLASSTYPE::_classname_()                              \
          { return #CLASSTYPE; }                                           \
	      string CLASSTYPE::classname() const                          \
          { return _classname_(); }                                        \
        OptionList& CLASSTYPE::_getOptionList_()                           \
          { static OptionList ol;                                          \
            if(ol.empty())                                                 \
              declareOptions(ol);                                          \
            return ol; }                                                   \
        OptionList& CLASSTYPE::getOptionList() const                       \
          { return _getOptionList_(); }                                    \
        Object* CLASSTYPE::_new_instance_for_typemap_()                    \
          { return new CLASSTYPE(); }                                      \
        bool CLASSTYPE::_isa_(Object* o)                                   \
          { return dynamic_cast<CLASSTYPE*>(o) != 0; }                     \
        CLASSTYPE* CLASSTYPE::deepCopy(CopiesMap& copies) const            \
          { CopiesMap::iterator it = copies.find(this);                    \
            if (it != copies.end())                                        \
              return static_cast<CLASSTYPE*>(it->second);                  \
            CLASSTYPE* deep_copy =                                         \
                 new CLASSTYPE(dynamic_cast<const CLASSTYPE&>(*this));     \
                 cout << "Created CLASSTYPE by copy" << endl;              \
            copies[this] = deep_copy;                                      \
            deep_copy->makeDeepCopyFromShallowCopy(copies);                \
            return deep_copy; }                                            \
        void CLASSTYPE::_static_initialize_()                              \
          { TypeFactory::register_type(                                    \
            #CLASSTYPE,                                                    \
	          inherited::_classname_(),                                \
            &CLASSTYPE::_new_instance_for_typemap_,                        \
            &CLASSTYPE::_getOptionList_,                                   \
            &CLASSTYPE::_isa_,                                             \
            ONELINEDESCR,                                                  \
            MULTILINEHELP  ); }                                            \
	      StaticInitializer CLASSTYPE::_static_initializer_(&CLASSTYPE::_static_initialize_)



#include "LocallyPrecomputedVMatrix.h"
#include <plearn/vmat/FileVMatrix.h>

namespace PLearn {
using namespace std;

///////////////////////////////
// LocallyPrecomputedVMatrix //
///////////////////////////////
LocallyPrecomputedVMatrix::LocallyPrecomputedVMatrix()
: local_dir("/Tmp"),
  remove_when_done(true)
{
  cout << "CREATION OF A LocallyPrecomputedVMatrix" << endl;
  precomp_type = "pmat";
}

PLEARN_IMPLEMENT_MY_OBJECT(LocallyPrecomputedVMatrix,
    "A VMat that precomputes its source in a local directory.",
    ""
);

////////////////////
// declareOptions //
////////////////////
void LocallyPrecomputedVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "local_dir", &LocallyPrecomputedVMatrix::local_dir, OptionBase::buildoption,
      "The local directory in which we want to save the precomputed data.");

  declareOption(ol, "remove_when_done", &LocallyPrecomputedVMatrix::remove_when_done, OptionBase::buildoption,
      "Whether we want or not to remove the precomputed data when this object is deleted.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  redeclareOption(ol, "metadatadir", &LocallyPrecomputedVMatrix::metadatadir, OptionBase::nosave,
      "The metadatadir will be defined by the 'local_dir' option.");

  redeclareOption(ol, "precomp_type", &LocallyPrecomputedVMatrix::precomp_type, OptionBase::nosave,
      "We always use 'pmat' here.");
}

///////////
// build //
///////////
void LocallyPrecomputedVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void LocallyPrecomputedVMatrix::build_()
{
  if (metadatadir == "") {
    metadatadir = newFilename(local_dir, "locally_precomputed_", true);
    inherited::build();
    precomp_source->setOption("remove_when_done", tostring(remove_when_done));
    precomp_source->setOption("track_ref", "1");
    precomp_source->build();
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LocallyPrecomputedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

////////////////////////////////
// ~LocallyPrecomputedVMatrix //
////////////////////////////////
LocallyPrecomputedVMatrix::~LocallyPrecomputedVMatrix()
{
  cout << "DELETION OF A LocallyPrecomputedVMatrix" << endl;
  if (remove_when_done && metadatadir != "") {
    string precomputed_file = precomp_source->getOption("filename");
    precomputed_file = precomputed_file.substr(1, precomputed_file.size() - 2);
    cout << "precomputed_file = " << precomputed_file << endl;
    // First we delete the precomputed source, so that it does not try to save
    // more stuff in the metadatadir after it has been deleted.
    precomp_source = 0;
    // Let's check whether more FileVMatrix are accessing the same precomputed file.
    if (FileVMatrix::countRefs(precomputed_file) == 0) {
      bool removed = force_rmdir(metadatadir);
      if (!removed)
        PLWARNING("In LocallyPrecomputedVMatrix::~LocallyPrecomputedVMatrix - The precomputed data could not be removed");
    }
  }
}


} // end of namespace PLearn

