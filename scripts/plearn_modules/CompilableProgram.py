#!/usr/bin/env python2.3

from toolkit import *
from threading import *

class CompilableProgram:
    """Status management."""

    FAILED       = -1
    NOT_COMPILED = 0
    LAUNCHED     = 1
    SUCCEEDED    = 2
    
    def __init__(self, global_prog_name):
        self.global_prog_name = global_prog_name
        self.sem       = Semaphore()
        self.__status    = self.NOT_COMPILED
        
    def compilation_failed(self):
        self.__status = self.FAILED
        #raw_input('RELEASE')
        self.sem.release()

    def compilation_succeeded(self):
        self.__status = self.SUCCEEDED
        #raw_input('RELEASE')
        self.sem.release()


    def launch_compilation(self):
        """This returns true if this thread actually was the one compiling the program.

        This method MUST be called prior to compiling a global program.
        """
        if self.__status != self.NOT_COMPILED:
            return False
        
        #raw_input('ACQUIRE')
        self.sem.acquire()
        if self.__status != self.NOT_COMPILED:
            raise RuntimeError("Bad threading management.")

        self.__status = self.LAUNCHED
        return True
        
    def status(self):
        #raw_input('ACQUIRE')
        self.sem.acquire()

        #raw_input('RELEASE')
        self.sem.release()
        return self.__status
