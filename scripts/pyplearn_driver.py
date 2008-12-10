#!/usr/bin/env python

import sys, os
from plearn.pyplearn import *
from plearn.pyplearn.plearn_repr import *
from plearn.utilities import options_dialog

if len(sys.argv)<=1:
    print "Wrong Usage: pyplearn_drvier.py must be invoked with a .pyplearn script as argument"
    sys.exit()

# Add the absolute directory portion of the current script to the path
sys.path = [os.path.dirname(os.path.abspath(sys.argv[1]))] + sys.path

pyplearn_file = open(sys.argv[1], 'U')
lines = pyplearn_file.read()
pyplearn_file.close()

orig_verb, orig_logs, gui_namespaces, use_gui = options_dialog.getGuiInfo(sys.argv)

if use_gui:
    lines += """
from plearn.utilities import options_dialog
runit, verb, logs= options_dialog.optionsDialog(%s,plargs.expdir,%d,%s,%s)
""" % (repr(sys.argv[1]), orig_verb, repr(orig_logs), repr(gui_namespaces))
else:
    lines += "\nrunit, verb, logs= True, %d, %s\n" % (orig_verb, repr(orig_logs))

if len(sys.argv) == 3 and sys.argv[2] == '--help':
    # Simply print the docstring of the pyplearn script
    lines += 'if __doc__ is not None: print __doc__\n'
    lines += 'print "### Possible command-line options with their default value: ###"\n'
    lines += 'print bindersHelp2()\n'
    lines += 'print\n'
    lines += 'print currentNamespacesHelp()\n'
    lines += 'print "Note: If you want to see the preprocessed output of this script in serialized .plearn format, you can invoke it with pyplearn_driver.py" \n'
elif len(sys.argv) >= 3 and '--PyPLearnScript' in sys.argv:
    # This is the mecanism used inside the run command to manage logs to
    # expdirs.
    sys.argv.remove('--PyPLearnScript')
    plargs.parse(sys.argv[2:])
    lines += 'if runit: print PyPLearnScript( main(), verbosity= verb, module_names= logs)\nelse: print PyPLearnScript("")\n'
else:
    # Default mode: simply dump the plearn_representation of this
    plargs.parse(sys.argv[2:])
    lines += 'if runit: print main()\nelse: print PyPLearnScript("")\n'


# Closing the current context so that, for instance, mispelled plargs can
# be identified.
lines += "import plearn; plearn.pyplearn.context.closeCurrentContext(); del plearn\n"

del globals()['sys']
del globals()['os']
exec ''.join(lines) in globals()
