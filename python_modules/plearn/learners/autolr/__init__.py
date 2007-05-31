from math import *
from numarray import *
from plearn.bridge import *

# YET TO BE DOCUMENTED AND CLEANED UP CODE


# ugly way to copy until done properly with PLearn's deepcopy
def deepcopy(plearnobject):
    # actually not a deep-copy, only copy options
    if plearn.bridgemode.useserver:
        raise NotImplementedError
    else:
        o = newObject(str(plearnobject))
        if o==None:
            print "deepcopy failed"
            raise NotImplementedError
        return o


def train_with_schedule(learner,
                        lr_options, # e.g. ["module.modules[0].cd_learning_rate" "module.modules[1].cd_learning_rate"]
                        learning_rates, # list of learning rates to try
                        stages,         # corresponding list of stages associated with each above learning rate
                        trainset,testsets,expdir,
                        cost_to_select_best=0,
                        selected_costnames = False,
                        logfile=False):
    costnames = learner.getTestCostNames()
    if not selected_costnames:
        # use all cost names if not user-provided
        selected_costnames=costnames
    cost_indices = [costnames.index(name) for name in selected_costnames]
    learner.setTrainingSet(trainset,False)
    n_train = len(learning_rates)
    n_tests = len(testsets)
    n_costs = len(costnames)
    results = zeros([n_train,1+n_tests*n_costs],Float32)
    best_err = 1e10
    if plearn.bridgemode.interactive:
        clf()
    colors="bgrcmyk"
    styles=['-', '--', '-.', ':', '.', ',', 'o', '^', 'v', '<', '>', 's', '+', 'x', 'D']
    for i in range(n_train):
        learner.nstages = stages[i]
        for lr_option in lr_options:
            learner.changeOptions({lr_option:str(learning_rates[i])})
        learner.train()
        results[i,0] = learner.stage
        for j in range(0,n_tests):
            ts = pl.VecStatsCollector()
            learner.test(testsets[j],ts,0,0)
            if logfile:
                print >>logfile, "At stage ",learner.stage," test" + str(j+1),": ",
            for k in range(0,n_costs):
                err = ts.getStat("E["+str(k)+"]")
                results[i,j*n_costs+k+1]=err
                costname = costnames[cost_indices[k]]
                if logfile:
                    print >>logfile, costname, "=", err,
                if k==cost_to_select_best and j==0 and err < best_err:
                    best_err = err
                    learner.save(expdir+"/"+"best_learner.psave","plearn_ascii")
                if plearn.bridgemode.interactive:
                    plot(results[0:i+1,0],results[0:i+1,
                         j*n_costs+k+1],colors[k%7]+styles[j%15],
                         label='test'+str(j+1)+':'+costname)
            if logfile:
                print >>logfile
        if plearn.bridgemode.interactive and i==0:
            legend()
    return (['stage']+selected_costnames,results)


def choose_initial_lr(learner,trainset,testset,lr_option,
                      nstages=10000,
                      cost_to_select_best=0,
                      initial_lr=0.01,
                      lr_steps=exp(log(10)/2)):

    log_initial_lr=log(initial_lr)
    log_steps=log(lr_steps)

    def lr(i):
        return exp(log_initial_lr+i*log_steps)

    def perf(i):
        learner.setTrainingSet(trainset,True) # call forget()
        learner.changeOptions({lr_option:str(lr(i))})
        learner.nstages = nstages
        learner.train()
        ts = pl.VecStatsCollector()
        learner.test(testset,ts,0,0)
        return ts.getStat("E["+str(cost_to_select_best)+"]")

    perfs = {}
    top = 1
    perfs[top]=perf(top)
    bottom = 0
    perfs[bottom]=perf(bottom)
    if perfs[bottom]<perfs[top]:
        best=bottom
    else:
        best=top
    
    while top-bottom<2 or (best==top or best==bottom):
        if best==top:
            top+=1
            perfs[top]=perf(top)
            if perfs[top]<perfs[best]:
                best=top
        if best==bottom:
            bottom-=1
            perfs[bottom]=perf(bottom)
            if perfs[bottom]<perfs[best]:
                best=bottom
    return (lr(best),perfs)

def train_adapting_lr(learner,
                      epoch,nstages,
                      trainset,testsets,expdir,
                      lr_options,
                      initial_lr=0.1,
                      nskip=2,
                      cost_to_select_best=0,
                      save_best=False,
                      selected_costnames = False,
                      min_epochs_to_delete = 2,
                      lr_steps=exp(log(10)/2),
                      logfile=False,
                      keep_lr=2):

    min_epochs_to_delete = max(1,min_epochs_to_delete) # although 1 is probably too small
        
    # used within the dominates function to predict future value of the error
    def dominates(c1,c2,current_t):
        """c1 has a lower last error than c2, but
           will c2 eventually cross c1? if yes return False o/w return True"""
        start_t = max(all_start[c1],all_start[c2])
        delta_t = current_t+1-start_t
        curve1 = all_results[c1][start_t:current_t+1,2+cost_to_select_best]
        curve2 = all_results[c2][start_t:current_t+1,2+cost_to_select_best]
        # wait to have at least 3 points in curve =2 epochs since split between the candidates
        if delta_t-1<min_epochs_to_delete or curve1[-1]>=curve2[-1]:
            return False
        slope1=curve1[-1]-curve1[-2]
        slope2=curve2[-1]-curve2[-2]
        if  slope1 >= slope2:
            return False
        #check to see if c2 is alone with its learning rate (or nearby); if yes keep it
        alone=True
        c2lr=all_lr[c2]
        c2err=all_last_err[c2]
        for a in actives:
            if all_lr[a]==c2lr and all_last_err[a]<c2err:
                return True # throw it away if worse than other actives of same lr
            if a!=c2 and abs(log(all_lr[a]/c2lr))<keep_lr*log(lr_steps):
                alone=False
        c1lr=all_lr[c1]
        if alone and c2lr>c1lr: # and slope2<0: 
            # keep if alone and a larger learning rate and improving
            return False
        return True
        #return all_last_err[j]>all_last_err[i] and all_slope[j]>=all_slope[i]
    
    costnames = learner.getTestCostNames()
    if not selected_costnames:
        # use all cost names if not user-provided
        selected_costnames=costnames
    cost_indices = [costnames.index(name) for name in selected_costnames]
    n_tests = len(testsets)
    n_costs = len(costnames)
    best_err = 1e10
    previous_best_err = best_err
    best_active = -1
    n_epochs=nstages/epoch
    all_results = [1e10*ones([n_epochs,2+n_tests*n_costs],Float32)]
    all_candidates = [learner]
    all_last_err = [best_err]
    #all_slope = [0]
    all_lr = [initial_lr]
    all_start = [0]
    actives = [0]
    if plearn.bridgemode.interactive:
        clf()
        colors="bgrcmyk"
        styles=['-', '--', '-.', ':', '.', ',', 'o', '^', 'v', '<', '>', 's', '+', 'x', 'D']
    initial_stage=learner.stage
    for (s,t) in zip(range(epoch,nstages+epoch,epoch),range(n_epochs)):
        if logfile:
            print >>logfile, "At stage ", initial_stage+s
        print "actives now: ",actives, " with lr=", array(all_lr)[actives]
        print >>logfile, "actives now: ",actives, " with lr=", array(all_lr)[actives]
        for active in actives:
            candidate = all_candidates[active]
            results = all_results[active]
            candidate.nstages = initial_stage+s
            for lr_option in lr_options:
                candidate.changeOptions({lr_option:str(all_lr[active])})
            candidate.setTrainingSet(trainset,False)
            candidate.train()
            results[t,0] = candidate.stage
            results[t,1] = all_lr[active]
            if logfile:
                print >>logfile, "candidate ",active,":",
            for j in range(0,n_tests):
                ts = pl.VecStatsCollector()
                candidate.test(testsets[j],ts,0,0)
                if logfile:
                    print >>logfile, " test" + str(j+1),": ",
                for k in range(0,n_costs):
                    err = ts.getStat("E["+str(k)+"]")
                    results[t,j*n_costs+k+2]=err
                    costname = costnames[cost_indices[k]]
                    if logfile:
                        print >>logfile, costname, "=", err,
                    if k==cost_to_select_best and j==0:
                        if plearn.bridgemode.interactive:
                            start = all_start[active]
                            if start==t:
                                plot(results[start:t+1,0],
                                     results[start:t+1,j*n_costs+k+2],colors[active%7]+styles[j%15],
                                     label='candidate'+str(active)+':'+costname)
                                legend() 
                            else:
                                plot(results[start:t+1,0],
                                     results[start:t+1,j*n_costs+k+2],colors[active%7]+styles[j%15])

                        #if s>epoch:
                        #        all_slope[active]=0.5*all_slope[active]+0.5*(err-all_last_err[active])
                        all_last_err[active]=err
                        if err < best_err:
                            best_err = err
                            best_active = active
                            if save_best:
                                candidate.save(expdir+"/"+"best_learner.psave","plearn_binary")
            if logfile:
                print >>logfile
                logfile.flush()
        if previous_best_err > best_err:
            previous_best_err = best_err
            if logfile:
                print >>logfile,"BEST to now is candidate ",best_active," with err=",best_err
        else:
            if logfile:
                print >>logfile, "THE BEST ACTIVE HAS GOTTEN WORSE!!!!"
        if s%(epoch*nskip)==0 and s<nstages:
            best_active = actives[argmin(array(all_last_err)[actives])]
            # remove candidates that are worse and have higher slope
            best_last = all_last_err[best_active]
            ndeleted = 0
            for (a,j) in zip(actives,range(len(actives))):
                if a!=best_active and dominates(best_active,a,t):
                    if logfile:
                        print >>logfile,"REMOVE candidate ",a
                    all_candidates[a]=None # hopefully this destroys the candidate
                    del actives[j-ndeleted]
                    ndeleted+=1
            # add a candidate with slightly lower learning rate than best_active, starting from it
            new_candidate = deepcopy(all_candidates[best_active])
            new_a = len(all_candidates)
            actives.append(new_a)
            all_candidates.append(new_candidate)
            all_results.append(all_results[best_active].copy())
            all_last_err.append(best_last)
            #all_slope.append(best_slope)
            all_lr.append(all_lr[best_active]/lr_steps) # always try a smaller learning rate
            all_start.append(t)
            if logfile:
                print >>logfile,"CREATE candidate ", new_a, " from ",best_active,"at epoch ",s," with lr=",all_lr[new_a]
                logfile.flush()
    if save_best:
        final_model = loadObject(expdir+"/"+"best_learner.psave")
    else:
        final_model = all_candidates[best_active]
    return (final_model,all_results[best_active],all_results,all_last_err,all_start)

