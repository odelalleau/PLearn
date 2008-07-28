## Automatically adapted for numpy.numarray Jun 13, 2007 by python_numarray_to_numpy (-xsm)

# PMat.py
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

#import numarray, sys, os, os.path
import numpy.numarray, sys, os, os.path
pyplearn_import_failed = False
try:
    from plearn.pyplearn.plearn_repr import plearn_repr, format_list_elements
except ImportError:
    pyplearn_import_failed = True
    
                             

def array_columns( a, cols ):
    indices = None
    if isinstance( cols, int ):
        indices = [ cols ]
    elif isinstance( cols, slice ):
        #print cols
        indices = range( *cols.indices(cols.stop) )
    else:
        indices = list( cols )            

    return numpy.numarray.take(a, indices, axis=1)

def load_pmat_as_array(fname):
    s = file(fname,'rb').read()
    formatstr = s[0:64]
    datastr = s[64:]
    structuretype, l, w, data_type, endianness = formatstr.split()

    if data_type=='DOUBLE':
        elemtype = 'd'
    elif data_type=='FLOAT':
        elemtype = 'f'
    else:
        raise ValueError('Invalid data type in file header: '+data_type)

    if endianness=='LITTLE_ENDIAN':
        byteorder = 'little'
    elif endianness=='BIG_ENDIAN':
        byteorder = 'big'
    else:
        raise ValueError('Invalid endianness in file header: '+endianness)

    l = int(l)
    w = int(w)
    X = numpy.numarray.fromstring(datastr,elemtype, shape=(l,w) )
    if byteorder!=sys.byteorder:
        X.byteswap(True)
    return X

def save_array_as_pmat( fname, ar, fieldnames=[] ):
    s = file(fname,'wb')
    
    length, width = ar.shape
    if fieldnames:
        assert len(fieldnames) == width
        metadatadir = fname+'.metadata'
        if not os.path.isdir(metadatadir):
            os.mkdir(metadatadir)
        fieldnamefile = os.path.join(metadatadir,'fieldnames')
        f = open(fieldnamefile,'wb')
        for name in fieldnames:
            f.write(name+'\t0\n')
        f.close()
    
    header = 'MATRIX ' + str(length) + ' ' + str(width) + ' '
    if ar.dtype.char=='d':
        header += 'DOUBLE '
        elemsize = 8

    elif ar.dtype.char=='f':
        header += 'FLOAT '
        elemsize = 4

    else:
        raise TypeError('Unsupported typecode: %s' % ar.dtype.char)

    rowsize = elemsize*width

    if sys.byteorder=='little':
        header += 'LITTLE_ENDIAN '
    elif sys.byteorder=='big':
        header += 'BIG_ENDIAN '
    else:
        raise TypeError('Unsupported sys.byteorder: '+repr(sys.byteorder))

    header += ' '*(63-len(header))+'\n'
    s.write( header )
    s.write( ar.tostring() )
    s.close()    


#######  Iterators  ###########################################################

class VMatIt:
    def __init__(self, vmat):                
        self.vmat = vmat
        self.cur_row = 0

    def __iter__(self):
        return self

    def next(self):
        if self.cur_row==self.vmat.length:
            raise StopIteration
        row = self.vmat.getRow(self.cur_row)
        self.cur_row += 1
        return row    

class ColumnIt:
    def __init__(self, vmat, col):                
        self.vmat = vmat
        self.col  = col
        self.cur_row = 0

    def __iter__(self):
        return self

    def next(self):
        if self.cur_row==self.vmat.length:
            raise StopIteration
        val = self.vmat[self.cur_row, self.col]
        self.cur_row += 1
        return val

#######  VMat classes  ########################################################

class VMat:
    def __iter__(self):
        return VMatIt(self)
    
    def __getitem__( self, key ):
        if isinstance( key, slice ):
            start, stop, step = key.start, key.stop, key.step
            if step!=None:
                raise IndexError('Extended slice with step not currently supported')

            if start is None:
                start = 0

            l = self.length
            if stop is None or stop > l:
                stop = l

            return self.getRows(start,stop-start)
        
        elif isinstance( key, tuple ):
            # Basically returns a SubVMatrix
            assert len(key) == 2
            rows = self.__getitem__( key[0] )

            shape = rows.shape                       
            if len(shape) == 1:
                return rows[ key[1] ]

            cols = key[1]
            if isinstance(cols, slice):
                start, stop, step = cols.start, cols.stop, cols.step
                if start is None:
                    start = 0

                if stop is None:
                    stop = self.width
                elif stop < 0:
                    stop = self.width+stop

                cols = slice(start, stop, step)

            return array_columns(rows, cols)

        elif isinstance( key, str ):
            # The key is considered to be a fieldname and a column is
            # returned.
            try:
                return array_columns( self.getRows(0,self.length),
                                      self.fieldnames.index(key)  )
            except ValueError:
                print >>sys.stderr, "Key is '%s' while fieldnames are:" % key
                print >>sys.stderr, self.fieldnames
                raise
                
        else:
            if key<0: key+=self.length
            return self.getRow(key)

    def getFieldIndex(self, fieldname):
        try:
            return self.fieldnames.index(fieldname)
        except ValueError:
            raise ValueError( "VMat has no field named %s. Field names: %s"
                              %(fieldname, ','.join(self.fieldnames)) )

class PMat( VMat ):

    def __init__(self, fname, openmode='r', fieldnames=[], elemtype='d',
                 inputsize=-1, targetsize=-1, weightsize=-1, array = None):
        self.fname = fname
        self.inputsize = inputsize
        self.targetsize = targetsize
        self.weightsize = weightsize
        self.openmode = openmode
        if openmode=='r':
            self.f = open(fname,'rb')
            self.read_and_parse_header()
            self.load_fieldnames()            
                
        elif openmode=='w':
            self.f = open(fname,'w+b')
            self.fieldnames = fieldnames
            self.save_fieldnames()
            self.length = 0
            self.width = len(fieldnames)
            self.elemtype = elemtype
            self.swap_bytes = False
            self.write_header()            
            
        elif openmode=='a':
            self.f = open(fname,'r+b')
            self.read_and_parse_header()
            self.load_fieldnames()

        else:
            raise ValueError("Currently only supported openmodes are 'r', 'w' and 'a': "+repr(openmode)+" is not supported")

        if array is not None:
            shape  = array.shape
            if len(shape) == 1:
                row_format = lambda r: [ r ]
            elif len(shape) == 2:
                row_format = lambda r: r

            for row in array:
                self.appendRow( row_format(row) )

    def __del__(self):
        self.close()

    def write_header(self):
        header = 'MATRIX ' + str(self.length) + ' ' + str(self.width) + ' '

        if self.elemtype=='d':
            header += 'DOUBLE '
            self.elemsize = 8
        elif self.elemtype=='f':
            header += 'FLOAT '
            self.elemsize = 4
        else:
            raise TypeError('Unsupported elemtype: '+repr(elemtype))
        self.rowsize = self.elemsize*self.width

        if sys.byteorder=='little':
            header += 'LITTLE_ENDIAN '
        elif sys.byteorder=='big':
            header += 'BIG_ENDIAN '
        else:
            raise TypeError('Unsupported sys.byteorder: '+repr(sys.byteorder))
        
        header += ' '*(63-len(header))+'\n'

        self.f.seek(0)
        self.f.write(header)
        
    def read_and_parse_header(self):        
        header = self.f.read(64)        
        mat_type, l, w, data_type, endianness = header.split()
        if mat_type!='MATRIX':
            raise ValueError('Invalid file header (should start with MATRIX)')
        self.length = int(l)
        self.width = int(w)
        if endianness=='LITTLE_ENDIAN':
            byteorder = 'little'
        elif endianness=='BIG_ENDIAN':
            byteorder = 'big'
        else:
            raise ValueError('Invalid endianness in file header: '+endianness)
        self.swap_bytes = (byteorder!=sys.byteorder)

        if data_type=='DOUBLE':
            self.elemtype = 'd'
            self.elemsize = 8
        elif data_type=='FLOAT':
            self.elemtype = 'f'
            self.elemsize = 4
        else:
            raise ValueError('Invalid data type in file header: '+data_type)
        self.rowsize = self.elemsize*self.width

    def load_fieldnames(self):
        self.fieldnames = []
        fieldnamefile = os.path.join(self.fname+'.metadata','fieldnames')
        if os.path.isfile(fieldnamefile):
            f = open(fieldnamefile)
            for row in f:
                row = row.split()
                if len(row)>0:
                    self.fieldnames.append(row[0])
            f.close()
        else:
            self.fieldnames = [ "field_"+str(i) for i in range(self.width) ]
            
    def save_fieldnames(self):
        metadatadir = self.fname+'.metadata'
        if not os.path.isdir(metadatadir):
            os.mkdir(metadatadir)
        fieldnamefile = os.path.join(metadatadir,'fieldnames')
        f = open(fieldnamefile,'wb')
        for name in self.fieldnames:
            f.write(name+'\t0\n')
        f.close()

    def getRow(self,i):
        if i<0 or i>=self.length:
            raise IndexError('PMat index out of range')
        self.f.seek(64+i*self.rowsize)
        data = self.f.read(self.rowsize)
        ar = numpy.numarray.fromstring(data, self.elemtype, (self.width,))
        if self.swap_bytes:
            ar.byteswap(True)
        return ar

    def getRows(self,i,l):
        if i<0 or l<0 or i+l>self.length:
            raise IndexError('PMat index out of range')
        self.f.seek(64+i*self.rowsize)
        data = self.f.read(l*self.rowsize)
        ar = numpy.numarray.fromstring(data, self.elemtype, (l,self.width))
        if self.swap_bytes:
            ar.byteswap(True)
        return ar

    def checkzerorow(self,i):
        if i<0 or i>self.length:
            raise IndexError('PMat index out of range')
        self.f.seek(64+i*self.rowsize)
        data = self.f.read(self.rowsize)
        ar = numpy.numarray.fromstring(data, self.elemtype, (len(data)/self.elemsize,))
        if self.swap_bytes:
            ar.byteswap(True)
        for elem in ar:
            if elem!=0:
                return False
        return True
    
    def putRow(self,i,row):
        if i<0 or i>=self.length:
            raise IndexError('PMat index out of range')
        if len(row)!=self.width:
            raise TypeError('length of row ('+str(len(row))+ ') differs from matrix width ('+str(self.width)+')')
        if i<0 or i>=self.length:
            raise IndexError
        if self.swap_bytes: # must make a copy and swap bytes
            ar = numpy.numarray.numarray(row,type=self.elemtype)
            ar.byteswap(True)
        else: # asarray makes a copy if not already a numarray of the right type
            ar = numpy.numarray.asarray(row,type=self.elemtype)
        self.f.seek(64+i*self.rowsize)
        self.f.write(ar.tostring())

    def appendRow(self,row):
        if len(row)!=self.width:
            raise TypeError('length of row ('+str(len(row))+ ') differs from matrix width ('+str(self.width)+') for file "'+self.fname+'"')
        if self.swap_bytes: # must make a copy and swap bytes
            ar = numpy.numarray.numarray(row,type=self.elemtype)
            ar.byteswap(True)
        else: # asarray makes a copy if not already a numarray of the right type
            ar = numpy.numarray.asarray(row,type=self.elemtype)

        self.f.seek(64+self.length*self.rowsize)
        self.f.write(ar.tostring())
        self.length += 1
        self.write_header() # update length in header

    def flush(self):
        if self.openmode!='r':
            self.f.flush()

    def close(self):
        if hasattr(self, 'f'):
            self.f.close()

    def append(self,row):
        self.appendRow(row)

    def __setitem__(self, i, row):
        l = self.length
        if i<0: i+=l
        self.putRow(i,row)

    def __len__(self):
        return self.length

    if not pyplearn_import_failed:
        def __str__( self ):
            return plearn_repr(self, indent_level=0)
    
        def plearn_repr( self, indent_level=0, inner_repr=plearn_repr ):
            # asking for plearn_repr could be to send specification over
            # to another prg so that will open the .pmat
            # So we make sure data is flushed to disk.
            self.flush()
    
            def elem_format( elem ):
                k, v = elem
                return '%s = %s' % ( k, inner_repr(v, indent_level+1) )
    
            options = [ ( 'filename',   self.fname      ),
                        ( 'inputsize',  self.inputsize  ), 
                        ( 'targetsize', self.targetsize ),
                        ( 'weightsize', self.weightsize ) ]
            return 'FileVMatrix(%s)' % format_list_elements( options, elem_format, indent_level+1 )
            
if __name__ == '__main__':
    pmat = PMat( 'tmp.pmat', 'w', fieldnames=['F1', 'F2'] )
    pmat.append( [1, 2] )
    pmat.append( [3, 4] )
    pmat.close()

    pmat = PMat( 'tmp.pmat', 'r' )
    print pmat
    print pmat[:]
    # print "+++ tmp.pmat contains: "
    # os.system( 'plearn vmat cat tmp.pmat' )

    os.remove( 'tmp.pmat' )
    if os.path.exists( 'tmp.pmat.metadata' ):
        import shutil
        shutil.rmtree( 'tmp.pmat.metadata' )
