#!/usr/bin/env python

import sys
import os
import os.path

import matplotlib
# chose a non-GUI backend
matplotlib.use( 'Agg' )

from plearn.io.server import *
from plearn.plotting.netplot import *


################
### methods ###
################

def print_usage_and_exit():
    print "Usage : deepnet_collect_filters.py dirname"
    print "  will collect ad save .png files of filters found in any *learner*.psave in specified directory and subdirectories"
    sys.exit()

#print "Press Enter to continue"
#raw_input()

def guess_image_dimensions(n):
    imgheight = int(math.sqrt(n))+1
    while n%imgheight != 0:
        imgheight = imgheight-1
    imgwidth = n/imgheight
    return imgwidth,imgheight


def collectFilters(basedir):
    for dirpath, dirs, files in os.walk(basedir):
        for filename in files:
            if filename.endswith(".psave") and "learner" in filename:
                filepath = os.path.join(dirpath,filename)
                print 
                print "*** EXTRACTING FILTERS FROM "+filepath
                learner = serv.load(filepath)
                # matrices = learner.listParameter() 
                names = learner.listParameterNames()
                for varname in ["Layer1_W","Layer1_Wr"]:
                    if varname in names:
                        matrix = learner.getParameterValue(varname)
                        imgwidth,imgheight = guess_image_dimensions(matrix.shape[1])
                        imgfilename = filename[:-6]+"_"+varname+".png"
                        print "   --> "+imgfilename
                        saveRowsAsImage(os.path.join(dirpath,imgfilename), matrix,
                                        imgwidth, imgheight, 10, 20, figtitle=imgfilename)


############
### main ###
############

if len(sys.argv)<2:
    print_usage_and_exit()

server_command = "myplearn server"
serv = launch_plearn_server(command = server_command)

basedir = sys.argv[1]
collectFilters(basedir)
    
