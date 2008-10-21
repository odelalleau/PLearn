
import plearn.bridgemode
from plearn.bridge import *

def test(learner,testset):
    ts = pl.VecStatsCollector()
    learner.test(testset,ts,0,0)
    return [ts.getStat("E["+str(i)+"]") for i in range(0,ts.length())]

# which libsvm are you using? 
# just set_libsvm_module('your.libsvm.module.name') before import plearn.learners.SVM
global _the_libsvm_module
_the_libsvm_module= 'libsvm'
def get_libsvm_module():
    global _the_libsvm_module
    return _the_libsvm_module
def set_libsvm_module(new_module):
    global _the_libsvm_module
    prev_module= _the_libsvm_module
    _the_libsvm_module= new_module
    return prev_module
