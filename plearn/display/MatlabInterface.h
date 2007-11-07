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
   * AUTHORS: Christian Jauvin
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearn/plearn/display/MatlabInterface.h */

#ifndef MATLAB_INTERFACE_INC
#define MATLAB_INTERFACE_INC

//#include <stdlib.h>
//#include "stringutils.h"
#include <plearn/sys/Popen.h>
#include <plearn/math/RowMapSparseMatrix.h>

namespace PLearn {
using namespace std;

/*!   This class permits the execution of a Matlab sub-routine from
  within a PLearn program.  The parametrization of the sub-routine
  (if required) is performed by the concatenation of a set of "lines"
  (a header), optionnally followed by an existing M-file (Matlab program file
  format).  Here is an example that shows how it works :
  
  Suppose you want to execute the following matlb lines:
  
   load mymat -ascii;
   [U,S,V] = svd(mymat);
   save U.dat U -ascii;
   save S.dat S -ascii;
   
  then you just do the following:
  
  MatlabInterface matlab("load mymat -ascii; 
                          [U,S,V] = svd(mymat);
                          save U.dat U -ascii; 
                          save S.dat S -ascii; 
                          fprintf(1,'done\n');");
  matlab.launchAndWaitFor("done");
  
  Where you should note the fprintf of 'done' in the matlab
  instructions so as to be able to detect the end of the matlab
  execution.
  
  Another way to use this class is to concatenate these explicitly
  given lines of code with an existing matlab file.
  Suppose you have defined the file foo.m :
  
  %file foo.m
  
  a = 1;
  a = b + c;
  
  %%%%%%%%%%%
  
  This execution of this file, by itself, would generate an error
  from the Matlab interpreter, stating that the variable b is not
  defined.  You can then declare a MatlabInterface instance, with the
  line "b = 2;" as the header, the program will acquire a correct
  meaning.  The underlying M-file will then become (exactly) :
  
  b = 2;
  %file foo.m
  
  a = 1;
  a = b + c;
  
  %%%%%%%%%%%
  
  Another usage that you may find interesting is passing a matrix
  from a PLearn program to a Matlab program.  Suppose you have
  defined a RowMapSparseMatrix M in a PLearn class.  You want to pass
  it to a Matlab program, let's say foo.m again.  First you have to
  export M to a format that Matlab can read :
  
  M.exportToMatlabReadableFormat("Mijv.dat");
  
  You can then generate a header using a STL vector :
  
  vector<string> header;
  header.push_back("load Mijv.dat;");
  header.push_back("M = spconvert(Mijv);");
  
  You can then use that header, along with a M-file that makes use of
  the sparse matrix M, to initialize a MatlabInterface.
  
  NOTE : to avoid any process conflicts at run-time, the M-file being
  executed is not the one supplied, but a new one, that has a unique
  id.  This is the case, even if you want to execute a M-file that
  requires no header.
  
  An example of application of MatlabInterface is given after this
  class definition, for finding eigen-pairs of a symmetric matrix
  using the eigs() program of matlab r11 on a RowMapSarseMatrix.
  
*/
  class MatlabInterface
  {

    public:
    
    //! returns where to find appropriate .m files
    //! will be appended to the matlab path
    static string path() 
    {
      char* plearndir = getenv("PLEARNDIR");
      if(!plearndir)
        PLERROR("PLEARNDIR environment variable not defined");
      return string(plearndir)+"/Contrib/matlab/";
    }

/*!   These constructors have 3 fields in common :
  
  id : if this is set to "", a unique id will be generated for this
  particular Matlab execution.
  
  launch_in_background : set this flag to true if you want to run
  Matlab in background.
  
  erase_tmp_files : set this flag to true if you don't want any
  residual files remaining after the execution.  If there is any
  trouble with the Matlab execution, you can examine the M-file that
  is the result of the concatenation of the header and the
  pre-defined M-file by setting this flag to false.
*/

/*!   Use this constructor if the Matlab program you want to run does not
  need any parametrization (the M-file will be executed as is the
  Matlab interpreter.  No header is necessary.
*/

/*!       DEPRECATED CONSTRUCTOR.
      MatlabInterface(string matlab_file, string id = "", 
      bool launch_in_background = false, bool erase_tmp_files = true);
*/

/*!   Use this constructor if your entire header is contained in a
  string.  Be careful to delimitate the lines with '\n'. If
  matlab_file=="" then no matlab_file is concatenated.
*/
      MatlabInterface(string matlab_file_header, string matlab_file = "", string id = "",
		      bool launch_in_background = false, bool erase_tmp_files = true);

      MatlabInterface(vector<string> matlab_file_header, string
                      matlab_file = "", string id = "",
		      bool launch_in_background = false, bool erase_tmp_files = true);

/*!   This method will run the Matlab interpreter on the prepared
  M-file.  A pointer to the Popen communication object is returned,
  for you to manage the termination of the program.  One way to do
  that is to poll the "in" field of the Popen object, waiting for an
  answer from Matlab :
  
  MatlabInterface matlab(...);
  Popen* p = matlab.launch();
  string answer;
  
  do
  {
    p->in >> answer;
  } while (answer != <desired answer>);
  
*/
      Popen* launch();
  
/*!   The method of "waiting for Matlab" described above is encapsulated
  in this function, where all you have to supply is the Matlab answer
  stating that the program is done. This method will also stop
  in case of matlab PLERROR(detected with the string "???"),
  and will return false in that case (return true if everything works).
*/
      bool launchAndWaitFor(string matlab_end_answer);

//!  The communication object that contains the Matlab command line.
      Popen* matlab;

//!  The pre-defined M-file.
      string matlab_file;

//!  The user-supplied header, that will be concatenated to the top of
//!  the M-file.
      string matlab_file_header;

//eigs_r11 routine (see below).
      static void eigs_r11(RowMapSparseMatrix<real>& A, 
			   Mat& evectors, 
			   int d, 
			   string which_eigenvalues, 
			   bool erase_tmp_files = true);

      static void eigs_r11(RowMapSparseMatrix<real>& A, 
			   Vec& evalues, 
			   int d, 
			   string which_eigenvalues, 
			   bool erase_tmp_files = true);

      static void eigs_r11(RowMapSparseMatrix<real>& A, 
			   Mat& evectors, 
			   Vec& evalues, 
			   int d, 
			   string which_eigenvalues, 
			   bool erase_tmp_files = true);

  protected:

//!  This is a unique id for a particular Matlab execution (to avoid conflicts).
      string id;

//!  Some flags.
      bool launch_in_background;

      bool erase_tmp_files;

  };

/*!     Compute k eigen-values / eigen-vectors of a sparse symmetric matrix
    using the eigs program of matlab-r11 (see
    PLearn/Contrib/matlab/eigs_r11.m).
    The 'which_eigenvalues' argument specifies which eigenvalues are
    desired:
     a number         the k eigen-values closest to that number
     "LM"             Largest Magnitude  (the default)
     "SM"             Smallest Magnitude
     "LR"             Largest Real part
     "SR"             Smallest Real part
     "BE"             Both Ends.  Computes k/2 eigenvalues
                      from each end of the spectrum (one more
                      from the high end if k is odd.)
    where k is the length of the eigen_values vector.
    If eigen_vectors.length()==0 then only the eigen_values are computed.
    N.B. in comparison with other methods available in PLearn or
    elsewhere, this function is particularly useful when dealing
    with symmetric sparse matrices whose smallest eigen-pairs are sought.
*/
  void matlabR11eigs(RowMapSparseMatrix<double>& A, Mat eigen_vectors, 
                     Vec eigen_values, string which_eigenvalues="LM");


} // end of namespace PLearn

#endif
