#!/usr/bin/env python2.3
__cvs_id__ = "$Id: pyplearn_driver.py,v 1.9 2005/02/20 06:55:54 chapados Exp $"

from plearn.pyplearn import *

import sys
import os.path
from plearn.pyplearn.pyplearn import _parse_plargs, _postprocess_refs

### Add the absolute directory portion of the current script to the path
sys.path = [os.path.dirname(os.path.abspath(sys.argv[1]))] + sys.path

pyplearn_file = open(sys.argv[1], 'U')
lines = pyplearn_file.read()
pyplearn_file.close()

### Simply print the docstring of the pyplearn script
if len(sys.argv) == 3 and sys.argv[2] == '--help':
    lines += 'print __doc__\n'

### This is the mecanism used inside the run command to manage logs to
### expdirs.
elif len(sys.argv) >= 3 and '--PyPLearnScript' in sys.argv:
    sys.argv.remove('--PyPLearnScript')
    _parse_plargs(sys.argv[2:])
    lines += 'print PyPLearnScript( main() )\n'

### Default mode: simply dump the plearn_representation of this
else:
    _parse_plargs(sys.argv[2:])
    lines += 'print _postprocess_refs(str(main()))\n'

# del pyplearn_file, sys, _parse_plargs
exec ''.join(lines) in globals()
