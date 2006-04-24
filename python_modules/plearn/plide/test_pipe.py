import os, select, sys, time, threading


class Watcher( threading.Thread ):
    def __init__( self, watch_fd ):
        threading.Thread.__init__(self)
        self.watch_fd = watch_fd
        self.setDaemon(True)


    def run( self ):
        print >>sys.stderr, "Watcher started"
        (in_fds, foo, bar) = select.select([self.watch_fd], [], [])
        print >>sys.stderr, "Result from select: ", (in_fds, foo, bar)
        read = os.read(in_fds[0],1000)
        print >>sys.stderr, "Actually read:", read



(r,w) = os.pipe()
watcher = Watcher(r)
watcher.start()

to_write = "Que j'aime voir chere indolente"
os.dup2(w, 1)
print to_write
sys.stdout.flush()

print >>sys.stderr, "To write:", to_write
time.sleep(1)
