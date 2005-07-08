#!/usr/bin/env python
from plearn.xp.Experiment import Experiment
from plearn.utilities.moresh import *

# This script simply drives the tools from the dispatch module.
from apstat.taskutils.dispatch import Dispatch

# If True the dispatch.start method will only count the experiments to be ran.
COUNT = False

# The script that contains the experiment settings to be ran.
script = "expgrad.pyplearn"

#
# The following functions are the different hyperparameters combinations
# that were tried:
#
#   Note that, when provided to dispatch.start, the function's name is
#   split at the first underscore and the first part is used as a key while
#   the second part is used as the value for that key. Hence, experiments
#   are tagged with information regarding the function which launched
#   them. In the following example:
#
#     xp grid=01
#
#   would return all experiments generated using the grid_01() function.
#
def grid_01( ):
    return { "HYPERPARAM_1" : "VALUE_1",
             "HYPERPARAM_2" : [1, 2, 3],
             "HYPERPARAM_3" : -1000
             }
    
#
#  Main
#
if __name__ == '__main__':
    dispatch = Dispatch( program       = 'echo plearn', # First look at the output, then REMOVE echo!!!
                         script        = script,
                         constant_args = "model_name=MODEL_NAME",
                         count         = COUNT
                         )

    task_count = dispatch.start( grid_01 )
    if COUNT:
        print task_count
