# readMMat.py
# Copyright (C) 2006 by Nicolas Chapados
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

from numarray import array
import fpconst

def safefloat(str):
    """Convert the given string to its float value. It is 'safe' in the sense
    that missing values ('nan') will be properly converted to the corresponding
    float value under all platforms, contrarily to 'float(str)'.
    """
    if str.lower() == 'nan':
        return fpconst.NaN
    else:
        return float(str)

def readAMat(amatname):
    """Read a PLearn .amat file and return it as a numarray Array.

    Return a tuple, with as the first argument the array itself, and as
    the second argument the fieldnames (list of strings).
    """
    ### NOTE: this version is much faster than first creating the array and
    ### updating each row as it is read...  Bizarrely enough
    f = open(amatname)
    a = []
    for line in f:
        if line.startswith("#size:"):
            (length,width) = line[6:].strip().split(" ")

        elif line.startswith("#:"):
            fieldnames = line[2:].strip().split(" ")
            pass

        else:
            row = [ safefloat(x) for x in line.strip().split(" ") ]
            a.append(row)

    f.close()
    return array(a), fieldnames
