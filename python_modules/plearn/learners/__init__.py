
import plearn.bridgemode
from plearn.bridge import *

def test(learner,testset):
    ts = pl.VecStatsCollector()
    learner.test(testset,ts,0,0)
    return [ts.getStat("E["+str(i)+"]") for i in range(0,ts.length())]
