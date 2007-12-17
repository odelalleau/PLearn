// -*- C++ -*-
 
// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2004 ApSTAT Technologies Inc.
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
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

#include "DisplayUtils.h"
#include <plearn/io/openString.h>
#include <plearn/io/TmpFilenames.h>

#if defined(WIN32) && !defined(__CYGWIN__)
#include <io.h>
#define unlink _unlink
#endif

namespace PLearn {
using namespace std;


  /*! scores is a (nsamples x nclasses) matrix with rows containing scores
  for each class.  winners is a (nsamples x 3) matrix with rows containing
  the winning class (argmax), its winning score (max), and the difference
  to the second best class (margin)
  */

  void scores_to_winners(Mat scores, Mat& winners)
  {
    int l = scores.length();
    winners.resize(l,3);
    for(int i=0; i<l; i++)
      {
        Vec scorerow = scores(i);
        int maxpos = argmax(scorerow);
        real maxval = scorerow[maxpos];
        scorerow[maxpos] = -FLT_MAX;
        real maxval2 = max(scorerow);
        scorerow[maxpos] = maxval;
        winners(i,0) = maxpos;
        winners(i,1) = maxval;
        winners(i,2) = maxval-maxval2;
      }
  }


  void color_luminance_to_rgb(int colornum, real luminance, real& r, real& g, real& b)
  {
    if(luminance<0 || luminance>1)
      PLERROR("In color_luminance_to_rgb luminance %f outside of range [0,1]",luminance);    
  }

  real color_luminance_to_rgbreal(int colornum, real luminance)
  {
    real r=0, g=0, b=0;
    color_luminance_to_rgb(colornum, luminance, r, g, b);
    return rgb2real(r,g,b);
  }

  void color_luminance_to_rgbreal(Vec colornum, Vec luminance, Vec& rgbreal)
  {
    int l = colornum.length();
    rgbreal.resize(l);
    for(int i=0; i<l; i++)
      rgbreal[i] = color_luminance_to_rgbreal((int)colornum[i],luminance[i]);
  }
    
  void transform_perclass_values_into_luminance(Vec classnums, const Vec& values, int ndiscretevals)
  {
    int l = classnums.length();
    int nclasses = (int)max(classnums);
    Vec minval(nclasses,FLT_MAX);
    Vec maxval(nclasses,-FLT_MAX);
    for(int i=0; i<l; i++)
      {
        int c = int(classnums[i]);
        real val = values[i];
        if(val<minval[c])
          minval[c] = val;
        if(val>maxval[c])
          maxval[c] = val;        
      }

    for(int i=0; i<l; i++)
      {
        int c = int(classnums[i]);
        real val = values[i];
        // rescale it between 0 and 1
        val = (val-minval[c])/(maxval[c]-minval[c]);
        if(ndiscretevals>1) // discretize it
          val = floor(val*ndiscretevals+0.5)/ndiscretevals;
        values[i] = val;
      }
  }


  void regulargrid_x_y_rgbreal_to_bitmap(Mat& regulargrid_x_y_rgbreal, 
                                         Mat& bm, real& xlow, real& xhigh, real& ylow, real& yhigh)
  {
    TVec<int> key_columns(2);
    key_columns[0] = 0;
    key_columns[1] = 1;
    sortRows(regulargrid_x_y_rgbreal, key_columns);
    int l = regulargrid_x_y_rgbreal.length();
    xlow = regulargrid_x_y_rgbreal(0,0);
    xhigh = regulargrid_x_y_rgbreal(l-1,0);
    ylow = regulargrid_x_y_rgbreal(0,1);
    yhigh = regulargrid_x_y_rgbreal(l-1,1);
    int ny=1;
    while(!fast_exact_is_equal(regulargrid_x_y_rgbreal(ny,1), ylow))
      ++ny;

    int nx = l/ny;
    if(nx*ny!=l)
      PLERROR("Problem in regulargrid_x_y_rgbreal_to_rgbimage : estimated_nx * estimated_ny != l (%d*%d!=%d)",nx,ny,l);

    bm.resize(ny,nx);    
    int k = 0;
    for(int j=0; j<nx; j++)
      for(int i=ny-1; i>=0; i--)
        bm(i,j) = regulargrid_x_y_rgbreal(k++,2);
  }


  void regulargrid_x_y_outputs_to_bitmap(Mat regulargrid_x_y_outputs, bool output_margin, int ndiscretevals,
                                         Mat& bm, real& xlow, real& xhigh, real& ylow, real& yhigh)

  {
    int l = regulargrid_x_y_outputs.length();
    int outputsize = regulargrid_x_y_outputs.width()-2;
    Mat regulargrid_x_y = regulargrid_x_y_outputs.subMatColumns(0,2);
    Mat outputs = regulargrid_x_y_outputs.subMatColumns(2,outputsize);
    Mat winners;
    scores_to_winners(outputs, winners);
    Vec classnums(l);
    Vec values(l);
    classnums << winners.column(0);
    values << winners.column(output_margin ?2 :1);
    transform_perclass_values_into_luminance(classnums, values, ndiscretevals);
    Vec rgbreal;
    color_luminance_to_rgbreal(classnums, values, rgbreal);
    Mat regulargrid_x_y_rgbreal(l,3);
    regulargrid_x_y_rgbreal.subMatColumns(0,2) << regulargrid_x_y;
    regulargrid_x_y_rgbreal.column(2) << rgbreal;    
    regulargrid_x_y_rgbreal_to_bitmap(regulargrid_x_y_rgbreal, 
                                      bm, xlow, xhigh, ylow, yhigh);  
  }

void displayHistogram(Gnuplot& gp, Mat dataColumn,
		      int n_bins, Vec* pbins, 
		      bool regular_bins,
		      bool normalized, string extra_args)
{
  Vec sorted_data = dataColumn.toVecCopy();
  sortElements(sorted_data);
  int n=sorted_data.length();
  real minv = sorted_data[0];
  real maxv = sorted_data[n-1];

  // compute "bins" vector, which specifies histogram intervals
  // [minv, bins[0]), [bins[0],bins[1]), ... [bin[n_bins-2],maxv]
  Vec bins;
  if (pbins)
    {
      bins = *pbins;
      n_bins = bins.length()+1;
    }
  else
    {
      if (n_bins==0)
	n_bins = MIN(5+n/10,1000);
      bins.resize(n_bins-1);

      // fill the bins
      if (regular_bins)
	{
	  real delta = (maxv-minv)/n_bins;
	  real v = minv+delta;
	  real* b=bins.data();
	  for (int i=0;i<n_bins-1;i++,v+=delta) b[i]=v;
	}
      else
	{
	  real n_expected_per_bin = n/(real)n_bins;
	  int current_bin=0;
	  real* v=sorted_data.data();
	  real* b=bins.data();
	  real previous = 1e30;
	  int n_repeat = 0;
	  int previous_n_repeat = 0;
	  int first_of_mass_point = 0;
	  for (int i=0;i<n;i++)
	    {
	      if (fast_exact_is_equal(previous, v[i]))
		{
		  if (previous_n_repeat==0) first_of_mass_point = i-1;
		  n_repeat++;
		}
	      else
		n_repeat=0;
	      if (n_repeat==0  && current_bin < n_bins-1) // put a left_side at i only if v[i]!=v[i-1]
		{
		  if (previous_n_repeat==0)
		    {
		      if (i+1 >= n_expected_per_bin*(1+current_bin))
			b[current_bin++]=v[i];
		    }
		  else
		    {
		      if (n_repeat/(real)n > n_expected_per_bin)
			{
			  if (current_bin>0 && b[current_bin-1] < v[first_of_mass_point])
			    b[current_bin++]=v[first_of_mass_point];
			  if (current_bin < n_bins-1)
			    b[current_bin++]=v[i];
			}
		      else
			if (i+1 >= n_expected_per_bin*(1+current_bin))
			  b[current_bin++]=v[i];
		    }
		}
	      previous = v[i];
	      previous_n_repeat = n_repeat;
	    }
	}
    }

  // fill histogram vector with counts in each interval:
  // first column is the left border of each bin, 2nd is the count
  Mat histogram(n_bins+1,2);
  real* left_side = &histogram(0,0);
  real* frequency = left_side+1;
  real* b = bins.data();
  real* v=sorted_data.data();
  int current_bin=0;
  real left = minv;
  for (int i=0;i<n;i++)
    {
      if (current_bin<n_bins-1 &&
	  v[i]>=b[current_bin])
	{
	  left_side[2*current_bin]=left;
	  left = v[i];
	  current_bin++;
	}
      frequency[2*current_bin]++;
    }
  left_side[2*current_bin]=left;
  left_side[2*n_bins]=maxv+(maxv-minv)/n;
  real norm_factor = normalized? (1.0/n) : 1.0;
  for (int i=0;i<n_bins;i++)
    {
      real deltax = left_side[2*(i+1)]-left_side[2*i];
      if (fast_exact_is_equal(deltax, 0)) {
          PLWARNING("displayHistogram: 0 deltax!");
          deltax=1.0;
      }
      frequency[i*2] *= norm_factor/deltax;
    }

  histogram(n_bins,1)=histogram(n_bins-1,1);

  // display the histogram
  string comm = string(" with steps")+extra_args;
  gp.plot(histogram,comm.c_str());
}


  //! returns a subvector made of the (max) n "central" values of v
  Vec centerSubVec(Vec v, int n=16)
  {
    int l = v.length();
    if(l<=n)
      return v;    
    return v.subVec((l-n)/2,n);
  }

/** VarGraph **/

void displayVarGraph(const VarArray& outputs, bool display_values, real boxwidth, const char* the_filename, bool must_wait, VarArray display_only_these)
{
  // parameters controlling appearance...
  real deltay = 100;
  real boxheight = 50;

  char filename[100];
  if(the_filename)
    strcpy(filename, the_filename);
  else
  {
    TmpFilenames tmpnam;
    strcpy(filename, tmpnam.addFilename().c_str());
  }
  
  multimap<real,Var> layers; 
  typedef multimap<real,Var>::iterator mmit;

  Mat center(Variable::nvars+1,2);
  center.fill(FLT_MAX);
  
  int n_display_only_these = display_only_these.size();
  bool display_all = n_display_only_these==0;

  // find sources of outputs which are not in the outputs array:
  outputs.unmarkAncestors();
  VarArray sources = outputs.sources();
  outputs.unmarkAncestors();
  // We dont want any source Var that is in outputs to be in sources so we remove them:
  outputs.setMark();
  VarArray nonoutputsources;
  for(int i=0; i<sources.size(); i++)
    if(!sources[i]->isMarked() && (display_all || display_only_these.contains(sources[i])))
      nonoutputsources.append(sources[i]);
  sources = nonoutputsources;
  outputs.clearMark();
  
  sources.setMark();

  // Place everything but the sources starting from outputs at the bottom

  outputs.unmarkAncestors();

  real y = boxheight;
  VarArray varray = outputs;
  
  while(varray.size()>0)
    {
      // varray.setMark(); // so that these don't get put in subsequent parents() calls
      VarArray parents;
      int nvars = varray.size();
      for(int i=0; i<nvars; i++)
        {
          Var v = varray[i];
          real old_y = center(v->varnum,1);
          if (!fast_exact_is_equal(old_y, FLT_MAX)) // remove pair (old_y,v) from layers
          {
            pair<mmit,mmit> range = layers.equal_range(old_y);
            for (mmit it = range.first; it != range.second; it++)
              if (v->varnum == it->second->varnum)
              {
                layers.erase(it);
                break;
              }
          }
          layers.insert(pair<real,Var>(y, v));
          center(v->varnum,1) = y;
          VarArray parents_i = v->parents();
          for (int j=0;j<parents_i.size();j++)
            if((display_all || display_only_these.contains(parents_i[j])) && !parents.contains(parents_i[j]))
              parents &= parents_i[j];
        }
      varray = parents;
      y += deltay;
    }
  // now place the sources
  int nvars = sources.size();
  for(int i=0; i<nvars; i++)
    {
      Var v = sources[i];
      real old_y = center(v->varnum,1);
      if (!fast_exact_is_equal(old_y, FLT_MAX)) // remove pair (old_y,v) from layers
      {
        pair<mmit,mmit> range = layers.equal_range(old_y);
        for (mmit it = range.first; it != range.second; it++)
          if (v->varnum == it->second->varnum)
          {
            layers.erase(it);
            break;
          }
      }
      layers.insert(pair<real,Var>(y,v));
    }
  real topy = y;

  outputs.unmarkAncestors();
  if (display_all)
  {
    VarArray ancestors = outputs.ancestors();
    outputs.unmarkAncestors();   
    varray = ancestors;
  }
  else varray = display_only_these;

  // Find the maximum number of vars in a level...
  int maxvarsperlevel = sources.size();

  for (real y_=boxheight;y_<=topy;y_+=deltay)
  {
    pair<mmit,mmit> range = layers.equal_range(y_);
    int nvars_ = (int)distance(range.first,range.second);
    if (maxvarsperlevel < nvars_)
      maxvarsperlevel = nvars_;
  }

  real usewidth = (maxvarsperlevel+1)*(boxwidth+boxheight);

  // Compute the bounding box:
  real min_x = 0;
  real min_y = 0;
  real max_x = usewidth;
  real max_y = topy;

  min_x -= boxwidth/2;
  max_x += boxwidth/2;
  min_y -= boxheight/2;
  max_y += boxheight/2;

  for (real y_=boxheight;y_<=topy;y_+=deltay)
  {
    pair<mmit,mmit> range = layers.equal_range(y_);
    int nvars_ = (int)distance(range.first,range.second);
    real deltax = usewidth/(nvars_+1);
    real x = deltax;
    for (mmit it = range.first; it != range.second; it++, x+=deltax)
    {
      Var v = it->second;
      center(v->varnum,0) = x;
      center(v->varnum,1) = y_;
    }
  }

  // Start outputting to the file
  {
    // make it an eps file with the computed bounding box
    GhostScript gs(filename,min_x,min_y,max_x,max_y);

  // Now paint

  // gs.setlinewidth(1.0);

  for (real y_=boxheight;y_<=topy;y_+=deltay)
  {
    pair<mmit,mmit> range = layers.equal_range(y_);
    int nvars_ = (int)distance(range.first,range.second);
    real deltax = usewidth/(nvars_+1);
    real x = deltax;
    for (mmit it = range.first; it != range.second; it++, x+=deltax)
    {
      Var v = it->second;
      real my_x = x;
      real my_y = y_;

      // Display v
      gs.drawBox(my_x-boxwidth/2, my_y-boxheight/2, boxwidth, boxheight);
      char nameline[100];
      sprintf(nameline,"%s (%d,%d)",v->getName().c_str(), v->matValue.length(), v->matValue.width());
      
      string descr;
      PStream str_descr = openString(descr, PStream::raw_ascii, "w");
      str_descr << v;

      if(display_values)
        {
          gs.usefont("Times-Bold", 11.0);
          gs.centerShow(my_x, my_y+boxheight/4, descr.c_str());
          gs.usefont("Times-Roman", 10.0);
          gs.centerShow(my_x, my_y, nameline);
          gs.usefont("Courrier", 6.0);
          if (v->rValue.length()>0) // print rvalue if there are some...
          {
            gs.centerShow(my_x, my_y-boxheight/5, centerSubVec(v->value));
            gs.centerShow(my_x, my_y-boxheight/3, centerSubVec(v->gradient));
            gs.centerShow(my_x, my_y-boxheight/1, centerSubVec(v->rValue));
          }
          else
          {
            gs.centerShow(my_x, my_y-boxheight/5, centerSubVec(v->value));
            gs.centerShow(my_x, my_y-boxheight/2.5, centerSubVec(v->gradient));
          }
          /*
          cout << descr << " " << nameline << " (" << v->value.length() << ")" << endl;
          cout << "value:    " << v->value << endl;
          cout << "gradient: " << v->gradient << endl;
          */
        }
      else
        {
          gs.usefont("Times-Bold", 12.0);
          gs.centerShow(my_x, my_y+boxheight/4, descr.c_str());
          gs.usefont("Times-Roman", 11.0);
          gs.centerShow(my_x, my_y-boxheight/4, nameline);
        }

      // Display the arrows from the parents
      VarArray parents = v->parents();
      int nparents = parents.size();
      for(int p=0; p<nparents; p++)
        {
          Var parent = parents[p];
          if (display_all || display_only_these.contains(parent))
          {
            real parent_x = center(parent->varnum,0);
            real parent_y = center(parent->varnum,1);

            gs.drawArrow(parent_x, parent_y-boxheight/2, 
                         my_x+0.75*boxwidth*(real(p+1)/real(nparents+1)-0.5), 
                         my_y+boxheight/2);
          }
        }
    }
  }
  outputs.unmarkAncestors();      
  }
  char command[1000];
  if (must_wait)
    sprintf(command,"gv %s",filename);
  else
    sprintf(command,"gv %s &",filename);

  system(command);

  if(the_filename==0 && must_wait)
    unlink(filename);
}

void OldDisplayVarGraph(const VarArray& outputs, bool display_values, real boxwidth, const char* the_filename, bool must_wait, VarArray display_only_these)
{
  // parameters controlling appearance...
  real deltay = 100;
  real boxheight = 50;

  char filename[100];
  if(the_filename)
    strcpy(filename, the_filename);
  else
  {
    TmpFilenames tmpnam;
    strcpy(filename, tmpnam.addFilename().c_str());
  }
  
  Mat center(Variable::nvars+1,2);
  center.fill(FLT_MAX);
  
  int n_display_only_these = display_only_these.size();
  bool display_all = n_display_only_these==0;

  // find sources of outputs which are not in the outputs array:
  outputs.unmarkAncestors();
  VarArray sources = outputs.sources();
  outputs.unmarkAncestors();
  // We dont want any source Var that is in outputs to be in sources so we remove them:
  outputs.setMark();
  VarArray nonoutputsources;
  for(int i=0; i<sources.size(); i++)
    if(!sources[i]->isMarked() && (display_all || display_only_these.contains(sources[i])))
      nonoutputsources.append(sources[i]);
  sources = nonoutputsources;
  outputs.clearMark();
  
  // Find the maximum number of vars in a level...
  int maxvarsperlevel = sources.size();
  sources.setMark();
  VarArray varray = outputs;
  while(varray.size()>0)
  {
    if(varray.size()>maxvarsperlevel)
      maxvarsperlevel = varray.size();
    varray.setMark(); // so that these don't get put in subsequent parents() calls
    VarArray parents;
    for(int i=0; i<varray.size(); i++)
      parents &= varray[i]->parents();
    varray = VarArray();
    for (int i=0;i<parents.size();i++)
      if(display_all || display_only_these.contains(parents[i]))
        varray &= parents[i];
  }
  sources.setMark();

  real usewidth = (maxvarsperlevel+1)*(boxwidth+boxheight);

  // Place everything but the sources starting from outputs at the bottom

  outputs.unmarkAncestors();

  real y = boxheight;
  varray = outputs;
  
  while(varray.size()>0)
    {
      // varray.setMark(); // so that these don't get put in subsequent parents() calls
      VarArray parents;
      int nvars = varray.size();
      for(int i=0; i<nvars; i++)
        {
          Var v = varray[i];
          center(v->varnum,0) = usewidth*(i+1)/(nvars+1);
          center(v->varnum,1) = y;                
          // bool marked = v->isMarked();
          // v->clearMark();
          VarArray parents_i = v->parents();
          for (int j=0;j<parents_i.size();j++)
            if((display_all || display_only_these.contains(parents_i[j])) && !parents.contains(parents_i[j]))
              parents &= parents_i[j];
        }
      varray = parents;
      y += deltay;
    }
  // now place the sources
  int nvars = sources.size();
  for(int i=0; i<nvars; i++)
    {
      Var v = sources[i];
      center(v->varnum,0) = usewidth*(i+1)/(nvars+1);
      center(v->varnum,1) = y;                
    }

  outputs.unmarkAncestors();
  if (display_all)
  {
    VarArray ancestors = outputs.ancestors();
    outputs.unmarkAncestors();   
    varray = ancestors;
  }
  else varray = display_only_these;

  // Compute the bounding box:
  real min_x = FLT_MAX;
  real min_y = FLT_MAX;
  real max_x = -FLT_MAX;
  real max_y = -FLT_MAX;

  for(int i=0; i<varray.size(); i++)
    {
      Var v = varray[i];
      real x = center(v->varnum,0);
      real y_ = center(v->varnum,1);
      if(x<min_x)
        min_x = x;
      if(y_<min_y)
        min_y = y_;
      if(x>max_x)
        max_x = x;
      if(y_>max_y)
        max_y = y_;
    }
  min_x -= boxwidth/2;
  max_x += boxwidth/2;
  min_y -= boxheight/2;
  max_y += boxheight/2;

  // Start outputting to the file
  {
    // make it an eps file with the computed bounding box
    GhostScript gs(filename,min_x,min_y,max_x,max_y);

  // Now paint

  // gs.setlinewidth(1.0);

  for(int i=0; i<varray.size(); i++)
    {
      Var v = varray[i];
      real my_x = center(v->varnum,0);
      real my_y = center(v->varnum,1);

      // Display v
      gs.drawBox(my_x-boxwidth/2, my_y-boxheight/2, boxwidth, boxheight);
      char nameline[100];
      sprintf(nameline,"%s (%d,%d)",v->getName().c_str(), v->matValue.length(), v->matValue.width());

      string descr;
      PStream str_descr = openString(descr, PStream::raw_ascii, "w");
      str_descr << v;

      if(display_values)
        {
          gs.usefont("Times-Bold", 11.0);
          gs.centerShow(my_x, my_y+boxheight/4, descr.c_str());
          gs.usefont("Times-Roman", 10.0);
          gs.centerShow(my_x, my_y, nameline);
          gs.usefont("Courrier", 6.0);
          gs.centerShow(my_x, my_y-boxheight/5, v->value);
          gs.centerShow(my_x, my_y-boxheight/2.5, v->gradient);
        }
      else
        {
          gs.usefont("Times-Bold", 12.0);
          gs.centerShow(my_x, my_y+boxheight/4, descr.c_str());
          gs.usefont("Times-Roman", 11.0);
          gs.centerShow(my_x, my_y-boxheight/4, nameline);
        }

      // Display the arrows from the parents
      VarArray parents = v->parents();
      int nparents = parents.size();
      for(int p=0; p<nparents; p++)
        {
          Var parent = parents[p];
          if (display_all || display_only_these.contains(parent))
          {
            real parent_x = center(parent->varnum,0);
            real parent_y = center(parent->varnum,1);

            gs.drawArrow(parent_x, parent_y-boxheight/2, 
                         my_x+0.75*boxwidth*(real(p+1)/real(nparents+1)-0.5), 
                         my_y+boxheight/2);
          }
        }
    }
  outputs.unmarkAncestors();      
  }

  char command[1000];
  if (must_wait)
    sprintf(command,"gv %s",filename);
  else
    sprintf(command,"gv %s &",filename);

  system(command);

  if(the_filename==0)
    unlink(filename);
}

void displayFunction(Func f, bool display_values, bool display_differentiation, real boxwidth, const char* the_filename, bool must_wait)
{ 
  if(display_differentiation)
    displayVarGraph(f->outputs & f->differentiate()->outputs, display_values, boxwidth, the_filename, must_wait);
  else
    displayVarGraph(f->outputs, display_values, boxwidth, the_filename, must_wait); 
}

Mat compute2dGridOutputs(PP<PLearner> learner, real min_x, real max_x, real min_y, real max_y, int length, int width, real singleoutput_threshold)
{
  Mat m(length,width);
  real delta_x = (max_x-min_x)/(width-1);
  real delta_y = (max_y-min_y)/(length-1);

  if(learner->inputsize()!=2 || (learner->outputsize()!=1 && learner->outputsize()!=2) )
     PLERROR("learner is expected to have an inputsize of 2, and an outputsize of 1 (or possibly 2 for binary classification)");

  Vec input(2);
  Vec output(learner->outputsize());
  for(int i=0; i<length; i++)
  {
    input[1] = min_y+(length-i-1)*delta_y;
    for(int j=0; j<width; j++)
      {
        input[0] = min_x+j*delta_x;
        learner->computeOutput(input,output);
        if(learner->outputsize()==2)
          m(i,j) = output[0]-output[1];
        else
          m(i,j) = output[0]-singleoutput_threshold;
      }
  }
  return m;
}

void displayPoints(GhostScript& gs, Mat data, real radius, bool color)
{
  for(int i=0; i<data.length(); i++)
    {
      Vec point = data(i);
      if(color)
        {
          if(point[2]<=0.0)
            gs.setcolor(1.0,0.0,0.0);
          else
            gs.setcolor(0.0,0.0,1.0);
          gs.drawCross(point[0], point[1], radius);
        }
      else
        gs.drawCross(point[0], point[1], radius, point[2]<=0);
    }
} 

/*
  // Old version based on a Classifier
void displayDecisionSurface(GhostScript& gs, Classifier& cl, real xmin, real xmax, int nxsamples, real ymin, real ymax, int nysamples)
{
  Vec input(2);
  Vec scores(1);
  Mat bm(nysamples,nxsamples);

  for(int i=0; i<nysamples; i++)
    for(int j=0; j<nxsamples; j++)
      {
        input[0] = xmin+(xmax-xmin)/(nxsamples-1)*j;
        input[1] = ymax-(ymax-ymin)/(nysamples-1)*i;
        cl.computeOutput(input,scores);
        // cerr << scores[0] << "| ";
        real r,g,b;
        if(scores[0]<=0.5)
          r(i,j) = scores[0]*1.8;
        else
          b(i,j) = (1.0-scores[0])*1.8;
      }
  gs.gsave();
  gs.translate(xmin,ymin);
  gs.scale((xmax-xmin)/nxsamples, (ymax-ymin)/nysamples);
  gs.displayRGB(0,0,r,g,b);
  gs.grestore();
}
*/

void displayDecisionSurface(GhostScript& gs, real destx, real desty, real destwidth, real destheight, 
                            PP<PLearner> learner, Mat trainset, 
                            Vec svindexes, Vec outlierindexes, int nextsvindex,
                            real min_x, real max_x, real min_y, real max_y,
                            real radius, 
                            int nx, int ny)
  {
    gs.gsave();
    real scalefactor = (max_x-min_x)/destwidth;
    gs.mapping(min_x,min_y,max_x-min_x,max_y-min_y, destx, desty, destwidth, destheight);
    gs.setlinewidth(1.0*scalefactor);

    real singleoutput_threshold = 0.;
    if(learner->outputsize()==1)
    {
      Mat targets = trainset.column(learner->inputsize());
      singleoutput_threshold = 0.5*(min(targets)+max(targets));
    }
    Mat decisions = compute2dGridOutputs(learner, min_x, max_x, min_y, max_y, ny, nx, singleoutput_threshold);

    //real posrange = max(decisions);
    //real negrange = min(decisions);

    for(int i=0; i<ny; i++)
      for(int j=0; j<nx; j++)
        {
          decisions(i,j) = (decisions(i,j)<0. ? 0.75 : 1.0);
          /*
          if(decisions(i,j) < 0.0)
            decisions(i,j) = 1.0-.5*decisions(i,j)/negrange;
          else
            decisions(i,j) = .5+.5*decisions(i,j)/posrange;
          */
        }

    gs.displayGray(decisions,min_x,min_y,max_x-min_x,max_y-min_y);

    // draw x and +
    displayPoints(gs, trainset, radius, false);

    // draw black circles around support vectors
    for(int k=0; k<svindexes.length(); k++)
      {
        real x = trainset(int(svindexes[k]),0);
        real y = trainset(int(svindexes[k]),1);
        // cerr << "{" << x << "," << y << "}";
        gs.drawCircle(x,y,radius);
      }
    // cerr << endl;

    // draw half radius circle around next support vector
    if(nextsvindex>=0)
    {
      real x = trainset(nextsvindex,0);
      real y = trainset(nextsvindex,1);
      gs.drawCircle(x,y,radius/2);
    }

    // draw white circles around outliers
    Vec dashpattern(2,4.0*scalefactor);
    gs.setdash(dashpattern);
    for(int k=0; k<outlierindexes.length(); k++)
      {
        real x = trainset(int(outlierindexes[k]),0);
        real y = trainset(int(outlierindexes[k]),1);
        gs.drawCircle(x,y,radius);
      }

    gs.grestore();
  }

#ifdef WIN32
#undef unlink
#endif

} // end of namespace PLearn
