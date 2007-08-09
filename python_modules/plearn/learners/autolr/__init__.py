from math import *
from numarray import *
from plearn import *
from plearn.bridge import *
from threading import *


# YET TO BE DOCUMENTED AND CLEANED UP CODE


if plearn.bridgemode.useserver:
    # Lock on the servers list so we don't have race conditions
    servers_lock = Lock()
    # List of [server, amount_of_jobs_server_is_running, lock_on_the_server]
    # lists
    servers = [[serv, 0, Lock()]]
    # Default maximal number of servers we are willing to run
    servers_max = 1e10

def execute(object, tasks, use_threads = False):
    def job(object):
        lock = [servinfo[2] for servinfo in servers if servinfo[0] is object.server][0]
        # we will only send our job to the server if nothing is already running
        lock.acquire()
        for method, args in tasks: # do each task sequentially
            if hasattr(object, method):
                getattr(object, method)(*args)
            else: # assume it is a function taking the args as arguments, not a method
                eval(method)(*args)
        lock.release()

    if plearn.bridgemode.useserver and use_threads:
        t = Thread(target = job, args = (object,))
        return t
    else:
        for method, args in tasks:
            if hasattr(object, method):
                getattr(object, method)(*args)
            else: # assume it is a function taking the object as first argument, not a method
                eval(method)(*args)
        return True

def acquire_server(use_threads = False):
    if not use_threads:
        return serv
    servers_lock.acquire()
    min_load = 1e10
    least_loaded = 0
    nservers = len(servers)
    for servinfo, i in zip(servers, xrange(nservers)):
        server, njobs, lock = servinfo
        if not njobs:
            servinfo[1] = 1
            servers_lock.release()
            return server
        if njobs < min_load:
            min_load = njobs
            least_loaded = i
    if nservers < servers_max:
        command = plearn.bridgemode.server_exe + ' server'
        server = launch_plearn_server(command = command)
        # give some time to the server to get born and well alive (taken from
        # bridge.py)
        time.sleep(0.5)
        lock = Lock()
        servers.append([server, 1, lock])
    else:
        servinfo = servers[least_loaded]
        server = servinfo[0]
        servinfo[1] += 1
    servers_lock.release()
    return server

def release_server(object, use_threads = False):
    if plearn.bridgemode.useserver and use_threads:
        server = object.server
        servers_lock.acquire()
        for servinfo in servers:
            server_, njobs, lock = servinfo
            if server_ is server: # == doesn't work
                # this server is done so we reduce its job count
                servinfo[1] -= 1
                break
        servers_lock.release()

def assign(object, use_threads = False):
    server = acquire_server(use_threads)
    if use_threads:
        lock = [servinfo[2] for servinfo in servers if servinfo[0] is server][0]
        lock.acquire() # plearn servers are not thread safe
    o = server.new(object)
    if use_threads:
        lock.release()
    return o

def getTestCostNames(learner):
    return learner.getTestCostNames()

def testlearner(learner,dataset,costs=[],ts=None):
    if not ts:
        ts = pl.VecStatsCollector()
        if plearn.bridgemode.useserver:
            ts=learner.server.new(ts)
    learner.test(dataset,ts,0,0)
    names = learner.getTestCostNames()
    del costs[:]
    for k in range(len(names)):
        costs.append(ts.getStat("E["+str(k)+"]"))
    return costs

def merge_schedules(schedules):
    """Merge several learning rate schedules into a kind of multi-schedule
    with one column per schedule but a unified sequence of stages.
    Each schedule is a Nx2 array, with the first column containing number of
    stages (examples) and the second column containing corresponding learning
    rates. N may vary across schedules. If we have a schedule first row with
    [10000 0.01] and a second row with [30000 0.001], it means that from the
    learner's current stage to stage 10000 (excluded) we should use a learning
    rate of 0.01, and from stage 10000 to 30000 (excluded) we should use a
    learning rate of 0.001. The different schedules do not have to have the
    same N nor the same maximum stage. They will be merged in one big schedule
    ranging from the mininum to the maximum of the stages found in all the
    schedules.
    The result is an array with the stages in the first column and one
    additional column (with a sequence of learning rates) for each input
    schedule.
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
                        trainset, testsets, expdir,
                        tester=None,
                        cost_to_select_best=0,
                        selected_costnames = None,
                        get_train_costs = True,
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
Optionally, a different learner can be supplied for training (the learner)
and testing (the tester).
"""
    if not tester:
        tester = learner
    train_costnames = learner.getTrainCostNames()
    test_costnames = getTestCostNames(tester)

    # Filter out unwanted costnames
    if selected_costnames is not None:
        train_costnames = [ name for name in train_costnames
                            if name in selected_costnames ]
        test_costnames = [ name for name in test_costnames
                           if name in selected_costnames ]

    n_train_costs = len(train_costnames)
    if not get_train_costs:
        n_train_costs = 0
    n_test_costs = len(test_costnames)

    learner.setTrainingSet(trainset,False)
    stages = schedules[:,0]
    learning_rates = schedules[:,1:]
    n_train = len(stages)
    n_schedules = len(lr_options)
    n_tests = len(testsets)
    results = zeros([n_train, 1 + n_schedules + n_train_costs + n_tests*n_test_costs], Float)
    best_err = 1e10
    initial_stage=learner.stage
    if plearn.bridgemode.interactive:
        clf()
    colors="bgrcmyk"
    styles=['-', '--', '-.', ':', '.', ',', 'o', '^', 'v', '<', '>', 's', '+', 'x', 'D']
    for i in range(n_train):
        learner.nstages = initial_stage + int(stages[i])
        options = {}
        for s in range(n_schedules):
            for lr_option in lr_options[s]:
                options[lr_option]=str(learning_rates[i,s])
        learner.changeOptions(options)
        learner.train()
        results[i,0] = learner.stage
        for s in range(n_schedules):
            results[i,1+s] = learning_rates[i][s]

        if get_train_costs:
            # Report approximate training error
            train_vsc = learner.getTrainStatsCollector()
            if train_vsc.fieldnames == []:
                # learner did not set train_stats fieldnames
                train_vsc.fieldnames = learner.getTrainCostNames()
        if get_train_costs:
            if logfile:
                print >>logfile, "At stage ", learner.stage, " train :",
            for k, costname in zip(range(n_train_costs), train_costnames):
                err = train_vsc.getStat('E['+costname+']')
                results[i, k+1+n_schedules] = err
                if logfile:
                    print >>logfile, costname, '=', err,
                if plearn.bridgemode.interactive:
                    plot(   results[0:i+1, 0],
                            results[0:i+1, 1+n_schedules+k],
                            colors[k%7]+styles[0],
                            label='train:'+costname)
            if logfile:
                print >>logfile
                logfile.flush()

        # Report error on test sets
        for j in range(n_tests):
            costs = testlearner(tester,testsets[j])
            if logfile:
                print >>logfile, "At stage ", learner.stage, " test" + str(j+1),": ",
            for k in range(0,n_test_costs):
                err = costs[k]
                results[i, 1+n_schedules+n_train_costs+(j*n_test_costs)+k] = err
                costname = test_costnames[k]
                if logfile:
                    print >>logfile, costname, "=", err,
                if k==cost_to_select_best and j==0 and err < best_err:
                    best_err = err
                    learner.save(expdir+"/"+"best_learner.psave","plearn_ascii")
                    if learner!=tester:
                        learner.save(expdir+"/"+"best_tester.psave","plearn_ascii")
                if plearn.bridgemode.interactive:
                    plot(results[0:i+1,0],
                            results[0:i+1, 1+n_schedules+n_train_costs+(j*n_test_costs)+k],
                            colors[k%7]+styles[(j+1)%15],
                            label='test'+str(j+1)+':'+costname)
            if logfile:
                print >>logfile
                logfile.flush()
        if plearn.bridgemode.interactive and i==0:
            legend()
    # Return headers for the result matrix, and the results themselves
    train_names = []
    if get_train_costs:
        train_names = ['train.' + costname for costname in train_costnames]
    return (['pstage', 'learning_rate']
            + train_names
            + ['test'+str(j+1)+'.'+costname for j in range(n_tests)
                                            for costname in test_costnames],
            results)


def choose_initial_lr(initial_learner,trainset,testset,lr_options,
                      nstages=10000,
                      cost_to_select_best=0,
                      initial_lr=0.01,
                      call_forget=True,
                      lr_steps=exp(log(10)/2),
                      logfile=None):
    """
Optimize initial learning rate by exploring greedily from a given initial
learning rate.
If call_forget then the provided initial_learner is changed (and not
necessarily the optimal one) upon return. But if not call_forget then the
initial_learner is unchanged (we make deep copies internally). However, if
note call_forget, then deep-copies are made and the best learner is returned
(3rd element of returned tuple).
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
        err=testlearner(learner,testset)[cost_to_select_best]
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
                      # List of lists of options (one list/group of options
                      # with a given schedule)
                      lr_options,
                      # Group of options that is actually optimized
                      # (if -1, then no optimization is performed)
                      optimized_group=0,
                      # Matrix of schedules
                      # (number of columns = 1 (stage) + number of groups)
                      schedules=None,
                      # Used to construct default schedule
                      # if 'schedules' is None
                      nstages=None,
                      epoch=None,   # ""
                      # Starting value for group being optimized
                      initial_lr=0.1,
                      # Number of epochs after which we add/remove candidates
                      nskip=2,
                      # Index of cost being optimized
                      cost_to_select_best=0,
                      # o/w return final model
                      return_best_model=False,
                      # for paranoids: save best model every save_best epochs,
                      # or not at all (if = False)
                      save_best=False,
                      selected_costnames = None,
                      min_epochs_to_delete = 2,
                      # Scaling coefficient when modifying learning rates
                      lr_steps=exp(log(10)/2),
                      # file (or sys.stdout) where to report progress
                      logfile=None,
                      # whether to get and report train costs
                      get_train_costs=True,
                      # do not try to go below this learning rate
                      min_lr=1e-6,
                      # Learning rate interval for heuristic
                      keep_lr=2,
                      use_threads=False):

    if plearn.bridgemode.useserver:
        servers[0][1] = 1 # learner

    train_costnames = learner.getTrainCostNames()
    if selected_costnames is not None:
        # Filter out unwanted costnames
        train_costnames = [ name for name in train_costnames
                            if name in selected_costnames ]
    n_train_costs = ifthenelse(get_train_costs,len(train_costnames),0)

    def error_curve(active,start_t,current_t):
        delta_t = current_t+1-start_t
        # in all_results, column 0 is stage, column 1 is option value
        # (learning rate) and column 2+test*n_costs+cost is the cost value for
        # cost number 'cost' (index in the selected_costnames list),
        # in testset 'test'.
        # And testset 0 is the one used for selection.
        return all_results[active][start_t:current_t+1,
                                   2+cost_to_select_best+n_train_costs]

    def error_curve_dominates(c1,c2,t):
        """curve1 has a lower last error than curve2, but will curve2
        eventually cross curve1? if yes return False o/w return True"""

        # check to see if c2 is alone with its learning rate (or nearby);
        # if yes keep it
        alone=True
        c2lr=all_lr[c2]
        c2err=all_last_err[c2]
        start_t = max(all_start[c1],all_start[c2])
        curve1 = error_curve(c1,start_t,t)#[valid_error]
        curve2 = error_curve(c2,start_t,t)
        if curve2.shape[0]-1<min_epochs_to_delete:
            return False
        for a in actives:
            if a!=c2 and all_lr[a]==c2lr and all_last_err[a]<=c2err:
                # throw it away if worse than other actives of same lr
                return True

            # say that it is alone if there are no other actives with nearby
            # and greater lr
            if a!=c2 and all_lr[a]>c2lr and abs(log(all_lr[a]/c2lr))<keep_lr*log(lr_steps):
                alone=False

        if curve1[-1]>=curve2[-1]:
            return False
        slope1=curve1[-1]-curve1[-2]
        slope2=curve2[-1]-curve2[-2]
        if  slope1 >= slope2 or all_last_err[c1] >= c2err:
            return False

        c1lr=all_lr[c1]
        if alone and c2lr>c1lr: # and slope2<0:
            # keep if alone and a larger learning rate and improving
            return False
        return True

    # although 1 is probably too small
    min_epochs_to_delete = max(1, min_epochs_to_delete)

    test_costnames = getTestCostNames(learner)
    n_tests = len(testsets)
    #n_costs = len(cost_indices)
    if selected_costnames is not None:
        test_costnames = [ name for name in test_costnames
                           if name in selected_costnames ]
    n_test_costs = len(test_costnames)

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
        # YB: QUI A MIS CA? POURQUOI???
        #if optimized_group != 0 and optimized_group != -1:
        #    print "Incorrect value for 'optimized_group'"
        #    raise Error
        if len(lr_options)!=1:
            lr_options=[lr_options[0]]

    n_train = len(stages)
    n_schedules = len(lr_options)
    assert n_schedules==learning_rates.shape[1]
    best_err = 1e10
    previous_best_err = best_err
    best_active = -1
    all_results = [1e10*ones([n_train,2+n_train_costs+n_tests*n_test_costs],Float)]
    all_candidates = [learner]
    all_last_err = [best_err]
    all_lr = [initial_lr]
    all_start = [0]
    actives = [0]
    best_candidate = learner
    best_early_stop = stages[0]
    initial_stage = learner.stage
    if plearn.bridgemode.interactive:
        clf()
        colors="bgrcmyk"
        styles=['-', '--', '-.', ':', '.', ',', 'o', '^', 'v', '<', '>', 's', '+', 'x', 'D']
    for t in range(n_train):
        stage=stages[t]
        if logfile:
            print >>logfile, "After ", stage, "stage"
        print "After",stage,"stages, actives now: ",actives, " with lr=", array(all_lr)[actives]
        print "current best actives:",best_active,"best_error:",all_last_err[best_active],"lr:",all_lr[best_active]
        print >>logfile, "actives now: ",actives, " with lr=", array(all_lr)[actives]

        threads = []
        active_stats = []
        for active in actives:
            candidate = all_candidates[active]
            results = all_results[active]
            candidate.nstages = initial_stage+int(stage)
            options = {}
            for s in range(n_schedules):
                for lr_option in lr_options[s]:
                    if s==optimized_group:
                        options[lr_option]=str(all_lr[active])
                    else:
                        options[lr_option]=str(learning_rates[t,s])
            candidate.changeOptions(options)
            candidate.setTrainingSet(trainset,False)
            if get_train_costs:
                train_vsc = pl.VecStatsCollector();
                if plearn.bridgemode.useserver:
                    train_vsc = candidate.server.new(train_vsc)
                candidate.setTrainStatsCollector(train_vsc)
            tasks = [('train', ())]
            stats = []
            for j in range(0,n_tests):
                stats.append([])
                tasks.append(('testlearner', (candidate,testsets[j],stats[j])))
            active_stats.append(stats)
            threads.append(execute(candidate, tasks, use_threads))

        # All threads must be started here. The servers are not thread-safe,
        # but the threads we start get a lock on their respective servers.
        for thread in threads:
            if isinstance(thread, Thread):
                thread.start()

        # Similarly, we must wait for all experiments to finish before doing
        # anything else.
        for thread in threads:
            if isinstance(thread, Thread):
                thread.join()

        for active, stats in zip(actives, active_stats):
            candidate = all_candidates[active]
            results = all_results[active]

            results[t,0] = candidate.stage
            results[t,1] = all_lr[active]
            if logfile:
                print >>logfile, "candidate ",active,":",
            if get_train_costs:
                # Report approximate training statistics
                if logfile:
                    print >>logfile, 'train :',
                ts=candidate.getTrainStatsCollector()
                for k, costname in zip(range(n_train_costs), train_costnames):
                    err = ts.getStat('E['+costname+']')
                    results[t, 2+k] = err
                    if logfile:
                        print >>logfile, costname, '=', err,

            # Report testing statistics
            for j, costs in zip(range(0,n_tests), stats):
                if logfile:
                    print >>logfile, " test" + str(j+1),": ",
                for k in range(0,n_test_costs):
                    err = costs[k]
                    costname = test_costnames[k]
                    results[t, 2+n_train_costs+(j*n_test_costs)+k] = err
                    if logfile:
                        print >>logfile, costname, "=", err,
                    if k==cost_to_select_best and j==0:
                        if plearn.bridgemode.interactive:
                            start = all_start[active]
                            if start==t:
                                plot(results[start:t+1,0],
                                     results[start:t+1, 2+n_train_costs+(j*n_test_costs)+k],
                                     colors[active%7]+styles[j%15],
                                     label='candidate'+str(active)+':'+costname)
                                legend()
                            else:
                                plot(results[start:t+1,0],
                                     results[start:t+1, 2+n_train_costs+(j*n_test_costs)+k],
                                     colors[active%7]+styles[j%15])

                        all_last_err[active]=err
                        if err < best_err:
                            best_err = err
                            best_active = active
                            best_early_stop = stage
                            if return_best_model:
                                best_candidate = deepcopy(candidate,
                                                          use_threads)
            if logfile:
                print >>logfile
                logfile.flush()
        if save_best and t%save_best==0:
            all_candidates[best_active].save(expdir+"/"+"best_learner.psave","plearn_binary")
        if previous_best_err >= best_err:
            previous_best_err = best_err
            if logfile:
                print >>logfile,"BEST to now is candidate ",best_active," with err=",best_err,"and lr=",all_lr[best_active]
                print >>logfile, "stage\tl.rate\t",
                if get_train_costs:
                    for costname in train_costnames:
                        print >>logfile, 'train.'+costname+"\t",
                    print >>logfile
                for costname in test_costnames:
                    print >>logfile, 'test.'+costname+'\t',
                print >>logfile
                for row in all_results[best_active][0:t+1,:]:
                    for val in row:
                        print >>logfile,val,"\t",
                    print >>logfile
                print >>logfile

        else:
            if logfile:
                print >>logfile, "THE BEST ACTIVE HAS GOTTEN WORSE!!!!"
        if learner.stage%(epoch*nskip)==0 and learner.stage<nstages:
            best_active = actives[argmin(array(all_last_err)[actives])]
            # remove candidates that are worse and have higher slope
            best_last = all_last_err[best_active]
            ndeleted = 0
            for (a,j) in zip(actives,range(len(actives))):
                if a!=best_active and error_curve_dominates(best_active,a,t):
                    if logfile:
                        print >>logfile,"REMOVE candidate ",a
                    release_server(all_candidates[a], use_threads)
                    # hopefully this destroys the candidate
                    all_candidates[a]=None
                    del actives[j-ndeleted]
                    ndeleted+=1
            # add a candidate with slightly lower learning rate than
            # best_active, starting from it
            # only try a smaller learning rate
            new_lr=all_lr[best_active]/lr_steps
            if new_lr>=min_lr:
                all_lr.append(new_lr)
                new_candidate = deepcopy(all_candidates[best_active],
                                         use_threads)
                new_a = len(all_candidates)
                actives.append(new_a)
                all_candidates.append(new_candidate)
                all_results.append(all_results[best_active].copy())
                all_last_err.append(best_last)
                all_start.append(t)
                if logfile:
                    print >>logfile,"CREATE candidate ", new_a, " from ",best_active,"at stage ",learner.stage," with lr=",all_lr[new_a]
                    logfile.flush()
    if return_best_model:
        final_model = best_candidate
    else:
        final_model = all_candidates[best_active]
    if optimized_group >= 0:
        schedules[:,1+optimized_group]=all_results[best_active][:,1]
    if logfile and best_err < all_last_err[best_active]:
        print >>logfile, "WARNING: best performing model would have stopped early at stage ",best_early_stop
    return (# Learner
            final_model,
            # Matrix of schedules (including the one that was optimized)
            schedules,
            # Error curve (matrix) for best model
            all_results[best_active],
            # List of all error curve matrices
            all_results,
            # List of all last errors recorded for each candidate
            # (not necessarily at last epoch)
            all_last_err,
            # Epoch index where each candidate was created (not necessarily
            # the first epoch)
            all_start,
            # Timestep (in stages) at which early-stopping should have happened
            # (i.e. stage of best error found)
            best_early_stop)

