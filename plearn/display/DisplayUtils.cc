// -*- C++ -*-
 
// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: DisplayUtils.cc,v 1.5 2004/02/29 16:44:05 nova77 Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

#include "DisplayUtils.h"
#include "TmpFilenames.h"
#include <strstream>

#ifdef WIN32
#include <io.h>
#define unlink _unlink
#endif

namespace PLearn {
using namespace std;

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
	      if (previous==v[i])
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
      if (deltax==0) { PLWARNING("displayHistogram: 0 deltax!"); deltax=1.0; }
      frequency[i*2] *= norm_factor/deltax;
    }

  histogram(n_bins,1)=histogram(n_bins-1,1);

  // display the histogram
  string comm = string(" with steps")+extra_args;
  gp.plot(histogram,comm.c_str());
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
          if (old_y != FLT_MAX) // remove pair (old_y,v) from layers
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
      if (old_y != FLT_MAX) // remove pair (old_y,v) from layers
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

  for (real y=boxheight;y<=topy;y+=deltay)
  {
    pair<mmit,mmit> range = layers.equal_range(y);
    int nvars = (int)distance(range.first,range.second);
    if (maxvarsperlevel < nvars)
      maxvarsperlevel = nvars;
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

  for (real y=boxheight;y<=topy;y+=deltay)
  {
    pair<mmit,mmit> range = layers.equal_range(y);
    int nvars = (int)distance(range.first,range.second);
    real deltax = usewidth/(nvars+1);
    real x = deltax;
    for (mmit it = range.first; it != range.second; it++, x+=deltax)
    {
      Var v = it->second;
      center(v->varnum,0) = x;
      center(v->varnum,1) = y;
    }
  }

  // Start outputting to the file
  {
    // make it an eps file with the computed bounding box
    GhostScript gs(filename,min_x,min_y,max_x,max_y);

  // Now paint

  // gs.setlinewidth(1.0);

  for (real y=boxheight;y<=topy;y+=deltay)
  {
    pair<mmit,mmit> range = layers.equal_range(y);
    int nvars = (int)distance(range.first,range.second);
    real deltax = usewidth/(nvars+1);
    real x = deltax;
    for (mmit it = range.first; it != range.second; it++, x+=deltax)
    {
      Var v = it->second;
      real my_x = x;
      real my_y = y;

      // Display v
      gs.drawBox(my_x-boxwidth/2, my_y-boxheight/2, boxwidth, boxheight);
      char nameline[100];
      sprintf(nameline,"%s (%d,%d)",v->getName().c_str(), v->matValue.length(), v->matValue.width());
      
      char buf[200];
      ostrstream descr(buf,200);
      v->print(descr);
      descr << ends;

      if(display_values && v->size() <= 16)
        {
          gs.usefont("Times-Bold", 11.0);
          gs.centerShow(my_x, my_y+boxheight/4, descr.str());
          gs.usefont("Times-Roman", 10.0);
          gs.centerShow(my_x, my_y, nameline);
          gs.usefont("Courrier", 6.0);
          if (v->rValue.length()>0) // print rvalue if there are some...
          {
            gs.centerShow(my_x, my_y-boxheight/5, v->value);
            gs.centerShow(my_x, my_y-boxheight/3, v->gradient);
            gs.centerShow(my_x, my_y-boxheight/1, v->rValue);
          }
          else
          {
            gs.centerShow(my_x, my_y-boxheight/5, v->value);
            gs.centerShow(my_x, my_y-boxheight/2.5, v->gradient);
          }
          /*
          cout << descr.str() << " " << nameline << " (" << v->value.length() << ")" << endl;
          cout << "value:    " << v->value << endl;
          cout << "gradient: " << v->gradient << endl;
          */
        }
      else
        {
          gs.usefont("Times-Bold", 12.0);
          gs.centerShow(my_x, my_y+boxheight/4, descr.str());
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
      real y = center(v->varnum,1);
      if(x<min_x)
        min_x = x;
      if(y<min_y)
        min_y = y;
      if(x>max_x)
        max_x = x;
      if(y>max_y)
        max_y = y;
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

      char buf[200];
      ostrstream descr(buf,200);
      v->print(descr);
      descr << ends;

      if(display_values)
        {
          gs.usefont("Times-Bold", 11.0);
          gs.centerShow(my_x, my_y+boxheight/4, descr.str());
          gs.usefont("Times-Roman", 10.0);
          gs.centerShow(my_x, my_y, nameline);
          gs.usefont("Courrier", 6.0);
          gs.centerShow(my_x, my_y-boxheight/5, v->value);
          gs.centerShow(my_x, my_y-boxheight/2.5, v->gradient);
        }
      else
        {
          gs.usefont("Times-Bold", 12.0);
          gs.centerShow(my_x, my_y+boxheight/4, descr.str());
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

Mat compute2dGridOutputs(Learner& learner, real min_x, real max_x, real min_y, real max_y, int length, int width, real singleoutput_threshold)
{
  Mat m(length,width);
  real delta_x = (max_x-min_x)/(width-1);
  real delta_y = (max_y-min_y)/(length-1);

  if(learner.inputsize()!=2 || (learner.outputsize()!=1 && learner.outputsize()!=2) )
     PLERROR("learner is expected to have an inputsize of 2, and an outputsize of 1 (or possibly 2 for binary classification)");

  Vec input(2);
  Vec output(learner.outputsize());
  for(int i=0; i<length; i++)
  {
    input[1] = min_y+(length-i-1)*delta_y;
    for(int j=0; j<width; j++)
      {
        input[0] = min_x+j*delta_x;
        learner.use(input,output);
        if(learner.outputsize()==2)
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
        cl.use(input,scores);
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
                            Learner& learner, Mat trainset, 
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
    if(learner.outputsize()==1)
    {
      Mat targets = trainset.column(learner.inputsize());
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
