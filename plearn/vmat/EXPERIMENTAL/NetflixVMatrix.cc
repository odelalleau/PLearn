// -*- C++ -*-

// NetflixVMatrix.cc
//
// Copyright (C) 2006 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file NetflixVMatrix.cc */


#include "NetflixVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NetflixVMatrix,
    "Used for holding the netflix data. Minimal implementation.",
    "Loads from a text file into a TVec< TVec< int > >.\n"
    "A hack for netflix."
    );

//////////////////
// NetflixVMatrix //
//////////////////
NetflixVMatrix::NetflixVMatrix()
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

////////////////////
// declareOptions //
////////////////////
void NetflixVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    declareOption(ol, "sourceFileName", &NetflixVMatrix::sourceFileName,
                   OptionBase::buildoption,
                   "Name of the source file - a netflix user profiles file.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void NetflixVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void NetflixVMatrix::build_()
{

    cout << "NetflixVMatrix::build_()" << endl;

    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    // Open source file
    ifstream input( sourceFileName.c_str() );

    if( ! input.is_open() ) {
        PLERROR("Could not open source file %s.", sourceFileName.c_str());
    }

    string str_line;
    int userIndex = 0;
    stringstream ss_tokens;
    int dummy;
    list< int > l_userProfile;

    // Determine number of users and resize data
    while( getline(input, str_line) )  {
        userIndex++;
    }

    cout << userIndex << " users" << endl;

    length_ = userIndex;

    data.resize(userIndex);
    input.clear();
    input.seekg(0, ios::beg);
    userIndex = 0;

    // Actual loading
    while( getline(input, str_line) )  {
        ss_tokens.str(str_line);
        ss_tokens.clear();

        while(ss_tokens >> dummy) {
            l_userProfile.push_back(dummy);
        }

        data[userIndex].resize( l_userProfile.size() );

        list< int >::iterator itr_userProfile = l_userProfile.begin();
        for(int i=0; itr_userProfile != l_userProfile.end(); itr_userProfile++, i++) {
            data[userIndex][i] = (*itr_userProfile);
        }
        l_userProfile.clear();
        userIndex++;
    }



    cout << "NetflixVMatrix::build_() - DONE!" << endl;

}

/////////
// get //
/////////
real NetflixVMatrix::get(int i, int j) const
{
    // get element at i-th row, j-th column
    PLERROR("Not implemented!");
    return 0.0;
}

////////////////
// getExample //
////////////////
void NetflixVMatrix::getExample(int i, Vec& input, Vec& target, real& weight)
{
    input.resize( data[i].length() );
    input << data[i];

}

/*
///////////////
// getSubRow //
///////////////
void NetflixVMatrix::getSubRow(int i, int j, Vec v) const
{
    // get part or all of the i-th, starting at the j-th column,
    // with v.length() elements; these elements are put in v.
    PLERROR("Not implemented!");
}

////////////
// getRow //
////////////
void NetflixVMatrix::getRow(int i, Vec v) const
{
    PLERROR("Not implemented!");
}
*/
/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NetflixVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    deepCopyField(sourceFileName, copies);
    deepCopyField(data, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("NetflixVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
