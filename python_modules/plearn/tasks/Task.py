import os, sys, shutil, string, traceback

from threading                      import *
from plearn.utilities.verbosity     import *
from plearn.utilities.FrozenObject  import *

__all__ = [ 'TaskStatus', 'Task', 'TaskDefaults', 'task_name_from_int' ]

def task_name_from_int(number):
    name = ''
    if number < 10:
        name = '000'
    elif number < 100:
        name = '00'
    elif number < 1000:
        name = '0'
    name += str( int(number) )
    return name

class TaskStatus:
    completion_types = ["Succeeded", "Failed", "Skipped"]
    status_types = ["New", "Ongoing"]+completion_types

    def __init__(self, status="New"):
        self.set_status(status)

    def set_status(self, status):
        if not status in self.status_types:
            raise ValueError(status)
        self.status = status

    def is_completed(self):
        return self.status in self.completion_types

    def __str__(self):
        return self.status

    def __repr__(self):
        return str(self)
    

class TaskDefaults:
    """Defaults field values for a task instance.

    Both must be None or both must be set...

    @cvar task_name: If it is let to None, the task_name will be
    generated for the Task.counter value at the task instanciation.

    @cvar require_shared_ressources: 
    """
    task_name                   = None
    require_shared_ressources   = None
    release_shared_ressources   = None
    completion_hook             = None
    status                      = TaskStatus()

class Task(Thread, FrozenObject):    
    """A specific type of thread.

    @cvar counter: Counting the tasks instanciated. If no name is
    provided to the constructor, it will be used to generate the
    task's name.
    """
    counter = 0
        
    def __init__( self, defaults = TaskDefaults, **overrides ):
        """Builds a task thread.

        @param defaults: A class whose class variables are the default
        values for this object's fields. The default value is L{TaskDefaults}.

        @type  defaults: Class

        Any keyword argument (other than defaults) will be set as a
        field of this created instance. B{Remember} that the task
        instance will be passed to the I{body_of_main} callable object.
        """
        ## This hack is necessary to allow the Thread constructor to
        ## set some members. It will be set back True by FrozenObject.
        self._frozen = False
        Thread.__init__(self)
        self.setDaemon(True)
        FrozenObject.__init__(self, defaults, overrides)

        Task.counter += 1
        self.set_attribute('task_id', Task.counter)
        if self.task_name is None:
            self.task_name = task_name_from_int(counter)
        self.setName( self.task_name )

        ## Both must be None or both must be set
        if self.require_shared_ressources is not None:
            assert self.release_shared_ressources is not None
        if self.release_shared_ressources is not None:
            assert self.require_shared_ressources is not None

    def body_of_task(self):
        raise NotImplementedError

    def preprocessing(self):
        pass

    def postprocessing(self):
        pass
    
    def run(self):        
        try:
            self.run_body()
        except:
            self.signal_completion()
            raise
        
    def run_body(self):
        self.preprocessing()
        if self.require_shared_ressources is not None:
            self.require_shared_ressources()
            self.shared_ressources_preprocessing()
            self.release_shared_ressources()

        self.body_of_task()
        if not self.status.is_completed():
            raise RuntimeError("Completion status must be signaled within the body_of_task().")
        
        self.postprocessing()
        if self.release_shared_ressources:
            self.require_shared_ressources()
            self.shared_ressources_postprocessing()
            self.release_shared_ressources()

        self.signal_completion()

    def set_completion_hook(self, hook):
        self.completion_hook = hook

    def set_status(self, status):
        self.status.set_status(status)
        
    def shared_ressources_preprocessing(self):
        raise NotImplementedError

    def shared_ressources_postprocessing(self):
        raise NotImplementedError

    def signal_completion(self):
        if self.completion_hook is not None:
            self.completion_hook( self )
    
