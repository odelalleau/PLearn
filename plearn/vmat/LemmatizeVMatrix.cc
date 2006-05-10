// -*- C++ -*-

// LemmatizeVMatrix.cc
//
// Copyright (C) 2005 Hugo Larochelle
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file LemmatizeVMatrix.cc */


#include "LemmatizeVMatrix.h"
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LemmatizeVMatrix,
    "Takes a VMatrix with a word and a POS field and adds a field consisting of the lemma form of the word",
    "This field becomes the last input field. First, customized\n"
    "mapping tables (with different priorities) are used, \n"
    "and finally, if the previous step was unsuccesful, \n"
    "WordNet is used to obtain a lemma.\n"
    );

//////////////////
// LemmatizeVMatrix //
//////////////////
LemmatizeVMatrix::LemmatizeVMatrix()
    : word_field(-1), pos_field(-1)
{
    lemma_dict = new Dictionary();

    if(morphinit<0)
        PLERROR("In WordNetSenseDictionary(): could not open WordNet exception list files");
//    if(wninit()<0)
//        PLERROR("In WordNetSenseDictionary(): could not open WordNet database files");
}


string LemmatizeVMatrix::getLemma(int row) const
{
    string pos;
    string word;

    word = lowerstring(source->getString(row,word_field));
    pos = source->getString(row,pos_field);

    // Verify if POS tag is compatible for word, using WordNet
    // wninit() most be called for that...
   /*
    TVec<string> ret_noun(0);
    TVec<string> ret_verb(0);
    TVec<string> ret_adj(0);
    TVec<string> ret_adv(0);
    TVec<string> stems;
    if(strstr(pos.c_str(),"NN"))
    {
        ret_noun.append(extractSenses(word,NOUN,"sense_key"));
        stems = stemsOfWord(word,NOUN);
        for(int i=0; i<stems.length(); i++)
            if(word != stems[i])
                ret_noun.append(extractSenses(stems[i],NOUN,"sense_key"));
    }

    if(strstr(pos.c_str(),"VB"))
    {
        ret_verb.append(extractSenses(word,VERB,"sense_key"));
        stems = stemsOfWord(word,VERB);
        for(int i=0; i<stems.length(); i++)
            if(word != stems[i])
                ret_verb.append(extractSenses(stems[i],VERB,"sense_key"));
    }

    if(strstr(pos.c_str(),"JJ"))
    {
        ret_adj.append(extractSenses(word,ADJ,"sense_key"));
        stems = stemsOfWord(word,ADJ);
        for(int i=0; i<stems.length(); i++)
            if(word != stems[i])
                ret_adj.append(extractSenses(stems[i],ADJ,"sense_key"));
    }

    if(strstr(pos.c_str(),"RB"))
    {
        ret_adv.append(extractSenses(word,ADV,"sense_key"));
        stems = stemsOfWord(word,ADV);
        for(int i=0; i<stems.length(); i++)
            if(word != stems[i])
                ret_adv.append(extractSenses(stems[i],ADV,"sense_key"));
    }

    if(!(ret_noun.length() == 0 && ret_verb.length() == 0 && ret_adj.length() == 0 && ret_adv.length() == 0))
    {
        if(strstr(pos.c_str(),"NN") && ret_noun.length() == 0)
            PLWARNING("In LemmatizeVMatrix::getLemma(): word %s cannot be a noun", word.c_str());
        if(strstr(pos.c_str(),"VB") && ret_verb.length() == 0)
            PLWARNING("In LemmatizeVMatrix::getLemma(): word %s cannot be a verb", word.c_str());
        if(strstr(pos.c_str(),"JJ") && ret_adj.length() == 0)
            PLWARNING("In LemmatizeVMatrix::getLemma(): word %s cannot be an adjective", word.c_str());
        if(strstr(pos.c_str(),"RB") && ret_adv.length() == 0)
            PLWARNING("In LemmatizeVMatrix::getLemma(): word %s cannot be an adverb", word.c_str());
    }
    */

    // Use word_pos_to_lemma_table
    for(int i=0; i<word_pos_to_lemma_table.length(); i++)
        if(word_pos_to_lemma_table(i,0) == word && word_pos_to_lemma_table(i,1) == pos)
            return word_pos_to_lemma_table(i,2);

    // Use word_to_lemma_table
    for(int i=0; i<word_to_lemma_table.length(); i++)
        if(word_to_lemma_table(i,0) == word)
            return word_to_lemma_table(i,1);

    // Use pos_to_lemma_table
    for(int i=0; i<pos_to_lemma_table.length(); i++)
        if(pos_to_lemma_table(i,0) == pos)
            return pos_to_lemma_table(i,1);

    if(strstr(pos.c_str(),"NN")) return stemWord(word,NOUN);
    else if(strstr(pos.c_str(),"VB")) return stemWord(word,VERB);
    else if(strstr(pos.c_str(),"JJ")) return stemWord(word,ADJ);
    else if(strstr(pos.c_str(),"RB")) return stemWord(word,ADV);

    return word;
}

////////////////////
// declareOptions //
////////////////////
void LemmatizeVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "lemma_dict", &LemmatizeVMatrix::lemma_dict, OptionBase::learntoption,
                  "Dictionary for the lemma field\n");
    declareOption(ol, "word_field", &LemmatizeVMatrix::word_field, OptionBase::buildoption,
                  "Index (position) of word field\n");
    declareOption(ol, "pos_field", &LemmatizeVMatrix::pos_field, OptionBase::buildoption,
                  "Index (position) of POS field\n");
    declareOption(ol, "word_pos_to_lemma_table", &LemmatizeVMatrix::word_pos_to_lemma_table, OptionBase::buildoption,
                  "Customized table that uses the word and POS tag to obtain a lemma\n"
                  "It must have exactly three columns [word POS lemma]. It has the first priority.\n");
    declareOption(ol, "word_to_lemma_table", &LemmatizeVMatrix::word_to_lemma_table, OptionBase::buildoption,
                  "Customized table that uses the word to obtain a lemma\n"
                  "It must have exactly two columns [word lemma]. It has the second priority.\n");
    declareOption(ol, "pos_to_lemma_table", &LemmatizeVMatrix::pos_to_lemma_table, OptionBase::buildoption,
                  "Customized table that uses the POS tag to obtain a lemma\n"
                  "It must have exactly two columns [POS lemma] . It has the third priority.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void LemmatizeVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void LemmatizeVMatrix::build_()
{
    if(source)
    {
        if(word_field < 0 || word_field >= source->inputsize())
            PLERROR("In LemmatizeVMatrix::build_(): word_field(%d) is not in input part of source VMatrix", word_field);
        if(pos_field < 0 || pos_field >= source->inputsize())
            PLERROR("In LemmatizeVMatrix::build_(): pos_field(%d) is not in input part of source VMatrix", pos_field);
        inputsize_ = source->inputsize() + 1;
        targetsize_ = source->targetsize();
        weightsize_ = source->weightsize();
        width_ = inputsize_+targetsize_+weightsize_;
        length_ = source->length();

        if(word_pos_to_lemma_table.length() != 0 && word_pos_to_lemma_table.width() != 3)
           PLERROR("In LemmatizeVMatrix::build_(): word_pos_to_lemma_table doesn't have three columns");
        if(word_to_lemma_table.length() != 0 && word_to_lemma_table.width() != 2)
           PLERROR("In LemmatizeVMatrix::build_(): word_to_lemma_table doesn't have two columns");
        if(pos_to_lemma_table.length() != 0 && pos_to_lemma_table.width() != 2)
           PLERROR("In LemmatizeVMatrix::build_(): pos_to_lemma_table doesn't have two columns");

        string lemma;
        for(int i=0; i<length_;i++)
        {
            lemma = getLemma(i);
            lemma_dict->getId(lemma);
        }
        lemma_dict->update_mode = NO_UPDATE;
        src_row.resize(source->width());
    }
}

///////////////
// getNewRow //
///////////////
void LemmatizeVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i,src_row);
    v.subVec(0,inputsize_-1) << src_row.subVec(0,inputsize_-1);
    v[inputsize_-1] = lemma_dict->getId(getLemma(i));
    v.subVec(inputsize_,targetsize_+weightsize_) << src_row.subVec(inputsize_-1,targetsize_+weightsize_);
}

//! returns value associated with a string (or MISSING_VALUE if there's no association for this string)
real LemmatizeVMatrix::getStringVal(int col, const string & str) const
{
    int ret;
    if(col == inputsize_-1)
    {
        ret = lemma_dict->getId(str);
        if(ret == -1) return MISSING_VALUE;
        else return ret;
    }
    return source->getStringVal(col - (col < inputsize_-1?0:1),str);
}

string LemmatizeVMatrix::getValString(int col, real val) const
{
    if(is_missing(val))return tostring(val);
    if(col == inputsize_-1)
        return lemma_dict->getSymbol((int)val);
    return source->getValString(col - (col < inputsize_-1?0:1),val);
}

PP<Dictionary>  LemmatizeVMatrix::getDictionary(int col) const
{
    if(col == inputsize_-1)
        return lemma_dict;
    return source->getDictionary(col - (col < inputsize_-1?0:1));
}


Vec LemmatizeVMatrix::getValues(int row, int col) const
{
    if(row < 0 || row >= length_) PLERROR("In LemmatizeVMatrix::getValues() : invalid row %d, length()=%d", row, length_);
    if(col == inputsize_-1)
        return lemma_dict->getValues();
    return source->getValues(row,col - (col < inputsize_-1?0:1));
}

Vec LemmatizeVMatrix::getValues(const Vec& input, int col) const
{
    if(col == inputsize_-1)
        return lemma_dict->getValues();
    return source->getValues(input,col - (col < inputsize_-1?0:1));
}



/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LemmatizeVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(pos_to_lemma_table, copies);
    deepCopyField(word_to_lemma_table, copies);
    deepCopyField(word_pos_to_lemma_table, copies);
    deepCopyField(lemma_dict, copies);
    deepCopyField(src_row, copies);

    //PLERROR("LemmatizeVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
