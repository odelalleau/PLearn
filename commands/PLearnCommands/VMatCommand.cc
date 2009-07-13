
// -*- C++ -*-

// VMatCommand.cc
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

/*! \file VMatCommand.cc */
#include <plearn/misc/vmatmain.h>
#include "VMatCommand.h"
#include <plearn/db/getDataSet.h>
#include <plearn/base/lexical_cast.h>
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/base/plerror.h>
#include <plearn/io/fileutils.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'VMatCommand' command in the command registry
PLearnCommandRegistry VMatCommand::reg_(new VMatCommand);

VMatCommand::VMatCommand():
    PLearnCommand(
        "vmat",
        "Examination and manipulation of vmat datasets",
        "Usage: vmat info <dataset> \n"
        "       Will info about dataset (size, etc..)\n"
        "   or: vmat fields <dataset> [name_only] [transpose] \n"
        "       To list the fields with their names (if 'name_only' is specified, the indexes won't be displayed,\n"
        "       and if 'transpose' is also added, the fields will be listed on a single line)\n"
        "   or: vmat fieldinfo <dataset> <fieldname_or_num> [--bin]\n"
        "       To display statistics for that field \n"
        "   or: vmat bbox <dataset> [<extra_percent>] \n"
        "       To display the data bounding box (i.e., for each field, its min and max, possibly extended by +-extra_percent ex: 0.10 for +-10% of the data range )\n"
        "   or: vmat cat <dataset>... [--precision=N] [<optional_vpl_filtering_code>]\n"
        "       To display the dataset \n"
        "   or: vmat sascat <dataset.vmat> <dataset.txt>\n"
        "       To output in <dataset.txt> the dataset in SAS-like tab-separated format with field names on the first line\n"
        "   or: vmat view <dataset>...\n"
        "       Interactive display to browse on the data. \n"
        "       ( will work only if your executable includes commands/PLearnCommands/VMatViewCommand.h )\n"
        "   or: vmat stats <dataset> \n"
        "       Will display basic statistics for each field \n"
        "   or: vmat convert <source> <destination> [--cols=col1,col2,col3,...] [--mat_to_mem] [--save_vmat] [--force_float]\n"
        "       To convert any dataset into a .amat, .pmat, .dmat, .vmat, .csv or .arff format. \n"
        "       The extension of the destination is used to determine the format you want. \n"
        "       WARNING: In dmat format, all double are currently casted to float!\n"
        "       If the option --cols is specified, it requests to keep only the given columns\n"
        "       (no space between the commas and the columns); columns can be given either as a\n"
        "       number (zero-based) or a column name (string).  You can also specify a range,\n"
        "       such as 0-18, or any combination thereof, e.g. 5,3,8-18,Date,74-85\n"
        "       If the option --mat_to_mem is specified, we load the original matrix into memory\n"
        "       If the option --save_vmat is specified, we save the source vmat in the destination metadatadir\n"
        "       If the option --update is specified, we generate the <destination> only when the <source> file is newer\n"
        "         then the destination file or when the destination file is missing\n"
        "       If .pmat is specified as the destination file, the option --force_float will save the data in float format\n"
        "       If .csv (Comma-Separated Value) is specified as the destination file, the \n"
        "       following additional options are also supported:\n"
        "         --skip-missings: if a row (after selecting the appropriate columns) contains\n"
        "                          one or more missing values, it is skipped during export\n"
        "         --precision=N:   a maximum of N digits is printed after the decimal point\n"
        "         --delimiter=C:   use character C as the field delimiter (default = ',')\n"
        "         --convert-date:  first column is assumed to be in CYYMMDD format; it is\n"
        "                          exported as YYYYMMDD in the .csv file (19000000 is added)\n"
        "       If .arff (Attribute-Relation File Format) is specified as the destination file, the \n"
        "       following additional options are also supported:\n"
        "         --skip-missings: if a row (after selecting the appropriate columns) contains\n"
        "                          one or more missing values, it is skipped during export\n"
        "         --precision=N:   a maximum of N digits is printed after the decimal point\n"
        "         --date-cols=col1,col2,...:  we flag the specified columns as a date\n"
        "                                     we also convert the date from CYYMMDD to YYYYMMDD (if necessary)\n"
        "   or: vmat gendef <source> [binnum1 binnum2 ...] \n"
        "       Generate stats for dataset (will put them in its associated metadatadir). \n"
        "   or: vmat genvmat <source_dataset> <dest_vmat> [binned{num} | onehot{num} | normalized]\n"
        "       Will generate a template .vmat file with all the fields of the source preprocessed\n"
        "       with the processing you specify\n"
        "   or: vmat genkfold <source_dataset> <fileprefix> <kvalue>\n"
        "       Will generate <kvalue> pairs of .vmat that are splitted so they can be used for kfold trainings\n"
        "       The first .vmat-pair will be named <fileprefix>_train_1.vmat (all source_dataset except the first 1/k)\n"
        "       and <fileprefix>_test_1.vmat (the first 1/k of <source_dataset>\n"
        "   or: vmat diff <dataset1> <dataset2> [<tolerance> [<verbose>]]\n"
        "       Will report all elements that differ by more than tolerance (default = 1e-6),\n"
        "       in an absolute way for numbers less than 1, and in a relative way otherwise.\n"
        "       If verbose==0 then print only total number of differences.\n"
        "   or: vmat cdf <dataset> [<dataset> ...] \n"
        "       To interactively display cumulative density function for each field \n"
        "       along with its basic statistics \n"
        // "   or: vmat cond <dataset> <condfield#> \n"
        // "       Interactive display of coditional statistics conditioned on the \n"
        // "       conditioning field <condfield#> \n"
        "   or: vmat diststat <dataset> <inputsize>\n"
        "       Will compute and output basic statistics on the euclidean distance \n"
        "       between two consecutive input points \n"
        "   or: vmat dictionary <dataset>\n"
        "       Will create <dataset>.field#.dict, where # is the\n"
        "       field (column) number, starting at 0. Those files contain the plearn\n"
        "       scripts of the Dictionary objets for each field.\n"
        "   or: vmat catstr <dataset>\n"
        "       Will output the content of <dataset>, using its string mappings\n"
        "   or: vmat compare_stats <dataset1> <dataset2> [stdev threshold] [missing threshold]\n"
        "       Will compare stats from dataset1 to dataset2\n\n"
        "   or: vmat compare_stats_ks <dataset1> <dataset2> [--mat_to_mem]\n"
        "       Will compare stats from dataset2 to dataset2 with "
        "       Kolmogorov-Smirnov 2 samples statistic\n\n"
        "   or: vmat mtime <dataset>\n"
        "       Print the mtime of a dataset\n"
        "   or: vmat pmat_float_save <dataset>...\n"
        "       Print the number of byte saved if we transform the mat to pmat in float format and the maximum différence and maximum relative différence in value."
        "<dataset> is a parameter understandable by getDataSet: \n"

        + getDataSetHelp()
        ) 
{}


//! The actual implementation of the 'VMatCommand' command 
void VMatCommand::run(const vector<string>& args)
{
    // new vmat sub-commands
    string command = args[0];
    if(command=="bbox")
    {
        string dataspec = args[1];
        real extra_percent = 0.00;
        if(args.size()==3)
            extra_percent = toreal(args[2]);
      
        VMat vm = getDataSet(dataspec);
        TVec< pair<real, real> > bbox = vm->getBoundingBox(extra_percent);
        for(int k=0; k<bbox.length(); k++)
            cout << bbox[k].first << " : " << bbox[k].second << endl;
    }
    else if (command == "view")
    {
        // The 'view' command has been moved to VMatViewCommand (to avoid
        // a forced dependency on the curses library).
        vector<string> new_args(args.size() - 1);
        for (size_t i = 1; i < args.size(); i++)
            new_args[i - 1] = args[i];
        PLearnCommandRegistry::run("vmat_view", new_args);
    }
    else if (command == "pmat_float_save")
    {
#ifdef USEFLOAT
        PLERROR("vmat pmat_float_save don't work correctly when compiled in float.");
#endif
        PLCHECK(args.size()>1);
        for(uint f=1;f<args.size();f++){
            PPath dataspec = args[f];
            if(!isfile(dataspec)){
                PLWARNING("%s is not a file!",dataspec.c_str());
                continue;
            }
                
            VMat vm = getDataSet(dataspec);
            int64_t orig_size = vm->getSizeOnDisk();

            if(orig_size==-1)
                cout<<"-1 -1 -1"<<endl;
            else{
                FileVMatrix n = FileVMatrix(dataspec+"dummy",vm.length(),vm.width(),true,false);
                int64_t new_size = n.getSizeOnDisk();
                Vec v(vm->width());
                double max_diff=0;
                double max_rel_diff=0;
                for(int i=0;i<vm->length();i++){
                    vm->getRow(i,v);
                    for(int j=0;j<vm->width();j++){
                        double diff = v[j]-float(v[j]);
                        if(max_diff<diff)
                            max_diff=diff;
                        double rel_diff = diff/v[j];
                        if(max_rel_diff<rel_diff)
                            max_rel_diff=rel_diff;
                    }
                }
                cout << orig_size-new_size <<" "<< max_diff <<" "<<max_rel_diff<<endl;
            }
        }
    }
    else
    {
        // Dirty hack to plug into old vmatmain code
        // Eventually, should get vmatmain code in here and clean

        int argc = (int)args.size()+1;
        char** argv = new char*[argc];
        string commandname = "vmat";
        argv[0] = const_cast<char*>(commandname.c_str());
        for(int i=1 ; i<argc; i++)
            argv[i] = const_cast<char*>(args[i-1].c_str());
        vmatmain(argc, argv);
        delete[] argv;
    }
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
