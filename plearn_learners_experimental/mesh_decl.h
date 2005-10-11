// -*- C++ -*-

// graph_decl.h
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
   * $Id: mesh_decl.h,v 1.4 2004/11/17 01:42:02 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file mesh_decl.h */


#ifndef mesh_decl_INC
#define mesh_decl_INC

// Put includes here
#include <boost/config.hpp>
#include <boost/utility.hpp>             // for boost::tie
#include <boost/graph/graph_traits.hpp>  // for boost::graph_traits
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#include "MeshVertex.h"
#include "MeshEdge.h"
#include "MeshGraph.h"

enum vertex_ppt_t { vertex_ppt };
enum edge_ppt_t { edge_ppt };
enum graph_ppt_t { graph_ppt };

namespace boost
{
    BOOST_INSTALL_PROPERTY(vertex, ppt);
    BOOST_INSTALL_PROPERTY(edge, ppt);
    BOOST_INSTALL_PROPERTY(graph, ppt);
}

namespace PLearn {

using namespace std;
using namespace boost;

// Put global function declarations here


typedef property< vertex_ppt_t, MVertex, property< vertex_index_t, int > > vertex_ppt_;
typedef property< edge_ppt_t, MEdge, property< edge_index_t, int > > edge_ppt_;
typedef property< graph_ppt_t, MGraph > graph_ppt_;

typedef adjacency_list< listS, listS, undirectedS,
                        vertex_ppt_, edge_ppt_, graph_ppt_ > graph;

class Graph_ : public graph, public PPointable {};
typedef PP< Graph_ > Graph;

typedef graph_traits<graph>::vertex_descriptor vertex_descriptor;
typedef graph_traits<graph>::edge_descriptor edge_descriptor;

typedef graph_traits<graph>::vertex_iterator vertex_iterator;
typedef graph_traits<graph>::edge_iterator edge_iterator;
typedef graph_traits<graph>::out_edge_iterator out_edge_iterator;
typedef graph_traits<graph>::adjacency_iterator adjacency_iterator;


} // end of namespace PLearn

#endif

