// -*- C++ -*-

// TextStreamVMatrix.cc
//
// Copyright (C) 2007 Vytenis Sakenas
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

// Authors: Vytenis Sakenas

/*! \file TextStreamVMatrix.cc */


#include "TextStreamVMatrix.h"
#include <fstream>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
    TextStreamVMatrix,
    "Character based sliding window data matrix from given text file.",
    "Given the text file, and symbols that must be parsed this creates a data"
	"matrix that every row contains concecutive number_of_symbols chars "
	"expressed in one-hot vector. For example:\n"
	"text file contents: abaab\n"
	"symbols:	ab\n"
	"number_of_symbols: 3\n"
	"result matrix will be:\n"
	"1 0 0 1 1 0 <-- aba\n"
	"0 1 1 0 1 0 <-- baa\n"
	"1 0 1 0 0 1 <-- aab\n"
	"Position of 1 in one-hot vector is defined by chars position in symbols string."
    );

TextStreamVMatrix::TextStreamVMatrix()
/* ### Initialize all fields to their default value */
{
}

void TextStreamVMatrix::getNewRow(int i, const Vec& v) const
{
    v.subVec(0, symbol_width*number_of_symbols) << data.subVec( i*symbol_width, symbol_width*number_of_symbols);
    v[symbol_width*number_of_symbols] = target[i];
}

void TextStreamVMatrix::declareOptions(OptionList& ol)
{
     declareOption(ol, "data_file", &TextStreamVMatrix::data_file,
                   OptionBase::buildoption,
                   "File that contains data.");
     declareOption(ol, "number_of_symbols", &TextStreamVMatrix::number_of_symbols,
                   OptionBase::buildoption,
                   "Number of symbols to represent in one row (window size).");
     declareOption(ol, "symbols", &TextStreamVMatrix::symbols,
                   OptionBase::buildoption,
                   "Symbols that are represented. All symbols that are not in this string will be ignored. "
			"Also the index of a symbol will show which bit will be set to one in the one-hot vector.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void TextStreamVMatrix::build_()
{
	int ids[256];

	symbol_width = symbols.length();

	// Calculate id table
	memset(ids, -1, sizeof(ids));
	for (int i = 0; i < symbol_width; ++i)
		ids[(unsigned int)symbols[i]] = i;

        updateMtime(data_file);

	int text_length = 0;
	string text;

	// Read text from file
	// TODO: optimize
	ifstream fin(data_file.c_str(), ios::in | ios::binary);
	unsigned char chr;
	
	fin.unsetf(ios::skipws);

	while (fin >> chr) {
		if (ids[(unsigned int)chr] == -1) continue;
		text += chr;
		++text_length;
	}

	fin.close();

	data.resize(symbol_width*text_length);
	data.fill(0);

	target.resize(text_length - number_of_symbols);

	//cout << text_length << endl;
	for (int i = 0; i < text_length; ++i) {
		//cout << symbol_width*i << endl;
 		//cout << "id: " << ids[text[i]] << " " << i << endl;
		data[ symbol_width*i + ids[(unsigned int)text[i]] ] = 1;
	}

	for (int i = 0; i < text_length-number_of_symbols; ++i)
		target[i] = ids[(unsigned int)text[i + number_of_symbols]];

	length_ = text_length - number_of_symbols;
	width_ = symbol_width*number_of_symbols + 1;
	inputsize_ = symbol_width*number_of_symbols;
	targetsize_ = 1;
	weightsize_ = 0;
	extrasize_ = 0;
}

// ### Nothing to add here, simply calls build_
void TextStreamVMatrix::build()
{
    inherited::build();
    build_();
}

void TextStreamVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
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
