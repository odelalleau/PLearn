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

#include "Gnuplot.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/io/FdPStreamBuf.h>
#include <plearn/base/tostring.h>  

#if defined(WIN32) && !defined(__CYGWIN__) && !defined(_MINGW_)
#include <io.h>
// norman: potentially dangerous if there is a function called with the same name in this
//         file. Beware!
#define chmod _chmod
#define fileno _fileno
#define popen _popen
#define pclose _pclose
#endif

namespace PLearn {
using namespace std;

Gnuplot::Gnuplot(int nb_max_plot) 
  : tmpfilenames(nb_max_plot,"/tmp/","gp"),
    gp_cstream(popen("gnuplot","w"))  //, tognuplot(fileno(gp_cstream))
{
  tognuplot = new FdPStreamBuf(-1, fileno(gp_cstream));
  tognuplot.outmode=PStream::raw_ascii;
  tognuplot << "set data style lines" << endl;
}

Gnuplot::Gnuplot(const Vec& v1, const string& opt1)
  : tmpfilenames(5,"/tmp/","gp"),
    gp_cstream(popen("gnuplot","w")) //, tognuplot(fileno(gp_cstream))
{
  tognuplot = new FdPStreamBuf(-1, fileno(gp_cstream));
  tognuplot.outmode=PStream::raw_ascii;
  tognuplot << "set data style lines" << endl;
  plot(v1,opt1);
}

Gnuplot::Gnuplot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2) 
  : tmpfilenames(5,"/tmp/","gp"),
    gp_cstream(popen("gnuplot","w")) //,    tognuplot(fileno(gp_cstream))
{
  tognuplot = new FdPStreamBuf(-1, fileno(gp_cstream));
  tognuplot.outmode=PStream::raw_ascii;
  tognuplot << "set data style lines" << endl;
  plot(v1,opt1,v2,opt2);
}

Gnuplot::Gnuplot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2, const Vec& v3, const string& opt3)
  : tmpfilenames(5,"/tmp/","gp"),
    gp_cstream(popen("gnuplot","w")) //,    tognuplot(fileno(gp_cstream))
{
  tognuplot = new FdPStreamBuf(-1, fileno(gp_cstream));
  tognuplot.outmode=PStream::raw_ascii;
  tognuplot << "set data style lines" << endl;
  plot(v1,opt1,v2,opt2,v3,opt3);
}


Gnuplot::~Gnuplot()
{ 
  tognuplot << "\nquit" << endl; 
  pclose(gp_cstream);
}

PStream& Gnuplot::operator<<(const string& str)
{ 
  tognuplot << str; 
  return tognuplot;
}

void Gnuplot::flush()
{ tognuplot.flush(); }

void Gnuplot::setxrange(real xmin, real xmax)
{  tognuplot << "set xrange [" << xmin << ":" << xmax << "]" << endl; }

void Gnuplot::setyrange(real ymin, real ymax)
{  tognuplot << "set yrange [" << ymin << ":" << ymax << "]" << endl; }

void Gnuplot::setrange(real xmin, real xmax, real ymin, real ymax)
{
  setxrange(xmin,xmax);
  setyrange(ymin,ymax);
}

void Gnuplot::seteps(const string &filename)
{
  tognuplot << "set term post color" << endl;
  tognuplot << "set output \"" << filename << "\"" << endl;
}

void Gnuplot::plot(const Vec& v1, const string& opt1)
{
  saveGnuplot(tmpfilenames[0].c_str(), v1);
  chmod(tmpfilenames[0].c_str(),0777);
  tognuplot << "plot '" << tmpfilenames[0] << "' " << opt1 << endl;
}

void Gnuplot::plot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2)
{
  saveGnuplot(tmpfilenames[0].c_str(), v1);
  chmod(tmpfilenames[0].c_str(),0777);
  saveGnuplot(tmpfilenames[1].c_str(), v2);
  chmod(tmpfilenames[1].c_str(),0777);
  tognuplot << "plot '" << tmpfilenames[0] << "' " << opt1 << ", '" << tmpfilenames[1] << "' " << opt2 << endl;
}

void Gnuplot::plot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2, const Vec& v3, const string& opt3)
{
  saveGnuplot(tmpfilenames[0].c_str(), v1);
  chmod(tmpfilenames[0].c_str(),0777);
  saveGnuplot(tmpfilenames[1].c_str(), v2);
  chmod(tmpfilenames[1].c_str(),0777);
  saveGnuplot(tmpfilenames[2].c_str(), v3);
  chmod(tmpfilenames[2].c_str(),0777);
  tognuplot << "plot '" << tmpfilenames[0] << "' " << opt1 
            << ", '" << tmpfilenames[1] << "' " << opt2 
            << ", '" << tmpfilenames[2] << "' " << opt3 << endl;
}

void Gnuplot::plot(const Vec& v1, const string& opt1, const Vec& v2, const string& opt2, const Vec& v3, const string& opt3, const Vec& v4, const string& opt4)
{
  saveGnuplot(tmpfilenames[0].c_str(), v1);
  chmod(tmpfilenames[0].c_str(),0777);
  saveGnuplot(tmpfilenames[1].c_str(), v2);
  chmod(tmpfilenames[1].c_str(),0777);
  saveGnuplot(tmpfilenames[2].c_str(), v3);
  chmod(tmpfilenames[2].c_str(),0777);
  saveGnuplot(tmpfilenames[3].c_str(), v4);
  chmod(tmpfilenames[3].c_str(),0777);
  tognuplot << "plot '" << tmpfilenames[0] << "' " << opt1 
            << ", '" << tmpfilenames[1] << "' " << opt2 
            << ", '" << tmpfilenames[2] << "' " << opt3
            << ", '" << tmpfilenames[3] << "' " << opt4 << endl; 
}
 
void Gnuplot::plot(const Mat& m1, const string& opt1)
{
  saveGnuplot(tmpfilenames[0].c_str(), m1);
  tognuplot << "plot '"<< tmpfilenames[0] << "' " << opt1 << endl;
}


void Gnuplot::plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2, const Mat& m3, const string& opt3)
{
  saveGnuplot(tmpfilenames[0].c_str(), m1);
  saveGnuplot(tmpfilenames[1].c_str(), m2);
  saveGnuplot(tmpfilenames[2].c_str(), m3);
  string command = "plot '" + tmpfilenames[0] + "' " + opt1 + ", " +
    "'" + tmpfilenames[1] + "' " + opt2 + ", " +
    "'" + tmpfilenames[2] + "' " + opt3;
  // cerr << command << endl;
  tognuplot << command << endl;
}

void Gnuplot::plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2)
{
  saveGnuplot(tmpfilenames[0].c_str(), m1);
  saveGnuplot(tmpfilenames[1].c_str(), m2);
  tognuplot << "plot '" << tmpfilenames[0] << "' " << opt1 << ", ";
  tognuplot << "'" << tmpfilenames[1] << "' " << opt2 << endl;
}

void Gnuplot::plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2, const Mat& m3, const string& opt3, const Mat& m4, const string& opt4)
{
  saveGnuplot(tmpfilenames[0].c_str(), m1);
  saveGnuplot(tmpfilenames[1].c_str(), m2);
  saveGnuplot(tmpfilenames[2].c_str(), m3);
  saveGnuplot(tmpfilenames[3].c_str(), m4);
  tognuplot << "plot '" << tmpfilenames[0] << "' " << opt1 << ", ";
  tognuplot << "'" << tmpfilenames[1] << "' " << opt2 << ", ";
  tognuplot << "'" << tmpfilenames[2] << "' " << opt3 << ", ";
  tognuplot << "'" << tmpfilenames[3] << "' " << opt4 << endl;
}

void Gnuplot::plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2, const Mat& m3, 
                   const string& opt3, const Mat& m4, const string& opt4, const Mat& m5, const string& opt5)
{
  saveGnuplot(tmpfilenames[0].c_str(), m1);
  saveGnuplot(tmpfilenames[1].c_str(), m2);
  saveGnuplot(tmpfilenames[2].c_str(), m3);
  saveGnuplot(tmpfilenames[3].c_str(), m4);
  saveGnuplot(tmpfilenames[4].c_str(), m5);
  tognuplot << "plot '" << tmpfilenames[0] << "' " << opt1 << ", ";
  tognuplot << "'" << tmpfilenames[1] << "' " << opt2 << ", ";
  tognuplot << "'" << tmpfilenames[2] << "' " << opt3 << ", ";
  tognuplot << "'" << tmpfilenames[3] << "' " << opt4 << ", ";
  tognuplot << "'" << tmpfilenames[4] << "' " << opt5 << endl;
}

void Gnuplot::plot(const Mat& m1, const string& opt1, const Mat& m2, const string& opt2, const Mat& m3, 
                   const string& opt3, const Mat& m4, const string& opt4, const Mat& m5, const string& opt5,
                   const Mat& m6, const string& opt6)
{
  saveGnuplot(tmpfilenames[0].c_str(), m1);
  saveGnuplot(tmpfilenames[1].c_str(), m2);
  saveGnuplot(tmpfilenames[2].c_str(), m3);
  saveGnuplot(tmpfilenames[3].c_str(), m4);
  saveGnuplot(tmpfilenames[4].c_str(), m5);
  saveGnuplot(tmpfilenames[5].c_str(), m6);
  tognuplot << "plot '" << tmpfilenames[0] << "' " << opt1 << ", ";
  tognuplot << "'" << tmpfilenames[1] << "' " << opt2 << ", ";
  tognuplot << "'" << tmpfilenames[2] << "' " << opt3 << ", ";
  tognuplot << "'" << tmpfilenames[3] << "' " << opt4 << ", ";
  tognuplot << "'" << tmpfilenames[4] << "' " << opt5 << ", ";
  tognuplot << "'" << tmpfilenames[5] << "' " << opt6 << endl;
}

void Gnuplot::plotClasses(const Mat& m)
{
  Mat s = m.copy();
  sortRows(s, 2);
  string fname = tmpfilenames[0];
  int l = s.length();
  string command = "plot ";
  ofstream out(fname.c_str());
  if(!out)
    PLERROR("Could not open file %s for writing ", fname.c_str());

  real oldc = FLT_MAX;
  int index = 0;
  for(int i=0; i<l; i++)
    {
      real x = s(i,0);
      real y = s(i,1);
      real c = s(i,2);
      if(!fast_exact_is_equal(c, oldc))
        {
          if(i>0)
            {
              out << "\n\n";
              command += ", ";
            }
          command += "'" + fname + "' index " + tostring(index) + " title '" + tostring(c) + "' with points";
          ++index;
        }
      out << x << " " << y << " " << c << "\n";      
      oldc = c;
    }
  out.close();

  //  cerr <<  command << endl;
  tognuplot << command << endl;
}

void Gnuplot::multiplot(vector<Mat *> &ms, vector<string> &opts)
{
  tognuplot << "plot ";
  for (unsigned int i = 0; i < ms.size(); ++i) {
    saveGnuplot(tmpfilenames[i].c_str(), *(ms[i]));
    if (i) tognuplot << ", ";
    tognuplot << "'" << tmpfilenames[i] << "' " << opts[i];
  }
  tognuplot << endl;
}

void Gnuplot::plot3d(const Mat &m1, const string &opt1)
{
    //tognuplot << "set contour" << endl;
    tognuplot << "set dgrid3d" << endl;
    //tognuplot << "set isosamples 10, 10" << endl;
    saveGnuplot(tmpfilenames[0].c_str(), m1);
    tognuplot << "splot '" << tmpfilenames[0] << "' " << opt1 << endl;
}

void Gnuplot::histoplot(Vec feature, real minval, real maxval, int
                        nbins, bool do_normalize, char* title)
{
  ofstream out(tmpfilenames[0].c_str());
  Vec histo = histogram(feature, minval, maxval, nbins);
  if(do_normalize)
    normalize(histo,real(1.0));
  real binwidth = (maxval-minval)/nbins;
  
  for(int i=0; i<nbins; i++)
    out << minval + (0.5+i)*binwidth + 0.5 << " " << histo[i] << " " << binwidth << endl;
    //out << minval+(0.5+i)*nbins << " " << histo[i] << " " << binwidth << endl;

  tognuplot << "plot '" << tmpfilenames[0] << "' title '" << title << "' with boxes" << endl;
}

void Gnuplot::histoplot(Vec feature, Vec classnums, real minval,
                        real maxval, int nbins, bool do_normalize)
{
  ofstream out(tmpfilenames[0].c_str());
  int nclasses = (int)max(classnums) + 1;
  real binwidth = (maxval-minval)/nbins;
  real deltaval = maxval-minval+ 1e-6;

  Mat histo(nclasses, nbins);
  for(int i=0; i<feature.length(); i++)
    {
      real val = feature[i];
      int binpos = int((val-minval)/deltaval*nbins);
      if(binpos>=0 && binpos<nbins)
        histo(int(classnums[i]),binpos)++;
    }

  if(do_normalize)
    normalize(histo,real(1.0));

  for(int c=0; c<nclasses; c++)
    {
      for(int i=0; i<nbins; i++)
        out << minval+(0.5+i)*binwidth << " " << histo(c,i) << " " << binwidth
            << endl;
      out << endl;
    }

  tognuplot << "plot '" << tmpfilenames[0] << "' every PLearn:::0::0 title 'Class 0' with boxes";
  for(int c=1; c<nclasses; c++)
    tognuplot << ", '" << tmpfilenames[0] << "' every PLearn:::" << c << "::" << c
              << "  title 'Class " << c << "' with boxes";
  tognuplot << endl;
}

void Gnuplot::featureplot(Mat inputs, Vec classnums, char* withwhat)
{
  ofstream out(tmpfilenames[0].c_str());
  
  int nclasses = (int)max(classnums) + 1;

  for(int c=0; c<nclasses; c++)
    {
      for(int i=0; i<inputs.length(); i++)
        if((int)classnums[i] == c)
          for(int j=0; j<inputs.width(); j++)
            out << j << ' ' << inputs(i,j) << endl;
      out << endl;
    }

  tognuplot << "plot '" << tmpfilenames[0] << "' every PLearn:::0::0 title 'Class 0' with " << withwhat;
  for(int c=1; c<nclasses; c++)
    tognuplot << ", '" << tmpfilenames[0] << "' every PLearn:::" << c << "::" << c
              << "  title 'Class " << c << "' with " << withwhat;
  tognuplot << endl;
}

void Gnuplot::plotcdf(Vec feature, const string& title)
{
  Vec v = feature.copy();
  sortElements(v);
  ofstream out(tmpfilenames[0].c_str());
  // out << "0 0\n";
  for(int i=0; i<v.length(); i++)
    out << v[i] << ' ' << real(i+1)/v.length() << '\n';
  out.close();
  tognuplot << "plot '" << tmpfilenames[0] << "' using 1:2 title '" << title << "' with impulses" << endl;
}

void Gnuplot::plotcdf(const Array<Vec>& vecarray, const Array<string>& titlearray)
{
  tognuplot << "plot ";
  for(int c=0; c<vecarray.size(); c++)
  {
    Vec v = vecarray[c].copy();
    sortElements(v);
    ofstream out(tmpfilenames[c].c_str());
    for(int i=0; i<v.length(); i++)
      out << v[i] << ' ' << real(i+1)/v.length() << '\n';
    out.close();
    tognuplot << "'" << tmpfilenames[c] << "' using 1:2 title '" << titlearray[c] << "'";
    if(c==vecarray.size()-1)
      tognuplot << endl;
    else
      tognuplot << ", ";
  }
}

void Gnuplot::plotdensity(Vec feature, const string& title, int halfwindowsize, string range)
{
  Vec v = feature.copy();
  sortElements(v);
  ofstream out(tmpfilenames[0].c_str());
  for (int i=1; i<v.length()-1; i++)
  {
    real x = v[i];
    int lowi = std::max(i-halfwindowsize,1);
    int highi = std::min(i+halfwindowsize,v.length()-2);
    real lowx = 0.5*(v[lowi-1]+v[lowi]);
    real highx = 0.5*(v[highi+1]+v[highi]);
    if (fast_exact_is_equal(highx, lowx))
      PLERROR("In Gnuplot::plotdensity(...), density at this point is inf, use larger windowsize");
    real density = (highi-lowi+1)/(highx-lowx);
    out << x << ' ' << density << "\n";
  }
  out.close();
  tognuplot << "plot " << range << " '" << tmpfilenames[0] << "' title '" << title << "' with lines" << endl;
}

void Gnuplot::plotdensity(const Array<Vec>& vecarray, const Array<string>& titlearray, int halfwindowsize)
{
  tognuplot << "plot ";
  for(int c=0; c<vecarray.size(); c++)
  {
    Vec v = vecarray[c].copy();
    sortElements(v);
    ofstream out(tmpfilenames[c].c_str());
    for (int i=1; i<v.length()-1; i++)
    {
      real x = v[i];
      int lowi = std::max(i-halfwindowsize,1);
      int highi = std::min(i+halfwindowsize,v.length()-2);
      real lowx = 0.5*(v[lowi-1]+v[lowi]);
      real highx = 0.5*(v[highi+1]+v[highi]);
      real density = (highi-lowi+1)/(highx-lowx);
      out << x << ' ' << density << "\n";
    }
    out.close();
    tognuplot << "'" << tmpfilenames[c] << "' using 1:2 title '" << titlearray[c] << "'";
    if(c==vecarray.size()-1)
      tognuplot << endl;
    else
      tognuplot << ", ";
  }
}


void Gnuplot::export_ps(string psfname, string psoptions)
{
  tognuplot << "set terminal postscript " << psoptions << "\n"
            << "set output '" << psfname << "'\n"
            << "replot\n"
            << "set output\n"
            << "set terminal x11" << endl;
}
  
#ifdef WIN32
#undef _chmod
#undef _fileno
#undef _popen
#undef _pclose
#endif

} // end of namespace PLearn
