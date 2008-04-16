// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef getDataSet_INC
#define getDataSet_INC

#include <string>
#include <map>
#include <time.h>

namespace PLearn {
using namespace std;

class VMat;
class PPath;

//! Return help on the dataset syntax.
string getDataSetHelp();

//! Return the last time a dataset was modified.
time_t getDataSetDate(const PPath& dataset_path);

//! Return the dataset pointed by 'dataset_path'.
VMat getDataSet(const PPath& dataset_path);


/**
 *  Extension registrar for new file types.  If you want to extend getDataSet
 *  so that it recognizes new extensions (and constructs the correct type of
 *  VMatrix automatically), you should instantiate a static
 *  VMatrixExtensionRegistrar object within the .cc of your new VMatrix type.
 *  This object will automatically register the new extension with getDataSet,
 *  so that when you try to open a file of that extension, the instantiation
 *  function is called.  For example:
 *
 *      VMatrixExtensionRegistrar(
 *          "fancyext",                      // Note: no leading period
 *          &FancyVMatrix::instantiateFromPPath,
 *          "Fancy new file format, giving a FancyVMatrix")
 *
 *  This associates files with the ".fancyext" extension with a construction
 *  function (that must take a single PPath argument and returns a VMat).  The
 *  last argument is some documentation.
 */
class VMatrixExtensionRegistrar
{
public:
    typedef VMat (*VMatrixInstantiator)(const PPath& filename);
    typedef map<string,VMatrixExtensionRegistrar> ExtensionMap;
    
public:
    VMatrixExtensionRegistrar(const string& file_extension,
                              VMatrixInstantiator instantiation_function,
                              const string& documentation);

    //! Return the documentation associated with a registrar
    const string& documentation() const
    {
        return m_documentation;
    }
    
    
    //#####  Static Interface  ################################################
    
    //! Return the list of all registered extensions
    static const ExtensionMap& registeredExtensions()
    {
        return registeredExtensionsAux();
    }

    //! Register a new extension
    static void registerExtension(const VMatrixExtensionRegistrar& new_extension);

    //! Return the instantiator given an extension, or NULL if not found
    static VMatrixInstantiator getInstantiator(const string& file_extension);

private:
    // Members
    string m_file_extension;
    VMatrixInstantiator m_instantiator;
    string m_documentation;
    
    //! Set of registered extensions
    static ExtensionMap& registeredExtensionsAux();
};

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
