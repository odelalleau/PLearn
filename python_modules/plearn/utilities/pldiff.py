#!/usr/bin/env python

# pldiff.py
# Copyright (C) 2006 Christian Dorion
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

import logging, os
import plearn.utilities.moresh  as moresh
import plearn.utilities.toolkit as toolkit

from plearn.math import floats_are_equal

_plearn_cmd = "%s --no-version --verbosity VLEVEL_IMP"
_plearn_exec = "plearn_tests"
def set_plearn_exec(plearn_exec):
    global _plearn_exec
    _plearn_exec = plearn_exec

def plearn_cmd(cmd=''):
    return _plearn_cmd%_plearn_exec + ' ' + cmd

def report_unique_paths(unique_paths, alterpath, prefix=''):
    report = []
    for path in unique_paths:
        if os.path.islink(path):
            continue

        elif os.path.isdir(path):
            if path.endswith('.metadata'):
                raise RuntimeError 
                
            if not moresh.is_recursively_empty(path):
                report.append(
                    "%s%s seems to be a significant directory but corresponds to no "
                    "directory under the %s tree.\n" % (prefix, path, alterpath) )

        elif os.path.isfile(path):
            report.append(
                "%s%s seems to be a significant file but corresponds to no "
                "file under the %s tree.\n" % (prefix, path, alterpath) )

    return report

__DUMMY__VALUE__ = "## __DUMMY__VALUE__ ##"
def pldiff(former, later, precision=1e-06,
           plearn_exec=__DUMMY__VALUE__, ignored_files_re=[]):
    """Compare to PLearn-compliant files or directory.

    TODO:
        1) .dmat directory should be managed intelligently
    """
    default_plearn_exec = _plearn_exec
    if plearn_exec != __DUMMY__VALUE__:
        set_plearn_exec(plearn_exec)
    
    report = []
    common_files = []

    # Compare two directories
    if os.path.isdir(former):
        assert os.path.isdir(later)        
        common_files, unique_to_former, unique_to_later =\
            moresh.compare_trees(former, later,
                ignored_files_re=ignored_files_re+["\.svn", ".\.metadata"])
    
        report.extend(report_unique_paths(unique_to_former, later, '--- '))
        report.extend(report_unique_paths(unique_to_later, former, '+++ '))

    # Arguments are only files
    else:
        assert not os.path.islink(former)
        assert not os.path.islink(later)
        common_files = [(former, later)]

    for former_file,later_file in common_files:
        diff_report = []

        if former_file.endswith('metainfos.txt'):
            continue # Skipping metainfos files

        elif former_file.endswith('.psave'):
            if plearn_exec is None:
                diff_report.append("Warning: %s encountered while plearn-exec is None\n"%former_file)
            else:
                diff_report.extend(psavediff(former_file, later_file, precision))

        elif toolkit.isvmat(former_file):
            if plearn_exec is None:
                diff_report.append("Warning: %s encountered while plearn-exec is None\n"%former_file)
            else:
                diff_report = vmatdiff(former_file, later_file, precision)
            
        else:
            diff = toldiff(former_file, later_file, precision)
            if diff:
                diff_report = ["--- %s and %s\n    %s"%(former_file, later_file, diff)]

        if diff_report and \
           not (len(diff_report)==1 and diff_report[0]==''):
            report.extend(diff_report)

    set_plearn_exec(default_plearn_exec)
    return report


def psavediff(former_file, later_file, precision=1e-06):
    """Special manipulation of psave files.
    
    The psave files are meant to change over time which additions of
    new options to the various class of the library. To avoid a diff to
    fail on the addition of a new option, the psave files are canonized
    through the read_and_write command prior to the diff call.
    """
    # Absolute path to the original files
    former_abspath = os.path.abspath(former_file)
    later_abspath = os.path.abspath(later_file)

    # Creating and changing to a temporary directory meant for comparison
    tmpdir = 'PSAVEDIFF/%s'%os.path.basename(later_file)
    if not os.path.exists(tmpdir):
        os.makedirs(tmpdir)
    moresh.pushd( tmpdir )
    logging.debug("--- pushd to %s"%os.getcwd())

    # Names for the files resulting from read_and_write
    former_rw = "rw_former_%s"%os.path.basename(former_file)
    later_rw = "rw_later_%s"%os.path.basename(later_file)

    # Creating the canonized files
    rw_cmd = plearn_cmd("read_and_write %s %s")
    os.system( rw_cmd%(former_abspath, former_rw) )
    assert os.path.exists(former_rw), "Error generating %s"%former_rw
    logging.debug(rw_cmd%(former_abspath, former_rw)+' succeeded.')

    os.system( rw_cmd%(later_abspath, later_rw) )
    assert os.path.exists(later_rw), "Error generating %s"%later_rw
    logging.debug(rw_cmd%(later_abspath, later_rw)+' succeeded.')

    ## Actual comparison
    report = []
    diff = toolkit.command_output(plearn_cmd("diff %s %s %s") \
                          % (former_rw, later_rw, precision))
    diff = "".join(diff)
    # diff = toldiff(former_rw, later_rw, precision)

    if diff:
        report = [ "--- %s and %s\n  Processed through read_and_write (%s)\n     %s"
                   % (former_file, later_file, tmpdir, diff) ]
    logging.debug('Report: %s'%report)

    ## Move back to original directory.
    moresh.popd( )
    logging.debug("--- popd to %s\n"%os.getcwd())
    return report

def toldiff(filename1, filename2, precision=1e-6, blanktol=0):
    """Returns an error message or an empty string if the files are tol-identical."""
    logging.debug("--- toldiff %s %s %g %d"%(filename1, filename2, precision, blanktol))
    f1 = open(filename1,'rb')
    f2 = open(filename2,'rb')

    blanks = ' \t\n\r'
    numerical = '+-0123456789.'
    numerical_e = numerical+'eE'

    c1 = f1.read(1) 
    c2 = f2.read(1)
    line1 = 1
    line2 = 1

    errmsg = lambda : 'Files differ before positions %d and %d at lines %d and %d\n'%(f1.tell(),f2.tell(), line1, line2)
    while c1 or c2:            

        if c1 == '\n':
            line1 += 1
        if c2 == '\n':
            line2 += 1

        if c1 and c1 in numerical:
            num1 = ''
            num2 = ''
            while c1 and c1 in numerical_e:
                num1 += c1
                c1 = f1.read(1)
            while c2 and c2 in numerical_e:
                num2 += c2
                c2 = f2.read(1)
            if num1!=num2:
                try:
                    if not floats_are_equal(float(num1),float(num2),precision):
                        return errmsg()
                except ValueError:
                    return errmsg()

        elif blanktol>0 and c1 in blanks:
            n1 = 0   # number of blanks in first file
            n2 = 0   # number of blanks in second file
            while c1 and c1 in blanks:
                n1 += 1
                c1 = f1.read(1)
            while c2 and c2 in blanks:
                n2 += 1
                c2 = f2.read(1)
            if abs(n1-n2)>blanktol:
                return errmsg()

        # Additional checks to avoid triggering a diff between UNIX and DOS
        # carriage returns.
        elif c1 == '\n' and c2 == '\r':
            c2 = f2.read(1)
            if c2 != '\n':
                return errmsg()
            else:
                line1 += 1
                line2 += 1
                c1 = f1.read(1)
                c2 = f2.read(1)
        elif c2 == '\n' and c1 == '\r':
            c1 = f1.read(1)
            if c1 != '\n':
                return errmsg()
            else:
                line1 += 1
                line2 += 1
                c1 = f1.read(1)
                c2 = f2.read(1)

        elif c1!=c2:
            return errmsg()
        
        else:
            c1 = f1.read(1)
            c2 = f2.read(1)

    # No errors encountered: empty error message.
    logging.debug("--- toldiff-equal %s %s\n"%(filename1, filename2))
    return ''

def vmatdiff(former_file, later_file, precision):
    logging.debug("--- vmatdiff %s %s %g"%(former_file, later_file, precision))
    diffcmd = plearn_cmd('vmat diff %s %s %g')%(former_file, later_file, precision)
    diffoutput = toolkit.command_output(diffcmd)

    if diffoutput:
        return [ diffcmd ] + [ (' '*4)+line for line in diffoutput ]
    return []
