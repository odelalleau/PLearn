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

#include "MatlabInterface.h"
#include <plearn/io/TypesNumeriques.h> //!< For looksNumeric.
#include <plearn/io/TmpFilenames.h>

namespace PLearn {
using namespace std;


/*
MatlabInterface::MatlabInterface(string the_matlab_file, string the_id, 
 				 bool the_launch_in_background, bool the_erase_tmp_files)
{
  matlab_file_header = "";
  matlab_file = the_matlab_file;
  id = the_id;
  launch_in_background = the_launch_in_background;
  erase_tmp_files = the_erase_tmp_files;
}
*/

MatlabInterface::MatlabInterface(string the_matlab_file_header, string the_matlab_file, string the_id,
				 bool the_launch_in_background, bool the_erase_tmp_files)
{
  matlab_file_header = the_matlab_file_header;
  matlab_file = the_matlab_file;
  id = the_id;
  launch_in_background = the_launch_in_background;
  erase_tmp_files = the_erase_tmp_files;
}

MatlabInterface::MatlabInterface(vector<string> the_matlab_file_header, string the_matlab_file, string the_id,
				 bool the_launch_in_background, bool the_erase_tmp_files)
{
  for (unsigned int i = 0; i < the_matlab_file_header.size(); i++)
    matlab_file_header += (the_matlab_file_header[i] + "\n");
  id = the_id;
  matlab_file = the_matlab_file;
  launch_in_background = the_launch_in_background;
  erase_tmp_files = the_erase_tmp_files;
}

Popen* MatlabInterface::launch()
{
  if (id == "")
  {
    id = newFilename("", "");
    unlink(id.c_str());
  }
  string tmp_matlab_file = id + ".m";
  ofstream tmp_out(tmp_matlab_file.c_str());
  tmp_out << matlab_file_header << endl;
  if (matlab_file != "")
  {
    string cat_command = "cat " + matlab_file + " >> " + tmp_matlab_file;
    system(cat_command.c_str());
  }
  string matlab_command = "matlab -nodisplay -nojvm < " + tmp_matlab_file + "";
  if (launch_in_background)
    matlab_command += " &";
  // Launch Matlab
  matlab = new Popen(matlab_command);
  return matlab;  
}

bool MatlabInterface::launchAndWaitFor(string matlab_end_answer)
{
  if (id == "")
  {
    id = newFilename("", "");
    unlink(id.c_str());
  }
  string tmp_matlab_file = id + ".m";
  ofstream tmp_out(tmp_matlab_file.c_str());
  tmp_out << matlab_file_header << endl;
  if (matlab_file != "")
  {
    string cat_command = "cat " + matlab_file + " >> " + tmp_matlab_file;
    system(cat_command.c_str());
  }
  string matlab_command = "matlab -nodisplay -nojvm < " + tmp_matlab_file + "";
  if (launch_in_background)
    matlab_command += " &";
  // Launch Matlab
  matlab = new Popen(matlab_command);

  // Wait until Matlab outputs 'matlab_end_answer'
  string matlab_answer;
  do
  {
    matlab->in >> matlab_answer;
    if (0)
      cout << matlab_answer << endl;
  } while (!matlab->in.eof() && matlab_answer != matlab_end_answer 
           && matlab_answer.find("???",0)==string::npos);
  if (matlab_answer != matlab_end_answer)
    return false;
  if (erase_tmp_files)
    unlink(tmp_matlab_file.c_str());
  return true;
}

void matlabR11eigs(RowMapSparseMatrix<real>& A, Mat eigen_vectors, 
                   Vec eigen_values, string which_eigenvalues)
{
  bool get_evectors = eigen_vectors.length()>0;
  TmpFilenames tmpfilename(1);
  //string Afile = tmpfilename.addFilename("/tmp/","A",".ijv");

  //Temporary workaround!
  string Afile = tmpfilename.addFilename("/tmp/","A");
  unlink(Afile.c_str());
  Afile += ".ijv";

  A.exportToMatlabReadableFormat(Afile);
  string Afile_noext = remove_extension(Afile);
  string evec_file = Afile_noext + "_evec.dat";
  string eval_file = Afile_noext + "_eval.dat";
  string Aname = extract_filename(Afile_noext);
  vector<string> header;
  // that's where we expect to find eigs_r11.m
  header.push_back("path(path,'"+MatlabInterface::path()+"')");
  header.push_back("load " + Afile + ";");
  // convert to matlab sparse format
  header.push_back("M = spconvert(" + Aname + ");");
  header.push_back("options.disp=0; options.isreal=1; options.issym = 1;");
  header.push_back("d = " + tostring(eigen_values.length()) + ";");
  if (!looksNumeric(which_eigenvalues.c_str()))
    which_eigenvalues = "'" + which_eigenvalues + "'";
  // call eigs_r11
  if (get_evectors)
  {
    header.push_back("[Evec, Eval] = eigs_r11(M,d,"+which_eigenvalues+",options);");
    header.push_back("Evec = Evec';");
    header.push_back("save " + evec_file + " Evec -ascii -double;");
  }
  else
    header.push_back("Eval = eigs_r11(M, d, "+which_eigenvalues+", options);");
  header.push_back("Eval = diag(Eval);");
  header.push_back("save " + eval_file + " Eval -ascii -double;");
  header.push_back("fprintf(1, 'done ');");
  header.push_back("fprintf(1, '%d ',size(Eval));");
  header.push_back("quit");
  
  MatlabInterface matlab(header,"",Afile_noext,false,false);
  bool successful = matlab.launchAndWaitFor("done");
  if (!successful)
    PLERROR("matlabR11eigs: call to matlab was not successful, executing: %s",
          matlab.matlab_file_header.c_str());
  // get actual number of e-values found
  int actual_n_eval;
  string answer;
  // matlab.matlab->in >> actual_n_eval;
  matlab.matlab->in >> answer;
  if (answer==">>")
    matlab.matlab->in >> answer;
  if (!looksNumeric(answer.c_str()))
    PLERROR("matlabR11eigs: expected nb of eigenvalues, got %s",answer.c_str());
  actual_n_eval=toint(answer);
  if (actual_n_eval<eigen_values.length())
  {
    PLWARNING("matlabR11eigs: found %d e-values out of %d required",
            actual_n_eval,eigen_values.length());
    eigen_values.resize(actual_n_eval);
    if (get_evectors)
      eigen_vectors.resize(actual_n_eval,A.length());
  }
  // get results
  loadAsciiWithoutSize(eval_file, eigen_values);
  if (get_evectors)
    loadAsciiWithoutSize(evec_file, eigen_vectors);
  unlink(Afile.c_str());
  unlink(eval_file.c_str());
  unlink(evec_file.c_str());
}

void MatlabInterface::eigs_r11(RowMapSparseMatrix<real>& A, Mat& evectors, Vec& evalues, 
			       int d, string which_eigenvalues, bool erase_tmp_files)
{
  string id = newFilename("", "");
  unlink(id.c_str());
  string A_file = "A_" + id + ".ijv";
  A.exportToMatlabReadableFormat(A_file);
  string A_file_noext = remove_extension(A_file);
  string evec_file = A_file_noext + "_evec.dat";
  string eval_file = A_file_noext + "_eval.dat";
  vector<string> header;
  // that's where we expect to find eigs_r11.m
  header.push_back("path(path, '"+MatlabInterface::path()+"')");
  header.push_back("load " + A_file + ";");
  // convert to matlab sparse format
  header.push_back("M = spconvert(" + A_file_noext + ");");
  header.push_back("options.disp = 0; options.isreal = 1; options.issym = 1;");
  header.push_back("d = " + tostring(d) + ";");
  if (!looksNumeric(which_eigenvalues.c_str()))
    which_eigenvalues = "'" + which_eigenvalues + "'";
  header.push_back("[Evec, Eval] = eigs_r11(M, d, "+which_eigenvalues+", options);");
  header.push_back("Evec = Evec';");
  header.push_back("save " + evec_file + " Evec -ascii -double;");
  header.push_back("Eval = diag(Eval);");
  header.push_back("save " + eval_file + " Eval -ascii -double;");
  header.push_back("fprintf(1, 'done ');");
  header.push_back("fprintf(1, '%d ',size(Eval));");
  header.push_back("quit");
  
  MatlabInterface matlab(header, "", id, false, erase_tmp_files);
  bool successful = matlab.launchAndWaitFor("done");
  if (!successful)
    PLERROR("eigs_r11: call to matlab was not successful, executing: %s",
          matlab.matlab_file_header.c_str());
  // get actual number of e-values found
  int actual_n_eval;
  string answer;
  // matlab.matlab->in >> actual_n_eval;
  matlab.matlab->in >> answer;
  if (answer==">>")
    matlab.matlab->in >> answer;
  if (!looksNumeric(answer.c_str()))
    PLERROR("eigs_r11: expected nb of eigenvalues, got %s",answer.c_str());
  actual_n_eval=toint(answer);
  evalues.resize(actual_n_eval);
  evectors.resize(actual_n_eval, A.length());
  //evectors.resize(A.length(), actual_n_eval);
  if (actual_n_eval < d)
    PLWARNING("matlabR11eigs: found %d e-values out of %d required",
            actual_n_eval, d);
  // get results
  loadAsciiWithoutSize(eval_file, evalues);
  loadAsciiWithoutSize(evec_file, evectors);
  if (erase_tmp_files)
  {
    unlink(A_file.c_str());
    unlink(eval_file.c_str());
    unlink(evec_file.c_str());
  }
}

void MatlabInterface::eigs_r11(RowMapSparseMatrix<real>& A, Mat& evectors, 
			       int d, string which_eigenvalues, bool erase_tmp_files)
{
  string id = newFilename("", "");
  unlink(id.c_str());
  string A_file = "A_" + id + ".ijv";
  A.exportToMatlabReadableFormat(A_file);
  string A_file_noext = remove_extension(A_file);
  string evec_file = A_file_noext + "_evec.dat";
  vector<string> header;
  // that's where we expect to find eigs_r11.m
  header.push_back("path(path, '"+MatlabInterface::path()+"')");
  header.push_back("load " + A_file + ";");
  // convert to matlab sparse format
  header.push_back("M = spconvert(" + A_file_noext + ");");
  header.push_back("options.disp = 0; options.isreal = 1; options.issym = 1;");
  header.push_back("d = " + tostring(d) + ";");
  if (!looksNumeric(which_eigenvalues.c_str()))
    which_eigenvalues = "'" + which_eigenvalues + "'";
  header.push_back("[Evec, Eval] = eigs_r11(M, d, "+which_eigenvalues+", options);");
  //header.push_back("Evec = Evec';");
  header.push_back("save " + evec_file + " Evec -ascii -double;");
  header.push_back("fprintf(1, 'done ');");
  header.push_back("fprintf(1, '%d ', size(Eval));");
  header.push_back("quit");
  
  MatlabInterface matlab(header, "", id, false, erase_tmp_files);
  bool successful = matlab.launchAndWaitFor("done");
  if (!successful)
    PLERROR("eigs_r11: call to matlab was not successful, executing: %s",
          matlab.matlab_file_header.c_str());
  // get actual number of e-values found
  int actual_n_eval;
  string answer;
  // matlab.matlab->in >> actual_n_eval;
  matlab.matlab->in >> answer;
  if (answer==">>")
    matlab.matlab->in >> answer;
  if (!looksNumeric(answer.c_str()))
    PLERROR("eigs_r11: expected nb of eigenvalues, got %s",answer.c_str());
  actual_n_eval=toint(answer);
  evectors.resize(A.length(), actual_n_eval);
  //evectors.resize(actual_n_eval, A.length());
  if (actual_n_eval < d)
    PLWARNING("matlabR11eigs: found %d e-values out of %d required",
            actual_n_eval, d);
  // get results
  loadAsciiWithoutSize(evec_file, evectors);
  if (erase_tmp_files)
  {
    unlink(A_file.c_str());
    unlink(evec_file.c_str());
  }
}

void MatlabInterface::eigs_r11(RowMapSparseMatrix<real>& A, Vec& evalues, 
			       int d, string which_eigenvalues, bool erase_tmp_files)
{
  string id = newFilename("", "");
  unlink(id.c_str());
  string A_file = "A_" + id + ".ijv";
  A.exportToMatlabReadableFormat(A_file);
  string A_file_noext = remove_extension(A_file);
  string eval_file = A_file_noext + "_eval.dat";
  vector<string> header;
  // that's where we expect to find eigs_r11.m
  header.push_back("path(path, '"+MatlabInterface::path()+"')");
  header.push_back("load " + A_file + ";");
  // convert to matlab sparse format
  header.push_back("M = spconvert(" + A_file_noext + ");");
  header.push_back("options.disp = 0; options.isreal = 1; options.issym = 1;");
  header.push_back("d = " + tostring(d) + ";");
  if (!looksNumeric(which_eigenvalues.c_str()))
    which_eigenvalues = "'" + which_eigenvalues + "'";
  header.push_back("Eval = eigs_r11(M, d, "+which_eigenvalues+", options);");
  header.push_back("Eval = diag(Eval);");
  header.push_back("save " + eval_file + " Eval -ascii -double;");
  header.push_back("fprintf(1, 'done ');");
  header.push_back("fprintf(1, '%d ', size(Eval));");
  header.push_back("quit");
  
  MatlabInterface matlab(header, "", id, false, erase_tmp_files);
  bool successful = matlab.launchAndWaitFor("done");
  if (!successful)
    PLERROR("eigs_r11: call to matlab was not successful, executing: %s",
          matlab.matlab_file_header.c_str());
  // get actual number of e-values found
  int actual_n_eval;
  string answer;
  // matlab.matlab->in >> actual_n_eval;
  matlab.matlab->in >> answer;
  if (answer==">>")
    matlab.matlab->in >> answer;
  if (!looksNumeric(answer.c_str()))
    PLERROR("eigs_r11: expected nb of eigenvalues, got %s",answer.c_str());
  actual_n_eval=toint(answer);
  evalues.resize(actual_n_eval);
  if (actual_n_eval < d)
    PLWARNING("matlabR11eigs: found %d e-values out of %d required",
            actual_n_eval, d);
  // get results
  loadAsciiWithoutSize(eval_file, evalues);
  if (erase_tmp_files)
  {
    unlink(A_file.c_str());
    unlink(eval_file.c_str());
  }
}



} // end of namespace PLearn
