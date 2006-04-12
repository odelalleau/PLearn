// -*- C++ -*-

// DatedJoinVMatrix.cc
//
// Copyright (C) 2004 *Yoshua Bengio* 
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

// Authors: *Yoshua Bengio*

/*! \file DatedJoinVMatrix.cc */


#include "DatedJoinVMatrix.h"
#include <plearn/base/PDate.h>
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;


DatedJoinVMatrix::DatedJoinVMatrix()
    :inherited(),master_date_field_index(-1),slave_date_interval_start_field_index(-1),
     slave_date_interval_end_field_index(-1), verbosity(0), output_the_slave(false), output_matching_index(false)
{
}

PLEARN_IMPLEMENT_OBJECT(DatedJoinVMatrix, 
                        "Join two vmatrices, taking into account a date field.", 
                        "The two vmatrices play an asymmetric role. They are called\n"
                        "master and slave. The resulting vmatrix has one row for each row\n"
                        "of the master vmatrix (or optionally of the slave vmatrix). Its\n"
                        "columns are a concatenation of selected columns of the master vmatrix\n"
                        "and of selected columns of the slave which 'match' according to a rule\n"
                        "(always in the order: master fields, slave fields). Matchint is\n"
                        "obtained using shared 'key fields'. Optionally, for matching, a date field\n"
                        "in the master is forced to belong to a date interval in the slave,\n"
                        "as follows: slave_date_start < master_date <= slave_date_end.\n"
                        "If no match is found then the master (or slave) columns are left with missing values.\n"
                        "If more than one slave row matches, then the one with the latest\n"
                        "slave_date_start is used (and a warning is optionally issued). If\n"
                        "no slave_date_start field is provided then no date constraint is\n"
                        "enforced, and the last key-matching slave row is matched to a master row.\n"
                        "An option (output_the_slave) allows to output one row for each slave row\n"
                        "instead of the default which outputs one row for each master row.\n"
                        "Note that if (output_the_slave) then the non-matching master rows are 'lost'\n"
                        "whereas if (!output_the_slave) then the non-matching slave rows are 'lost'.\n"
                        "If output_the_slave and more than one master row matches with a given slave_row\n"
                        "then the SUM of the master fields is computed (i.e. be careful that their sum is meaningful)\n"
    );

void DatedJoinVMatrix::getNewRow(int i, const Vec& v) const
{
    if (!master || !slave || slave_key_indices.length()==0) // etc...
        PLERROR("DatedJoinVMatrix: object was not build properly!");
    list<int> master_index;
    int slave_index=-1;
    if (output_the_slave)
    {
        slave_index = i;
        master_index = slave2master[i];
        if (output_matching_index)
            v[0] = *(master_index.begin());
    }
    else
    {
        master_index.push_back(i);
        slave_index = master2slave[i];
        if (output_matching_index)
            v[0] = slave_index;
    }

    Vec master_part = v.subVec(output_matching_index,n_master_fields);
    Vec slave_part = v.subVec(n_master_fields+output_matching_index,n_slave_fields);

    if (master_index.size()>0)
    {
        list<int>::const_iterator b_it = master_index.begin();
        list<int>::const_iterator e_it = master_index.end();
        master_part.clear();
        for (list<int>::const_iterator it=b_it;it!=e_it;++it)
        {
            // copy the master fields 
            master->getRow(*it,master_row);
            if (master_field_indices.size()>0)
                for (int j=0;j<master_field_indices.size();j++)
                    master_part[j] += master_row[master_field_indices[j]];
            else
                master_part += master_row;
        }
    }
    else 
        master_part.fill(MISSING_VALUE);

    if (slave_index>=0)
    {
        // copy the slave fields 
        slave->getRow(slave_index,slave_row);
        if (slave_field_indices.size()>0)
            for (int j=0;j<slave_field_indices.size();j++)
                slave_part[j] = slave_row[slave_field_indices[j]];
        else
            slave_part << slave_row;
    }
    else 
        slave_part.fill(MISSING_VALUE);

}

void DatedJoinVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "master", &DatedJoinVMatrix::master, OptionBase::buildoption,
                  "Master vmatrix, whose columns are directly copied in the result.");

    declareOption(ol, "slave", &DatedJoinVMatrix::slave, OptionBase::buildoption,
                  "Slave vmatrix, of which only some columns are copied, when the\n"
                  "key fields and the dates match.");

    declareOption(ol, "master_key_indices", &DatedJoinVMatrix::master_key_indices, 
                  OptionBase::buildoption,
                  "Indices of the 'key' fields in the master vmatrix. It is not necessary\n"
                  "to specify them if the master_key_names are given or if the slave_key_names\n"
                  "are specified (in that case they are assumed to be the same)\n"
        );

    declareOption(ol, "master_key_names", &DatedJoinVMatrix::master_key_names, 
                  OptionBase::buildoption,
                  "Names of the 'key' fields in the master vmatrix. They should not be\n"
                  "specified if the master_key_indices are given directly. If not provided\n"
                  "and if the slave_key_names are specified they are assumed to be the same.\n"
        );

    declareOption(ol, "slave_key_indices", &DatedJoinVMatrix::slave_key_indices, 
                  OptionBase::buildoption,
                  "Indices of the 'key' fields in the slave vmatrix. It is not necessary\n"
                  "to specify them if the slave_key_names are given or if the master_key_names\n"
                  "are specified (in that case they are assumed to be the same)\n"
        );

    declareOption(ol, "slave_key_names", &DatedJoinVMatrix::slave_key_names, 
                  OptionBase::buildoption,
                  "Names of the 'key' fields in the slave vmatrix. They should not be\n"
                  "specified if the slave_key_indices are given directly. If not provided\n"
                  "and if the master_key_names are specified they are assumed to be the same.\n"
        );

    declareOption(ol, "slave_field_indices", &DatedJoinVMatrix::slave_field_indices, 
                  OptionBase::buildoption,
                  "Indices of the fields in the slave vmatrix to be copied in result. It is not necessary\n"
                  "to specify them if the slave_field_names are given.\n"
                  "N.B. IF NEITHER slave_field_indices NOR slave_field_names are given then it is assumed\n"
                  "ALL slave fields should be copied on output.\n"
        );

    declareOption(ol, "slave_field_names", &DatedJoinVMatrix::slave_field_names, 
                  OptionBase::buildoption,
                  "Names of the fields in the slave vmatrix to be copied in result. It is not necessary\n"
                  "to specify them if the slave_field_indices are given.\n"
                  "N.B. IF NEITHER slave_field_indices NOR slave_field_names are given then it is assumed\n"
                  "ALL slave fields should be copied on output.\n"
        );

    declareOption(ol, "master_field_indices", &DatedJoinVMatrix::master_field_indices, 
                  OptionBase::buildoption,
                  "Indices of the fields in the master vmatrix to be copied in result. It is not necessary\n"
                  "to specify them if the slave_field_names are given.\n"
                  "N.B. IF NEITHER master_field_indices NOR master_field_names are given then it is assumed\n"
                  "ALL master fields should be copied on output.\n"
        );

    declareOption(ol, "master_field_names", &DatedJoinVMatrix::master_field_names, 
                  OptionBase::buildoption,
                  "Names of the fields in the slave vmatrix to be copied in result. It is not necessary\n"
                  "to specify them if the slave_field_indices are given.\n"
                  "N.B. IF NEITHER master_field_indices NOR master_field_names are given then it is assumed\n"
                  "ALL master fields should be copied on output.\n"
        );

    declareOption(ol, "master_date_field_index", &DatedJoinVMatrix::master_date_field_index, 
                  OptionBase::buildoption,
                  "Index of the date field in the master vmatrix. Should not be specified\n"
                  "if the master_date_field_name is given.\n"
        );

    declareOption(ol, "master_date_field_name", &DatedJoinVMatrix::master_date_field_name, 
                  OptionBase::buildoption,
                  "Name of the date field in the master vmatrix. Should not be specified\n"
                  "if the master_date_field_index is given.\n"
        );

    declareOption(ol, "slave_date_interval_start_field_index", 
                  &DatedJoinVMatrix::slave_date_interval_start_field_index, 
                  OptionBase::buildoption,
                  "Index of the date interval start field in the slave vmatrix.\n"
                  "Should not be specified if the slave_date_interval_start_field_name is given.\n"
        );

    declareOption(ol, "slave_date_interval_start_field_name", 
                  &DatedJoinVMatrix::slave_date_interval_start_field_name, 
                  OptionBase::buildoption,
                  "Name of the date interval start field in the slave vmatrix.\n"
                  "Should not be specified if the slave_date_interval_start_field_index is given.\n"
        );

    declareOption(ol, "slave_date_interval_end_field_index", 
                  &DatedJoinVMatrix::slave_date_interval_end_field_index, 
                  OptionBase::buildoption,
                  "Index of the date interval end field in the slave vmatrix.\n"
                  "Should not be specified if the slave_date_interval_end_field_name is given.\n"
        );

    declareOption(ol, "slave_date_interval_end_field_name", 
                  &DatedJoinVMatrix::slave_date_interval_end_field_name, 
                  OptionBase::buildoption,
                  "Name of the date interval end field in the slave vmatrix.\n"
                  "Should not be specified if the slave_date_interval_end_field_index is given.\n"
        );

    declareOption(ol, "verbosity", &DatedJoinVMatrix::verbosity,
                  OptionBase::buildoption,
                  "0: no warning issued,\n"
                  "1: warning issued if more than one slave row matches,\n"
                  "2: details about these matches are printed\n"
        );

    declareOption(ol, "output_the_slave", &DatedJoinVMatrix::output_the_slave,
                  OptionBase::buildoption,
                  "If true than output the SLAVE rows (with master_fields_* from matching master row)\n"
                  "instead of the MASTER rows (with slave_fields_* from the matching slave row)\n"
        );

    declareOption(ol, "output_matching_index", &DatedJoinVMatrix::output_matching_index,
                  OptionBase::buildoption,
                  "If true than output an extra variable 'matching_index' which contains the row\n"
                  "index of the matching slave row (if !output_the_slave) or matching master row\n"
                  "if (output_the_slave).\n"
        );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DatedJoinVMatrix::build_()
{
    if (master && slave) // we can't really build if we don't have them
    {
        // convert field names into indices
        // * get master key indices
        if (master_key_names.length()>0)
        {
            master_key_indices.resize(master_key_names.length());
            for (int i=0;i<master_key_names.length();i++)
                master_key_indices[i] = master->getFieldIndex(master_key_names[i]);
        } 
        else if (master_key_indices.length()==0)
        {
            if (slave_key_names.length()>0)
            {
                master_key_indices.resize(slave_key_names.length());
                for (int i=0;i<slave_key_names.length();i++)
                    master_key_indices[i] = master->getFieldIndex(slave_key_names[i]);
            }
            else PLERROR("DatedJoinVMatrix: No key names were provided and no master_key_indices were provided!");
        }
        // * get slave key indices
        if (slave_key_names.length()>0)
        {
            slave_key_indices.resize(slave_key_names.length());
            for (int i=0;i<slave_key_names.length();i++)
                slave_key_indices[i] = slave->getFieldIndex(slave_key_names[i]);
        } 
        else if (slave_key_indices.length()==0)
        {
            if (master_key_names.length()>0)
            {
                slave_key_indices.resize(master_key_names.length());
                for (int i=0;i<master_key_names.length();i++)
                    slave_key_indices[i] = slave->getFieldIndex(master_key_names[i]);
            }
            else PLERROR("DatedJoinVMatrix: No key names were provided and no slave_key_indices were provided!");
        }
        // * get slave field indices
        if (slave_field_names.length()>0)
        {
            slave_field_indices.resize(slave_field_names.length());
            for (int i=0;i<slave_field_names.length();i++)
                slave_field_indices[i] = slave->getFieldIndex(slave_field_names[i]);
        } 
        // * get master field indices
        if (master_field_names.length()>0)
        {
            master_field_indices.resize(master_field_names.length());
            for (int i=0;i<master_field_names.length();i++)
                master_field_indices[i] = master->getFieldIndex(master_field_names[i]);
        } 
        // * get master date field index
        if (master_date_field_name!="")
            master_date_field_index = master->getFieldIndex(master_date_field_name);
        else if (master_date_field_index<0)
            PLWARNING("DatedJoinVMatrix: No master_date_field_name was provided and no master_date_field_index was provided!");
        // * get slave date interval start field index
        if (slave_date_interval_start_field_name!="")
            slave_date_interval_start_field_index = slave->getFieldIndex(slave_date_interval_start_field_name);
        else if (slave_date_interval_start_field_index<0 && master_date_field_index>=0)
            PLERROR("DatedJoinVMatrix: No slave_date_interval_start_field_name was provided and no slave_date_interval_start_field_index was provided!");
        // * get slave date interval end field index
        if (slave_date_interval_end_field_name!="")
            slave_date_interval_end_field_index = slave->getFieldIndex(slave_date_interval_end_field_name);
        else if (slave_date_interval_end_field_index<0 && master_date_field_index>=0)
            PLERROR("DatedJoinVMatrix: No slave_date_interval_end_field_name was provided and no slave_date_interval_end_field_index was provided!");

        // INDEX THE SLAVE
        ProgressBar* pb=new ProgressBar("DatedJoinVMatrix: indexing the slave.",slave.length());
        key.resize(slave_key_indices.length());
        slave_row.resize(slave.width());
        master_row.resize(master.width());
        for (int i=0;i<slave.length();i++)
        {
            slave->getRow(i,slave_row);
            for (int j=0;j<slave_key_indices.size();j++)
                key[j] = slave_row[slave_key_indices[j]];
            mp.insert(make_pair(key,i));
            pb->update(i);
        }
        delete pb;

        // set the width and the length
        if (master_field_indices.size()>0)
            n_master_fields = master_field_indices.size();
        else
            n_master_fields = master->width();
        if (slave_field_indices.size()>0)
            n_slave_fields = slave_field_indices.size();
        else
            n_slave_fields = slave->width();
        width_ = output_matching_index + n_master_fields + n_slave_fields;
        if (output_the_slave)
            length_ = slave.length();
        else
            length_ = master.length();

        //! Copy the appropriate VMFields
        fieldinfos.resize(width_);
        Array<VMField> master_infos = master->getFieldInfos();
        Array<VMField> slave_infos = slave->getFieldInfos();
        if (output_matching_index)
            fieldinfos[0].name="matching_index";
        if (master_infos.size() > 0)
        {
            if (master_field_indices.size()>0)
                for (int i=0; i<n_master_fields; ++i)
                    fieldinfos[output_matching_index+i] = master_infos[master_field_indices[i]];
            else
                for (int i=0; i<n_master_fields; ++i)
                    fieldinfos[output_matching_index+i] = master_infos[i];
        }
        if (slave_infos.size() > 0)
        {
            if (slave_field_indices.size()>0)
                for (int i=0; i<slave_field_indices.size(); ++i)
                {
                    VMField f=slave_infos[slave_field_indices[i]];
                    if ((master_field_indices.size()==0 && master->fieldIndex(f.name)>=0)
                        || master_field_names.contains(f.name))
                        f.name = "slave." + f.name;
                    fieldinfos[output_matching_index+n_master_fields+i] = f;
                }
            else
                for (int i=0; i<n_slave_fields; ++i)
                {
                    VMField f=slave_infos[i];
                    if ((master_field_indices.size()==0 && master->fieldIndex(f.name)>=0)
                        || master_field_names.contains(f.name))
                        f.name = "slave." + f.name;
                    fieldinfos[output_matching_index+n_master_fields+i] = f;
                }
        }
        pb=new ProgressBar("DatedJoinVMatrix: matching the master and slave.",master->length());
        // pre-compute the 'match' (N.B. this is expensive...)
        master2slave.resize(master->length());
        master2slave.fill(-1);
        slave2master.resize(slave->length());
        for (int i=0;i<master->length();i++)
        {
            master->getRow(i,master_row);
            // build a key to search in the slave vmat
            for (int j=0;j<master_key_indices.size();j++)
                key[j] = master_row[master_key_indices[j]];

            // get the list of matching slave rows
            Maptype::const_iterator it,low,upp; 
            pair<Maptype::const_iterator,Maptype::const_iterator> matches_it=mp.equal_range(key);
            low=matches_it.first;
            upp=matches_it.second;
            if (low!=mp.end())
            {
                PDate master_date;
                if (master_date_field_index>=0)
                    master_date = float_to_date(master_row[master_date_field_index]);
                PDate latest_match;
                int n_matches=0;
                static TVec<int> matches;
                if (verbosity>1) matches.resize(0);
                int matching_slave_row_index = -1;
                // iterate over the matching slave rows
                for (it=low;it!=upp;++it)
                {
                    slave->getRow(it->second,slave_row);
                    if (master_date_field_index>=0)
                    {
                        PDate slave_date_interval_start = float_to_date(slave_row[slave_date_interval_start_field_index]);
                        PDate slave_date_interval_end = float_to_date(slave_row[slave_date_interval_end_field_index]);
                        if (master_date>slave_date_interval_start && master_date<=slave_date_interval_end)
                        {
                            if (n_matches==0 || slave_date_interval_start > latest_match)
                            {
                                latest_match = slave_date_interval_start;
                                matching_slave_row_index = it->second;
                            }
                            n_matches++;
                            if (verbosity>1) matches.append(it->second);
                        }
                    } else // no date, the LAST one will remain
                    {
                        matching_slave_row_index = it->second;
                        n_matches++;
                        if (verbosity>1) matches.append(it->second);
                    }
                }
                if (matching_slave_row_index>=0)
                {
                    master2slave[i] = matching_slave_row_index;
                    slave2master[matching_slave_row_index].push_back(i);
                }
                if (n_matches>1 && verbosity>0)
                {
                    PLWARNING("DatedJointVMatrix:getRow(%d,.) matched more than once\n",i);
                    if (verbosity >1)
                        for (int j=0;j<n_matches;j++)
                            cerr << "master row " << i << " matched slave row " << matches[j] << endl;
                }
            }
            pb->update(i);
        }
        delete pb;
    }
}

// ### Nothing to add here, simply calls build_
void DatedJoinVMatrix::build()
{
    inherited::build();
    build_();
}

void DatedJoinVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(slave_row, copies);
    deepCopyField(key, copies);
    deepCopyField(master2slave, copies);
    deepCopyField(slave2master, copies);
    deepCopyField(master, copies);
    deepCopyField(slave, copies);
    deepCopyField(master_key_indices, copies);
    deepCopyField(slave_key_indices, copies);
    deepCopyField(master_key_names, copies);
    deepCopyField(slave_key_names, copies);
    deepCopyField(slave_field_indices, copies);
    deepCopyField(slave_field_names, copies);
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
