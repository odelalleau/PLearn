// -*- C++ -*-

// RBMWoodsLayer.cc
//
// Copyright (C) 2008 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file RBMWoodsLayer.cc */



#include "RBMWoodsLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMWoodsLayer,
    "RBM layer with tree-structured groups of units.",
    "");

RBMWoodsLayer::RBMWoodsLayer( real the_learning_rate ) :
    inherited( the_learning_rate ),
    n_trees( 10 ),
    tree_depth( 3 ),
    use_signed_samples( false )
{
}

////////////////////
// generateSample //
////////////////////
void RBMWoodsLayer::generateSample()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectation_is_up_to_date, "Expectation should be computed "
            "before calling generateSample()");

    if(use_signed_samples)
        sample.fill(-1);
    else
        sample.clear();

    int n_nodes_per_tree = size / n_trees;    
    int node, depth, node_sample, sub_tree_size;
    int offset = 0;

    for( int t=0; t<n_trees; t++ )
    {
        depth = 0;
        node = n_nodes_per_tree / 2;
        sub_tree_size = node;
        while( depth < tree_depth )
        {
            node_sample = random_gen->binomial_sample( 
                local_node_expectation[ node + offset ] );
            if( use_signed_samples )
                sample[node + offset] = 2*node_sample-1;
            else
                sample[node + offset] = node_sample;
            
            // Descending in the tree
            sub_tree_size /= 2;
            if ( node_sample > 0.5 )
                node -= sub_tree_size+1;
            else
                node += sub_tree_size+1;
            depth++;
        }
        offset += n_nodes_per_tree;
    }
}

/////////////////////
// generateSamples //
/////////////////////
void RBMWoodsLayer::generateSamples()
{
    PLASSERT_MSG(random_gen,
                 "random_gen should be initialized before generating samples");

    PLCHECK_MSG(expectations_are_up_to_date, "Expectations should be computed "
            "before calling generateSamples()");

    PLASSERT( samples.width() == size && samples.length() == batch_size );

    PLERROR( "RBMWoodsLayer::generateSamples(): not implemented yet" );
}

void RBMWoodsLayer::computeProbabilisticClustering(Vec& prob_clusters)
{
    computeExpectation();
    int offset = 0;
    int n_nodes_per_tree = size / n_trees;
    prob_clusters.resize(n_trees*(n_nodes_per_tree+1));
    for( int t=0; t<n_trees; t++ )
    {
        for( int i=0; i<n_nodes_per_tree; i = i+2)
            prob_clusters[i+offset+t] = expectation[i+offset];
        for( int i=0; i<n_nodes_per_tree; i = i+2)
            prob_clusters[i+1+offset+t] = off_expectation[i+offset];
        offset += n_nodes_per_tree;
    }
}

////////////////////////
// computeExpectation //
////////////////////////
void RBMWoodsLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    int n_nodes_per_tree = size / n_trees;    
    int node, depth, sub_tree_size, grand_parent;
    int offset = 0;
    bool left_of_grand_parent;
    real grand_parent_prob;

    // Get local expectations at every node
    
    // Divide and conquer computation of local (conditional) free energies
    for( int t=0; t<n_trees; t++ )
    {
        depth = tree_depth-1;
        sub_tree_size = 0;

        // Initialize last level
        for( int n=sub_tree_size; n<n_nodes_per_tree; n += 2*sub_tree_size + 2 )
        {
            //on_free_energy[ n + offset ] = safeexp(activation[n+offset]);
            //off_free_energy[ n + offset ] = 1;
            // Now working in log-domain
            on_free_energy[ n + offset ] = activation[n+offset];
            if( use_signed_samples )
                off_free_energy[ n + offset ] = -activation[n+offset];
            else
                off_free_energy[ n + offset ] = 0;
        }

        depth = tree_depth-2;
        sub_tree_size = 1;

        while( depth >= 0 )
        {
            for( int n=sub_tree_size; n<n_nodes_per_tree; n += 2*sub_tree_size + 2 )
            {
                //on_free_energy[ n + offset ] = safeexp(activation[n+offset]) * 
                //    ( on_free_energy[n + offset - sub_tree_size] + off_free_energy[n + offset - sub_tree_size] ) ;
                //off_free_energy[ n + offset ] = 
                //    ( on_free_energy[n + offset + sub_tree_size] + off_free_energy[n + offset + sub_tree_size] ) ;
                // Now working in log-domain
                on_free_energy[ n + offset ] = activation[n+offset] + 
                    logadd( on_free_energy[n + offset - (sub_tree_size/2+1)],
                            off_free_energy[n + offset - (sub_tree_size/2+1)] ) ;
                if( use_signed_samples )
                    off_free_energy[ n + offset ] = -activation[n+offset] +
                        logadd( on_free_energy[n + offset + (sub_tree_size/2+1)],
                                off_free_energy[n + offset + (sub_tree_size/2+1)] ) ;
                else
                    off_free_energy[ n + offset ] = 
                        logadd( on_free_energy[n + offset + (sub_tree_size/2+1)],
                                off_free_energy[n + offset + (sub_tree_size/2+1)] ) ;
                
            }
            sub_tree_size = 2 * ( sub_tree_size + 1 ) - 1;
            depth--;
        }
        offset += n_nodes_per_tree;
    }    
    
    for( int i=0 ; i<size ; i++ )
        //local_node_expectation[i] = on_free_energy[i] / ( on_free_energy[i] + off_free_energy[i] );
        // Now working in log-domain
        local_node_expectation[i] = safeexp(on_free_energy[i] 
                                            - logadd(on_free_energy[i], off_free_energy[i]));

    // Compute marginal expectations
    offset = 0;
    for( int t=0; t<n_trees; t++ )
    {
        // Initialize root        
        node = n_nodes_per_tree / 2;
        expectation[ node + offset ] = local_node_expectation[ node + offset ];
        off_expectation[ node + offset ] = (1 - local_node_expectation[ node + offset ]);
        sub_tree_size = node;

        // First level nodes
        depth = 1;
        sub_tree_size /= 2;

        // Left child
        node = sub_tree_size;
        expectation[ node + offset ] = local_node_expectation[ node + offset ]
            * local_node_expectation[ node + offset + sub_tree_size + 1 ];
        off_expectation[ node + offset ] = (1 - local_node_expectation[ node + offset ])
            * local_node_expectation[ node + offset + sub_tree_size + 1 ];

        // Right child
        node = 3*sub_tree_size+2;
        expectation[ node + offset ] = local_node_expectation[ node + offset ]
            * (1 - local_node_expectation[ node + offset - sub_tree_size - 1 ]);
        off_expectation[ node + offset ] = (1 - local_node_expectation[ node + offset ])
            * (1 - local_node_expectation[ node + offset - sub_tree_size - 1 ]);

        // Set other nodes, level-wise
        depth = 2;
        sub_tree_size /= 2;
        while( depth < tree_depth )
        {
            // Left child
            left_of_grand_parent = true;
            for( int n=sub_tree_size; n<n_nodes_per_tree; n += 4*sub_tree_size + 4 )
            {
                if( left_of_grand_parent )
                {
                    grand_parent = n + offset + 3*sub_tree_size + 3;
                    grand_parent_prob = expectation[ grand_parent ];
                    left_of_grand_parent = false;
                }
                else
                {
                    grand_parent = n + offset - sub_tree_size - 1;
                    grand_parent_prob = off_expectation[ grand_parent ];
                    left_of_grand_parent = true;
                }

                expectation[ n + offset ] = local_node_expectation[ n + offset ]
                    * local_node_expectation[ n + offset + sub_tree_size + 1 ]
                    * grand_parent_prob;
                off_expectation[ n + offset ] = (1 - local_node_expectation[ n + offset ])
                    * local_node_expectation[ n + offset + sub_tree_size + 1 ]
                    * grand_parent_prob;

            }

            // Right child
            left_of_grand_parent = true;
            for( int n=3*sub_tree_size+2; n<n_nodes_per_tree; n += 4*sub_tree_size + 4 )
            {
                if( left_of_grand_parent )
                {
                    grand_parent = n + offset + sub_tree_size + 1;
                    grand_parent_prob = expectation[ grand_parent ];
                    left_of_grand_parent = false;
                }
                else
                {
                    grand_parent = n + offset - 3*sub_tree_size - 3;
                    grand_parent_prob = off_expectation[ grand_parent ];
                    left_of_grand_parent = true;
                }

                expectation[ n + offset ] = local_node_expectation[ n + offset ]
                    * (1 - local_node_expectation[ n + offset - sub_tree_size - 1 ])
                    * grand_parent_prob;
                off_expectation[ n + offset ] = (1 - local_node_expectation[ n + offset ])
                    * (1 - local_node_expectation[ n + offset - sub_tree_size - 1 ])
                    * grand_parent_prob;
            }
            sub_tree_size /= 2;
            depth++;
        }
        offset += n_nodes_per_tree;
    }

    expectation_is_up_to_date = true;
}

/////////////////////////
// computeExpectations //
/////////////////////////
void RBMWoodsLayer::computeExpectations()
{
    if( expectations_are_up_to_date )
        return;

    PLERROR( "RBMWoodsLayer::computeExpectations(): not implemented yet" );

    expectations_are_up_to_date = true;
}

///////////
// fprop //
///////////
void RBMWoodsLayer::fprop( const Vec& input, Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    output.resize( output_size );

    int n_nodes_per_tree = size / n_trees;    
    int node, depth, sub_tree_size, grand_parent;
    int offset = 0;    
    bool left_of_grand_parent;
    real grand_parent_prob;

    // Get local expectations at every node
    
    // Divide and conquer computation of local (conditional) free energies
    for( int t=0; t<n_trees; t++ )
    {
        depth = tree_depth-1;
        sub_tree_size = 0;

        // Initialize last level
        for( int n=sub_tree_size; n<n_nodes_per_tree; n += 2*sub_tree_size + 2 )
        {
            //on_free_energy[ n + offset ] = safeexp(input[n+offset] + bias[n+offset]);
            //off_free_energy[ n + offset ] = 1;
            // Now working in log-domain
            on_free_energy[ n + offset ] = input[n+offset] + bias[n+offset];
            if( use_signed_samples )
                off_free_energy[ n + offset ] = -(input[n+offset] + bias[n+offset]);
            else
                off_free_energy[ n + offset ] = 0;
        }

        depth = tree_depth-2;
        sub_tree_size = 1;

        while( depth >= 0 )
        {
            for( int n=sub_tree_size; n<n_nodes_per_tree; n += 2*sub_tree_size + 2 )
            {
                //on_free_energy[ n + offset ] = safeexp(input[n+offset] + bias[n+offset]) * 
                //    ( on_free_energy[n + offset - sub_tree_size] + off_free_energy[n + offset - sub_tree_size] ) ;
                //off_free_energy[ n + offset ] = 
                //    ( on_free_energy[n + offset + sub_tree_size] + off_free_energy[n + offset + sub_tree_size] ) ;
                // Now working in the log-domain
                on_free_energy[ n + offset ] = input[n+offset] + bias[n+offset] +
                    logadd( on_free_energy[n + offset - (sub_tree_size/2+1)], 
                            off_free_energy[n + offset - (sub_tree_size/2+1)] ) ;
                if( use_signed_samples )
                    off_free_energy[ n + offset ] = -(input[n+offset] + bias[n+offset]) +
                        logadd( on_free_energy[n + offset + (sub_tree_size/2+1)], 
                                off_free_energy[n + offset + (sub_tree_size/2+1)] ) ;
                else
                    off_free_energy[ n + offset ] = 
                        logadd( on_free_energy[n + offset + (sub_tree_size/2+1)], 
                                off_free_energy[n + offset + (sub_tree_size/2+1)] ) ;
            }
            sub_tree_size = 2 * ( sub_tree_size + 1 ) - 1;
            depth--;
        }
        offset += n_nodes_per_tree;
    }    
    
    for( int i=0 ; i<size ; i++ )
        //local_node_expectation[i] = on_free_energy[i] / ( on_free_energy[i] + off_free_energy[i] );
        // Now working in log-domain
        local_node_expectation[i] = safeexp(on_free_energy[i] 
                                            - logadd(on_free_energy[i], off_free_energy[i]));

    // Compute marginal expectations
    offset = 0;    
    for( int t=0; t<n_trees; t++ )
    {
        // Initialize root        
        node = n_nodes_per_tree / 2;
        output[ node + offset ] = local_node_expectation[ node + offset ];
        off_expectation[ node + offset ] = (1 - local_node_expectation[ node + offset ]);
        sub_tree_size = node;

        // First level nodes
        depth = 1;
        sub_tree_size /= 2;

        // Left child
        node = sub_tree_size;
        output[ node + offset ] = local_node_expectation[ node + offset ]
            * local_node_expectation[ node + offset + sub_tree_size + 1 ];
        off_expectation[ node + offset ] = (1 - local_node_expectation[ node + offset ])
            * local_node_expectation[ node + offset + sub_tree_size + 1 ];

        // Right child
        node = 3*sub_tree_size+2;
        output[ node + offset ] = local_node_expectation[ node + offset ]
            * (1 - local_node_expectation[ node + offset - sub_tree_size - 1 ]);
        off_expectation[ node + offset ] = (1 - local_node_expectation[ node + offset ])
            * (1 - local_node_expectation[ node + offset - sub_tree_size - 1 ]);

        // Set other nodes, level-wise
        depth = 2;
        sub_tree_size /= 2;
        while( depth < tree_depth )
        {
            // Left child
            left_of_grand_parent = true;
            for( int n=sub_tree_size; n<n_nodes_per_tree; n += 4*sub_tree_size + 4 )
            {
                if( left_of_grand_parent )
                {
                    grand_parent = n + offset + 3*sub_tree_size + 3;
                    grand_parent_prob = output[ grand_parent ];
                    left_of_grand_parent = false;
                }
                else
                {
                    grand_parent = n + offset - sub_tree_size - 1;
                    grand_parent_prob = off_expectation[ grand_parent ];
                    left_of_grand_parent = true;
                }

                output[ n + offset ] = local_node_expectation[ n + offset ]
                    * local_node_expectation[ n + offset + sub_tree_size + 1 ]
                    * grand_parent_prob;
                off_expectation[ n + offset ] = (1 - local_node_expectation[ n + offset ])
                    * local_node_expectation[ n + offset + sub_tree_size + 1 ]
                    * grand_parent_prob;
            }

            // Right child
            left_of_grand_parent = true;
            for( int n=3*sub_tree_size+2; n<n_nodes_per_tree; n += 4*sub_tree_size + 4 )
            {
                if( left_of_grand_parent )
                {
                    grand_parent = n + offset + sub_tree_size + 1;
                    grand_parent_prob = output[ grand_parent ];
                    left_of_grand_parent = false;
                }
                else
                {
                    grand_parent = n + offset - 3*sub_tree_size - 3;
                    grand_parent_prob = off_expectation[ grand_parent ];
                    left_of_grand_parent = true;
                }

                output[ n + offset ] = local_node_expectation[ n + offset ]
                    * (1 - local_node_expectation[ n + offset - sub_tree_size - 1 ])
                    * grand_parent_prob;
                off_expectation[ n + offset ] = (1 - local_node_expectation[ n + offset ])
                    * (1 - local_node_expectation[ n + offset - sub_tree_size - 1 ])
                    * grand_parent_prob;
            }
            sub_tree_size /= 2;
            depth++;
        }
        offset += n_nodes_per_tree;
    }
}

void RBMWoodsLayer::fprop( const Mat& inputs, Mat& outputs ) const
{
    int mbatch_size = inputs.length();
    PLASSERT( inputs.width() == size );
    outputs.resize( mbatch_size, size );

    PLERROR( "RBMWoodsLayer::fprop(): not implemented yet" );
}

void RBMWoodsLayer::fprop( const Vec& input, const Vec& rbm_bias,
                              Vec& output ) const
{
    PLASSERT( input.size() == input_size );
    PLASSERT( rbm_bias.size() == input_size );
    output.resize( output_size );

    PLERROR( "RBMWoodsLayer::fprop(): not implemented yet" );
}

/////////////////
// bpropUpdate //
/////////////////
void RBMWoodsLayer::bpropUpdate(const Vec& input, const Vec& output,
                                   Vec& input_gradient,
                                   const Vec& output_gradient,
                                   bool accumulate)
{
    PLASSERT( input.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }
    else
    {
        input_gradient.resize( size );
        input_gradient.clear();
    }

    // Compute gradient on marginal expectations
    int n_nodes_per_tree = size / n_trees;    
    int node, depth, sub_tree_size, grand_parent;
    int offset = 0;
    bool left_of_grand_parent;
    real grand_parent_prob;
    real node_exp, parent_exp, out_grad, off_grad;
    local_node_expectation_gradient.clear();
    on_tree_gradient.clear();
    off_tree_gradient.clear();

    for( int t=0; t<n_trees; t++ )
    {
        // Set other nodes, level-wise
        depth = tree_depth-1;
        sub_tree_size = 0;
        while( depth > 1 )
        {
            // Left child
            left_of_grand_parent = true;
            for( int n=sub_tree_size; n<n_nodes_per_tree; n += 4*sub_tree_size + 4 )
            {
                out_grad = output_gradient[ n + offset ] + 
                    on_tree_gradient[ n + offset ] ;
                off_grad = off_tree_gradient[ n + offset ] ;
                node_exp = local_node_expectation[ n + offset ];
                parent_exp = local_node_expectation[ n + offset + sub_tree_size + 1 ];

                if( left_of_grand_parent )
                {
                    grand_parent = n + offset + 3*sub_tree_size + 3;
                    grand_parent_prob = output[ grand_parent ];
                    // Gradient for rest of the tree
                    on_tree_gradient[ grand_parent ] += 
                        ( out_grad * node_exp 
                          + off_grad * (1 - node_exp) ) 
                        * parent_exp;
                    left_of_grand_parent = false;
                }
                else
                {
                    grand_parent = n + offset - sub_tree_size - 1;
                    grand_parent_prob = off_expectation[ grand_parent ];
                    // Gradient for rest of the tree
                    off_tree_gradient[ grand_parent ] += 
                        ( out_grad * node_exp 
                          + off_grad * (1 - node_exp) )
                        * parent_exp;
                    left_of_grand_parent = true;
                }

                // Gradient w/r current node
                local_node_expectation_gradient[ n + offset ] += 
                    ( out_grad - off_grad ) * parent_exp * grand_parent_prob;
                    //* node_exp * ( 1 - node_exp );

                // Gradient w/r parent node
                local_node_expectation_gradient[ n + offset + sub_tree_size + 1 ] += 
                    ( out_grad * node_exp + off_grad * (1 - node_exp) )  * grand_parent_prob;
                    //* parent_exp * (1-parent_exp) ;

            }

            // Right child
            left_of_grand_parent = true;
            for( int n=3*sub_tree_size+2; n<n_nodes_per_tree; n += 4*sub_tree_size + 4 )
            {
                out_grad = output_gradient[ n + offset ] + 
                    on_tree_gradient[ n + offset ] ;
                off_grad = off_tree_gradient[ n + offset ] ;
                node_exp = local_node_expectation[ n + offset ];
                parent_exp = local_node_expectation[ n + offset - sub_tree_size - 1 ];

                if( left_of_grand_parent )
                {
                    grand_parent = n + offset + sub_tree_size + 1;
                    grand_parent_prob = output[ grand_parent ];
                    // Gradient for rest of the tree
                    on_tree_gradient[ grand_parent ] += 
                        ( out_grad * node_exp 
                          + off_grad * (1 - node_exp) ) 
                        * ( 1 - parent_exp );
                    left_of_grand_parent = false;
                }
                else
                {
                    grand_parent = n + offset - 3*sub_tree_size - 3;
                    grand_parent_prob = off_expectation[ grand_parent ];
                    // Gradient for rest of the tree
                    off_tree_gradient[ grand_parent ] += 
                        ( out_grad * node_exp 
                          + off_grad * (1 - node_exp) ) 
                        * ( 1 - parent_exp );
                    left_of_grand_parent = true;
                }

                // Gradient w/r current node
                local_node_expectation_gradient[ n + offset ] += 
                    ( out_grad - off_grad ) * ( 1 - parent_exp ) * grand_parent_prob;
                    //* node_exp * ( 1 - node_exp );

                // Gradient w/r parent node
                local_node_expectation_gradient[ n + offset - sub_tree_size - 1 ] -= 
                    ( out_grad * node_exp + off_grad * (1 - node_exp) )  * grand_parent_prob;
                    //* parent_exp * (1-parent_exp) ;
            }
            sub_tree_size = 2 * ( sub_tree_size + 1 ) - 1;
            depth--;
        }

        ////// First level nodes
        depth = 1;

        //// Left child
        node = sub_tree_size;
        out_grad = output_gradient[ node + offset ] + 
            on_tree_gradient[ node + offset ] ;
        off_grad = off_tree_gradient[ node + offset ] ;
        node_exp = local_node_expectation[ node + offset ];
        parent_exp = local_node_expectation[ node + offset + sub_tree_size + 1 ];
        
        // Gradient w/r current node
        local_node_expectation_gradient[ node + offset ] += 
            ( out_grad - off_grad ) * parent_exp;
            //* node_exp * ( 1 - node_exp );
        
        // Gradient w/r parent node
        local_node_expectation_gradient[ node + offset + sub_tree_size + 1 ] += 
            ( out_grad * node_exp  + off_grad * (1 - node_exp) );
            //* parent_exp * (1-parent_exp) ;

        //// Right child
        node = 3*sub_tree_size+2;
        out_grad = output_gradient[ node + offset ] + 
            on_tree_gradient[ node + offset ] ;
        off_grad = off_tree_gradient[ node + offset ] ;
        node_exp = local_node_expectation[ node + offset ];
        parent_exp = local_node_expectation[ node + offset - sub_tree_size - 1 ];

        // Gradient w/r current node
        local_node_expectation_gradient[ node + offset ] += 
            ( out_grad - off_grad ) * ( 1 - parent_exp ) ;
            //* node_exp * ( 1 - node_exp );
        
        // Gradient w/r parent node
        local_node_expectation_gradient[ node + offset - sub_tree_size - 1 ] -= 
            ( out_grad * node_exp + off_grad * (1 - node_exp) ) ;
            //* parent_exp * (1-parent_exp) ;
        
        ////// Root
        node = n_nodes_per_tree / 2;
        sub_tree_size = 2 * ( sub_tree_size + 1 ) - 1;

        out_grad = output_gradient[ node + offset ] + 
            on_tree_gradient[ node + offset ] ;
        off_grad = off_tree_gradient[ node + offset ] ;
        node_exp = local_node_expectation[ node + offset ];
        local_node_expectation_gradient[ node + offset ] += 
            ( out_grad - off_grad );// * node_exp * ( 1 - node_exp );

        offset += n_nodes_per_tree;
    }

    for( int i=0 ; i<size ; i++ )
    {
        node_exp = local_node_expectation[i];
        out_grad = local_node_expectation_gradient[i];
        on_free_energy_gradient[i] = out_grad * node_exp * ( 1 - node_exp );
        off_free_energy_gradient[i] = -out_grad * node_exp * ( 1 - node_exp );
    }

    offset = 0;
    for( int t=0; t<n_trees; t++ )
    {
        depth = 0;
        sub_tree_size = n_nodes_per_tree / 2;

        while( depth < tree_depth-1 )
        {
            for( int n=sub_tree_size; n<n_nodes_per_tree; n += 2*sub_tree_size + 2 )
            {
                out_grad = on_free_energy_gradient[ n + offset ];
                node_exp = local_node_expectation[n + offset - (sub_tree_size/2+1)];
                input_gradient[n+offset] += out_grad;
                on_free_energy_gradient[n + offset - (sub_tree_size/2+1)] += out_grad * node_exp; 
                off_free_energy_gradient[n + offset - (sub_tree_size/2+1)] += out_grad * (1 - node_exp); 

                out_grad = off_free_energy_gradient[ n + offset ];
                node_exp = local_node_expectation[n + offset + (sub_tree_size/2+1)];
                if( use_signed_samples )
                    input_gradient[n+offset] -= out_grad;
                on_free_energy_gradient[n + offset + (sub_tree_size/2+1)] += out_grad * node_exp; 
                off_free_energy_gradient[n + offset + (sub_tree_size/2+1)] += 
                    out_grad * (1 - node_exp); 
            }
            sub_tree_size /= 2;
            depth++;
        }

        depth = tree_depth-1;
        sub_tree_size = 0;

        for( int n=sub_tree_size; n<n_nodes_per_tree; n += 2*sub_tree_size + 2 )
        {
            input_gradient[n+offset] += on_free_energy_gradient[ n + offset ];
            if( use_signed_samples )
                input_gradient[n+offset] -= off_free_energy_gradient[ n + offset ];                
        }

        offset += n_nodes_per_tree;
    }

    if( momentum != 0. )
        bias_inc.resize( size );

    for( int i=0 ; i<size ; i++ )
    {
        real in_grad_i = input_gradient[i];

        if( momentum == 0. )
        {
            // update the bias: bias -= learning_rate * input_gradient
            bias[i] -= learning_rate * in_grad_i;
        }
        else
        {
            // The update rule becomes:
            // bias_inc = momentum * bias_inc - learning_rate * input_gradient
            // bias += bias_inc
            bias_inc[i] = momentum * bias_inc[i] - learning_rate * in_grad_i;
            bias[i] += bias_inc[i];
        }
    }

    applyBiasDecay();
}

void RBMWoodsLayer::bpropUpdate(const Mat& inputs, const Mat& outputs,
                                   Mat& input_gradients,
                                   const Mat& output_gradients,
                                   bool accumulate)
{
    PLASSERT( inputs.width() == size );
    PLASSERT( outputs.width() == size );
    PLASSERT( output_gradients.width() == size );

    int mbatch_size = inputs.length();
    PLASSERT( outputs.length() == mbatch_size );
    PLASSERT( output_gradients.length() == mbatch_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == size &&
                input_gradients.length() == mbatch_size,
                "Cannot resize input_gradients and accumulate into it" );
    }
    else
    {
        input_gradients.resize(mbatch_size, size);
        input_gradients.clear();
    }

    PLERROR( "RBMWoodsLayer::bpropUpdate(): not implemeted yet" );

    if( momentum != 0. )
        bias_inc.resize( size );

    // TODO Can we do this more efficiently? (using BLAS)

    // We use the average gradient over the mini-batch.
    real avg_lr = learning_rate / inputs.length();

    for (int j = 0; j < mbatch_size; j++)
    {
        for( int i=0 ; i<size ; i++ )
        {
            real output_i = outputs(j, i);
            real in_grad_i = output_i * (1-output_i) * output_gradients(j, i);
            input_gradients(j, i) += in_grad_i;

            if( momentum == 0. )
            {
                // update the bias: bias -= learning_rate * input_gradient
                bias[i] -= avg_lr * in_grad_i;
            }
            else
            {
                PLERROR("In RBMWoodsLayer:bpropUpdate - Not implemented for "
                        "momentum with mini-batches");
                // The update rule becomes:
                // bias_inc = momentum * bias_inc - learning_rate * input_gradient
                // bias += bias_inc
                bias_inc[i] = momentum * bias_inc[i] - learning_rate * in_grad_i;
                bias[i] += bias_inc[i];
            }
        }
    }

    applyBiasDecay();
}


//! TODO: add "accumulate" here
void RBMWoodsLayer::bpropUpdate(const Vec& input, const Vec& rbm_bias,
                                   const Vec& output,
                                   Vec& input_gradient, Vec& rbm_bias_gradient,
                                   const Vec& output_gradient)
{
    PLASSERT( input.size() == size );
    PLASSERT( rbm_bias.size() == size );
    PLASSERT( output.size() == size );
    PLASSERT( output_gradient.size() == size );
    input_gradient.resize( size );
    rbm_bias_gradient.resize( size );

    PLERROR( "RBMWoodsLayer::bpropUpdate(): not implemeted yet" );

    for( int i=0 ; i<size ; i++ )
    {
        real output_i = output[i];
        input_gradient[i] = output_i * (1-output_i) * output_gradient[i];
    }

    rbm_bias_gradient << input_gradient;
}

real RBMWoodsLayer::fpropNLL(const Vec& target)
{
    PLASSERT( target.size() == input_size );

    PLERROR( "RBMWoodsLayer::fpropNLL(): not implemeted yet" );

    real ret = 0;
    real target_i, activation_i;
    if(use_fast_approximations){
        for( int i=0 ; i<size ; i++ )
        {
            target_i = target[i];
            activation_i = activation[i];
            ret += tabulated_softplus(activation_i) - target_i * activation_i;
            // nll = - target*log(sigmoid(act)) -(1-target)*log(1-sigmoid(act))
            // but it is numerically unstable, so use instead the following identity:
            //     = target*softplus(-act) +(1-target)*(act+softplus(-act))
            //     = act + softplus(-act) - target*act 
            //     = softplus(act) - target*act
        }
    } else {
        for( int i=0 ; i<size ; i++ )
        {
            target_i = target[i];
            activation_i = activation[i];
            ret += softplus(activation_i) - target_i * activation_i;
        }
    }
    return ret;
}

void RBMWoodsLayer::fpropNLL(const Mat& targets, const Mat& costs_column)
{
    // computeExpectations(); // why?

    PLERROR( "RBMWoodsLayer::fpropNLL(): not implemeted yet" );

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );

    for (int k=0;k<batch_size;k++) // loop over minibatch
    {
        real nll = 0;
        real* activation = activations[k];
        real* target = targets[k];
        if(use_fast_approximations){
            for( int i=0 ; i<size ; i++ ) // loop over outputs
            {
                if(!fast_exact_is_equal(target[i],0.0))
                    // nll -= target[i] * pl_log(expectations[i]); 
                    // but it is numerically unstable, so use instead
                    // log (1/(1+exp(-x))) = -log(1+exp(-x)) = -softplus(-x)
                    nll += target[i] * tabulated_softplus(-activation[i]);
                if(!fast_exact_is_equal(target[i],1.0))
                    // nll -= (1-target[i]) * pl_log(1-output[i]);
                    // log (1 - 1/(1+exp(-x))) = log(exp(-x)/(1+exp(-x)))
                    //                         = log(1/(1+exp(x)))
                    //                         = -log(1+exp(x))
                    //                         = -softplus(x)
                    nll += (1-target[i]) * tabulated_softplus(activation[i]);
            }
        } else {
            for( int i=0 ; i<size ; i++ ) // loop over outputs
            {
                if(!fast_exact_is_equal(target[i],0.0))
                    // nll -= target[i] * pl_log(expectations[i]); 
                    // but it is numerically unstable, so use instead
                    // log (1/(1+exp(-x))) = -log(1+exp(-x)) = -softplus(-x)
                    nll += target[i] * softplus(-activation[i]);
                if(!fast_exact_is_equal(target[i],1.0))
                    // nll -= (1-target[i]) * pl_log(1-output[i]);
                    // log (1 - 1/(1+exp(-x))) = log(exp(-x)/(1+exp(-x)))
                    //                         = log(1/(1+exp(x)))
                    //                         = -log(1+exp(x))
                    //                         = -softplus(x)
                    nll += (1-target[i]) * softplus(activation[i]);
            }
        }
        costs_column(k,0) = nll;
    }
}

void RBMWoodsLayer::bpropNLL(const Vec& target, real nll, Vec& bias_gradient)
{
    PLERROR( "RBMWoodsLayer::bpropNLL(): not implemeted yet" );
    computeExpectation();

    PLASSERT( target.size() == input_size );
    bias_gradient.resize( size );

    // bias_gradient = expectation - target
    substract(expectation, target, bias_gradient);
}

void RBMWoodsLayer::bpropNLL(const Mat& targets, const Mat& costs_column,
                                Mat& bias_gradients)
{
    PLERROR( "RBMWoodsLayer::bpropNLL(): not implemeted yet" );
    computeExpectations();

    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );
    bias_gradients.resize( batch_size, size );

    // bias_gradients = expectations - targets
    substract(expectations, targets, bias_gradients);

}

void RBMWoodsLayer::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_trees", &RBMWoodsLayer::n_trees,
                  OptionBase::buildoption,
                  "Number of trees in the woods.");

    declareOption(ol, "tree_depth", &RBMWoodsLayer::tree_depth,
                  OptionBase::buildoption,
                  "Depth of the trees in the woods (1 gives the ordinary "
                  "RBMBinomialLayer).");

    declareOption(ol, "use_signed_samples", &RBMWoodsLayer::use_signed_samples,
                  OptionBase::buildoption,
                  "Indication that samples should be in {-1,1}, not {0,1}.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMWoodsLayer::build_()
{
    PLASSERT( n_trees > 0 );
    PLASSERT( tree_depth > 0 );

    if ( tree_depth < 2 )
        PLERROR("RBMWoodsLayer::build_(): tree_depth < 2 not supported, use "
                "RBMBinomialLayer instead.");

    size = n_trees * ( ipow( 2, tree_depth ) - 1 );
    local_node_expectation.resize( size );
    on_free_energy.resize( size );
    off_free_energy.resize( size );
    off_expectation.resize( size );
    local_node_expectation_gradient.resize( size );
    on_tree_gradient.resize( size );
    off_tree_gradient.resize( size );
    on_free_energy_gradient.resize( size );
    off_free_energy_gradient.resize( size );

    // Must call parent's build, since size was just set
    inherited::build();
}

void RBMWoodsLayer::build()
{
    inherited::build();
    build_();
}


void RBMWoodsLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField( off_expectation, copies );
    deepCopyField( local_node_expectation, copies );
    deepCopyField( on_free_energy, copies );
    deepCopyField( off_free_energy, copies );
    deepCopyField( local_node_expectation_gradient, copies );
    deepCopyField( on_tree_gradient, copies );
    deepCopyField( off_tree_gradient, copies );
    deepCopyField( on_free_energy_gradient, copies );
    deepCopyField( off_free_energy_gradient, copies );
}

real RBMWoodsLayer::energy(const Vec& unit_values) const
{
    PLERROR( "RBMWoodsLayer::energy(): not implemeted yet" );
    return -dot(unit_values, bias);
}

real RBMWoodsLayer::freeEnergyContribution(const Vec& unit_activations)
    const
{
    PLERROR( "RBMWoodsLayer::freeEnergyContribution(): not implemeted yet" );
    PLASSERT( unit_activations.size() == size );

    // result = -\sum_{i=0}^{size-1} softplus(a_i)
    real result = 0;
    real* a = unit_activations.data();
    for (int i=0; i<size; i++)
    {
        if (use_fast_approximations)
            result -= tabulated_softplus(a[i]);
        else
            result -= softplus(a[i]);
    }
    return result;
}

int RBMWoodsLayer::getConfigurationCount()
{
    PLERROR( "RBMWoodsLayer::getConfigurationCount(): not implemeted yet" );
    return size < 31 ? 1<<size : INFINITE_CONFIGURATIONS;
}

void RBMWoodsLayer::getConfiguration(int conf_index, Vec& output)
{
    PLERROR( "RBMWoodsLayer::getConfigurationCount(): not implemeted yet" );
    PLASSERT( output.length() == size );
    PLASSERT( conf_index >= 0 && conf_index < getConfigurationCount() );

    for ( int i = 0; i < size; ++i ) {
        output[i] = conf_index & 1;
        conf_index >>= 1;
    }
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
