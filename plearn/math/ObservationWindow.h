// -*- C++ -*-
// ObservationWindow.h
// 
// Copyright (C) 2006 Christian Dorion
// Copyright (C) 2006 ApStat Technologies Inc.
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

/*! \file ObservationWindow.h */
#ifndef ObservationWindow_INC
#define ObservationWindow_INC

// From C++ stdlib
#include <map>

// From PLearn
#include <plearn/base/Object.h>
#include <plearn/base/PP.h>
#include <plearn/base/tuple.h>
#include <plearn/math/TVec.h>

namespace PLearn {
using namespace std;

/**
 *  Used by StatsCollector to keep a finite-size window of observations
 *
 *  This allows StatsCollector to compute finite-horizon moving averages, for
 *  instance.
 */
class ObservationWindow: public Object
{
    typedef Object inherited;

public:    
    int m_window;

    int m_nobs;
    int m_cursor;
    Mat m_observations;
    Vec m_obs_weights;    
    bool unlimited_size;

    tuple<Vec, real> m_last_update_rvalue;
    
    ObservationWindow(int window=-1);// -2 means unlimited size

    //! Returns the current length (not m_window!).
    int  length () const;
    bool isEmpty() const { return length() == 0; }
    bool isFull () const { return length() == m_window; }
    
    void forget();
    tuple<Vec, real> update(const Vec& obs, real weight=1.0);

    //! Intelligent accessors
    const Vec   getObs   (int t)          const;
          real  getObs   (int t, int col) const;
          real  getWeight(int t)          const;
    
    const Vec lastObs() const;
    real lastWeight() const;

    //TBA: // Get the min/max value in a column
    //TBA: void columnMin(int col, real& min, real& agemin) const;
    //TBA: void columnMax(int col, real& max, real& agemax) const;

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(ObservationWindow);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Deep copying
    //ObservationWindow* deepCopy(CopiesMap& copies) const;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();


};

/*
inline PStream& operator<<(PStream& out, const ObservationWindow& win)
{
    out << "ObservationWindow(len=" << win.length() << ", window=" << win.m_window << ")";
    return out;
}

inline PStream& operator<<(PStream& out, const ObservationWindow* win)
{
    out << *win;
    return out;
}
*/

DECLARE_OBJECT_PTR(ObservationWindow);

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
