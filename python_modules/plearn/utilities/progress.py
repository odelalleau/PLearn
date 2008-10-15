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
    out = sys.stderr

    def write(str):
        StdoutProgressBar.out.write(str)
        StdoutProgressBar.out.flush()
    write = staticmethod(write)
        
    def __init__(self, title, n):
        self.n = n
        self.pos = 0
        self.title = title
        self.closed = False
        titlestr = ' '+title+' ('+str(n)+') '
        npoints = StdoutProgressBar.npoints
        if len(titlestr)>npoints:
            titlestr = titlestr[0:npoints]
        leftn = (npoints-len(titlestr))//2
        rightn = npoints-leftn-len(titlestr)
        StdoutProgressBar.write('['+'-'*leftn+titlestr+'-'*rightn+']\n')
        StdoutProgressBar.write('[')
            
    def update(self, pos):
        if not self.closed:
            npoints = StdoutProgressBar.npoints
            oldcharpos = min(npoints, int(self.pos*npoints/max(1, self.n-1)))
            newcharpos = min(npoints, int(pos*npoints/max(1, self.n-1)))
            nchars = newcharpos-oldcharpos
            if nchars>0:
                StdoutProgressBar.write('.'*nchars)
            if pos>=self.n-1:
                StdoutProgressBar.write(']\n')
                self.closed = True
            self.pos = pos
            

class LineOutputProgressBar(StdoutProgressBar):
    
    def __init__(self, title, n):
        self.n = n
        self.pos = 0
        self.title = title
        self.closed = False
        titlestr = ' '+title+' ('+str(n)+') '
        StdoutProgressBar.write('In progress: '+titlestr+'\n')
            
    def update(self, pos):
        if not self.closed:
            npoints = StdoutProgressBar.npoints
            oldcharpos = min(npoints, int(self.pos*npoints/(self.n-1)))
            newcharpos = min(npoints, int(pos*npoints/(self.n-1)))
            nchars = newcharpos-oldcharpos
            if nchars>0:
                StdoutProgressBar.write(self.title + ': ' + str(pos) + '/' + str(self.n)
                                        + '(' + str(float(pos)*100./float(self.n)) +'%)\n')
            if pos>=self.n-1:
                StdoutProgressBar.write('Finished '+self.title+': '+str(pos)+'/'+str(self.n)+' (100%)\n')
                self.closed = True
            self.pos = pos

PBar = StdoutProgressBar

