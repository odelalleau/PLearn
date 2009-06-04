## Automatically adapted for numpy.numarray Jun 13, 2007 by python_numarray_to_numpy (-xsm)

"""
table.py

Copyright (C) 2005-2009 ApSTAT Technologies Inc.

This file was contributed by ApSTAT Technologies to the
PLearn library under the following BSD-style license: 

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
"""

# Author: Pascal Vincent

import os, os.path, string, struct, zlib, fpconst, pickle, csv, time, copy, subprocess

from numpy.numarray import array, argsort, random_array

from plearn.utilities.progress import PBar

from random import seed, randint, uniform

# Use of VMat through the Python-bridge
try:
    from plearn.pyext import DiskVMatrix, FileVMatrix
except ImportError:
    print "WARNING: Import of DiskVMatrix, FileVMatrix from plearn.pyext failed. Probably no plearn python extension compiled. Skipping this import." 

def float_or_str(x):
    try: return float(x)
    except: return str(x)

def float_or_zero(x):
    try: return float(x)
    except: return 0.

def float_or_value(x, val):
    try: return float(x)
    except: return val

def int_or_zero(x):
    try: return int(x)
    except: return 0

def int_or_value(x, val):
    try: return int(x)
    except: return val

class StructFile:
    """File with fixed size packed binary records.
    Records are packed using the struct module;
    all records use the same format string.
    """

    def __init__(self, fname, struct_format, openmode='r', data_offset=0):
        """data_offset is the pos. of beg. of data from beg. of file, in bytes"""
        self.fname = fname
        self.openmode = openmode
        self.struct_format = struct_format
        self.struct_size = struct.calcsize(struct_format)
        self.closed = False
        self.data_offset = data_offset
        self.last_call_was_append = False
        
        if openmode=='r':
            self._len = (os.path.getsize(fname)-data_offset)/self.struct_size
            self.f = open(fname,'rb')
        elif openmode=='r+':
            self._len = (os.path.getsize(fname)-data_offset)/self.struct_size
            self.f = open(fname,'rb+')
        elif openmode=='w+':
            self._len = 0
            self.f = open(fname,'wb+')
        else:
            raise ValueError("Invalid value for openmode ("+`openmode`+" Must be one of 'r' 'r+' or 'w+'")
            
    def append(self, x):
        if not self.last_call_was_append:
            self.f.seek(0,2) # seek to end of index file
            
        if isinstance(x,list):
            self.f.write(struct.pack(self.struct_format,*x))
        else:
            self.f.write(struct.pack(self.struct_format,x))
        self._len += 1

        self.last_call_was_append = True

    def __getitem__(self,i):
        self.last_call_was_append = False
        
        l = self.__len__()
        if i<0: i+=l
        if i<0 or i>=l:
            raise IndexError('StructFile index out of range')

        self.f.seek(self.data_offset + i*self.struct_size)
        x = struct.unpack(self.struct_format,self.f.read(self.struct_size))

        if len(x)==1:
            return x[0]
        else:
            return x

    def __setitem__(self, i, x):
        self.last_call_was_append = False

        l = self.__len__()
        if i<0: i+=l
        if i<0 or i>=l:
            raise IndexError('StructFile index out of range')
        self.f.seek(self.data_offset + i*self.struct_size)
        try: # should work when x is a kind of list
            self.f.write(struct.pack(self.struct_format,*x))
        except TypeError: # should work when x is a single element
            self.f.write(struct.pack(self.struct_format,x))
        #self.flush() ##is it necessary?
            

    def flush(self):
        self.last_call_was_append = False
        self.f.flush()

    def close(self):
        self.last_call_was_append = False

        if not self.closed:
            self.f.close()
            self.closed = True

    # Note: it's better for your code to call close() explicitly rather than rely on this being called automatically
    def __del__(self):
        self.close()
        
    def __len__(self):
        return int(self._len)


class IntVecFile(StructFile): # read-only for now...
    def __init__(self, fname, openmode='r', endianness='L'):
        """endianness is ignored when reading"""

        #self.fname = fname
        #self.openmode = openmode

        
        if openmode[0]=='r':
            self.f = open(fname,'rb')
            self.readFileSignature()
            self.f.close()
        else:
            self.endianness= endianness
#         if openmode=='r':
#             self.f = open(fname,'rb')
#             self.readFileSignature()
#         elif openmode=='r+':
#             self.f = open(fname,'rb+')
#             self.readFileSignature()
#         elif openmode=='w+':
#             self._len = 0
#             self.f = open(fname,'wb+')
#             self.endianness= endianness
#             self.writeFileSignature()
#         else:
#             raise ValueError("Invalid value for openmode ("+`openmode`+" Must be one of 'r' 'r+' or 'w+'")

        endcode = '<'
        if self.endianness=='L': endcode = '<'
        elif self.endianness=='B': endcode = '>'
        else: raise ValueError("Invalid value endianness (" + self.endianness + ") must be 'L' or 'B'")
        #self.struct_format = endcode+'l'
        struct_format = endcode+'l'
        #self.struct_size = struct.calcsize(self.struct_format)

        #if openmode[0]=='r':
        #    self._len = os.path.getsize(fname)/self.struct_size -2

        StructFile.__init__(self, fname, struct_format, openmode, 8)
        if openmode[0]=='w':
            self.writeFileSignature()


    def readFileSignature(self):
        self.f.seek(0)
        s = self.f.read(4)
        if s != '\xde\xad\xbe\xef':
            raise RuntimeError("Is this an IntVecFile?? ("+self.fname+")")
        self.endianness= self.f.read(1)
        if self.endianness not in ['L','B']:
            raise RuntimeError("Invalid endianness ("+self.endianness+") for IntVecFile "+self.fname\
                               +" (should be 'L' or 'B')")
        
    def writeFileSignature(self):
        self.f.seek(0)
        self.f.write('\xde\xad\xbe\xef') #dead beef
        self.f.write(self.endianness)
        self.f.write('\0\0')
        self.f.write('\1') # version number

#     def append(self, x):
#         self.f.seek(0,2) # seek to end of index file
#         if isinstance(x,list):
#             self.f.write(struct.pack(self.struct_format,*x))
#         else:
#             self.f.write(struct.pack(self.struct_format,x))
#         self._len += 1

#     def __getitem__(self,i):
#         l = self.__len__()
#         if i<0: i+=l
#         if i<0 or i>=l:
#             raise IndexError('StructFile index out of range')
#         self.f.seek((i+2)*self.struct_size)
#         x = struct.unpack(self.struct_format,self.f.read(self.struct_size))
#         if len(x)==1:
#             return x[0]
#         else:
#             return x

#     def __setitem__(self, i, x):
#         l = self.__len__()
#         if i<0: i+=l
#         if i<0 or i>=l:
#             raise IndexError('StructFile index out of range')
#         self.f.seek(i*self.struct_size)
#         try: # should work when x is a kind of list
#             self.f.write(struct.pack(self.struct_format,*x))
#         except TypeError: # should work when x is a single element
#             self.f.write(struct.pack(self.struct_format,x))
            

#     def flush(self):
#         self.f.flush()

#     def close(self):
#         self.f.close()
        
#     def __len__(self):
#         return int(self._len)


def build_row_index_file(txtfilename, idxfilename, struct_format = '<Q', offset= 0):
    """Creates an index file giving the offset to each line (record) in a
    text file.    
    """
    f = open(txtfilename)
    index = StructFile(idxfilename,'<Q','w+')
    while True:
        pos = f.tell()
        line = f.readline()
        if len(line)==0:
            break
        if offset > 0:
            offset-= 1
        else:
            index.append(pos)

class FieldValues:
    """DEPRECATED -- use ListWithFieldNames instead
    FieldValues are objects that represent a row of data by associating
    field names with their values as object attributes.
    """
    def __init__(self, fieldnames, values):
        for name,val in zip(fieldnames, values):
            setattr(self,name,val)

class FieldIndex:
    """FieldIndex objects have an attribute for each field in a table
    indicating the index of that field.
    """
    def __init__(self, fieldnames):
        for i in range(len(fieldnames)):
            setattr(self,fieldnames[i],i)


class ListWithFieldNames:
    """This allows to have a view on a sequence
    object with associated fieldnames, so that the elements of the sequence
    can be accessed with either its numerical position, or its
    fieldname."""
    
    def __init__(self, list_items, fieldnames=None, fieldpos=None):
        """elemlist can be any sequence of elements

        fieldnames should be a sequence of strings of the same length as
        elemlist

        fieldpos can optionally be passed (otherwise self.fieldpos will be
        computed from the fieldnames): it corresponds to a dictionary,
        mapping a fieldname to its position in the list.
        """
        self.list = list(list_items)
        self.fieldnames = fieldnames
        self.fieldpos = fieldpos
        self.rownum = -1

        # if autoappend is set to True, then accessing a field with [fieldname] will
        # append it if fieldname is not already a fieldname of the list
        # if autoappend is set to false, an error will be raised if accessing
        self.autoappend = False
        
        # we suppose we may initially have shared fieldnames and fieldpos
        # make_private_copy_of_fields will make a private copy of those, and
        # will be called as soon as a we append or delete an item.
        self.shared_fieldnames = True    

        assert(fieldnames is None or len(fieldnames)==len(list_items))
        if fieldnames and not fieldpos:
            self.fieldpos = {}
            pos = 0
            for name in fieldnames:
                self.fieldpos[name] = pos
                pos += 1
                
    def __len__(self):
        return len(self.list)

    def __getitem__(self,key):
        if isinstance(key,str):
            key = self.fieldpos[key]
        return self.list[key]

    def __setitem__(self, key, value):
        if isinstance(key,str):
            try:
                key = self.fieldpos[key]
                self.list[key] = value
            except KeyError:
                if self.autoappend:
                    self.append(value, key)
                else:
                    raise
        else:
            self.list[key] = value

    def __delitem__(self,key):
        if self.shared_fieldnames:
            self.make_private_copy_of_fields()
        if isinstance(key,str):
            key = self.fieldpos[key]
        del self.list[key]
        if self.fieldnames:
            fieldname = self.fieldnames[key]
            del self.fieldnames[key]
            del self.fieldpos[fieldname]

    def __iter__(self):
        return self.list.__iter__()

    def __repr__(self):
        return self.list.__repr__()

    def __str__(self):
        return self.list.__str__()
    
    def __add__(self, other):
        return self.list + list(other)


    def set_fieldnames(self,fieldnames):
        self.fieldnames = fieldnames
        self.fieldpos = {}
        pos = 0
        for name in fieldnames:
            self.fieldpos[name] = pos
            pos += 1

    def make_private_copy_of_fields(self):
        if self.fieldnames:
            self.set_fieldnames(self.fieldnames[:])
        self.shared_fieldnames = False

    def append(self, val, name='?'):
        if self.shared_fieldnames:
            self.make_private_copy_of_fields()
        self.list.append(val)
        if self.fieldnames:
            self.fieldpos[name] = len(self.fieldnames)
            self.fieldnames.append(name)

    def as_dict(self):
        return dict(zip(self.fieldnames, self.list))


class Table:
    """Subclasses should call self.set_fieldnames(fieldnames) with an appropriate list of fieldnames
    And they should implement:
      getRow(self,i) (no need to perform bound checks for i)
      __len__(self)
      """

    def set_fieldnames(self,fieldnames):
        self.fieldnames = fieldnames
        self.fieldpos = {}
        pos = 0
        for name in fieldnames:
            self.fieldpos[name] = pos
            pos += 1

    def colname(self,i):
        try:
            return self.fieldnames[i]
        except:
            return str(i)

    def get_column(self,c):
        return [r[c] for r in self]

    def set_rownames(self,rownames):
        self.rownames = rownames

    def rowname(self,i):
        try:
            return self.rownames[i]
        except:
            return str(i)

    def set_title(self,title):
        self.title_ = title

    def title(self):
        try: return self.title_
        except: return ''            

    def set_filepath(self,filepath):
        self.filepath_ = filepath

    def filepath(self):
        try: return self.filepath_
        except: return ''

    def rename_fields(self, name_map, fieldname_must_exist = True):
        names = self.fieldnames[:]
        for oldname, newname in name_map.items():
            try:
                k = self.fieldpos[oldname]
                names[k] = newname
            except KeyError:
                if fieldname_must_exist:
                    raise ValueError('No field named '+oldname)
        self.set_fieldnames(names)

    def fieldnum(self,fieldname_or_num):
        try:
            return self.fieldpos[fieldname_or_num]
        except:
            pass
        return int(fieldname_or_num)
            
    def __getitem__(self,i):
        # print 'getitem',i
        if isinstance(i,slice):
            start, stop, step = i.start,i.stop,i.step
            if step!=None:
                raise IndexError('Extended slice with step not currently supported')
            l = self.__len__()
            if stop>l:
                stop = l
            return SelectRowRange(self,start,stop)
        else:
            l = self.__len__()
            if i<0: i+=l
            if i<0 or i>=l:
                raise IndexError('TableFile index out of range ('+str(i)+'/'+str(l)+')')
            row = ListWithFieldNames(self.getRow(i), self.fieldnames, self.fieldpos)
            row.rownum = i
            return row

    def length(self):
        return len(self)

    def width(self):
        return len(self.fieldnames)

    def __concat__(self,other):
        return VConcatTable([self,other])

    def close(self):
        pass

##     def __getslice__(self,start,stop):
##         print 'getsli
##         l = self.__len__()
##         if start<0: start+=l
##         if stop<0: stop+=l
##         if stop 
##         if i<0 or i>=l:

class StructTable(Table):
    """Table implemented as a StructFile.
    The struct format and the field names are saved in a secondary file
    with the same name as the data file, plus a '.format' extension.
    This file is executed when the table is openned for read ('r' or 'r+');
    it is created from the supplied field names and format when the table
    is created ('w+').
    """
    
    def __init__(self, fname, openmode='r', fieldnames=None, struct_format=None):
        self.set_fieldnames(fieldnames)
        self.fname = fname
        self.set_filepath(fname)
        self.closed = False
        if openmode=='r' or openmode=='r+':
            self.load_format()
        elif openmode=='w+':
            self.set_format(fieldnames, struct_format)
        else:
            raise ValueError('Invalid openmode '+openmode)
        
        self.struct = StructFile(fname,self.struct_format,openmode)

    def set_format(self, fieldnames, struct_format):
            if not isinstance(fieldnames,list) or not isinstance(struct_format,str):
                raise ValueError('You must specify the list of fieldnames and the struct_format string')
            self.set_fieldnames(fieldnames)
            self.struct_format = struct_format
            self.save_format()
            
    def load_format(self):
        vars = {}
        execfile(self.fname+'.format',vars)
        self.set_fieldnames(vars['fieldnames'])
        self.struct_format = vars['struct_format']

    def save_format(self):
        f = open(self.fname+'.format','wb')
        f.write('fieldnames = '+repr(self.fieldnames)+'\n\n')
        f.write('struct_format = '+repr(self.struct_format)+'\n\n')
        
    def getRow(self,i):
        return self.struct[i]

    def __len__(self):
        return len(self.struct)

    def append(self, x):
        self.struct.append(x)

    def close(self):
        if not self.closed:
            self.struct.close()        
            self.closed = True

    # Note: it's better for your code to call close() explicitly rather than rely on this being called automatically
    def __del__(self):
        self.close()

def compr_factor(m):
    s = '\n'.join([ '\t'.join(r) for r in m ])
    cs = zlib.compress(s)
    return float(len(s))/len(cs)

def compr_factor2(m):
    w = m.width()
    m = [ r for r in m ]
    uncompr_len = 0
    compr_len = 0
    for c in range(w):
        s = '\n'.join([r[c] for r in m])
        uncompr_len += len(s)
        compr_len += len(zlib.compress(s))
    return float(uncompr_len)/compr_len

def cf(m):
    return compr_factor(m), compr_factor2(m)


class CSVTable(Table):
    """Read-only for now"""
    
    def __init__(self, datafname, openmode="r"):
        if openmode!="r":
            raise ValueError("Currently only openmode=='r' is supported")
        
        self.datafname = datafname
        self.len = -1
        self.f = open(datafname,'rb')
        self.reader = csv.reader(self.f)
        self.i = -1
        self.row_i = self.reader.next()
        self.set_fieldnames(self.row_i)
        
    def getRow(self,i):
        if i<self.i:
            self.f.close()
            self.f = open(self.datafname,'rb')
            self.reader = csv.reader(self.f)
            self.i = -1
            self.row_i = self.reader.next()
        
        while self.i<i:
            self.i += 1
            self.row_i = self.reader.next()

        return self.row_i

    def __len__(self):
        if self.len<0:
            f = open(self.datafname,'rb')
            tmpreader = csv.reader(f)
            self.len = -1
            for line in tmpreader:                
                self.len += 1
            f.close()
        return self.len


class CompressedTableFile(Table):
    """
    Main file format:
    header: PLTABLE <version> <nrows_per_chunk> \n
    fieldnames_row
    chunks of data
    
    Each chunk of data contains the zlib compressed representation of at most nrows_per_chunk \n separated rows.
    
    There is also an associated index file containing 8-byte integers.
    Integer #i gives the byteindex of chunk #i.
    The last integer gives the number of rows in the last chunk.
    """
    
    def __init__(self, datafname, openmode='r', fieldnames=None, nrows_per_chunk=100):
        self.openmode = openmode
        indexfname = datafname+'.idx'
        self.set_filepath(datafname)
        self.closed = False
        self.final_chunk_rows = []
        
        if openmode=='r':
            self.dataf = open(datafname,'rb')
            self.index = StructFile(indexfname,'!Q','r')
            self.read_header_and_fieldnames()
            self.cached_chunk_rows = []
            self.cached_chunknum = -1
        elif openmode=='a' or openmode=='r+':
            self.dataf = open(datafname,'r+b')
            self.index = StructFile(indexfname,'!Q','r+')
            self.read_header_and_fieldnames()
            if self.nrows_in_last_chunk==self.nrows_per_chunk or self.nrows_in_last_chunk==0:
                self.final_chunk_rows = []
            else:
                self.final_chunk_rows = self.read_chunk_rows(self.last_chunknum())
        elif openmode=='w' or openmode=='w+': 
            self.nrows_per_chunk = nrows_per_chunk
            self.set_fieldnames(fieldnames)
            self.dataf = open(datafname,'w+b')
            self.index = StructFile(indexfname,'!Q','w+')
            self.write_header_and_fieldnames()
            self.final_chunk_rows = []
            self.nrows_in_last_chunk = self.nrows_per_chunk
        else:
            raise ValueError('Invalid openmode: '+openmode)

    def read_header_and_fieldnames(self):
        self.dataf.seek(0)
        headerline = self.dataf.readline()
        headcode, version, nrows = headerline.split()
        if headcode!='PLTABLE' or version!='01':
            raise TypeError('Invalid header or version'+headcode+' '+version)
        self.nrows_per_chunk = int(nrows)
        fieldnamesline = self.dataf.readline()
        self.set_fieldnames(fieldnamesline.split())
        self.nrows_in_last_chunk = int(self.index[-1])
        self._len = self.last_chunknum()*self.nrows_per_chunk + self.nrows_in_last_chunk
        self.dataf.seek(0,2)   # seek to end of file

    def last_chunknum(self):
        return len(self.index)-3

    def write_header_and_fieldnames(self):
        self.dataf.seek(0)
        self.dataf.write('PLTABLE 01\t'+str(self.nrows_per_chunk)+'\n')
        self.dataf.write('\t'.join(self.fieldnames)+'\n')
        self.dataf.flush()
        self._len = 0
        self.index.append(self.dataf.tell())
        self.index.append(0)
        self.index.flush()

    def compress(self, rows):
        return zlib.compress('\n'.join(rows))
        # return '\n'.join(rows)

    def decompress(self, encodedstring):
        chunk = zlib.decompress(encodedstring)
        # chunk = encodedstring
        rows = chunk.split('\n')
        return rows
    
    def read_chunk_rows(self, chunknum):
        if chunknum>self.last_chunknum():
            raise IndexError('chunk out of range: '+str(chunknum))
        startpos = self.index[chunknum]
        endpos = self.index[chunknum+1]
        self.dataf.seek(startpos)
        encodedchunk = self.dataf.read(endpos-startpos)
        rows = self.decompress(encodedchunk)
        return rows

    def get_chunk_rows(self, chunknum):
        if chunknum!=self.cached_chunknum:
            self.cached_chunk_rows = self.read_chunk_rows(chunknum)
            self.cached_chunknum = chunknum
        return self.cached_chunk_rows
        
    def getRow(self,i):
        if self.openmode!='r':
            raise IOError("Can only getRow if in 'r' openmode, not in "+repr(self.openmode)+" mode.")
        chunknum,ii = divmod(i,self.nrows_per_chunk)
        line = self.get_chunk_rows(chunknum)[int(ii)]
        elements = line.split('\t')
        return elements

    def flush(self):
        if self.openmode!='r' and self.final_chunk_rows:
            encodedchunk = self.compress(self.final_chunk_rows)
            # print 'Flushing with nrows_in_last_chunk = ',self.nrows_in_last_chunk,' and self.final_chunk_rows=',repr(self.final_chunk_rows)
            if self.nrows_in_last_chunk<self.nrows_per_chunk: # some more room in last chunk
                # print 'Rewriting last chunk'
                self.dataf.seek(self.index[self.last_chunknum()]) # seek to beginning of existing last chunk
                self.dataf.write(encodedchunk)
                self.index[-2] = self.dataf.tell()
                self.nrows_in_last_chunk = len(self.final_chunk_rows)
                self.index[-1] = self.nrows_in_last_chunk
            else: # append new chunk
                # print 'Appending new chunk'
                self.dataf.seek(0,2) # seek to end of file
                self.dataf.write(encodedchunk)
                self.index[-1] = self.dataf.tell()
                self.nrows_in_last_chunk = len(self.final_chunk_rows)
                self.index.append(self.nrows_in_last_chunk)
            if len(self.final_chunk_rows)==self.nrows_per_chunk:
                self.final_chunk_rows = []
            self.dataf.flush()
            self.index.flush()    

    def close(self):
        if not self.closed:
            self.flush()
            self.dataf.close()
            self.index.close()
            self.closed = True

    def append(self, row):
        if self.openmode not in ['w','a','w+','r+']:
            raise IOError("Can only append if in 'w','w+','r+' or 'a' openmode, not in "+repr(self.openmode)+" mode.")
        if len(row)!=self.width():
            raise ValueError('length of row does not match table width')
        self.final_chunk_rows.append('\t'.join([ str(elem).strip() for elem in row]))

        if len(self.final_chunk_rows)==self.nrows_per_chunk:
            self.flush()
        self._len += 1

    # Note: it's better for your code to call close() explicitly rather than rely on this being called automatically
    def __del__(self):
        self.close()

    def __len__(self):
        return int(self._len)


        

class MemoryTable(Table):
    """Table saved in RAM.
    This is a simple list of lists of fields; field names must be supplied to
    the constructor.
    """

    def __init__(self, fieldnames, data=None):
        self.set_fieldnames(fieldnames)
        if data==None:
            self.data = []
        else:
            self.data = data
        
    def append(self,elements):
        if len(elements)!=len(self.fieldnames):
            raise ValueError("elements (len="+str(len(elements))+") must be a vector of same length as fieldnames (len="+str(len(self.fieldnames))+")")
        self.data.append(elements)
    
    def getRow(self,i):
        return self.data[i][:]

    def __len__(self):
        return len(self.data)

def memorize(table):
    """Loads a table in memory: fetches all rows from table into a
    MemoryTable and returns that object.
    """
    mtable = MemoryTable(table.fieldnames)
    for row in table:
        mtable.append(row)
    return mtable


class PMatTable(Table):
    """Matrix of reals saved on disk in binary format.
    The data file starts with a 64 bytes header that indicates:
    - the number of rows in the table
    - the number of fields in a row
    - the data type (float or double)
    - the endianness of the data
    The rest is the data itself, in a fixed width format.
    Other data is saved in a subdirectory with the same name as the
    data file, plus a '.metadata' extension.  For example, field names
    are saved in '<table_name>.metadata/fieldnames', one name per line.
    """
    def __init__(self, fname, openmode='r', fieldnames=None):
        self.fname = fname
        self.set_filepath(fname)
        if openmode=='r':
            self.f = open(fname,'rb')
            self.read_and_parse_header()
            self.set_fieldnames(self.determine_fieldnames())
        else:
            raise ValueError("Currently only supported openmode is 'r'."+repr(openmode)+" is not supported")

    def read_and_parse_header(self):        
            header = self.f.read(64)
            mat_type, l, w, data_type, endianness = header.split()
            if mat_type!='MATRIX':
                raise ValueError('Invalid file header (should start with MATRIX)')
            self.len = int(l)
            self.w = int(w)
            if endianness=='LITTLE_ENDIAN':              
                struct_format = '<'
            elif endianness=='BIG_ENDIAN':
                struct_format = '>'
            else:
                raise ValueError('Invalid endianness in file header: '+endianness)

            if data_type=='DOUBLE':
                struct_format += 'd'*self.w
            elif data_type=='FLOAT':
                struct_format += 'f'*self.w
                
            self.struct_format = struct_format
            self.struct_size = struct.calcsize(struct_format)

    def determine_fieldnames(self):
        fieldnames = []
        fieldnamefile = os.path.join(self.fname+'.metadata','fieldnames')
        if os.path.isfile(fieldnamefile):
            for row in open(fieldnamefile):
                row = row.split()
                if len(row)>0:
                    fieldnames.append(row[0])
        else:
            fieldnames = map(str,range(self.w))
        return fieldnames

    def getRow(self,i):
        self.f.seek(64+i*self.struct_size)
        x = struct.unpack(self.struct_format,self.f.read(self.struct_size))
        return list(x)

    def __len__(self):
        return int(self.len)



class TableFile(Table):
    """Table saved as a tab separated text file (variable width records.)
    An index giving the offset to each record is saved in a file with the same
    name as the data file, plus a '.idx' extension; this index is a simple list
    of 64 bit offsets in binary format.
    The first line of the data file contains the field names, separated by
    tabs; these must be supplied when the file is created ('w+').
    """

    def __init__(self, fname, openmode='r', fieldnames=None,
                 tolerate_different_field_count=True, separator='\t'):
        self.separator= separator
        self.struct_format = '<Q'
        self.fname = fname
        self.set_filepath(fname)
        self.closed = False
        self.openmode = openmode
        self.tolerate_different_field_count = tolerate_different_field_count
        self.last_call_was_append = False
       
        indexfname = self.fname+'.idx'
        if openmode=='r':
            self.f = open(fname,'r')
            fieldnames = self.f.readline().strip('\r\n').split(self.separator)
            self.set_fieldnames(fieldnames)

            if os.path.isfile(indexfname) and os.path.getmtime(self.fname)>os.path.getmtime(indexfname):
                os.remove(indexfname)
                
            if not os.path.isfile(indexfname):
                build_row_index_file(fname,indexfname,self.struct_format)

            self.index = StructFile(indexfname,self.struct_format,'r')
        elif openmode=='r+':
            self.f = open(fname,'r+')
            fieldnames = self.f.readline().strip('\r\n').split(self.separator)
            self.set_fieldnames(fieldnames)
            if not os.path.isfile(indexfname):
                build_row_index_file(fname,indexfname)
            self.index = StructFile(indexfname,self.struct_format,'r+')
        elif openmode=='w+':
            self.f = open(fname,'w+')
            fieldnames = fieldnames[:]
            self.set_fieldnames(fieldnames)
            self.index = StructFile(indexfname,self.struct_format,'w+')
            self.append(fieldnames)
        else:
            raise ValueError("Invalid value for openmode ("+`openmode`+" Must be one of 'r' 'r+' or 'w+'")

            
    def append(self,elements):
        if len(elements)!=len(self.fieldnames):
            raise ValueError("elements must be a vector of same length as fieldnames")

        self.index.append(self.f.tell())
        
        if not self.last_call_was_append:
            self.f.seek(0,2) # seek to end of file

        self.f.write(self.separator.join(map(str,elements))+'\r\n')
        
        self.last_call_was_append = True

    def flush(self):
        if self.openmode!='r':
            self.f.flush()
            self.index.flush()

    def close(self):
        if not self.closed:
            self.flush()
            self.f.close()
            self.index.close()
            self.closed = True

    # Note: it's better for your code to call close() explicitly rather than rely on this being called automatically
    def __del__(self):
        self.close()

    def getRow(self,i):
        self.last_call_was_append = False
        
        self.f.seek(self.index[i+1])
        line = self.f.readline().strip('\r\n')
        elements = line.split(self.separator)
        if len(elements)!=len(self.fieldnames):
            if self.tolerate_different_field_count:
                if len(elements)<len(self.fieldnames):
                    elements += ['']*(len(self.fieldnames)-len(elements))
                else: # len(elements)>len(self.fieldnames)
                    elements = elements[0:len(self.fieldnames)]
            else:
                raise ValueError("At row "+str(i)+
                                 ", read only "+str(len(elements))+
                                 " while there are "+str(len(self.fieldnames))+
                                 " fieldnames")
        return map(string.strip, elements)

    def __len__(self):
        return len(self.index)-1


class SortWithinGroup(Table):
    """Creates a view from an existing table by grouping
    records and then sorting records within a group.  Grouping is made
    according to the supplied grouping fields: all consecutive records with
    the same values for all those fields will be part of the same group.  The
    group is then sorted according to the supplied sorting fields (ascending,
    in the same order the fields are given.)
    """
    def __init__(self, table, grouping_fields, sorting_fields):
        self.table = table
        self.set_fieldnames(table.fieldnames)
        self.grouping_fields = [self.fieldnum(f) for f in grouping_fields]
        self.sorting_fields = [self.fieldnum(f) for f in sorting_fields]
        self.cached_group_start = None
        self.cached_group_end = None
        self.cached_group = None
    
    def getRow(self,i):

        if self.cached_group and i>=self.cached_group_start and i<self.cached_group_end:
            return self.cached_group[i-self.cached_group_start]

        row_i = self.table[i]
        groupkeys = [ row_i[k] for k in self.grouping_fields ]

        group = []
        pos = i-1
        while pos>=0:
            row = self.table[pos]
            if [ row[k] for k in self.grouping_fields ] != groupkeys:
                break
            group.append(row)
            pos -= 1
        
##         if pos>=0:
##             row = self.table[pos]
##         while(pos>=0 and [ row[k] for k in self.grouping_fields ] == groupkeys):
##             group.append(row)
##             pos -= 1
##             if pos>=0:
##                 row = self.table[pos]

        self.cached_group_start = pos+1
        group.reverse()
        group.append(row_i)

        l = len(self.table)
        pos = i+1
        while pos<l:
            row = self.table[pos]
            if [ row[k] for k in self.grouping_fields ] != groupkeys:
                break
            group.append(row)
            pos += 1
            
        
##         row = self.table[pos]
##         while(pos<l and [ row[k] for k in self.grouping_fields ] == groupkeys):
##             group.append(row)
##             pos += 1
##             print 'getting ',pos,l
##             if pos<l:
##                 row = self.table[pos]

        self.cached_group_end = pos

        def cmpfunc(x,y):
            xkeys = [ x[k] for k in self.sorting_fields ]
            ykeys = [ y[k] for k in self.sorting_fields ]
            return cmp(xkeys, ykeys)
        
        group.sort(cmpfunc)
        self.cached_group = group

        return list(self.cached_group[i-self.cached_group_start])

    def __len__(self):
        return len(self.table)


class SelectRowRange(Table):
    """Creates view of a table by extracting a range of rows from an
    existing table.
    """
    def __init__(self, table, start, stop):
        self.table = table
        self.start = start
        self.stop = stop
        try: self.set_fieldnames(table.fieldnames)
        except: pass

    def getRow(self,i):
        #return self.table[self.start+i]
        return self.table.getRow(self.start+i)

    def __len__(self):
        return int(self.stop-self.start)


class VConcatTable(Table):
    """Creates a view by concatenating the rows of several
    existing tables.  All tables must have the same fields.
    """
    def __init__(self, tables):
        self.tables = tables
        self.set_fieldnames(tables[0].fieldnames)
        for t in tables:
            if t.fieldnames != self.fieldnames:
                print "got: ", t.fieldnames
                print "s/b: ", self.fieldnames
                raise RuntimeError('In VConcatTable fieldnames of the individual tables differ')

    def getRow(self,i):
        start = 0
        stop = 0
        for t in self.tables:
            stop += len(t)
            if i<stop:
                #return t[i-start]
                return t.getRow(i-start)
            start = stop
        raise IndexError('In VConcatTable: index out of range')

    def __len__(self):
        l = 0
        for t in self.tables:
            l += len(t)
        return l

class HConcatTable(Table):
    """Creates a view by concatenating the fields of several
    existing tables.  All tables must have the same number of rows.
    """
    def __init__(self, tables):
        self.tables = [] 
        fieldnames = []
        l = len(tables[0])
        for table in tables:
            if len(table)!=l:
                raise RuntimeError('In HConcatTable length of the individual tables differ')
            self.tables.append(table)
            fieldnames.extend(table.fieldnames)
        self.set_fieldnames(fieldnames)

    def getRow(self,i):
        row = []
        for table in self.tables:
            #row.extend(table[i])
            row.extend(table.getRow(i))
        return row

    def __len__(self):
        return len(self.tables[0])
        
class SelectRows(Table):
    """Creates a view by selecting specific rows from an
    existing table, in the given order.
    """
    def __init__(self, table, indexes):
        self.table = table
        self.indexes = indexes
        #///***///***///***
        # ARGHHH!!! FIXME:
        try: self.set_fieldnames(table.fieldnames)
        except: pass

    def getRow(self,i):
        #return self.table[self.indexes[i]]
        #///***///***///***
        # TODO: make sure this is OK (not that ^)
        return self.table.getRow(self.indexes[i])

    def __len__(self):
        return len(self.indexes)

class UpsideDownTable(Table):
    """Creates a view of a table with rows in reverse order"""
    def __init__(self, table):
        self.table = table
        try: self.set_fieldnames(table.fieldnames)
        except: pass

    def getRow(self,i):
        #return self.table[len(self.table)-1-i]
        return self.table.getRow(len(self.table)-1-i)

    def __len__(self):
        return len(self.table)

class AddFieldsTable(Table):
    def __init__(self, table, extra_fields):
        self.table= table
        self.extra_fields= extra_fields
        self.set_fieldnames(table.fieldnames + extra_fields)

        
    def getRow(self,i):
        return self.table.getRow(i) + ['' for f in self.extra_fields]
        
    def __len__(self):
        return len(self.table)
    

class SelectFields(Table):
    """Returns a view of a table by selecting only some of the fields.
    """

    def __init__(self, table, selected_fields, newnames=[], must_exist=True):
        """Returns a view of table with only the selected_fields (in that order).
        selected_fields is a list of fieldnames or fieldpositions in the original table.
        The fields can optionally be renamed by giving a non-empty list of newnames
        (which must be the same length as the selected_fields list)"""
        
        if len(newnames)==0:
            newnames = selected_fields
        elif len(newnames)!= len(selected_fields):
            raise ValueError('In SelectFields invalid specification of newnames len(newnames) is not 0 and is different from len(selected_fields)')

        self.table = table
        self.fieldnums = []
        fieldnames = []
        for field,newname in zip(selected_fields,newnames):
            if isinstance(field,int):
                self.fieldnums.append(field)
                fieldnames.append(table.fieldnames[field])
            else:
                try: pos = table.fieldnames.index(field)
                except ValueError: pos = -1
                if pos>=0:
                    self.fieldnums.append(pos)
                    fieldnames.append(newname)
                elif must_exist:
                    raise ValueError('In SelectFields invalid field specification: '+repr(field)+'\n Fields are:'+repr(table.fieldnames))
        self.set_fieldnames(fieldnames)

    def getRow(self,i):
        row = self.table[i]
        return [ row[field] for field in self.fieldnums ]

    def __len__(self):
        return len(self.table)

def DeleteFields(table, deleted_fields):
    """Creates a SelectFields table by selecting all table fields _not_ listed
    in deleted_fields.
    """
    selected_fields = [ field for field in table.fieldnames if field not in deleted_fields ]
    return SelectFields(table,selected_fields)

def GeneratedTable(tablefname, generating_func, list_of_dependencies=[]):
    """Opens a table with dependencies, regenerating the table when necessary.
    The last modification time of the table is compared against the last
    modification time of each file listed in list_of_dependencies.  If some
    dependency was modified after the table, generating_func is called
    prior to openning the file.
    """
    try:
        target_mtime = os.path.getmtime(tablefname)
    except OSError:
        target_mtime = 0

    dep_mtime = 0
    for fname in list_of_dependencies:
        try:
            mtime = os.path.getmtime(fname)
        except OSError:
            mtime = 0
        if mtime>dep_mtime:
            dep_mtime = mtime

    if target_mtime==0 or dep_mtime>target_mtime:
        generating_func()

    return openTable(tablefname)

#     def getRow(self,i):
#         row = self.table.getRow
#         t = FieldValues(self.table.fieldnames, self.table[i])
#         t._rownum_ = i
#         self.processingfunc(t)
#         return [ getattr(t,fieldname) for fieldname in self.fieldnames ]
    
#     def __len__(self):
#         return len(self.table)


class ProcessFields(Table):
    """DEPRECATED -- use ProcessFields2 instead
    Creates a view by mapping a function on each row of an existing table.
    processingfunc must take a FieldValues object and modify it in place.
    newfieldnames is a list of names of fields to be added to the already
    existing fields; these should be created by processingfunc.
    """
    def __init__(self, table, processingfunc, newfieldnames=[]):
        self.table = table
        self.processingfunc = processingfunc
        fieldnames = table.fieldnames+newfieldnames
        self.set_fieldnames(fieldnames)

    def getRow(self,i):
        t = FieldValues(self.table.fieldnames, self.table[i])
        t._rownum_ = i
        self.processingfunc(t)
        return [ getattr(t,fieldname) for fieldname in self.fieldnames ]
    
    def __len__(self):
        return len(self.table)

class ProcessFields2(Table):
    """Creates a view by applying a function to each row of an existing table.
    processingfunc must work in place on a ListWithFieldNames object, to
    which it can add fields if autoappend is True.
    """
    def __init__(self, table, processingfunc, autoappend=False):
        self.table = table
        self.processingfunc = processingfunc
        self.autoappend = autoappend
        row = table[0]
        row.autoappend = self.autoappend
        processingfunc(row)
        self.set_fieldnames(row.fieldnames)

    def getRow(self,i):
        row = self.table[i]
        row.autoappend = self.autoappend
        self.processingfunc(row)
        return list(row)
    
    def __len__(self):
        return len(self.table)

class AppendRownum(Table):
    """Creates a view from an existing table by adding a field that contains
    the row number.
    """
    def __init__(self, table, rownum_fieldname='rownum'):
        self.table = table
        fieldnames = table.fieldnames+[rownum_fieldname]
        self.set_fieldnames(fieldnames)

    def getRow(self,i):
        #return self.table[i]+[i]
        return list(self.table.getRow(i))+[i]
    
    def __len__(self):
        return len(self.table)

class AppendConstantFields(Table):
    """Creates a view from an existing table by appending a list of fields with 
    constant values.
    """
    def __init__(self, table, extra_fieldnames, extra_fieldvals):
        self.table = table
        self.set_fieldnames(table.fieldnames+extra_fieldnames)
        self.extra_fieldvals = extra_fieldvals

    def getRow(self,i):
        #return self.table[i]+self.extra_fieldvals
        return self.table.getRow(i)+self.extra_fieldvals
    
    def __len__(self):
        return len(self.table)

class StrippedFieldsTable(Table):
    """Creates a view from an existing table by stripping all leading
    and trailing whitespace from text fields.  If stripquotes is true, double
    quotes and leading and trailing whitespace within those quotes will also
    be stripped.
    """
    def __init__(self, table, stripquotes=True):
        self.stripquotes = stripquotes
        self.table = table
        self.set_fieldnames(table.fieldnames)

    def mystrip(self,val):
        if isinstance(val,str):
            val = val.strip()
            if self.stripquotes and len(val)>=2 and val[0]=='"' and val[-1]=='"':
                val = val[1:-1].strip()
        return val
            

    def getRow(self,i):
        #return map(self.mystrip, self.table[i])
        return map(self.mystrip, self.table.getRow(i))
    
    def __len__(self):
        return len(self.table)


class CacheTable(Table):
    """Creates a direct view of an existing table and caches the last
    max_nrows_in_cache rows fetched in RAM.
    """
    def __init__(self, table, max_nrows_in_cache=200):
        self.table = table
        self.max_nrows_in_cache = 200
        self.set_fieldnames(table.fieldnames)
        self.cached_rownum = []
        self.cached_rows = []

    def getRow(self,i):

        try:
            k = self.cached_rownum.index(i)
            return self.cached_rows[k]
        except ValueError:
            row = self.table.getRow(i)
            if len(self.cached_rows)>=self.max_nrows_in_cache:
                del self.cached_rows[0]
                del self.cached_rownum[0]
            self.cached_rows.append(row)
            self.cached_rownum.append(i)
            return row
            
    def clearCache(self):
        self.cached_rownum = []
        self.cached_rows = []
    
    def __len__(self):
        return len(self.table)

class ShuffleTable(SelectRows):
    """Creates a view of an existing table with rows randomly permuted.
    If no seed is given, this will always generate the same permutation.
    """
    def __init__(self, table, seed_x=123, seed_y=456):
        random_array.seed(seed_x,seed_y)
        indexes = random_array.permutation(len(table))
        SelectRows.__init__(self, table, indexes)
        

class FilterTable(SelectRows):
    """DEPRECATED -- use FilterTable2 instead
    Creates a view of an existing table by selecting some of the rows
    according to boolfunc.  An indexfile can also be supplied to save/restore
    the indexes of rows selected by boolfunc.
    boolfunc must be a predicate that takes a FieldValues parameter.
    """
    def __init__(self, table, boolfunc, indexfile=""):
            
        if os.path.isfile(indexfile):
            indexes = StructFile(indexfile, '<L', 'r')
            
        else: # create indexes
            if indexfile=='':
                indexes = []
            else:
                indexes = StructFile(indexfile+'.tmp', '<L', 'w+')
            l = len(table)
            pbar = PBar('Filtering',l)
            for i in xrange(l):
                t = FieldValues(table.fieldnames, table[i])
                if boolfunc(t):
                    indexes.append(i)
                pbar.update(i)
            pbar.close()
            if indexfile!='':
                os.rename(indexfile+'.tmp',indexfile)

        SelectRows.__init__(self, table, indexes)


class FilterTable2(SelectRows):
    """Creates a view of an existing table by selecting some of the rows
    according to boolfunc.  An indexfile can also be supplied to save/restore
    the indexes of rows selected by boolfunc.
    boolfunc must be a predicate that takes a ListWithFieldNames parameter.
    """
    def __init__(self, table, boolfunc, indexfile=""):

        struct_fmt= '<L'
            
        if os.path.isfile(indexfile):
            indexes = StructFile(indexfile, struct_fmt, 'r')
            
        else: # create indexes
            if indexfile=='':
                indexes = []
            else:
                indexes = StructFile(indexfile+'.tmp', struct_fmt, 'w+')
            l = len(table)
            pbar = PBar('Filtering',l)
            for i in xrange(l):
                if boolfunc(table[i]):
                    indexes.append(i)
                pbar.update(i)
            pbar.close()
            if indexfile!='':
                indexes.close()
                os.rename(indexfile+'.tmp',indexfile)
                indexes= StructFile(indexfile,struct_fmt)

        SelectRows.__init__(self, table, indexes)

class BootstrapTable(SelectRows):
    """Creates a table by sampling (with replacement) 
       the rows of an original table.
       Unweighted version: tables (original and sample) are of same length.
       Weighted version: total weight of sample table is made close to the original's
    """
    def __init__(self, table, manualseed="", weightfield="", indexfile="", verbose=1):

        def binsearch(x,vec):
            vec = [0] + vec
            n = len(vec)
            il = 0
            ih = n-1
            while ih - il > 1: 
                im = int((ih+il)/2)
                if x > vec[im]:
                    il = im
                else:
                    ih = im
            return il
            
        struct_fmt= '<L'
            
        if os.path.isfile(indexfile):
            indexes = StructFile(indexfile, struct_fmt, 'r')
            
        else: # create indexes
            if indexfile=='':
                indexes = []
            else:
                indexes = StructFile(indexfile+'.tmp', struct_fmt, 'w+')
            l = len(table)
            if manualseed: seed(manualseed)
            if not weightfield:
                if verbose:
                    pb = PBar('Sampling table',l) 
                for i in range(l):
                    if verbose:
                        pb.update(i)
                    indexes.append(randint(0,l-1))
                if verbose:
                    pb.close()
            else:
                cumw = l*[0]
                totw = 0
                for i in range(l):
                    totw += table[i][weightfield]
                    cumw[i] = totw
                totw_sam = 0
                if verbose:
                    pb = PBar('Sampling table', int(totw))
                while totw_sam < totw:
                    if verbose:
                        pb.update(int(totw_sam))
                    cw = totw * uniform(0,1)
                    i = binsearch(cw,cumw)
                    indexes.append(i)
                    wi = table[i][weightfield]
                    totw_sam += wi
                if wi < 2*(totw_sam - totw):
                    indexes.pop()
                if verbose:
                    pb.close()
            if indexfile!='':
                indexes.close()
                os.rename(indexfile+'.tmp',indexfile)
                indexes= StructFile(indexfile,struct_fmt)

        SelectRows.__init__(self, table, indexes)

class SortTable(SelectRows):
    """Creates a view of an existing table by sorting the rows in
    ascending order of sortfields.  An index file name can be supplied
    to save/restore the index so that it does not need to be recalculated.
    """
    def __init__(self, table, sortfields, indexfile="", reverse=False, verbose=1):
            
        if os.path.isfile(indexfile):
            indexes = StructFile(indexfile, '<L', 'r')
            
        else: # create indexes
            sortfields = [ isinstance(field,int) and field or table.fieldnames.index(field) for field in sortfields ]
            keylist = []
            l = len(table)
            if verbose:
                pbar = PBar('Reading fields to be sorted',l)
            for i in xrange(l):
                row = table[i]
                keylist.append( [ row[field] for field in sortfields ]+[i] )
                if verbose:
                    pbar.update(i)
            keylist.sort(reverse=reverse)
            if indexfile=="":
                indexes = [ row[-1] for row in keylist ]
            else:
                indexes = StructFile(indexfile+'.tmp', '<L', 'w+')
                if verbose:
                    pbar = PBar('Writing sorted index',l)
                for row,i in zip(keylist,xrange(l)):
                    indexes.append(row[-1])
                    if verbose:
                        pbar.update(i)
                os.rename(indexfile+'.tmp',indexfile)

        SelectRows.__init__(self, table, indexes)


class EfficientSortTable(SelectRows):
    """Creates a view of an existing table by sorting rows in ascending order
    of a single numeric field.  Can be more memory efficient than SortTable.
    """
    def __init__(self, table, sortfield, sortfieldtype='d', indexfile=""):
            
        if os.path.isfile(indexfile):
            indexes = StructFile(indexfile, '<L', 'r')
            
        else: # create indexes
            if not isinstance(sortfield,int):
                sortfield = table.fieldnames.index(sortfield)

            l = len(table)
            keys = array(type=sortfieldtype, shape=(l,) )
            pbar = PBar('Reading fields to be sorted',l)
            for i in xrange(l):
                key = table[i][sortfield]
                keys[i] = float(key)
                pbar.update(i)

            indexarray = argsort(keys)
            if indexfile=="":
                indexes = indexarray
            else:
                indexes = StructFile(indexfile+'.tmp', '<L', 'w+')
                pbar = PBar('Writing sorted index',l)
                for pos in indexarray:
                    indexes.append(pos)
                    pbar.update(i)
                os.rename(indexfile+'.tmp',indexfile)

        SelectRows.__init__(self, table, indexes)


        

def group_by(table, selected_fields):
    """Creates a generator that acts like a list of sub tables, where the rows
    are grouped by field values of selected_fields to make each sub table.
    """
    fieldnums = []
    for field in selected_fields:
        try:
            if isinstance(field,int):
                fieldnums.append(field)
            else:
                fieldnums.append(table.fieldnames.index(field))
        except ValueError:
            raise ValueError('In table.group_by invalid field specification: '+repr(field)+'\n Fields are:'+repr(table.fieldnames))

    row = table[0]
    current_values = [ row[field] for field in fieldnums ]
    start = 0
    # print len(table)
    for stop in xrange(len(table)):
        # print stop
        row = table[stop]
        values = [ row[field] for field in fieldnums ]
        if values!=current_values:
            yield table[start:stop]
            current_values = values
            start = stop
    yield table[start:len(table)]


def float_or_NaN_if_empty(x):
    """Returns a float from another value, or NaN if the parameter is
    the empty string.
    """
    if x=='':
        return fpconst.NaN
    else:
        return float(x)


class FieldTypeStats:

    def __init__(self):
        self.n = 0
        self.n_missing_value = 0
        self.n_numerical_value = 0
        self.n_other_value = 0
        self.n_numerical_type = 0
        self.min_value = None
        self.max_value = None
                
    def __repr__(self):
        return 'FieldTypeStats(' + \
        'n ='+ repr(self.n) + \
        ', n_missing_value ='+ repr(self.n_missing_value) + \
        ', n_numerical_value ='+ repr(self.n_numerical_value) + \
        ', n_other_value ='+ repr(self.n_other_value) + \
        ', n_numerical_type ='+ repr(self.n_numerical_type) + \
        ', min_value ='+ repr(self.min_value) + \
        ', max_value ='+ repr(self.max_value) + ')'       

    def update(self, val):
        self.n += 1
        if isinstance(val, (int, long, float, bool)):
            self.n_numerical_type += 1
            
        if val is None or val=="":
            self.n_missing_value += 1
        else:
            try:
                numval = float(val)
                self.n_numerical_value +=1
                if self.min_value is None:
                    self.min_value = numval
                    self.max_value = numval
                else:
                    self.min_value = min(numval, self.min_value)
                    self.max_value = max(numval, self.max_value)
            except ValueError:
                self.n_other_value += 1

    def smap_start_id(self):
        if self.max_value is None:
            return 1
        else:
            return int(self.max_value+1)
        

class StringMapFile:

    def __init__(self, filepath, openmode='a', startid=1):
        if openmode not in "rwa":
            raise ValueError("openmode must be one of 'r','w','a'")
        self.startid = startid
        self.maxid = None
        self.filepath = filepath
        self.openmode = openmode
        self.map = {}
        if openmode=='r':
            self.load_map()
        elif openmode=='w':
            self.f = open(filepath,'w')
        elif openmode=='a':
            if os.path.exists(filepath):
                self.load_map()
            self.f = open(filepath,'a')

    def load_map(self):
        f = open(self.filepath,'r')
        for row in f:
            pos = row.rfind(' ')
            strval = row[1:pos-1]
            numid = float(row[pos+1:])
            self.map[strval] = numid
            if self.maxid is None:
                self.maxid = numid
            else:
                self.maxid = max(numid, self.maxid)
        f.close()

    def __getitem__(self,strval):
        return self.map[str(strval)]

    def __len__(self):
        return len(self.map)

    def append(self, strval, numid=None):
        strval = str(strval) # make sure it's a string
        if numid is None:
            if self.maxid is None:
                self.maxid = self.startid
            else:
                self.maxid += 1
            numid = self.maxid
        # self.f.seek(0,2)
        print >> self.f, '"'+strval.replace('"','\\"')+'"', numid
        self.map[strval] = numid
        self.flush()
        return numid
            
    def close(self):
        self.f.close()
        
    def flush(self):
        self.f.flush()

def computeFieldTypeStats(table, show_progress=True):
    w = table.width()
    stats = []
    for j in xrange(w):
        stats.append(FieldTypeStats())
    if show_progress:
        pbar = PBar('Computing stats',len(table))
    i = 0
    for row in table:
        for j in xrange(w):
            stats[j].update(row[j])
        if show_progress:
            pbar.update(i)
        i += 1
    if show_progress:
        pbar.close()

    return stats

def saveFieldTypeStats(stats, fieldnames, statsfname):
    f = open(statsfname,'w')
    for fieldname,stat in zip(fieldnames,stats):
        print >> f, fieldname, ': ', stat
    f.close()

def saveTableAsVMAT(table, vmat_name, stringmap_reference_dir="", show_progress=True, overwrite=True):
    ext = os.path.splitext(vmat_name)[1]
    assert ext in ['.dmat', '.pmat']

    ## What should we do if the VMat aleardy exists?
    if os.path.exists(vmat_name):
        ## Erase VMat
        if overwrite:
            vmat_metadata = vmat_name + '.metadata'
            rm_cmd = 'rm -fr %s %s' %(vmat_name, vmat_metadata)
            subprocess.Popen(rm_cmd, shell=True, bufsize=0, stdout=subprocess.PIPE).stdout.read()
        else:
            raise RuntimeError, 'File or directory path "%s" already exists.' %vmat_name

    if isinstance(table, str):
        table = openTable(table)
    width = table.width()

    if ext == '.pmat':
        vmat = FileVMatrix(filename=vmat_name, width=width, length=0)
    elif ext == '.dmat':
        vmat = DiskVMatrix(dirname=vmat_name, writable=True, width=width, length=0)
    print 'len(table) = ', len(table)
    print 'table.fieldnames = ', table.fieldnames
    print 'vmat_name = ', vmat_name
    print 'vmat = ', vmat
    vmat.declareFieldNames(table.fieldnames)
    vmat.saveFieldInfos()

    if stringmap_reference_dir=="":
        if show_progress:
            i = 0
            pbar = PBar('Saving '+vmat_name, len(table))
        for row in table:
            vmat.appendRow( [ float_or_NaN_if_empty(e) for e in row ] )
            if show_progress:
                pbar.update(i)
                i += 1
        if show_progress:
            pbar.close()

    else: # we have a reference directory from which to retrieve/store stringmaps
        refdir = os.path.join(stringmap_reference_dir,"mappings")
        if not os.path.isdir(refdir):
            os.makedirs(refdir)

        logdir = os.path.join(stringmap_reference_dir,"logdir")
        logdir = os.path.join(logdir, time.strftime('%Y-%m-%d_%H:%M:%S'))
        if not os.path.isdir(logdir):
            os.makedirs(logdir)
        statsfname = os.path.join(logdir,'stats.txt')

        # get and save field type stats
        stats = computeFieldTypeStats(table, show_progress)
        saveFieldTypeStats(stats, table.fieldnames, statsfname)
        
        stringmaps = {}
        extramappings = {}

        # now loop over the rows and save as pmat, completing the mapping if needed
        if show_progress:
            i = 0
            pbar = PBar('Saving '+vmat_name, len(table))
        for row in table:
            numrow = []
            for j in xrange(width):
                val = row[j]
                if val is None or val=="":
                    numrow.append(fpconst.NaN)
                else:
                    try:
                        numrow.append(float(val))
                    except ValueError:
                        strval = str(val)
                        fieldname = table.fieldnames[j]
                        try:
                            smap = stringmaps[fieldname]
                        except KeyError:                            
                            smap = StringMapFile(os.path.join(refdir,fieldname+".smap"),"a",stats[j].smap_start_id())
                            stringmaps[fieldname] = smap
                        try:
                            numval = smap[strval]
                        except KeyError:
                            numval = smap.append(strval)
                            try:
                                extramap = extramappings[fieldname]
                            except KeyError:
                                extramap = StringMapFile(os.path.join(logdir,fieldname+".smap"),"w")
                                extramappings[fieldname] = extramap
                            extramap.append(strval,numval)
                        numrow.append(numval)
                        
            vmat.appendRow(numrow)
            if show_progress:
                pbar.update(i)
                i += 1
        if show_progress:
            pbar.close()

        # now close the maps
        for map in stringmaps.values():
            map.close()
        for map in extramappings.values():
            map.close()

        # Create a symbolic link from vmat_name.metadata/FieldInfo -> refdir
        if show_progress:
            inmetadata = os.path.join(vmat_name+'.metadata', 'FieldInfo')
            absrefdir = os.path.abspath(refdir)
            print ">>> Creating symbolic link: ",inmetadata,'->',absrefdir
        os.symlink(absrefdir, inmetadata)

        if show_progress:
            print ">>> If any, extra added mappings have been written in", logdir
            print "    You should consult this directory to check if there's anything suspect."
            print "    (Did you expect any new previously unseen strings?)"
            print

    ## Close vmat file
    vmat.flush()
    del(vmat)
            
def saveTable(table, fname, show_progress=True, stringmap_reference_dir=""):
    """Saves a table to disk in the format specified by the extension of fname.
    Can be one of:
    - .pmat : PLearn matrix: table of reals in binary format
    - .ptab : table compressed in chunks using zlib
    - .ptabdir : TBD! do not use!
    - .txt : tab separated text file
    table can be a Table object or the name of the file containing the table.
    When the extension is .pmat, stringmap_reference_dir could be used for string mapping.
    """
    if isinstance(table,str):
        table = openTable(table)
    if fname.endswith('.pmat') or fname.endswith('.dmat'):
        saveTableAsVMAT(table, fname, stringmap_reference_dir, show_progress)
    else:
        if fname.endswith('.ptab'):
            m = CompressedTableFile(fname, 'w', table.fieldnames)
        elif fname.endswith('.ptabdir'):
            m = TableDir(fname, 'w+', table.fieldnames)
        elif fname.endswith('.txt'):
            m = TableFile(fname, 'w+', table.fieldnames)
        else:
            raise ValueError('Invalid extension for destination file '+fname)

        if show_progress:
            pbar = PBar('Saving '+fname,len(table))
        i = 0
        for row in table:
            m.append( [ str(e) for e in row ] )
            if show_progress:
                pbar.update(i)
            i += 1
        m.close()
        if show_progress:
            pbar.close()


class VMatTable(Table):
    """
    Table that wraps a PLearn VMatrix
    """
    __buflen= 256
    
    def __init__(self, vmat, get_strings=False):
        self.vmat= vmat
        self.set_fieldnames(vmat.fieldNames())
        self.buf= None
        self.bufstart= -self.__buflen
        self.get_strings= get_strings

    def getRow(self, i):
        if self.get_strings:
            r= []
            for j in range(self.width()):
                r+= [self.vmat.getString(i,j)]
            return r
        if i < self.bufstart or i >= self.bufstart+self.__buflen:
            buflen= min(self.__buflen, len(self)-i)
            self.buf= self.vmat.subMat(i, 0, buflen, self.width()).getMat()
            self.bufstart= i
        return self.buf[i-self.bufstart]

    def __len__(self):
        return int(self.vmat.length)


class H5Table(Table):
    """
    Table from an HDF5 array
    """
    def __init__(self, filename, arrayname, fieldnames=None, mode='r'):
        import tables
        if mode!='r': raise NotImplementedError
        self.file= tables.openFile(filename)
        self.array= self.file.getNode(arrayname)
        if fieldnames:
            self.set_fieldnames(fieldnames)
        else:
            self.set_fieldnames(['f'+str(i) for i in range(len(self.array[0]))])
        
    def getRow(self, i):
        return self.array[i]

    def __len__(self):
        return len(self.array)

class AMATTable(TableFile):
    """
    read-only for now
    """
    def __init__(self, fname, openmode):
        self.struct_format = '<Q'
        self.fname = fname
        self.set_filepath(fname)
        self.closed = False
        self.openmode = openmode
        self.tolerate_different_field_count = True
        self.separator= None
        indexfname = self.fname+'.idx'
        if openmode=='r':
            self.f = open(fname,'r')
            l= self.f.readline().strip('\r\n').split(self.separator)
            n= 0
            while l[0] != '#:':
                l= self.f.readline().strip('\r\n').split(self.separator)
                n+= 1
            fieldnames = l[1:]
            fieldnames = [f for f in fieldnames if f != '']
            self.set_fieldnames(fieldnames)

            if not os.path.isfile(indexfname):
                build_row_index_file(fname,indexfname,self.struct_format, n)
            self.index = StructFile(indexfname,self.struct_format,'r')
        else:
            raise ValueError("Invalid value for openmode ("+`openmode`+" Must be 'r'")


class JoinTable(Table):
    def __init__(self, tables, key_fields, indexfname= None):
        ntables= len(tables)
        assert(ntables >= 2)
        self.tables= tables #should already be sorted by key_fields!
        self.key_fields= key_fields

        # take all fieldnames from all files
        fieldnames= []
        nextid= 1 # to differentiate dup. field names
        for t in tables:
            for f in t.fieldnames:
                while f in fieldnames: # add suffix if already there
                    f+= '-'+str(nextid)
                    nextid+= 1
                fieldnames.append(f)
        self.set_fieldnames(fieldnames)

        idxstructformat= '!'+'Q'*ntables
        if indexfname and os.path.isfile(indexfname):
            self.index= StructFile(indexfname, idxstructformat, 'r')
        else: # build index
            rowidx= [0]*ntables
            index= []
            pbar= PBar('Building Join Index', len(tables[0]))
            while rowidx[0] < len(tables[0]):
                match= True
                key= [tables[0][rowidx[0]][f] for f in key_fields]
                for i in range(1,ntables):
                    while rowidx[i] < len(tables[i]) and key > [tables[i][rowidx[i]][f] for f in key_fields]:
                        rowidx[i]+= 1 #try to find a match
                    if rowidx[i] == len(tables[i]) or key != [tables[i][rowidx[i]][f] for f in key_fields]:
                        match= False
                        break
                if match: index.append(copy.copy(rowidx))
                rowidx[0]+= 1
                pbar.update(rowidx[0])
            del pbar
            self.index= index
            if indexfname and not os.path.isfile(indexfname):
                indexfile= StructFile(indexfname, idxstructformat, 'w+')
                for i in index: indexfile.append(i)

    def getRow(self, i):
        rowidx= self.index[i]
        row= []
        for i,t in zip(rowidx, self.tables):
            row+= t[i]
        return row

    def __len__(self):
        return len(self.index)


def openTable(tablespec,openmode='r'):
    """Opens a file containing a representation of a Table object.
    The format of the file is determined by its extension like for saveTable,
    but the '.pytable' type is also supported; this is a file containing
    Python code to create the table.
    openmode is used only for CompressedTableFile,PMatTable,TableDir and
    TableFile
    """
    
    if isinstance(tablespec,str):
        if tablespec.endswith('.pytable'):
            olddir = os.getcwd()
            try:
                dirname, filename = os.path.split(os.path.abspath(tablespec))
                os.chdir(dirname)
                vars = {}
                execfile(filename, vars)
            finally:
                os.chdir(olddir)
            table = vars['result']
            table.set_filepath(tablespec)
            return table
        elif tablespec.endswith('.ptab'):
            return CompressedTableFile(tablespec,openmode)
        elif tablespec.endswith('.csv'):
            return CSVTable(tablespec, openmode)
        elif tablespec.endswith('.pmat'):
            return PMatTable(tablespec,openmode)
        elif tablespec.endswith('.ptabdir'):
            return TableDir(tablespec,openmode)
        elif tablespec.endswith('.amat'):
            return AMATTable(tablespec,openmode)
        elif tablespec.endswith('.dmat') or tablespec.endswith('.vmat'):
            from plearn import pyext
            vm= pyext.AutoVMatrix(filename= tablespec)
            return VMatTable(vm)
        elif tablespec.endswith('.psave'):
            from plearn import pyext
            vm= pyext.loadObject(tablespec)
            return VMatTable(vm)
        else:
            return TableFile(tablespec,openmode)
    # otherwise, assume tablespec is already a Table
    return tablespec

# vim: filetype=python:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
