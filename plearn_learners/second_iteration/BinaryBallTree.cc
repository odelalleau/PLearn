// -*- C++ -*-

// BinaryBallTree.cc
//
// Copyright (C) 2004 Pascal Lamblin 
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
 * $Id: BinaryBallTree.cc 3994 2005-08-25 13:35:03Z chapados $ 
 ******************************************************* */

// Authors: Pascal Lamblin

/*! \file BinaryBallTree.cc */


#include "BinaryBallTree.h"

namespace PLearn {
using namespace std;

BinaryBallTree::BinaryBallTree() 
    : pivot( Vec() ),
      radius( 0 )
{}

PLEARN_IMPLEMENT_OBJECT( BinaryBallTree,
                         "Binary Tree, containing a point, a radius, and a set of points", 
                         "Each node of the tree contains the parameters of a ball :\n"
                         "a point and a radius.\n"
                         "Each leaf node contains a list of indices of points,\n"
                         "each non-leaf node has two children nodes.");

void BinaryBallTree::declareOptions( OptionList& ol )
{
    declareOption( ol, "pivot", &BinaryBallTree::pivot, OptionBase::buildoption,
                   "Center of the ball" );

    declareOption(ol, "radius", &BinaryBallTree::radius, OptionBase::buildoption,
                  "Radius of the ball" );

    declareOption(ol, "point_set", &BinaryBallTree::point_set, OptionBase::buildoption,
                  "List of indices of the points owned by this node (leaf only)" );

    declareOption(ol, "child1", &BinaryBallTree::child1, OptionBase::tuningoption,
                  "Pointer to first child (non-leaf only)" );

    declareOption(ol, "child2", &BinaryBallTree::child2, OptionBase::tuningoption,
                  "Pointer to second child (non-leaf only)" );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void BinaryBallTree::build_()
{
    if( child1 )
    { child1->parent = this; }

    if( child2 )
    { child2->parent = this; }
}

void BinaryBallTree::build()
{
    inherited::build();
    build_();
}

void BinaryBallTree::setFirstChild( const BinBallTree& first_child )
{
    this->child1 = first_child;
    if( first_child )
    {
        first_child->parent = this;
    }
}

void BinaryBallTree::setSecondChild( const BinBallTree& second_child )
{
    this->child2 = second_child;
    if( second_child )
    {
        second_child->parent = this;
    }
}

BinBallTree BinaryBallTree::getFirstChild()
{
    return this->child1;
}

BinBallTree BinaryBallTree::getSecondChild()
{
    return this->child2;
}

BinaryBallTree* BinaryBallTree::getParent()
{
    return this->parent;
}

void BinaryBallTree::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField( child1, copies );
    deepCopyField( child2, copies );
    deepCopyField( pivot, copies );
    deepCopyField( point_set, copies );
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
