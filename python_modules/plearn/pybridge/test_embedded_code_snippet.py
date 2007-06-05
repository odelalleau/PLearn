
from embedded_code_snippet import *
import gc, sys

class Bidule(EmbeddedCodeSnippet):
    def __init__(self, **kwargs):
        pass

class Machin(EmbeddedCodeSnippet):
    def __init__(self, **kwargs):
        pass

    def getN(self):
        sys.setrecursionlimit(2999)
        return 1999
    
    def test0(self):
        l= self.getLearner()
        l.setTrainingSet([[20,0,0,50,199],
                          [36,1,0,45,121],
                          [55,0,1,32,101]],
                         False)
        #ds= [[0,1,0,1,100],
        #     [8,0,1,3,200],
        #     [0,5,1,2,300],
        #     [1,1,1,-1,400]]
        #l.setTrainingSet(ds, False)
        l.inputsize= 4
        l.targetsize= 1
        l.weightsize= 0
        l.train()
        print 'use result=',l.use2([[25,1,1,44],
                                    [64,0,0,111]])
        sys.stdout.flush()
        l.save('linreg.psave','plearn_ascii')
        self.chkObj(l)
        l.nstages= self.getN()

    def chkObj(self, o):
        for opt in o._optionnames:
            x= o.__getattr__(opt)
            print opt, type(x), x
            sys.stdout.flush()
        print o
        sys.stdout.flush()
            
    def testRec(self, n):
        if n <= 0:
            print 'FINI!', gc.garbage
            sys.stdout.flush()
        else:
            if n%100 == 0:
                print "testRec",n
                sys.stdout.flush()
            self.recTest(n-1)

    def testRec2(self, l):
        if l.nstages <= 0:
            print 'FINI!2', gc.garbage
            sys.stdout.flush()
        else:
            if l.nstages%100 == 0:
                print "testRec2",l.nstages
                sys.stdout.flush()
            self.recTest2(l)

    def testRecCrash(self, n):
        if n <= 0:
            print 'FINI!', gc.garbage
            sys.stdout.flush()
            raise RuntimeError, 'Crash on purpose: test'
        else:
            print "testRecCrash",n
            sys.stdout.flush()
            self.recTestCrash(n-1)

    def fini(self):
        gc.collect()
        print "self", len(gc.get_referrers(self))
        print "learner", len(gc.get_referrers(self.getLearner()))
        print "testRec", len(gc.get_referrers(self.testRec))
        print "recTest", len(gc.get_referrers(self.recTest))
        print "1010", len(gc.get_referrers(1010))
        sys.stdout.flush()
        
    def getRecTestReferrers(self):
        gc.collect()
        return gc.get_referrers(self.recTest)

class Truc(Machin, Bidule):
    pass
    

pl_embedded_code_snippet_type= Truc
