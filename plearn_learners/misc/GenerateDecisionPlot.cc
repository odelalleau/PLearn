
// -*- C++ -*-

// GenerateDecisionPlot.cc
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
   * $Id: GenerateDecisionPlot.cc,v 1.1 2003/05/26 04:12:43 plearner Exp $ 
   ******************************************************* */

/*! \file GenerateDecisionPlot.cc */
#include "GenerateDecisionPlot.h"
#include "VecStatsCollector.h"
#include "VMat_maths.h"

namespace PLearn <%
using namespace std;

//! considers data to have 2d input (first 2 columns)
//! 
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

/*!
If fieldnames is omitted then the fields will be named basename_0 basename_1 ...
Otherwise they are named basename_ followed by the corresponding field name.
*/
void DX_write_2D_fields(ostream& out, const string& basename, TVec<Mat> fields, real x0, real y0, real deltax, real deltay, 
                        TVec<string> fieldnames=TVec<string>())
{
  int nfields = fields.length();
  int nx = fields[0].length();
  int ny = fields[0].width();

  out << "object \"" << basename << "_gridpos\" class gridpositions counts " << nx << " " << ny << "\n"
      << "origin  " << x0 << " " << y0 << "\n"
      << "delta   " << deltax << " 0 \n"
      << "delta    0 " << deltay << " \n\n\n";

  out << "object \"" << basename << "_gridcon\" class gridconnections counts " << nx << " " << ny << "\n"
      << "attribute \"element type\" string \"cubes\" \n"
      << "attribute \"ref\" string \"positions\" \n\n\n";

  for(int k=0; k<nfields; k++)
    {
      Mat& m = fields[k];
      string fieldname = tostring(k);
      if(fieldnames)
        fieldname = fieldnames[k];

      string dataname = basename + "_" + fieldname + "_data";

      out << "object \"" << dataname << "\" class array type float rank 0 items " << nx*ny << " data follows \n";
      for(int i=0; i<nx; i++)
        {
          for(int j=0; j<ny; j++)
            out << m(i,j) << " ";
          out << "\n";
        }
      out << "attribute \"dep\" string \"positions\" \n\n\n";

      out << "object \"" << fieldname << "\" class field \n"
          << "component \"positions\" \"" << basename << "_gridpos\" \n"
          << "component \"connections\" \"" << basename << "_gridcon\" \n"
          << "component \"data\" \"" << dataname << "\" \n\n\n";
    }
}

TVec<Mat> computeOutputFields(PP<PLearner> learner, int nx, int ny, real x0, real y0, real deltax, real deltay)
{
  int noutputs = learner->outputsize();
  int nfields = 1;
  if(noutputs>1)  // make additional first 2 fields: argmax and max
    nfields = 2+noutputs;

  TVec<Mat> fields(nfields);
  for(int k=0; k<nfields; k++)
    fields[k].resize(nx,ny);

  Vec input(2);
  Vec output(noutputs);

  ProgressBar pb("Computing " + tostring(nx) + " x " + tostring(ny) + " output field",nx*ny);

  real x = x0;
  real y = y0;
  for(int i=0; i<nx; i++, x+=deltax)
    for(int j=0; j<ny; j++, y+=deltay)
      {
        input[0] = x;
        input[1] = y;
        learner->computeOutput(input,output);
        // cerr << "in: " << input << " out: " << output << endl;
        if(noutputs==1)
          fields[0](i,j) = output[0];
        else
          {            
            fields[0](i,j) = argmax(output);
            fields[1](i,j) = max(output);
            for(int k=0; k<noutputs; k++)
              fields[2+k](i,j) = output[k];
          }
        pb.update(i*nx+j);
      }

  return fields;
}

// Finds appropriate x0, y0, deltax, deltay from the dataset range, computes the fields and returns them
// extraspace of .10 means we'll look 10% beyond the data range on every side
TVec<Mat> computeOutputFieldsAutoRange(PP<PLearner> learner, VMat dataset, int nx, int ny, 
                                       real& x0, real& y0, real& deltax, real& deltay, real extraspace=.10)
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
  return computeOutputFields(learner, nx, ny, x0, y0, deltax, deltay);
}


void DX_create_decision_data_file(const string& filename, PP<PLearner> learner, VMat dataset, int nx, int ny)
{
  ofstream out(filename.c_str());

  int l = dataset.length();
  int w = dataset.width();
  int inputsize = learner->inputsize();
  int outputsize = learner->outputsize();
  Mat data(l, w+outputsize);
  data.subMatColumns(0,w) << dataset;
  for(int i=0; i<l; i++)
    {
      Vec row = data(i);
      Vec input = row.subVec(0,inputsize);
      Vec output = row.subVec(w,outputsize);
      learner->computeOutput(input, output);
    }
  DX_write_2D_data(out, "dset", data);

  real x0, y0, deltax, deltay;
  TVec<Mat> fields = computeOutputFieldsAutoRange(learner, dataset, nx, ny, x0, y0, deltax, deltay);
  int nfields = fields.length();
  TVec<string> fieldnames(nfields);
  if(nfields==1)
    fieldnames[0] = "output";
  else 
    {
      fieldnames[0] = "argmax";
      fieldnames[1] = "max";
      for(int k=2; k<nfields; k++)
        fieldnames[k] = "output" + tostring(k-2);
    }

  DX_write_2D_fields(out, "decision", fields, x0, y0, deltax, deltay, fieldnames);
  out << "end" << endl;
}

GenerateDecisionPlot::GenerateDecisionPlot() 
  :dxfilename("decisionsurf.dx"),
   nx(10), ny(10)
  {
  }

  PLEARN_IMPLEMENT_OBJECT_METHODS(GenerateDecisionPlot, "GenerateDecisionPlot", Object);

  void GenerateDecisionPlot::declareOptions(OptionList& ol)
  {
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "dxfilename", &GenerateDecisionPlot::dxfilename, OptionBase::buildoption,
                   "Name of the .dx data file to generate");
    declareOption(ol, "dataset", &GenerateDecisionPlot::dataset, OptionBase::buildoption,
                   "Dataset to train the learner on, and to include in the generated file");
    declareOption(ol, "learner", &GenerateDecisionPlot::learner, OptionBase::buildoption,
                   "The learner to train/test");
    declareOption(ol, "nx", &GenerateDecisionPlot::nx, OptionBase::buildoption,
                   "Number of x sample coordinates (grid)");
    declareOption(ol, "ny", &GenerateDecisionPlot::ny, OptionBase::buildoption,
                   "Number of y sample coordinates (grid)");
    declareOption(ol, "save_learner_as", &GenerateDecisionPlot::save_learner_as, OptionBase::buildoption,
                   "(Optionally) save trained learner in this file (.psave)");


    // ### ex:
    // declareOption(ol, "myoption", &GenerateDecisionPlot::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  string GenerateDecisionPlot::help()
  {
    // ### Provide some useful description of what the class is ...
    return 
      "GenerateDecisionPlot implements a ...\n";
  }

  void GenerateDecisionPlot::build_()
  {
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
  }

  //! Overload this for runnable objects (default method issues a runtime error)
void GenerateDecisionPlot::run()
{
  if(!dataset)
    PLERROR("Must specify a dataset");
  learner->setTrainingSet(dataset);
  VecStatsCollector st;
  learner->train(st);
  if(save_learner_as!="")
    {
      cerr << "Saving trained learner in file " << save_learner_as << endl;
      PLearn::save(save_learner_as, *learner);
    }
  DX_create_decision_data_file(dxfilename, learner, dataset, nx, ny);
  cerr << "Writing output data to file " << dxfilename << endl; 
  cerr << "You can now view it with OpenDX." << endl;
}


  // ### Nothing to add here, simply calls build_
  void GenerateDecisionPlot::build()
  {
    inherited::build();
    build_();
  }


  void GenerateDecisionPlot::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    inherited::makeDeepCopyFromShallowCopy(copies);
  }

%> // end of namespace PLearn
