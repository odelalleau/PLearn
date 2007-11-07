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
   * $Id$
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearn/plearn/display/Gnuplot.h */

#ifndef GNUPLOT_INC
#define GNUPLOT_INC

#include <plearn/io/PStream.h>
#include <plearn/math/Mat.h>
#include <plearn/io/TmpFilenames.h>
#include <plearn/base/Array.h>

namespace PLearn {
using namespace std;


class Gnuplot
{
 protected:
  TmpFilenames tmpfilenames;
  FILE* gp_cstream;  
  PStream tognuplot;

 public:
  Gnuplot(int max_nb_plot=20);   
  Gnuplot(const Vec& v1, const string& opt1);
  Gnuplot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2);
  Gnuplot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2, const Vec& v3, const string& opt3);

 ~Gnuplot();
  
  PStream& operator<<(const string& str);
  void flush();

  void setxrange(real xmin, real xmax);
  void setyrange(real ymin, real ymax);
  void setrange(real xmin, real xmax, real ymin, real ymax);
  void seteps(const string &filename); //!<  set to eps mode, will save to specified .eps file

  void plot(const Vec& v1, const string& opt1="");
  void plot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2);
  void plot(const Vec& v1, const Vec&v2) { plot(v1,"",v2,""); }
  void plot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2, const Vec& v3, const string& opt3);
  void plot(const Vec& v1, const Vec&v2, const Vec& v3) { plot(v1,"",v2,"",v3,""); }
  void plot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2, const Vec& v3, const string& opt3, const Vec& v4, const string& opt4);
  void plot(const Vec& v1, const Vec&v2, const Vec& v3, const Vec& v4) { plot(v1,"",v2,"",v3,"",v4,""); }
  void plot(const Mat& m1, const string& opt1="");
  void plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2);
  void plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2, const Mat& m3, const string& opt3);
  void plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2, const Mat& m3, const string& opt3, const Mat& m4, const string& opt4);
  void plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2, const Mat& m3, const string& opt3, 
            const Mat& m4, const string& opt4, const Mat& m5, const string& opt5);
  void plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2, const Mat& m3, const string& opt3, 
            const Mat& m4, const string& opt4, const Mat& m5, const string& opt5, const Mat& m6, const string& opt6);

  // m must have 3 columns, with the 3rd column containing class numbers
  // all points (x,y) will be plotted, with a different color and shape for each class
  void plotClasses(const Mat& m);

  void multiplot(vector<Mat *> &ms, vector <string> &opts);
  void plot3d(const Mat &m1, const string &opt1="");

    void plotcdf(Vec feature, const string& title="cdf");
    void plotcdf(const Array<Vec>& vecarray, const Array<string>& titlearray);
    void plotdensity(Vec feature, const string& title, int halfwindowsize, string range="");
    void plotdensity(const Array<Vec>& vecarray, const Array<string>& titlearray, int halfwindowsize);

  void histoplot(Vec feature, real minval, real maxval, int nbins, bool do_normalize=false, char* title="histogram");

  void histoplot(Vec feature, Vec classnums, real minval, real maxval, int nbins, bool do_normalize=false);

/*!     Plots each input feature (column of inputs), with the
    different values it takes in the dataset and with a different
    color for each class
*/
  void featureplot(Mat inputs, Vec classnums, char* withwhat="points");

  void featureplot(Mat dataset, char* withwhat="points") 
    { 
      featureplot(dataset.subMatColumns(0,dataset.width()-1),
                  dataset.lastColumn().toVecCopy(), withwhat);
    }

    // export as postscript file
    void export_ps(string psfname, string psoptions="eps color");
  
};

} // end of namespace PLearn

#endif
