// -*- C++ -*-

// HyperSetOption.cc
//
// Copyright (C) 2003-2004 ApSTAT Technologies Inc.
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

// Author: Pascal Vincent

/* *******************************************************
 * $Id$
 ******************************************************* */

/*! \file HyperSetOption.cc */

#include "HyperSetOption.h"
#include "HyperLearner.h"

#define PL_LOG_MODULE_NAME "HyperSetOption"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    HyperSetOption,
    "HyperCommand to set an object option during HyperOptimization.",
    ""
);

////////////////////
// HyperSetOption //
////////////////////
HyperSetOption::HyperSetOption():
    call_build(false)
{}

////////////////////
// declareOptions //
////////////////////
void HyperSetOption::declareOptions(OptionList& ol)
{
    // TODO Deprecated 'option_name' and 'option_value'.

    declareOption(ol, "option_name", &HyperSetOption::option_name,
                  OptionBase::buildoption,
                  "Name of a single option to set.");

    declareOption(ol, "option_value", &HyperSetOption::option_value,
                  OptionBase::buildoption,
                  "Value 'option_name' should be set to.");

    declareOption(ol, "options", &HyperSetOption::options,
                  OptionBase::buildoption,
        "List of pairs \"optionname\":\"optionvalue\" to set.");

    declareOption(ol, "call_build", &HyperSetOption::call_build,
                  OptionBase::buildoption,
        "If set to 1, then the learner and its sub-objects will be re-built\n"
        "if (and only if) one of their options has been changed, or an\n"
        "option of one of their sub-objects has been changed.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void HyperSetOption::build_()
{
}

///////////
// build //
///////////
void HyperSetOption::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void HyperSetOption::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(options, copies);
}

//////////////
// optimize //
//////////////
Vec HyperSetOption::optimize()
{
    TVec<string> names, values;
    if (option_name != "") {
        names.push_back(option_name);
        values.push_back(option_value);
    }
    for (int i=0; i<options.size(); ++i) {
        names.push_back(options[i].first);
        values.push_back(options[i].second);
    }
    hlearner->setLearnerOptions(names, values);

    if (call_build && !names.isEmpty()) {
        // Call build on all objects affected by the changes.
        // We need to ensure this is done in the correct order.
        // Map each object that needs to be built to its parent.
        map<Object*, Object*> parent;
        // Map each object that needs to be built to the number of its children
        // that have to be built first.
        map<Object*, int> n_children_must_build;
        // Build the above maps.
        for (int i = 0; i < names.length(); i++) {
            string option = names[i];
            Object* object = hlearner->getLearner();
            Object* previous_object = NULL;
            while (true) {
                // Update maps if necessary.
                if (parent.find(object) == parent.end()) {
                    parent[object] = previous_object;
                    if (previous_object)
                        n_children_must_build[previous_object]++;
                }
                if (n_children_must_build.find(object) ==
                        n_children_must_build.end())
                    n_children_must_build[object] = 0;
                // Continue to next object.
                size_t dot_pos = option.find('.');
                if (dot_pos == string::npos)
                    break;
                size_t bracket_pos = option.find(']');
                bool indexed_object = (dot_pos == bracket_pos + 1);
                string sub_object_opt;
                int index = -1;
                if (indexed_object) {
                    size_t left_bracket_pos = option.find('[');
                    sub_object_opt = option.substr(0, left_bracket_pos);
                    index = toint(option.substr(left_bracket_pos + 1,
                                                bracket_pos));
                    PLCHECK( index >= 0 );
                } else {
                    sub_object_opt = option.substr(0, dot_pos);
                }
                OptionList& options = object->getOptionList();
                bool found = false;
                for (OptionList::iterator it = options.begin();
                    it != options.end(); ++it)
                {
                    if ((*it)->optionname() == sub_object_opt) {
                        previous_object = object;
                        if (indexed_object) {
                            object = (*it)->getIndexedObject(object, index);
                        } else {
                            object = (*it)->getAsObject(object);
                        }
                        found = true;
                        break;
                    }
                }
                if (!found)
                    PLERROR("In HyperSetOption::optimize - Could not find "
                            "option '%s' in an object of class '%s'",
                            sub_object_opt.c_str(),
                            object->classname().c_str());
                option = option.substr(dot_pos + 1);
            }
        }
        // Build objects in the correct order.
        bool finished = false;
        size_t count_builds = 0;
        while (!finished) {
            finished = true;
            map<Object*, int>::iterator it = n_children_must_build.begin();
            for (; it != n_children_must_build.end(); it++) {
                if (it->second == 0) {
                    // This object is ready to be built.
                    DBG_MODULE_LOG << "Building a " << it->first->classname()
                                   << endl;
                    it->first->build();
                    it->second = -1;
                    finished = false;
                    Object* parent_obj = parent[it->first];
                    if (parent_obj) {
                        PLASSERT( n_children_must_build.find(parent_obj) !=
                                  n_children_must_build.end() );
                        n_children_must_build[parent_obj]--;
                    }
                    count_builds++;
                }
            }
        }
        PLASSERT( count_builds == n_children_must_build.size() );
        DBG_MODULE_LOG << "All necessary builds performed" << endl;
    }

    return Vec();
}

////////////////////
// getResultNames //
////////////////////
TVec<string> HyperSetOption::getResultNames() const
{
    return TVec<string>();
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
