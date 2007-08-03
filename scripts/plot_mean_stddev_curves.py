#!/usr/bin/env python

# plot_mean_stddev_curves.py
# Copyright (C) 2007 Pascal Vincent, Pierre-Antoine Manzagol
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

# Authors: Pascal Vincent, Pierre-Antoine Manzagol

import sys
import glob
from numpy import *
from matplotlib.pylab import *
from plearn.vmat.smartReadMat import smartReadMat

def extract_x_y(matfilename, xfieldname, yfieldname, exclude_last_line=True):
    m, fieldnames = smartReadMat(matfilename)
    xpos = fieldnames.index(xfieldname)
    ypos = fieldnames.index(yfieldname)
    x = m[:,xpos]
    y = m[:,ypos]
    if exclude_last_line:
        x = x[0:-1]
        y = y[0:-1]
    return x,y

def plotcurves(xfieldname, yfieldname, map):
    """Takes a map of the form {'curvename1': [matfilename1, ..., matfilenamen ],
                                'curvename2': [ ... ] }
       As an alternative, the map may contain in place of list of mat files, a string with wildcard characters.
       opens each of the files containing curves (with same first coordinate field), computes the mean and stddevor,
       and plots resulting curves with error bars on a single graph.
       """
    for curvename, filenames in map.iteritems():
        if isinstance(filenames,str): # it's a wildcard specification
            filenames = glob.glob(filenames)
        xyvecs = [ extract_x_y(filename, xfieldname, yfieldname) for filename in filenames ]
        xvec = xyvecs[0][0]
        yvecs = [ y for x,y in xyvecs ]
        ymat = array(yvecs)
        meanvec = mean(ymat,0)
        stdvec = std(ymat,0)
        errorbar(xvec, meanvec, stdvec, label=curvename)
    xlabel(xfieldname)
    ylabel(yfieldname)
    legend()

def printHelpAndExit():
    print """Script to plot mean curves with std error bars computed from several matrix files.
    It takes three arguments: xfieldname yfieldname map
    xfieldname is the name of the x field in the matrix files
    yfieldname is the name of the y field
    map is a map indicating the matrixfiles to serve as population for each curve and its errorbars.
    Takes a map of the form {'curvename1': [matfilename1, ..., matfilenamen ],
                             'curvename2': [ ... ] }
    As an alternative, the map may contain in place of list of mat files, a string with wildcard characters.
    You should pass the map as a single argument, for ex. surrounding it with duble quotes
    Ex: plot_mean_stddev_curves.py "nstages" "E[train.E[class_error]]" "{'group1': '[1234].pmat', 'group2': '[5678].pmat' }"
    Note: current default behaviour excludes the last line of each loaded matrix (since plearn typically uses it for repeating best result.
    """
    sys.exit()

args = sys.argv[1:]
if len(args)!=3:
    printHelpAndExit()

xfieldname = args[0]
yfieldname = args[1]
map = eval(args[2])

plotcurves(xfieldname, yfieldname, map)
show()

