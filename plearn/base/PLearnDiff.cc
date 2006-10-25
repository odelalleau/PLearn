// -*- C++ -*-

// PLearnDiff.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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
 * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file PLearnDiff.cc */


#include "PLearnDiff.h"
#include <plearn/math/TMat.h>

namespace PLearn {
using namespace std;

PLearnDiff::PLearnDiff():
    absolute_tolerance(ABSOLUTE_TOLERANCE),
    relative_tolerance(RELATIVE_TOLERANCE),
    save_diffs(true)
{
    build_();
    forget();
}

PLEARN_IMPLEMENT_OBJECT(PLearnDiff,
                        "Object that represents settings and results of a diff between two PLearn objects",
                        ""
    );

void PLearnDiff::declareOptions(OptionList& ol)
{
    declareOption(ol, "absolute_tolerance", &PLearnDiff::absolute_tolerance, OptionBase::buildoption,
            "The absolute tolerance used when comparing real numbers less than 1.");

    declareOption(ol, "relative_tolerance", &PLearnDiff::relative_tolerance, OptionBase::buildoption,
            "The relative tolerance used when comparing real numbers and one is more than 1.");

    declareOption(ol, "save_diffs", &PLearnDiff::save_diffs, OptionBase::buildoption,
            "If set to 1, each call to 'diff' will save any difference found.\n"
            "Otherwise, differences will not be saved.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PLearnDiff::build_()
{
    diffs.resize(diffs.length(), 3);
}

///////////
// build //
///////////
void PLearnDiff::build()
{
    inherited::build();
    build_();
}

///////////////////
// addDiffPrefix //
///////////////////
void PLearnDiff::addDiffPrefix(const string& prefix, int n)
{
    PLASSERT( n >= 0 && n <= diffs.length() );
    int l = diffs.length() - 1;
    for (int j = 0; j < n; j++)
        diffs(l - j, 0) = prefix + diffs(l - j, 0);
}

void addDiffPrefix(PLearnDiff* diffs, const string& prefix, int n)
{
    PLASSERT( diffs );
    diffs->addDiffPrefix(prefix, n);
}

//////////
// diff //
//////////
int PLearnDiff::diff(const string& refer, const string& other, const string& name)
{
    static TVec<string> diff_row;
    diff_row.resize(3);
    if (refer != other) {
        if (save_diffs) {
            diff_row[0] = name;
            diff_row[1] = refer;
            diff_row[2] = other;
            diffs.appendRow(diff_row);
        }
        return 1;
    } else
        return 0;
}

int diff(PLearnDiff* diffs, const string& refer, const string& other, const string& name)
{
    PLASSERT( diffs );
    return diffs->diff(refer, other, name);
}

////////////
// forget //
////////////
void PLearnDiff::forget()
{
    diffs.resize(0, diffs.width());
}

////////////////////////////
// get_absolute_tolerance //
////////////////////////////
real get_absolute_tolerance(PLearnDiff* diffs)
{
    PLASSERT( diffs );
    return diffs->absolute_tolerance;
}

////////////////////////////
// get_relative_tolerance //
////////////////////////////
real get_relative_tolerance(PLearnDiff* diffs)
{
    PLASSERT( diffs );
    return diffs->relative_tolerance;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PLearnDiff::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("PLearnDiff::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////
// printDiffs //
////////////////
void PLearnDiff::printDiffs(PStream& out, unsigned int indent,
                            unsigned int tab_step, unsigned int max_width)
{
    int n = nDiffs();
    for (int i = 0; i < n; i++) {
        const string& diff_name = diffs(i, 0);
        const string& diff_val1 = diffs(i, 1);
        const string& diff_val2 = diffs(i, 2);
        unsigned int n_blanks_after_name =
            tab_step - (unsigned int)(diff_name.size()) % tab_step;
        unsigned int n_blanks_after_val1 =
            tab_step - (unsigned int)(diff_val1.size()) % tab_step;
        if (indent + diff_name.size() + n_blanks_after_name + 3 
            + diff_val1.size() + n_blanks_after_val1 + 7 + diff_val2.size()
            < max_width) {
            out << string(indent, ' ')
                << diff_name << string(n_blanks_after_name, ' ')
                << ":  "
                << diff_val1 << string(n_blanks_after_val1, ' ')
                << "  -->  "
                << diff_val2 << endl;
        } else {
            out << string(indent, ' ')
                << diff_name << ":" << endl
                << diff_val1 << endl
                << "  -->  " << endl
                << diff_val2 << endl;
        }
    }
}


//////////////////
// setSaveDiffs //
//////////////////
void PLearnDiff::setSaveDiffs(bool save_diffs, bool* save_diffs_backup)
{
    if (save_diffs_backup)
        *save_diffs_backup = this->save_diffs;
    this->save_diffs = save_diffs;
}

void setSaveDiffs(PLearnDiff* diffs, bool save_diffs, bool* save_diffs_backup)
{
    diffs->setSaveDiffs(save_diffs, save_diffs_backup);
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
