# pobject.py
# Copyright (C) 2006 Nicolas Chapados
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

# Author: Nicolas Chapados

class PObject(object):
    """Python wrapper for object received from PLearn.

    Objects that are received from a PLearn Server using plservice and
    plio are created with this Python type.  Each PLearn option is mapped
    to an object attribute.  There is a magic attribute _classname_ that
    contains the PLearn classname.

    This class also contains a VERY LIGHTWEIGHT mechanism for sending
    itself back to PLearn as a value type.  For more full-fledged support,
    you should use a PyPLearnObject.
    """
    def __init__(self, **members):
        self._classname_ = self.__class__.__name__
        self.__dict__.update(members)
    
    def __repr__(self):
        s = self._classname_ + '('
        s += ', '.join([ "%s=%s" % (key, PObject.__optionrepr(value))
                         for (key,value) in self.__dict__.iteritems()
                         if not key.startswith('_') ])
        s += ')'
        return s
    
    def __optionrepr(value):
        return str(value)
    __optionrepr = staticmethod(__optionrepr)
