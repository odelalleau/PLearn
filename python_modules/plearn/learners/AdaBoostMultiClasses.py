from plearn.pyext import *
from plearn.pyplearn.plargs import *
import time

class AdaBoostMultiClasses:
    def __init__(self,trainSet1,trainSet2,weakLearner,confusion_target=1):
#        """
#        Initialize a AdaBoost for 3 classes learner
#        trainSet1 is used for the first sub AdaBoost learner,
#        trainSet2 is used for the second sub learner
#        weakLearner should be a function that take the class number for the one vs other
#                and should return a new weak learner
#        """
        self.trainSet1=trainSet1
        self.trainSet2=trainSet2

        if weakLearner:
            self.learner1 = self.myAdaBoostLearner(weakLearner(0),trainSet1)
            self.learner1.setExperimentDirectory(plargs.expdirr+"/learner1")
            self.learner1.setTrainingSet(trainSet1,True)
            
            self.learner2 = self.myAdaBoostLearner(weakLearner(2),trainSet2)
            self.learner2.setExperimentDirectory(plargs.expdirr+"/learner2")
            self.learner2.setTrainingSet(trainSet2,True)

        self.nstages = 0
        self.stage = 0
        self.train_time = 0
        self.confusion_target=confusion_target
        
    def myAdaBoostLearner(self,sublearner,trainSet):
        l = pl.AdaBoost()
        l.weak_learner_template=sublearner
        l.pseudo_loss_adaboost=plargs.pseudo_loss_adaboost
        l.weight_by_resampling=plargs.weight_by_resampling
        l.setTrainingSet(trainSet,True)
        l.early_stopping=False
        l.compute_training_error=True
        l.forward_sub_learner_test_costs=True
        l.provide_learner_expdir=True
#        l.save_often=True
        tmp=VecStatsCollector()
        tmp.setFieldNames(l.getTrainCostNames())
        l.setTrainStatsCollector(tmp)
        return l

    def train(self):
        t1=time.time()
        self.learner1.nstages = self.nstages
        self.learner1.train()
        self.learner2.nstages = self.nstages
        self.learner2.train()
        self.stage=self.learner1.stage
        t2=time.time()
        self.train_time+=t2-t1
        self.train_stats=VecStatsCollector()
        self.train_stats.append(self.learner1.getTrainStatsCollector(),
                                "sublearner1.",[])
        self.train_stats.append(self.learner2.getTrainStatsCollector(),
                                "sublearner2.",[])

        
    def getTestCostNames(self):
        costnames = ["class_error","linear_class_error","square_class_error"]
        for i in range(3):
            for j in range(3):
                costnames.append("conf_matrix_%d_%d"%(i,j))
        costnames.append("train_time")
        costnames.append("conflict")
        costnames.extend(["class0","class1","class2"])

        for c in self.learner1.getTestCostNames():
            costnames.append("subweaklearner1."+c)
        for c in self.learner2.getTestCostNames():
            costnames.append("subweaklearner2."+c)
        return costnames
    
    def getTrainCostNames(self):
        costnames = ["sublearner1."+x for x in self.learner1.getTrainCostNames()]
        costnames += ["sublearner2."+x for x in self.learner2.getTrainCostNames()]                                
        return costnames

    def computeOutput_at_stage(self,input,stage):
        """ compute the output for the input with the first stage weak learner
        
        return a tuple: (predicted result, output of sub learner1,output of sub learner2)
        """
        out1=self.learner1.computeOutput_at_stage(input,stage)[0]
        out2=self.learner2.computeOutput_at_stage(input,stage)[0]
        ind1=int(round(out1))
        ind2=int(round(out2))
        if ind1==ind2==0:
            ind=0
        elif ind1==1 and ind2==0:
            ind=1
        elif ind1==ind2==1:
            ind=2
        else:
            ind=self.confusion_target
        return (ind,out1,out2)

    def computeOutput(self,input):
        """ compute the output for the input
        
        return a tuple: (predicted result, output of sub learner1,output of sub learner2)
        """
        out1=self.learner1.computeOutput(input)[0]
        out2=self.learner2.computeOutput(input)[0]
        ind1=int(round(out1))
        ind2=int(round(out2))
        if ind1==ind2==0:
            ind=0
        elif ind1==1 and ind2==0:
            ind=1
        elif ind1==ind2==1:
            ind=2
        else:
            ind=self.confusion_target
        return (ind,out1,out2)
    
    def computeCostsFromOutput(self,input,output,target_,costs=[],forward_sub_learner_costs=True):
        target=int(target_)
        del costs[:]
        class_error=int(output[0] != target)
        linear_class_error=abs(output[0]-target)
        square_class_error=pow(abs(output[0]-target),2)
        costs.append(class_error)
        costs.append(linear_class_error)
        costs.append(square_class_error)
        for i in range(3):
            for j in range(3):
                costs.append(0)
        costs[output[0]*3+target+3]=1
        costs.append(self.train_time)

        #append conflict cost
        if int(round(output[1]))==0 and int(round(output[2]))==1:
            costs.append(1)
        else:
            costs.append(0)
        
        #append class output cost
        t=[0,0,0]
        t[output[0]]=1
        costs.extend(t)
        if target==0:
            t1=array([0.])
        else:
            t1=array([1.])
        if target==2:
            t2=array([1.])
        else:
            t2=array([0.])
        o1=array([output[1]])
        o2=[output[2]]
        if forward_sub_learner_costs:
            c1=self.learner1.computeCostsFromOutputs(input,o1,t1)
            c2=self.learner2.computeCostsFromOutputs(input,o2,t2)
            costs.extend(c1)
            costs.extend(c2)
        return costs

    def computeOutputAndCosts(self,input,target):
        output=self.computeOutput(input)
        costs=self.computeCostsFromOutput(input,output,target)
        return (output,costs)

    def test(self,testSet,testMat,test_stats,return_outputs,return_costs):
        testSet1=pl.ProcessingVMatrix(source=testSet,
                               prg = "[%0:%"+str(testSet.inputsize-1)+"] @CLASSE_REEL 1 0 ifelse :CLASSE_REEL")
        testSet2=pl.ProcessingVMatrix(source=testSet,
                               prg = "[%0:%"+str(testSet.inputsize-1)+"] @CLASSE_REEL 2 - 0 1 ifelse :CLASSE_REEL")

        stats1=pl.VecStatsCollector()
        stats2=pl.VecStatsCollector()
        (test_stats1, testoutputs1, testcosts1)=self.learner1.test(testSet1,
                                                                   stats1,
                                                                   True,True)
        (test_stats2, testoutputs2, testcosts2)=self.learner2.test(testSet2,
                                                                   stats2,
                                                                   True,True)
        outputs=[]
        costs=[]
        #calculate stats, outputs, costs
        for i in range(len(testMat)):
            out1=testoutputs1[i][0]
            out2=testoutputs2[i][0]
            ind1=int(round(out1))
            ind2=int(round(out2))
            if ind1==ind2==0:
                ind=0
            elif ind1==1 and ind2==0:
                ind=1
            elif ind1==ind2==1:
                ind=2
            else:
                ind=self.confusion_target
            output=[ind,out1,out2]
            if return_outputs:
                outputs.append(output)
            input=testMat[i][:-1]
            target=testMat[i][-1]
            cost=self.computeCostsFromOutput(input,output,target,
                                             forward_sub_learner_costs=False)
            cost.extend(testcosts1[i])
            cost.extend(testcosts2[i])
            test_stats.update(cost,1)
            if return_costs:
                costs.append(cost)
        return(test_stats,outputs,costs)
    
    def outputsize(self):
        return len(self.getTestCostNames())

    def save(self,path="",encoding="plearn_ascii"):
        if not os.path.exists(path):
            os.mkdir(path)
        if path:
            path+="/"
        else:
            print "WARNING: AdaBoost3PLearner - no path for saving the learner, we use the current directory"
        self.learner1.save(path+"learner1_stage="+str(self.stage)+".psave",encoding)
        self.learner2.save(path+"learner2_stage="+str(self.stage)+".psave",encoding)
    
    def load_old_learner(self,filepath=None,trainSet1=None,trainSet2=None,stage1=-1,stage2=-1):
        assert(trainSet1 and trainSet2)
        if not filepath:
            assert(not self.learner1.expdir.endswith("/learner1") or not self.learner2.expdir.endswith("/learner2"))
            path=self.learner1.expdir[:-9]
            assert(path==self.learner2.expdir[:-9])
            i=path.rfind("-2007-")
            (subdir,subfile)=os.path.split(path[:i])
            tmp=[x for x in os.listdir(subdir) if x.startswith(subpath)]
            assert(len(tmp)>0)
            filepath=max(tmp)
        #if stage=-1 we find the last one
        if stage1 == -1:
            s="learner1_stage#"
            lens=len(s)
            e=".psave"
            lene=len(e)
            tmp=[ x for x in os.listdir(filepath) if x.startswith(s) and x.endswith(".psave") ]
            for file in tmp:
                t=int(file[lens:-lene])
                if t>stage1: stage1=t
        #We must split stage1 and stage2 as one learner can early stop.
        if stage2 == -1:
            s="learner2_stage#"
            lens=len(s)
            e=".psave"
            lene=len(e)
            tmp=[ x for x in os.listdir(filepath) if x.startswith(s) and x.endswith(e) ]
            for x in tmp:
                t=int(x[lens:-lene])
                if t>stage2: stage2=t
                
        file1=filepath+"/learner1_stage#"+str(stage1)+".psave"
        file2=filepath+"/learner2_stage#"+str(stage2)+".psave"
        if (not os.path.exists(file1)) or (not os.path.exists(file2)):
            print "ERROR: no file to load in the gived directory"
            sys.exit(1)
        self.learner1=loadObject(file1)
        self.learner2=loadObject(file2)
        if not self.learner1.found_zero_error_weak_learner and not self.learner2.found_zero_error_weak_learner:
            assert(self.learner1.stage==self.learner2.stage)
        self.stage=self.learner1.stage
        self.nstages=self.learner1.nstages
        if trainSet1:
            self.learner1.setTrainingSet(trainSet1,False)
        if trainSet2:
            self.learner2.setTrainingSet(trainSet2,False)
        self.learner1.setTrainStatsCollector(VecStatsCollector())
        self.learner2.setTrainStatsCollector(VecStatsCollector())

