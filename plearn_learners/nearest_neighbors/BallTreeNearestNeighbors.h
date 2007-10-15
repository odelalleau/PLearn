// -*- C++ -*-

// BallTreeNearestNeighbors.h
//
// Copyright (C) 2004 Pascal Lamblin & Marius Muja
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
 * $Id$ 
 ******************************************************* */

// Authors: Pascal Lamblin & Marius Muja

/*! \file BallTreeNearestNeighbors.h */


#ifndef BallTreeNearestNeighbors_INC
#define BallTreeNearestNeighbors_INC


#include "BinaryBallTree.h"
#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/nearest_neighbors/GenericNearestNeighbors.h>
#include <plearn/vmat/SelectRowsVMatrix.h>
#include <plearn/ker/DistanceKernel.h>

#include <queue>

namespace PLearn {
using namespace std;

class BallTreeNearestNeighbors;
typedef PP< BallTreeNearestNeighbors > BallTreeNN;

class BallTreeNearestNeighbors: public GenericNearestNeighbors
{

private:

    typedef GenericNearestNeighbors inherited;

protected:

    // *********************
    // * protected options *
    // *********************

    BinBallTree ball_tree;
    int nb_train_points;
    int nb_points;

public:

    // ************************
    // * public build options *
    // ************************

    TVec<int> point_indices;
    int rmin;
    string train_method;
    TVec<Mat> anchor_set;
    TVec<int> pivot_indices;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    BallTreeNearestNeighbors();

    //! Constructor from a TrainSet and a BinBallTree.
    BallTreeNearestNeighbors( const VMat& tr_set, const BinBallTree& b_tree );

    // ********************
    // * PLearner methods *
    // ********************

private: 

    //! This does the actual building. 
    // (Please implement in .cc)
    void build_();
    void anchorTrain();

protected: 
  
    //! Declares this class' options.
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Static methods ****
    // ************************
    // Maybe should I put this somewhere else...

    // Returns true if the balls defined by (center1, radius1) and
    // (center2, radius2) have a common part

    static bool intersect( const Vec& center1, const real& radius1,
                           const Vec& center2, const real& radius2 );

    // Returns true if the first ball contains the second one
    static bool contain( const Vec& center1, const real& radius1,
                         const Vec& center2, const real& radius2 );

    // Returns the smallest ball containing two balls
    static void smallestContainer( const Vec& center1, const real& radius1,
                                   const Vec& center2, const real& radius2,
                                   Vec& t_center, real& t_radius);

    virtual void BallKNN( priority_queue< pair<real,int> >& q,
                          BinBallTree node, const Vec& t,
                          real& d_sofar, real d_minp, const int k ) const;

    virtual void FindBallKNN( priority_queue< pair<real,int> >& q,
                              const Vec& point, int k ) const;


    // ************************
    // **** Object methods ****
    // ************************

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(BallTreeNearestNeighbors);


    // **************************
    // **** PLearner methods ****
    // **************************

    //! (Re-)initializes the PLearner in its fresh state (that state may
    //! depend on the 'seed' option) And sets 'stage' back to 0 (this is
    //! the stage of a fresh learner!).
    virtual void forget();


    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training
    //! costs measured on-line in the process.
    virtual void train();

    void createAnchors( int nb_anchors );


    BinBallTree leafFromAnchor( int anchor_index );

    BinBallTree treeFromLeaves( const TVec<BinBallTree>& leaves );

    BinBallTree getBallTree();


    //! Computes the output and costs from the input (more effectively)
    virtual void computeOutputAndCosts( const Vec& input, const Vec& target,
                                        Vec& output, Vec& costs ) const;
    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output. 
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;


    //! Returns the names of the costs computed by computeCostsFromOutpus
    //! (and thus the test method).
    virtual TVec<string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method
    //computes and ! for which it updates the VecStatsCollector
    //train_stats.  (PLEASE IMPLEMENT IN .cc)
    virtual TVec<string> getTrainCostNames() const;

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(BallTreeNearestNeighbors);

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
