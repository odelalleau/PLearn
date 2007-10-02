// -*- C++ -*-

// VMatKernel.cc
//
// Copyright (C) 2005 Benoit Cromp
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

// Authors: Benoit Cromp, Jerome Louradour

/*! \file VMatKernel.cc */


#include "VMatKernel.h"

namespace PLearn {
using namespace std;

//////////////////
// VMatKernel //
//////////////////
// ### Initialize all fields to their default value here
VMatKernel::VMatKernel() 
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(VMatKernel,
                        "Kernel that is given its Gram matrix.",
                        "This kernel can only be applied on examples that are integers, and that\n"
                        "correspond to indices in the matrix.\n"
    );

////////////////////
// declareOptions //
////////////////////
void VMatKernel::declareOptions(OptionList& ol)
{
    declareOption(ol,"source",&VMatKernel::source,
                  OptionBase::buildoption,
        "Gram matrix");

    declareOption(ol,"train_indices",&VMatKernel::train_indices,
                  OptionBase::learntoption,
        "List of (real)indices corresponding to training samples");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void VMatKernel::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void VMatKernel::build_()
{
    PLASSERT( !(source) || ( source->length() == source->width() ) );
    if ( !specify_dataset )
        train_indices.resize(0);
}

//////////////
// evaluate //
//////////////
real VMatKernel::evaluate(const Vec& x1, const Vec& x2) const
{
    PLASSERT( x1.size()==1 && x2.size()==1 );
    return evaluate( x1[0], x2[0] );
}

real VMatKernel::evaluate(real x1, real x2) const
{
    PLASSERT( fabs(x1-(real)((int)x1)) < 0.1 );
    PLASSERT( fabs(x2-(real)((int)x2)) < 0.1 );
    return evaluate( int(x1), int(x2) );
}

real VMatKernel::evaluate(int x1, int x2) const
{
    PLASSERT( source );
    PLASSERT( x1 >= 0 );
    PLASSERT( x1 < source->length() );
    PLASSERT( x2 >= 0 );
    PLASSERT( x2 < source->width() );
    return source->get( x1, x2);
}

//////////////////
// evaluate_i_j //
//////////////////
real VMatKernel::evaluate_i_j(int i, int j) const
{
    PLASSERT( source );
    if( train_indices.length() == 0 )
        return evaluate( i, j );
    PLASSERT( i >= 0 );
    PLASSERT( i < n_examples );
    PLASSERT( j >= 0 );
    PLASSERT( j < n_examples );
    return evaluate( train_indices[i], train_indices[j] );
}

//////////////////
// evaluate_i_x //
//////////////////
real VMatKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const
{
    if( train_indices.length() == 0 )
        return evaluate( i, (int)x[0] );
    PLASSERT( i >= 0 );
    PLASSERT( i < n_examples );
    PLASSERT( x.size() == 1 );
    return evaluate( train_indices[i], x[0] );
}

//////////////////
// evaluate_x_i //
//////////////////
real VMatKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
{
//    if( is_symmetric )
//        return evaluate_i_x( i, x, squared_norm_of_x);
    if( train_indices.length() == 0 )
        return evaluate( (int)x[0], i);
    PLASSERT( i >= 0 );
    PLASSERT( i < n_examples );
    PLASSERT( x.size() == 1 );
    return evaluate( x[0], train_indices[i]);
}

///////////////////////
// computeGramMatrix //
///////////////////////
void VMatKernel::computeGramMatrix(Mat K) const
{
    PLASSERT( source );
    if( train_indices.length() > 0 )
    {
        K.resize(n_examples, n_examples);
        if( is_symmetric )
            for(int i = 0; i < n_examples; i++ )
	    {
	        K(i,i) = evaluate( train_indices[i], train_indices[i] );
                for(int j = 0; j < i; j++ )
                {
	            K(i,j) = evaluate( train_indices[i], train_indices[j] );
		    K(j,i) = K(i,j);
	        }
            }
        else
            for(int i = 0; i < n_examples; i++ )
                for(int j = 0; j < n_examples; j++ )
	            K(i,j) = evaluate( train_indices[i], train_indices[j] );
    }
    else
        K << source->toMat();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void VMatKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(source, copies);
    deepCopyField(train_indices, copies);
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void VMatKernel::setDataForKernelMatrix(VMat the_data)
{
    inherited::setDataForKernelMatrix(the_data);

    if( n_examples > 1 )
    {
        PLASSERT( data_inputsize == 1 );
        train_indices.resize(n_examples);
        for(int i = 0; i < n_examples; i++)
        {
            PLASSERT( the_data->get(i,0) >= 0 );
            PLASSERT( !(source) || ( the_data->get(i,0) < (real)source->width() ) );
            train_indices[i] = the_data->get(i,0);
        }
    }
    else
    {
	PLASSERT( source );
        PLWARNING("in VMatKernel::setDataForKernelMatrix: all values in the VMatKernel source are taken into acount for training");
	n_examples = source->width();
	train_indices.resize(0);
    }
}

////////////////////////////
// addDataForKernelMatrix //
////////////////////////////
void VMatKernel::addDataForKernelMatrix(const Vec& newRow)
{
    PLASSERT( newRow.size() == 1 );
    inherited::addDataForKernelMatrix( newRow );
    if( train_indices.length() == 0 )
    {
        PLASSERT( source );
	n_examples = source->width();
        train_indices.resize( n_examples );
        for(int i = 0; i < n_examples; i++)
	    train_indices[i] = (real)i;
    }
    PLASSERT( newRow[0] > 0 );
    PLASSERT( !(source) || ( newRow[0] < source->width() ) );
    train_indices.resize( n_examples + 1 );
    train_indices[ n_examples ] = newRow[0];
    n_examples += 1;
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
