__cvs_id__ = "$Id: ResourceManager.py,v 1.2 2004/12/21 15:31:50 dorionc Exp $"


import os, shelve, time

import plearn.utilities.glock   as glock
import plearn.utilities.toolkit as toolkit

LOCK_MINUTES = 10

class ResourceManager:

    def new_shelf_path(cls):
        path  = os.path.abspath( os.path.dirname(__file__) )
        fname = ( ".__%s__%s"
                  % ( cls.__name__, toolkit.date_time_random_string() )
                  )
        return os.path.join( path, fname )
        
    new_shelf_path = classmethod(new_shelf_path)

    def __init__(self, shelf_path=None):
        if shelf_path is None:
            self.shelf_path = self.new_shelf_path()            
        else:
            if os.path.exists( shelf_path ):
                self.shelf_path = shelf_path
            else:
                raise ValueError( "%s does not appear to be a valid shelf path."
                                  % shelf_path
                                  )
        self.lock        = None

    def acquire(self):
        self.lock  = glock.lock(self.shelf_path, wait=LOCK_MINUTES)

    def release(self):
        assert self.lock
        self.lock.unlock()
        self.lock        = None

    def get(self, resource_name):
        assert self.lock
        
##         ## Extend the lock
##         self.lock  = glock.lock(self.shelf_path, wait=LOCK_MINUTES)

        shelf    = shelve.open(self.shelf_path)
        resource = shelf[resource_name]
        shelf.close()

        return resource

    def set(self, resource_name, value):
        assert self.lock

##         ## Extend the lock
##         self.lock  = glock.lock(self.shelf_path, wait=LOCK_MINUTES)

        shelf    = shelve.open(self.shelf_path)
        shelf[resource_name] = value
        shelf.close()

    def close(self):
        assert os.path.exists( self.shelf_path )

        ## To ensure no other manager is currently accessing the resources
        if self.lock is None:
            self.acquire()
            
        os.remove(self.shelf_path)
        self.lock.unlock()
        self.lock = None

if __name__ == "__main__":
    print "Embedded Test/Tutorial"
    
    mng = ResourceManager()

    mng.acquire()

    mng.set("ints",    [1, 2, 3])
    mng.set("floats",  [1.0, 2.0, 3.0])
    mng.set("strings", ["1", "2", "3"])
    
    mng.release()

    
    
    from threading import *
    class getter(Thread):
        def __init__(self, typ):
            Thread.__init__(self)
            self.typ = typ
            self.done = Event()
            
        def __str__(self):            
            return "%s: %s" % (self.typ, self.data)

        def run(self):
            time.sleep(3)
            mng.acquire()
            time.sleep(5)

            self.data = mng.get(self.typ)
            mng.release()
            self.done.set()

    getters = []
    for typ in ["ints", "floats", "strings"]:
        g = getter(typ)
        g.start()
        getters.append( g )

    for g in getters:
        g.done.wait()

    for g in getters:
        print g
        
    mng.close()
