import os, shutil, time

from ArgumentsOracle                 import *
from plearn.utilities.toolkit        import command_output 
from plearn.pyplearn.PyPLearnObject  import PyPLearnObject

class Cluster(PyPLearnObject):
    class Defaults:            
        expdir_pattern           = "expdir_.*"
        command_format           = 'cluster --execute "%s" --force --wait --duree=120h'
        max_nmachines            = 15
        sleep_time               = 120
        logdir_path              = "LOGS"
        wait_for_expdir_creation = True

    def dispatch( self, program_call, arguments_oracle ):
        if not isinstance(arguments_oracle, ArgumentsOracle): raise TypeError

        expdir_count = None
        if self.wait_for_expdir_creation:
            expdir_count = count_expdirs( self.expdir_pattern )

        ## The ArgumentsOracle is an iterator whose next() method returns a
        ## formatted string representing the program arguments.
        for arguments in arguments_oracle:

            ## Building the raw command and, afterwards, the cluster command
            ## from the program_call and stringified arguments
            raw_command = self.join_prog_and_arguments( program_call, arguments )
            cluster_cmd = cluster_command( raw_command, self.logdir_path, self.command_format )

            print "Launching %s on cluster" % raw_command
            os.system( cluster_cmd )

            ## This hack is a turnaround to the cluster command bug of
            ## possibly returning before the task is actually launched
            if self.wait_for_expdir_creation:
                current_count = count_expdirs( self.expdir_pattern )
                while current_count == expdir_count:
                    time.sleep(2)
                    current_count = count_expdirs( self.expdir_pattern )

                    
            while ( machines_used_by_user() >= self.max_nmachines ):
                time.sleep( self.sleep_time )
            print

        ### Wait for all experiments completion
        while ( machines_used_by_user() > 0 ):
            time.sleep( self.sleep_time )

    def join_prog_and_arguments( self, program_call, arguments ):
        return " ".join([ program_call, arguments ])
    
def cluster_command( raw_command, logdir_path = None, format = Cluster.Defaults.command_format ):
    processed_cmd = raw_command

    ## Processing the raw command and piping the output in a logfile
    if logdir_path is not None:
        if not os.path.exists( logdir_path ):
            os.mkdir( logdir_path )
        
        logfile_name = raw_command
        for s in [" ", ".", "/"]:
            logfile_name = logfile_name.replace(s, "_")
        processed_cmd = "%s >& %s" % (
            raw_command, os.path.join( logdir_path, logfile_name)
            )

    ## Enclosing the command in a cluster call
    cluster_cmd = format % processed_cmd
    return cluster_cmd

def count_expdirs( expdir_pattern = Cluster.Defaults.expdir_pattern ):
    xpdirs = command_output( 'ls -1 | grep "%s"' % expdir_pattern )
    return len(xpdirs)

def machines_used_by_user():
    cl = command_output( 'cluster --list | grep "cpu.*%s"'
                         % os.getenv("USER")
                         )
    return len(cl)

if __name__ == '__main__':
    pairs = []
    pairs.append(( "first",  [1,2,3] ))
    pairs.append(( "second", 10 ))
    pairs.append(( "third",  [100, 200] ))

    cporacle = CrossProductOracle( pairs )
    print "CrossProductOracle( %s )" % pairs

    ## Simply echoing
    cluster = Cluster( command_format = "%s", wait_for_expdir_creation=False )
    cluster.dispatch( "echo", cporacle )
    
    print "## End of basic dispatch"

    ## Creating a directory
    cporacle.reset()
    cporacle.arguments_joining_token = "_"    

    cluster = Cluster( command_format = "%s",
                       logdir_path    = "LOGS_MKDIR",
                       expdir_pattern = "first=.*"  )    
    cluster.dispatch( "mkdir", cporacle )
    print "## End of mkdir dispatch"

    ## Creating a directory
    cporacle.reset()
    dsoracle = DirectorySplittedOracle( cporacle, "first" )

    cluster  = Cluster( command_format = "%s",
                        logdir_path    = "LOGS_DSORACLE",
                        wait_for_expdir_creation=False  )
    cluster.dispatch( "touch", dsoracle )
    
    print "## End of dsoracle dispatch"
    
