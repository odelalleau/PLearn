// -*- C++ -*-

// DictionaryVMatrix.cc
//
// Copyright (C) 2004 Christopher Kermorvant 
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
   * $Id: DictionaryVMatrix.cc,v 1.1 2004/08/25 14:44:39 kermorvc Exp $ 
   ******************************************************* */

// Authors: Christopher Kermorvant

/*! \file DictionaryVMatrix.cc */


#include "DictionaryVMatrix.h"
#include "DiskVMatrix.h"
#include "plearn/io/fileutils.h"

namespace PLearn {
using namespace std;


DictionaryVMatrix::DictionaryVMatrix()
  :inherited()
  /* ### Initialise all fields to their default value */
{
  // ...
  dic_specification_file="";
  use_wordnet_stemmer=false;
  use_lower_case=false;
  pos_attribute_index=-1;
  data=0;
  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

DictionaryVMatrix::DictionaryVMatrix(const string filename)
{
  load(filename);
}


PLEARN_IMPLEMENT_OBJECT(DictionaryVMatrix,
    "VMat encoded  with Dictionaries",
    "NO HELP"
);

void DictionaryVMatrix::getNewRow(int i, const Vec& v) const
{
  // ...
  v << data(i);
}
//! returns value associated with a string (or MISSING_VALUE if there's no association for this string)                                         
real DictionaryVMatrix::getStringVal(int col, const string & str) const
{
 if(dic_type[col].first==2){
   PLERROR("word and sense are needed to retrieve sense ID : TODO");
   //    return dictionaries[col].getId(word+"/"+str);
 }else{
   return dictionaries[col].getId(str);
 }
}

//! returns element as a string, even if value doesn't map to a string, in which case tostring(value) is returned                               
string DictionaryVMatrix::getString(int row, int col) const
{
  return dictionaries[col].getSymbol((int)data(row,col));
}

string DictionaryVMatrix::getValString(int col, real val) const
{
  if(is_missing(val))return tostring(val);
  return dictionaries[col].getSymbol((int)val);
}

void DictionaryVMatrix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // ### ex:
  // declareOption(ol, "myoption", &DictionaryVMatrix::myoption, OptionBase::buildoption,
  //               "Help text describing this option");
  // ...
  declareOption(ol, "input_file", &DictionaryVMatrix::input_file, OptionBase::buildoption,
		  "The text file from which we create the VMat");
  declareOption(ol, "dic_specification_file", &DictionaryVMatrix::dic_specification_file, OptionBase::buildoption,
                "File with the path to all the dictionaries\n The file must contains as many lines as attributes in the VMat. Each line contains : dic_type path_to_dic\n where dic_type is 0 (file) or 1 (ontology)\n");
  declareOption(ol, "use_wordnet_stemmer", &DictionaryVMatrix::use_wordnet_stemmer, OptionBase::buildoption,
                "Use WordNet stemmer to extract the word stem\n");
  declareOption(ol, "use_lower_case", &DictionaryVMatrix::use_lower_case, OptionBase::buildoption,
                "Transform word to lower case\n");
  declareOption(ol, "pos_attribute_index", &DictionaryVMatrix::pos_attribute_index, OptionBase::buildoption,
                "Index of the pos attribute (if exists)\n");
  declareOption(ol, "dictionaries", &DictionaryVMatrix::dictionaries, OptionBase::buildoption,
                "Vector of dictionaries\n");
   declareOption(ol, "data", &DictionaryVMatrix::data, OptionBase::buildoption,
                "Encoded Matrix\n");
  
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void DictionaryVMatrix::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
    // translation of UPenn POS tags to WordNet POS tags
  WNPosIndex["NN"] = 1;
  WNPosIndex["NNP"] = 1;
  WNPosIndex["NNS"] = 1;
  WNPosIndex["NNPS"] = 1;
  WNPosIndex["MD"] = 2;
  WNPosIndex["VB"] = 2;
  WNPosIndex["VBD"] = 2;
  WNPosIndex["VBG"] = 2;
  WNPosIndex["VBN"] = 2;
  WNPosIndex["VBP"] = 2;
  WNPosIndex["VBZ"] = 2;
  WNPosIndex["JJ"] = 3;
  WNPosIndex["JJR"] = 3;
  WNPosIndex["JJS"] = 3;
  WNPosIndex["RB"] = 4;
  WNPosIndex["RBR"] = 4;
  WNPosIndex["RBS"] = 4;
  WNPosIndex["WRB"] = 4;

  TVec<string> tokens;
  Vec row;
  string line = "";
  string word="";
  int   input_length;
   // we have not found a word attribute yet
   word_attribute_index=-1;
   //Check input file
   if(input_file=="")PLERROR("DictionaryVMatrix: you must specify the option input_file\n");
   input_file = abspath(input_file);
   ifstream input_stream(input_file.c_str());
   if (!input_stream) PLERROR("DictionaryVMatrix: can't open input_file %s", input_file.c_str());
   // get file length
   input_length = countNonBlankLinesOfFile(input_file.c_str());
   // read first lines to set attributes_number value
   while (!input_stream.eof()){
     getline(input_stream, line, '\n');
     if (line == "" || line[0] == '#') continue;
     tokens = split(line);
     attributes_number = tokens.size();
     break;
   }
   input_stream.seekg (0, ios::beg);
   //   input_stream.seekg (0, ios::end);
   //input_length = input_stream.tellg();
   //input_stream.seekg (0, ios::beg);
  // reset file pointer to the beginning of the file
  //input_stream.seekg(0,ios::beg);
  // check values
  if(attributes_number<=0) PLERROR("DictionaryVMatrix: bad attributes_number value (read from %s)\n", input_file.c_str());
  if(input_length<=0) PLERROR("DictionaryVMatrix: bad input_length value\n");
  if(pos_attribute_index!=-1 && (pos_attribute_index>attributes_number || pos_attribute_index<0))PLERROR("DictionaryVMatrix: bad pos_attribute_index value %d\n",pos_attribute_index);
  
  data.resize(input_length,attributes_number);
  width_ = attributes_number;
  length_ = input_length;
  // Check output file
  //  if(output_file=="")PLERROR("DictionaryVMatrix: you must specify the option output_file\n");
  //DiskVMatrix dvm(output_file, attributes_number,false);
  row.resize(attributes_number);
  // Read the dictionary specification file to build the dics
  if(dic_specification_file=="") PLERROR("DictionaryVMatrix: you must specify the option dic_specification_file\n");
  extractDicType();
  
  //  while (!input_stream.eof()){
  for(int i=0;i<input_length;i++){
    getline(input_stream, line, '\n');
    if (line == "" || line[0] == '#') continue;
    tokens = split(line);
    if (tokens.size()!=dic_type.size())PLERROR("Bad file format in %s : %s", input_file.c_str(),line.c_str());
    for(int j=0;j<dic_type.size();j++){
      // special processing for word sense attribute
      // we need the word to get the sense ID, so we concatenate the word with the sense
      if(dic_type[j].first==2){
	data(i,j)=dictionaries[j].getId(tokens[word_attribute_index]+"/"+tokens[j]);
      }else{
	// in the case of word type
	if(dic_type[j].first==1){
	  // convert to lower case if wanted
	  if(use_lower_case)tokens[j]= lowerstring(tokens[j]);
	  // extract word stem if wanted
	  if(use_wordnet_stemmer){
	    if(pos_attribute_index!=-1){
	      // if we know the pos, we use it
	      tokens[j] = stemWord(tokens[j],WNPosIndex[tokens[pos_attribute_index]]);
	    }else{
	      // if we don't know the pos
	      tokens[j] = stemWord(tokens[j]);
	    }
	  }
	}
	data(i,j)=dictionaries[j].getId(tokens[j]);
	//	dvm.addStringMapping(i,tokens[j], dictionaries[j].getId(tokens[j]));
      }
    }
    //    cout<< row <<dvm.getValString(0,row[0])<<endl;
    //dvm.appendRow(row);
  }
  //input_stream.close();  
  //dvm.saveAllStringMappings();
  //  string fname;
  //for(int j=0;j<dic_type.size();j++)
  // {
      //  fname = dvm.getSFIFFilename(j,".dic");
  //  dictionaries[j].save(fname);
  // }
}

/*
void DictionaryVMatrix::save(const string& filename)
{
  string fname;
  DiskVMatrix dvm(filename,data.width(),false);
  Vec row(data.width());
  for(int i=0;i<data.length();i++){
    dvm.appendRow(data(i));
  }
  for(int j=0;j<dic_type.size();j++){
    fname = dvm.getSFIFFilename(j,".dic");
    dictionaries[j].save(fname);
  }
}
*/

// ### Nothing to add here, simply calls build_
void DictionaryVMatrix::build()
{
  inherited::build();
  build_();
}

void DictionaryVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("DictionaryVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}
void  DictionaryVMatrix::extractDicType()
{
  ifstream file_in(dic_specification_file.c_str());
  if (!file_in) PLERROR("cannot open file %s", dic_specification_file.c_str());
  string line;
  bool sense_attribute_flag=false;
  vector<string> tokens;
  int dic_count = 0;
  dic_type.resize(attributes_number);
  // by defaults, all dictionary are  DEFAULT_UPDATE
  TVec <bool> dic_mode(attributes_number, (bool)DEFAULT_UPDATE);

  // we have as many dictionaries than attributes
  dictionaries.resize(attributes_number);
  
  while (!file_in.eof()){
    line = pgetline(file_in);
    if (line == "" || line[0] == '#') continue;
    if(dic_count>=attributes_number)PLERROR("Too many lines in %s (found %d lines for %d attributes)", dic_specification_file.c_str(),dic_count+1,attributes_number);
    tokens = split(line);
    if(tokens.size()!=2 && tokens.size()!=3) PLERROR("Bad file format %s", dic_specification_file.c_str());
    cout<<line<<endl;
    // dic type
    dic_type[dic_count].first=toint(tokens[0]);
    // dic name
    dic_type[dic_count].second= abspath(tokens[1]);
    // store dic_mode if specified
    if( tokens.size()==3) {
      tokens[2]=lowerstring(tokens[2]);
      // default is false
      if(tokens[2]=="1" || tokens[2]=="update")dic_mode[dic_count] = true;
    }
    // total_lines += ShellProgressBar::getWcAsciiFileLineCount(file);
    dic_count++;
  }
  file_in.close();
  cout << "retrieved " << dic_count << " dictionary files" << endl;
  if(dic_count!=attributes_number)PLERROR("Found only %d lines in %s instead of %d", dic_count,dic_specification_file.c_str(),attributes_number);

  // Build dictionaries
  for(int i=0;i<dic_type.size();i++){
    // Ontology type
    if(dic_type[i].first==1){
      // We have found a word attribute
      word_attribute_index=i;
      // Build ontology if needed
      if(ontology==NULL ){
	string voc_file = dic_type[i].second + ".voc";
	string synset_file = dic_type[i].second + ".synsets";
	string ontology_file = dic_type[i].second + ".ontology";
	string sense_key_file = dic_type[i].second + ".sense_key";
	ontology = new WordNetOntology(voc_file, synset_file, ontology_file, sense_key_file, dic_mode[i], false);
      }
      dictionaries[i]=Dictionary(ontology, dic_type[i].second,WORDNET_WORD_DICTIONARY);
    }
    if(dic_type[i].first==2){
      sense_attribute_flag=true;
      if(ontology==NULL ){
	string voc_file = dic_type[i].second + ".voc";
	string synset_file = dic_type[i].second + ".synsets";
	string ontology_file = dic_type[i].second + ".ontology";
	string sense_key_file = dic_type[i].second + ".sense_key";
	ontology = new WordNetOntology(voc_file, synset_file, ontology_file, sense_key_file,dic_mode[i] , false);
      }
      dictionaries[i]=Dictionary(ontology, dic_type[i].second,WORDNET_SENSE_DICTIONARY);
    }
    // text file type
    if(dic_type[i].first==0){
      dictionaries[i]=Dictionary(dic_type[i].second,dic_mode[i]);
      dictionaries[i].build();
      //cout << dictionaries[i]<<endl;
    }
    
  }
   // If we want to use sense attribute we need a word attribute
  if(sense_attribute_flag && word_attribute_index==-1)PLERROR("You must specify a word attribute to use sense attribute");
  
}

} // end of namespace PLearn

