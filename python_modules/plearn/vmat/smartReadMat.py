# smartReadMat.py
# Copyright (C) 2007 by Nicolas Chapados
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

import csv
import numpy.numarray

from plearn.vmat.PMat import PMat
from plearn.vmat.readAMat import readAMat


### BEGIN ugly hack

# Code copied from module csv.py, but with the rdr = ... line changed
# so we don't get an exception
try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO

from _csv import reader

def has_header(self, sample):
    # Creates a dictionary of types of data in each column. If any
    # column is of a single type (say, integers), *except* for the first
    # row, then the first row is presumed to be labels. If the type
    # can't be determined, it is assumed to be a string in which case
    # the length of the string is the determining factor: if all of the
    # rows except for the first are the same length, it's a header.
    # Finally, a 'vote' is taken at the end for each column, adding or
    # subtracting from the likelihood of the first row being a header.

    rdr = reader(StringIO(sample))

    header = rdr.next() # assume first row is header

    columns = len(header)
    columnTypes = {}
    for i in range(columns): columnTypes[i] = None

    checked = 0
    for row in rdr:
        # arbitrary number of rows to check, to keep it sane
        if checked > 20:
            break
        checked += 1

        if len(row) != columns:
            continue # skip rows that have irregular number of columns

        for col in columnTypes.keys():

            for thisType in [int, long, float, complex]:
                try:
                    thisType(row[col])
                    break
                except (ValueError, OverflowError):
                    pass
            else:
                # fallback to length of string
                thisType = len(row[col])

            # treat longs as ints
            if thisType == long:
                thisType = int

            if thisType != columnTypes[col]:
                if columnTypes[col] is None: # add new column type
                    columnTypes[col] = thisType
                else:
                    # type is inconsistent, remove column from
                    # consideration
                    del columnTypes[col]

    # finally, compare results against first row and "vote"
    # on whether it's a header
    hasHeader = 0
    for col, colType in columnTypes.items():
        if type(colType) == type(0): # it's a length
            if len(header[col]) != colType:
                hasHeader += 1
            else:
                hasHeader -= 1
        else: # attempt typecast
            try:
                colType(header[col])
            except (ValueError, TypeError):
                hasHeader += 1
            else:
                hasHeader -= 1

    return hasHeader > 0


csv.Sniffer.has_header = has_header

### END ugly hack


#####  smartReadMat  ########################################################

def smartReadMat(filename):
    """Read a matrix from a file, determining the file type automatically.

    This function returns a pair (matrix,fieldnames).  The filetype is
    determined automatically from the extension.  The following formats are
    currently supported:

    - '.amat' : PLearn ascii matrix format
    - '.pmat' : PLearn binary matrix format
    - '.csv'  : Text-file comma-separated values format
    """

    if filename.endswith(".amat"):
        arr, fieldnames = readAMat(filename)

    elif filename.endswith(".pmat"):
        pmat = PMat(filename)
        arr  = pmat.getRows(0, pmat.length)
        fieldnames = pmat.fieldnames
        pmat.close()

    elif filename.endswith(".csv"):
        # Use CSV sniffer to detect presence of header.
        sniffer = csv.Sniffer()
        f = open(filename)
        sample = f.read(1000)
        has_header = sniffer.has_header(sample)
        f.seek(0)

        # Load csv into array
        csv_reader = csv.reader(f)
        if has_header:
            fieldnames = csv_reader.next()
        arr = numpy.numarray.array([[float(value) for value in fields] for fields in csv_reader])
        if not has_header:
            # Generate fake fieldnames
            fieldnames = ['field%d' % (i + 1) for i in range(arr.shape[1])]
            
        f.close()

    else:
        raise ValueError, "Unrecognized file type for '%s'; valid extensions are: " \
                          "{'.amat', '.pmat', '.csv'}" % filename

    return arr, fieldnames
