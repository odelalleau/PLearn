# A hack to use StatsCollector.py without needing a symbolic link.

import os, sys
from plearn.utilities import toolkit

homedir = os.environ.get('HOME', '/')
plearndir = os.environ.get('PLEARNDIR', os.path.join(homedir,'PLearn'))

full_path = os.path.join(plearndir, 'python_modules', 'plearn', 'math', \
                         'StatsCollector.py')

if sys.platform == 'win32':
    full_path = toolkit.command_output("cygpath -w %s" % full_path)
    full_path = full_path[0].rstrip('\n')

execfile(full_path)

