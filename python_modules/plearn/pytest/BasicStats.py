
from toolkit import plural

class BasicStats:
    def __init__(self, test_type):
        self.nb_of_tests = 0

        self.succeeded = 0
        self.skipped   = 0
        self.failed    = []

        self.test_type = test_type
        
    def __str__(self):
        nfailed = len(self.failed)
##         if nfailed+self.succeeded != self.nb_of_tests:
##             raise RuntimeError('Some tests were not logged.')
        
        if self.nb_of_tests == 0:
            return ("No %s runned!" % self.test_type)
        
        mystr = ( '+++ %s success: %d / %d\n'
                  '    Failure%s: %s\n'
                  % (self.test_type, 
                     self.succeeded, self.nb_of_tests,
                     plural(nfailed), str(self.failed))
                  )
        if self.skipped > 0:
            mystr = '%s    Skipped: %d\n' % (mystr, self.skipped)

        return mystr
        
    def __repr__(self):
        return str(self)

    def success(self):
        self.succeeded += 1

    def skip(self):
        self.skipped += 1

    def failure(self, name):
        self.failed.append( name )

    def new_test(self):
        self.nb_of_tests += 1
