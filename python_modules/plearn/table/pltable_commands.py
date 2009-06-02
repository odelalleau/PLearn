
"""
pltable_command.py

Copyright (C) 2005 ApSTAT Technologies Inc.

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

from plearn.table.viewtable import *

def print_usage_and_exit():
    print """
    Usage: pytable [options] command [params...]

    Options:
           -i <indexfile>
           --index <indexfile>
             uses indexfile as an index for the first table

    Commands:
           pytable info <table_file>
             prints length x width
             
           pytable fields <table_file>
             prints all fieldnames in original order
             
           pytable sorted_fields <table_file>
             prints all fieldnames sorted alphabetically
             
           pytable view <table_file>
             view of table interactively (curses based)

           pytable convert <src_table> <dest_file>
             saves src_table (can be a .txt .pytable .ptab .ptabdir .pmat)
             as dest_file which must be a .txt .ptab .ptabdir .pmat or .dmat
             Note that if you save into a .pmat or .dmat, all values must either
             be convertible to float, or be the empty string '' (which will
             be output as missing values (NaN)). If you want a smarter
             handling of strings, consider invoking 'pytable tovmat'
             instead.

           pytable tovmat <src_table> <dest_vmat> <stringmap_reference_dir>
             saves src_table (can be a .txt .pytable .ptab .ptabdir .pmat)
             as dest_vmat which must be a .pmat or .dmat
             The stringmap_reference_dir is a directory which
             will contain the mapping from non-numerical string
             representations to numerical value. This mapping will be
             automatically extended if yet unmapped representations are
             encountered. The dest_vmat.metadata/FieldInfo will be created
             as a symbolic link to stringmap_reference_dir/mappings.
             Also stringmap_reference_dir/logdir/YYYY-MM-DD_HH:MM:SS/ will
             contain stats.txt with the new type stat counts, and a .smap
             for each field whose string mapping had to be extended
             containing the extra mappings.

           pytable scan <table_file>
             will do a pass through the table, accessing every row

           pytable diff <table_file1> <table_file2>
             will report the differences between the 2 tables
          
           pytable countall <table.pytable> <fieldname>
             will count all possible instances of the value of the given field
             and print a list of all values with the associated count.

           pytable sum <table.pytable>
             will sum the values of each fields and print the result

           pytable dump <table.pytable>
             will dump the whole table as a tab-separated text table

      table_file can be a tab-separated text table
      or a python script ending in .pytable and defining a
      result variable that is a Table.
      """
    sys.exit()

    
def main(argv):
    
    if len(argv)<3:
        print_usage_and_exit()
    
    index_name = None
    n= 1
    if argv[n][:7]=='--index':
        if len(argv[n]) > 7:
            if argv[n][7] == '=':
                index_name = argv[n][8:]
                n+= 1
            else:
                print "invalid option syntax: " + argv[n]
                print_usage_and_exit()
    
        else:
            index_name = argv[n+1]
            n+= 2
    
    if argv[n][:2]=='-i':
        if len(argv[n]) > 2:
            if argv[n][2] == '=':
                index_name = argv[n][3:]
                n+= 1
            else:
                print "invalid option syntax: " + argv[n]
                print_usage_and_exit()
        else:
            index_name = argv[n+1]
            n+= 2
    
    command = argv[n]
    
    cmd_args = argv[n+1:]
    
    table_name= cmd_args[0]
    table= openTable(table_name)
    
    if index_name!=None:
        index= IntVecFile(index_name)
        table= SelectRows(table, index)
    
    
    if command == 'info':
        print str(table.length())+'x'+str(table.width())
    elif command == 'fields':
        for fieldname in table.fieldnames:
            print fieldname
    elif command == 'sorted_fields':
        fieldnames = table.fieldnames[:]
        fieldnames.sort()
        for fieldname in fieldnames:
            print fieldname
        
    elif command == 'view':
        fname = table_name
        table.set_title('FILE: '+fname)
        viewtable(table)
    
    elif command == 'convert':
        saveTable(table,cmd_args[1])
    
    elif command == 'tovmat':
        saveTableAsVMAT(table,cmd_args[1],cmd_args[2])
    
    elif command == 'scan':
        fname = table_name
        i = 0
        pbar = PBar('Scanning '+fname,len(table))
        # try:
        for row in table:
            pbar.update(i)
            i += 1
        # except Exception, e:
        #    print 'At row ',i
        #    raise e
        print 'Successfully scanned',i,'rows of',len(table)
    
    elif command == 'diff':
        m1 = table
        m2 = openTable(cmd_args[1])
        if len(m1)!=len(m2):
            print 'Lengths differ: ',len(m1),'!=',len(m2)
        if m1.width() != m2.width():
            print 'Widths differ: ',m1.width(),'!=',m2.width()
        if m1.fieldnames != m2.fieldnames:
            print 'Fieldnames differ: ',m1.fieldnames,'!=',m2.fieldnames
        w = m1.width()
        for i in xrange(min(len(m1),len(m2))):
            r1 = m1[i]
            r2 = m2[i]
            for j in xrange(w):
                if str(r1[j])!=str(r2[j]):
                    print 'Difference at (',i,',',j,') : ', r1[j], '!=', r2[j]            
    
    elif command == 'countall':
        fieldname = cmd_args[1]
        fieldpos = table.fieldnum(fieldname)
        counts = {}
        pbar = PBar('Counting all possible values of field '+fieldname,len(table))
        i = 0
        for row in table:
            val = row[fieldpos]
            try:
                counts[val] += 1
            except KeyError:
                counts[val] = 1
            i += 1
            pbar.update(i)
        pbar.close()
        print '## Value : Count'
        for key,val in counts.items():
            print ' ',key,'\t:',val
    
    elif command == 'sum':
        w = table.width()
        tot = [0.]*w
        pbar = PBar('Computing sums',len(table))
        i = 0
        for row in table:
            for j in xrange(w):
                try:
                    tot[j] += float(row[j])
                except ValueError:
                    pass
            i += 1
            pbar.update(i)
        pbar.close()
        print '## Sums:'
        for j in xrange(w):
            print '#',j,table.fieldnames[j],':',tot[j]
    
    elif command == 'dump':
        print '\t'.join(table.fieldnames)
        for i in xrange(len(table)):
            print '\t'.join(map(str,table[i]))
    
    else:
        print "invalid command: ", command
        print_usage_and_exit()

    table.close()
    
