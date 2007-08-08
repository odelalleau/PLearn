## Automatically adapted for numpy.numarray Jun 13, 2007 by python_numarray_to_numpy (-xsm)

# serialize.py
# Copyright (C) 2005, 2006 Pascal Vincent, Christian Dorion
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


# Author: Pascal Vincent, Christian Dorion
"""Streams and PLearnIO stream handler.

Streams are built using the File objects' specifications
"""
import logging, sys, string
import numpy.numarray as numarray
from plearn.pyplearn import pl
from plearn.pyplearn.plearn_repr import plearn_repr, clear_ref_map
import plearn.utilities.pl_struct as struct  ## corrige un bug dans struct

RPOS = 0
WPOS = 1

SIGN_AND_DIGITS           = string.digits + "+-"
FLOAT_CHARS               = string.digits+"+-.eE" + "naif"   # "nan" and "inf"
BLANK_CHARS               = " \t\n\r"
BLANK_AND_SEPARATOR_CHARS = BLANK_CHARS+",;"
WORDSEPARATORS            = " \t\n\r()[]{};,:|#"

PL_TYPECODE_TO_ARRAYTYPE  = {
    '\x01': 'i1',
    '\x02': 'u2',
    '\x03': 'i2',
    '\x04': 'i2',
    '\x05': 'u2',
    '\x06': 'u2',
    '\x07': 'i4',
    '\x08': 'i4',
    '\x0B': 'u4',
    '\x0C': 'u4',
    '\x0E': 'f4',
    '\x0F': 'f4',
    '\x10': 'f8',
    '\x11': 'f8'
    }


class PLearnIO:

    def __init__(self, pin, pout):
        self.pin = pin
        self.pout = pout
        self.unreadstring = ''
        self.copies_map_in = {}
        # self.copies_map_out = {}
        self.return_lists = False # set it to true if you want binread of sequences to return lista rather than arrays

    def clear_maps(self):
        self.copies_map_in.clear()
        # self.copies_map_out.clear()
        clear_ref_map()

    def read(self,size):
        result = self.unreadstring[0:size]
        nleft = size-len(self.unreadstring)
        self.unreadstring = self.unreadstring[size:]
        if nleft>0:
            result += self.pin.read(nleft)
        return result

    def readline(self):
        s = ''
        c = self.get()
        while c not in '\n\r':
            s += c
            c = self.get()
        s += c
        return s

    def get(self):
        return self.read(1)

    def peek(self):
        c = self.read(1)
        self.unread(c)
        return c

    def unread(self,s):
        self.unreadstring = s+self.unreadstring

    def write(self,str):
        self.pout.write(str)

    def flush(self):
        self.pout.flush()

    def close(self):
        self.pout.close()
        self.pin.close()
        
    def skip_blanks(self):
        c = ' '
        while c in BLANK_CHARS:
            c = self.get()
        self.unread(c)

    def skip_rest_of_line(self):
        c = self.get()
        while c!='\n':
            c = self.get()

    def skip_blanks_and_comments(self):
        c = ' '
        while True:
            if c=='#':
                self.skip_rest_of_line()
            elif c not in BLANK_CHARS:
                self.unread(c)
                return
            c = self.get()
        
    def skip_blanks_and_comments_and_separators(self):
        c = ' '
        while True:
            if c=='#':
                self.skip_rest_of_line()
            elif c not in BLANK_AND_SEPARATOR_CHARS:
                self.unread(c)
                return
            c = self.get()

    def read_string(self):
        s = ''
        self.skip_blanks_and_comments()
        c = self.peek()
        if c=='"': # quoted string
            self.get() # skip the quote
            c = self.get()
            while c!='"':
                if c=='\\':
                    c = self.get()
                    if c=='n':
                        s += '\n'
                    else:
                        s += c
                else:
                    s += c
                c = self.get()
            return s

        else: # unquoted string
            return self.read_word()

    def read_word(self):
        s = ''
        c = self.get()
        while c not in WORDSEPARATORS:
            s += c
            c = self.get()
        self.unread(c)
        return s
       

    def read_int(self):
        s = ''
        self.skip_blanks_and_comments()
        c = self.get()
        if ord(c) in [0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0B, 0x0C]:
            self.unread(c)
            return self.binread()
        
        while c in SIGN_AND_DIGITS:
            s += c
            c = self.get()
        self.unread(c)
        return int(s)

    def read_float(self):
        s = ''
        self.skip_blanks_and_comments()
        c = self.get()
        while c in FLOAT_CHARS:
            s += c
            c = self.get()
        self.unread(c)
        print >>sys.stderr, "read_float:", s
        return float(s)

            
    def write_typed(self, x):
        spec = plearn_repr(x)+' '
        self.write(spec)

    def binread(self):
        self.skip_blanks_and_comments_and_separators()
        c = self.get()

        if   c=="\x01": # signed char
            return self.get()
        elif c=="\x02": # unsigned char
            return self.get()

        elif c=="\x03": # short little endian
            return struct.unpack('<h',self.read(2))[0]
        elif c=="\x04": # short big endian
            return struct.unpack('>h',self.read(2))[0]
        elif c=="\x05": # unsigned short little endian
            return struct.unpack('<H',self.read(2))[0]
        elif c=="\x06": # unsigned short big endian
            return struct.unpack('>H',self.read(2))[0]
        
        elif c=="\x07": # 32 bit int little endian
            return struct.unpack('<i',self.read(4))[0]
        elif c=="\x08": # 32 bit int big endian
            return struct.unpack('>i',self.read(4))[0]
        elif c=="\x0B": # unsigned int little endian
            return struct.unpack('<I',self.read(4))[0]
        elif c=="\x0C": # unsigned int big endian
            return struct.unpack('>I',self.read(4))[0]

        elif c=="\x0E": # float little endian
            return struct.unpack('<f',self.read(4))[0]
        elif c=="\x0F": # float big endian
            return struct.unpack('>f',self.read(4))[0]

        elif c=="\x10": # double little endian
            return struct.unpack('<d',self.read(8))[0]
        elif c=="\x11": # double big endian
            return struct.unpack('>d',self.read(8))[0]

        elif c in "\x12\x13\x14\x15": # 1D or 2D sequence
            self.unread(c)
            return self.binread_sequence()

        elif c=="\x16": # 64 bit int little endian
            return struct.unpack('<q',self.read(8))[0]
        elif c=="\x17": # 64 bit int big endian
            return struct.unpack('>q',self.read(8))[0]

        #elif c=="\x16": # DEPRECATED binary pair format (for backward compatibility)
        #    self.unread(c)
        #    return self.binread_pair()

        elif c=='"': # string
            self.unread(c)
            return self.read_string()

        elif c=='*': # pointer
            self.unread(c)
            return self.binread_pointer()

        elif c=='{': # dictionary
            self.unread(c)
            return self.binread_dict()

        elif c=='(': # tuple
            self.unread(c)
            return self.binread_tuple()
        
        elif c=='0': # boolean
            return False
        elif c=='1':
            return True

        elif c in string.ascii_letters: # object name
            self.unread(c)
            return self.binread_object()

        else:
            raise TypeError("binread read unexpected character "+c)



    def binread_sequence(self):
        self.skip_blanks_and_comments()
        seqtype = self.get()
        elemtype = self.get()
        if seqtype=='\x12': # 1D little endian sequence
            shape = struct.unpack('<i',self.read(4))
            endianness = 'little'
        elif seqtype=='\x13': # 1D big endian sequence
            shape = struct.unpack('>i',self.read(4))
            endianness = 'big'
        elif seqtype=='\x14': # 2D little endian sequence
            shape = struct.unpack('<ii',self.read(8))
            endianness = 'little'
        elif seqtype=='\x15': # 2D big endian sequence
            shape = struct.unpack('>ii',self.read(8))
            endianness = 'big'
        else:
            raise TypeError('In binread_sequence, read invalid starting seqtype code char: '+seqtype)

        seq = []
        nelems = 1
        for dim in shape:
            nelems *= dim
        if elemtype=='\xFF': # generic sequence
            for i in xrange(nelems):
                seq.append(self.binread())
            if len(shape)>1:
                raise TypeError('Reading generic 2D sequence not yet suppored.. Must implement turning this into a list of lists...')
            return seq

        try:
            atype = PL_TYPECODE_TO_ARRAYTYPE[elemtype]
        except KeyError:
            raise TypeError('Unsuppored array elemtype: '+elemtype)

        elemsize = int(atype[-1])
        data = self.read(nelems*elemsize)
        ar = numarray.fromstring(data, atype, shape)
        
        if sys.byteorder!=endianness:
            ar.byteswap(True)

        if self.return_lists:
            ar = ar.tolist()
        return ar

##         elif elemtype==0x01: # signed char
##             ar = numarray.fromfile(self, '', shape)
##             # seq = struct.unpack(nelems*'b',self.read(nelems))
##         elif elemtype==0x02: # unsigned char
##             seq = struct.unpack(nelems*'B',self.read(nelems))

##         elif elemtype==0x03: # short little endian
##             seq =  struct.unpack('<'+nelems*'h',self.read(2*nelems))
##         elif elemtype==0x04: # short big endian
##             seq =  struct.unpack('>'+nelems*'h',self.read(2*nelems))
##         elif elemtype==0x05: # unsigned short little endian
##             seq =  struct.unpack('<'+nelems*'H',self.read(2*nelems))
##         elif elemtype==0x06: # unsigned short big endian
##             seq =  struct.unpack('>'+nelems*'H',self.read(2*nelems))
        
##         elif elemtype==0x07: # int little endian
##             seq =  struct.unpack('<'+nelems*'i',self.read(4*nelems))
##         elif elemtype==0x08: # int big endian
##             seq =  struct.unpack('>'+nelems*'i',self.read(4*nelems))
##         elif elemtype==0x0B: # unsigned int little endian
##             seq =  struct.unpack('<'+nelems*'I',self.read(4*nelems))
##         elif elemtype==0x0C: # unsigned int big endian
##             seq =  struct.unpack('>'+nelems*'I',self.read(4*nelems))

##         elif elemtype==0x0E: # float little endian
##             seq =  struct.unpack('<'+nelems*'f',self.read(4*nelems))
##         elif elemtype==0x0F: # float big endian
##             seq =  struct.unpack('>'+nelems*'f',self.read(4*nelems))

##         elif elemtype==0x10: # double little endian
##             seq =  struct.unpack('<'+nelems*'d',self.read(8*nelems))
##         elif elemtype==0x11: # double big endian
##             seq =  struct.unpack('>'+nelems*'d',self.read(8*nelems))


##         if dimensions==2:
##             a = reshape(a,length,width)


    def binread_pair(self):
        self.skip_blanks_and_comments()
        c = self.get()
        if c=="\x16": # binary header byte for pair
            a = self.binread()
            b = self.binread()
            return a,b
        else: # suppose it's an "ascii" kind of pair  A : B
            self.unread(c)
            a = self.binread()
            self.skip_blanks_and_comments()
            c = self.get()
            if c!=':':
                raise TypeError('Expected to read : but read '+c)
            b = self.binread()
            return a,b
    
    def binread_dict(self):        
        d = {}
        self.skip_blanks_and_comments()
        c = self.get()
        if c!='{':
            raise TypeError('Expected to read a {, but got '+c)
        while True:
            self.skip_blanks_and_comments_and_separators()
            if self.peek() == '}':
                break
            key = self.binread()
            self.skip_blanks_and_comments()
            c = self.get()
            if c!=':':
                raise TypeError('Expected to read : but read '+c)
            val = self.binread()
            d[key] = val
        c = self.get()                  # Eat trailing '}'
        assert c == '}', "Expected a closing '}' in reading dict, but got " + c
        return d

    def binread_tuple(self):        
        elems = []
        self.skip_blanks_and_comments()
        c = self.get()
        if c!='(':
            raise TypeError('Expected to read a (, but got '+c)
        while True:
            self.skip_blanks_and_comments_and_separators()
            if self.peek() == ')':
                break
            elem = self.binread()
            elems.append(elem)
        c = self.get()                  # Eat trailing '}'
        assert c == ')', "Expected a closing ')' in reading tuple, but got " + c
        return tuple(elems)

    def binread_Storage(self):
        self.skip_blanks_and_comments()
        head = self.read(7)
        if head!='Storage':
            raise TypeError('Expected to read Storage but read '+head)
        self.skip_blanks_and_comments()
        c = self.get()
        if c!='(':
            raise TypeError('Expected to read ( but read '+c)
        ar = self.binread_sequence()
        return ar

    def binread_TVec(self):
        c = self.peek()
        if c=='T':
            head = self.read(4)
            if head!='TVec':
                raise TypeError('Expected to read TVec but read '+head)
            self.skip_blanks_and_comments()
            c = self.get()
            if c!='(':
                raise TypeError('Expected to read ( but read '+c)
            length = self.binread()
            offset = self.binread()
            storage = self.binread_object()
            self.skip_blanks_and_comments()
            c = self.get()
            if c!=')':
                raise TypeError("Reading TVec(  ... should end with ), not "+c)
            if storage is None:
                return []
            else:
                return storage[offset:offset+length]
        else:
            raise TypeError("binread_TVec with first char "+c+" not implemented")

    def binread_pointer(self):
        self.skip_blanks_and_comments()
        c = self.get()
        if c!='*':
            raise TypeError('Expected to read * but read '+c)
        id = self.read_int()
        if id==0:
            return None
        self.skip_blanks_and_comments()
        if id in self.copies_map_in.keys():
            c = self.peek()
            if c=='-':
                raise TypeError('Pointer *'+str(id)+' already in copies_map_in, but is followed by character -')
            elif c in ",;":
                self.get() # skip it
            return self.copies_map_in[id]
        else:
           arrow = self.read(2)
           if arrow!='->':
               raise TypeError('Pointer *'+str(id)+' not in copies_map_in but is not followed by -> but by '+arrow)
           obj = self.binread()
           self.copies_map_in[id] = obj
           return obj
        
    def binread_object(self):
        self.skip_blanks_and_comments()
        if self.peek()=='*':
            return self.binread_pointer()
        classname = self.read_word()
        self.skip_blanks_and_comments()
        c = self.get()
        if c!='(':
            raise TypeError('Expected ( after classname '+classname)

        if classname=='Storage':
            self.unread('Storage(')
            return binread_Storage()
        elif classname=='TVec':
            self.unread('TVec(')
            return self.binread_TVec()
        elif classname=='TMat':
            self.unread('TMat(')
            return binread_TMat()
        else:
            # This uses the black magic defined in the magic module pl
            cls = getattr(pl, classname)
            obj = cls()

            # print 'In binread_object of class',classname
            while True:
                self.skip_blanks_and_comments_and_separators()
                if self.peek() == ')':
                    c = self.get()
                    # print 'Finished object because read',c
                    return obj
                key = self.read_word()
                # print 'key: ',key
                self.skip_blanks_and_comments()
                c = self.get()
                if c!='=':
                    raise TypeError('Expected to read = but read '+c)
                val = self.binread()

                # print >>sys.stderr, "Reading (key,val) =",(key,val)
                
                setattr(obj,key,val)

#######  Builtin Tests  #######################################################

def inner_test_StringStream(stream, collected):
    for lno, line in enumerate(stream):
        assert isinstance(line, str)
        assert line==collected[lno], \
               "(%d) %s != %s (%d)"%(len(line), line,
                    collected[lno],
                    len(collected[lno]))        
    
    print
    print "Read after iteration:", stream.read()
    
    print
    print "Pos, seek(0), pos:", stream.tell(), stream.seek(0), stream.tell()
    
    print
    s = stream.read(10)
    print "read(10) %s [len=%d]"%(s, len(s))
    
    lines = stream.readlines()
    assert lines[0]!=collected[0]
    for lno, line in enumerate(lines[1:]):
        lno += 1
        assert line==collected[lno], \
               "(%d) %s != %s (%d)"%(len(line), line,
                    collected[lno],
                    len(collected[lno]))

def test_PLearnIO_basics():
    from plearn.pyplearn.PyPLearnObject import PyPLearnObject, PLOption
    from StringIO import StringIO
    logging.root.setLevel(logging._levelNames['DEBUG'])
    from_server = StringIO()
    to_server   = StringIO()
    plearn_io   = PLearnIO(from_server, to_server)

    class Test(PyPLearnObject):
        option1 = PLOption("value1")
        option2 = PLOption(2)
        option3 = PLOption({"key3" : 3})

    test1 = Test()
    print >>from_server, test1
    print "Object sent by server..."

    # print from_server.read()
    
    read_test1 = plearn_io.binread_object()
    print read_test1

# test_PLearnIO_basics()

