// -*- C++ -*-

// RegressionTreeQueue.cc
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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


/* ********************************************************************************    
 * $Id: RegressionTreeQueue.cc, v 1.0 2005/02/35 10:00:00 Bengio/Kegl/Godbout    *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "RegressionTreeQueue.h"
#include "RegressionTreeNode.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(RegressionTreeQueue,
                        "Object to represent the priority queue of a regression tree.",
                        "The queue is used to keep all the nodes of the tree not yet expanded.\n"
                        "They are kept in a heap with the best possible split always on top.\n"
    );

RegressionTreeQueue::RegressionTreeQueue()    
    : verbosity(0),
      maximum_number_of_nodes(400)
{
    build();
}
RegressionTreeQueue::RegressionTreeQueue(int verbosity_,
                                         int maximum_number_of_nodes_)
    : verbosity(verbosity_),
      maximum_number_of_nodes(maximum_number_of_nodes_)
{
    build();
}
RegressionTreeQueue::~RegressionTreeQueue()
{
}

void RegressionTreeQueue::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "verbosity", &RegressionTreeQueue::verbosity, OptionBase::buildoption,
                  "The desired level of verbosity\n");
    declareOption(ol, "maximum_number_of_nodes", &RegressionTreeQueue::maximum_number_of_nodes, OptionBase::buildoption,
                  "The maximum number of entries in the heap\n");
 
    declareOption(ol, "next_available_node", &RegressionTreeQueue::next_available_node, OptionBase::learntoption,
                  "The next available entry in the heap to add a node\n");
    declareOption(ol, "nodes", &RegressionTreeQueue::nodes, OptionBase::learntoption,
                  "The table of nodes kept with the best possible one to split on top\n");
    inherited::declareOptions(ol);
}

void RegressionTreeQueue::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(verbosity, copies);
    deepCopyField(maximum_number_of_nodes, copies);
    deepCopyField(next_available_node, copies);
    deepCopyField(nodes, copies);
}

void RegressionTreeQueue::build()
{
    inherited::build();
    build_();
}

void RegressionTreeQueue::build_()
{
    next_available_node = 0;
    nodes.resize(maximum_number_of_nodes);
}

void RegressionTreeQueue::addHeap(PP<RegressionTreeNode> new_node)
{
    if (new_node->getErrorImprovment() < 0.0)
    {
        return;
    }
    if (next_available_node >= maximum_number_of_nodes) PLERROR("RegressionTreeQueue: maximum number of entries exceeded (400)");
    nodes[next_available_node] = upHeap(new_node, next_available_node);
    next_available_node += 1;
}

PP<RegressionTreeNode> RegressionTreeQueue::popHeap()
{
    PP<RegressionTreeNode> return_value;
    return_value = nodes[0];
    next_available_node -= 1;
    nodes[0] = downHeap(nodes[next_available_node], 0);
    return return_value;
}

PP<RegressionTreeNode> RegressionTreeQueue::upHeap(PP<RegressionTreeNode> new_node, int node_ind)
{
    int parent_node;
    PP<RegressionTreeNode> saved_node;
    if (node_ind == 0) return new_node;
    parent_node = (node_ind - 1) / 2;
    if (compareNode(new_node, nodes[parent_node]) < 0)
    {
        saved_node = nodes[parent_node];
        nodes[parent_node] = upHeap(new_node, parent_node);
        return saved_node;      
    }
    return new_node;
}

PP<RegressionTreeNode> RegressionTreeQueue::downHeap(PP<RegressionTreeNode> new_node, int node_ind)
{
    int left_child_node;
    int right_child_node;
    int smallest_child_node;
    PP<RegressionTreeNode> saved_node;
    left_child_node = 2 * node_ind + 1;
    if (left_child_node >= next_available_node) return new_node;
    right_child_node = 2 * node_ind + 2;
    smallest_child_node = left_child_node;
    if (right_child_node < next_available_node)
    {
        if (compareNode(nodes[left_child_node], nodes[right_child_node]) > 0)
        {
            smallest_child_node = right_child_node;
        }
    }
    if (compareNode(new_node, nodes[smallest_child_node]) > 0)    
    {
        saved_node = nodes[smallest_child_node];
        nodes[smallest_child_node] = downHeap(new_node, smallest_child_node);
        return saved_node;      
    }
    return new_node;
}
int RegressionTreeQueue::isEmpty()
{
    return next_available_node;
}

int RegressionTreeQueue::compareNode(PP<RegressionTreeNode> node1, PP<RegressionTreeNode> node2)
{
    if (node1->getErrorImprovment() > node2->getErrorImprovment()) return -1;
    if (node1->getErrorImprovment() < node2->getErrorImprovment()) return +1;
    if (node1->getSplitBalance() < node2->getSplitBalance()) return -1;
    return +1;
}

void RegressionTreeQueue::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        cout << the_msg << endl;
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
