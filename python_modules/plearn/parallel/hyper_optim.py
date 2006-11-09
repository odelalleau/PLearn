#!/usr/bin/env python

import time
import thread
from threading import Thread
import random
import os
import shutil
from dbi import *
UPDATE_DELAY = 5 # nr second delay

class Param:
    def __init__(self, name, type, **args):
       self.name = name
       self.type = type
       
       for key in args.keys():
           self.__dict__[key] = args[key]
        

class PerfMeasure:
    def __init__(self, valid_error, test_error = -1):
        self.valid_error = valid_error

    def __str__(self):
        return str(valid_error) + ',' + str(test_error)


def create_command(param_desc, param_values):
    command = './plearn dbn.pyplearn '
    for param_index in range(len(param_desc)):
        command = command + param_desc[param_index].name +'='+ str(param_values[param_index]) + ' '        

    return command

class Optimizer(Thread):
    def __init__(self, n_machines):
        self.results = {}
        self.pending = {}
        self.n_machines = n_machines
        self.jobs = []
        Thread.__init__(self) 

    def update_results(self):
        # check how many jobs have finished
        nr_jobs_finished = 0
        for job in self.jobs:
            job_done = 1
            for task in job.tasks:
                if task.get_status() == STATUS_FINISHED:
                    try: 
                        valid_error,test_error = get_results(task.unique_id)
                    except IOError:
                        continue

                    print 'valid_error', valid_error
                    prev_error = get_config_value('best_result.txt','valid_error')
                    print 'prev_error', prev_error
                    if float(prev_error) > float(valid_error) or prev_error == -1:
                        set_config_value('best_result.txt', 'valid_error', valid_error)
                        set_config_value('best_result.txt', 'test_error', test_error)
                        set_config_value('best_result.txt', 'param_values', task.param_values )

                    try: 
                        del self.pending[task.param_values] 
                    except KeyError:
                        pass
                        
                    self.results[task.param_values] = PerfMeasure(valid_error, test_error)
                    
                    
                    nr_jobs_finished = nr_jobs_finished + 1
                    
                    #delete temporary files
                    try:
                        pass
                        #                        shutil.rmtree('expdir_' + task.unique_id) 
                        #os.remove(task.unique_id + '.amat')
                        #os.remove(task.unique_id + '.vmat')
                    except IOError:
                        pass

                    #dump dictonary to file
                    #TODO
                    
                else:
                    job_done = 0
            if job_done:
                self.jobs.remove(job)
                    
        n_jobs_running = 0
        for job in self.jobs:
            for task in job.tasks:
                if task.get_status() == STATUS_RUNNING:
                    n_jobs_running = n_jobs_running + 1
        
        jobs_to_run = min(self.n_machines - nr_jobs_finished - n_jobs_running, len(self.pending.keys() ))                    
        print 'jobs_to_run', jobs_to_run
        commands = [] 
        param_values = []
        for key in self.pending.keys():
            #check if job is currently running
            in_queue = 0
            for job in self.jobs:
                for task in job.tasks:
                    if task.param_values == key:
                        in_queue = 1
                        break
                    
            if in_queue > 0:
                continue
            if jobs_to_run <= 0:
                break
            commands.append(create_command(self.param_desc, key))
            jobs_to_run = jobs_to_run - 1
            param_values.append(key)
            

        if len(commands) > 0:
            batch = DBICondor(commands,add_unique_id = 1)
            self.jobs.append(batch)
            # add the info about the unique_id
            for index, task in enumerate(batch.tasks):
#                task.command = task.command + 'unique_id=' + task.unique_id
                task.param_values = param_values[index] 
#		print task.command

            batch.run()

    def get_best_result(self):
        param_stat = PerfMeasure(100000)
        param_values = tuple()
        for key,value in self.results.items():
            if value.valid_error < param_stat.valid_error:
                param_stat, param_values = value, key

        return param_values, param_stat

class ListOptimizer(Optimizer):
    def __init__(self, n_machines):
        pass

    def run(self):
        time.sleep(UPDATE_DELAY)


class CrossProductOptimizer(Optimizer):
    def __init__(self, param_desc, n_machines, results_function):

        Optimizer.__init__(self, n_machines)
        
        self.already_run = 0
        self.param_desc = param_desc
        self.results_function = results_function
        

    def generate_cart_prod(self):
        def rloop(param_desc, comb):
            if param_desc:
                for item in param_desc[0].values:
                    newcomb = comb+[item]
                    for item in rloop(param_desc[1:], newcomb):
                        yield tuple(item)
            else:
                yield tuple(comb)

        return rloop(self.param_desc, [])
                        
    def run(self):
        self.not_finished = 1
        while 1:            
            if len(self.pending.keys()) == 0 and self.already_run > 0:
                break
            if self.already_run == 0:
                self.already_run = 1
                # add all combinations to pending list
                gen = self.generate_cart_prod()
                for new_comb in gen:
                    self.pending[new_comb] = PerfMeasure(100000)
                    
            
            self.update_results()
            time.sleep(UPDATE_DELAY)
    
class DiscreteRandomOptimizer(Optimizer):
    def __init__(self, param_desc, n_machines, results_function, **args):

        Optimizer.__init__(self, n_machines)
        
        self.already_run = 0
        self.param_desc = param_desc
        self.results_function = results_function

        for key in args.keys():
            self.__dict__[key] = args[key]
        

    def generate_random_combinations(self):
        for i in range(self.n_max_iter):
            comb = []
            for param in self.param_desc:
                x = random.randint(0,len(param.values)-1)
                comb.append(param.values[x])
            yield tuple(comb)
                        
    def run(self):
        self.not_finished = 1
        while 1:            
            if len(self.pending.keys()) == 0 and self.already_run > 0:
                break
            if self.already_run == 0:
                self.already_run = 1
                # add all combinations to pending list
                gen = self.generate_random_combinations()
                for new_comb in gen:
                    self.pending[new_comb] = PerfMeasure(100000)
                    
            
            self.update_results()
            time.sleep(UPDATE_DELAY)
    

def get_results(job_id):
    fi = open(job_id+'.amat','r')
    line = ''
    for index in range(4):
        line = fi.readline()

    values = line.split()
    return (float(values[9]), float(values[10]))

if __name__=="__main__":
    param_desc = []
#    param_desc.append(Param(name = 'weight_decay', type='real',values = [0.01,0.1,0.5]  ) )
    param_desc.append(Param(name = 'n_hidden', type='int',values = [10,20,40,80,160]  ) )
    param_desc.append(Param(name = 'n_epochs_grad', type='int',values = [4000,8000,16000,32000,64000,128000,250000]  ) )
    param_desc.append(Param(name = 'n_epochs_cd', type='int',values = [0,0,50,100,200,1000]  ) )
    param_desc.append(Param(name = 'grad_learning_rate', type='double',values = [0.05,0.001,0.1,0.01]  ) )
    param_desc.append(Param(name = 'cd_learning_rate', type='double',values = [0.0001,0.001,0.0001,0.000001]  ) )
    opt = DiscreteRandomOptimizer(param_desc, 20, get_results, n_max_iter = 5000)
    opt.start()
    opt.join()
    best_tuple, best_perf = opt.get_best_result()
    print 'best values ', best_tuple
    print 'valid error', best_perf.valid_error
#    thread.start_new_thread(myfunction,("Thread No:1",5))
#    time.sleep(100)
#    while 1:pass
