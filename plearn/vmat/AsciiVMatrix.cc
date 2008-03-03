// -*- C++ -*-

// AsciiVMatrix.cc
//
// Copyright (C) 2003 Rejean Ducharme, Pascal Vincent
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

/*! \file AsciiVMatrix.cc */

#include "AsciiVMatrix.h"
#include <plearn/base/stringutils.h>
#include <plearn/io/fileutils.h>      //!< For isfile()
#include <plearn/io/openFile.h>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(AsciiVMatrix,
                        "AsciiVMatrix implements a file in ASCII format",
                        ""
    );

AsciiVMatrix::AsciiVMatrix()
    :readwritemode(false), newfile(true),
     rewrite_length(true)
{}

AsciiVMatrix::AsciiVMatrix(const string& fname, bool readwrite)
    :filename(fname), readwritemode(readwrite), newfile(false),
     rewrite_length(true)
{
    build();
}

AsciiVMatrix::AsciiVMatrix(const string& fname, int the_width,
                           const TVec<string>& the_fieldnames,
                           const string& comment)
    :inherited(0,the_width), filename(fname),
     readwritemode(true), newfile(true), rewrite_length(true)
{
    inherited::build();

    if (isfile(filename))
        PLERROR("In AsciiVMatrix constructor: filename %s already exists",filename.c_str());
    file = openFile(filename, PStream::raw_ascii, "w");
    // TODO Is this really the same as old code ?
    // file->open(filename.c_str(), fstream::in | fstream::out | fstream::trunc);

    file << "#size: ";
    PLERROR("In AsciiVMatrix::AsciiVMatrix - Code is not yet PStram compatible");
    // vmatlength_pos = file.tellp(); // TODO See how to do this with PStreams.
    length_max = 9999999;  // = 10 000 000 - 1
    file << "0 " << width() << "      " << endl;


    if(the_fieldnames.length()>0)
    {
        if(the_fieldnames.length()!=the_width)
            PLERROR("In AsciiVMatrix constructor: number of given fieldnames (%d) differs from given width (%d)",
                    the_fieldnames.length(), the_width);
        file << "#: ";
        for(int k=0; k<the_width; k++)
        {
            string field_k = space_to_underscore(the_fieldnames[k]);
            declareField(k, field_k);
            file << field_k << ' ';
        }
        file << endl;
    }

    if(comment!="")
    {
        if(comment[0]!='#')
            PLERROR("In AsciiVMatrix constructor: comment must start with a #");
        file << comment;
        if(comment[comment.length()-1]!='\n')
            file << endl;
    }

}

void AsciiVMatrix::build()
{
    inherited::build();
    build_();
}

void AsciiVMatrix::build_()
{
    updateMtime(filename);

    if(!newfile)  // open old file
    {
        if (!isfile(filename))
            PLERROR("In AsciiVMatrix constructor (with specified width), filename %s does not exists",filename.c_str());
        file = openFile(filename, PStream::raw_ascii, "w");
        // TODO Is this really the same as the old code ?
        // file->open(filename.c_str(), fstream::in | fstream::out);

        // read the matrix in old or new format
        int length = -1;
        int width = -1;
        bool could_be_old_amat = true;
        // file->seekg(0,fstream::beg); // TODO Will it work without this ?
        file >> ws;
        string line;
        while (file.peek()=='#')
        {
            PLERROR("In AsciiVMatrix::build_ - Code is not PStream-compatible yet");
            // streampos old_pos = file.tellg(); // TODO See how to do this with PStreams.
            file.getline(line);
            could_be_old_amat = false;
            size_t pos=line.find(":");
            if (pos!=string::npos)
            {
                string sub=line.substr(0,pos);
                if (sub=="#size") // we've found the dimension specification line
                {
                    string siz=removeblanks((line.substr(pos)).substr(1));
                    vector<string> dim = split(siz," ");
                    if (dim.size()!=2)  PLERROR("I need exactly 2 dimensions for matrix");
                    length = toint(dim[0]);
                    width = toint(dim[1]);

                    // we set vmatlength_pos
                    // streampos current_pos = file.tellg(); // TODO See how to do this with PStreams.
                    PLERROR("In AsciiVMatrix::build_ - Code is not PStream-compatible yet");
                    // file->seekg(old_pos); // TODO How to do this with PStreams ?
                    char c = file.get();
                    while (c != ':')
                    {
                        c = file.get();
                    }
                    c = file.get();
                    // vmatlength_pos = file.tellp(); // TODO See how to do this with PStreams.
                    //file.seekg(current_pos); // TODO How to do this with PStreams ?

                    // we set length_max
                    int width_ndigits = (int)pl_log(real(width)) + 1;
                    string remain = removenewline(line.substr(pos+1));
                    int length_ndigits = (int)remain.length() - width_ndigits - 1;
                    length_max = (int)pow(10.0,double(length_ndigits)) - 1;
                }
            }
            file >> ws;
        }

        if (length==-1)  // still looking for size info...
        {
            getNextNonBlankLine(file,line);
            int nfields1 = (int)split(line).size();
            getNextNonBlankLine(file,line);
            if (line=="") // only one line, no length nor width info
            {
                length=1;
                width=nfields1;
                rewrite_length = false;
                could_be_old_amat = false;
            }
            int nfields2 = (int)split(line).size();
            int guesslength = countNonBlankLinesOfFile(filename);
            real a, b;
            if (could_be_old_amat && nfields1==2) // could be an old .amat with first 2 numbers being length width
            {
                PLERROR("In AsciiVMatrix::build_ - Code is not PStream-compatible yet");
                // file.seekg(0,fstream::beg); // TODO How to do this with PStreams ?
                // file.clear(); // TODO Same question
                // vmatlength_pos = file.tellp(); // TODO See how to do this with PStreams.
                file >> a >> b;
                if (guesslength == int(a)+1              &&
                    fast_exact_is_equal(real(int(a)), a) &&
                    fast_exact_is_equal(real(int(b)), b) &&
                    a>0 && b>0 && int(b)==nfields2) // it's clearly an old .amat
                {
                    length = int(a);
                    width = int(b);

                    // file.seekg(vmatlength_pos); // TODO How to do this with PStreams ?
                    file.getline(line);
                    int width_ndigits = (int)pl_log(real(width)) + 1;
                    int max_length_ndigits = (int)line.length() - width_ndigits - 1;
                    length_max = (int)pow(10.0,double(max_length_ndigits)) - 1;
                }
            }

            if (length==-1) // still don't know size info...
            {
                if (nfields1==nfields2) // looks like a plain ascii file
                {
                    rewrite_length = false;
                    length=guesslength;
                    if (width!=-1 && width!=nfields1)
                    {
                        PLWARNING("In AsciiVMatrix:  Number of fieldnames and width mismatch in file %s.", filename.c_str());
                    }
                    width = nfields1;
                }
                else
                    PLERROR("In AsciiVMatrix: trying to load but couldn't determine file format automatically for %s",filename.c_str());
            }
        }

        length_ = length;
        width_ = width;

        // build the vector of position of the begining of the lines
        PLERROR("In AsciiVMatrix::build_ - Code is not PStream-compatible yet");
        // file.seekg(0,fstream::beg); // TODO How to do this with PStreams ?
        // file.clear();
        if (could_be_old_amat && rewrite_length)
        {
            file.getline(line);
        }
        pos_rows.clear();
        while (!file.eof())
        {
            file >> ws;
            // if (file.peek()!='#' && file.peek()!=EOF) pos_rows.push_back(file.tellg()); // TODO See how to do this with PStreams.
            file.getline(line);
        }
        // file.clear(); // TODO See how to do this with PStreams.
        if ((int)pos_rows.size() != length)
            PLERROR("In AsciiVMatrix: the matrix has not the rigth size");
    }
}

void AsciiVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>length())
        PLERROR("In AsciiVMatrix::getNewRow, bad row number %d",i);
    if(v.length() != width())
        PLERROR("In AsciiVMatrix::getNewRow, length of v (%d) does not match matrix width (%d)",v.length(),width());
#endif

    // file.seekg(pos_rows[i]); // TODO How to do this with PStreams ?
    for (int j=0; j<width(); j++)
        file >> v[j];
}

void AsciiVMatrix::appendRow(Vec v)
{
    if(v.length()!=width())
        PLERROR("In AsciiVMatrix::appendRow, length of Vec to append (%d) differs from width of matrix (%d)",v.length(), width());

    if (length() == length_max)
        PLERROR("AsciiVMatrix::appendRow aborted: the matrix has reach its maximum length.");

    if(!readwritemode)
        PLERROR("AsciiVMatrix::appendRow aborted: the vmat was opened in read only format.");

    // write the Vec at the end of the file
    // file.seekp(0,fstream::end); // TODO How to do this with PStreams ?
    // pos_rows.push_back(file.tellg()); // TODO How to do this with PStreams ?

    // file.precision(15); // TODO See what it means to remove it.
    for (int i=0; i<v.length(); i++)
        file << v[i] << ' ';
    file << endl;

    // update the length
    length_++;
    if (rewrite_length)
    {
        // file->seekp(vmatlength_pos); // TODO How to do this with PStreams ?
        file << length() << " " << width();
    }
}

void AsciiVMatrix::put(int i, int j, real value)
{ PLERROR("In AsciiVMatrix::put not permitted."); }
void AsciiVMatrix::putSubRow(int i, int j, Vec v)
{ PLERROR("In AsciiVMatrix::putSubRow not permitted."); }
void AsciiVMatrix::putRow(int i, Vec v)
{ PLERROR("In AsciiVMatrix::putRow not permitted."); }

void AsciiVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "filename", &AsciiVMatrix::filename, OptionBase::buildoption, "Filename of the matrix");
    declareOption(ol, "readwritemode", &AsciiVMatrix::readwritemode, OptionBase::buildoption, "Is the file to be opened in read/write mode");
    inherited::declareOptions(ol);
}

AsciiVMatrix::~AsciiVMatrix()
{}

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
