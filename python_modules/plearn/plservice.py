# plearn_service.py
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

import os, string
from plearn.pyplearn import *
import plearn.plio
import sys

def launch_plearn_server(command = 'plearn server'):
    to_server, from_server = os.popen2(command, 'b')
    return RemotePLearnServer(from_server, to_server)
        
class RemotePLearnServer:

    def __init__(self, from_server, to_server):
        self.io = plearn.plio.PLearnIO(from_server, to_server)
        self.nextid = 1
        self.objects = {}
        self.clear_maps = True
        self.callFunction("binary")
        self.callFunction("implicit_storage",True)
        self.dbg_dump = False

    def new(self, objectspec):
        """
        objectspec is the specification of a plearn object.
        It can be either a string in the plearn serialization format,
        or a PyPLearnObject such as those built by calling pl.ClassName(...)
        """
        specstr = objectspec
        if type(specstr)!=str:
            specstr = specstr.plearn_repr()
            
        objid = self.nextid        
        self.callNewObject(objid,specstr)
        self.nextid += 1
        obj = RemotePObject(self, objid)
        self.objects[objid] = obj
        return obj

    def load(self, objfilepath):
        objid = self.nextid        
        self.callLoadObject(objid, objfilepath)
        self.nextid += 1
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
            
    def callNewObject(self, objid, objspecstr):
        self.clearMaps()
        self.io.write('!N '+str(objid)+' '+objspecstr+'\n')
        self.io.flush()
        self.expectResults(0)

    def callLoadObject(self, objid, filepath):
        self.clearMaps()
        self.io.write('!L '+str(objid)+' '+filepath+'\n')
        self.io.flush()
        self.expectResults(0)

    def callDeleteObject(self, objid):
        self.io.write('!D '+str(objid)+'\n')
        self.io.flush()
        self.expectResults(0)

    def callDeleteAllObjects(self):
        self.io.write('!Z \n')
        self.io.flush()
        self.expectResults(0)

    def clearMaps(self):
        if self.clear_maps:
            self.io.copies_map_in.clear()
            self.io.copies_map_out.clear()

    def sendFunctionCallHeader(self, funcname, nargs):
        self.clearMaps()
        self.io.write('!F '+funcname+' '+str(nargs)+' ')

    def sendMethodCallHeader(self, objid, methodname, nargs):
        self.clearMaps()
        self.io.write('!M '+str(objid)+' '+methodname+' '+str(nargs)+' ')

    def getResultsCount(self):
        self.io.skip_blanks_and_comments()
        c = self.io.get()
        if c!='!':
            raise TypeError("Returns received from server are expected to start with a ! but read "+c)
        c = self.io.get()
        if c=='R':
            nreturned = self.io.read_int()
            return nreturned
        elif c=='E':
            msg = self.io.read_string()
            raise RuntimeError(msg)
        else:
            raise TypeError("Expected !R or !E but read !"+c)

    def expectResults(self, nargs_expected):
        nreturned = self.getResultsCount()
        if nreturned!=nargs_expected:
            raise TypeError("Expected "+str(nargs_expected)+" return arguments, but read R "+str(nreturned))

    def callFunction(self, functionname, *args):
        self.sendFunctionCallHeader(functionname, len(args))
        for arg in args:
            self.io.write_typed(arg)
        self.io.write('\n')
        self.io.flush()
        # print 'python sent it!'
        nresults = self.getResultsCount()
        results = []
        for i in xrange(nresults):
            results.append(self.io.binread())
        if len(results)==1:
            return results[0]
        elif len(results)>1:
            return results

    def callMethod(self, objid, methodname, *args):
        self.sendMethodCallHeader(objid, methodname, len(args))
        for arg in args:
            self.io.write_typed(arg)
        self.io.flush()

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
            results.append(self.io.binread())
        if len(results)==1:
            return results[0]
        elif len(results)>1:
            return results

    def close(self):
        self.io.write('!Q')
        self.io.flush()
        

class RemotePObject:
    
    def __init__(self, serv, objid):
        self.server = serv
        self.objid = objid
        
    def __getattr__(self,methodname):
        def f(*args):
            return self.callMethod(methodname,*args)
        return f

    def callMethod(self,methodname, *args):
        return self.server.callMethod(self.objid, methodname, *args)

    def delete(self):
        self.server.delete(self.objid)

##     def getOptionAsString(self, optionname):
##         serv = self.server
##         serv.sendMethodCallHeader(self.objid, "getOptionAsString", 1)
##         serv.io.write_typed(optionname)
##         serv.io.flush()
##         serv.expectResults(1)
##         return serv.io.read_string()

##     def changeOptions(self, **options):
##         to_server.write()


