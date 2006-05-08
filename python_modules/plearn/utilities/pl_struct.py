# struct.py
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

"""Workaround for some bugs in Python 2.3 and 2.4 standard struct module.

The purpose of this module is to work around some bugs that were observed
with the standard Python 'struct' module.  In particular, unpacking a C
'float' or 'double' NaN as a Python float yields an 'inf' (infinity) when
the unpacking format '<' is used.  This module simply replaces the
unpacking format by '@' on little-endian platforms when unpacking a single
float or double.

This module imports the standard python 'struct' module in its entirety,
and is a drop-in replacement for this module.  Simply use the following
import to interface with existing code:

    import plearn.utilities.pl_struct as struct
"""

import sys
from struct import *

if sys.byteorder == 'little':
    import struct
    def unpack(fmt,s):
        if fmt == '<f':
            return struct.unpack('@f', s)
        elif fmt == '<d':
            return struct.unpack('@d', s)
        else:
            return struct.unpack(fmt, s)
    unpack.__doc__ = struct.unpack.__doc__
    
del sys
