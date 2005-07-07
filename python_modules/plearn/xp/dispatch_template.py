#!/usr/bin/env python
from plearn.xp.Experiment import Experiment
from plearn.utilities.moresh import *

from apstat.taskutils.dispatch import Dispatch

COUNT = False

script = "{SCRIPT_NAME}.pyplearn"
expkey = [ "HYPERPARAM_1", "HYPERPARAM_2" ] 

#
#  The following functions are the different hyperparameters combinations that were tried
#
def grid_01( ):
    globals()['GRID'] = 01    
    return [ "VALUES...",           # HYPERPARAM_1
             "VALUES..."            # HYPERPARAM_2
             ]

#
#  Main
#   The GRID argument is provided only to tag the experiments. The
#    'GRID' argument could be part of the expkey, but this would result in
#    breaking the inexistance predicate used to avoid experiment
#    repetitions.
#
if __name__ == '__main__':
    grid     = grid_01()
    dispatch = Dispatch( program       = 'plearn',
                         script        = script,
                         constant_args = "SOME_FIX_PARAM=1 GRID=%d"%GRID,
                         count         = COUNT
                         )

    task_count = dispatch.start( expkey, *grid )
    if COUNT:
        print task_count
