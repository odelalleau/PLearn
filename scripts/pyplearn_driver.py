#!/usr/bin/python

#import plearn.pyplearn.funcs as pyplearn
from plearn.pyplearn import *

import sys
from plearn.pyplearn import _parse_plargs

pyplearn_file = open(sys.argv[1], 'U')
lines = pyplearn_file.read()
pyplearn_file.close()

if len(sys.argv) == 3 and sys.argv[2] == '--help':
    lines += 'print __doc__\n'
else:
    _parse_plargs(sys.argv[2:])
    lines += 'print main()\n'

del pyplearn_file, sys, _parse_plargs
exec ''.join(lines) in globals()
