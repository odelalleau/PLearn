import sys

class ProgressBar:
    def __init__(self, title, n):
        self.n = n
        self.pos = 0
        self.title = title
    
    def update(self, pos):
        self.pos = pos
        
    def close(self):
        self.update(self.n-1)

class StdoutProgressBar(ProgressBar):
    
    npoints = 100
    out = sys.stdout

    def write(str):
        StdoutProgressBar.out.write(str)
        StdoutProgressBar.out.flush()
    write = staticmethod(write)
        
    def __init__(self, title, n):
        self.n = n
        self.pos = 0
        self.title = title
        titlestr = ' '+title+' ('+str(n)+') '
        npoints = StdoutProgressBar.npoints
        if len(titlestr)>npoints:
            titlestr = titlestr[0:npoints]
        leftn = (npoints-len(titlestr))//2
        rightn = npoints-leftn-len(titlestr)
        StdoutProgressBar.write('['+'-'*leftn+titlestr+'-'*rightn+']\n')
        StdoutProgressBar.write('[')
            
    def update(self, pos):
        npoints = StdoutProgressBar.npoints
        oldcharpos = int(self.pos*npoints/(self.n-1)) 
        newcharpos = int(pos*npoints/(self.n-1))
        nchars = newcharpos-oldcharpos
        if nchars>0:
            StdoutProgressBar.write('.'*nchars)
        if pos>self.pos and pos==self.n-1:
            StdoutProgressBar.write(']\n')
        self.pos = pos

PBar = StdoutProgressBar

