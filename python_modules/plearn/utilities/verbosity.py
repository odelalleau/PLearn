class VerbosityPrint:
    def __init__(self, verbosity, default_priority=0):
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

vprint = __global_vprint()

def set_vprint(vpr):
    if not isinstance(vpr, VerbosityPrint):
        raise TypeError
    __global_vprint.vpr = vpr
