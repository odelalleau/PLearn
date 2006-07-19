// -*- C++ -*-

// BallTreeNearestNeighbors.cc
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

/*! \file BallTreeNearestNeighbors.cc */

#include "BallTreeNearestNeighbors.h"
#include <plearn/base/lexical_cast.h>

namespace PLearn {
using namespace std;

BallTreeNearestNeighbors::BallTreeNearestNeighbors() 
    : rmin( 1 ),
      train_method( "anchor" )
{
    num_neighbors = 1;
    expdir = "";
    stage = 0;
    nstages = -1;
    report_progress = 0;
}

BallTreeNearestNeighbors::BallTreeNearestNeighbors( const VMat& tr_set, const BinBallTree& b_tree )
    : rmin( 1 ),
      train_method( "anchor" )
{
    num_neighbors = 1;
    expdir = "";
    stage = 1;
    nstages = 1;
    report_progress = 0;

    setTrainingSet( tr_set );
    ball_tree = b_tree;
}

PLEARN_IMPLEMENT_OBJECT( BallTreeNearestNeighbors, 
                         "Organizes hierarchically a set of points to perform efficient  KNN search", 
                         "This learner builds a Ball Tree, a hierarchized structure\n"
                         "allowing to perform efficient KNN search.\n"
                         "Output is formatted as in GenericNearestNeighbors.\n"
                         "The square distance to this point can be computed as the error.\n" );

void BallTreeNearestNeighbors::declareOptions( OptionList& ol )
{
    // build options
    declareOption( ol, "point_indices", &BallTreeNearestNeighbors::point_indices, 
                   OptionBase::buildoption,
                   "Indices of the points we will consider" );

    declareOption( ol, "rmin", &BallTreeNearestNeighbors::rmin, OptionBase::buildoption,
                   "Max number of points in a leaf node of the tree" );

    declareOption( ol, "train_method", &BallTreeNearestNeighbors::train_method, 
                   OptionBase::buildoption,
                   "Method used to build the tree. Just one is supported:\n"
                   "  \"anchor\" (middle-out building based on Anchor\'s hierarchy\n"
        );

    declareOption( ol, "anchor_set", &BallTreeNearestNeighbors::anchor_set, 
                   OptionBase::learntoption, 
                   "Set of anchors, hierarchizing the set of points" );

    declareOption( ol, "pivot_indices", &BallTreeNearestNeighbors::pivot_indices, 
                   OptionBase::learntoption, "Indices of the anchors' centers" );

    // saved options
    declareOption( ol, "train_set", &BallTreeNearestNeighbors::train_set, 
                   OptionBase::buildoption,
                   "Indexed set of points we will be working with" );

    declareOption( ol, "nb_train_points", &BallTreeNearestNeighbors::nb_train_points, 
                   OptionBase::learntoption, "Number of points in train_set" );

    declareOption( ol, "nb_points", &BallTreeNearestNeighbors::nb_points, 
                   OptionBase::learntoption, "Number of points in point_indices" );

    declareOption( ol, "ball_tree", &BallTreeNearestNeighbors::ball_tree, 
                   OptionBase::learntoption, "Built ball-tree" );


    // Now call the parent class' declareOptions
    inherited::declareOptions( ol );
}

void BallTreeNearestNeighbors::build_()
{
    if (train_set) {
        // initialize nb_train_points
        nb_train_points = train_set.length();
        
        // if point_indices isn't specified, we take all the points in train_set
        if( !point_indices )
            point_indices = TVec<int>( 0, nb_train_points-1, 1 );

        // initialize nb_points
        nb_points = point_indices.size();
    }
}


void BallTreeNearestNeighbors::build()
{
    inherited::build();
    build_();
}


void BallTreeNearestNeighbors::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField( ball_tree, copies );
    deepCopyField( point_indices, copies );
    deepCopyField( anchor_set, copies );
    deepCopyField( pivot_indices, copies );
}


void BallTreeNearestNeighbors::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)

    anchor_set.resize( 0 );
    pivot_indices.resize( 0 );
    ball_tree = new BinaryBallTree;
    stage = 0;
    build();
}




void BallTreeNearestNeighbors::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.

    if( train_method == "anchor" )
    {
        anchorTrain();
    }
    else
        PLERROR( "train_method \"%s\" not implemented", train_method.c_str() );
}


void BallTreeNearestNeighbors::anchorTrain()
{
    /*  nstages and stage conventions, for "anchor" train method:
     *
     *  nstages == -1
     *    We will construct ball_tree recursively,
     *    until, for all leaf, nb_points <= rmin,
     *    no matter how many iterations it will take.
     *
     *  nstages == 0
     *    We want the PLearner il its fresh, blank state.
     *
     *  nstages == 1
     *    We want ball_tree to be a unique leaf node,
     *    containing all the point indices, with no children.
     *
     *  nstages > 1
     *    We want to build ball_tree recursively,
     *    but limiting the levels of recursion.
     *    This means we will decrement this number at each recursive call,
     *    the recursion will stop when nstages == 1 or nb_points <= rmin.
     *
     *  stage == 0
     *    The learner is it its fresh, blank state.
     *
     *  stage == 1
     *    The learner has one anchor, that's all.
     *
     *  Other values of stage might be used one day or anoter...
     */

    if( stage == 0 && nstages !=0 )
    {
        // That means we weren't provided with any anchor nor node parameter,
        // or that they were just bullsh!t

        // So, we build a single anchor
        pivot_indices.resize( 1 );
        pivot_indices[ 0 ] = 0;
        Vec pivot = train_set.getSubRow( 0, inputsize() );

        distance_kernel->setDataForKernelMatrix( train_set );
        distance_kernel->build();
        Vec distances_from_pivot( nb_train_points );
        distance_kernel->evaluate_all_i_x( pivot, distances_from_pivot );

        anchor_set.resize( 1 );
        Mat* p_anchor = &anchor_set[ 0 ];
        p_anchor->resize( nb_points, 2 );
        p_anchor->column( 0 ) << Vec( 0, nb_points-1, 1 );
        p_anchor->column( 1 ) << distances_from_pivot;
        sortRows( *p_anchor, TVec<int>( 1, 1 ), false );

        // then, we build the corresponding tree
        ball_tree = leafFromAnchor( 0 );

        ++stage;
    }

    if( nstages == 0 )
    {
        // we want a fresh, blank learner
        forget();
    }
    else if( nstages == 1 )
    {
        // We have an anchor, and we want a leaf node
        ball_tree = leafFromAnchor( 0 );
    }
    else
    {
        // nstages to be used on children learners
        int new_nstages = nstages<0 ? -1 : nstages-1;

        // First create sqrt( R )-1 anchors, from the initial anchor_set
        int nb_anchors = (int) sqrt( (float) nb_points ) + 1 ;
        nb_anchors = min( nb_anchors, nb_points );

        createAnchors( nb_anchors-1 ); // because we already have one

        // Convert them into leaf nodes
        TVec< BinBallTree > leaf_set = TVec<BinBallTree>( nb_anchors );
        for ( int i=0 ; i<nb_anchors ; i++ )
        {
            leaf_set[ i ] = leafFromAnchor( i );
        }

        // Then, group them to form the ball_tree
        // keep an index of the leaves
        ball_tree = treeFromLeaves( leaf_set );

        // Now, recurse...
        for( int i=0 ; i<leaf_set.size() ; i++ )
        {
            int rec_nb_points = anchor_set[ i ].length();

            // if the leaf is too small, don't do anything
            if( rec_nb_points > rmin )
            {
                // child learner
                PP<BallTreeNearestNeighbors> p_rec_learner = new BallTreeNearestNeighbors();

                // initializes child's nstages (see explanation above)
                stringstream out;
                out << new_nstages;
                p_rec_learner->setOption( "nstages" , out.str() );

                // keep the same training set: it give us all the point coordinates !
                // but we don't want to call forget() after that
                p_rec_learner->setTrainingSet( train_set, false );

                // however, we only work on the points contained by current leaf
                p_rec_learner->anchor_set.resize( 1 );
                p_rec_learner->anchor_set[ 0 ].resize( rec_nb_points, 2 );
                p_rec_learner->anchor_set[ 0 ] << anchor_set[ i ];

                p_rec_learner->pivot_indices.resize( 1 );
                p_rec_learner->pivot_indices[ 0 ] = pivot_indices[ i ];

                p_rec_learner->point_indices.resize( rec_nb_points );
                p_rec_learner->point_indices << 
                    p_rec_learner->anchor_set[ 0 ].column( 0 );

                p_rec_learner->stage = 1; 
                // faudra peut-etre faire ça plus subtilement

                p_rec_learner->rmin = rmin;
                p_rec_learner->train_method = train_method;
                p_rec_learner->build();
                p_rec_learner->train();

                // once the child learner is trained, we can get the sub-tree,
                // and link it correctly
                BinBallTree subtree = p_rec_learner->getBallTree();
                leaf_set[ i ]->pivot = subtree->pivot;
                leaf_set[ i ]->radius = subtree->radius;
                leaf_set[ i ]->point_set.resize( subtree->point_set.size() );
                leaf_set[ i ]->point_set << subtree->point_set;
                leaf_set[ i ]->setFirstChild( subtree->getFirstChild() );
                leaf_set[ i ]->setSecondChild( subtree->getSecondChild() );

            }
        }
    }
}


void BallTreeNearestNeighbors::createAnchors( int nb_anchors )
{
    // This method creates nb_anchors new anchors, and adds them to anchor_set

    // Make room
    int anchor_set_size = anchor_set.size();
    anchor_set.resize( anchor_set_size, nb_anchors );

    for( int i=0 ; i<nb_anchors ; i++ )
    {
        Mat new_anchor = Mat( 1, 2 );
        int new_pivot_index;

        // Search for the largest ball.
        // pivot of the new anchor will be the point of this ball
        // that is the furthest from the pivot.
        int largest_index = 0;
        real largest_radius = 0;
        for( int j=0 ; j<anchor_set_size ; j++ )
        {
            // points are sorted in decreasing order of distance, 
            // so anchor_set[ j ]( 0, 1 ) is the furthest point from 
            // pivot_indices[ j ]
            real current_radius = anchor_set[ j ]( 0, 1 );
            if( current_radius > largest_radius )
            {
                largest_radius = current_radius;
                largest_index = j;
            }
        }

        Mat* p_largest_anchor = &anchor_set[ largest_index ];
        new_pivot_index = (int) (*p_largest_anchor)( 0, 0 );

        // assign the point to its new anchor
        new_anchor( 0, 0 ) = new_pivot_index;
        new_anchor( 0, 1 ) = 0;
        Vec new_pivot = train_set.getSubRow( new_pivot_index, inputsize() );

        int largest_anchor_length = p_largest_anchor->length();

        // Verify that largest_anchor owns at least 2 points
        if( largest_anchor_length <= 1 )
        {
            PLERROR("In BallTreeNearestNeighbors::createAnchors, more anchors asked than points");
        }

        // delete this point from its original anchor
        *p_largest_anchor = p_largest_anchor->
            subMatRows( 1, largest_anchor_length-1 );

        // now, try to steal points from all the existing anchors
        for( int j=0 ; j<anchor_set_size ; j++ )
        {
            Mat* p_anchor = &anchor_set[ j ];
            int nb_points = p_anchor->length();
            int pivot_index = pivot_indices[ j ];
            Vec pivot = train_set.getSubRow( pivot_index, inputsize() );
            real pivot_pow_dist = powdistance( new_pivot, pivot, 2 );

            // loop on the anchor's points
            for( int k=0 ; k<nb_points ; k++ )
            {
                int point_index = (int) (*p_anchor)( k, 0 );
                real point_pow_dist = (*p_anchor)( k, 1 );

                // if this inequality is verified,
                // then we're sure that all the points closer to the pivot 
                // belong to the pivot, and we don't need to check
                if( 4*point_pow_dist < pivot_pow_dist )
                {
                    break;
                }

                Vec point = train_set.getSubRow( point_index, inputsize() );
                real new_pow_dist = powdistance( new_pivot, point, 2 );

                // if the point is closer to the new pivot, then steal it
                if( new_pow_dist < point_pow_dist )
                {
                    Vec new_row( 2 );
                    new_row[ 0 ] = point_index;
                    new_row[ 1 ] = new_pow_dist;
                    new_anchor.appendRow( new_row );

                    *p_anchor = removeRow( *p_anchor, k );
                    // bleaah, this is ugly !
                    --k;
                    --nb_points;
                }
            }
        }

        // sort the points by decreasing distance
        sortRows( new_anchor, TVec<int>( 1, 1 ), false );

        // append the new anchor to the anchor_set (and same for pivot)
        anchor_set.append( new_anchor );
        pivot_indices.append( new_pivot_index );
        ++anchor_set_size;
    }
}

BinBallTree BallTreeNearestNeighbors::leafFromAnchor( int anchor_index )
{
    BinBallTree leaf = new BinaryBallTree();

    int pivot_index = pivot_indices[ anchor_index ];
    leaf->pivot = train_set.getSubRow( pivot_index, inputsize() );

    leaf->radius = anchor_set[ anchor_index ]( 0, 1 );

    int nb_leaf_points = anchor_set[ anchor_index ].length();
    leaf->point_set.resize( nb_leaf_points );
    leaf->point_set << anchor_set[ anchor_index ].column( 0 );

    return leaf;
}


BinBallTree BallTreeNearestNeighbors::treeFromLeaves( const TVec<BinBallTree>& leaves )
{
    int nb_nodes = leaves.size();
    TVec<BinBallTree> nodes = TVec<BinBallTree>( nb_nodes );
    nodes << leaves;

    // if there is no leaf
    if( nb_nodes < 1 )
    {
        PLERROR( "In BallTreeNearestNeighbors::treeFromLeaves(): no leaf existing" );
    }

    while( nb_nodes > 1 )
    {
        int min_i = 0;
        int min_j = 0;
        Vec min_center;
        real min_radius = -1;

        // we get the most "compatible" pair of nodes :
        // the ball containing them both is the smallest
        for( int i=0 ; i<nb_nodes ; i++ )
        {
            Vec center_i = nodes[ i ]->pivot;
            real radius_i = nodes[ i ]->radius;

            // to scan all pairs only once, and avoid i==j
            for( int j=0 ; j<i ; j++ )
            {
                Vec center_j = nodes[ j ]->pivot;
                real radius_j = nodes[ j ]->radius;

                Vec t_center;
                real t_radius;
                smallestContainer( center_i, radius_i, center_j, radius_j, 
                                   t_center, t_radius );

                if( t_radius < min_radius || min_radius < 0 )
                {
                    min_i = i;
                    min_j = j ;
                    min_radius = t_radius;
                    min_center = t_center;
                }
            }
        }

#ifdef DEBUG_CHECK_NAN
        if (min_center.hasMissing())
            PLERROR("In BallTreeNearestNeighbors::treeFromLeaves: min_center is NaN");
#endif
        
        // Group these two nodes into a parent_node.
        // TODO: something more sensible for the radius and center...
        BinBallTree parent_node = new BinaryBallTree();
        parent_node->pivot = min_center;
        parent_node->radius = min_radius;
        parent_node->setFirstChild( nodes[ min_i ] );
        parent_node->setSecondChild( nodes[ min_j ] );

        nodes[ min_j ] = parent_node;
        nodes.remove( min_i );

        --nb_nodes;
    }

    // then, we have only one anchor
    BinBallTree root = nodes[ 0 ];
    return root;
}


BinBallTree BallTreeNearestNeighbors::getBallTree()
{
    return ball_tree;
}


void BallTreeNearestNeighbors::computeOutputAndCosts(
    const Vec& input, const Vec& target, Vec& output, Vec& costs ) const
{
    int nout = outputsize();
    output.resize( nout );
    costs.resize( num_neighbors );

    // we launch a k-nearest-neighbors query on the root node (ball_tree)
    priority_queue< pair<real,int> > q;
    FindBallKNN( q, input, num_neighbors );

    // dequeue the found nearest neighbors, beginning by the farthest away
    int n_found = int(q.size());
    TVec<int> neighbors( n_found );
    for( int i=n_found-1 ; i>=0 ; i-- )
    {
        const pair<real,int>& cur_top = q.top();
        costs[i] = cur_top.first;
        neighbors[i] = cur_top.second;
        q.pop();
    }

    // fill costs with missing values
    for( int i= n_found ; i<num_neighbors ; i++ )
        costs[i] = MISSING_VALUE;

    constructOutputVector( neighbors, output );
}

void BallTreeNearestNeighbors::computeOutput(
    const Vec& input, Vec& output ) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);

    int nout = outputsize();
    output.resize( nout );

    // we launch a k-nearest-neighbors query on the root node (ball_tree)
    priority_queue< pair<real,int> > q;
    FindBallKNN( q, input, num_neighbors );

    // dequeue the found nearest neighbors, beginning by the farthest away
    int n_found = int(q.size());
    TVec<int> neighbors( n_found );
    for( int i=n_found-1 ; i>=0 ; i-- )
    {
        const pair<real,int>& cur_top = q.top();
        neighbors[i] = cur_top.second;
        q.pop();
    }

    constructOutputVector( neighbors, output );

}


void BallTreeNearestNeighbors::computeCostsFromOutputs(
    const Vec& input, const Vec& output, const Vec& target, Vec& costs ) const
{
    // Compute the costs from *already* computed output.
    costs.resize( num_neighbors );

    int inputsize = train_set->inputsize();
    int targetsize = train_set->targetsize();
    int weightsize = train_set->weightsize();

    Mat out( num_neighbors, inputsize );

    if( copy_input )
    {
        for( int i=0 ; i<num_neighbors ; i++ )
            out( i ) << output.subVec( i*outputsize(), inputsize );
    }
    else if( copy_index )
    {
        int offset = 0;

        if( copy_target )
            offset += targetsize;

        if( copy_weight )
            offset += weightsize;

        for( int i=0 ; i<num_neighbors ; i++ )
            out( i ) << train_set( (int) output[ i*outputsize() + offset ] );
    }
    else
    {
        PLERROR( "computeCostsFromOutput:\n"
                 "neither indices nor coordinates of output computed\n" );
    }

    for( int i=0 ; i<num_neighbors ; i++ )
        costs[ i ] = powdistance( input, out( i ) );
}

TVec<string> BallTreeNearestNeighbors::getTestCostNames() const
{
    return TVec<string>( num_neighbors, "squared_distance" );
}

TVec<string> BallTreeNearestNeighbors::getTrainCostNames() const
{
    return TVec<string>();
}

bool BallTreeNearestNeighbors::intersect(
    const Vec& center1, const real& powrad1,
    const Vec& center2, const real& powrad2 )
{
    real radius1 = sqrt( powrad1 );
    real radius2 = sqrt( powrad2 );

    real pow_dist = powdistance( center1, center2, 2 );
    real rad_sum = radius1 + radius2;
    bool result = ( pow_dist <= ( rad_sum * rad_sum ) );
    return result;
}

bool BallTreeNearestNeighbors::contain(
    const Vec& center1, const real& powrad1,
    const Vec& center2, const real& powrad2 )
{
    real radius1 = sqrt( powrad1 );
    real radius2 = sqrt( powrad2 );
    real rad_dif = radius1 - radius2;

    if( rad_dif >= 0 )
    {
        real pow_dist = powdistance( center1, center2, 2 );
        bool result = ( pow_dist <= ( rad_dif * rad_dif ) );
        return result;
    }
    else
    {
        return false;
    }
}

void BallTreeNearestNeighbors::smallestContainer(
    const Vec& center1, const real& powrad1,
    const Vec& center2, const real& powrad2,
    Vec& t_center, real& t_powrad )
{
    if( center1 == center2 )
    {
        t_center = center1;
        t_powrad = max( powrad1, powrad2 );
    }
    else if( contain( center1, powrad1, center2, powrad2 ) )
    {
        t_center = center1;
        t_powrad = powrad1;
    }
    else if( contain( center2, powrad2, center1, powrad1 ) )
    {
        t_center = center2;
        t_powrad = powrad2;
    }
    else
    {
        real radius1 = sqrt( powrad1 );
        real radius2 = sqrt( powrad2 );
        real center_dist = dist( center1, center2, 2 ) ;
        real coef = ( radius1 - radius2 ) / center_dist ;
        t_center = real(0.5) * ( ( 1 + coef ) * center1  +  ( 1 - coef ) * center2 ) ;
        real t_radius = real(0.5) * ( center_dist + radius1 + radius2 ) ;
        t_powrad = t_radius * t_radius;
    }

#ifdef DEBUG_CHECK_NAN
    if (t_center.hasMissing())
        PLERROR("In BallTreeNearestNeighbors::smallestContainer: t_center is NaN.");
#endif
}



void BallTreeNearestNeighbors::BallKNN(
     priority_queue< pair<real,int> >& q, BinBallTree node,
     const Vec& t, real& d2_sofar, real d2_pivot, const int k ) const
{
    real d_minp = max( sqrt(d2_pivot) - node->radius, 0.0 );
#ifdef DEBUG_CHECK_NAN
    if (isnan(d_minp))
        PLERROR("BallTreeNearestNeighbors::BallKNN: d_minp is NaN");
#endif

    if (d_minp*d_minp > d2_sofar)
    {
        // no chance of finding anything closer around this node
        return;
    }
    else if (node->point_set.size()!=0) // node is leaf
    {
        int n_points = node->point_set.size();
        for( int i=0 ; i<n_points ; i++ )
        {
            int j = node->point_set[i];
            real dist;
            // last point is pivot, and we already now the distance
            if( i==n_points-1 )
            {
                dist = d2_pivot;
            }
            else
            {
                Vec x = train_set.getSubRow(j, inputsize());
                dist = powdistance(x, t, 2);
            }
            if( dist < d2_sofar )
            {
                q.push( make_pair(dist, j) );
                int n_found = int(q.size());
                if( n_found > k )
                    q.pop();
                if( n_found >= k )
                    d2_sofar = q.top().first;
            }
        }
    }
    else if (!node->isEmpty()) // node is not leaf
    {
        BinBallTree node1 = node->getFirstChild();
        BinBallTree node2 = node->getSecondChild();

        real d2_pivot1 = powdistance(t, node1->pivot, 2);
        real d2_pivot2 = powdistance(t, node2->pivot, 2);

        if( d2_pivot1 > d2_pivot2 ) // node1 is closer to t
        {
            pl_swap(node1, node2);
            pl_swap(d2_pivot1, d2_pivot2);
        }

        BallKNN(q, node1, t, d2_sofar, d2_pivot1, k);
        BallKNN(q, node2, t, d2_sofar, d2_pivot2, k); 
    }
}


void BallTreeNearestNeighbors::FindBallKNN(
    priority_queue< pair<real,int> >& q, const Vec& point, const int k ) const
{
    real d2_sofar;
    pl_isnumber("+inf", &d2_sofar);
    real d2_pivot = powdistance(point, ball_tree->pivot, 2);
//    real d_minp = 0;
    BallKNN(q, ball_tree, point, d2_sofar, d2_pivot, k);
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
