from plearn.utilities.toolkit   import plural
from plearn.utilities.verbosity import vprint

class BasicStats:
    """Basic test routine statistics.

    This class and the tasks.TaskStatus are closely related and should
    eventually be integrated.
    """
    def __init__(self, type):
        self.test_to_directory = {}

        self.succeeded = []
        self.failed    = []
        self.skipped   = []

        self.type = type

    def current_stats(self):
        nsucceeded = len(self.succeeded)
        nfailed    = len(self.failed)
        nskipped   = len(self.skipped)

        cur = ( "Success%s: %d, " % (plural(nsucceeded, plur='es'), nsucceeded) +
                "Failure%s: %d, " % (plural(nfailed), nfailed) +
                "Skipped: %d" % nskipped
                )
        return "%s (/%d)" % (cur, nsucceeded+nfailed+nskipped)

        
    def print_stats(self):
        ntests     = len(self.test_to_directory)
        nsucceeded = len(self.succeeded)
        nfailed    = len(self.failed)
        nskipped   = len(self.skipped)

        if nsucceeded + nfailed + nskipped != ntests:
            raise RuntimeError( '\n+++ Some tests were not logged (su=%d + f=%d + sk=%d != %d).'
                                % (nsucceeded, nfailed, nskipped, ntests)
                                )        

        if ntests == 0:
            return ("No %s runned!" % self.type)

        stat_lines   = [ "%s:" % self.type ]

        indent      =     "    "        
        formatter = lambda s,n: " - %s: %d / %d" % (s,n,ntests)

        successes_str = formatter( "Success%s" % plural(nsucceeded, plur='es'), nsucceeded )

        stat_lines.extend( ["", successes_str, ""] )

        def array_formatter( array ):
            directory_to_test = {}
            for test in array:
                directory = self.test_to_directory[test]
                if directory_to_test.has_key( directory ):
                    directory_to_test[directory].append(test)
                else:
                    directory_to_test[directory] = [test]

            formatted_array = []
            for directory in directory_to_test.iterkeys():
                formatted_array.append( indent+directory )
                formatted_array.extend( [ indent+'  '+failed
                                          for failed in directory_to_test[directory] ] )
                formatted_array.append( "" )
            return formatted_array

        if nfailed > 0:
            stat_lines.append( formatter("Failure%s" % plural(nfailed), nfailed) )
            stat_lines.extend( array_formatter(self.failed) )
            stat_lines.append( "" )

        if nskipped > 0:
            stat_lines.append( formatter("Skipped", nskipped) )
            stat_lines.extend( array_formatter(self.skipped) )
            stat_lines.append( "" )
        
        vprint.highlight( stat_lines )
        
    def __repr__(self):
        return str(self)

    def set_status(self,  test_name, status):
        if status == "Succeeded":
            self.succeeded.append( test_name )

        elif status == "Failed":
            self.failed.append( test_name )

        elif status == "Skipped":
            self.skipped.append( test_name )

        else:
            raise ValueError(status) 
        
    def new_test(self, test_directory, test_name):
        self.test_to_directory[test_name] = test_directory
        
