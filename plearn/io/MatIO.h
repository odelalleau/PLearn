// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef MatIO_INC
#define MatIO_INC

#include <plearn/math/TMat.h>
#include "fileutils.h"                  //!< For getNextNonBlankLine.
#include <plearn/base/stringutils.h>    //!< For toint.
#include <plearn/base/lexical_cast.h>   //!< For pl_strtod.
#include <plearn/io/openFile.h>

namespace PLearn {
using namespace std;


//! convenience construction from string
//! allows to write things such as Mat m = newMat(2,2, "1 2   3 4")
inline Mat makeMat(int length, int width, const string& values)
{ Mat m(length,width); m << values; return m; } 

inline Vec makeVec(int length, const string& values)
{ Vec v(length); v << values; return v; }

//! Tries to guess the format... (quite dumb right now)
//! This is currently what the old constructor with string argument did
void loadMat(const string& filename, TMat<float>& mat);
void loadMat(const string& filename, TMat<double>& mat);
void loadVec(const string& filename, TVec<float>& vec);
void loadVec(const string& filename, TVec<double>& vec);

//! Native PLearn binary format (.pmat)
void savePVec(const string& filename, const TVec<float>& vec);
void savePVec(const string& filename, const TVec<double>& vec);
void loadPVec(const string& filename, TVec<float>& vec);
void loadPVec(const string& filename, TVec<double>& vec);
void savePMat(const string& filename, const TMat<float>& mat);
void savePMat(const string& filename, const TMat<double>& mat);
//! Will save the fieldnames in corresponding pmatfilename.metadata/fieldnames (creating the metadata directory if necessary)
void savePMatFieldnames(const string& pmatfilename, const TVec<string>& fieldnames);
void loadPMat(const string& filename, TMat<float>& mat);
void loadPMat(const string& filename, TMat<double>& mat);

//! WARNING: use only for float, double, and int types. Other type are not guaranteed to work

//! intelligent functions that will load a file in almost all ascii formats that ever existed in this lab
template<class T>
void loadAscii(const PPath& filename, TMat<T>& mat, TVec<string>& fieldnames, int& inputsize, int& targetsize, int& weightsize, TVec<map<string,real> >* map_sr=0);
template<class T> void loadAscii(const PPath& filename, TMat<T>& mat, TVec<string>& fieldnames, TVec<map<string,real> >* map_sr = 0);
template<class T> void loadAscii(const PPath& filename, TMat<T>& mat);

void parseSizeFromRemainingLines(const PPath& filename, PStream& in, bool& could_be_old_amat, int& length, int& width);

// norman: added another function to solve the internal compiler error of .NET when using
// default parameter with templates. See old declaration:
//template<class T> void saveAscii(const string& filename, const TMat<T>& mat, 
//                                const TVec<string>& fieldnames = TVec<string>() );
template<class T> void saveAscii(const string& filename, const TMat<T>& mat, 
                                 const TVec<string>& fieldnames, 
                                 int inputsize=-1, int targetsize=-1, int weightsize=-1, int extrasize=0);
template<class T> void saveAscii(const string& filename, const TMat<T>& mat);

//! first number in file is length
template<class T> void saveAscii(const string& filename, const TVec<T>& vec);
template<class T> void loadAscii(const PPath& filename, TVec<T>& vec);

//! Format readable by gnuplot
void loadGnuplot(const string& filename, Mat& mat);
void saveGnuplot(const string& filename, const Vec& vec);
void saveGnuplot(const string& filename, const Mat& mat);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Format readable by matlab

/*! 
  The following two are simply calling the matrix version after transforming 
  the Vec in a one column Mat. See below.
*/
void matlabSave( const PPath& dir, const string& plot_title, const Vec& data, 
                 const Vec& add_col, const Vec& bounds, string lengend="", bool save_plot=true);  
void matlabSave( const PPath& dir, const string& plot_title, 
                 const Vec& xValues,
                 const Vec& yValues, const Vec& add_col, const Vec& bounds, string lengend="", bool save_plot=true);  

//! Simply calls the coming matlabSave function with an empty xValues Vec. See below.
void matlabSave( const PPath& dir, const string& plot_title, const Mat& data, 
                 const Vec& add_col, const Vec& bounds, TVec<string> legend=TVec<string>(), bool save_plot=true);

/*! 
  This is the *real* matlabSave function.

  1) If xValues is empty, the yValues are plotted against the row indices.
  
  2) If xValues is not empty and its length is not equal to the length of yValues, 
  then its length must be one and the value xValues[0] will be the start index for the xValues.
*/
void matlabSave( const PPath& dir, const string& plot_title, 
                 const Vec& xValues,
                 const Mat& yValues, const Vec& add_col, const Vec& bounds, TVec<string> legend=TVec<string>(), bool save_plot=true);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//!  Reads and writes an ascii file without the size header (assuming that the size(length() and width()) is set)
void loadAsciiWithoutSize(const string& filename, const Vec& vec);
void saveAsciiWithoutSize(const string& filename, const Vec& vec);
void loadAsciiWithoutSize(const string& filename, const Mat& mat);
void saveAsciiWithoutSize(const string& filename, const Mat& mat);

//!  SN Format
Mat loadSNMat(const string& filename);
Vec loadSNVec(const string& filename);
void saveSNMat(const string& filename, const Mat& mat);
void saveSNVec(const string& filename, const Vec& vec);

//!  Native AD format
Mat loadADMat(const string& filename);
Vec loadADVec(const string& filename);

/*!     UCI machine-learning-database format
  Format used for most of the UCI machine-learning-database.
  The missing value is represented with the '?' character
  in the source file, and with the MISSING_VALUE in the Mat.
  If some symbols are detected then integer codes are assigned to
  them (by sorting them for each symbolic column in lexicographic
  order). The *to_symbols table has one element per column,
  each of which is a table of strings. The number of strings
  (i.e., symbols) for each column is given in the table *to_n_symbols. 
  Additionally, if provided, the 'max_in_col' vector will contain
  the (rounded to lowest integer) value of the maxium in each column
  (this will be -1 if there is no numerical value in the column).
  Also, if 'header_columns' vector is provided, the first line is considered
  to be the header and the vector will contain the column names.
*/
Mat loadUCIMLDB(const string& filename, char ****to_symbols=0, int **to_n_symbols=0, TVec<int>* max_in_col = 0, TVec<string>* header_columns = 0);

/*!     STATLOG machine-learning-database-format
  Format used for most of the STATLOG machine-learning-database.
  The missing value is represented with the '?' character
  in the source file, and with the MISSING_VALUE in the Mat.
  If some symbols are detected then integer codes are assigned to
  them (by sorting them for each symbolic column in lexicographic
  order). The *to_symbols table has one element per column,
  each of which is a table of strings. The number of strings
  (i.e., symbols) for each column is given in the table *to_n_symbols. 
*/
Mat loadSTATLOG(const string& filename, char ****to_symbols=0, int **to_n_symbols=0);

/*!     read a file in JPEG format (read the RGB components).
  this will be resized to a (npixels x 3) matrix, where
  the (R,G,B) pixels are ordered by rows of the original image.
  To figure the actual image dimensions, the row size is also
  returned (so the number of columns is length()/row_size).
  An optional subsampling factor can be given (1,2,4 or 8) 
  The R,G,B components always range from 0 to 255.
*/
void loadJPEGrgb(const string& jpeg_filename, Mat& rgbmat, int& row_size, int scale = 1);


template<class T>
void loadAscii(const PPath& filename, TMat<T>& mat, TVec<string>& fieldnames, TVec<map<string,real> >* map_sr)
{ 
    int inputsize=-1, targetsize=-1, weightsize=-1, extrasize=-1;
    loadAscii(filename, mat, fieldnames, inputsize, targetsize, weightsize, extrasize, map_sr);
}

template<class T>
void loadAscii(const PPath& filename, TMat<T>& mat, TVec<string>& fieldnames, int& inputsize, int& targetsize, int& weightsize, TVec<map<string,real> >* map_sr)
{
    int extrasize = -1;
    loadAscii(filename, mat, fieldnames, inputsize, targetsize, weightsize, extrasize, map_sr);
}

// Intelligent function that will load a file in almost all ascii formats that ever existed in this lab.
// Additionally, if 'map_sr' is provided, it will fill it with the string -> real mappings encountered.
template<class T>
void loadAscii(const PPath& filename, TMat<T>& mat, TVec<string>& fieldnames, int& inputsize, int& targetsize, int& weightsize, int& extrasize, TVec<map<string,real> >* map_sr)
{
    PStream in = openFile(filename, PStream::raw_ascii, "r");
  
    int length = -1;
    int width = -1;
    inputsize = -1;
    targetsize = -1;
    weightsize = 0;
    bool could_be_old_amat=true; // true while there is still a chance that this be an "old" amat format (length and width in first row with no starting ##)
  
    in >> ws;
    string line;
  
    while(in.peek()=='#')
    {
        in.getline(line);
        could_be_old_amat = false;

        size_t pos=line.find(":");
        if(pos!=string::npos)
        {
            string sub=line.substr(0,pos);
            if(sub=="#size") // we've found the dimension specification line
            {
                string siz=removeblanks((line.substr(pos)).substr(1));
                vector<string> dim = split(siz," ");
                if(dim.size()!=2)  PLERROR("I need exactly 2 dimensions for matrix");
                length = toint(dim[0]);
                width = toint(dim[1]);
            }
            else if(sub=="#sizes") // we've found inputsize targetsize weightsize specification
            {
                string siz=removeblanks((line.substr(pos)).substr(1));
                vector<string> dim = split(siz," ");
                if(dim.size()!=3 && dim.size()!=4)  
                    PLERROR("I need 3 or 4 numbers after #sizes: inputsize targetsize weightsize extrasize");
                inputsize = toint(dim[0]);
                targetsize = toint(dim[1]);
                weightsize = toint(dim[2]);
                if(dim.size()==4)
                    extrasize = toint(dim[3]);
                else
                    extrasize = 0;
            }
            else if(sub=="#") // we've found the fieldnames specification line
            {
                string fnl=line.substr(pos).substr(1);
                fieldnames = split(fnl," ");
                width=fieldnames.size();
            }              
        }
        in >> ws;
    }

    if(length==-1)  // still looking for size info...
        parseSizeFromRemainingLines(filename, in, could_be_old_amat, length, width);

    if(length==-1)
        PLERROR("In loadAscii: trying to load but couldn't determine file format automatically for %s",filename.absolute().c_str());

    if(width != -1 && width != fieldnames.length())
    {
        if (fieldnames.length() != 0)
            PLWARNING("In loadAscii:  Number of fieldnames (%d) and width (%d) mismatch in file %s.  "
                      "Replacing fieldnames by 'Field-0', 'Field-1', ...", 
                      fieldnames.length(), width, filename.absolute().c_str());
        fieldnames.resize(width);
        for(int i= 0; i < width; ++i)
            fieldnames[i]= string("Field-") + tostring(i);
    }
  
    // We are now more careful about the possibility of the stream being in a
    // bad state. The sequel in.seekg(0); in.clear(); did not seem to do the job.
    in = PStream(); // Close file.
    PStream loadmat = openFile(filename, PStream::raw_ascii, "r");
  
    mat.resize(length,width);
    TVec<int> current_map(width);
    current_map.fill(1001);   // The value of the string mapping we start with.
    TVec<T> current_max(width); // The max of the numerical values in each column.
    current_max.clear();
    // Initialize the mappings to empty mappings.
    if (map_sr) 
        map_sr->resize(width);

    string inp_element;
    for(int i=0; i<length; i++)
    {
        T* mat_i = mat[i];
        skipBlanksAndComments(loadmat);
        for(int j=0; j<width; j++) 
        {
            if (loadmat) 
            {
                loadmat >> inp_element;
                if (pl_isnumber(inp_element)) 
                {
                    mat_i[j] = pl_strtod(inp_element.c_str(), 0);
                    if (map_sr) 
                    {
                        T val = mat_i[j];
                        // We need to make sure that this number does not conflict
                        // with a string mapping.
                        if (val > current_max[j])
                            current_max[j] = val;
                        if (current_max[j] >= current_map[j])
                            current_map[j] = int(current_max[j] + 1);
                        map<string,real>& m = (*map_sr)[j];
                        for (map<string,real>::iterator it = m.begin(); it != m.end(); it++) 
                        {
                            if (fast_exact_is_equal(it->second, val)) 
                            {
                                // We're screwed, there is currently a mapping between a string
                                // and this numeric value. We have to change it.
                                // We pick either the next string mapping value, or the current
                                // max in the column (+ 1) if it is larger.
                                int cur_max_plus_one = int(real(current_max[j]) + 1);
                                if (cur_max_plus_one > current_map[j]) 
                                {
                                    it->second = cur_max_plus_one;
                                    current_map[j] = cur_max_plus_one;
                                } 
                                else
                                    it->second = current_map[j];
                                current_map[j]++;
                                // In addition, we have to modify all previous data, which sucks.
                                for (int k = 0; k < i; k++) 
                                {
                                    if (fast_exact_is_equal(mat(k, j), val))
                                        mat(k, j) = it->second;
                                }
                            }
                        }
                    }
                } 
                else 
                {
                    // This is a string!
                    if (map_sr) // Already encountered ?
                    {                        
                        map<string,real>& m = (*map_sr)[j];
                        map<string,real>::iterator it = m.find(inp_element);
                        if(it != m.end()) // It already exists in the map.
                        {
                            mat_i[j] = it->second;
                        } 
                        else           // We need to add it.
                        {                            
                            (*map_sr)[j][inp_element] = current_map[j];
                            mat_i[j] = current_map[j];
                            current_map[j]++;
                        }
                    } 
                    else
                        PLERROR("In loadAscii - You need to provide 'map_sr' if you want to load an ASCII file with strings");
                }
            } 
            else 
            {
                PLERROR("In loadAscii - Missing values are not supported anymore (for the moment)");
                /* Old code, not PStream-compatible.
                   if (!loadmat) {
                   // loadmat.clear();
                   mat_i[j] = MISSING_VALUE;
                   }
                */
            }
        }
    }
}

//! Load an ASCII matrix whose format is:
//! (entry_name, long_binary_dscriptor)
//! with 'long_binary_dscriptor' being of the form '001100101011',
//! each character being an entry of the matrix.
//! (entry_name is ignored).
//! Header must be: #size: length width
template<class T>
void loadAsciiSingleBinaryDescriptor(const PPath& filename, TMat<T>& mat)
{
    PStream in = openFile(filename, PStream::raw_ascii, "r");

    int length = -1;
    int width = -1;
  
    in >> ws;
    string line;

    while(in.peek()=='#')
    {
        in.getline(line);
 
        size_t pos=line.find(":");
        if(pos!=string::npos)
        {
            string sub=line.substr(0,pos);
            if(sub=="#size") // we've found the dimension specification line
            {
                string siz=removeblanks((line.substr(pos)).substr(1));
                vector<string> dim = split(siz," ");
                if(dim.size()!=2)  PLERROR("In loadAsciiSingleBinaryDescriptor: I need exactly 2 dimensions for matrix");
                length = toint(dim[0]);
                width = toint(dim[1]);
            }
        }
        in >> ws;
    }

    if(length==-1)  // still looking for size info...
    {
        PLERROR("In loadAsciiSingleBinaryDescriptor: Be nice and specify a width and length");
    }

    // We are now more careful about the possibility of the stream being in a
    // bad state.
    mat.resize(length,width);
    string inp_element;
    for(int i=0; i<length; i++)
    {
        T* mat_i = mat[i];
        skipBlanksAndComments(in);
        in >> inp_element;  // Read the entry name.
        in >> inp_element;  // Read the binary descriptor.
        if (inp_element.length() != (unsigned int) width) {
            PLERROR("In loadAsciiSingleBinaryDescriptor, a descriptor isn't the right size");
        }
        for(int j=0; j<width; j++) {
            mat_i[j] = pl_strtod(inp_element.substr(j,1).c_str(), 0);
        }
    }
}

template<class T>
void loadAscii(const PPath& filename, TVec<T>& vec)
{
    ifstream in(filename.absolute().c_str());
    if(!in)
        PLERROR("In loadAscii could not open file %s for reading",filename.c_str());
 
    int size = -1;
    in >> size;
    if (size<0 || size>1e10)
        PLERROR("In Vec::loadAscii the file is probably not in the right format: size=%d", size);
    vec.resize(size);
    typename TVec<T>::iterator it = vec.begin();
    typename TVec<T>::iterator itend = vec.end();

    // We are now more careful about the possibility of the stream being in a
    // bad state
    string inp_element;
    for(; it!=itend; ++it) {
        if (in) {
            in >> inp_element;
            *it = pl_strtod(inp_element.c_str(), 0);
        }
        if (!in) {
            in.clear();
            *it = MISSING_VALUE;
        }
    }
}

// norman: very stupid function to solve compiler error of VS .NET (see declaration)
template<class T>
void saveAscii(const string& filename, const TMat<T>& mat)
{
    saveAscii(filename, mat, TVec<string>());
}

template<class T> 
void saveAscii(const string& filename, const TMat<T>& mat, const TVec<string>& fieldnames,
               int inputsize, int targetsize, int weightsize, int extrasize)
{
    ofstream out(filename.c_str());
    if (!out)
        PLERROR("In saveAscii could not open file %s for writing",filename.c_str());

    out << "#size: "<< mat.length() << ' ' << mat.width() << endl;
    out.precision(15);
    if(fieldnames.size()>0)
    {
        out << "#: ";
        for(int k=0; k<fieldnames.size(); k++)
            //there must not be any space in a field name...
            out << space_to_underscore(fieldnames[k]) << ' ';
        out << endl;
    }
    if(inputsize>=0)
        out << "#sizes: " << inputsize << ' ' << targetsize << ' ' << weightsize << ' ' << extrasize << endl;

    for(int i=0; i<mat.length(); i++) 
    {
        const T* row_i = mat[i];
        for(int j=0; j<mat.width(); j++)
            out << row_i[j] << ' ';
        out << '\n';
    }
}
  
template<class T> 
void saveAscii(const string& filename, const TVec<T>& vec)
{
    ofstream out(filename.c_str());
    if (!out)
        PLERROR("In saveAscii: could not open file %s for writing",filename.c_str());

    out << vec.length() << endl;
    out.precision(15);

    typename TVec<T>::iterator it = vec.begin();
    typename TVec<T>::iterator itend = vec.end();
    for(; it!=itend; ++it)
        out << *it << ' ';
    out << endl;
}

template<class T>
void loadAscii(const PPath& filename, TMat<T>& mat)
{
    TVec<std::string> fn;
    loadAscii(filename,mat,fn);
}

} // end of namespace PLearn


#endif


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
