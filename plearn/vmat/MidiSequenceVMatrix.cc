// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2004 Jasmin Lapalme
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


#include "MidiSequenceVMatrix.h"
#include "random.h"
#include "dirent.h"
#include "libmidi/Shepard.h"

namespace PLearn {
using namespace std;


/** MidiSequenceVMatrix **/

PLEARN_IMPLEMENT_OBJECT(MidiSequenceVMatrix, "ONE LINE DESCR", "NO HELP");

////////////////
// SequenceVMatrix //
////////////////
MidiSequenceVMatrix::MidiSequenceVMatrix()
  :SequenceVMatrix(0, 0)
{
  type_= "note";
  normalize_value_ = false;
}

////////////////////
// declareOptions //
////////////////////
void MidiSequenceVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "dirname", &MidiSequenceVMatrix::dirname_, OptionBase::buildoption, "Name of the midi file to read");
  declareOption(ol, "type", &MidiSequenceVMatrix::type_, OptionBase::buildoption, "The type(representation) of sequence to build. Shepard or midi note");
  declareOption(ol, "track", &MidiSequenceVMatrix::track_, OptionBase::buildoption, "The track number to put in the sequence");
  declareOption(ol, "normalize", &MidiSequenceVMatrix::normalize_value_, OptionBase::buildoption, "The track number to put in the sequence");
  inherited::declareOptions(ol);
}

string MidiSequenceVMatrix::get_dirname() const {
  return dirname_;
}

string MidiSequenceVMatrix::get_type() const {
  return type_;
}

int MidiSequenceVMatrix::get_track() const {
  return track_;
}

void MidiSequenceVMatrix::build()
{
  inherited::build();
  build_();
}

void MidiSequenceVMatrix::add_seq_shepard(string filename) {

  int sz = 0;
  double * dat = NULL;
  int atime;
  int prevATime=-10000;

  read_meta_event(filename);

  Shepard* shp = new Shepard(-1.0, 1.0);
  MidiFilePtr midifile = new MidiFile( filename.c_str() );
  
  if (length_ == 0) {
    inputsize_ = midifile->trackCount() * SHP_LEN;
    targetsize_ = midifile->trackCount() * SHP_LEN;
    weightsize_ = 0;
    width_ = inputsize_ + targetsize_ + weightsize_;
  }

  dat = midifile->eventsArr(sz,0,REPR_MSEC);
  int evCount = sz / EV_VEC_LEN;

  sequences.resize(length_+1);
  sequences[length_] = Mat(0, width_, MISSING_VALUE);
  Vec r = Vec(width_, MISSING_VALUE);

  int off = offset[midifile->trackCount()-1];
  for (int i = off; i < evCount; i++) {
    int pitch = (int)dat[i*EV_VEC_LEN+EV_VEC_DATA1];
    atime = (int)dat[i*EV_VEC_LEN+EV_VEC_ATIME];
    if ((atime-prevATime)<(lags[0]*.9)) { //extra note!
 	continue;
    }
    prevATime = atime;
    for (int j = 0; j < SHP_LEN; j++)
      r[j] = shp->noteAsPtr(pitch)[j];
    (sequences[length_]).appendRow(r);
  }
  delete dat;

  for (int h = 1; h < 4; h++) {
    dat = midifile->eventsArr(sz,h,REPR_MSEC);
    evCount = sz / EV_VEC_LEN;
    int ns = (off - offset[h])/harms[h];  // Number of note to skip
    for (int i = ns; i < evCount && (i-ns)*harms[h] < sequences[length_].nrows(); i++) {
      int pitch = (int)dat[i*EV_VEC_LEN+EV_VEC_DATA1];
      for (int j = 0; j < SHP_LEN; j++)
	(sequences[length_])[i-ns][h*SHP_LEN+j] = shp->noteAsPtr(pitch)[j];
    }
    delete dat;
  }
  length_++;
  delete midifile;
  delete shp;

  /*  int sz = 0;
  double * dat = NULL;

  read_meta_event(filename);

  MidiFilePtr midifile = new MidiFile( filename.c_str() );

  if (length_ == 0) {
    inputsize_ = midifile->trackCount() * SHP_LEN;
    targetsize_ = midifile->trackCount() * SHP_LEN;
    weightsize_ = 0;
    width_ = inputsize_ + targetsize_ + weightsize_;
  }
  
  dat = midifile->eventsArr(sz,0,REPR_SHEPARD);
  int evCount = sz / (SHP_LEN+SHP_DUR_LEN);

  sequences.resize(length_+1);
  sequences[length_] = Mat(evCount-1, width_);

  int off = offset[midifile->trackCount()-1];

  for (int i = 0; i < evCount-1; i++) {
    for (int s = 0; s < SHP_LEN; s++) {
      (sequences[length_])[i][s] = dat[i*(SHP_LEN+SHP_DUR_LEN)+s];
    }
  }


  delete dat;

  for (int h = 1; h < 4; h++) {
    dat = midifile->eventsArr(sz,h,REPR_SHEPARD);
    evCount = sz / (SHP_LEN+SHP_DUR_LEN);
    int ns = (off - offset[h])/harms[h];  // Number of note to skip
    for (int i = ns; i < evCount && (i-ns)*harms[h] < sequences[length_].nrows(); i++) {
      double* begin = &dat[i*(SHP_LEN+SHP_DUR_LEN)];
      for (int j = 0; j < SHP_LEN; j++)
	(sequences[length_])[i-ns][h*SHP_LEN+j] = begin[j];
    }
    delete dat;
  }
  length_++;
  delete midifile;
  */
}

void MidiSequenceVMatrix::fill_shepard() {
  DIR* dp = 0;
  struct dirent* d;

  length_ = 0;
  sequences = TVec<Mat>(0);

  if ((dp = opendir(get_dirname().c_str())) == 0)
    PLERROR("In MidiSequenceVMatrix::fill_shepard() : Unable to open the dir %s", 
	    get_dirname().c_str());
  
  while ((d = readdir(dp)) != 0) {
    if (d->d_ino == 0)
      continue;
    string f_name = d->d_name;
    if ((f_name.length() > 4) && (f_name.substr(f_name.length()-4,4) == ".mid")) {
      add_seq_shepard(get_dirname() + "/" + f_name);
    }
  }

  closedir(dp);
}

void MidiSequenceVMatrix::read_meta_event(string filename) {
  if (harms != NULL)
    delete harms;
  if (offset != NULL)
    delete offset;
  if (lags != NULL)
    delete lags;
  
  MidiFilePtr midifile = new MidiFile( filename.c_str() );
  
  harms = new int[midifile->trackCount()];
  offset = new int[midifile->trackCount()];
  lags =  new int[midifile->trackCount()];

  for (int i=0;i<midifile->trackCount();i++) {
    MidiTrack * tr=midifile->track(i);
    for (int e=0;e<tr->eventCount();e++) {
      MidiEvent * ev=tr->event(e);
      if (ev->typ()==MIDI_META && ev->subtyp()==META_TXT_TEXT) {
	char * msg = ((MidiEventMeta *)ev)->text();
	if (strncmp(msg,"ACORR",5)==0) {
	  char * tks = strtok(msg, " "); //now ACORR
	  tks = strtok(NULL," "); //now offset_steps
	  tks = strtok(NULL," "); //now val
	  offset[i]=atoi(tks); 
	  tks = strtok(NULL," "); //now harmonic_steps
	  tks = strtok(NULL," "); //now val
	  harms[i] = atoi(tks);
	  tks = strtok(NULL," "); //now offset_ms
	  tks = strtok(NULL," "); //now val
	  //	  lags[i] = atoi(tks);
	  tks = strtok(NULL," "); //now harmonic_ms
	  tks = strtok(NULL," "); //now val
	  lags[i] = atoi(tks);
	  break;
	}
      }
    }
  }

  delete midifile;
}

void MidiSequenceVMatrix::build_target() {
  for (int i = 0; i < getNbSeq(); i++) {
    int l_seq = sequences[i].nrows();
    for (int j = 0; j < l_seq-1; j++) {
      for (int k = 0; k < inputsize_; k++) {
	(sequences[i])[j][k+inputsize_] = (sequences[i])[j+1][k];
      }
    }
  }
}

void MidiSequenceVMatrix::add_seq_noteson(string filename) {

  int sz = 0;
  double * dat = NULL;
  int atime;
  int prevATime=-10000;

  read_meta_event(filename);

  MidiFilePtr midifile = new MidiFile( filename.c_str() );
  
  if (length_ == 0) {
    inputsize_ = midifile->trackCount();
    targetsize_ = midifile->trackCount();
    weightsize_ = 0;
    width_ = inputsize_ + targetsize_ + weightsize_;
  }

  dat = midifile->eventsArr(sz,0,REPR_MSEC);
  int evCount = sz / EV_VEC_LEN;

  sequences.resize(length_+1);
  sequences[length_] = Mat(0, width_, MISSING_VALUE);
  Vec r = Vec(width_, MISSING_VALUE);

  int off = offset[midifile->trackCount()-1];
  for (int i = off; i < evCount; i++) {
    int pitch = (int)dat[i*EV_VEC_LEN+EV_VEC_DATA1];
    atime = (int)dat[i*EV_VEC_LEN+EV_VEC_ATIME];
    if ((atime-prevATime)<(lags[0]*.9)) { //extra note!
 	continue;
    }
    prevATime = atime;
    r[0] = normalize(pitch);
    (sequences[length_]).appendRow(r);
  }
  delete dat;

  for (int h = 1; h < 4; h++) {
    dat = midifile->eventsArr(sz,h,REPR_MSEC);
    evCount = sz / EV_VEC_LEN;
    int ns = (off - offset[h])/harms[h];  // Number of note to skip
    for (int i = ns; i < evCount && (i-ns)*harms[h] < sequences[length_].nrows(); i++) {
      int pitch = (int)dat[i*EV_VEC_LEN+EV_VEC_DATA1];
      (sequences[length_])[i-ns][h] = normalize(pitch);
    }
    delete dat;
  }
  length_++;
  delete midifile;
}

real MidiSequenceVMatrix::normalize(real r) {
  if (!normalize_value_)
    return r;
  if (is_missing(r))
    return MISSING_VALUE;
  if (r == 0.0)
    return 0.0;
  return ((int)r % 12) + 1;
}

void MidiSequenceVMatrix::fill_noteson() {
  DIR* dp = 0;
  struct dirent* d;

  length_ = 0;
  sequences = TVec<Mat>(0);

  if ((dp = opendir(get_dirname().c_str())) == 0)
    PLERROR("In MidiSequenceVMatrix::fill_noteson() : Unable to open the dir %s", 
	    get_dirname().c_str());
  
  while ((d = readdir(dp)) != 0) {
    if (d->d_ino == 0)
      continue;
    string f_name = d->d_name;
    if ((f_name.length() > 4) && (f_name.substr(f_name.length()-4,4) == ".mid")) {
      add_seq_noteson(get_dirname() + "/" + f_name);
    }
  }

  closedir(dp);
}

void MidiSequenceVMatrix::fill_test() {
  /*
  int __harms[] = {1, 3, 6, 12};
  static int n_seq = 10;
  static int n_longseq = 100;

  inputsize_ = 4;
  targetsize_ = 4;
  weightsize_ = 0;
  width_ = inputsize_ + targetsize_ + weightsize_;
  length_ = n_seq;

  sequences = TVec<Mat>(n_seq);

  for (int i = 0; i < n_seq; i++) {
    sequences[i] = Mat(n_longseq, width_, MISSING_VALUE);
    for (int h = 1; h < n_longseq; h++) {
      for (int j = 0; j < n_repeat; j++) {
	for (int k = 0; k < 12; k++) {
	  if ((k % __harms[h]) == 0)
	    (sequences[i])[j*12+k][h] = (real)(k + 1);
	}
      }
    }
  }
  */
}

void MidiSequenceVMatrix::build_()
{

  harms = NULL;
  offset = NULL;
  lags = NULL;

  if (get_type() == "SHEPARD") {
    fill_shepard();
    //    normalize_mean();
  } else if (get_type() == "NOTESON") {
    fill_noteson();
  } else if (get_type() == "TEST") {
    fill_test();
  } else
    PLERROR("In MidiSequenceVMatrix::build_() : the type %s is unknown", get_type().c_str());
  build_target();
}

void MidiSequenceVMatrix::run() {
  print();
  /*
  for (int i_seq = 0; i_seq < getNbSeq(); i_seq++) {
    int pos = 0;
    int w = sequences[i_seq].ncols() / 2;
    int l = sequences[i_seq].nrows();
    Vec curr = Vec(w);
    while (is_missing((sequences[i_seq])[pos][w-1]))
      pos++;
    for (int i = 0; i < w; i++)
      curr[i] = (sequences[i_seq])[pos][i];
    int start = pos;
    pos++;
    while (pos < l) {
      for (int i = 0; i < w; i++)
	cout << curr[i] << " ";
      cout << (sequences[i_seq])[pos][0] << endl;
      for (int i = 0; i < w && ((pos-start) % harms[i]) == 0; i++) {
	curr[i] = (sequences[i_seq])[pos][0];
      }
      pos++;
    }
    }*/
}

} // end of namespcae PLearn
  
