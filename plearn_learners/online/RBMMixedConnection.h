// -*- C++ -*-

// RBMMixedConnection.h
//
// Copyright (C) 2006 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file RBMMixedConnection.h */


#ifndef RBMMixedConnection_INC
#define RBMMixedConnection_INC

#include "RBMConnection.h"

namespace PLearn {
using namespace std;


/**
 * Contains a matrix of other RBMConnections, acting as submatrix of the linear
 * transformation this class computes. If a PP<RBMConnection> is null, it will
 * be considered as a 0-filled matrix.
 *
 * @todo: yes
 */
class RBMMixedConnection: public RBMConnection
{
    typedef RBMConnection inherited;

public:
    //#####  Public Build Options  ############################################

    //#####  Learned Options  #################################################

    //! Matrix containing the sub-transformations (blocks)
    TMat< PP<RBMConnection> > sub_connections;

    //#####  Not Options  #####################################################

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMMixedConnection();

    // Your other public member functions go here

    //! Sets the learning rate, also in the sub_connections
    virtual void setLearningRate( real the_learning_rate );
    //! Sets the momentum, also in the sub_connections
    virtual void setMomentum( real the_momentum );

    virtual void setAsUpInput( const Vec& input ) const;
    virtual void setAsDownInput( const Vec& input ) const;

    virtual void setAsUpInputs( const Mat& inputs ) const;

    virtual void setAsDownInputs( const Mat& input ) const;

    //! Accumulates positive phase statistics to *_pos_stats
    virtual void accumulatePosStats( const Vec& down_values,
                                     const Vec& up_values );

    virtual void accumulatePosStats( const Mat& down_values,
                                     const Mat& up_values )
    {
        PLASSERT_MSG( false, "Not implemented" );
    }

    //! Accumulates negative phase statistics to *_neg_stats
    virtual void accumulateNegStats( const Vec& down_values,
                                     const Vec& up_values );

    virtual void accumulateNegStats( const Mat& down_values,
                                     const Mat& up_values )
    {
        PLASSERT_MSG( false, "Not implemented" );
    }

    //! Updates parameters according to contrastive divergence gradient
    virtual void update();

    //! Updates parameters according to contrastive divergence gradient,
    //! not using the statistics but the explicit values passed
    virtual void update( const Vec& pos_down_values,
                         const Vec& pos_up_values,
                         const Vec& neg_down_values,
                         const Vec& neg_up_values );

    //! Not implemented.
    virtual void update( const Mat& pos_down_values,
                         const Mat& pos_up_values,
                         const Mat& neg_down_values,
                         const Mat& neg_up_values);

    //! Clear all information accumulated during stats
    virtual void clearStats();

    //! Computes the vectors of activation of "length" units,
    //! starting from "start", and stores them into "activations".
    //! "start" indexes an up unit if "going_up", else a down unit.
    virtual void computeProduct( int start, int length,
                                 const Vec& activations,
                                 bool accumulate=false ) const;

    //! Same as 'computeProduct' but for mini-batches.
    virtual void computeProducts(int start, int length,
                                 Mat& activations,
                                 bool accumulate=false ) const;

    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop; it should be
    //! called with the same arguments as fprop for the first two arguments
    //! (and output should not have been modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
    //! JUST CALLS
    //!     bpropUpdate(input, output, input_gradient, output_gradient)
    //! AND IGNORES INPUT GRADIENT.
    // virtual void bpropUpdate(const Vec& input, const Vec& output,
    //                          const Vec& output_gradient);

    //! this version allows to obtain the input gradient as well
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient,
                             const Vec& output_gradient,
                             bool accumulate=false);

    //! Batch version
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate=false);

    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();

    //! optionally perform some processing after training, or after a
    //! series of fprop/bpropUpdate calls to prepare the model for truly
    //! out-of-sample operation.  THE DEFAULT IMPLEMENTATION PROVIDED IN
    //! THE SUPER-CLASS DOES NOT DO ANYTHING.
    // virtual void finalize();

    //! return the number of parameters
    virtual int nParameters() const;

    //! Make the parameters data be sub-vectors of the given global_parameters.
    //! The argument should have size >= nParameters. The result is a Vec
    //! that starts just after this object's parameters end, i.e.
    //!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
    //! This allows to easily chain calls of this method on multiple RBMParameters.
    virtual Vec makeParametersPointHere(const Vec& global_parameters);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RBMMixedConnection);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //! Initial vertical index of the blocks
    TVec<int> up_init_positions;

    //! Vertical sizes of the blocks
    TVec<int> up_block_sizes;

    //! Initial horizontal index of the blocks
    TVec<int> down_init_positions;

    //! Horizontal sizes of the blocks
    TVec<int> down_block_sizes;

    TVec<int> row_of;
    TVec<int> col_of;

    //! sub_connections.length()
    int n_up_blocks;

    //! sub_connections.width()
    int n_down_blocks;


protected:
    //#####  Protected Member Functions  ######################################
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMMixedConnection);

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
