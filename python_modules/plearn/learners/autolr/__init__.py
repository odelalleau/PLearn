from math import *

# YET TO BE DOCUMENTED AND CLEANED UP CODE

def deepcopy(plearnobject): # ugly way to copy until done properly with PLearn's deepcopy
    # actually not a deep-copy, only copy options
    if useserver:
        raise NotImplementedError
    else:
        return newObject(str(plearnobject))


def train_with_schedule(learner,
                        lr_options, # e.g. ["module.modules[0].cd_learning_rate" "module.modules[1].cd_learning_rate"]
                        learning_rates, # list of learning rates to try
                        stages,         # corresponding list of stages associated with each above learning rate
                        trainset,testsets,expdir,
                        cost_to_select_best=0,
                        selected_costnames = False):
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
    if interactive:
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
            print >>logfile, "At stage ",learner.stage," test" + str(j+1),": ",
            for k in range(0,n_costs):
                err = ts.getStat("E["+str(k)+"]")
                results[i,j*n_costs+k+1]=err
                costname = costnames[cost_indices[k]]
                print >>logfile, costname, "=", err,
                if k==cost_to_select_best and j==0 and err < best_err:
                    best_err = err
                    learner.save(expdir+"/"+"best_learner.psave","plearn_ascii")
                if interactive:
                    plot(results[0:i+1,0],results[0:i+1,
                         j*n_costs+k+1],colors[k%7]+styles[j%15],
                         label='test'+str(j+1)+':'+costname)
            print >>logfile
        if interactive and i==0:
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
                      lr_steps=exp(log(10)/2)):
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
    all_results = [zeros([n_epochs,2+n_tests*n_costs],Float32)]
    all_candidates = [learner]
    all_last_err = [best_err]
    all_slope = [0]
    all_lr = [initial_lr]
    all_start = [0]
    actives = [0]
    if interactive:
        clf()
        colors="bgrcmyk"
        styles=['-', '--', '-.', ':', '.', ',', 'o', '^', 'v', '<', '>', 's', '+', 'x', 'D']
    initial_stage=learner.stage
    for (s,i) in zip(range(epoch,nstages+epoch,epoch),range(n_epochs)):
        print >>logfile, "At stage ", initial_stage+s
        for active in actives:
            candidate = all_candidates[active]
            results = all_results[active]
            candidate.nstages = initial_stage+s
            for lr_option in lr_options:
                candidate.changeOptions({lr_option:str(all_lr[active])})
            candidate.setTrainingSet(trainset,False)
            candidate.train()
            results[i,0] = candidate.stage
            results[i,1] = all_lr[active]
            print >>logfile, "candidate ",active,":",
            for j in range(0,n_tests):
                ts = pl.VecStatsCollector()
                candidate.test(testsets[j],ts,0,0)
                print >>logfile, " test" + str(j+1),": ",
                for k in range(0,n_costs):
                    err = ts.getStat("E["+str(k)+"]")
                    results[i,j*n_costs+k+2]=err
                    costname = costnames[cost_indices[k]]
                    print >>logfile, costname, "=", err,
                    if k==cost_to_select_best and j==0:
                        if interactive:
                            start = all_start[active]
                            plot(results[start:i+1,0],
                                 results[start:i+1,j*n_costs+k+2],colors[active%7]+styles[j%15],
                                 label='candidate'+str(active)+':'+costname)
                            if active==0 and i==0:
                                legend()
                        if s>epoch:
                                all_slope[active]=0.5*all_slope[active]+0.5*(err-all_last_err[active])
                        all_last_err[active]=err
                        if err < best_err:
                            best_err = err
                            best_active = active
                            if save_best:
                                candidate.save(expdir+"/"+"best_learner.psave","plearn_binary")
            print >>logfile
            logfile.flush()
        if previous_best_err > best_err:
            previous_best_err = best_err
            print >>logfile,"BEST to now is candidate ",best_active," with err=",best_err
        if s%(epoch*nskip)==0 and s<nstages:
            best_active = argmin(all_last_err)
            # remove candidates that are worse and have higher slope
            best_slope = all_slope[best_active]
            best_last = all_last_err[best_active]
            ndeleted = 0
            for (a,i) in zip(actives,range(len(actives))):
                if a!=best_active and all_last_err[a]>best_last and all_slope[a]>=best_slope:
                    print >>logfile,"REMOVE candidate ",a
                    all_candidates[a]=None # hopefully this destroys the candidate
                    del actives[i-ndeleted]
                    ndeleted+=1
            # add a candidate with slightly lower learning rate than best_active, starting from it
            new_candidate = deepcopy(all_candidates[best_active])
            new_i = len(all_candidates)
            actives.append(new_i)
            all_candidates.append(new_candidate)
            all_results.append(all_results[best_active].copy())
            all_last_err.append(best_last)
            all_slope.append(best_slope)
            all_lr.append(all_lr[best_active]/lr_steps) # always try a smaller learning rate
            all_start.append(i)
            print >>logfile,"CREATE candidate ", new_i, " from ",best_active,"at epoch ",s," with lr=",all_lr[new_i]
            logfile.flush()
    if save_best:
        final_model = loadObject(expdir+"/"+"best_learner.psave")
    else:
        final_model = all_candidates[best_active]
    return (final_model,all_results[best_active],all_results,all_last_err,all_start)

