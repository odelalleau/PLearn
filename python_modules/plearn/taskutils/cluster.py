import os, shutil, signal, sys, time
import plearn.xperiments.Xperiment as Xperiment

from popen2                          import Popen4
from ArgumentsOracle                 import *
from plearn.utilities.toolkit        import command_output 
from plearn.pyplearn.PyPLearnObject  import PyPLearnObject

class Cluster(PyPLearnObject):
    class Defaults:            
        command_format           = 'cluster --execute "%s" --force --wait --duree=120h'
        max_nmachines            = 15
        sleep_time               = 90
        logdir_path              = "LOGS"
        provide_expdirs          = True
        popen_instances          = list

    def dispatch( self, program_call, arguments_oracle ):
        ## if not isinstance(arguments_oracle, ArgumentsOracle): raise TypeError

        ## Frees all tasks if a keyboard interrupt is caught
        try:
            ## The ArgumentsOracle is an iterator whose next() method returns a
            ## formatted string representing the program arguments.
            for arguments in arguments_oracle:

                if self.provide_expdirs: 
                    arguments = "%s expdir=%s" % ( arguments, Xperiment.generate_expdir() )
                    time.sleep( 1 ) ## Making sure the next expdir will be
                                    ## generated at another 'time', i.e. on
                                    ## another second                
                                           
                ## Using the arguments to build the cluster command and launch the process.
                self.dispatch_task( program_call, arguments )

                message = "Using %d machines or more; waiting ." % self.max_nmachines
                while ( machines_used_by_user() >= self.max_nmachines ):
                    sys.stderr.write(message)
                    message = "."
                    time.sleep( self.sleep_time )
                print

            ### Wait for all experiments completion
            still_running = machines_used_by_user()
            while ( still_running > 0 ):
                sys.stderr.write( "\nWaiting for completion: %d tasks still running.\n"
                                  % still_running
                                  )
                time.sleep( self.sleep_time )
                still_running = machines_used_by_user()

        except KeyboardInterrupt:
            sys.stderr.write("\nInterrupted by user.\n")
            self.free( )

    def dispatch_task( self, program_call, arguments ):
        """Using the arguments to build the cluster command and launch the process.

        Note that this method does not manage keyboard interrupts. These
        are managed in dispatch().
        """

        ## Building the raw command and, afterwards, the cluster command
        ## from the program_call and stringified arguments
        raw_command = self.join_prog_and_arguments( program_call, arguments )
        cluster_cmd = cluster_command( raw_command, self.logdir_path, self.command_format )

        print "Launching: %s\n--" % raw_command
        ## os.system( "%s &" % cluster_cmd )
        self.popen_instances.append( Popen4(cluster_cmd) )        
        print cluster_cmd

    def free( self ):
        try:
            cluster_free() ## The following currently doesn't work for
                           ## lisa's cluster: cluster_free is a patch
            
            for task in self.popen_instances:
                try:
                    os.kill( task.pid, signal.SIGTERM )
                except OSError:
                    pass
            sys.stderr.write("\nDone.\n")
        except KeyboardInterrupt:
            sys.stderr.write( "\nCan not interrupt Cluster.free() please wait...\n" )
            self.free( )

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

def cluster_free( ):
    cl = command_output( 'cluster --list | grep %s | grep -v cpu'
                         % os.getenv("USER")
                         )
    nfree        = len(cl)
    progress_bar = "[%s]" % "-"*nfree
    sys.stderr.write( "Freeing %d tasks\n%s\n[" %
                      (nfree, progress_bar)
                      )

    for reservation in cl:
        index  = reservation.find(":")
        res_id = int(reservation[:index])

        os.system("cluster --libere --id=%d" % res_id)
        sys.stderr.write(".")

    sys.stderr.write("]\n")        

def machines_used_by_user():
    cl = command_output( 'cluster --list | grep "cpu.*%s"'
                         % os.getenv("USER")
                         )
    return len(cl)

if __name__ == '__main__':
    raise NotImplementedError("Please update.")

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
                       logdir_path    = "LOGS_MKDIR" )    
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
    
