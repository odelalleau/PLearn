
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
   * $Id: GenerateDecisionPlot.cc,v 1.4 2003/06/04 02:56:37 plearner Exp $ 
   ******************************************************* */

/*! \file GenerateDecisionPlot.cc */
#include "GenerateDecisionPlot.h"
#include "VecStatsCollector.h"
#include "VMat_maths.h"

namespace PLearn <%
using namespace std;


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

  string posname = string("\"") + basename + "_gridpos\"";

  out << "object " << posname << " class gridpositions counts " << nx << " " << ny << "\n"
      << "origin  " << x0 << " " << y0 << "\n"
      << "delta   " << deltax << " 0 \n"
      << "delta    0 " << deltay << " \n\n\n";

  string conname = string("\"") + basename + "_gridcon\"";

  out << "object " << conname << " class gridconnections counts " << nx << " " << ny << "\n"
    //      << "attribute \"element type\" string \"cubes\" \n"
      << "attribute \"ref\" string \"positions\" \n\n\n";

  for(int k=0; k<nfields; k++)
    {
      Mat& m = fields[k];
      string fieldname = tostring(k);
      if(fieldnames)
        fieldname = fieldnames[k];

      string dataname = string("\"") + basename + "_" + fieldname + "_data\"";

      out << "object " << dataname << " class array type float rank 0 items " << nx*ny << " data follows \n";
      for(int i=0; i<nx; i++)
        {
          for(int j=0; j<ny; j++)
            out << m(i,j) << " ";
          out << "\n";
        }
      out << "attribute \"dep\" string \"positions\" \n\n\n";

      out << "object \"" << fieldname << "\" class field \n"
          << "component \"positions\" " << posname << " \n"
          << "component \"connections\" " << conname << " \n"
          << "component \"data\" " << dataname << " \n\n\n";
    }
}


void DX_write_2D_fields(ostream& out, const string& basename, Vec X, Vec Y, TVec<Mat> fields)
{
  int nfields = fields.length();
  int nx = fields[0].length();
  int ny = fields[0].width();

  /*
  out << "object \"" << basename << "_X\" class array type float rank 0 items " << nx << " data follows \n";
  for(int i=0; i<nx; i++)
    out << X[i] << "\n";
  out << "\n\n";
    
  out << "object \"" << basename << "_Y\" class array type float rank 0 items " << ny << " data follows \n";
  for(int i=0; i<ny; i++)
    out << Y[i] << "\n";
  */

  string posname = string("\"") + basename + "_gridpos\"";
  out << "object " << posname << " class array type float rank 1 shape 2 items " << nx*ny << " data follows\n";
  for(int i=0; i<nx; i++)
    for(int j=0; j<ny; j++)
      out << X[i] << " " << Y[j] << "\n";
  out << "\n\n";

  string conname = string("\"") + basename + "_gridcon\"";
  out << "object " << conname << " class gridconnections counts " << nx << " " << ny << "\n"
    //      << "attribute \"element type\" string \"cubes\" \n"
      << "attribute \"ref\" string \"positions\" \n\n\n";

  for(int k=0; k<nfields; k++)
    {
      Mat& m = fields[k];
      string fieldname = "output" + tostring(k);
      string dataname = string("\"") + basename + "_" + fieldname + "_data\"";

      out << "object " << dataname << " class array type float rank 0 items " << nx*ny << " data follows \n";
      for(int i=0; i<nx; i++)
        {
          for(int j=0; j<ny; j++)
            out << m(i,j) << " ";
          out << "\n";
        }
      out << "attribute \"dep\" string \"positions\" \n\n\n";

      out << "object \"" << fieldname << "\" class field \n"
          << "component \"positions\" " << posname << " \n"
          << "component \"connections\" " << conname << " \n"
          << "component \"data\" " << dataname << " \n\n\n";
    }
}


TVec<Mat> computeOutputFields(PP<PLearner> learner, Vec X, Vec Y)
{
  int noutputs = learner->outputsize();

  int nx = X.length();
  int ny = Y.length();
  int nfields = noutputs;
  TVec<Mat> fields(nfields);

  for(int k=0; k<nfields; k++)
    fields[k].resize(nx,ny);

  Vec input(2);
  Vec output(noutputs);

  ProgressBar pb("Computing " + tostring(nx) + " x " + tostring(ny) + " output field",nx*ny);
  
  for(int i=0; i<nx; i++)
    for(int j=0; j<ny; j++)
      {
        input[0] = X[i];
        input[1] = Y[j];
        learner->computeOutput(input,output);
        // cerr << "in: " << input << " out: " << output << endl;
        for(int k=0; k<noutputs; k++)
          fields[k](i,j) = output[k];
        pb.update(i*nx+j);
      }

  return fields;
}


TVec<Mat> computeOutputFields(PP<PLearner> learner, int nx, int ny, real x0, real y0, real deltax, real deltay)
{
  int noutputs = learner->outputsize();
  int nfields = noutputs;

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
        for(int k=0; k<noutputs; k++)
          fields[k](i,j) = output[k];
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


void computeXYPositions(VMat dataset, int nx, int ny, Vec& X, Vec& Y, real extraspace=.10)
{
  Vec minv(2);
  Vec maxv(2);
  computeRange(dataset.subMatColumns(0,2), minv, maxv);
  real extrax = (maxv[0]-minv[0])*extraspace;
  real x0 = minv[0]-extrax;
  real deltax = (maxv[0]+extrax-x0)/nx;
  real extray = (maxv[1]-minv[1])*extraspace;
  real y0 = minv[1]-extray;
  real deltay = (maxv[1]+extray-y0)/ny;

  set<real> xpos;
  set<real> ypos;
  int l = dataset.length();
  Vec datapoint(2);
  for(int i=0; i<l; i++)
    {
      dataset->getRow(i,datapoint);
      xpos.insert(datapoint[0]);
      ypos.insert(datapoint[1]);
    }
  real x = x0;
  for(int i=0; i<nx; i++, x+=deltax)
    xpos.insert(x);
  real y = y0;
  for(int j=0; j<ny; j++, y+=deltay)
    ypos.insert(y);
  set<real>::iterator it;
  X.resize(xpos.size());
  real* xptr = X.data();
  it = xpos.begin();
  while(it!=xpos.end())
    *xptr++ = *it++;
  Y.resize(ypos.size());
  real* yptr = Y.data();
  it = ypos.begin();
  while(it!=ypos.end())
    *yptr++ = *it++;
}



//! Will write a file containing a field with the dataset positions
//! "dset" field will be input -> target, outputs
void DX_create_dataset_outputs_file(const string& filename, PP<PLearner> learner, VMat dataset)
{
  ofstream out(filename.c_str());

  int l = dataset.length();
  int inputsize = learner->inputsize();
  int targetsize = learner->targetsize();
  int outputsize = learner->outputsize();

  // First write data points (input -> target, output)
  Vec input(inputsize);
  Vec target(targetsize);
  real weight;
  Vec output(outputsize);

  // write 2D positions
  out << "object \"dset_pos\" class array type float rank 1 shape " << inputsize << " items " << l << " data follows \n";
  for(int i=0; i<l; i++)
    {
      dataset->getExample(i,input,target,weight);
      for(int j=0; j<inputsize; j++)
        out << input[j] << " ";
      out << "\n";
    }
  out << "\n\n\n";

  // Now write data for those positions (target and output)
  if(targetsize+outputsize>0)
    {
      ProgressBar pb("Computing outputs for dataset points",l);
      out << "object \"dset_value\" class array type float rank 1 shape " << targetsize+outputsize << " items " << l << " data follows \n";
      for(int i=0; i<l; i++)
        {
          dataset->getExample(i,input,target,weight);
          for(int j=0; j<targetsize; j++)
            out << target[j] << " ";
          learner->computeOutput(input, output);
          for(int j=0; j<outputsize; j++)
            out << output[j] << " ";
          out << "\n";
          pb.update(i);
        }
      out << "attribute \"dep\" string \"positions\" \n\n\n";
    }

  // Field is created with two components: "positions" and "data"
  out << "object \"dset\" class field \n"
      << "component \"positions\" \"dset_pos\" \n";
  if(targetsize+outputsize>0)
    out << "component \"data\" \"dset_value\" \n";
  out << "\n\n\n";


  
  out << "end" << endl;
}


//! The "outputs" field will contain sample-grid inputs -> outputs
//! Where the sample grid is made of a regular grid of nx.ny points (in the range [xmin, xmax] x [ymin, ymax])
//! xmin, xmax, ymin and ymax may be left to MISSING_VALUE, in which case an automatic range will be determined
//! from the range of the points in the given dataset extended by extraspace (ex: .10 == 10%).
//! This regular grid is possibly complemented (if include_datapoint_grid) with an irregular grid 
//! made of the x and y coordinates of the dataset that fall within the [xmin, xmax] x [ymin, ymax] range.

void DX_create_grid_outputs_file(const string& filename, PP<PLearner> learner, VMat dataset, 
                                  int nx, int ny, bool include_datapoint_grid=false, 
                                  real xmin=MISSING_VALUE, real xmax=MISSING_VALUE, 
                                  real ymin=MISSING_VALUE, real ymax=MISSING_VALUE,
                                  real extraspace=.10)
{
  ofstream out(filename.c_str());

  double logsum = -FLT_MAX;

  int l = dataset.length();
  int inputsize = learner->inputsize();
  int targetsize = learner->targetsize();
  int outputsize = learner->outputsize();

  Vec input(inputsize);
  Vec target(targetsize);
  real weight;
  Vec output(outputsize);

  // Create the grid field

  set<real> xpos;
  set<real> ypos;

  // First the regular grid coordinates
  Vec minv(2);
  Vec maxv(2);
  computeRange(dataset.subMatColumns(0,2), minv, maxv);
  real extrax = (maxv[0]-minv[0])*extraspace;
  real extray = (maxv[1]-minv[1])*extraspace;
  if(is_missing(xmin))
    xmin = minv[0]-extrax;
  if(is_missing(xmax))
    xmax = maxv[0]+extrax;
  if(is_missing(ymin))
    ymin = minv[1]-extray;
  if(is_missing(ymax))
    ymax = maxv[1]+extray;
  real deltax = (xmax-xmin)/nx;
  real deltay = (ymax-ymin)/ny;

  real x = xmin;
  for(int i=0; i<nx; i++, x+=deltax)
    xpos.insert(x);
  real y = ymin;
  for(int j=0; j<ny; j++, y+=deltay)
    ypos.insert(y);

  // also include irregular grid coordinates based on coordinates of dataset points?
  if(include_datapoint_grid) 
    {
      for(int i=0; i<l; i++)
        {
          dataset->getExample(i,input,target,weight);
          x = input[0];
          y = input[1];
          if(x>xmin && x<xmax)
            xpos.insert(x);
          if(y>ymin && y<ymax)
            ypos.insert(y);
        }
    }

  nx = xpos.size();
  ny = ypos.size();
  set<real>::iterator itx;
  set<real>::iterator ity;

  out << "object \"outputs_gridpos\" class array type float rank 1 shape 2 items " << nx*ny << " data follows\n";
  for(itx=xpos.begin(); itx!=xpos.end(); ++itx)
    for(ity=ypos.begin(); ity!=ypos.end(); ++ity)
      out << *itx << " " << *ity << "\n";
  out << "\n\n";

  out << "object \"outputs_gridcon\" class gridconnections counts " << nx << " " << ny << "\n"
    //      << "attribute \"element type\" string \"cubes\" \n"
      << "attribute \"ref\" string \"positions\" \n\n\n";

  out << "object \"outputs_values\" class array type float rank 1 shape " << outputsize << " items " << nx*ny << " data follows \n";
  
  ProgressBar pb("Computing outputs for grid positions: " + tostring(nx)+"x"+tostring(ny), nx*ny);
  int n = 0;
  for(itx=xpos.begin(); itx!=xpos.end(); ++itx)
    {
      input[0] = *itx;
      for(ity=ypos.begin(); ity!=ypos.end(); ++ity)
        {
          input[1] = *ity;
          learner->computeOutput(input, output);
          for(int j=0; j<outputsize; j++)
            out << output[j] << " ";
          out << "\n";
          if(logsum==-FLT_MAX)
            logsum = output[0];
          else 
            logsum = logadd(logsum, output[0]);
          pb.update(n++);
        }
    }
  pb.close();
  out << "attribute \"dep\" string \"positions\" \n\n\n";

  out << "object \"outputs\" class field \n"
      << "component \"positions\" \"outputs_gridpos\" \n"
      << "component \"connections\" \"outputs_gridcon\" \n"
      << "component \"data\" \"outputs_values\" \n\n\n";
  
  out << "end" << endl;

  double surfelem = deltax*deltay;
  double surfintegral = exp(logsum)*surfelem;
  cerr << "Estimated integral over sampled domain: " << surfintegral << endl;
}


GenerateDecisionPlot::GenerateDecisionPlot() 
  :basename("dxplot"),
   nx(10), ny(10),
   include_datapoint_grid(false),
   xmin(MISSING_VALUE), 
   xmax(MISSING_VALUE), 
   ymin(MISSING_VALUE), 
   ymax(MISSING_VALUE)
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

    declareOption(ol, "basename", &GenerateDecisionPlot::basename, OptionBase::buildoption,
                  "Base name of the .dx data file to generate. Running this class will generate\n"
                  "files basename_dset.dx containing targets and outputs for the given dataset positions\n"
                  "and basename_outputs.dx containing outputs computed at grid positions\n");
    declareOption(ol, "learner", &GenerateDecisionPlot::learner, OptionBase::buildoption,
                   "The learner to train/test. Its train_set will be used as the dataset on\n"
                  "which to base this decision plot (ranges are inferred from it, and decisions\n"
                  "on the training points are written in basename_dset.dx");
    declareOption(ol, "nx", &GenerateDecisionPlot::nx, OptionBase::buildoption,
                   "Number of x sample coordinates (grid)");
    declareOption(ol, "ny", &GenerateDecisionPlot::ny, OptionBase::buildoption,
                   "Number of y sample coordinates (grid)");
    declareOption(ol, "include_datapoint_grid", &GenerateDecisionPlot::include_datapoint_grid, OptionBase::buildoption,
                   "");
    declareOption(ol, "xmin", &GenerateDecisionPlot::xmin, OptionBase::buildoption,
                   "");
    declareOption(ol, "xmax", &GenerateDecisionPlot::xmax, OptionBase::buildoption,
                   "");
    declareOption(ol, "ymin", &GenerateDecisionPlot::ymin, OptionBase::buildoption,
                   "");
    declareOption(ol, "ymax", &GenerateDecisionPlot::ymax, OptionBase::buildoption,
                   "");
    declareOption(ol, "save_learner_as", &GenerateDecisionPlot::save_learner_as, OptionBase::buildoption,
                   "(Optionally) save trained learner in this file (.psave)");

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
  VMat dataset = learner->getTrainingSet();
  learner->setTrainStatsCollector(new VecStatsCollector());
  learner->train();

  if(save_learner_as!="")
    {
      cerr << "Saving trained learner in file " << save_learner_as << endl;
      PLearn::save(save_learner_as, *learner);
    }

  string dset_fname = basename+"_dset.dx";
  cerr << "Computing and writing dataset output field to file " << dset_fname << endl;
  DX_create_dataset_outputs_file(dset_fname, learner, dataset);

  string outputs_fname = basename+"_outputs.dx";
  cerr << "Computing and writing grid output field to file " << outputs_fname << endl; 
  DX_create_grid_outputs_file(outputs_fname, learner, dataset, nx, ny, 
                               include_datapoint_grid, 
                               xmin, xmax, ymin, ymax);
  cerr << "You can now view those files with OpenDX." << endl;
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
