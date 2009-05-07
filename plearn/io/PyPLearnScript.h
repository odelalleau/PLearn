// -*- C++ -*-

// PyPLearnScript.h
//
// Copyright (C) 2005 Christian Dorion 
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

// Authors: Christian Dorion

/*! \file PyPLearnScript.h */


#ifndef PyPLearnScript_INC
#define PyPLearnScript_INC

#include <plearn/base/Object.h>
#include <plearn/io/PPath.h>
#include <plearn/io/openString.h>
#include <vector>

namespace PLearn {

class PyPLearnScript: public Object
{
public:
    // STATIC METHODS

// Althouth 'pyplearn_driver.py' is supposed to be in the path, it cannot be
// executed by NSPR under Windows. Thus we have to give it to the Python
// executable, which will need the full path to correctly run the script.
#ifdef WIN32
#define PYPLEARN_DRIVER_PATH \
    PPath("PLEARNDIR:scripts/pyplearn_driver.py").absolute()
#else
#define PYPLEARN_DRIVER_PATH "pyplearn_driver.py"
#endif

    /*!
      Given a filename, call an external process with the given name (default
      = "pyplearn_driver.py") to preprocess it and return the preprocessed
      version.  Arguments to the subprocess can be passed.
      Passing '--help' passes it unmodified to the subprogram
      and is assumed to print a help string.  If '--dump' is passed,
      the Python-preprocessed script is printed to standard output.
    */
    static PP<PyPLearnScript> process(
        const std::string& filename,
        const std::vector<std::string>& args = std::vector<std::string>(),
        const std::string& drivername        = PYPLEARN_DRIVER_PATH );

    /*!
      Given a filename, does a job similar to process() but parses \c argc &
      \c argv as given to a main(). Returns a PStream valid for reading an object.
    */
    static PStream openScriptFile(
        int argc, char** argv,
        const std::string& drivername = PYPLEARN_DRIVER_PATH );

    /*!
     * This static method forwards its arguments to process() and returns a 
     * pointer on an object of template type Obj by loading the resulting plearn 
     * script. Note that the object IS NOT BUILT since one may want to set other 
     * options prior to calling build().  
     */
    template<class Obj>
    static PP<Obj> load(
        const std::string& filename,
        const std::vector<std::string>& args = std::vector<std::string>(),
        const std::string& drivername        = PYPLEARN_DRIVER_PATH )
    {
        PP<PyPLearnScript> script = PyPLearnScript::process(filename, args, drivername);

        string plearn_script_string = script->plearn_script;
        PStream in = openString( plearn_script_string, PStream::plearn_ascii );
        PP<Obj> o  = new Obj();
        in >> o;

        return o;
    }

 
private:
    typedef Object inherited;

    //! This does the actual building. 
    void build_();

protected:
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // * public build options *
    // ************************

    //! The plearn_script
    std::string plearn_script;
  
    //! Variables set at command line
    std::map<std::string, std::string> vars;

    //! Script is in fact an help message
    bool do_help;

    //! Dump the script and forget it
    bool do_dump;  

    /*!
      Informations relative to the experiment settings, to be wrote in an
      expdir file.
    */
    std::string metainfos;

    //! The expdir of the experiment described by the script
    PPath expdir;

    //! The verbosity level to set for this experiment
    int verbosity;

    //! Modules for which logging should be activated
    vector<string> module_names;

    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    PyPLearnScript();

    //! Runs the embedded runnable object.
    virtual void run();

    //! Returns the internal script representation
    const string& getScript() { return plearn_script; }

    //! Saving metainfos to the expdir
    void close();
  
    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(PyPLearnScript);

    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PyPLearnScript);


//! Reads an object from the given filepath, performaing adequate 
//! preprocessing according to file extension.
//! Currently supported file extensions are: 
//! .psave : no preprocessing performed
//! .plearn .vmat : perform simple plearn macro processing
//! .pyplearn .pymat: use python preprocessor
//! The given args vector can be used to pass string arguments of the form argname=value.
//! The return_date is set to the lastest date of dependence of file. 
//!   Otherwise return (time_t)0. Work for .vmat, .psave and .plearn file.
Object* smartLoadObject(PPath filepath, const vector<string>& args, time_t& return_date);

//! Same as smartLoadObject(PPath, vector<string>, time_t) but passing an empty return_date
inline Object* smartLoadObject(PPath filepath, const vector<string>& args)
{ time_t d=0; return smartLoadObject(filepath, args, d); }
//! Same as smartLoadObject(PPath, vector<string>, time_t) but passing an empty vector<string>
inline Object* smartLoadObject(PPath filepath, time_t& return_date)
{ vector<string> args; args.push_back("");/* empty script filename*/ return smartLoadObject(filepath, args,return_date); }
//! Same as smartLoadObject(PPath, vector<string>, time_t) but passing an empty vector<string> 
//! and an empty return_date
inline Object* smartLoadObject(PPath filepath)
{time_t d=0;return smartLoadObject(filepath, d);}

  
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
