"""
viewtable.py

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

import sys
import curses
import threading
import cPickle as pickle
import bisect
import os
import os.path
import subprocess
import traceback
import locale
from plearn.table.table import *
from plearn.table.tablestat import *
from plearn.table.date import *
from plearn.plotting.numpy_utils import to_numpy_float_array, mapping_of_values_to_pos
import matplotlib.colors

def format_string_to_width(s, width):
    if len(s)<width:
        s += ' '*(width-len(s))
    else:
        s = s[0:width-1]+' '
    return s

def daydiff(cyymmdd1, cyymmdd2):
    return (CYYMMDD_to_date(cyymmdd1)-CYYMMDD_to_date(cyymmdd2)).days

def select_item(scr, items, title='',
               instructions='Select one of the following by pressing the appropriate key'):

    """scr is the curses screen or window,
    items is a list of strings to be selected from.  This displays a selection
    screen allowing the user to select one of the items.
    The index of the selected item is returned"""

    scr.erase()
    scr.addstr(0,0,title,curses.A_REVERSE)
    scr.addstr(2,1,instructions,curses.A_BOLD)

    n = len(items)
    keys = "123456789abcdefghijklmnop"
    keys = keys[0:n]
    keycodes = map(ord,keys)
    for i in xrange(n):
        scr.addstr(4+i,3,'['+keys[i]+'] '+items[i])
    scr.move(4+n,3)
    scr.refresh();
    while True:
        k = scr.getch();
        try:
            id = keycodes.index(k)
            break
        except ValueError:
            pass
    scr.erase()
    return id

class TableView:

    def __init__(self, table, stdscr):
        
        self.stdscr = stdscr
        self.transposed = False
        self.dot_mode = False
        self.margin_top = 2
        self.margin_left = 0
        self.margin_right = 0
        self.margin_bottom = 2
        self.rowname_width = 20
        self.set_field_dim(20, 1)

        self.monetary_format= False
        self.orig_locale= locale.getlocale(locale.LC_NUMERIC)
        self.full_shuffle_stats= False

        self.reinit(table)
        

    def reinit(self, table):
        self.i0 = 0
        self.j0 = 0
        self.current_i = 0
        self.current_j = 0
        self.setTable(table)

        self.conditioning_field = ''
        self.weight_field = ''
        self.target_fields = []

        self.search_field = 0
        self.search_value = table[0][0]
        self.search_expression = ''

        self.stats = None
        self.statsthread = None
        self.stop_thread = False
        self.statslock = threading.Lock()
        self.shuffled_rowidx = None
        self.statsview = None

        self.sumvar = ''
        self.divvar = ''
        self.marginalize = 0
        # combinvarsmode 0: numerator_sum_var/denominator_sum_var   1: numerator_sum_var-0.7*denominator_sum_var
        self.combinvarsmode = 0
        # a map containing for each variable name the summarize_remove_n setting
        self.summarize_remove_n = {}
        self.minprob = 0.

        # used by sort and filter operations
        self.selected_rows = None
        self.selected_fields = []
        
        self.graph_fields = {}
        
        self.redraw()        


    def writecolname(self,sj,text,attr=curses.A_NORMAL):
        text = format_string_to_width(text, self.fieldwidth)
        si = self.margin_top
        sj = self.margin_left+self.rowname_width+sj*self.fieldwidth
        self.stdscr.addstr(si,sj,text,attr)        
        
    def writerowname(self,si,text,attr=curses.A_NORMAL):
        text = format_string_to_width(text, self.rowname_width)
        si = self.margin_top+1+si*self.fieldheight
        sj = self.margin_left
        self.stdscr.addstr(si,sj,text,attr)                

    def writetop(self,text,attr=curses.A_NORMAL):
        text = format_string_to_width(text,curses.COLS-1)
        self.stdscr.addstr(0,0,text,attr)

    def writebottom(self,text,attr=curses.A_NORMAL):
        self.stdscr.addstr(curses.LINES-2,0,'_'*curses.COLS)
        text = format_string_to_width(text,curses.COLS-1)
        self.stdscr.addstr(curses.LINES-1,0,text,attr)

    def writefield(self,si,sj,text,attr=curses.A_NORMAL):
        text = format_string_to_width(text, self.fieldwidth)
        si = self.margin_top+1+si*self.fieldheight
        sj = self.margin_left+self.rowname_width+sj*self.fieldwidth
        self.stdscr.addstr(si,sj,text,attr)

    def setTable(self,table):
        self.table = table
        self.selected_columns = range(self.table.width())
        self.cache = {}
        self.cache_capacity = 100
        self.cache_min_i = 0
        self.cache_max_i = 0
        if self.current_i>=self.table.length():
            self.current_i = 0
            self.i0 = 0
        if self.current_j>=self.table.width():
            self.current_j = 0
            self.j0 = 0
        l,w = table.length(), table.width()
        self.set_toprightinfo(str(l)+'x'+str(w))

    def getFullRow(self,i):
        if self.selected_rows is not None:
            i = self.selected_rows[i]
        # row = self.table.getRow(i)
        row = self.table[i]
        return row

    def getRow(self,i):
        if self.selected_rows is not None:
            i = self.selected_rows[i]
        try:
            row = self.cache[i]
        except KeyError:
            self.statslock.acquire()
            row = self.table.getRow(i)
            self.statslock.release()
            if len(self.cache)>=self.cache_capacity: # delete one row from the cache
                if i==self.cache_max_i+1:
                    del self.cache[self.cache_min_i]
                    self.cache_min_i = min(self.cache)
                elif i==self.cache_min_i-1:
                    del self.cache[self.cache_max_i]
                    self.cache_max_i = max(self.cache)
                else:
                    self.cache = {}
                    self.cache_min_i = i
                    self.cache_max_i = i
                    
            self.cache[i] = row
            self.cache_min_i = min(i,self.cache_min_i)
            self.cache_max_i = max(i, self.cache_max_i)

        return [ row[col] for col in self.selected_columns ]

    def getVal(self,i,j):
        return self.getRow(i)[j]
    
##         if i>=self.row_cache_mini and i<=self.row_cache_maxi:
##         try:
##             return self.row_cache[i-]
    

    def set_field_dim(self, fieldwidth, fieldheight):
        self.fieldwidth = max(2, min(fieldwidth, 100))
        self.fieldheight = max(1, min(fieldheight, 50))
        self.compute_ni_nj()

    def compute_ni_nj(self):
        if self.transposed:            
            self.ni = (curses.COLS-(self.rowname_width+self.margin_left+self.margin_right))/self.fieldwidth
            self.nj = (curses.LINES-(self.margin_top+self.margin_bottom+1))/self.fieldheight
        else:            
            self.ni = (curses.LINES-(self.margin_top+self.margin_bottom+1))/self.fieldheight
            self.nj = (curses.COLS-(self.rowname_width+self.margin_left+self.margin_right))/self.fieldwidth

    def display_help(self,c=0):
#        self.stdscr.erase()
        helptext = """
*************************
**  KEYS IN DATA VIEW  **
*************************
 arrow keys   : move in corresponding direction
 page up/down : move up/down one screen
 home/end     : move to first / last screen
 t            : transpose view
 > <          : increase or decrease width of display field
 ) (          : sort rows in increasing or decreasing order of values of current field 
 a            : sort fieldnames alphabetically
 SPACE        : select/deselect field 
 c            : select constant fields (fields having constant value in all rows)
 ENTER        : keep only selected fields in display
 k            : hide selected fields from display (or if none selected, hide current field)
 l            : prompt for a line number and go to that line
 .            : toggle displaying of ... for values that do not change
 =            : search for a value of the current field
 /            : search for row satisfying python expression (ex: float(AGE1)<float(AGE2) )
 N            : search next
 P            : search previous
 !            : filter rows satisfying python expression (or current selected fields values)
 o            : revert to the original table (all fields and rows in orignal order)
 h            : display this help screen
 *            : mark main conditioning field (Also used for subplot rows)
 +            : mark conditional summing fields (Also used for subplot columns)
 ~            : mark (optional) weight field. (Also used for figure number).
 s            : choose among saved stats
 x            : execute a shell command
 TAB          : switch to stats view
 F            : toggle full shuffle of records for stats [currently: %s]
 f            : find a field by name
 m            : toggle display of numeric fields in monetary format [currently: %s]
 X            : select x field
 Y            : select y field
 M            : select marker field (discrete variable)
 C            : select color field (discrete variable)
 S            : select marker-size field
 U            : select arrow x component field
 V            : select arrow y component field
 G            : graphical plot of selected fields
 w            : write (save) current view as a pytable file (NEEDS TO BE FIXED)
 q            : quit program

*******************************
** EXTRA KEYS IN STATS VIEW: **
*******************************
 1/2       : cycle through numerator or denominator sum variable 
 3         : cycle through percentage views
 4         : toggle between numerator/denominator and numerator-0.7*denominator
 - =       : decrease/increase number of numerical intervals by increasing min cond prob
 n/p       : move to next/previous field for first conditioning var
 SPACE     : update view 
 TAB or q  : go back to data view

(press any key to continue)
""" % (self.full_shuffle_stats, self.monetary_format)
        # if c:
        #    helptext = 'Pressed '+str(c)+'\n'+helptext

        self.display_fullscreen(helptext)
            
#         helpi = 1
#         for line in helptext.split('\n'):
#             self.safeaddstr(helpi,2,line)
#             helpi += 1
#         self.stdscr.refresh();
#         self.stdscr.getch();
#         self.redraw()
        
    def safeaddstr(self,i,j,line,attr=curses.A_NORMAL):
        if i<curses.LINES-1 and j<curses.COLS:
            self.stdscr.addstr(i,j,line,attr)
        
    def fieldname(self, j):
        return self.table.fieldnames[self.selected_columns[j]]

    def fieldnames(self):
        return [ self.table.fieldnames[self.selected_columns[j]] for j in xrange(self.width()) ]

    def length(self):
        if self.selected_rows is not None:
            return len(self.selected_rows)
        else:
            return self.table.length()

    def width(self):
        return len(self.selected_columns)

    def set_graph_field(self, fieldchar):
        fieldname = self.fieldname(self.current_j)
        if fieldchar in self.graph_fields and self.graph_fields[fieldchar]==fieldname:
            del self.graph_fields[fieldchar]
        else:
            self.graph_fields[fieldchar]=fieldname
        self.redraw()
        
    def fieldname_repr_and_style(self, j):
        fieldname = self.fieldname(j)
        fnamerepr = fieldname
        style = curses.A_BOLD
        if fieldname in self.target_fields:
            fnamerepr = ' + '+fnamerepr
            style = curses.A_BOLD | curses.A_UNDERLINE
        if self.conditioning_field and fieldname==self.conditioning_field:
            fnamerepr = ' * '+fnamerepr
            style = curses.A_BOLD | curses.A_UNDERLINE
        if self.weight_field and fieldname==self.weight_field:
            fnamerepr = ' ~ '+fnamerepr
            style = curses.A_BOLD | curses.A_UNDERLINE

        for fieldchar,gfieldname in self.graph_fields.items():
            if fieldname==gfieldname:
                fnamerepr = fieldchar+' '+fnamerepr            

        pos = None
        try:
            pos = self.selected_fields.index(fieldname)
        except ValueError:
            pass

        if pos != None:
            fnamerepr = str(pos)+'. '+fnamerepr 
            style = style | curses.A_REVERSE
           
        return fnamerepr, style

    def redraw(self):
        self.stdscr.erase()
        l, w = self.length(),self.width()
        table = self.table
        if self.current_j>=self.width():
            self.current_j = self.width()-1
        if self.transposed:
            self.draw_transposed()
        else:
            self.draw_normal()

        # Write title
        # self.stdscr.hline(0,0,curses.COLS-1,ord(' '))
        title = self.table.title()
        self.stdscr.addstr(0,0,title,curses.A_BOLD)
        # Write dimensions
        dimstr = self.get_toprightinfo()
        self.stdscr.addstr(0,curses.COLS-len(dimstr)-1,dimstr,curses.A_BOLD)
        
        # write info line
        self.stdscr.hline(curses.LINES-2,0,curses.COLS,ord('_'))
        # self.stdscr.addstr(curses.LINES-2,0,'_'*curses.COLS)
        current_val = self.getVal(self.current_i,self.current_j)
        infoline = self.rowname(self.current_i)+' | '+self.fieldname(self.current_j)+' | '+str(current_val) + ' | ' + str(type(current_val))
        self.writebottom(infoline)
        self.stdscr.refresh();

    def get_toprightinfo(self):
        return self.toprightinfo

    def set_toprightinfo(self, text):
        self.toprightinfo = text

    def rowname(self,i):
        return self.table.rowname(i)

    def format(self,x):
        v = None
        try: v = float(x)
        except ValueError: pass
        except TypeError: pass
        if v == None or not self.monetary_format:
            return str(x)
        return locale.format('%.2f', v, True)


    def draw_normal(self):
        istart, jstart = self.i0, self.j0
        istop = istart+self.ni
        if istop>self.length():
            istop = self.length()
        jstop = jstart+self.nj
        if jstop>self.width():
            jstop = self.width()

        # write fieldnames
        for j in xrange(jstart,jstop):
            fieldname, style = self.fieldname_repr_and_style(j)
            self.writecolname(j-jstart,fieldname,style)

        # write rowname and values of table row i
        prev_row = None
        for i in xrange(istart,istop):
            # write name of row i            
            rowname = self.rowname(i)
            self.writerowname(i-istart,rowname,curses.A_BOLD)
            row = self.getRow(i)
            # write values of row i
            for j in xrange(jstart,jstop):
                val = row[j]
                #strval = str(val)
                strval = self.format(val)
                if self.dot_mode and prev_row!=None and prev_row[j]==val:
                    strval = '...'
                style = curses.A_NORMAL
                if j==self.current_j or i==self.current_i:
                    style = curses.A_REVERSE
                self.writefield(i-istart,j-jstart,strval,style)
            prev_row = row

    def draw_transposed(self):
        istart, jstart = self.i0, self.j0
        istop = istart+self.ni
        if istop>self.length():
            istop = self.length()
        jstop = jstart+self.nj
        if jstop>self.width():
            jstop = self.width()

        # write fieldnames
        for j in xrange(jstart,jstop):
            fieldname, style = self.fieldname_repr_and_style(j)
            self.writerowname(j-jstart,fieldname,style)

        # write rowname and values of table row i
        prev_row = None
        for i in xrange(istart,istop):
            # write name of row i            
            rowname = self.rowname(i)
            self.writecolname(i-istart,rowname,curses.A_BOLD)
            row = self.getRow(i)
            # write values of row i
            for j in xrange(jstart,jstop):
                val = row[j]
                #strval = str(val)
                strval = self.format(val)
                if self.dot_mode and prev_row!=None and prev_row[j]==val:
                    strval = '...'
                style = curses.A_NORMAL
                if j==self.current_j or i==self.current_i:
                    style = curses.A_REVERSE
                self.writefield(j-jstart,i-istart,strval,style)
            prev_row = row

    def saveStats(self):
        self.statslock.acquire()
        fpath = self.statsFilePathFromSelectedVars()
        dirpath = os.path.dirname(fpath)
        if not os.path.isdir(dirpath):
            os.makedirs(dirpath)
        f = open(fpath+'.new','wb')
        pickle.dump(self.stats, f, 0) # using protocol 0, because 1 and 2 have bug not allowing pickling of inf
        f.close()
        try: os.remove(fpath)
        except OSError: pass
        os.rename(fpath+'.new',fpath)
        self.statslock.release()

    def statsFilePathFromSelectedVars(self):
        dirpath = self.table.filepath()+'.stats'
        fname = 'CONDSTAT_'+self.conditioning_field+'___'+self.weight_field+'___'+'___'.join(self.target_fields)+'.pickle'
        fpath = os.path.join(dirpath,fname)
        return fpath

    def loadStats(self, statsfpath):
        self.clearStats()
        self.selectVarsFromStatsFilePath(statsfpath)
        f = open(statsfpath)
        self.stats = pickle.load(f)
        f.close()
        self.sumvar = ''
        self.divvar = ''
        self.marginalize = 0
        self.combinvarsmode = 0
        
    def loadMostRecentStats(self):
        dirpath = self.table.filepath()+'.stats'
        try:
            fpathlist = [ os.path.join(dirpath,fname) for fname in os.listdir(dirpath) if fname.startswith('CONDSTAT_') and fname.endswith('.pickle') ]
        except OSError:
            fpathlist = []
        if not fpathlist:
            scr = self.stdscr
            scr.erase()
            self.safeaddstr(5,3,"You must choose at least the conditioning field with * before getting statistics",curses.A_BOLD)
            scr.refresh();
            scr.getch();
            self.redraw()
            return                
        fpathtimes = [ os.stat(fpath).st_mtime for fpath in fpathlist ]
        fpath = fpathlist[argmax(fpathtimes)]
        self.loadStats(fpath)        

    def chooseStats(self):
        self.stopThread()
        dirpath = self.table.filepath()+'.stats'
        try:
            fnamelist = [ fname for fname in os.listdir(dirpath) if fname.startswith('CONDSTAT_') and fname.endswith('.pickle') ]
        except OSError:
            fnamelist = []
        if not fnamelist:
            return
        else:
            headlen = len('CONDSTAT_')
            taillen = len('.pickle')
            def myrepr(fname):
                tokens = fname[headlen:-taillen].split('___')
                condfield = tokens[0]
                wfield = tokens[1]
                rest = tokens[2:]
                if wfield:
                    return '*'+condfield+'  ~'+wfield+'  +'+' +'.join(rest)
                else:
                    return '*'+condfield+'  +'+' +'.join(rest)
            itemlist = map(myrepr, fnamelist)
            itemnum = select_item(self.stdscr, itemlist, 'Choose a stats file to load')
        statsfpath = os.path.join(dirpath,fnamelist[itemnum])
        self.loadStats(statsfpath)        

    def selectVarsFromStatsFilePath(self, fpath):
        fname = os.path.basename(fpath)
        basename,ext   = os.path.splitext(fname)

        if ext!='.pickle':
            raise ValueError('statfilepath should end with .pickle')
        if not basename.startswith('CONDSTAT_'):
            raise ValueError('statfilepath should start with CONDSTAT_')
        fieldnames = basename[len('CONDSTAT_'):].split('___')
        self.conditioning_field = fieldnames[0]
        self.weight_field = fieldnames[1]
        if fieldnames[2]:
            self.target_fields = fieldnames[2:]
        else:
            self.target_fields = []
            
        # f = open('toto.log','w')
        # print >>f, 'fpath: ',fpath 
        # print >>f, 'fieldnames: ',fieldnames 
        # print >>f, 'self.target_fields: ',self.target_fields
        # f.close()

    def initStats(self):
        """Initializes the stats member according to selected variables
        trying to load a start version from file if available"""

        try:
            self.loadStats(self.statsFilePathFromSelectedVars())
        except IOError:
            var_combinations = all_combinations([self.table.fieldnames, [self.conditioning_field]])
            var_domains = {}
            for varname in self.table.fieldnames:
                var_domains[varname] = AutoDomain()
            self.stats = BaseTableStats(var_combinations, var_domains, self.target_fields, self.weight_field,
                                        self.full_shuffle_stats)
            self.initShuffledIndex()
            # update with row 0 so that stats are not empty
            row = self.table[self.shuffled_rowidx[0]]
            self.stats.update(row) 
            self.startThread()
            self.sumvar = ''
            self.divvar = ''
            self.marginalize = 0
            self.combinvarsmode = 0

    def clearStats(self):
        self.stopThread()
        self.stats = None

    def initShuffledIndex(self):
        min_len_for_partial_shuffle= 5000
        if self.shuffled_rowidx is None:
            self.stdscr.erase()
            self.safeaddstr(5,3,"Building shuffled index. Please be patient...",curses.A_BOLD)
            self.stdscr.refresh()
            numpy.numarray.random_array.seed(58273,26739)
            self.shuffled_rowidx = numpy.numarray.random_array.permutation(self.table.length()).astype(int)
            if hasattr(self.stats, 'full_shuffle_stats'):
                self.full_shuffle_stats= self.stats.full_shuffle_stats
            else:
                self.full_shuffle_stats= True
            if not self.full_shuffle_stats and self.table.length() >= min_len_for_partial_shuffle:
                x= self.shuffled_rowidx
                #x.resize(min_len_for_partial_shuffle)
                resize(x,[min_len_for_partial_shuffle])
                xd = {}
                for z in x:
                    xd[z]=z
                y = numpy.numarray.arange(self.table.length())
                y= numpy.numarray.array([z for z in y if xd.get(z) == None])
                self.shuffled_rowidx= numpy.numarray.concatenate((x,y)).astype(int)
    
    def startThread(self):
        if not self.statsthread and self.stats.nsamples<self.table.length():
            self.initShuffledIndex()
            self.statsthread = threading.Thread(target=self.statsThreadRun)
            self.statsthread.start()

    def statsThreadRun(self):
        l = self.table.length()
        idx = self.shuffled_rowidx
        self.statslock.acquire()
        nsamples = self.stats.nsamples
        self.statslock.release()

        #f=open('xxxxx.nfg.tmp','w')
        

        while nsamples<l:            
            self.statslock.acquire()
            row = self.table[idx[nsamples]]
            
            #f.write(str([nsamples, idx[nsamples]]))

            nsamples = self.stats.update(row)
            stop_thread = self.stop_thread
            self.statslock.release()

            
            if stop_thread:
                break
            if nsamples % 2000 == 0:
                self.saveStats()
        self.saveStats()

        #f.close()
            
    def stopThread(self):
        if self.statsthread:
            self.statslock.acquire()
            self.stop_thread = True
            self.statslock.release()
            self.statsthread.join()
            self.statsthread = None
            self.stop_thread = False

    def transpose(self):
        self.transposed = not self.transposed
        self.compute_ni_nj()
        self.i0 = max(0,self.current_i-self.ni//2)
        self.j0 = max(0,self.current_j-self.nj//2)
        self.redraw()

    def sort_rows(self, j, reverse=False):
        """sort in incresing order of field j"""
        if self.selected_rows is None:
            self.selected_rows = range(self.length())
        if reverse: # hack to make sure we keep the partial order of previous sorts on other columns
            self.selected_rows.reverse()
        biglist = [ (float_or_str(self.getVal(pos,j)), pos) for pos in xrange(self.length()) ]
        biglist.sort(reverse=reverse)
        self.selected_rows = [ self.selected_rows[pos] for val,pos in biglist ]
        self.redraw()

    def previous_row(self):
        if self.current_i>0:
            self.current_i -= 1
            if self.current_i<self.i0:
                self.i0 -= 1

    def next_row(self):
        if self.current_i<self.length()-1:
            self.current_i += 1
            if self.current_i>=self.i0+self.ni:
                self.i0 += 1

    def previous_field(self):
        if self.current_j>0:
            self.current_j -= 1
            if self.current_j<self.j0:
                self.j0 -= 1

    def next_field(self):
        if self.current_j<self.width()-1:
            self.current_j += 1
            if self.current_j>=self.j0+self.nj:
                self.j0 += 1

    def left(self):
        if self.transposed:
            self.previous_row()
        else:
            self.previous_field()
        self.redraw()

    def right(self):
        if self.transposed:
            self.next_row()
        else:
            self.next_field()
        self.redraw()

    def up(self):
        if self.transposed:
            self.previous_field()
        else:
            self.previous_row()
        self.redraw()

    def down(self):
        if self.transposed:
            self.next_field()
        else:
            self.next_row()
        self.redraw()

    def pgdown(self):
        if self.transposed:
            self.j0 = min(self.j0+self.nj, max(0,self.width()-self.nj))
            self.current_j = min(self.current_j+self.nj, self.width()-1)
        else:
            self.i0 = min(self.i0+self.ni, max(0,self.length()-self.ni))
            self.current_i = min(self.current_i+self.ni, self.length()-1)
        self.redraw()

    def pgup(self):
        if self.transposed:
            self.j0 = max(self.j0-self.nj, 0)
            self.current_j = max(self.current_j-self.nj, 0)
        else:
            self.i0 = max(self.i0-self.ni, 0)
            self.current_i = max(self.current_i-self.ni, 0)
        self.redraw()

    def home(self):
        if self.transposed:
            self.j0 = 0
            self.current_j = 0
        else:
            self.i0 = 0
            self.current_i = 0
        self.redraw()

    def end(self):
        if self.transposed:
            self.current_j = self.width()-1
            self.j0 = max(0, self.width()-self.nj)
        else:
            self.current_i = self.length()-1
            self.i0 = max(0, self.length()-self.ni)
        self.redraw()

    def input(self,prompt):
        si = curses.LINES-1
        self.stdscr.addstr(si,0,' '*(curses.COLS-1))
        self.stdscr.addstr(si,0,prompt, curses.A_BOLD)
        curses.echo()
        entry = self.stdscr.getstr(si,len(prompt))
        curses.noecho()
        return entry

    def goto_line(self,i):
        i = max(0, min(i, self.length()-1))
        self.current_i = i
        self.i0 = max(0,i-self.ni//2)
        self.redraw()

    def goto_col(self,j):
        j = max(0, min(j, self.width()-1))
        self.current_j = j
        self.j0 = max(0,j-self.nj//2)
        self.redraw()

    def search_next(self):
        orig_i = self.current_i
        for i in xrange(self.current_i+1, self.length()):
            if i%10000 == 0: #prorgess...
                self.goto_line(i)
            if self.search_expression=='':
                val = self.getVal(i,self.search_field)
                if str(val) == self.search_value:
                    self.goto_line(i)
                    return #break
            else:
                v = dict(zip(self.table.fieldnames, self.getFullRow(i)))
                try:
                    res = eval(self.search_expression, globals(), v)
                except:
                    self.goto_line(i)
                    self.display_exception(sys.exc_info())
                    return #break
                else:
                    if res:
                        self.goto_line(i)
                        return #break
        self.goto_line(orig_i) # not found, reset cursor (was moved to show progress)

    def select_constant_cols(self):
        """Will select all columns for which the values do not change"""

        constant_cols = self.selected_columns[:]

        first_row = None
        for i in xrange(self.length()):
            if self.selected_rows is None:
                orig_i = i
            else:
                orig_i = self.selected_rows[i]
            row = self.table.getRow(orig_i)
            if first_row is None:
                first_row = row[:]
            constant_cols = [ j for j in constant_cols if row[j]==first_row[j] ]
        self.selected_fields = [ self.table.fieldnames[j] for j in constant_cols ]
        self.redraw()

    def filter(self):
        if self.selected_fields == []:
            filter_expression = self.input('Filter expression ex: float(AGE)>3 : ').strip()
        else:
            row = self.getFullRow(self.current_i)
            filter_expression = ' and '.join([ 'locals()["'+fname+'"]=='+repr(row[fname]) for fname in self.selected_fields ])
            #self.selected_columns = [ self.table.fieldnames.index(fname) for fname in self.selected_fields]

        if filter_expression!='':
            selection = []
            for i in xrange(self.length()):
                if self.selected_rows is None:
                    orig_i = i
                else:
                    orig_i = self.selected_rows[i]
                v = dict(zip(self.table.fieldnames, self.table.getRow(orig_i)))
                try:
                    keepit = eval(filter_expression, globals(), v)
                except:
                    self.goto_line(i)
                    self.display_exception(sys.exc_info(), filter_expression)
                    self.redraw()
                    return
                else:
                    if keepit:
                        selection.append(orig_i)
                if i==self.current_i:
                    self.current_i = len(selection)-1
            self.selected_rows = selection
            self.selected_fields = []
        self.i0 = 0
        #self.j0 = 0
        self.current_i = 0
        #self.current_j = 0
        self.redraw()

    def executeShellCommand(self, command):

        filepathdir = ""
        # First figure out filepathdir (if possible)
        try:
            filepath = self.getFullRow(self.current_i)["_filepath_"]
            filepathdir = os.path.dirname(filepath)
        except KeyError:
            pass

        # replace _DIRPATH_ in command by filepathdir
        command = command.replace('_DIRPATH_',filepathdir)

        # k = self.display_fullscreen(command)
        
        # os.system(command)
        subprocess.Popen(command, shell=True)
        
    def chooseAndExecuteShellCommand(self):
        
        commands = [
            ('s',"Launch terminal and shell in this matrix's directory",
             """xterm -e sh -c 'cd "_DIRPATH_"; pwd; ls; sh' """),
            ('1',"View layer 1 unsup training costs",
             """xterm -e sh -c 'cd "_DIRPATH_"; myplearn vmat view `find . -name training_costs_layer_1.pmat`' """),
            ('i',"deepnetplot.py plotRepAndRec learner.psave", 
             """xterm -e sh -c 'cd "_DIRPATH_"; pwd; learnerfile=`find . -name learner.psave`; echo "found $learnerfile"; deepnetplot.py plotRepAndRec $learnerfile ~/data/mnist/mnist_small/mnist_basic2_valid.pmat; sh' """),
            ('w',"deepnetplot.py plotEachRow learner.psave", 
             """xterm -e sh -c 'cd "_DIRPATH_"; pwd; learnerfile=`find . -name learner.psave`; echo "found $learnerfile"; deepnetplot.py plotEachRow $learnerfile ~/data/mnist/mnist_small/mnist_basic2_valid.pmat; sh' """),
            ('I',"deepnetplot.py plotRepAndRec final_learner.psave", 
             """xterm -e sh -c 'cd "_DIRPATH_"; pwd; learnerfile=`find . -name final_learner.psave`; echo "found $learnerfile"; deepnetplot.py plotRepAndRec $learnerfile ~/data/mnist/mnist_small/mnist_basic2_valid.pmat; sh' """),
            ('W',"deepnetplot.py plotEachRow final_learner.psave", 
             """xterm -e sh -c 'cd "_DIRPATH_"; pwd; learnerfile=`find . -name final_learner.psave`; echo "found $learnerfile"; deepnetplot.py plotEachRow $learnerfile ~/data/mnist/mnist_small/mnist_basic2_valid.pmat; sh' """),
            ]

        menutxt = '\n'.join([ '['+commands[i][0]+'] '+commands[i][1] for i in range(len(commands)) ])+'\n'
        k = self.display_fullscreen(menutxt)
        for com in commands:
            key, descr, command = com
            if k==ord(key):
                self.executeShellCommand(command)
                break
            
        if( k==ord('\n') ):
            return

    def collect_fieldvecs(self, fieldnames):
        """Returns a dictionary indexed by the names of fields,
        containing vectors of values of those fields."""
        orig_fieldnames = self.table.fieldnames
        orig_pos = [ orig_fieldnames.index(fname) for fname in fieldnames ]
        l = self.length()
        n = len(fieldnames)
        fieldvecs = [ [] for i in xrange(n) ]
        for i in xrange(l):
            row = self.getFullRow(i)
            for j in xrange(n):
                fieldvecs[j].append( row[orig_pos[j]] )
        fieldvecdict = {}
        for j in xrange(n):
            fieldvecdict[fieldnames[j]] = numpy.array(fieldvecs[j])
        return fieldvecdict

    def take_part_of_fieldvecs(self, positions, fieldvecs):
        result = {}
        for name,fieldvec in fieldvecs.items():
            result[name] = numpy.take(fieldvec, positions)
        return result
    
    def graphical_scatter_plot(self):
        """
        (STILL UNDER DEVELOPMENT, SOME DEBUGGING LEFT TO DO)
        This docstring is probably not fully accurate.
        The function can potentially create several figures, one for each value of the 'weight field' (if specified).
        For each figure, it can create several subplots, based on the values of the 'conditioning field' and selected target fields.
        For each such subplot, it will use the following tagged fields:
        X: coordinate
        Y: coordinate
        C: color
        M: marker
        S: size of marker

        A color field can be specified for a categorical variable.
        A marker field can be specified for a categorical variable (points will use different markers).
        
        If U and/or V are specified, an arrow plot is superposed (pylab.quiver) of given U,V relative tip coordinates
        """

        # First find out what fields are of interest
        fieldnames = self.graph_fields.values()
        if self.conditioning_field!='':
            fieldnames.append(self.conditioning_field)
        if self.weight_field!='':
            fieldnames.append(self.weight_field)
        fieldnames += self.target_fields

        # collect the values of those fields in a dictionary indexed by fieldname
        fieldvecs = self.collect_fieldvecs(fieldnames)

        if self.weight_field!='':
            val2pos = mapping_of_values_to_pos(fieldvecs[self.weight_field])
            fignum = 0
            for val, positions in val2pos.items():
                fieldvecs_part = self.take_part_of_fieldvecs(positions, fieldvecs)
                fignum += 1
                pylab.figure(fignum)
                self.scatter_plot_all_subplots(fieldvecs_part, title=self.weight_field+'='+str(val))
        else:
            pylab.figure(1)
            self.scatter_plot_all_subplots(fieldvecs, title="")            

        pylab.show()

    def display_colors_legend(self, colorlabel, colorlist):
        """colorlist is a list of pairs (value, coilrspec) for the legend"""
        x = 0.92
        y = 0.9
        pylab.figtext(x,y,'['+colorlabel+']',color='k',weight='bold')
        y -= 0.025
        for val,color in colorlist:
            pylab.figtext(x,y,str(val),color=color)
            y -= 0.025

    def display_markers_legend(self, markerlabel, markerdict):
        legend_lines = []
        legend_labels = markerdict.keys()
        legend_labels.sort()
        for label in legend_labels:
            legend_lines.append(pylab.Line2D(range(2), range(2), linestyle=' ',
                                             marker=markerdict[label], color='k'))
        label_line = pylab.Line2D(range(2), range(2), linestyle=' ', marker=' ', color='k')
        pylab.figlegend([label_line]+legend_lines,
                        [markerlabel]+legend_labels,
                        'lower right')        
                
    def scatter_plot_all_subplots(self, fieldvecs, title=''):
        # TODO: fix this: pylab.suptitle(title)
        rows_field = self.conditioning_field
        if rows_field=='':
            nrows = 1
        else:
            rows_val2pos = mapping_of_values_to_pos(fieldvecs[rows_field])
            rows_vals = rows_val2pos.keys()
            nrows = len(rows_vals)
        
        if len(self.target_fields)==0:
            cols_field = ''
            ncols = 1
        else:
            cols_field = self.target_fields[0]
            cols_val2pos = mapping_of_values_to_pos(fieldvecs[cols_field])
            cols_vals = cols_val2pos.keys()
            ncols = len(cols_vals)

        # For convenience, make a new dictionary indexes by keys ('X', 'Y','C') rather than by actual fieldnames
        kvecs = {}
        for key,fieldname in self.graph_fields.items():
            kvecs[key] = fieldvecs[fieldname]

        mymarkers= ['o','s','>','<','^','v','d','p','h','g','+','x']
        mymarkerdict = {} # will map values of the 'M' field to marker symbols
        self.colors = ['b','g','r','c','y','k','m']
        self.C2idx = {} # will map values of the 'C' field to color index
        self.cmap = matplotlib.colors.ListedColormap(self.colors)

        # Numerize a number of fields
        if 'X' in kvecs:
            kvecs['X'] = to_numpy_float_array(kvecs['X'], missing_value = 0.)
        if 'Y' in kvecs:
            kvecs['Y'] = to_numpy_float_array(kvecs['Y'], missing_value = 0.)
        if 'S' in kvecs:
            kvecs['S'] = to_numpy_float_array(kvecs['S'], missing_value = 0.)
        if 'U' in kvecs:
            kvecs['U'] = to_numpy_float_array(kvecs['U'], missing_value = 0.)
        if 'V' in kvecs:
            kvecs['V'] = to_numpy_float_array(kvecs['V'], missing_value = 0.)
        if 'C' in kvecs: # transform it to real values indexed in self.cmap
            n = len(self.colors)
            for val in kvecs['C']:
                self.C2idx.setdefault(val, len(self.C2idx)%n)
            # print >>sys.stderr, self.C2idx
            kvecs['Cfloat'] = [ float(self.C2idx[val])/n for val in kvecs['C'] ]
        if 'M' in kvecs: # transform it to marker strings
            for val in kvecs['M']:
                mymarkerdict.setdefault(val, mymarkers[len(mymarkerdict)%len(mymarkers)])
            kvecs['M'] = [ mymarkerdict[val] for val in kvecs['M'] ]
            
        # make sure we have both X and Y
        if 'X' not in kvecs or 'Y' not in kvecs:
            if 'X' in kvecs:
                kvecs['Y'] = zeros(len(kvecs['X']))
            elif 'Y' in kvecs:
                kvecs['X'] = zeros(len(kvecs['Y']))
            else:
                raise ValueError("Scatter plot requires you to specify at least one of X or Y fields")
            
        for i in range(nrows):             
            newtitle = ''
            if rows_field=='':
                r_kvecs_part = kvecs
            else:
                newtitle += rows_field+'='+str(rows_vals[i])
                r_kvecs_part = self.take_part_of_fieldvecs(rows_val2pos[rows_vals[i]], kvecs)        
            
            if cols_field=='':
                pylab.subplot(nrows, ncols, 1+i)
                pylab.title(newtitle)
                self.scatter_plot_in_axes(r_kvecs_part)
            else:
                cols_val2pos = mapping_of_values_to_pos(r_kvecs_part[cols_field])                
                for cval,j in zip(cols_vals, range(ncols)):
                    if cval in cols_val2pos:
                        newtitle += cols_field+'='+str(cols_vals[j])
                        c_kvecs_part = self.take_part_of_fieldvecs(cols_val2pos[cval], r_kvecs_part)
                        pylab.subplot(nrows, ncols, 1+i*ncols+j)
                        pylab.title(newtitle)
                        self.scatter_plot_in_axes(c_kvecs_part)


        C2spec = [ (val, self.colors[self.C2idx[val]]) for val in self.C2idx ]
        C2spec.sort()
        self.display_colors_legend(self.graph_fields.get('C',''), C2spec)
        self.display_markers_legend(self.graph_fields.get('M',''), mymarkerdict)

        
    def scatter_plot_in_axes(self, kvecs, default_color='b', default_marker='o'):

        # make a useful all-zeros vector
        n = len(kvecs['X'])
        zer = zeros(n)

        X = kvecs.get('X',None)
        Y = kvecs.get('Y',None)
        U = kvecs.get('U',None)
        V = kvecs.get('V',None)

        # plot arrows
        if U is not None or V is not None:
            if V is None:
                V = zer
            elif U is None:
                U = zer
            pylab.quiver(X,Y,U,V,width=0.002, color='gray')
                

#         if 'M' in kvecs and 'C' in kvecs:
#             markers_v2pos = mapping_of_values_to_pos(kvecs['M'])
#             i = 0
#             for mval, mpositions in markers_v2pos.items():
#                 kvecs_m = self.take_part_of_fieldvecs(mpositions, kvecs)
#                 marker = markers[i%len(markers)]
                
#                 colors_v2pos = mapping_of_values_to_pos(kvecs_m['C'])
#                 j = 0
#                 for cval, cpositions in colors_v2pos.items():
#                     kvecs_mc = self.take_part_of_fieldvecs(cpositions, kvecs_m)
#                     color = colors[j%len(colors)]
#                     line = pylab.plot(kvecs_mc['X'],
#                                       kvecs_mc['Y'],
#                                       # s = kvecsp.get('S',20),
#                                       marker = marker,
#                                       color = color,
#                                       linestyle = 'None'
#                                       )
#                     if i==0:
#                         colors_legend_lines.append(line)
#                         colors_legend_labels.append(str(cval))
#                     if j==0:
#                         markers_legend_lines.append(line)
#                         markers_legend_labels.append(str(mval))
#                     j = j+1
#                 i = i+1

            
        # split on marker?        
        if 'M' in kvecs:
            val2pos = mapping_of_values_to_pos(kvecs['M'])
            for marker, positions in val2pos.items():
                kvecsp = self.take_part_of_fieldvecs(positions, kvecs)
                #print >>sys.stderr, "Len: ",len(kvecsp['X']),len(kvecsp['Cfloat'])
                #print >>sys.stderr, marker, ": ",kvecsp.get('C',default_color)
                line = pylab.scatter(kvecsp['X'],
                                     kvecsp['Y'],
                                     s = kvecsp.get('S',40),
                                     marker = marker,
                                     c = kvecsp.get('Cfloat',default_color),
                                     cmap = self.cmap,
                                     #norm = matplotlib.colors.NoNorm(vmin=0, vmax=1)
                                     vmin = 0.,
                                     vmax = 1.)
                
        else: # no markers, make a single scatter plot
#             msg = " x="+repr(kvecs['X'])\
#                   +"\r\n y="+repr(kvecs['Y'])\
#                   +"\r\n s="+repr(kvecs.get('S',20))\
#                   +"\r\n marker="+repr(default_marker)\
#                   +"\r\n c="+repr(kvecs.get('Cfloat',default_color))\
#                   +"\r\n";            
#             sys.stderr.write(msg)                  
            pylab.scatter(kvecs['X'],
                          kvecs['Y'],
                          s = kvecs.get('S',40),
                          marker = default_marker,
                          c = kvecs.get('Cfloat',default_color),
                          cmap = self.cmap,
                          #norm = matplotlib.colors.NoNorm(vmin=0, vmax=1)
                          vmin = 0.,
                          vmax = 1.)
                
        pylab.xlabel(self.graph_fields.get('X','no X'))
        pylab.ylabel(self.graph_fields.get('Y','no Y'))


        #if len(colors_legend_lines)!=0:
        #    pylab.legend(colors_legend_lines, colors_legend_labels, loc=1)

#         if len(markers_legend_lines)!=0:
#             pylab.legend(markers_legend_lines, markers_legend_labels, loc=4)

#         legend_lines = []
#         legend_lines.append(pylab.Line2D(range(10), range(10), linestyle='-', marker='o', color='b'))
#         legend_labels.append('aaaa')
#         legend_lines.append(pylab.Line2D(range(10), range(10), linestyle='-', marker='x', color='g'))
#         legend_labels.append('bbbb')
#         pylab.legend(legend_lines, legend_labels)

    def graphical_plot(self):
        menutxt = """
        ************************************************
        ** Choose the kind of graphical plot you want **
        ************************************************
          NOTE: THIS FEATURE IS STILL UNDER DEVELOPMENT
                AND NOT QUITE READY TO USE YET (Pascal)
                
          [1]: scatter plot (with optional horizontal and vertical arrows).
               Can use X, Y, C (color), M (marker), S (size), U, V.
               If present uses U,V for a horizontal and vertical arrow of different scales.
          [2]: scatter plot (with optional single arrow).
               Same as [1] but U,V are considered the relative coordinates
               of the tip of a single arrow and will have the same scale. 
        
          Or press ENTER to cancel
        """

        if "pylab" not in globals():
            global pylab
            import matplotlib.pylab as pylab

        self.graphical_scatter_plot()

#         k = self.display_fullscreen(menutxt,"12\n")
#         if( k==ord('\n') ):
#             return
    
#         if( k==ord('1') ):           
#         elif( k==ord('2') ):           
#             self.graphical_scatter_plot()


            
    def display_exception(self,exc, expression=''):
        ls= (["Error in search expression:\n"] +
             [ expression +'\n'] +
             traceback.format_exception(*exc) +
             ["\n(press any key to continue.)"])
        txt= ""
        for l in ls:
            txt= txt + l
        self.display_fullscreen(txt)


    def display_fullscreen(self,txt, check_key=""):
        """Displays the given text, and waits for a valid key press.
        The code of the pressed key is returned.
        Any key is considered valid if check_key is the empty string
        If check_key is a character string, then one of the
        specified keys must be pressed."""
        self.stdscr.erase()
        i = 1
        for line in txt.split('\n'):
            self.safeaddstr(i,2,line)
            i += 1
        self.stdscr.refresh();
        k = self.stdscr.getch();
        if check_key!="":
            allowed_codes = [ ord(c) for c in check_key ]
            while k not in allowed_codes:
                k = self.stdscr.getch();                            
        self.redraw()
        return k
        
    def search_previous(self):
        for i in xrange(self.current_i-1, -1, -1):
            if self.search_expression=='':
                val = self.getVal(i,self.search_field)
                if str(val) == self.search_value:
                    self.goto_line(i)
                    break
            else:
                v = dict(zip(self.table.fieldnames, self.getFullRow(i)))
                if eval(self.search_expression, globals(), v):
                    self.goto_line(i)
                    break

    def alphabetical_order(self):        
        selected_fieldnames = [ self.table.fieldnames[col] for col in self.selected_columns ]
        selected_fieldnames.sort()
        self.selected_columns = [ self.table.fieldnames.index(fname) for fname in selected_fieldnames ]
        self.redraw()

    def revert_to_original(self):
        self.selected_columns = range(self.table.width())
        self.selected_rows = None
        self.redraw()        

    def hideSelectedFields(self):
        if self.selected_fields != []:
            self.selected_columns = [ k for k in self.selected_columns if self.table.fieldnames[k] not in self.selected_fields ]
        else: # no selected fields: simply hide the current field           
            del self.selected_columns[self.current_j]
        self.selected_fields = []
        self.redraw()

    def find_field(self):
        fldname= self.input('Field name to find: ')
        good = False
        for j in xrange(self.table.width()):
            if fldname.lower() == self.fieldname(j).lower():
                self.goto_col(j)
                self.redraw()
                good = True
                break
        if not good:
            self.writebottom("Field '" + fldname + "' not found.")

    def event_loop(self):
        ret = None
        while not ret:
            k = self.stdscr.getch() 
            ret = self.handle_key_press(k)
        if self.statsthread:
            self.stopThread()
        return ret

    def handle_key_press(self, c):
        if c == ord('q') or c==27:
            return c
        elif c == ord('\t'):
            self.viewStatsTable()
        elif c == ord('.'):
            self.dot_mode = not self.dot_mode
            self.redraw()
        elif c == ord('t'):
            self.transpose()
        elif c == curses.KEY_UP:
            self.up()
        elif c == curses.KEY_DOWN:
            self.down()
        elif c == curses.KEY_LEFT:
            self.left()
        elif c == curses.KEY_RIGHT:
            self.right()      
        elif c == curses.KEY_NPAGE:
            self.pgdown()            
        elif c == curses.KEY_PPAGE:
            self.pgup()
        elif c == curses.KEY_HOME:
            self.home()
        elif c == curses.KEY_END:
            self.end()
        elif c == ord('s'):
            self.chooseStats()
            self.redraw()
        elif c == ord('l'):
            try: self.goto_line(int(self.input('Goto line #')))
            except: pass
        elif c == ord('='):
            self.search_field = self.current_j
            self.search_value = self.input('Search '+self.fieldname(self.current_j)+ ' = ')
            self.search_expression = ''
            self.search_next()
        elif c == ord('/'):
            self.search_field = self.current_j
            self.search_expression = self.input('Search expression ex: float(AGE)>3 : ').strip()
            if self.search_expression!='':
                self.search_next()
        elif c == ord('!'):
            self.filter()
        elif c == ord('N'):
            self.search_next()
        elif c == ord('P'):
            self.search_previous()
        elif c == ord('n'):
            self.next_field()
            self.redraw()
        elif c == ord('p'):
            self.previous_field()
            self.redraw()
        elif c == ord('a'):
            self.alphabetical_order()
        elif c == ord('o'):
            self.revert_to_original()
        elif c == ord('c'):
            self.select_constant_cols()
        elif c == ord('k'):
            self.hideSelectedFields()
        elif c == ord('\n'):
            self.hideFieldsNotSelected()
        elif c == ord('h'):
            self.display_help()
        elif c == ord('>'):
            self.set_field_dim(self.fieldwidth+1,self.fieldheight)
            self.redraw()
        elif c == ord('<'):
            self.set_field_dim(self.fieldwidth-1,self.fieldheight)
            self.redraw()
        elif c == ord(')'):
            self.sort_rows(self.current_j)
        elif c == ord('('):
            self.sort_rows(self.current_j, reverse=True)
        elif c == ord('x'):
            self.chooseAndExecuteShellCommand()
        elif c == ord('*'):
            self.clearStats()
            fieldname = self.fieldname(self.current_j)
            if self.conditioning_field == fieldname:
                self.conditioning_field = ''
            else:
                self.conditioning_field = fieldname
            self.redraw()
        elif c == ord('~'):
            self.clearStats()
            fieldname = self.fieldname(self.current_j)
            if self.weight_field == fieldname:
                self.weight_field = ''
            else:
                self.weight_field = fieldname
            self.redraw()
        elif c == ord('+'):
            self.clearStats()
            fieldname = self.fieldname(self.current_j)
            pos = bisect.bisect_left(self.target_fields, fieldname)
            if pos<len(self.target_fields) and self.target_fields[pos]==fieldname:
                del self.target_fields[pos]
            else:
                self.target_fields.insert(pos,fieldname)
            self.redraw()
        elif c == ord('f'):
            self.find_field()
        elif c == ord('m'):
            self.monetary_format= not self.monetary_format
            if self.monetary_format:
                locale.setlocale(locale.LC_NUMERIC, ('en_US','utf-8'))
            else:
                locale.setlocale(locale.LC_NUMERIC, self.orig_locale)
            self.redraw()
        elif c == ord('F'):
            self.full_shuffle_stats= not self.full_shuffle_stats

        elif c == ord(' '):
            self.selectField()
            self.redraw()

        elif chr(c) in 'XYCMSUVW':
            self.set_graph_field(chr(c))

        elif c == ord('G'):
            self.graphical_plot()

        elif c == ord('w'):
            self.saveSubTable()        

        else:
            curses.flash()
            self.display_help(c)

    def selectField(self):
        field = self.fieldname(self.current_j)
        if field in self.selected_fields:
            self.selected_fields.remove(field)
        else:
            self.selected_fields.append(field)

    def hideFieldsNotSelected(self):
        if self.selected_fields != []:
            self.j0 = 0
            self.current_j = 0
            self.selected_columns = [ self.table.fieldnames.index(fname) for fname in self.selected_fields]
            self.selected_fields= []
        self.redraw()

    def saveSubTable(self):
        orig_table= self.table.filepath()
        pytablecode= """
from plearn.table.table import *
result = SelectFields(openTable('%s'),%s)
        """ % (orig_table, [ self.table.fieldnames[col] for col in self.selected_columns ])

        new_tablename= self.input('File name for sub-table (.pytable): ').strip()
        if new_tablename != '':
            (base,ext)= os.path.splitext(new_tablename)
            if ext != '.pytable':
                new_tablename+= '.pytable'
            if os.path.exists(new_tablename):
                self.writebottom("File '" + new_tablename + "' already exists.")
                return
            self.stopThread()
            f= open(new_tablename, 'w')
            f.write(pytablecode)
            f.close()
            newtable = openTable(new_tablename)
            newtable.set_title('FILE: '+new_tablename)
            self.reinit(newtable)

    def viewStatsTable(self):
        if not self.stats:
            if self.conditioning_field: # we have a conditioning field
                self.initStats()
            else: # no conditioning field, try to load the most recently updated stats
                self.loadMostRecentStats()
            if not self.stats:
                return

        while 1:
            self.statslock.acquire()
            ns = self.stats.nsamples
            self.statslock.release()
            if ns<1:
                break
            currentvar = self.fieldname(self.current_j)
            condvar = self.conditioning_field
            valuenames = self.stats.extended_value_names()
            if not self.sumvar:
                self.sumvar = valuenames[0]
##             try:
##                 rmn = self.summarize_remove_n[currentvar]
##             except KeyError:
##                 rmn = -1
##                 self.summarize_remove_n[currentvar] = rmn
                
            self.statslock.acquire()
            stattab = self.stats.getCondTable(currentvar, condvar,
                                              self.sumvar, self.divvar,
                                              self.marginalize, self.minprob, self.combinvarsmode)
            self.statslock.release()

            if not self.statsview:
                self.statsview = StatsTableView(stattab, self.stdscr)
            else:
                self.statsview.setTable(stattab)
            l = self.table.length()
            progress = str(ns)+'/'+str(l)+'  [ '+str((ns*1000/l)*0.1)+'% ]'
            self.statsview.set_toprightinfo(progress)
            self.statsview.redraw()
            k = self.statsview.event_loop()

            if k==ord(' '):
                self.startThread()
            if k==ord('1'):
                if not self.sumvar:
                    self.sumvar = valuenames[0]
                else:
                    idx = valuenames.index(self.sumvar)+1
                    if idx>=len(valuenames):
                        idx = 0
                    self.sumvar = valuenames[idx]
            elif k==ord('2'):
                if not self.divvar:
                    self.divvar = valuenames[0]
                else:
                    idx = valuenames.index(self.divvar)+1
                    if idx>=len(valuenames):
                        self.divvar = ''
                    else:
                        self.divvar = valuenames[idx]
            elif k==ord('3'):
                self.marginalize = (self.marginalize+1)%4
            elif k==ord('4'):
                self.combinvarsmode = (self.combinvarsmode+1)%2
            elif k==ord('-'):
                # self.summarize_remove_n[currentvar] += 1
                if self.minprob<=0.:
                    self.minprob = 0.001
                else:
                    self.minprob *= 1.1
            elif k==ord('='):
                # rmn = self.summarize_remove_n[currentvar]                
                # self.summarize_remove_n[currentvar] = max(rmn-1,0)
                self.minprob /= 1.1               
                if self.minprob<0.001:
                    self.minprob = 0.
            elif k==ord('p'):
                self.previous_field()
            elif k==ord('n'):
                self.next_field()
            elif k==ord('s'):
                self.chooseStats()
            elif k==ord('q') or k==ord('\t'):
                break
        self.redraw()
        

class StatsTableView(TableView):

    #def __init__(self, table, stdscr):
    #    TableView.__init__(self, table, stdscr)

    def handle_key_press(self, k):
        if k in map(ord,"\tpns 1234-="):
            return k
        elif k in map(ord,"~+*"):
            pass # ignore this key
        else:
            return TableView.handle_key_press(self, k)

    
def curses_viewtable(stdscr, table):
    view = TableView(table, stdscr)
    view.redraw()
    view.event_loop()

def curses_showtable(stdscr, table):
    view = TableView(table, stdscr)
    view.redraw()

def viewtable(table):
    # table = CacheTable(table,100)
    curses.wrapper(curses_viewtable,table)

    

if __name__ == "__main__":
    viewtable(openTable(sys.argv[1]))
    
