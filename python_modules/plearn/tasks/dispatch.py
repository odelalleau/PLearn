#!/usr/bin/env python2.3

from toolkit                    import *
from threading                  import *
from Task                       import Task           
from plearn.utilities.verbosity import vprint

###############################################################
### Management of KeyboadInterrupt within threads #############
### requires the following                         #############
__dispatch_end = Event()
def wait_for_dispatch_to_end():
    __dispatch_end.wait()

def dispatch_was_killed():
    return __dispatch_end.isSet()

def kill_dispatch():
    __dispatch_end.set()

###############################################################
class Dispatch(Thread, WithOptions):

    OPTIONS = { 'cluster_command':'cluster --wait --duree 240h --execute',
                'localhost':False,
                'nb_hosts':10      }
    
    def __init__(self, options=OPTIONS):
        Thread.__init__(self)
        WithOptions.__init__(self, options)
        
        self.__tasks = {}
            
        self.__shared_files_access = Semaphore()
        self.one_task_is_finished = Event()
        self.__last_finished_task = ''

    def add_task(self, task):
        if not isinstance(task, Task):
            raise TypeError("The add_task() procedure expects a Task (%s provided)."
                            % type(task))
        self.__tasks[task.getName()] = task
        #raw_input("\n\n %s" % str(self.__tasks))

    def acquire_shared_files_access(self, child_thread):
        if dispatch_was_killed():
            return False
        self.__shared_files_access.acquire(True)
        if dispatch_was_killed():
            return False
        return True
        
    def dispatch_command(self):
        if self._localhost:
            return ''
        return self._cluster_command

    def get_last_finished_task(self):
        return self.__last_finished_task

    def localhost_run(self):
        tasks_to_run = self.__tasks.values()
        for task in tasks_to_run:
            task.set_option('quote_cmd_line', False)

        nb_task = len(tasks_to_run)
        task_done = 0
        for task in tasks_to_run:
            task.start()
            
            # Wait for something to finish...
            self.one_task_is_finished.wait()
            self.one_task_is_finished.clear()

            task_done += 1
            vprint("*** %d tasks done.    %d to go.\n"%(task_done,nb_task-task_done), 1)
            self.release_shared_files_access(self)

    def release_shared_files_access(self, child_thread):
        self.__shared_files_access.release()
        return True

    def run(self):
        try:
            self.__run_body()
        except KeyboardInterrupt, ex:
            vprint("Dispatch killed by KeyboardInterrupt.", 0)
        kill_dispatch()

    def __run_body(self):        
        if self._localhost:
            self.localhost_run()
            return
        
        tasks_to_run = self.__tasks.values()

        hostnum = 0
        nb_hosts = self._nb_hosts
        nb_task = len(tasks_to_run)
        vprint("%d tasks to run on at most %d hosts." % (nb_task,nb_hosts), 1)
    
        launched_task = 0
        current_task = {}

        task_done = 0

        while launched_task < nb_task:
            task = tasks_to_run[launched_task]
                    
            if hostnum < nb_hosts:
                hostnum += 1

                task.start()
                current_task[task.getName()] = task
                launched_task += 1

                vprint("*** %d tasks done.    %d to go.    %d running."
                            %(task_done,nb_task-task_done,len(current_task)), 1)
                vprint("%d hosts used -- %s max.\n" % (hostnum,nb_hosts), 1)
                
            else:
                # Processes are busy, wait for something to finish...
                self.one_task_is_finished.wait()
                self.one_task_is_finished.clear()
            
                # One of the tasks just finished
                del current_task[ self.get_last_finished_task() ]            
                task_done += 1
            
                # Launch a new task on the free host
                task.start()
                current_task[task.getName()] = task
                launched_task += 1
            
                vprint("*** %d tasks done.    %d to go.    %d running.\n"
                            %(task_done,nb_task-task_done,len(current_task)), 1)
                self.shared_files_access.release()
            ##END if/else
        ##END while

        # Now wait until everybody is finished
        while current_task:
            # Processes are busy, wait for something to finish...
            self.one_task_is_finished.wait()
            self.one_task_is_finished.clear()

            # One of the tasks just finished
            del current_task[ self.get_last_finished_task() ]            
            task_done += 1

            vprint("*** %d tasks done.    %d to go.    %d running.\n"
                        %(task_done,nb_task-task_done,len(current_task)), 1)
            self.shared_files_access.release()
        ##END while
            
        vprint("+++ END OF TASKS.", 0)

    def set_last_finished_task(self, task_name):
        self.__last_finished_task = task_name
