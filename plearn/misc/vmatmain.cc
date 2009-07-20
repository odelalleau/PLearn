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
 * $Id$
 ******************************************************* */

#include "vmatmain.h"
#include <commands/PLearnCommands/PLearnCommandRegistry.h>
#include <plearn/base/general.h>
#include <plearn/base/stringutils.h>
#include <plearn/base/lexical_cast.h>
#include <plearn/math/StatsCollector.h>
#include <plearn/math/stats_utils.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/SelectColumnsVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/vmat/VMatLanguage.h>
#include <plearn/vmat/VVMatrix.h>
#include <plearn/vmat/VMat.h>
#include <plearn/vmat/SelectRowsFileIndexVMatrix.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/db/getDataSet.h>
#include <plearn/display/Gnuplot.h>
#include <plearn/io/openFile.h>
#include <plearn/io/load_and_save.h>
#include <plearn/base/HelpSystem.h>
#include <plearn/base/PDate.h>
#include <algorithm>                         // for max
#include <iostream>
#include <iomanip>                           // for setw and such

namespace PLearn {
using namespace std;

/**
 * This function converts a VMat to a CSV (comma-separated value) file with
 * the given name.  One can also specify a list of column names or numbers
 * to keep, as well as whether any missing values on a row cause that row
 * to be skipped during export.  In addition, the number of significant
 * digits after the decimal period can be specified.
 *
 * If the 'convert_date' option is true (whose purpose is to convert CYYMMDD
 * dates into YYYYMMDD dates), then the integer 19000000 is added to the first
 * element of each row (assumed to contain a date column).
 */
static void save_vmat_as_csv(VMat source, ostream& destination,
                             bool skip_missings, int precision = 12,
                             string delimiter = ",",
                             bool verbose = true,
                             bool convert_date = false)
{
    char buffer[1000];
  
    // First, output the fieldnames in quoted CSV format.  Don't forget
    // to quote the quotes
    TVec<string> fields = source->fieldNames();
    for (int i=0, n=fields.size() ; i<n ; ++i) {
        string curfield = fields[i];
        search_replace(curfield, "\"", "\\\"");
        destination << '"' << curfield << '"';
        if (i < n-1)
            destination << delimiter;
    }
    destination << "\n";

    PP<ProgressBar> pb;
    if (verbose)
        pb = new ProgressBar(cout, "Saving to CSV", source.length());

    // Next, output each line.  Perform missing-value checks if required.
    for (int i=0, n=source.length() ; i<n ; ++i) {
        if (pb)
            pb->update(i+1);
        Vec currow = source(i);
        if (! skip_missings || ! currow.hasMissing()) {
            for (int j=0, m=currow.size() ; j<m ; ++j) {
                string strval="";
                if (convert_date && j==0)
                    // Date conversion: add 19000000 to convert from CYYMMDD to
                    // YYYYMMDD, and always output without trailing . if not
                    // necessary
                    sprintf(buffer, "%8f", currow[j] + 19000000.0);
                else if((strval=source->getValString(j,currow[j]))!=""){
                    search_replace(strval, "\"", "\\\"");
                    strval = "\"" + strval + "\"";
                    if(strval.length()>1000-1)
                        PLERROR("a value is too big!");
                    strncpy(buffer,strval.c_str(),1000);
                }else{
                    // Normal processing
                    sprintf(buffer, "%#.*f", precision, currow[j]);

                    // strip all trailing zeros and final period
                    // there is always a period since sprintf includes # modifier
                    char* period = buffer;
                    while (*period && *period != '.')
                        period++;
                    for (char* last = period + strlen(period) - 1 ;
                         last >= period && (*last == '0' || *last == '.') ; --last) {
                        bool should_break = *last == '.';
                        *last = '\0';
                        if (should_break)
                            break;
                    }
                }
                destination << buffer;
                if (j < m-1)
                    destination << delimiter;
            }
            destination << "\n";
        }
    }
}

  
/**
 * This function converts a VMat to a ARFF (Attribute-Relation File Format) file
 * with the given name.  One can also specify whether any missing values on a row
 * cause that row to be skipped during export.
 * In addition, the number of significant digits after the decimal period can be specified.
 *
 * The 'date_columns' option (if not empty) flags the specified columns as a
 * date.  Also, it converts the date from CYYMMDD to YYYYMMDD (if necessary).
 */
static void save_vmat_as_arff(VMat source, ostream& destination, TVec<string>& date_columns,
                             bool skip_missings, int precision = 12, bool verbose = true)
{
    PP<ProgressBar> pb;
    if (verbose)
        pb = new ProgressBar(cout, "Saving to ARFF", source.length());

    // First, write the ARFF specific file header
    destination << "@relation arff-database\n";
    TVec<string> fields = source->fieldNames();
    int nb_fields = fields.size();
    for (int i=0; i<nb_fields; ++i)
    {
        string curfield = fields[i];
        destination << "@attribute " << curfield << " ";
        map<string,real> string_map = source->getStringToRealMapping(i);
        if (date_columns.contains(curfield))  // e.g. @attribute start_date date "yyyyMMdd"
            destination << "date \"yyyyMMdd\"\n";
        else if (string_map.empty())  // e.g. @attribute Age numeric
            destination << "numeric\n";
        else  // e.g. @attribute Country {"Canada", "China", "Columbia"}
        {
            int nb_keys = string_map.size();
            int key_i = 0;
            destination << "{";
            map<string,real>::iterator it;
            for (it = string_map.begin(); it != string_map.end(); ++it)
            {
                string key = it->first;
                search_replace(key, "\"", "\\\"");
                destination << '"' << key << '"';
                if (key_i < nb_keys-1)
                    destination << ", ";
                key_i++;
            }
            destination << "}\n";
        }
    }
    destination << "@data\n";

    // Next, output each line.  Perform missing-value checks if required.
    const string delimiter = ",";
    const int buffer_size = 10000;
    char buffer[buffer_size];
    for (int i=0, n=source.length(); i<n; ++i) {
        if (pb)
            pb->update(i+1);

        // Skip missing values?
        Vec currow = source(i);
        if (skip_missings && currow.hasMissing())
            continue;

        for (int j=0, m=currow.size(); j<m; ++j)
        {
            string strval = "";
            // Date field
            if (date_columns.contains(fields[j]))
            {
                // Date conversion: add 19000000 to convert from CYYMMDD to
                // YYYYMMDD, and always output without trailing . if not
                // necessary
                real curfield = currow[j];
                if (is_missing(curfield)  ||  curfield==0)  // missing value
                {
                    strval = "?";
                    strncpy(buffer, strval.c_str(), buffer_size);
                }
                else
                {
                    if (curfield < 10000000)  // CYYMMDD format
                        curfield += 19000000.0;
                    sprintf(buffer, "%8d", int(curfield));
                }
            }
            // String mapped field
            else if ((strval=source->getValString(j,currow[j])) != "")
            {
                search_replace(strval, "\"", "\\\"");
                strval = "\"" + strval + "\"";
                if (strval.length()>buffer_size-1)
                    PLERROR("a value is too big!");
                strncpy(buffer, strval.c_str(), buffer_size);
            }
            // Numeric field
            else
            {
                real curfield = currow[j];
                if (is_missing(curfield))  // missing value
                {
                    strval = "?";
                    strncpy(buffer, strval.c_str(), buffer_size);
                }
                else
                {
                    // Normal processing
                    sprintf(buffer, "%#.*f", precision, currow[j]);

                    // strip all trailing zeros and final period
                    // there is always a period since sprintf includes # modifier
                    char* period = buffer;
                    while (*period && *period != '.')
                        period++;
                    for (char* last = period + strlen(period) - 1; last >= period && (*last == '0' || *last == '.'); --last)
                    {
                        bool should_break = *last == '.';
                        *last = '\0';
                        if (should_break)
                            break;
                    }
                }
            }
            destination << buffer;
            if (j < m-1)
                destination << delimiter;
        }
        destination << "\n";
    }
}

  
//! Prints where m1 and m2 differ by more than tolerance
//! returns the number of such differences, or -1 if the sizes differ
int print_diff(ostream& out, VMat m1, VMat m2, double tolerance, int verbose)
{
    int ndiff = 0;
    if(m1.length()!=m2.length() || m1.width()!=m2.width())
    {
        out << "Size of the two matrices differ: " 
            << m1.length() << " x " << m1.width() << "  vs.  "
            << m2.length() << " x " << m2.width() << endl;
        return -1;
    }
    if(m1->getFieldInfos()!=m2->getFieldInfos()){
        Array<VMField> a1=m1->getFieldInfos();
        Array<VMField> a2=m2->getFieldInfos();
        if(verbose)
            pout << "Field infos differ:";
        //compare fieldnames
        for(int i=0;i<m1.width();i++){
            if(a1[i].name!=a2[i].name){
                ++ndiff;
                if(verbose)
                    pout << " " << a1[i].name << "!=" << a2[i].name;
            }
        }
        //compare fieldtype
        for(int i=0;i<m1.width();i++){
            if(a1[i].fieldtype!=a2[i].fieldtype){
                ++ndiff;
                if(verbose)
                    pout << " " << a1[i].fieldtype << "!=" << a2[i].fieldtype;
            }
        }
        if(verbose)
            pout <<endl;
        
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
            if (!is_equal(v1[j], v2[j], 1.0, real(tolerance), real(tolerance)))
            {
                if (verbose)
                    out << "Elements at " << i << ',' << j << " differ by "
                        << v1[j] - v2[j] << endl;
                ++ndiff;
            } else if (m1->getValString(j, v1[j]) != m2->getValString(j, v2[j])) {
                if (verbose)
                    out << "Elements at " << i << ',' << j << " differ: "
                        << "'" << m1->getValString(j, v1[j]) << "' != "
                        << "'" << m2->getValString(j, v2[j]) << "'" << endl;
                ++ndiff;
            }
        }
    }
    if (!verbose) out << ndiff <<endl;
    return ndiff;
}

void interactiveDisplayCDF(const Array<VMat>& vmats)
{
    int k = vmats.size();
    int w = vmats[0]->width();

    Array<string> name(k);
    pout << ">>>> Dimensions of vmats: \n";
    for(int i=0; i<k; i++)
    {
        name[i] = vmats[i]->getMetaDataDir();
        name[i] = name[i].erase(name[i].size()-10);
        pout << name[i] << ": \t " << vmats[i]->length() << " x " << vmats[i]->width() << endl;
    }

    vmats[0]->printFields(pout);

    Gnuplot gp;

    for(;;)
    {
        // TVec<RealMapping> ranges = vm->getRanges();

        vector<string> command;
        int varnum = -1;
        real low = -FLT_MAX; // means autorange
        real high = FLT_MAX; // means autorange
        do
        {
            pout << "Field (0.." << w-1 << ") [low high] or exit? " << flush;
            command = split(pgetline(cin));
            if(command.size()==0)
                vmats[0]->printFields(pout);
            else if(command.size()==1&&command[0]=="exit")
                exit(0);
            else
            {
                varnum = vmats[0]->getFieldIndex(command[0],false);
                if(varnum == -1)
                    pout<<"Bad column name or number("<<command[0]<<")"<<endl;
                if(varnum<0 || varnum>=w)
                    vmats[0]->printFields(pout);
                else if(command.size()==3)
                {
                    low = toreal(command[1]);
                    high = toreal(command[2]);
                }
            }
        } while(varnum<0 || varnum>=w);


        pout << "\n\n*************************************" << endl;
        pout << "** #" << varnum << ": " << vmats[0]->fieldName(varnum) << " **" << endl;
        pout << "*************************************" << endl;

        Array<Mat> m(k);

        for(int i=0; i<k; i++)
        {
            TVec<StatsCollector> stats = vmats[i]->getStats();        
            StatsCollector& st = stats[varnum];
            m[i] = st.cdf(true);
            pout << "[ " << name[i]  << " ]" << endl;
            pout << st << endl;
        }
        // pout << "RANGES: " << endl;
        // pout << ranges[varnum];

        if(is_equal(low,-FLT_MAX))
            gp << "set xrange [*:*]" << endl;      
        else
            //The -0.01 and 0.01 is to clearly see the end value.
            gp << "set xrange [" << low-0.01 << ":" << high+0.01 << "]" << endl;

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
    TVec<StatsCollector> stats = vm->getStats(true);        

    // find longest field name
    size_t fieldlen = 0;
    for (int k=0; k<nfields; ++k)
        fieldlen = max(fieldlen, vm->fieldName(k).size());
    fieldlen++;
  
    cout << std::left << setw(6)  << "# "
         << setw(int(fieldlen)) << " fieldname " << std::right
         << setw(15) << " mean "
         << setw(15) << " stddev "
         << setw(15) << " min "
         << setw(15) << " max "
         << setw(15) << " count "
         << setw(15) << " nmissing "
         << setw(15) << " stderr" << endl; 
    for(int k=0; k<nfields; k++)
    {
        cout << std::left << setw(6)  << k << " " 
             << setw(int(fieldlen)) << vm->fieldName(k) << " " << std::right
             << setw(15) << stats[k].mean() << " " 
             << setw(15) << stats[k].stddev() << " "
             << setw(15) << stats[k].min() << " " 
             << setw(15) << stats[k].max() << " " 
             << setw(15) << stats[k].n() << " " 
             << setw(15) << stats[k].nmissing() << " " 
             << setw(15) << stats[k].stderror() << " " 
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

    pout << "Euclidean distance statistics: " << endl;
    pout << collector << endl;
}

/*
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
*/

/*
  int findNextIndexOfValue(VMat m, int col, real value, int startrow=0)
  {
  if(m->hasMetaDataDir())
  {
  string fpath = apppend_slash(m->getMetaDataDir())+"CachedColumns/"+tostring(col);
  if(!isfile(filepath))
        
      
  }
  }
*/


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
  
    pout << "Press any key to close GNUplot window and exit." << endl;
    cin.get();
}


VMat getVMat(const PPath& source, const PPath& indexf)
{
    VMat vm= getDataSet(source);
    if(indexf != "")
        vm= new SelectRowsFileIndexVMatrix(vm, indexf);
    return vm;
}


int vmatmain(int argc, char** argv)
{
  
    if(argc<3)
    {
        // Use the VMatCommand help instead of repeating the same help message twice...
        // help message in file commands/PLearnCommands/VMatCommand.cc
        pout << HelpSystem::helpOnCommand("vmat") << flush;
        exit(0);
    }

    PPath indexf= "";
    if(string(argv[1])=="-i")
    {
        indexf= argv[2];
        argv+= 2;//skip -i and indexfile name
    }

    string command = argv[1];

    if(command=="cdf")
    {      
        Array<VMat> vmats;
        for(int i=2; i<argc; i++)
        {
            string dbname = argv[i];
            VMat vm = getVMat(dbname, indexf);
            vmats.append(vm);
        }
        interactiveDisplayCDF(vmats);
    }
    /*
      else if(command=="cond")
      {
      string dbname = argv[2];
      VMat vm = getDataSet(dbname);
      cout << "** Using dataset: " << dbname << " **" << endl;
      cout << "Metadata for this dataset in: " << vm->getMetaDataDir() << endl;
      int condfield = atoi(argv[3]);
      printConditionalStats(vm, condfield);    
      }
    */
    else if(command=="convert")
    {
        if(argc<4)
            PLERROR("Usage: vmat convert <source> <destination> "
                    "[--mat_to_mem] [--cols=col1,col2,col3,...] [--save_vmat] [--skip-missings] [--precision=N] [--delimiter=CHAR] [--force_float] [--auto_float]");

        PPath source = argv[2];
        PPath destination = argv[3];
        bool mat_to_mem = false;
        if(source==destination)
            PLERROR("You are overwriting the source. This is not allowed!");
        /**
         * Interpret the following options:
         *
         *     --cols=col1,col2,col3,...
         *           :: keep only the given columns (no space between the commas
         *              and the columns); columns can be given either as a number
         *              (zero-based) or a string.  You can also specify a range,
         *              such as 0-18, or any combination thereof.
         *
         *     --skip-missings
         *           :: if a row (after selecting the appropriate columns)
         *              contains one or more missing values, skip it during export
         *
         *     --precision=N
         *           :: conversion to CSV keeps N digits AFTER THE DECIMAL POINT
         *
         *     --delimiter=CHAR
         *           :: conversion to CSV uses specified character as field delimiter
         *     --mat_to_mem
         *           :: load the source vmat in memory before saving
         *     --save_vmat 
         *           :: if the source is a vmat, we serialize the constructed
         *           :: object in the metadatadir of the destination
         *     --update
         *           :: we generate the <destination> only when the <source> file is newer than
         *           :: the destination  file or when the destination file is missing
         *     --force_float
         *           :: if the destination is a pmat, we force the pmat file to be in float format
         *     --auto_float
         *           :: if the destination is a pmat, we will store the data in float format if this don't loose any precision compared to double format.
         */
        TVec<string> columns;
        TVec<string> date_columns;
        bool skip_missings = false;
        int precision = 12;
        string delimiter = ",";
        bool convert_date = false;
        bool save_vmat = false;
        bool update = false;
        bool force_float = false;
        bool auto_float = false;

        string ext = extract_extension(destination);

        for (int i=4 ; i < argc && argv[i] ; ++i) {
            string curopt = removeblanks(argv[i]);
            if (curopt == "")
                continue;
            if (curopt.substr(0,7) == "--cols=") {
                string columns_str = curopt.substr(7);
                columns = split(columns_str, ',');
            }
            else if (curopt.substr(0,12) == "--date-cols=") {
                string columns_str = curopt.substr(12);
                date_columns = split(columns_str, ',');
            }
            else if (curopt == "--skip-missings")
                skip_missings = true;
            else if (curopt.substr(0,12) == "--precision=") {
                precision = toint(curopt.substr(12));
            }
            else if (curopt.substr(0,12) == "--delimiter=") {
                PLCHECK(ext==".cvs");
                delimiter = curopt.substr(12);
            }
            else if (curopt == "--convert-date")
                convert_date = true;
            else if (curopt =="--mat_to_mem")
                mat_to_mem = true;
            else if (curopt == "--save_vmat")
                save_vmat = true;
            else if (curopt == "--update")
                update = true;
            else if (curopt == "--force_float"){
                PLCHECK(ext==".pmat");
                force_float = true;
            }else if (curopt == "--auto_float"){
                PLCHECK(ext==".pmat");
                auto_float = true;
            }else
                PLWARNING("VMat convert: unrecognized option '%s'; ignoring it...",
                          curopt.c_str());
        }
        VMat vm = getVMat(source, indexf);

        // If columns specified, select them.  Note: SelectColumnsVMatrix is very
        // powerful and allows ranges, etc.
        if (columns.size() > 0)
            vm = new SelectColumnsVMatrix(vm, columns);

        if (ext != ".csv" && skip_missings)
            PLWARNING("Option '--skip-missings' not supported for extension '%s'; ignoring it...",
                      ext.c_str());
        if(mat_to_mem)
            vm.precompute();
        if(update && vm->isUpToDate(destination))
            pout << "The file is up to date. We don't regenerate it."<<endl;
        else if(ext==".amat")
            // Save strings as strings so they are not lost.
            vm->saveAMAT(destination, true, false, true);
        else if(ext==".cmat")
            vm->saveCMAT(destination);
        else if(ext==".pmat")
            vm->savePMAT(destination, force_float, auto_float);
        else if(ext==".dmat")
            vm->saveDMAT(destination);
        else if(ext == ".csv")
        {
            if (destination == "-.csv")
                save_vmat_as_csv(vm, cout, skip_missings, precision, delimiter, true /*verbose*/, convert_date);
            else
            {
                ofstream out(destination.c_str());
                save_vmat_as_csv(vm, out, skip_missings, precision, delimiter, true /*verbose*/, convert_date);
            }
        }
        else if(ext == ".arff")
        {
            ofstream out(destination.c_str());
            save_vmat_as_arff(vm, out, date_columns, skip_missings, precision);
        }
        else if(ext == ".vmat")
            PLearn::save(destination,vm);
        else
        {
            cerr << "ERROR: can only convert to .amat .pmat .dmat, .vmat or .csv" << endl
                 << "Please specify a destination name with a valid extension " << endl;
        }
        if(save_vmat && extract_extension(source)==".vmat")
            PLearn::save(destination+".metadata/orig.vmat",vm);
        else if(save_vmat)
            PLWARNING("We haven't saved the original file as it is not a vmat");
    }
    else if(command=="info")
    {
        for(int i=2;i<argc;i++){
            string dbname = argv[i];
            VMat vm = getVMat(dbname, indexf);
            if(argc>3)
                pout<<dbname<<endl;
            pout<<vm.length()<<" x "<<vm.width()<<endl;
            pout << "inputsize: " << vm->inputsize() << endl;
            pout << "targetsize: " << vm->targetsize() << endl;
            pout << "weightsize: " << vm->weightsize() << endl;
            pout << "extrasize: " << vm->extrasize() << endl;
            VVMatrix * vvm = dynamic_cast<VVMatrix*>((VMatrix*)vm);
            if(vvm!=NULL)
            {
                pout<< "Last modification (including dependencies of .vmat): "
                    << int32_t(vvm->getMtime()) << endl;
                bool ispre=vvm->isPrecomputedAndUpToDate();
                pout<<"precomputed && uptodate : ";
                if(ispre)
                {
                    pout <<"yes : " << vvm->getPrecomputedDataName()<<endl;
                    pout<< "timestamp of precom. data : "
                        <<int32_t(getDataSetDate(vvm->getPrecomputedDataName()))
                        << endl;
                }
                else pout <<"no"<<endl;
            }
        }
    }
    else if(command=="fields")
    {
        bool add_info = true;
        bool transpose = false;
        if (argc >= 4) {
            add_info = !(string(argv[3]) == "name_only");
        }
        if (argc >= 5) {
            transpose = (string(argv[4]) == "transpose");
        }
        string dbname = argv[2];
        VMat vm = getVMat(dbname, indexf);
        if (add_info) {
            pout<<"FieldNames: ";
            if (!transpose) {
                pout << endl;
            }
        }
        for(int i=0;i<vm.width();i++) {
            if (add_info) {
                pout << i << ": ";
            }
            pout << vm->fieldName(i);
            if (transpose) {
                pout << " ";
            } else {
                pout << endl;
            }
        }
        if (transpose) {
            // It misses a carriage return after everything is displayed.
            pout << endl;
        }
    }
    else if(command=="fieldinfo")
    {
        if (argc < 4)
            PLERROR("The 'fieldinfo' subcommand requires more parameters, please check the help");
        string dbname = argv[2];
        string fieldname_or_num = argv[3];

        bool print_binning = false;
        if (argc == 5) {
            if (argv[4] == string("--bin"))
                print_binning = true;
            else
                PLERROR("vmat fieldinfo: unrecognized final argument; can be '--bin' "
                        "to print the binning");
        }
        
        VMat vm = getVMat(dbname, indexf);
        vm->printFieldInfo(pout, fieldname_or_num, print_binning);
    }
    else if(command=="stats")
    {
        string dbname = argv[2];
        VMat vm = getVMat(dbname, indexf);
        displayBasicStats(vm);
    }
    else if(command=="gendef")
    {
        string dbname = argv[2];
        TVec<int> bins(argc-3);
        for(int i=3;i<argc;i++)
            bins[i-3]=toint(argv[i]);
      
        VMat vm = getVMat(dbname, indexf);
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
            PPath f_name = vm->getMetaDataDir() / "bins"+tostring(b)+".def";
            PStream bfile = openFile(f_name, PStream::raw_ascii, "w");
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
        VMat vm = getVMat(dbname, indexf);
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

        VMat vm = getVMat(dbname, indexf);
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
        VMat vm = getVMat(argv[2], indexf);
        int inputsize = atoi(argv[3]);
        printDistanceStatistics(vm, inputsize);      
    }
    else if(command=="diff")
    {
        if(argc < 4)
            PLERROR("'vmat diff' must be used that way : vmat diff <dataset1> <dataset2> [<tolerance> [<verbose>]]");

        VMat vm1 = getVMat(argv[2], indexf);
        VMat vm2 = getVMat(argv[3], indexf);
        double tol = 1e-6;
        int verb = 1;
        if(argc >= 5)
            tol = atof(argv[4]);
        if (argc >= 6)
            verb = atoi(argv[5]);
        print_diff(cout, vm1, vm2, tol, verb);      
    }
    else if(command=="cat")
    {
        if(argc < 3)
            PLERROR("'vmat cat' must be used that way : vmat cat FILE... [--precision=N] [vplFilteringCode]");
        string code;
        int nb_file=argc-2;
        int precision = -1;
        for (int i=argc-1 ; i >=3 && argv[i] ; i--) {
            string curopt = removeblanks(argv[i]);
            if(curopt.substr(0,12) == "--precision="){
                precision = toint(curopt.substr(12));
                nb_file--;
            }else if(!isfile(argv[argc-1])){
                code=argv[argc-1];
                nb_file--;
            }
        }
        if(precision>0){
            char tmpbuf[100];
            snprintf(tmpbuf,100,"%%#.%df",precision);
            pout.setDoubleFormat(tmpbuf);
            pout.setFloatFormat(tmpbuf);
        }
        for(int file=0;file<nb_file;file++)
        {
            string dbname=argv[file+2];
            if(nb_file>1)
                pout<<dbname<<endl;
            VMat vm = getVMat(dbname, indexf);
            Vec tmp(vm.width());
            if(code.length()>0){
                VMatLanguage vpl(vm);
                vector<string> fn; 
                for(int i=0;i<vm->width();i++)
                    fn.push_back(vm->fieldName(i));
                vpl.compileString(code,fn);
                Vec answer(1);
                for(int i=0;i<vm.length();i++)
                {
                    vpl.run(i,answer);
                    if(!fast_exact_is_equal(answer[0], 0)) {
                        vm->getRow(i, tmp);
                        pout<<tmp<<endl;
                    }
                }

            }
            else
                for(int i=0;i<vm.length();i++)
                {
                    vm->getRow(i,tmp);      
                    pout<<tmp<<endl;
                }
        }
    }
    else if(command=="catstr")
    {
        if(argc!=3 && argc != 4)
            PLERROR("'vmat catstr' must be used that way : vmat cat FILE [separator]");
        string dbname = argv[2];
        string sep = "\t";
        if(argc==4)
            sep = argv[3];
        VMat vm = getVMat(dbname, indexf);
        Vec tmp(vm.width());
        string out = "";
        for(int i=0;i<vm.length();i++)
        {
            vm->getRow(i,tmp);
            for(int j=0; j<vm.width(); j++)
            {
                out = vm->getValString(j,tmp[j]);
                if(out == "") out = tostring(tmp[j]);
                pout << out << sep;
            }
            pout << endl;
        }
    }
    else if(command=="sascat")
    {
        if(argc!=4)
            PLERROR("'vmat sascat' must be used that way : vmat sascat <in-dataset> <out-filename.txt>");
        string dbname = argv[2];
        string outname = argv[3];
        string code;
        VMat vm = getVMat(dbname, indexf);
        ofstream out(outname.c_str());
        for (int i=0;i<vm.width();i++)
            out << vm->fieldName(i) << "\t";
        out << endl;
        for(int i=0;i<vm.length();i++)
        {
            for (int j=0;j<vm.width();j++)
                out << vm->getString(i,j) << "\t";
            out<<endl;
        }
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
        pout << getDataSetHelp() << endl;
    }
    else if(command=="compare_stats")
    {
        if(!(argc==4||argc==5||argc==6))
            PLERROR("vmat compare_stats must be used that way: vmat compare_stats <dataset1> <dataset2> [[stderror threshold [missing threshold]]");

        VMat m1 = getVMat(argv[2], indexf);
        VMat m2 = getVMat(argv[3], indexf);

        m1->compatibleSizeError(m2);

        real stderror_threshold = 1;
        real missing_threshold = 10;
        if(argc>4)
            stderror_threshold=toreal(argv[4]);
        if(argc>5)
            missing_threshold=toreal(argv[5]);
        Vec missing(m1->width());
        Vec stderror(m1->width());

        pout << "Test of difference that suppose gaussiane variable"<<endl;
        m1->compareStats(m2, stderror_threshold, missing_threshold,
                         stderror, missing);

        Mat score(m1->width(),3);

        for(int col = 0;col<m1->width();col++)
        {
            score(col,0)=col;
            score(col,1)=stderror[col];
            score(col,2)=missing[col];
        }
        
        int nbdiff = 0;

        pout<<"Print the field that do not pass the threshold sorted by the stderror"<<endl;
        sortRows(score,1,false);
        for(int i=0;i<score.length();i++)
        {
            if(score(i,1)>stderror_threshold)
            {
                const StatsCollector tstats = m1->getStats(i);
                const StatsCollector lstats = m2->getStats(i);
                real tmean = tstats.mean();
                real lmean = lstats.mean();
                real tstderror = sqrt(pow(tstats.stderror(), 2) + 
                                      pow(lstats.stderror(), 2));

                pout<<i<<"("<<m1->fieldName(int(round(score(i,0))))<<")"
                    <<" differ by "<<score(i,1)<<" stderror."
                    <<" The mean is "<<lmean<<" while the target mean is "<<tmean
                    <<" and the used stderror is "<<tstderror<<endl;
                nbdiff++;
            }
        }

        cout<<"Print the field that do not pass the threshold sorted by the missing error"<<endl;
        sortRows(score,2,false);
        for(int i=0;i<score.length();i++)
        {
            if(score(i,2)>missing_threshold)
            {
                const StatsCollector tstats = m1->getStats(i);
                const StatsCollector lstats = m2->getStats(i);
                real tmissing = tstats.nmissing()/tstats.n();
                real lmissing = lstats.nmissing()/lstats.n();
                pout<<i<<"("<<m1->fieldName(int(round(score(i,0))))<<")"
                    <<" The missing stats difference is "<< score(i,2)
                    <<". There are "<<lmissing<<" missing while target has "
                    <<tmissing<<" missing."<<endl;
                nbdiff++;
            }
        }

        pout<<"There are "<<nbdiff<<"/"<<m1.width()
            <<" fields that have different stats"<<endl;

    }
    else if(command=="compare_stats_ks")
    {
        bool err = false;
        real threshold = REAL_MAX;
        bool mat_to_mem = false;
        if(argc<4||argc>6)
            err = true;
        if(argc==5)
        {
            if(argv[4]==string("--mat_to_mem"))
                mat_to_mem=true;
            else if(!pl_isnumber(string(argv[4]),&threshold))
                err = true;
        }
        else if(argc==6)
        {
             if(argv[5]!=string("--mat_to_mem"))
                 err = true;
             else if(!pl_isnumber(string(argv[4]),&threshold))
                 err = true;
        }
        if(err)
            PLERROR("vmat compare_stats_ks must be used that way:"
                    " vmat compare_stats_ks <dataset1> <dataset2> [threshold]"
                    " [--mat_to_mem]");

        VMat m1 = getVMat(argv[2], indexf);
        VMat m2 = getVMat(argv[3], indexf);
        if(mat_to_mem)
        {
            m1.precompute();
            m2.precompute();
        }

        m1->compatibleSizeError(m2);
        int pc_value_99=0;
        int pc_value_95=0;
        int pc_value_90=0;
        int pc_value_0=0;

        uint size_fieldnames=m1->maxFieldNamesSize();

        Vec Ds(m1->width());
        Vec p_values(m1->width());
        KS_test(m1,m2,10,Ds,p_values,true);
        Mat score(m1->width(),3);
            
        for(int col = 0;col<m1->width();col++)
        {
            score(col,0)=col;
            score(col,1)=Ds[col];
            real p_value = p_values[col];
            score(col,2)=p_value;
            if(p_value>0.99)
                pc_value_99++;
            if(p_value>0.95)
                pc_value_95++;
            if(p_value>0.90)
                pc_value_90++;
            else
                pc_value_0++;
        }

        sortRows(score,2,false);
        pout <<"Kolmogorov Smirnov two sample test"<<endl<<endl;
        if(threshold<REAL_MAX)
            pout<<"Variables that are under the threshold"<<endl;
        pout<<"Sorted by p_value"<<endl;
        cout << std::left << setw(8) << "# "
             << setw(size_fieldnames) << " fieldname " << std::right
             << setw(15) << " D"
             << setw(15) << " p_value"
             <<endl;
        int threshold_fail=0;
        for(int col=0;col<score.length();col++)
        {
            if(threshold>=score(col,2))
            {
                cout << std::left << setw(8) << tostring(col)+"/"+tostring(score(col,0))
                     << setw(size_fieldnames) << m1->fieldName(int(round(score(col,0))))
                     << std::right
                     << setw(15) << score(col,1)
                     << setw(15) << score(col,2)
                     <<endl;
                threshold_fail++;
            }
        }
        if(threshold<REAL_MAX)
            pout << "There are "<<threshold_fail<<" variables that are under the threshold"<<endl;
        if(threshold==REAL_MAX)
        {
            pout << "99% cutoff: "<<pc_value_99<<endl;
            pout << "95% cutoff: "<<pc_value_95<<endl;
            pout << "90% cutoff: "<<pc_value_90<<endl;
            pout << "0-90% cutoff: "<<pc_value_0<<endl;
        }
        pout <<"Kolmogorov Smirnov two sample test end"<<endl<<endl;
    }
    else if(command=="compare_stats_desjardins")
    {      
        bool err=false;
        bool mat_to_mem=false;
        if(!(argc==8||argc==9))
            err=true;
        if(argc==9)
        {
            if(argv[8]!=string("--mat_to_mem"))
                 err = true;
            else
                mat_to_mem=true;
        }
        if(err)
            PLERROR("vmat compare_stats_desjardins must be used that way:"
                    " vmat compare_stats_desjardins <orig dataset1> <orig dataset2> <new dataset3> <ks_threshold> <stderror_threshold> <missing_threshold> [--mat_to_mem]");

        VMat m1 = getVMat(argv[2], indexf);
        VMat m2 = getVMat(argv[3], indexf);
        VMat m3 = getVMat(argv[4], indexf);
        real ks_threshold = toreal(argv[5]);

        m3->compatibleSizeError(m1);
        m3->compatibleSizeError(m2);

        Vec Ds(m1->width());
        Vec p_values(m1->width());
        Mat score(m1->width(),3);
        uint size_fieldnames=m1->maxFieldNamesSize();
        if(mat_to_mem==true)
        {
            m1.precompute();
            m2.precompute();
            m3.precompute();
        }

        KS_test(m1,m3,10,Ds,p_values,true);
        for(int col = 0;col<m1->width();col++)
        {
            score(col,0)=col;
            score(col,1)=Ds[col];
            real p_value = p_values[col];
            score(col,2)=p_value;
        }

        KS_test(m2,m3,10,Ds,p_values,true);
        for(int col = 0;col<m1->width();col++)
        {
            if(p_values[col]>score(col,2))
            {
                score(col,1)=Ds[col];
                score(col,2)=p_values[col];
            }
        }

        sortRows(score,2,false);
        pout <<"Kolmogorov Smirnov two sample test"<<endl<<endl;
        pout<<"Variables that are under the ks_threshold"<<endl;
        pout<<"Sorted by p_value"<<endl;
        cout << std::left << setw(8) << "# "
             << setw(size_fieldnames) << " fieldname " << std::right
             << setw(15) << " D"
             << setw(15) << " p_value"
             <<endl;
        int threshold_fail = 0;
        for(int col=0;col<score.length();col++)
        {
            if(ks_threshold>=score(col,2))
            {
                cout << std::left << setw(8) << tostring(col)+"/"+tostring(score(col,0))
                     << setw(size_fieldnames) << m1->fieldName(int(round(score(col,0))))
                     << std::right
                     << setw(15) << score(col,1)
                     << setw(15) << score(col,2)
                     <<endl;
                threshold_fail++;
            }
        }
        pout << "There are "<<threshold_fail<<"/"<<m1->width()<<
            " variables that are under the threshold"<<endl<<
            " Kolmogorov Smirnov two sample test end"<<endl<<endl;


//         real stderror_threshold = 1;
//         real missing_threshold = 10;
//         stderror_threshold=toreal(argv[6]);
//         missing_threshold=toreal(argv[7]);

//         pout << "Test of difference that suppose gaussiane variable"<<endl;
//         pout << "Comparing with dataset1"<<endl;

//         m3->compareStats(m1, stderror_threshold, missing_threshold,
//                          stderr, missing);
//         pout << "Comparing with dataset2"<<endl;
//         m3->compareStats(m2, stderror_threshold, missing_threshold,
//                          stderr, missing);
//         pout<<"There are "<<diff<<"/"<<m1.width()
//             <<" fields that have different stats"<<endl;
    }
    else if(command=="characterize")
    {
        if(argc!=3)
            PLERROR("The command 'vmat characterize' must be used that way: vmat caracterize <dataset1>");
        VMat m1 = getVMat(argv[2], indexf);
        TVec<StatsCollector> stats = 
            m1->getStats();//"stats_all.psave",-1,true);
        TVec<string> caracs;
        uint size_fieldnames=m1->maxFieldNamesSize();

        for(int i=0;i<stats.size();i++)
        {
            StatsCollector& stat=stats[i];
            string carac = tostring(i)+"\t"+left(m1->fieldName(i),size_fieldnames);
            if(stat.isbinary())
                carac += "\tBinary";
            else if(stat.isinteger())
                carac+="\tInteger";
            else
                carac+="\tReal";

            //find is normal or not
            int m=min(100,int(round(sqrt(stat.nnonmissing()))));
            int nelem=int(stat.nnonmissing()/m);
            int row=0;
            real gsum=0;
            for(int bloc=0;bloc<m;bloc++)
            {
                real bloc_sum=0;
                for(int bloc_elem=0;bloc_elem<nelem;)
                {
                    real v = m1->get(row,i);
                    if(is_missing(v))
                        continue;
                    else
                    {
                        bloc_elem++;
                        row++;
                        bloc_sum+=v;
                    }
                    
                }
                gsum+=pow((bloc_sum/nelem)-stat.mean(),2);
            }
            real s2=(stat.variance()/nelem);
            real mu_square = stat.mean()*stat.mean();
            real th = ((1./(m-1))*gsum)/s2;
            real th2= mu_square+s2-12*mu_square*s2+mu_square*mu_square
                * s2*s2;
            bool b = th>(s2+2*th2)/s2;
                carac+="\tnormal test value: "+tostring(th)+" "+tostring(th2)
                + " "+tostring(b);
            caracs.append(carac);
            pout<<carac<<endl;
        }

    }
    else if(command=="mtime")
    {    
        if(argc!=3)
            PLERROR("The command 'vmat mtime' must be used that way: vmat mtime <dataset>");
        VMat m1 = getVMat(argv[2], indexf);
        pout<<m1->getMtime()<<endl;
    }
    else
        PLERROR("Unknown command : %s",command.c_str());
    return 0;
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
