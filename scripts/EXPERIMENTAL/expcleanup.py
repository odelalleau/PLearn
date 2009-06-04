#!/usr/bin/env python

# expcleanup.py 
# Copyright (C) 2009 Pascal Vincent

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
# 
#  3. The name of the authors may not be used to endorse or promote
#     products derived from this software without specific prior written
#     permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
# NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# This file is part of the PLearn library. For more information on the PLearn
# library, go to the PLearn Web site at www.plearn.org


import sys
import os
import os.path
import shutil


def expcleanup(rootdir):

    # remove all no loner needed outmat?.pmat
    # This is commented out: it's problematic since for now, the outmats are not closed before all
    # pretraining is finiched. It is thus problematic to delete them while they're still open by the prg
    # and apparently causes a bug while running.
    #
    # os.system("""sh -c 'PATH=/bin:/usr/bin; find %s -name training_costs_layer_2.pmat -execdir rm -f outmat1.pmat \;' """ % rootdir)
    # os.system("""sh -c 'PATH=/bin:/usr/bin; find %s -name training_costs_layer_3.pmat -execdir rm -f outmat2.pmat \;' """ % rootdir)

    # search for finished experiments
    for dirpath, dirs, files in os.walk(rootdir):

        # Now try detecting signature of various experiment types to call appropriate cleanup operation

        # 1) newautoencoders experiment
        if 'tmpstats.txt' in files and 'global_stats.pmat' in files and 'Split0' in dirs \
            and os.path.exists(os.path.join(dirpath,'Split0/LearnerExpdir/Strat0/Trials0/Split0/LearnerExpdir/training_costs_layer_1.pmat')):
            cleanup_newautoencoders_exp(dirpath)

    print "All done."

def cleanup_newautoencoders_exp(dirpath):
    print
    print "************************************************"
    print "*** Cleaning up "+dirpath+"***"     

    # move useful stuff out of srcdir and into destdir

    srcdir = os.path.join(dirpath,'Split0/LearnerExpdir/Strat0/Trials0/Split0/LearnerExpdir')
    destdir = os.path.join(dirpath,'Split0/LearnerExpdir')

    useful_files = [
        "learner.psave",
        "training_costs_layer_1.pmat",
        "training_costs_layer_1.pmat.metadata",
        "training_costs_layer_2.pmat",
        "training_costs_layer_2.pmat.metadata",
        "training_costs_layer_3.pmat",
        "training_costs_layer_3.pmat.metadata"
        ]

    # move those files to destdir
    for filename in useful_files:
        srcfile = os.path.join(srcdir,filename)
        dest = os.path.join(destdir,filename)
        if os.path.exists(srcfile):
            print "* moving",srcfile,"->",dest
            shutil.move(srcfile, dest)
    
    # remove the Split0/LearnerExpdir/Strat0 subdirectory
    dirtoremove = os.path.join(dirpath,'Split0/LearnerExpdir/Strat0')
    print '* removing ',dirtoremove
    shutil.rmtree(dirtoremove, True)

    print



# main script:

if len(sys.argv)!=2:
    print """
Usage: expcleanup.py dirname

Will attempt to free some disk space in all experiments contained in dirname
removing unnecessary subdirectories (possibly after having relocated useful files)
    """
    sys.exit()

expcleanup(sys.argv[1])




