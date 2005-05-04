# plio.py
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

import string
import struct
import plearn.pyplearn
from numarray import array,reshape  # for sequences

sign_and_digits = string.digits + "+-"
float_chars = string.digits+"+-.eE"
blank_chars = " \t\n\r"
blank_and_separator_chars = blank_chars+",;"
wordseparators = " \t\n\r()[]{};,:|#"

class PObject:
    pass

class PLearnIO:

    def __init__(self, pin, pout):
        self.pin = pin
        self.pout = pout
        self.unreadstring = ''
        self.copies_map_in = {}

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
        while c in blank_chars:
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
            elif c not in blank_chars:
                self.unread(c)
                return
            c = self.get()
        
    def skip_blanks_and_comments_and_separators(self):
        c = ' '
        while True:
            if c=='#':
                self.skip_rest_of_line()
            elif c not in blank_and_separator_chars:
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
        while c not in wordseparators:
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
        
        while c in sign_and_digits:
            s += c
            c = self.get()
        self.unread(c)
        return int(s)

    def read_float(self):
        s = ''
        self.skip_blanks_and_comments()
        c = self.get()
        while c in float_chars:
            s += c
            c = self.get()
        self.unread(c)
        return float(s)

            
    def write_typed(self, x):
        if type(x) is bool:
            if x:
                self.write('1 ')
            else:
                self.write('0 ')
        else:
            spec = plearn.pyplearn.PLearnRepr.repr(x)+' '
            print 'Sending spec string:',spec
            self.write(spec)

    def binread(self):
        self.skip_blanks_and_comments_and_separators()
        c = self.get()
        code = ord(c)

        if   code==0x01: # signed char
            return self.get()
        elif code==0x02: # unsigned char
            return self.get()

        elif code==0x03: # short little endian
            return struct.unpack('<h',self.read(2))[0]
        elif code==0x04: # short big endian
            return struct.unpack('>h',self.read(2))[0]
        elif code==0x05: # unsigned short little endian
            return struct.unpack('<H',self.read(2))[0]
        elif code==0x06: # unsigned short big endian
            return struct.unpack('>H',self.read(2))[0]
        
        elif code==0x07: # int little endian
            return struct.unpack('<i',self.read(4))[0]
        elif code==0x08: # int big endian
            return struct.unpack('>i',self.read(4))[0]
        elif code==0x0B: # unsigned int little endian
            return struct.unpack('<I',self.read(4))[0]
        elif code==0x0C: # unsigned int big endian
            return struct.unpack('>I',self.read(4))[0]

        elif code==0x0E: # float little endian
            return struct.unpack('<f',self.read(4))[0]
        elif code==0x0F: # float big endian
            return struct.unpack('>f',self.read(4))[0]

        elif code==0x10: # double little endian
            return struct.unpack('<d',self.read(8))[0]
        elif code==0x11: # double big endian
            return struct.unpack('>d',self.read(8))[0]

        elif c in "\x12\x13\x14\x15": # 1D or 2D sequence
            self.unread(c)
            return self.binread_sequence()

        elif c=='"':
            self.unread(c)
            return self.read_string()

        elif c=='*':
            self.unread(c)
            return self.binread_pointer()

        elif c=='{':
            self.unread(c)
            return self.binread_dict()

        elif c=='0':
            return False
        elif c=='1':
            return True

        elif c in string.ascii_letters:
            self.unread(c)
            return self.binread_object()

        else:
            raise TypeError("binread read unexpected character "+c)


    def binread_sequence(self):
        self.skip_blanks_and_comments()
        c = self.get()
        code = ord(c)        
        if code==0x12: # 1D little endian sequence
            dimensions = 1
            elemtype = ord(self.get())
            length, = struct.unpack('<i',self.read(4))
            nelems = length
        elif code==0x13: # 1D big endian sequence
            dimensions = 1
            elemtype = ord(self.get())
            length, = struct.unpack('>i',self.read(4))
            nelems = length
        elif code==0x14: # 2D little endian sequence
            dimensions = 2
            elemtype = ord(self.get())
            length,width = struct.unpack('<ii',self.read(4))
            nelems = length*width
        elif code==0x15: # 2D big endian sequence
            dimensions = 2
            elemtype = ord(self.get())
            length,width = struct.unpack('>ii',self.read(4))
            nelems = length*width
        else:
            raise TypeError('In binread_sequence, read invalid starting code char: '+c)

        seq = []
        if elemtype==0xFF: # generic sequence
            for i in xrange(nelems):
                seq.append(self.binread())

        elif elemtype==0x01: # signed char
            seq = struct.unpack(nelems*'b',self.read(nelems))
        elif elemtype==0x02: # unsigned char
            seq = struct.unpack(nelems*'B',self.read(nelems))

        elif elemtype==0x03: # short little endian
            seq =  struct.unpack('<'+nelems*'h',self.read(2*nelems))
        elif elemtype==0x04: # short big endian
            seq =  struct.unpack('>'+nelems*'h',self.read(2*nelems))
        elif elemtype==0x05: # unsigned short little endian
            seq =  struct.unpack('<'+nelems*'H',self.read(2*nelems))
        elif elemtype==0x06: # unsigned short big endian
            seq =  struct.unpack('>'+nelems*'H',self.read(2*nelems))
        
        elif elemtype==0x07: # int little endian
            seq =  struct.unpack('<'+nelems*'i',self.read(4*nelems))
        elif elemtype==0x08: # int big endian
            seq =  struct.unpack('>'+nelems*'i',self.read(4*nelems))
        elif elemtype==0x0B: # unsigned int little endian
            seq =  struct.unpack('<'+nelems*'I',self.read(4*nelems))
        elif elemtype==0x0C: # unsigned int big endian
            seq =  struct.unpack('>'+nelems*'I',self.read(4*nelems))

        elif elemtype==0x0E: # float little endian
            seq =  struct.unpack('<'+nelems*'f',self.read(4*nelems))
        elif elemtype==0x0F: # float big endian
            seq =  struct.unpack('>'+nelems*'f',self.read(4*nelems))

        elif elemtype==0x10: # double little endian
            seq =  struct.unpack('<'+nelems*'d',self.read(8*nelems))
        elif elemtype==0x11: # double big endian
            seq =  struct.unpack('>'+nelems*'d',self.read(8*nelems))

        a = array(seq)
        if dimensions==2:
            a = reshape(a,length,width)
        return a


    def binread_pair(self):
        self.skip_blanks_and_comments()
        a = self.binread()
        self.skip_blanks_and_comments()
        c = self.get()
        if c!=':':
            raise TypeError('Expected to read : but read '+c)
        b = self.binread
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
            key,val = self.binread_pair()
            d[key] = val
        return d

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
                return array([])
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
            obj = PObject()
            obj._classname_ = classname
            print 'In binread_object of class',classname
            while True:
                self.skip_blanks_and_comments_and_separators()
                if self.peek() == ')':
                    c = self.get()
                    print 'Finished object because read',c
                    return obj
                key = self.read_word()
                print 'key: ',key
                self.skip_blanks_and_comments()
                c = self.get()
                if c!='=':
                    raise TypeError('Expected to read = but read '+c)
                val = self.binread()
                setattr(obj,key,val)
                
