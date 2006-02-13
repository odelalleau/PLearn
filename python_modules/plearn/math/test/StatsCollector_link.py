# A hack to use StatsCollector.py without needing a symbolic link.

import os

homedir = os.environ.get('HOME', '/')
plearndir = os.environ.get('PLEARNDIR', os.path.join(homedir,'PLearn'))

execfile(os.path.join(os.path.join(os.path.join(os.path.join(
        plearndir, 'python_modules'), 'plearn'), 'math'), 'StatsCollector.py'))

