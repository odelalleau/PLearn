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
   * $Id: DatedJoinVMatrix.cc,v 1.2 2004/03/16 14:22:27 yoshua Exp $
   ******************************************************* */

// Authors: *Yoshua Bengio*

/*! \file DatedJoinVMatrix.cc */


#include "DatedJoinVMatrix.h"

namespace PLearn {
using namespace std;


DatedJoinVMatrix::DatedJoinVMatrix()
  :inherited(),slave_date_field_index(-1),master_date_interval_start_field_index(-1),
   master_date_interval_end_field_index(-1)
{
}

PLEARN_IMPLEMENT_OBJECT(DatedJoinVMatrix, 
                        "Join two vmatrices, taking into account a date field.", 
                        "The two vmatrices play an asymmetric role. They are called\n"
                        "master and slave. The resulting vmatrix has one row for each row\n"
                        "of the master vmatrix. Its columns are a concatenation of the columns\n"
                        "of the master vmatrix as well as of selected columns of the slave\n"
                        "vmatrix, in a row of the slave vmatrix that 'matches'. Matching is\n"
                        "obtained using shared 'key fields'. In addition, a date field\n"
                        "in the master is forced to belong to a date interval in the slave,\n"
                        "as follows: slave_date_start <= master_date < slave_date_end.\n"
                        "If no match is found then the slave columns are left with missing values.\n"
                        );

void DatedJoinVMatrix::getRow(int i, Vec v) const
{
  if (!master || !slave || slave_key_indices.length()==0) // etc...
    PLERROR("DatedJoinVMatrix: object was not build properly!")

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

  declareOption(ol, "slave_key_names", &DatedJoinVMatrix::save_key_names, 
                OptionBase::buildoption,
                "Names of the 'key' fields in the slave vmatrix. They should not be\n"
                "specified if the slave_key_indices are given directly. If not provided\n"
                "and if the master_key_names are specified they are assumed to be the same.\n"
                );

  declareOption(ol, "slave_fields_indices", &DatedJoinVMatrix::slave_field_indices, 
                OptionBase::buildoption,
                "Indices of the fields in the slave vmatrix to be copied in result. It is not necessary\n"
                "to specify them if the slave_fields_names are given.\n"
                );

  declareOption(ol, "slave_field_names", &DatedJoinVMatrix::save_field_names, 
                OptionBase::buildoption,
                "Names of the fields in the slave vmatrix to be copied in result. It is not necessary\n"
                "to specify them if the slave_fields_indices are given.\n"
                );

  declareOption(ol, "slave_date_field_index", &DatedJoinVMatrix::slave_date_field_index, 
                OptionBase::buildoption,
                "Index of the date field in the slave vmatrix. Should not be specified\n"
                "if the slave_date_field_name is given.\n"
                );

  declareOption(ol, "slave_date_field_name", &DatedJoinVMatrix::slave_date_field_name, 
                OptionBase::buildoption,
                "Name of the date field in the slave vmatrix. Should not be specified\n"
                "if the slave_date_field_index is given.\n"
                );

  declareOption(ol, "master_date_interval_start_field_index", 
                &DatedJoinVMatrix::master_date_interval_start_field_index, 
                OptionBase::buildoption,
                "Index of the date interval start field in the master vmatrix.\n"
                "Should not be specified if the master_date_interval_start_field_name is given.\n"
                );

  declareOption(ol, "master_date_interval_start_field_name", 
                &DatedJoinVMatrix::master_date_interval_start_field_name, 
                OptionBase::buildoption,
                "Name of the date interval start field in the master vmatrix.\n"
                "Should not be specified if the master_date_interval_start_field_index is given.\n"
                );

  declareOption(ol, "master_date_interval_end_field_index", 
                &DatedJoinVMatrix::master_date_interval_end_field_index, 
                OptionBase::buildoption,
                "Index of the date interval end field in the master vmatrix.\n"
                "Should not be specified if the master_date_interval_end_field_name is given.\n"
                );

  declareOption(ol, "master_date_interval_end_field_name", 
                &DatedJoinVMatrix::master_date_interval_end_field_name, 
                OptionBase::buildoption,
                "Name of the date interval end field in the master vmatrix.\n"
                "Should not be specified if the master_date_interval_end_field_index is given.\n"
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
        for (int i=0;i<slave_key_names.length();i++)
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
    else PLERROR("DatedJoinVMatrix: No slave_field_names were provided and no slave_field_indices were provided!");
    // * get slave date field index
    if (slave_date_field_name!="")
      slave_date_field_index = slave->getFieldIndex(slave_date_field_name);
    else if (slave_date_field_index<0)
      PLERROR("DatedJoinVMatrix: No slave_date_field_name was provided and no slave_date_field_index was provided!");
    // * get master date interval start field index
    if (master_date_interval_start_field_name!="")
      master_date_interval_start_field_index = slave->getFieldIndex(master_date_interval_start_field_name);
    else if (master_date_interval_start_field_index<0)
      PLERROR("DatedJoinVMatrix: No master_date_interval_start_field_name was provided and no master_date_interval_start_field_index was provided!");
    // * get master date interval end field index
    if (master_date_interval_end_field_name!="")
      master_date_interval_end_field_index = slave->getFieldIndex(master_date_interval_end_field_name);
    else if (master_date_interval_end_field_index<0)
      PLERROR("DatedJoinVMatrix: No master_date_interval_end_field_name was provided and no master_date_interval_end_field_index was provided!");

    // INDEX THE SLAVE
    slave_key.resize(slave_key_indices.length());
    slave_row.resize(slave.width());
    for (int i=0;i<slave.length();i++)
    {
      slave->getRow(i,slave_row);
      for (int j=0;j<slave_key_indices.size();j++)
        slave_key[j] = slave_row[slave_key_indices[j]];
      mp.insert(make_pair(slave_key,i));
    }
  }
}

// ### Nothing to add here, simply calls build_
void DatedJoinVMatrix::build()
{
  inherited::build();
  build_();
}

void DatedJoinVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(slave_row, copies);
  deepCopyField(slave_key, copies);
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

