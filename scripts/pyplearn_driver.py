#!/usr/bin/python

from plearn.pyplearn import *

import sys
from plearn.pyplearn import _parse_plargs, _postprocess_refs

pyplearn_file = open(sys.argv[1], 'U')
lines = pyplearn_file.read()
pyplearn_file.close()

if len(sys.argv) == 3 and sys.argv[2] == '--help':
    lines += 'print __doc__\n'
else:
    _parse_plargs(sys.argv[2:])
    lines += 'print _postprocess_refs(str(main()))\n'

del pyplearn_file, sys, _parse_plargs
exec ''.join(lines) in globals()
