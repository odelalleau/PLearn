import os, shutil, string
import plearn.utilities.toolkit as toolkit

class Reporter:
    def __init__(self, directory=None):
        if directory is not None:
            self.open_reporter( directory )
        
    def open_reporter(self, directory):
        if hasattr(self, 'directory'):
            raise RuntimeError("Must call open_reporter only once.")

        self.directory = os.path.abspath( directory )

        ## What to do with old reports
        if os.path.exists( self.directory ):
            answer = None
            while not answer in ['a', 'k', 'r']:
                answer = raw_input(
                    "Directory %s exists: abort (a), archive (k) or remove (r): "
                    % self.directory
                    )

            if answer == 'a':
                raise KeyboardInterrupt
            elif answer == 'k':
                toolkit.timed_version_backup( self.directory )
            elif answer == 'r':
                shutil.rmtree( self.directory )

        ## Finally, ensure that a fresh version of the directory exists
        os.makedirs( self.directory )

    def new_report(self, report_name, lines):
        if not hasattr(self, 'directory'):
            raise RuntimeError("Must call open_reporter prior to use new_report.")

        report_path = os.path.join(self.directory, report_name)
        report_file = open(report_path, 'w')
        for line in lines:
            report_file.write( line )
        report_file.close()

            
class VerbosityPrint( Reporter ):
    def __init__(self, verbosity, default_priority=0, report_directory=None):
        Reporter.__init__(self, report_directory)
        
        self.verbosity = int(verbosity)
        self.default_priority = default_priority

    def __call__(self, msg, priority=None):
        if priority is None:
            priority = self.default_priority

        if self.verbosity >= priority:
            if hasattr(self, 'file'):
                self.file.write(msg)

            if hasattr(self, 'output'):
                self.output.append( msg )
                
            print msg
            
    def add_file(self, file_name):
        if file_name:
            self.file = open(file_name, 'w')

    def close(self):
        if hasattr(self, 'file'):
            self.file.close()
        if hasattr(self, 'output'):            
            return self.output
        return None

    def keep_output(self):
        self.output = []


    def highlight(self, lines, highlighter = "*"):
        max_len = -1
        for line in lines:
            l = len(line)
            if l > max_len:
                max_len = l

        prefix      = highlighter+"  "
        suffix      = "  "+highlighter
        extra_len   = len(prefix) + len(suffix)
        formatter   = lambda line: prefix + string.ljust(line, max_len) + suffix 

        star_line  = string.join([ highlighter for i in range(0, max_len+extra_len) ], '') 
        empty_line = formatter(string.join([ " " for i in range(0, max_len) ], ''))

        formatted = [star_line, empty_line]
        formatted.extend( [ formatter(line) for line in lines ] )
        formatted.extend( [empty_line, star_line] )

        ## Calling the print method
        self( "" )
        self( string.join( formatted, '\n' ) )
        self( "" )

                  
class __global_vprint:
    vpr = None
    def __call__(self, msg, priority=None):
        if self.vpr is not None:
            self.vpr(msg, priority)

    def close(self):
        if self.vpr is not None:
            cl = self.vpr.close()
            self.vpr = None 
            return cl
        return []

    def __getattr__(self, attr):
        if self.vpr is not None:
            return getattr(self.vpr, attr)

vprint = __global_vprint()

def set_verbosity(level):
    set_vprint( VerbosityPrint(level) )

def set_vprint(vpr):
    if not isinstance(vpr, VerbosityPrint):
        raise TypeError
    __global_vprint.vpr = vpr

