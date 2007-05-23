#
# pyext
# Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

from plearn.pyext.plext import *
from plearn.pyplearn.plargs import *
import cgitb
cgitb.enable(format='PLearn')

print versionString()

class pl:
    class __metaclass__(type):
        def __getattr__(cls, name):
            def newObj(**kwargs):
                obj= newObject(name+'()')
                for k in kwargs.keys():
                    obj.__setattr__(k, kwargs[k])
                obj.build()
                return obj
            return newObj

# Enact the use of plargs: the current behavior is to consider as a plargs
# any command-line argument that contains a '=' char and to neglect all
# others
plargs.parse([ arg for arg in sys.argv if arg.find('=') != -1 ])

if __name__ == "__main__":
    class A(plargs):
        T = plopt(0)

    class B(plnamespace):
        T = plopt(1)

    print sys.argv[1:]
    print A.T
    print B.T
        
    # python __init__.py T=10 B.T=25
    # ['T=10', 'B.T=25']
    # 10
    # 25
