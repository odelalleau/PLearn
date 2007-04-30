// -*- C++ -*-

// CorrelationProfiler.cc
//
// Copyright (C) 2007 Pierre-Antoine Manzagol
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

/*! \file CorrelationProfiler.cc */


#include "CorrelationProfiler.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    CorrelationProfiler,
    "Object used to profile the correlation between the elements of a vector",
    "\n"
    );

CorrelationProfiler::CorrelationProfiler()      :   its_dim(0), 
                                                    its_name("noname")
    /* ### Initialize all fields to their default value */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}


CorrelationProfiler::CorrelationProfiler(int dim, string name)    :  its_dim(dim),
                                                its_name(name)
{
}


void CorrelationProfiler::reset()   
{
    A.clear();
    sum_v.clear();
    n = 0;
}

// Accumulates statistics for computing the correlation
void CorrelationProfiler::operator()(Vec& v)
{
    PLASSERT( A.length() == v.length() );
    externalProductAcc(A, v, v);
    sum_v += v;
    n++;
}


void CorrelationProfiler::printAndReset()
{

    cout << its_name << " - correlation based on " << n << " samples." << endl;

    // *** Get mean
    sum_v /= n;

    // *** Divide by n-1 to get non-centered covariance
    A /= (real)(n-1);

    // *** Get the centered covariance
    externalProductScaleAcc(A, sum_v, sum_v, -1.0);

    // *** Get correlation by dividing by the product of the standard deviations
    Vec diagA = diag( A );
    // TODO Not very efficient. Isn't there a lapack function for this. 
    for(int i=0; i<A.length(); i++)   {
        for(int j=0; j<i; j++)   {
            A(i,j) /= sqrt( diagA[i]*diagA[j] );
            A(j,i) = A(i,j);
        }
        // the diagonal
        A(i,i) = 1.0;
    }

    // *** Open file, print correlation and close.
    // TODO check opening
    string file_name;
    file_name = its_name + ".txt";
    ofstream fd;
    fd.open( file_name.c_str() );
    A.print(fd);
    fd.close();

    // *** Reset
    reset();
}


// ### Nothing to add here, simply calls build_
void CorrelationProfiler::build()
{
    inherited::build();
    build_();
}

void CorrelationProfiler::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    deepCopyField(A, copies);
    deepCopyField(sum_v, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("CorrelationProfiler::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void CorrelationProfiler::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "dimension", &CorrelationProfiler::its_dim,
                   OptionBase::buildoption,
                   "Dimension of the vector whose correlantion we want (integer).");
    declareOption(ol, "name", &CorrelationProfiler::its_name,
                   OptionBase::buildoption,
                   "Name of the vector whose correlantion we want (string)."
                    "Used in determining the output file's name in"
                    "printAndReset().");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void CorrelationProfiler::build_()
{
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
    PLASSERT( its_dim > 0 );
    A.resize(its_dim, its_dim);
    sum_v.resize(its_dim);
    reset();
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
