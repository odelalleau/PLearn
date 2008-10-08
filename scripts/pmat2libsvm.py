#!/usr/bin/env python

import sys
from plearn.vmat.PMat import pmat2libsvm

def usage():
        sys.stderr.write('pmat2libsvm takes 2 argument: pmat_file_name and libsvm_file_name\n')
        sys.exit(1)

if len(sys.argv) != 3:
    usage()
    
pmat2libsvm(sys.argv[1], sys.argv[2])



