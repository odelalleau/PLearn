import os, select, shutil, signal, sys, time
import plearn.xperiments.Xperiment as Xperiment

from popen2                          import Popen3, Popen4
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
        self.popen_instances.append( Popen4(cluster_cmd) )  ## Replace by ClusterTask.launch()       
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

class EmptyTaskListError( Exception ): pass
    
class ClusterTask( Popen3 ):
    _outfiles_map = { }
    _errfiles_map = { } 

    def select( cls ):
        """Selecting a completed task."""
        if len( _outfiles_map ) == 0:
            raise self.EmptyTaskListError()
            
        (iwtd, owtd, ewtd) = select.select( _outfiles_map.keys() + _errfiles_map.keys(), [], [] )
        f = iwtd[0]        
        if f in _outfiles_map:
            completed = _outfiles_map[f]
        elif f in _errfiles_map:
            completed = _errfiles_map[f]
        else:
            raise RuntimeError("Invalid file descriptor in ClusterTask.select().")

        return completed        
    select = classmethod( select )

    def task_count( cls ):
        """Return the number of uncompleted tasks."""
        return len( cls._outfiles_map )
    task_count = classmethod( task_count )
 
    def __init__( self, program, arguments, logdir = None ):
        ## Some more instance variables
        self.command = " ".join([ program, arguments ])

        logfile = None
        if os.path.isdir( logdir ):
            logfile = self.command            
            for s in [" ", ".", "/"]:
                logfile = logfile.replace(s, "_")
            logfile = os.path.join( logdir, logfile )
                
            self.command = "%s >& %s" % ( self.command, logfile )                         
        self.logfile = logfile

        ## Process management
        Popen3.__init__( self, command, True, 10000 )
        ClusterTask._outfiles_map[ self.fromchild ] = self
        ClusterTask._errfiles_map[ self.childerr  ] = self

    def close( self ):
        """Removing it from the maps and closing the files."""
        ## Avoiding defunct and zombies
        try:
            self.wait()
        except OSError:
            pass
        
        ## Removing it from the maps
        del ClusterTask._outfiles_map[ self.fromchild ]
        del ClusterTask._errfiles_map[ self.childerr  ]

        ## Closing files
        self.fromchild.close()
        self.tochild.close()
        self.childerr.close()        
        

class PyPLearnTask( ClusterTask ):
    def __init__( self, program, arguments, logdir = None ):
        self.expdir = Xperiment.generate_expdir()
        time.sleep( 1 ) ## Making sure the next expdir will be generated at
                        ## another 'time', i.e. on another second

        arguments.append( "expdir=%s" % self.expdir )
        ClusterTask.__init__( self, program, arguments, logdir )        


## Under Development
class NewCluster(PyPLearnObject):
    class Defaults:            
        command_format           = 'cluster --execute "%s" --force --wait --duree=120h'
        max_nmachines            = 15
        logdir                   = "LOGS"
        pyplearn_tasks           = True
        running_tasks            = list

    def dispatch( self, program, arguments_oracle ):
        """Frees all tasks if a keyboard interrupt is caught.

        @param arguments_oracle: An iterator whose next() method returns a
        formatted string representing the program arguments.
        """
        if self.logdir is not None:
            if not os.path.exists( self.logdir ):
                os.mkdir( self.logdir )

        try:
            self.__dispatch( program, arguments_oracle )
        except KeyboardInterrupt:
            sys.stderr.write("\nInterrupted by user.\n")
            self.free( )

    def __dispatch( self, program, arguments_oracle ):
        ## Parallel dispatch; respecting max_nmachines
        for arguments in arguments_oracle:
            if self.pyplearn_tasks:
                PyPLearnTask( program, arguments, self.logdir )
            else:
                ClusterTask( program, arguments, self.logdir )

            if ClusterTask.task_count( ) == self.max_nmachines:
                print >>sys.stderr, "Using %d machines or more; waiting..." % self.max_nmachines
                completed = ClusterTask.select()
                completed.close()                    

        ## Wait for all experiments completion
        print >>sys.stderr, "[%s]" % ( "-" * ClusterTask.task_count() )
        sys.stderr.write("[")
        while True:
            try:
                completed = ClusterTask.select()
                completed.close()
                sys.stderr.write(".")
            except EmptyTaskListError:
                break
        sys.stderr.write("]\n")

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
    progress_bar = "[%s]" % ("-"*nfree)
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
    
