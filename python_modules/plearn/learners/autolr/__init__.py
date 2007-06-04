from math import *
from numarray import *
from plearn.bridge import *

# YET TO BE DOCUMENTED AND CLEANED UP CODE


# ugly way to copy until done properly with PLearn's deepcopy
def deepcopy(plearnobject):
    # actually not a deep-copy, only copy options
    if plearn.bridgemode.useserver:
        o = serv.new(plearnobject.getObject())
    else:
        o = newObject(str(plearnobject))
    if o==None:
        print "deepcopy failed"
        raise NotImplementedError
    return o


def merge_schedules(schedules):
    """Merge several learning rate schedules into a kind of multi-schedule
    with one column per schedule but a unified sequence of stages.
    Each schedule is a Nx2 array, with the first column containing number of
    stages (examples) and the second column containing corresponding learning
    rates. N may vary across schedules. If we have a schedule first row with [10000 0.01]
    and a second row with [30000 0.001], it means that from the learner's current stage
    to stage 10000 (excluded) we should use a learning rate of 0.01, and from stage 10000
    to 30000 (excluded) we should use a learning rate of 0.001. The different
    schedules do not have to have the same N nor the same maximum stage. They
    will be merged in one big schedule ranging from the mininum to the maximum
    of the stages found in all the schedules.
    The result is an array with the stages in the first column and one
    additional column (with a sequence of learning rates) for each input schedule.
    """
    n_schedules = len(schedules)
    stages = []
    learning_rates = []
    row_indices = zeros([n_schedules],Int)
    lrates = array([schedule[0,1] for schedule in schedules],Float)
    schedule_lengths = array([schedule.shape[0] for schedule in schedules],Int)
    maxstage = max([max(schedule[:,0]) for schedule in schedules])
    while sum(row_indices<schedule_lengths)>0:
        changed = []
        stage = maxstage+1
        for i in range(n_schedules):
            if row_indices[i]<schedule_lengths[i]:
                s=schedules[i][row_indices[i],0]
                if s<stage:
                    changed=[i]
                    stage=s
                elif s==stage:
                    changed.append(i)
        for i in changed:
            lrates[i]=schedules[i][row_indices[i],1]
            row_indices[i]+=1
        stages.append(stage)
        learning_rates.append(lrates.copy())
    res=zeros([len(stages),1+n_schedules],Float)
    res[:,0]=stages
    res[:,1:]=learning_rates
    return res
    

def train_with_schedule(learner,
                        lr_options,
                        schedules,
                        trainset,testsets,expdir,
                        cost_to_select_best=0,
                        selected_costnames = False,
                        logfile=None):
    """Train a learner with one or more schedules of learning rates.
lr_options is a list of list of option strings. Each list in lr_options
is associated with a group of options (e.g. associated with a group
of modules within the learner) that has its own learning rates schedule.
Exemple of lr_options with only one schedule applied to two modules:
  [["module.modules[0].cd_learning_rate" "module.modules[1].cd_learning_rate"]]
The schedules argument is an array with the stages sequence in its first
column and sequences of learning rates (one sequence per group) in
each of the other columns (just like the result of the call to merge_schedules).
"""
    costnames = learner.getTestCostNames()
    if not selected_costnames:
        # use all cost names if not user-provided
        selected_costnames=costnames
    cost_indices = [costnames.index(name) for name in selected_costnames]
    learner.setTrainingSet(trainset,False)
    stages = schedules[:,0]
    learning_rates = schedules[:,1:]
    n_train = len(stages)
    n_schedules = len(lr_options)
    n_tests = len(testsets)
    n_costs = len(costnames)
    results = zeros([n_train,2+n_tests*n_costs],Float)
    best_err = 1e10
    if plearn.bridgemode.interactive:
        clf()
    colors="bgrcmyk"
    styles=['-', '--', '-.', ':', '.', ',', 'o', '^', 'v', '<', '>', 's', '+', 'x', 'D']
    for i in range(n_train-1):
        learner.nstages = int(stages[i])
        options = {}
        for s in range(n_schedules):
            for lr_option in lr_options[s]:
                options[lr_option]=str(learning_rates[i,s])
        learner.changeOptions(options)
        learner.train()
        results[i,0] = learner.stage
        for s in range(n_schedules):
            results[i,1+s] = learning_rates[i][s]
        for j in range(0,n_tests):
            ts = pl.VecStatsCollector()
            if plearn.bridgemode.useserver:
                ts=serv.new(ts)
            learner.test(testsets[j],ts,0,0)
            if logfile:
                print >>logfile, "At stage ",learner.stage," test" + str(j+1),": ",
            for k in range(0,n_costs):
                err = ts.getStat("E["+str(k)+"]")
                results[i,j*n_costs+k+1+n_schedules]=err
                costname = costnames[cost_indices[k]]
                if logfile:
                    print >>logfile, costname, "=", err,
                if k==cost_to_select_best and j==0 and err < best_err:
                    best_err = err
                    learner.save(expdir+"/"+"best_learner.psave","plearn_ascii")
                if plearn.bridgemode.interactive:
                    plot(results[0:i+1,0],results[0:i+1,
                         j*n_costs+k+1+n_schedules],colors[k%7]+styles[j%15],
                         label='test'+str(j+1)+':'+costname)
            if logfile:
                print >>logfile
                logfile.flush()
        if plearn.bridgemode.interactive and i==0:
            legend()
    return (['stage']+selected_costnames,results)


def choose_initial_lr(initial_learner,trainset,testset,lr_options,
                      nstages=10000,
                      cost_to_select_best=0,
                      initial_lr=0.01,
                      call_forget=True,
                      lr_steps=exp(log(10)/2),
                      logfile=None):
    """
Optimize initial learning rate by exploring greedily from a given initial learning rate
If call_forget then the provided initial_learner is changed (and not necessarily the
optimal one) upon return. But if not call_forget then the initial_learner is unchanged
(we make deep copies internally).
"""
    log_initial_lr=log(initial_lr)
    log_steps=log(lr_steps)
    global best_err
    global best_learner
    global initial_stage
    best_learner=initial_learner
    initial_stage=initial_learner.stage
    best_err=1e20

    def lr(i):
        return exp(log_initial_lr+i*log_steps)

    def perf(i):
        global best_err
        global best_learner
        
        if call_forget:
            learner = initial_learner
        else:
            learner = deepcopy(initial_learner)
        learner.setTrainingSet(trainset,call_forget)
        options = {}
        for lr_option in lr_options:
            options[lr_option]=str(lr(i))
        learner.changeOptions(options)
        learner.nstages = int(nstages+initial_stage)
        learner.train()
        ts = pl.VecStatsCollector()
        if plearn.bridgemode.useserver:
            ts=serv.new(ts)
        learner.test(testset,ts,0,0)
        err=ts.getStat("E["+str(cost_to_select_best)+"]")
        if logfile:
            print >>logfile, "*trying* initial learning rate ",lr(i), \
                  " and obtained err=",err," on cost ",cost_to_select_best
        if err<best_err:
            best_learner=learner
            best_err=err
        return err

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
    return (lr(best),perfs,best_learner)

def train_adapting_lr(learner,
                      trainset,testsets,expdir,
                      lr_options, # List of lists of options (one list/group of options with a given schedule)
                      optimized_group=0, # Group of options that is actually optimized
                      schedules=None, # Matrix of schedules (number of columns = 1 (stage) + number of groups)
                      nstages=None, # Used to construct default schedule if 'schedules' is None
                      epoch=None,   # ""
                      initial_lr=0.1, # Starting value for group being optimized
                      nskip=2,   # Number of epochs after which we add/remove candidates
                      cost_to_select_best=0, # Index of cost being optimized
                      return_best_model=False, # o/w return final model
                      save_best=False, # for paranoids: save best model each time an improvement is found
                      selected_costnames = False,
                      min_epochs_to_delete = 2,
                      lr_steps=exp(log(10)/2), # Scaling coefficient when modifying learning rates
                      logfile=False,
                      keep_lr=2): # Learning rate interval for heuristic

    min_epochs_to_delete = max(1,min_epochs_to_delete) # although 1 is probably too small

    def error_curve(active,start_t,current_t):
        delta_t = current_t+1-start_t
        return all_results[active][start_t:current_t+1,2+cost_to_select_best]
        
    def error_curve_dominates(c1,c2,t):
        """curve1 has a lower last error than curve2, but
           will curve2 eventually cross curve1? if yes return False o/w return True"""

        start_t = max(all_start[c1],all_start[c2])
        curve1 = error_curve(c1,start_t,t)
        curve2 = error_curve(c2,start_t,t)
        if curve1.shape[0]-1<min_epochs_to_delete or curve1[-1]>=curve2[-1]:
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
    
    costnames = learner.getTestCostNames()
    if not selected_costnames:
        # use all cost names if not user-provided
        selected_costnames=costnames
    cost_indices = [costnames.index(name) for name in selected_costnames]
    n_tests = len(testsets)
    n_costs = len(cost_indices)
    if schedules:
        stages = schedules[:,0]
        learning_rates = schedules[:,1:]
    else:
        if not nstages or not epoch:
            raise ValueError("if schedules is not specified, then nstages and epoch must be")
        stages = arange(learner.stage+epoch,learner.stage+nstages+1,epoch,Float)
        learning_rates = initial_lr*ones([len(stages),1],Float)
        schedules = zeros([len(stages),2],Float)
        schedules[:,0]=stages
        schedules[:,1]=learning_rates[:,0]
        optimized_group=0 # no choice, there is only one group
        if len(lr_options)!=1:
            lr_options=[lr_options[0]]
    n_train = len(stages)
    n_schedules = len(lr_options)
    assert n_schedules==learning_rates.shape[1]
    best_err = 1e10
    previous_best_err = best_err
    best_active = -1
    all_results = [1e10*ones([n_train,2+n_tests*n_costs],Float)]
    all_candidates = [learner]
    all_last_err = [best_err]
    all_lr = [initial_lr]
    all_start = [0]
    actives = [0]
    best_candidate = learner
    best_early_stop = stages[0]
    if plearn.bridgemode.interactive:
        clf()
        colors="bgrcmyk"
        styles=['-', '--', '-.', ':', '.', ',', 'o', '^', 'v', '<', '>', 's', '+', 'x', 'D']
    for t in range(n_train):
        stage=stages[t]
        if logfile:
            print >>logfile, "At stage ", stage
        print "actives now: ",actives, " with lr=", array(all_lr)[actives]
        print >>logfile, "actives now: ",actives, " with lr=", array(all_lr)[actives]
        for active in actives:
            candidate = all_candidates[active]
            results = all_results[active]
            candidate.nstages = int(stage)
            options = {}
            for s in range(n_schedules):
                for lr_option in lr_options[s]:
                    if s==optimized_group:
                        options[lr_option]=str(all_lr[active])
                    else:
                        options[lr_option]=str(learning_rates[t,s])
            candidate.changeOptions(options)
            candidate.setTrainingSet(trainset,False)
            candidate.train()
            results[t,0] = candidate.stage
            results[t,1] = all_lr[active]
            if logfile:
                print >>logfile, "candidate ",active,":",
            for j in range(0,n_tests):
                ts = pl.VecStatsCollector()
                if plearn.bridgemode.useserver:
                    ts=serv.new(ts)
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

                        all_last_err[active]=err
                        if err < best_err:
                            best_err = err
                            best_active = active
                            best_early_stop = stage
                            if return_best_model:
                                best_candidate = deepcopy(candidate)
                            if save_best: 
                                candidate.save(expdir+"/"+"best_learner.psave","plearn_binary")
            if logfile:
                print >>logfile
                logfile.flush()
        if previous_best_err > best_err:
            previous_best_err = best_err
            if logfile:
                print >>logfile,"BEST to now is candidate ",best_active," with err=",best_err
                print >>logfile, "stage\tl.rate\t",
                for cost_index in cost_indices:
                    costname = costnames[cost_index]
                    print >>logfile, costname+"\t",
                print >>logfile
                for row in all_results[best_active][0:t+1,:]:
                    for val in row:
                        print >>logfile,val,"\t",
                    print >>logfile
                print >>logfile
                
        else:
            if logfile:
                print >>logfile, "THE BEST ACTIVE HAS GOTTEN WORSE!!!!"
        if s%(epoch*nskip)==0 and s<nstages:
            best_active = actives[argmin(array(all_last_err)[actives])]
            # remove candidates that are worse and have higher slope
            best_last = all_last_err[best_active]
            ndeleted = 0
            for (a,j) in zip(actives,range(len(actives))):
                if a!=best_active and error_curve_dominates(best_active,a,t):
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
    if return_best_model:
        final_model = best_candidate
    else:
        final_model = all_candidates[best_active]
    schedules[:,1+optimized_group]=all_results[best_active][:,1]
    if logfile and best_err < all_last_err[best_active]:
        print >>logfile, "WARNING: best performing model would have stopped early at stage ",best_early_stop
    return (final_model,   # Learner
            schedules,     # Matrix of schedules (including the one that was optimized)
            all_results[best_active], # Error curve (matrix) for best model
            all_results, # List of all error curve matrices
            all_last_err, # List of all last errors recorded for each candidate (not necessarily at last epoch)
            all_start,  # Epoch index where each candidate was created (not necessarily the first epoch)
            best_early_stop) # Timestep (in stages) at which early-stopping should have happened (i.e. stage of best error found)

