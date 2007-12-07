// -*- C++ -*-

// DTWKernel.cc
//
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

// Authors: Xavier Saint-Mleux

/*! \file DTWKernel.cc */


#include "DTWKernel.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DTWKernel,
    "Kernel for Dynamic Time Warping",
    "Kernel for Dynamic Time Warping\n"
    "(see sect.4.7 of Rabiner, L. and Juang, B. 1993 'Fundamentals of Speech Recognition'. Prentice-Hall, Inc.)\n"
    "TODO: Add global path constraints and other goodies...\n"
    );

//////////////////
// DTWKernel //
//////////////////
DTWKernel::DTWKernel()
    :subvec_length(-1), local_paths(), distance_type("euclidean")
{
    /*
     * default local paths: 
     * (0,1) or (1,0) cost 1.
     * (1,1) costs 2.
     */
    local_paths.push_back(TVec<LocalStep>(1,make_pair(make_pair(0,1),1.)));
    local_paths.push_back(TVec<LocalStep>(1,make_pair(make_pair(1,0),1.)));
    local_paths.push_back(TVec<LocalStep>(1,make_pair(make_pair(1,1),2.)));
}

////////////////////
// declareOptions //
////////////////////
void DTWKernel::declareOptions(OptionList& ol)
{
    declareOption(ol, "subvec_length", &DTWKernel::subvec_length,
                  OptionBase::buildoption,
                  "length of each sub-vec within an example");

    declareOption(ol, "local_paths", &DTWKernel::local_paths,
                  OptionBase::buildoption,
                  "Specifies local path constraints.");

    declareOption(ol, "distance_type", &DTWKernel::distance_type,
                  OptionBase::buildoption,
                  "Name of the 'distance' function to use "
                  "when comparing features (sub-vecs).");

    inherited::declareOptions(ol);
}

void DTWKernel::declareMethods(RemoteMethodMap& rmm)
{
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(
        rmm, "dtw", &DTWKernel::remote_dtw,
        (BodyDoc("Calc. DTW on two feature vectors\n"),
         ArgDoc("x1","first vector"),
         ArgDoc("x2","second vector"),
         RetDoc ("dpoint, dpath, bptrs")));
}



///////////
// build //
///////////
void DTWKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void DTWKernel::build_()
{
    if(subvec_length <= 0)
        PLERROR("In DTWKernel::build_() : "
                "subvec_length must be specified before build "
                "(%d is not a valid value).", subvec_length);

    if(distance_type == "euclidean")
        dist_fn= powdistance;
    else
        PLERROR("In DTWKernel::build_() : "
                "only 'euclidean' distance_type is supported for now "
                "('%s' is not a valid value).", distance_type.c_str());
}

//////////////
//   dtw    //
//////////////
void DTWKernel::dtw(const Vec& x1, const Vec& x2) const
{
    PLASSERT(x1.length() % subvec_length == 0);
    PLASSERT(x2.length() % subvec_length == 0);

    int n1= x1.length() / subvec_length;// n1 features
    TVec<Vec> subvecs1(n1);
    for(int i= 0; i < n1; ++i)
        subvecs1[i]= x1.subVec(i*subvec_length, subvec_length);
    int n2= x2.length() / subvec_length;// n2 features
    TVec<Vec> subvecs2(n2);
    for(int j= 0; j < n2; ++j)
        subvecs2[j]= x2.subVec(j*subvec_length, subvec_length);

    //init: calc. point-to-point distances
    dpoint.resize(n1,n2);
    for(int i= 0; i < n1; ++i)
        for(int j= 0; j < n2; ++j)
            dpoint(i,j)= dist_fn(subvecs1[i], subvecs2[j]);

    //recurs: calc. path distances
    dpath.resize(n1,n2);
    bptrs.resize(n1,n2);
    dpath(0,0)= dpoint(0,0); //starting point
    
    int i,j;
    real mn; //min. found at each step
    int ai, aj; //'actual' coords when following a local path
    pair<int,int> scoords; //coords for a step
    real dist; //'distance' to a given point
    TVec<LocalPath>::iterator it;
    TVec<LocalPath>::iterator itbeg= local_paths.begin();
    TVec<LocalPath>::iterator itend= local_paths.end();
    TVec<LocalStep>::iterator jt;
    TVec<LocalStep>::iterator jtend;
    bool path_ok; //is this path valid?
    bool some_path_ok; //is there any valid path to those coords?

    for(i= 0; i < n1; ++i)
        for(j= 0; j < n2; ++j)
        {
            if(i==0 && j==0)
                continue; //starting point already calc.
            some_path_ok= false;
            mn= REAL_MAX;
            for(it= itbeg; it != itend; ++it)
            {// for all local paths
                path_ok= true;
                ai= i; aj= j;
                dist= 0.;
                jtend= it->end();
                for(jt= it->begin(); jt != jtend; ++jt)
                {// for each step from a local path
                    dist+= dpoint(ai,aj) * jt->second;
                    scoords= jt->first;
                    ai-= scoords.first;
                    aj-= scoords.second;
                    if(ai < 0 || aj < 0)//invalid path
                    {
                        path_ok= false;
                        break;
                    }
                }
                if(path_ok)
                {
                    dist+= dpath(ai,aj);//add dist. to beg. of path
                    if(dist < mn || !some_path_ok)
                    {
                        mn= dist;
                        bptrs(i,j)= make_pair(ai,aj);
                        some_path_ok= true;
                    }
                }
            }
            dpath(i,j)= mn;//will be REAL_MAX if no path
        }
}

////////////////
// remote dtw //
////////////////
tuple<Mat, Mat, TMat<pair<int, int> > > DTWKernel::remote_dtw(const Vec& x1, const Vec& x2) const
{
    dtw(x1,x2);
    return make_tuple(dpath, dpoint, bptrs);

}
//////////////
// evaluate //
//////////////
real DTWKernel::evaluate(const Vec& x1, const Vec& x2) const 
{
    dtw(x1, x2);
    return dpath.lastElement();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void DTWKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(dpoint, copies);
    deepCopyField(dpath, copies);
    deepCopyField(bptrs, copies);
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
