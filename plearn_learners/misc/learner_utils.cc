
// -*- C++ -*-

// learner_utils.cc
//
// Copyright (C) 2003  Pascal Vincent 
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

/*! \file learner_utils.cc */
#include "learner_utils.h"
#include <plearn/vmat/VMat_basic_stats.h>

namespace PLearn {
using namespace std;


Mat compute_learner_outputs(PP<PLearner> learner, VMat inputs)
{
    Vec input(learner->inputsize());
    int l = inputs.length();
    Mat outputs(l,learner->outputsize());
    for(int i=0; i<l; i++)
    {
        inputs->getRow(i,input);
        Vec output = outputs(i);
        learner->computeOutput(input, output);
    }
    return outputs;
}


// Finds appropriate x0, y0, deltax, deltay from the dataset range,
// extraspace of .10 means we'll look 10% beyond the data range on every side
void determine_grid_for_dataset(VMat dataset, int nx, int ny, 
                                real& x0, real& y0, real& deltax, real& deltay, 
                                real extraspace)
{
    Vec minv(2);
    Vec maxv(2);
    computeRange(dataset.subMatColumns(0,2), minv, maxv);
    real extrax = (maxv[0]-minv[0])*extraspace;
    x0 = minv[0]-extrax;
    deltax = (maxv[0]+extrax-x0)/nx;
    real extray = (maxv[1]-minv[1])*extraspace;
    y0 = minv[1]-extray;
    deltay = (maxv[1]+extray-y0)/ny;
}

double determine_density_integral_from_log_densities_on_grid(Vec log_densities, real deltax, real deltay)
{
    double logsum = logadd(log_densities);
    double surfelem = deltax*deltay;
    double surfintegral = exp(logsum)*surfelem;
    return surfintegral;
}

Mat compute_learner_outputs_on_grid(PP<PLearner> learner, int nx, int ny, real x0, real y0, real deltax, real deltay)
{
    if(learner->inputsize()!=2)
        PLERROR("In compute_learner_outputs_on_grid learner's input must be 2D");

    int noutputs = learner->outputsize();
    Vec input(2);
    Mat results(nx*ny, noutputs);

    ProgressBar pb("Computing " + tostring(nx) + " x " + tostring(ny) + " learner outputs",nx*ny);

    real x = x0;
    for(int i=0; i<nx; i++, x+=deltax)
    {
        real y = y0;
        for(int j=0; j<ny; j++, y+=deltay)
        {
            input[0] = x;
            input[1] = y;
            Vec output = results(i*nx+j);
            learner->computeOutput(input,output);
            // cerr << input << " --> " << output << endl;
            pb.update(i*nx+j);
        }
    }

    return results;
}

void DX_write_2D_data(ostream& out, const string& basename, Mat data)
{
    int l = data.length();
    int nvals = data.width()-2;

    // write 2D positions
    out << "object \"" << basename << "_pos\" class array type float rank 1 shape 2 items " << l << " data follows \n";
    for(int i=0; i<l; i++)
        out << data(i,0) << " " << data(i,1) << "\n";
    out << "\n\n\n";

    // Write data, which is in a one-to-one correspondence with the positions
    if(nvals==1)  // scalar field
    {
        out << "object \"" << basename << "_value\" class array type float rank 0 items " << l << " data follows \n";
        for(int i=0; i<l; i++)
            out << data(i,2) << "\n";
        out << "attribute \"dep\" string \"positions\" \n\n\n";
    }
    else if(nvals>1) // vector field
    {
        out << "object \"" << basename << "_value\" class array type float rank 1 shape " << nvals << " items " << l << " data follows \n";
        for(int i=0; i<l; i++)
        {
            for(int j=0; j<nvals; j++)
                out << data(i,2+j) << " ";
            out << "\n";
        }
        out << "attribute \"dep\" string \"positions\" \n\n\n";
    }

    // Finally field is created with two components: "positions" and "data"
    out << "object \"" << basename << "\" class field \n"
        << "component \"positions\" \"" << basename << "_pos\" \n";
    if(nvals>0)
        out << "component \"data\" \"" << basename << "_value\" \n";

    out << "\n\n";
}


void DX_write_2D_data_for_grid(ostream& out, const string& basename, 
                               int nx, int ny, real x0, real y0, real deltax, real deltay,
                               Mat data)
{
    int l = data.length();
    int nvals = data.width();

    string posname = string("\"") + basename + "_gridpos\"";
    out << "object " << posname << " class gridpositions counts " << nx << " " << ny << "\n"
        << "origin  " << x0 << " " << y0 << "\n"
        << "delta   " << deltax << " 0 \n"
        << "delta    0 " << deltay << " \n\n\n";

    string conname = string("\"") + basename + "_gridcon\"";
    out << "object " << conname << " class gridconnections counts " << nx << " " << ny << "\n"
        //      << "attribute \"element type\" string \"cubes\" \n"
        << "attribute \"ref\" string \"positions\" \n\n\n";

    string dataname = string("\"") + basename + "_values\"";
    // Write data, which is in a one-to-one correspondence with the positions
    if(nvals==1)  // scalar field
    {
        out << "object " << dataname << " class array type float rank 0 items " << l << " data follows \n";
        for(int i=0; i<l; i++)
            out << data(i,0) << "\n";
        out << "attribute \"dep\" string \"positions\" \n\n\n";
    }
    else if(nvals>1) // vector field
    {
        out << "object " << dataname << " class array type float rank 1 shape " << nvals << " items " << l << " data follows \n";
        for(int i=0; i<l; i++)
        {
            for(int j=0; j<nvals; j++)
                out << data(i,j) << " ";
            out << "\n";
        }
        out << "attribute \"dep\" string \"positions\" \n\n\n";
    }

    // Finally field is created with 3 components: "positions" "connections" and "data"
    out << "object \"" << basename << "\" class field \n"
        << "component \"positions\" " << posname << " \n"
        << "component \"connections\" " << conname << " \n"
        << "component \"data\" " << dataname << " \n\n\n";

    out << "\n\n";
}


void DX_save_2D_data(const string& filename, const string& basename, Mat data)
{
    ofstream out(filename.c_str());
    if(!out)
        PLERROR("Could not open %s for writing",filename.c_str());
    DX_write_2D_data(out, basename, data);
}

void DX_save_2D_data_for_grid(const string& filename, const string& basename, 
                              int nx, int ny, real x0, real y0, real deltax, real deltay,
                              Mat data)
{
    ofstream out(filename.c_str());
    if(!out)
        PLERROR("Could not open %s for writing",filename.c_str());
    DX_write_2D_data_for_grid(out, basename, nx, ny, x0, y0, deltax, deltay, data); 
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
