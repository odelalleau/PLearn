# publisher.py
# Copyright (C) 2006 Christian Dorion
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

# Author: Christian Dorion

import logging
# __hdlr = logging.StreamHandler()
# __hdlr.setFormatter( logging.Formatter("[futures.py] %(message)s") )
# logging.root.addHandler(__hdlr)
# logging.root.setLevel(logging._levelNames["DEBUG"])

class Publisher:
    """Publishes staticmethod instances from the provided base class."""
    class Function:
        def __init__(self, publisher, function_name):
            self.name = function_name
            self.publisher = publisher

        def __call__(self, subclass, *args, **kwargs):
            if subclass not in self.publisher.subclasses:
                raise ValueError("%s(%s, ...): Class %s is unknown. Choose among:\n%s"
                    %(self.name, subclass, subclass, self.publisher.subclasses.keys()))
            
            SubClass = self.publisher.subclasses[subclass]
            function = getattr(SubClass, self.name)
            try:
                return function(*args, **kwargs)
            except TypeError, err:
                raise TypeError("%s -- Note that class name %s was extracted from arguments"
                                %(err, subclass))
                        
    def __init__(self, environment, basecls, dic):
        self.basecls = basecls
        dic["_subclasses"] = {}
        self.subclasses = dic["_subclasses"]
        
        for function_name, function in dic.iteritems():
            if isinstance(function, staticmethod):
                logging.debug("* PUBLISHING %s"%function_name)
                environment[function_name] = self.Function(self, function_name)
                logging.debug("%s: %s"%(function_name, environment[function_name]))
                logging.debug("environment: %s\n---\n"%environment)

    def registerSubClass(self, subclass):
        assert issubclass(subclass, self.basecls)
        self.subclasses[subclass.__name__] = subclass
        logging.debug("%s registered."%subclass.__name__)


