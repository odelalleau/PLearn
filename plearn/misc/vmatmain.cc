// -*- C++ -*-

// vmatmain.cc
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux, Rejean Ducharme
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
   * $Id: vmatmain.cc,v 1.2 2002/09/05 05:14:42 plearner Exp $
   ******************************************************* */

#include "vmatmain.h"
#include "general.h"
#include "StatsCollector.h"
#include "VMatLanguage.h"
#include "PreproVMat.h"
#include "VMat.h"
#include "TMat_maths.h"
#include "stringutils.h"
#include "getDataSet.h"
#include "Gnuplot.h"
#include <curses.h>


namespace PLearn <%
using namespace std;

//! Prints where m1 and m2 differ by more than tolerance
//! returns the number of such differences, or -1 if the sizes differ
int print_diff(ostream& out, VMat m1, VMat m2, double tolerance)
{
  int ndiff = 0;
  if(m1.length()!=m2.length() || m1.width()!=m2.width())
    {
      out << "Size of the two matrices differ: " 
           << m1.length() << " x " << m1.width() << "  vs.  "
           << m2.length() << " x " << m2.width() << endl;
      return -1;
    }
  int l = m1.length();
  int w = m1.width();
  Vec v1(w);
  Vec v2(w);
  for(int i=0; i<l; i++)
    {
      m1->getRow(i,v1);
      m2->getRow(i,v2);
      for(int j=0; j<w; j++)
        {
          double d = v1[j]-v2[j];
          if(fabs(d)>tolerance)
            {
              out << "Elements at " << i << ',' << j << " differ by " << d << endl;
              ++ndiff;
            }
        }
    }
}

void interactiveDisplayCDF(const Array<VMat>& vmats)
{
  int k = vmats.size();
  int w = vmats[0]->width();

  Array<string> name(k);
  cout << ">>>> Dimensions of vmats: \n";
  for(int i=0; i<k; i++)
    {
      name[i] = vmats[i]->getMetaDataDir();
      cout << name[i] << ": \t " << vmats[i]->length() << " x " << vmats[i]->width() << endl;
    }

  vmats[0]->printFields(cout);

  Gnuplot gp;

  for(;;)
  {
  // TVec<RealMapping> ranges = vm->getRanges();

    cout << "Field (0.." << w-1 << ") [low high] ? ";
    vector<string> command;
    int varnum = -1;
    real low = -FLT_MAX; // means autorange
    real high = FLT_MAX; // means autorange
    do
      {
        command = split(pgetline(cin));
        if(command.size()==0)
          vmats[0]->printFields(cout);
        else
          {
            varnum = toint(command[0]);
            if(varnum<0 || varnum>=w)
              vmats[0]->printFields(cout);
            else if(command.size()==3)
              {
                low = toreal(command[1]);
                high = toreal(command[2]);
              }
          }
      } while(varnum<0 || varnum>=w);


    cout << "\n\n*************************************" << endl;
    cout << "** #" << varnum << ": " << vmats[0]->fieldName(varnum) << " **" << endl;
    cout << "*************************************" << endl;

    Array<Mat> m(k);

    for(int i=0; i<k; i++)
      {
        TVec<StatsCollector> stats = vmats[i]->getStats();        
        StatsCollector& st = stats[varnum];
        m[i] = st.cdf(true);
        cout << "[ " << name[i]  << " ]" << endl;
        cout << st << endl;
      }
    // cout << "RANGES: " << endl;
    // cout << ranges[varnum];

    if(low == -FLT_MAX)
      gp << "set xrange [*:*]" << endl;      
    else
      gp << "set xrange [" << low << ":" << high << "]" << endl;

    if(k>=4)
      gp.plot(m[0],"title '"+name[0]+"'", m[1], "title '" + name[1]+"'", m[2], "title '" + name[2]+"'", m[3], "title '"+name[3]+"'");    
    else if(k>=3)
      gp.plot(m[0],"title '"+name[0]+"'", m[1], "title '"+name[1]+"'", m[2], "title '"+name[2]+"'");
    else if(k>=2)
      gp.plot(m[0],"title '"+name[0]+"'", m[1], "title '"+name[1]+"'");
    else
      gp.plot(m[0],"title '"+name[0]+"'");
  }
}

void displayBasicStats(VMat vm)
{
  int nfields = vm.width();
  TVec<StatsCollector> stats = vm->getStats();        

  cout << "# \t fieldname \t mean \t stddev \t min \t max \t count \t nmissing \t stderr" << endl; 
  for(int k=0; k<nfields; k++)
    {
      cout << k << " \t" 
           << vm->fieldName(k) << " \t" 
           << stats[k].mean() << " \t"
           << stats[k].stddev() << " \t"
           << stats[k].min() << " \t"
           << stats[k].max() << " \t"
           << stats[k].n() << " \t"
           << stats[k].nmissing() << " \t"
           << stats[k].stderror() << " \t"
           << endl;
    }
}

void printDistanceStatistics(VMat vm, int inputsize)
{
  int l = vm.length();
  int w = vm.width();
  Vec x1(w);
  Vec x2(w);
  StatsCollector collector(2);  
  ProgressBar pb(cerr, "Computing distance statistics", l-1);
  for(int i=0; i<l-1; i++)
    {
      vm->getRow(i,x1);
      vm->getRow(i+1,x2);
      real d = L2distance(x1.subVec(0,inputsize),x2.subVec(0,inputsize));
      collector.update(d);
      pb(i);
    }

  cout << "Euclidean distance statistics: " << endl;
  cout << collector << endl;
}

void printConditionalStats(VMat vm, int condfield)
{
  cout << "*** Ranges ***" << endl;
  TVec<RealMapping> ranges = vm->getRanges();
  PP<ConditionalStatsCollector> st = vm->getConditionalStats(condfield);
  int w = vm->width();
  for(int i=0; i<w; i++)
    {
      cout << "Field #" << i << ": " << vm->fieldName(i) << endl;
      cout << "Ranges: " << ranges[i] << endl;
    }
  cout << "\n\n------------------------------------------------------------" << endl;
  cout << "** Raw counts conditioned on field #" << condfield << " (" << vm->fieldName(condfield) << ") **\n" << endl;
  for(int k=0; k<w; k++)
    {
      cout << "#" << k << " " << vm->fieldName(condfield) << endl;
      cout << st->counts[k] << endl;
    }
  
  cout << "\n\n------------------------------------------------------------" << endl;
  cout << "** Joint probabilities (percentage) **\n" << endl;
  for(int k=0; k<w; k++)
    {
      TMat<int>& C = st->counts[k];
      Mat m(C.length(), C.width());
      m << C;
      m /= sum(m);
      m *= real(100);
      cout << "#" << k << " " << vm->fieldName(condfield) << endl;
      cout << m << endl;
    }

  cout << "\n\n------------------------------------------------------------" << endl;
  cout << "** Conditional probabilities conditioned on << " << vm->fieldName(condfield) << "  **\n" << endl;
  for(int k=0; k<w; k++)
    {
      TMat<int>& C = st->counts[k];
      Mat m(C.length(), C.width());      
      m << C;
      normalizeRows(m);
      m *= real(100);
      cout << "#" << k << " " << vm->fieldName(condfield) << endl;
      cout << m << endl;
    }

  cout << "\n\n------------------------------------------------------------" << endl;
  cout << "** Conditional probabilities conditioned on the other variables **\n" << endl;
  for(int k=0; k<w; k++)
    {
      TMat<int>& C = st->counts[k];
      Mat m(C.length(), C.width());      
      m << C;
      normalizeColumns(m);
      m *= real(100);
      cout << "#" << k << " " << vm->fieldName(condfield) << endl;
      cout << m << endl;
    }


}


void viewVMat(const VMat& vm, int lin, int col)
{
  initscr();
  cbreak();
  noecho();
  keypad(stdscr,TRUE);
  
  int key= 0, curl= 0, curc= 0;
  bool view_strings= true;
  
  while(key != 'q' && key != 'Q')
    {
      erase();

      for(int j= 0; (j+2)*10 < COLS && j+col<vm->width(); ++j)
	{
	  string s= vm->fieldName(j+col);
	  mvprintw(0,(j+1)*10-1," %9s", s.substr(0,9).c_str());
	  if(s.length() > 9)
	    mvprintw(1,(j+1)*10-1," %9s", s.substr(9,9).c_str());
	}
      for(int i= 0; i < LINES-3 && i+lin<vm->length(); ++i)
	{
	  mvprintw(i+2,0,"%d", i+lin);
	  for(int j= 0; (j+2)*10 < COLS && j+col<vm->width(); ++j)
	    {
	      if(i == curl || j == curc)
		attron(A_REVERSE);

	      real x= vm(i+lin, j+col);
	      string s= vm->getValString(j+col, x);
	      if(!view_strings || s == "")
		mvprintw(i+2,(j+1)*10-1," %9f", x);
	      else
		mvprintw(i+2,(j+1)*10-1," %9s", s.substr(0,9).c_str());

	      attroff(A_REVERSE);
	    }
	}

      //real x= vm(curl+lin, curc+col);
      //string strval= vm->getValString(curc+col, x);
      //if(!view_strings || strval == "")
      //strval= tostring(x);

      string strval = vm->getString(curl+lin, curc+col);
      mvprintw(LINES-1,0," %dx%d   line= %d   col= %d [%s]   val= %s", 
	       vm->length(), vm->width(),
	       curl+lin, curc+col, vm->fieldName(curc+col).c_str(), strval.c_str());


      refresh();
      key= getch();

      switch(key)
	{
	case KEY_UP: 
	  if(0 < curl) --curl; 
	  else if(lin>0) --lin; 
	  break;
	case KEY_DOWN: 
	  if(curl < LINES-4 && curl+lin < vm->length()-1) ++curl; 
	  else if(lin  < vm->length()-1) ++lin; 
	  if(curl+lin >= vm->length()) curl= vm->length()-lin-1;
	  break;
	case KEY_PPAGE: 
	  lin-=LINES-3; 
	  if(lin < 0) lin= 0; 
	  break;
	case KEY_NPAGE: 
	  lin+=LINES-3; 
	  if(lin >= vm->length()) lin= vm->length()-1; 
	  if(curl+lin >= vm->length()) curl= vm->length()-lin-1;
	  break;
	case KEY_LEFT: 
	  if(0 < curc) --curc; 
	  else if(col>0) --col; 
	  break;
	case KEY_RIGHT: 
	  if(curc < COLS/10-2 && curc+col < vm->width()-1) ++curc; 
	  else if(col < vm->width()-1) ++col;
	  if(curc+col >= vm->width()) curc= vm->width()-col-1;
	  break;
	case (int)'l': case (int)'L': 
	  {
	    echo();
	    mvprintw(LINES-1,0,"Goto line:                                                                            ");
	    move(LINES-1, 11);
	    char l[10];
	    getnstr(l, 10);
	    if(!pl_isnumber(l) || toint(l) < 0 || toint(l)>=vm->length())
	      {
		mvprintw(LINES-1,0,"*** Invalid line number ***");
		refresh();
		sleep(1);
	      }
	    else
	      lin= toint(l);
	    noecho();
	  }
	  break;
	case (int)'c': case (int)'C': 
	  {
	    echo();
	    mvprintw(LINES-1,0,"Goto column:                                                                           ");
	    move(LINES-1, 13);
	    char c[10];
	    getnstr(c, 10);
	    if(!pl_isnumber(c) || toint(c) < 0 || toint(c)>=vm->width())
	      {
		mvprintw(LINES-1,0,"*** Invalid column number ***");
		refresh();
		sleep(1);
	      }
	    else
	      col= toint(c);
	    noecho();
	  }
	  break;

	case (int)'s': case (int)'S': 
	  if(view_strings)
	    {
	    mvprintw(LINES-1,0,"*** Strings already shown ***");
	    refresh();
	    sleep(1);
	    }
	  else
	    view_strings= true;
	  break;

	case (int)'n': case (int)'N': 
	  if(!view_strings)
	    {
	    mvprintw(LINES-1,0,"*** Numbers already shown ***");
	    refresh();
	    sleep(1);
	    }
	  else
	    view_strings= false;
	  break;

	case (int)'h': case (int)'H':
	  erase();

	  mvprintw(0,COLS/2-6,"*** HELP ***");

	  mvprintw(2,10,"KEYS:");
	  mvprintw(3,10," - up: move up one line");
	  mvprintw(4,10," - down: move down one line");
	  mvprintw(5,10," - right: move right one column");
	  mvprintw(6,10," - left: move left one column");
	  mvprintw(7,10," - page up: move up one screen");
	  mvprintw(8,10," - page down: move down one screen");
	  mvprintw(9,10," - 'l' or 'L': prompt for a line number and go to that line");
	  mvprintw(10,10," - 'c' or 'C': prompt for a column number and go to that column");
	  mvprintw(11,10," - 's' or 'S': display string fields as strings");
	  mvprintw(12,10," - 'n' or 'N': display string fields as numbers");
	  mvprintw(13,10," - 'h' or 'H': display this screen");
	  mvprintw(14,10," - 'q' or 'Q': quit program");

	  mvprintw(16,COLS/2-13,"(press any key to continue)");

	  refresh();
	  getch();
	  
	  break;

	case (int)'q': case (int)'Q': 
	  break;
 
	default:
	  mvprintw(LINES-1,0,"*** Invalid command (type 'h' for help) ***");
	  refresh();
	  sleep(1);
	  break;
	}
    }
  
  endwin();
}


void plotVMats(char* defs[], int ndefs)
{
  /* defs[] is of format:
     { "<dataset0>", "<col0>[:<row0>:<nrows0>]", ..., "<datasetN>", "<colN>[:<rowN>:<nrowsN>]" }
   */
  int nseries= ndefs/2;
  TmpFilenames tmpfnames(nseries, "/tmp/", "_vmat_plot_");
  Array<VMat> vmats(nseries);
  Array<Vec> series(nseries);
  string gp_command= "plot ";
  for(int i= 0; i < nseries; ++i)
    {
      vmats[i]= getDataSet(string(defs[2*i]));

      vector<string> spec= PLearn::split(defs[2*i+1], ":");
      
      series[i].resize(vmats[i].length());
      vmats[i]->getColumn(toint(spec[0]),series[i]);

      if(spec.size() == 3)
	{
	  int row= toint(spec[1]), nrows= toint(spec[2]);
	  if(row+nrows > series[i].length())
	    nrows= series[i].length()-row;
	  series[i]= series[i].subVec(row, nrows);
	}
      else if(spec.size() != 1)
	PLERROR("in plotVMats: invalid spec for vmat %s: '%s'; sould be '<col>[:<row>:<nrows>]'.",
		defs[2*i], defs[2*i+1]);

      saveGnuplot(tmpfnames[i].c_str(), series[i]);
      chmod(tmpfnames[i].c_str(),0777);      
      gp_command+= " '" + tmpfnames[i] + "' title '" + defs[2*i] + ' ' + defs[2*i+1] + "' " + tostring(i+1)  +", ";
    }
  gp_command.resize(gp_command.length()-2);

  Gnuplot gp;
  gp << gp_command << endl;
  
  cout << "Press any key to close GNUplot window and exit." << endl;
  cin.get();
}

int vmatmain(int argc, char** argv)
{
  
  if(argc<3)
  {
    cerr << 
      "Usage: vmat info <dataset> \n"
      "       Will info about dataset (size, etc..)\n"
      "   or: vmat fields <dataset> \n"
      "       To list the fields with their names \n"
      "   or: vmat cat <dataset> [<optional_vpl_filtering_code>]\n"
      "       To display the dataset \n"
      "   or: vmat view <dataset> [<line#> [<column#>]]\n"
      "       Interactive display to browse on the data. \n"
      "   or: vmat stats <dataset> \n"
      "       Will display basic statistics for each field \n"
      "   or: vmat convert <source> <destination> \n"
      "       To convert any dataset into a .amat .pmat or .dmat format \n"
      "       The extension of the destination is used to determine the format you want \n"
      "   or: vmat gendef <source> [binnum1 binnum2 ...] \n"
      "       Generate stats for dataset (will put them in its associated metadatadir). \n"
      "   or: vmat genvmat <source_dataset> <dest_vmat> [binned{num} | onehot{num} | normalized]\n"
      "       Will generate a template .vmat file with all the fields of the source preprocessed\n"
      "       with the processing you specify\n"
      "   or: vmat genkfold <source_dataset> <fileprefix> <kvalue>\n"
      "       Will generate <kvalue> pairs of .vmat that are splitted so they can be used for kfold trainings\n"
      "       The first .vmat-pair will be named <fileprefix>_train_1.vmat (all source_dataset except the first 1/k)\n"
      "       and <fileprefix>_test_1.vmat (the first 1/k of <source_dataset>\n"
      "   or: vmat diff <dataset1> <dataset2> [tolerance] \n"
      "       Will report all elements that differ by more than tolerance (defauts to 1e-6) \n"
      "   or: vmat cdf <dataset> [<dataset> ...] \n"
      "       To interactively display cumulative density function for each field \n"
      "       along with its basic statistics \n"
      "   or: vmat cond <dataset> <condfield#> \n"
      "       Interactive display of coditional statistics conditioned on the \n"
      "       conditioning field <condfield#> \n"
      "   or: vmat diststat <dataset> <inputsize>\n"
      "       Will compute and output basic statistics on the euclidean distance \n"
      "       between two consecutive input points \n"
      "<dataset> is a parameter understandable by getDataSet. This includes all matrix file\n"
      "file formats. Type 'vmat help dataset' so see what other <dataset> strings are possible." << endl;
    exit(0);
  }

  string command = argv[1];

  if(command=="cdf")
    {      
      Array<VMat> vmats;
      for(int i=2; i<argc; i++)
        {
          string dbname = argv[i];
          VMat vm = getDataSet(dbname);
          vmats.append(vm);
        }
      interactiveDisplayCDF(vmats);
    }
  else if(command=="cond")
    {
      string dbname = argv[2];
      VMat vm = getDataSet(dbname);
      cout << "** Using dataset: " << dbname << " **" << endl;
      cout << "Metadata for this dataset in: " << vm->getMetaDataDir() << endl;
      int condfield = atoi(argv[3]);
      printConditionalStats(vm, condfield);    
    }
  else if(command=="convert")
    {
      string source = argv[2];
      string destination = argv[3];
      VMat vm = getDataSet(source);

      string ext = extract_extension(destination);
      if(ext==".amat")
        vm->saveAMAT(destination);
      else if(ext==".pmat")
        vm->savePMAT(destination);
      else if(ext==".dmat")
        vm->saveDMAT(destination);
      else
        {
          cerr << "ERROR: can only convert to .amat .pmat or .dmat " << endl
               << "Please specify a destination name with a valid extension " << endl;
        }
    }
  else if(command=="info")
    {
      string dbname = argv[2];
      VMat vm = getDataSet(dbname);
      cout<<vm.length()<<" x "<<vm.width()<<endl;
      VVMatrix * vvm = dynamic_cast<VVMatrix*>((VMatrix*)vm);
      if(vvm!=NULL)
      {
        cout<<"Last modification (including dependencies of .vmat): "<<vvm->getMtime()<<endl;
        bool ispre=vvm->isPrecomputedAndUpToDate();
        cout<<"precomputed && uptodate : ";
        if(ispre)
        {
          cout <<"yes : " << vvm->getPrecomputedDataName()<<endl;
          cout<<"timestamp of precom. data : "<<getDataSetDate(vvm->getPrecomputedDataName())<<endl;
        }
        else cout <<"no"<<endl;
      }
    }
  else if(command=="fields")
    {
      string dbname = argv[2];
      VMat vm = getDataSet(dbname);
      cout<<"FieldNames: "<<endl;
      for(int i=0;i<vm.width();i++)
        cout<<i<<": "<<vm->fieldName(i)<<endl;
    }
  else if(command=="stats")
    {
      string dbname = argv[2];
      VMat vm = getDataSet(dbname);
      displayBasicStats(vm);
    }
  else if(command=="gendef")
    {
      string dbname = argv[2];
      TVec<int> bins(argc-3);
      for(int i=3;i<argc;i++)
        bins[i-3]=toint(argv[i]);
      
      VMat vm = getDataSet(dbname);
      TVec<StatsCollector> sc = vm->getStats();
      // write stats file in metadatadir
      string name = vm->getMetaDataDir()+"/stats.def";
      ofstream sfile(name.c_str());
      for(int i=0;i<sc.size();i++)
        {
          sfile<<"DEFINE @"<<vm->fieldName(i)<<".mean "<<tostring(sc[i].mean())<<endl;
          sfile<<"DEFINE @"<<vm->fieldName(i)<<".stddev "<<tostring(sc[i].stddev())<<endl;
          sfile<<"DEFINE @"<<vm->fieldName(i)<<".stderr "<<tostring(sc[i].stderror())<<endl;
          sfile<<"DEFINE @"<<vm->fieldName(i)<<".min "<<tostring(sc[i].min())<<endl;
          sfile<<"DEFINE @"<<vm->fieldName(i)<<".max "<<tostring(sc[i].max())<<endl;
          sfile<<"DEFINE @"<<vm->fieldName(i)<<".normalized @"<<vm->fieldName(i)<<" @"<<vm->fieldName(i)<<".mean - @"<<
            vm->fieldName(i)<<".stddev /"<<endl;
          sfile<<"DEFINE @"<<vm->fieldName(i)<<".sum "<<tostring(sc[i].sum())<<endl;
          sfile<<"DEFINE @"<<vm->fieldName(i)<<".sumsquare "<<tostring(sc[i].sumsquare())<<endl;
          sfile<<"DEFINE @"<<vm->fieldName(i)<<".variance "<<tostring(sc[i].variance())<<endl;
        }
      for(int i=0;i<bins.size();i++)
        {
          int b=bins[i];
          string name = vm->getMetaDataDir()+"/bins"+tostring(b)+".def";
          ofstream bfile(name.c_str());
          RealMapping rm;
          for(int j=0;j<sc.size();j++)
            {
              rm = sc[j].getBinMapping(int(vm.length()/real(b)),int(vm.length()/real(b)));
              bfile<<"DEFINE @"<<vm->fieldName(j)<<".ranges"+tostring(b)+" "<<rm<<endl;
              bfile<<"DEFINE @"<<vm->fieldName(j)<<".ranges"+tostring(b)+".nbins "<<rm.size()<<endl;
              bfile<<"DEFINE @"<<vm->fieldName(j)<<".ranges"+tostring(b)+".nbins_m1 "<<rm.size()-1<<endl;
              bfile<<"DEFINE @"<<vm->fieldName(j)<<".binned"+tostring(b)+" @"<<vm->fieldName(j)<<" @"
                   <<vm->fieldName(j)<<".ranges"+tostring(b)<<endl;
              bfile<<"DEFINE @"<<vm->fieldName(j)<<".onehot"+tostring(b)+" @"<<vm->fieldName(j)<<".binned"
                +tostring(b)+" @"<<vm->fieldName(j)<<".ranges"+tostring(b)+".nbins onehot"<<endl;

            }
        }
    }
  else if(command=="genkfold")
    {
      if(argc<5)
        {
          cerr<<"usage vmat genkfold <source_dataset> <fileprefix> <kvalue>\n";
          exit(1);
        }
      string dbname = argv[2];
      string prefix = argv[3];
      int kval=toint(argv[4]);
      VMat vm = getDataSet(dbname);
      for(int i=0;i<kval;i++)
      {
        ofstream out((prefix+"_train_"+tostring(i+1)+".vmat").c_str());
        out<<"<SOURCES>"<<endl;
        int ntest = vm.length()/kval;
        int ntrain_before_test = i*ntest;
        int ntrain_after_test = vm.length()-(i+1)*ntest;
        if(ntrain_before_test>0)
          out<<dbname<<":0:"<<ntrain_before_test<<endl;
        if(ntrain_after_test>0)
          out<<dbname<<":"<<ntest+ntrain_before_test<<":"<<ntrain_after_test<<endl;
        out<<"</SOURCES>"<<endl;
        ofstream out2((prefix+"_test_"+tostring(i+1)+".vmat").c_str());
        out2<<"<SOURCES>"<<endl;
        out2<<dbname<<":"<<ntrain_before_test<<":"<<ntest<<endl;
        out2<<"</SOURCES>"<<endl;
      }
    }
  else if(command=="genvmat")
    {
      if(argc<5)
        {
          cerr<<"usage vmat genvmat <source_dataset> <dest_vmat> (binned{num} | onehot{num} | normalized)\n";
          exit(1);
        }
      string dbname = argv[2];
      string destvmat = argv[3];
      string type=argv[4];
      int typen= 0;
      int bins= 0;
      if(type.find("binned")!=string::npos)
        {
          typen=0;
          bins=toint(type.substr(6));
        }
      else if(type.find("onehot")!=string::npos)
        {
          typen=1;
          bins=toint(type.substr(6));
        }
      else if(type.find("normalized")!=string::npos)
        typen=2;
      else PLERROR("Unknown operation: %s",type.c_str());

      VMat vm = getDataSet(dbname);
      ofstream out(destvmat.c_str());
      
      out<<"<SOURCES>"<<endl;
      out<<dbname<<endl;
      out<<"</SOURCES>"<<endl;
      out<<"<PROCESSING>"<<endl;
      out<<"INCLUDE "<<dbname+".metadata/stats.def"<<endl;
      if(typen!=2)
        out<<"INCLUDE "<<dbname+".metadata/bins"<<bins<<".def"<<endl;

      for(int i=0;i<vm.width();i++)
        {
          switch(typen)
            {
            case 0:
              out<<"@"<<vm->fieldName(i)<<".binned"<<bins<<endl;
              out<<":"<<vm->fieldName(i)<<endl;
              break;
            case 1:
              out<<"@"<<vm->fieldName(i)<<".onehot"<<bins<<endl;
              out<<":"<<vm->fieldName(i)<<".:0:@"<<vm->fieldName(i)<<".ranges"<<bins<<".nbins_m1"<<endl;
              break;
            case 2:
              out<<"@"<<vm->fieldName(i)<<".normalized"<<endl;
              out<<":"<<vm->fieldName(i)<<endl;
              break;
            }

        }
      out<<"</PROCESSING>"<<endl;
      out.close();
    }
  else if(command=="diststat")
    {
      VMat vm = getDataSet(argv[2]);
      int inputsize = atoi(argv[3]);
      printDistanceStatistics(vm, inputsize);      
    }
  else if(command=="diff")
    {
      VMat vm1 = getDataSet(argv[2]);
      VMat vm2 = getDataSet(argv[3]);
      double tol = 1e-6;
      if(argc>=5)
        tol = atof(argv[5]);
      print_diff(cout, vm1, vm2, tol);      
    }
  else if(command=="cat")
    {
      if(argc!=4 && argc!=3)
        PLERROR("'vmat cat' must be used that way : vmat cat FILE [vplFilteringCode]");
      string dbname = argv[2];
      string code;
      VMat vm = getDataSet(dbname);
      Vec tmp(vm.width());
      if(argc==4) 

        {
          code=argv[3];
         
          VMatLanguage vpl(vm);
          vector<string> fn; 
          for(int i=0;i<vm->width();i++)
            fn.push_back(vm->fieldName(i));
          vpl.compileString(code,fn);
          Vec answer(1);
          for(int i=0;i<vm.length();i++)
            {
              vpl.run(i,answer);
              if(answer[0])
                cout<<tmp<<endl;
            }
        }
      else
          for(int i=0;i<vm.length();i++)
            {
              vm->getRow(i,tmp);      
              cout<<tmp<<endl;
            }
    }
  else if(command=="view")
    {
      if(argc > 5)
        PLERROR("Bad number of arguments. Syntax for option view: %s view <dbname> [<row> [<col>]]", argv[0]);
      VMat vm= getDataSet(string(argv[2]));
      viewVMat(vm, argc>=4? toint(argv[3]) : 0, argc==5? toint(argv[4]) : 0);
    }
  else if(command=="plot")
    {
      if(0 != argc%2)
	PLERROR("Bad number of arguments. Syntax for option plot:\n"
		"%s plot <dbname0> <col0>[:<row0>:<nrows0>] {<dbnameN> <colN>[:<rowN>:<nrowsN>]}", argv[0]);
      plotVMats(argv+2, argc-2);
    }
  else if(command=="help")
    {
      cout << getDataSetHelp() << endl;
    }
  else
      PLERROR("Unknown command : %s",command.c_str());
  return 0;
}

%> // end of namespace PLearn
