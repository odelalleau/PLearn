# xp_workbench.py
# Copyright (C) 2008 by Nicolas Chapados
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

"""Provide a console-like object that supports stdin/stdout redirection.
"""

import fcntl
import os
import sys
import threading
import time
import wx
from   select import select

from enthought.traits.api       import *
from enthought.traits.ui.api    import CodeEditor, Group, Item, View
from enthought.traits.ui.menu   import NoButtons


## Raw stdout/stderr as global objects, for ease of debugging (python streams)
_raw_stdout = None
_raw_stderr = None


class ConsoleLogger(HasTraits):

    ## Title of logger object
    title = "Output"

    ## Contents of the actual logging window
    data = Str

    ## Whether this logger is currently active
    is_active = false

    ## Class variable containing which logger is currently active; cannot
    ## have more than one active logger simultaneously.  NOTE: we assume
    ## that this variable is only accessed within the UI thread.
    active_logger = None

    ## Default view
    traits_view = View(Item('data~', editor=CodeEditor(foldable=False), show_label=False))

    def activate_stdouterr_redirect(self, streams_to_watch={}):
        """Redirect standard output and error to be sent to the log pane
        instead of the console.
        """
        ## If we already have an active logger, raise
        if ConsoleLogger.active_logger is not None:
            raise RuntimeError, "Trying to activate redirection for a ConsoleLogger " +\
                  "while it is already active for a different one"
        ConsoleLogger.active_logger = self

        ## Ensure that any pending writes are expelled before redirection
        ## (Python only)
        sys.stdout.flush()
        sys.stderr.flush()
        
        ## Redirect standard output and standard error to display to the
        ## log pane area.  Keep around old stdout/stderr in order to
        ## display debugging messages.  They are called, respectively,
        ## raw_stdout and raw_stderr (Python file objects); make them
        ## global for ease of debugging.
        global _raw_stdout, _raw_stderr
        if _raw_stdout is None:
            old_stdout_fd = os.dup(sys.stdout.fileno())
            _raw_stdout   = os.fdopen(old_stdout_fd, 'w')
        if _raw_stderr is None:
            old_stderr_fd = os.dup(sys.stderr.fileno())
            _raw_stderr   = os.fdopen(old_stderr_fd, 'w')

        # print >>sys.stderr, "Original stderr"
        # print >>_raw_stderr, "Redirected stderr"
        # sys.stderr.flush()
        # _raw_stderr.flush()
        
        (self.stdout_read, self.stdout_write) = os.pipe()
        (self.stderr_read, self.stderr_write) = os.pipe()
        os.dup2(self.stdout_write, sys.stdout.fileno())
        os.dup2(self.stderr_write, sys.stderr.fileno())
        
        out_flags = fcntl.fcntl(self.stdout_read, fcntl.F_GETFL)
        err_flags = fcntl.fcntl(self.stderr_read, fcntl.F_GETFL)
        fcntl.fcntl(self.stdout_read, fcntl.F_SETFL, out_flags | os.O_NONBLOCK)
        fcntl.fcntl(self.stderr_read, fcntl.F_SETFL, err_flags | os.O_NONBLOCK)

        streams_to_watch.update({ self.stdout_read:'stdout',
                                  self.stderr_read:'stderr' })

        ## We're going to create a thread whose purpose is to indefinitely
        ## wait (using 'select') on the stdout/stderr file descriptors.
        ## When text arrives read it and update the log window.
        class _ListenerThread(threading.Thread):
            def run(inner_self):
                listen_fds = streams_to_watch.keys()
                eof = False
                while not eof and self.is_active:   # Note: traits are thread-safe
                    ready = select(listen_fds, [], [])
                    for fd in listen_fds:
                        if fd in ready[0]:
                            chunk = os.read(fd, 65536)
                            if chunk == '':
                                eof = True
                            self.data += chunk
                    if eof:
                        break
                    select([],[],[],.1) # Give a little time for buffers to fill

        listener = _ListenerThread()
        self.is_active = True
        listener.start()
        

    def desactivate_stdout_err_redirect(self):
        """Bring back redirected file descriptors to their original state
        and relinquish control as the active logger.
        """
        global _raw_stdout, _raw_stderr
        if ConsoleLogger.active_logger == self:
            ## Wait a little bit to ensure that the last data printed to
            ## stdout/stderr has had time to be processed by the logging
            ## thread
            sys.stdout.flush()
            sys.stderr.flush()
            time.sleep(0.1)
            
            if _raw_stdout is not None:
                os.dup2(_raw_stdout.fileno(), sys.stdout.fileno())
                _raw_stdout = None

            if _raw_stderr is not None:
                os.dup2(_raw_stderr.fileno(), sys.stderr.fileno())
                _raw_stderr = None

            ConsoleLogger.active_logger = None
            self.is_active = False


#####  Test Case  ###########################################################

if __name__ == "__main__":
    class X(HasTraits):
        log = Instance(ConsoleLogger,())
        go  = Button("Go!")

        def _go_fired(self):
            self.log.activate_stdouterr_redirect()
            print "This is printed to stdout but should go to logger"
            print "Another print"
            print >>sys.stderr, "This one goes to stderr"
            self.log.desactivate_stdout_err_redirect()
            print "Normal print to stdout"
            print >>sys.stderr, "Normal print to stderr"

        traits_view = View(Group(Item('log@'), Item('go'),
                                 show_labels=False),
                           resizable=True, buttons=NoButtons, width=0.5, height=0.5)

    X().configure_traits()
