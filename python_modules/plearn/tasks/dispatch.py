#!/usr/bin/env python2.3

from threading                      import *
from Task                           import *
from plearn.utilities.verbosity     import *
from plearn.utilities.FrozenObject  import *

__all__ = ['Dispatch', 'DispatchDefaults']

class DispatchDefaults:
    cluster_command  = 'cluster --wait --duree 240h --execute'
    localhost        = False
    nb_hosts         = 10
    tasks            = {}

class Dispatch(FrozenObject):    
    def __init__(self, defaults=DispatchDefaults, **overrides):
        FrozenObject.__init__(self, defaults, overrides)        
        self._frozen = False
        self.shared_ressources_access  = Semaphore()
        self.one_task_is_finished      = Event()
        self.last_finished_task        = None
        self._frozen = True
        
    def add_task(self, task):
        if not isinstance(task, Task):
            raise TypeError("The add_task() procedure expects a Task (%s provided)."
                            % type(task))
        self.tasks[task.getName()] = task

    def acquire_shared_ressources(self):
        if not self.localhost:
            Dispatch.shared_ressources_access.acquire(True)
        
    def release_shared_ressources(self):
        if not self.localhost:
            Dispatch.shared_ressources_access.release()

    def run(self):        
        hostnum       = 0
        tasks_to_run  = self.tasks.values()
        nb_task       = len(tasks_to_run)
        launched_task = 0
        task_done     = 0
        current_task  = {}
        nb_hosts      = self.nb_hosts
        if self.localhost:
            nb_hosts  = 1
        else:
            vprint( "%d tasks to run on at most %d hosts."
                    % (nb_task,nb_hosts), 1)

        print_status = ( lambda :
                         vprint("-- %d tasks done.    %d to go.    %d running. --\n"
                                %(task_done,nb_task-task_done,len(current_task)), 1)
                         )

        while launched_task < nb_task:
            task = tasks_to_run[launched_task]
            task.signal_completion = self.task_completion
            
            if hostnum < nb_hosts:
                hostnum       += 1
                self.start_task( task, current_task )
                launched_task += 1
                print_status()
                
            else:
                # Processes are busy, wait for something to finish...
                self.wait_for_task_completion( current_task )
                task_done += 1
                
                # Launch a new task on the free host
                self.start_task( task, current_task )
                launched_task += 1

                print_status()
                ##self.shared_ressources_access.release()

        # Now wait until everybody is finished
        while current_task:
            # Processes are busy, wait for something to finish...
            self.wait_for_task_completion( current_task )
            task_done += 1

            print_status()
            ##self.shared_ressources_access.release()

    def start_task(self, task, current_task):
        task.start()
        current_task[task.getName()] = task
                   
    def task_completion(self, task):
        self.last_finished_task = task.getName()
        self.one_task_is_finished.set()

    def wait_for_task_completion(self, current_task):
        self.one_task_is_finished.wait()
        self.one_task_is_finished.clear()
        
        # One of the tasks just finished
        del current_task[ self.last_finished_task ]
        self.last_finished_task = None
