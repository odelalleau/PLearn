# server.py
# Copyright (C) 2005 Pascal Vincent
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org


# Author: Pascal Vincent

import os, socket, string, sys
from threading import Timer
from thread    import interrupt_main

from plearn.utilities.progress import LineOutputProgressBar

from plearn.pyplearn import *
import plearn.io.serialize

def launch_plearn_server(command = 'plearn server', logger=None):
    if logger: logger.info('LAUNCHING PLEARN SERVER: command = '+command)                
    ## If available, we use module "subprocess" instead of os.popen2 so that
    ## we can have information on the child pid
    try:
        from subprocess import Popen, PIPE
        p = Popen([command], shell=True, stdin=PIPE, stdout=PIPE, close_fds=True)
        (to_server, from_server, child_pid) = (p.stdin, p.stdout, p.pid)
    except:
        to_server, from_server = os.popen2(command, 'b')
        child_pid = -1
    return RemotePLearnServer(from_server, to_server, pid=child_pid, logger=logger)

def connect_to_plearn_server(hostname, port, logger=None):
    if logger: logger.info('CONNECTING TO PLEARN SERVER ON HOST '+hostname+', PORT '+str(port))                
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((hostname, port))
    io = s.makefile()
    return RemotePLearnServer(io, io, logger=logger)


def default_log_callback(module, vlevel, message):
    print ("SERVER-LOG: [" + module +"] "+ str(vlevel) + ' ' + message)
    
def default_pbar_callback(pbars, cmd, ptr, pos, title):
    if cmd=='A':
        pbars[ptr]= LineOutputProgressBar(title, pos)
    else:
        pbars[ptr].update(pos)
        if cmd == 'K':
            del(pbars[ptr])


        
class RemotePLearnServer:

    def help(self):
        print """
        You can create remote object with either the new or load methods:
         + new takes an object specification as either a string or pyplearn object.
         + load takes a lath to a file (.plearn, .pyplearn, .psave, .vmat, .pymat)
        You can then call (remote) methods on the returned stubs as you would call normal methods.
        
        Remote PLearn functions can be called as methods of the present server object.
        
        Remote functions that are part of the online help system:
        (Note: all ...name arguments must be strings)
        
        listFunctions()
        listFunctionPrototypes()
        helpOnFunction(functionname, arity)
        helpClasses()
        helpOnClass(classname)
        listMethods(classname)
        listMethodPrototypes(classname)
        helpOnMethod(classname, methodname, arity)
        listClassOptions(classname)
        helpOnOption(classname, optionname)
        helpClassParents(classname)
        """

    def __repr__(self):
        return "PLearnServer"

    def __str__(self):
        return "PLearnServer"

    def __init__(self, from_server, to_server, pid=-1, logger=None):
        """from_server and to_server are expected to be two file-like objects
        (supporting read, write, flush).
        If you wish to log debugging info, pass at logger an instance of a logging.Logger
        as returned for ex. by logging.getLogger()
        """
        self.io = plearn.io.serialize.PLearnIO(from_server, to_server)
        self.pid = pid
        self.log = logger
        self.reserved_ids = []
        self.nextid = 1
        self.objects = {}
        self.clear_maps = True
        self.dbg_dump = False
        self.closed = False
        self.pbars= {}

        ## Ensure that the server is responding. Otherwise raise an error
        if not self.isAlive():
            raise RuntimeError, \
                  "Cannot establish connection with PLearn Server: " \
                  "'ping' request did not respond"
        
        if self.log:
            self.log.info('CONNEXION ESTABLISHED WITH PLEARN SERVER')

        self.callFunction("binary")
        self.callFunction("implicit_storage",True)        

    def set_return_lists(self, ret_lists=True):
        """If this is called with True, PLearn sequences will be
        returned as lists (rather than as arrays, which is the default
        behaviour, and can be restored by calling this with False)"""
        self.io.return_lists = ret_lists
        
    def reserve_new_id(self):
        """Returns an available object id and adds it to self.reserved_ids"""
        startid = 10000
        newid = startid
        if self.reserved_ids:
            first_id = self.reserved_ids[0]
            last_id  = self.reserved_ids[-1]
            if first_id>startid:
                newid = first_id-1
                self.reserved_ids.insert(0,newid)
            elif last_id-first_id+1 == len(self.reserved_ids):
                newid = last_id+1
                self.reserved_ids.append(newid)
            else:
                prev_id = first_id
                for pos in xrange(1,len(self.reserved_ids)):
                    id = self.reserved_ids[pos]
                    if id-prev_id>1:
                        newid = id-1
                        self.reserved_ids.insert(pos,newid)
                        break
                    prev_id = id
        else:
            self.reserved_ids.append(newid)
        return newid
                
    def free_id(self,id):
        """Removes an object id from the list self.reserved_ids"""
        self.reserved_ids.remove(id)


    def new(self, objectspec):
        """
        objectspec is the specification of a plearn object.
        It can be either a string in the plearn serialization format,
        or a PyPLearnObject such as those built by calling pl.ClassName(...)
        """
        specstr = objectspec
        if type(specstr)!=str:
            specstr = specstr.plearn_repr()
            
        objid = self.reserve_new_id()        
        self.callNewObject(objid,specstr)
        obj = RemotePObject(self, objid)
        self.objects[objid] = obj
        return obj

    def load(self, objfilepath):
        """This will load the object from the specified file
        into the server, and return a remote handler for it.
        File may be a .plearn, .pyplearn, .psave, .vmat, .pymat
        """
        objid = self.reserve_new_id()
        self.callLoadObject(objid, objfilepath)
        obj = RemotePObject(self, objid)
        self.objects[objid] = obj
        return obj

    def delete(self, obj):
        if type(obj) is int:
            objid = obj
        else:
            objid = obj.objid
        self.callDeleteObject(objid)
        del self.objects[objid]
        self.free_id(objid)

    def deleteAll(self):
        self.callDeleteAllObjects()
        del self.objects
        self.objects = {}
        self.reserved_ids = []

    def logged_write(self,msg):
        """Writes msg to self.io and possibly writes a corresponding entry in the logfile"""
        self.io.write(msg)
        self.io.flush()
        if self.log: self.log.debug('SENDING: '+msg)
        
    def logged_write_args(self,args):
        argnum = 0
        for arg in args:
            self.io.write_typed(arg)
            if self.log: self.log.debug(' ARG_'+str(argnum)+': '+repr(arg))
            argnum += 1
        self.io.write('\n')
        self.io.flush()

    def callNewObject(self, objid, objspecstr):
        self.clearMaps()
        self.logged_write('!N '+str(objid)+' '+objspecstr+'\n')
        self.expectResults(0)

    def callLoadObject(self, objid, filepath):
        self.clearMaps()
        self.logged_write('!L '+str(objid)+' "'+filepath+'"\n')
        self.expectResults(0)

    def callDeleteObject(self, objid):
        self.logged_write('!D '+str(objid)+'\n')
        self.expectResults(0)

    def callDeleteAllObjects(self):
        self.logged_write('!Z \n')
        self.expectResults(0)

    def isAlive(self, timeout=2):
        """Check if the connection is still up by doing a ping.

        The optional 'timeout' argument specifies how long we should wait
        (in seconds; may be a fraction) for an answer before giving up.
        """
        ## The implementation of this function is complicated by the fact
        ## that we cannot expect the pipes to really work at all; in this
        ## case, expectResults() might get stuck in an infinite loop.  To
        ## this end, we start a timer which interrupts the main thread if
        ## we wait too long.  (It is not possible to kill a thread in
        ## Python, so the alternative of busy waiting in a second thread
        ## must be ruled out).

        t = Timer(timeout, interrupt_main)
        
        try:
            t.start()
            self.logged_write('!P \n')
            self.expectResults(0)   # Might die in infinite loop...
            t.cancel()
            return True
        except:
            t.cancel()
            return False

    def clearMaps(self):
        if self.clear_maps:
            self.io.clear_maps()

        # re-register all objects from self.objects:
        pref_map = PRefMap.getCurrentPRefMap()
        for obj in self.objects.values():
            pref_map.registerPLearnRef(obj._serial_number())

    def sendFunctionCallHeader(self, funcname, nargs):
        self.clearMaps()
        self.logged_write('!F '+funcname+' '+str(nargs)+' ')

    def sendMethodCallHeader(self, objid, methodname, nargs):
        self.clearMaps()
        self.logged_write('!M '+str(objid)+' '+methodname+' '+str(nargs)+' ')



    def waitForResult(self, log_callback= default_log_callback, pbar_callback= default_pbar_callback):
        while True:
            self.io.skip_blanks_and_comments()
            c = self.io.peek()
            if c=='*': #log or progress message
                c = self.io.get()#get '*'
                c = self.io.get()
                if c == 'L': # Log message
                    module= self.io.read_string()
                    vlevel= self.io.read_int()
                    message= self.io.read_string()
                    log_callback(module, vlevel, message)
                elif c == 'P': # Progress message
                    command= self.io.get()
                    ptr= self.io.read_int()
                    pos= 0
                    title= ''
                    if command != 'K':
                        pos= self.io.read_int()
                    if command == 'A':
                        title= self.io.read_string()
                        title= '(pb#'+str(ptr)+') '+title
                    pbar_callback(self.pbars, command, ptr, pos, title)
                else:
                    raise TypeError("Expected *L or *P, but read *"+c)
            else:
                return
        

    def getResultsCount(self):
        self.waitForResult()
        self.io.skip_blanks_and_comments()
        c = self.io.get()
        if c!='!':
            print c
            while c!='':
                print self.io.get()
            raise TypeError("Returns received from server are expected to start with a ! but read "+c)
        c = self.io.get()
        if c=='R':
            nreturned = self.io.read_int()
            if self.log: self.log.debug("RECEIVED RESULT COUNT: !"+c+' '+str(nreturned))
            return nreturned
        elif c=='E':
            msg = self.io.read_string()
            if self.log: self.log.error("RECEIVED ERROR: "+msg)                
            raise RuntimeError(msg)
        else:
            raise TypeError("Expected !R or !E but read !"+c)

    def expectResults(self, nresults_expected):
        # if self.log: self.log.debug("EXPECTING "+str(nresults_expected)+" RESULTS")
        nreturned = self.getResultsCount()
        if nreturned!=nresults_expected:
            raise TypeError("Expected "+str(nresults_expected)+" return arguments, but read R "+str(nreturned))

    def callFunction(self, functionname, *args):
        self.sendFunctionCallHeader(functionname, len(args))
        self.logged_write_args(args)
        nresults = self.getResultsCount()
        results = []
        for i in xrange(nresults):
            res = self.io.binread()
            if self.log: self.log.debug(' RES_'+str(i)+': '+repr(res))
            results.append(res)
        if len(results)==1:
            return results[0]
        elif len(results)>1:
            return results

    def __getattr__(self,functionname):
        def f(*args):
            return self.callFunction(functionname,*args)
        return f

    def callMethod(self, objid, methodname, *args):
        self.sendMethodCallHeader(objid, methodname, len(args))
        #print 'sending ARGS', args
        self.logged_write_args(args)

        if self.dbg_dump:
            print 'DEBUG DUMP AFTER CALL OF METHOD',methodname,args
            while True:
                sys.stderr.write(self.io.get())

        nresults = self.getResultsCount()
        # print 'Now reading',nresults,'results'
        #while True:
        #    print repr(self.io.readline())
        results = []
        for i in xrange(nresults):
            res = self.io.binread()
            if self.log: self.log.debug(' RES_'+str(i)+': '+repr(res))
            results.append(res)
        if len(results)==1:
            return results[0]
        elif len(results)>1:
            return results

    def close(self):
        if not self.closed:
            if self.log: self.log.info('NOW CLOSING: method close() called')                
            try:
                self.logged_write('!Q')
            except IOError:
                pass
            if self.log: self.log.info('WAITING FOR CHILD PROCESS TO FINISH: os.wait()')                
            os.wait()
            if self.log: self.log.info('CLOSED.')
            self.closed = True
                
    def __del__(self):
        self.close()

    def kill(self):
        """Kill the process with the TERM signal."""
        if self.pid == -1:
            raise RuntimeError, 'The "kill" method is only available with Python 2.4 and above.'
        os.kill(self.pid, 15)


class RemotePObject(object):
    
    def __init__(self, serv, objid):
        self.server = serv
        self.objid = objid
        self._by_value = False
        self.__initialised = True

    def _serial_number(self):
        return self.objid
        
    def __getattr__(self,methodname):
        """methodname can also be an optionname"""
        if self.callMethod("hasOption", methodname):
            return self.getOption(methodname)
        else:
            def f(*args):
                return self.callMethod(methodname,*args)
            return f

    def __setattr__(self, optionname, value):
        dict = self.__dict__
        if not dict.has_key('_RemotePObject__initialised'):  # this test allows attributes to be set in the __init__ method
            return object.__setattr__(self, optionname, value)
        elif dict.has_key(optionname):       # any normal attributes are handled normally
            return object.__setattr__(self, optionname, value)
        else:
            self.callMethod('changeOptions',{optionname:plearn_repr(value)})

    def callMethod(self,methodname, *args):
        return self.server.callMethod(self.objid, methodname, *args)

    def delete(self):
        self.server.delete(self.objid)

    def plearn_repr(self, indent_level, inner_repr):
        return " *"+str(self.objid)+"; "

    def __repr__(self):
        # return repr(self.getObject())
        return 'RemotePObject(objid='+str(self.objid)+')'
    
    def __str__(self):
        #    return str(self.getObject())
        return self.__repr__()

##     def getOptionAsString(self, optionname):
##         serv = self.server
##         serv.sendMethodCallHeader(self.objid, "getOptionAsString", 1)
##         serv.io.write_typed(optionname)
##         serv.io.flush()
##         serv.expectResults(1)
##         return serv.io.read_string()

##     def changeOptions(self, **options):
##         to_server.write()


# vim: filetype=python:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
