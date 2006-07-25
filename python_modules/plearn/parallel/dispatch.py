import inspect, operator, os, signal, sys, time

from socket                         import getfqdn
from plearn.utilities.ppath         import get_domain_name
from plearn.utilities.moresh        import *
from plearn.utilities.Bindings      import *
from plearn.pyplearn.context        import generateExpdir
from plearn.pyplearn.PyPLearnObject import PyPLearnObject, PLOption
from plearn.xp.Experiment           import inexistence_predicate, Experiment

__all__ = [
    # Variables
    "Task",
    
    # Functions
    "get_ssh_machines", "launch_task", "set_logdir",

    # Classes
    "ArgumentsOracle", "Dispatch"
    ]


#######  Module Variables  ####################################################

#
# Definitions for the different known clusters
#  These could be moved in a config file...
#  ( cluster.config in .plearn )
#
TASK_TYPE_MAP    = { 'apstat.com':       'SshTask',
                     'iro.umontreal.ca': 'ClusterTask'
                     }

# Used only for clusters of type 'ssh'.
SSH_MACHINES_MAP = { 'apstat.com': [ # 'embla',
                                     'inari',
                                     'inari',
                                     'inari',
                                     'inari',
                                     'kamado',
                                     'kamado',
                                     'kamado',
                                     'kamado',
                                     'loki',
                                     'odin',
                                     'midgard',
                                     # 'valhalla',
                                     'vili'
                                     ],

                     'iro.umontreal.ca' : [ 'lhmm',    'lknn',    'lmfa',      'lmlp',
                                            'lsom',    'lsvm',    'currie',    'dirac',
                                            'fermi',   'plank',   'einstein'
                                            ]                         
                     }

# To override the default of 2
MAX_LOADAVG = { 'inari'  : 6 ,
                'kamado' : 6 }

LOGDIR      = None  # May be set by set_logdir()
DOMAIN_NAME = get_domain_name()


#######  To be assigned to a subclass of TaskType when these will be declared
Task = None


#######  Module Functions  ####################################################

################################################################################
# KNOWN ISSUE: The current way of managing machines is slightly hackish
# with regard to load average. Here we have to multiply instances of the
# name of a machine by ist MAX_LOADAVG due the remove in launch_args()
#
#           self.host = self.available_machines().next()
#           self._machines.remove( self.host )
# 
################################################################################
def get_ssh_machines():
    from random import shuffle
    machines = []
    for m in SSH_MACHINES_MAP[ DOMAIN_NAME ]:
        machines.extend( [m]*MAX_LOADAVG.get(m,2) )        
    shuffle( machines )
    return machines

def launch_task( argv, wait = False ):
    # global Task
    # if Task is None:
    #     Task = globals()[ TASK_TYPE_MAP[ DOMAIN_NAME ] ]
    assert Task is not None
    task = Task( argv )
    task.launch( wait )

def set_logdir( logdir ):
    """Instead of writing to stdout, tasks will be logged in a file within I{logdir}."""
    global LOGDIR
    LOGDIR = logdir


#######  Module Classes  ######################################################

class ArgumentsOracle( list ):
    def build(cls, oracle_struct):
        if isinstance(oracle_struct, Bindings):
            list_of_bindings = oracle_struct.explode_values()
        else:
            try:
                list_of_bindings = Bindings(oracle_struct).explode_values()
            except Exception, ex:
                raise TypeError(ex)
        return list_of_bindings
    build = classmethod(build)
    
    def __init__( self, *args ):
        list.__init__(self, reduce(operator.add, [ self.build(a) for a in args ]))

class EmptyTaskListError( Exception ): pass
class EmptyMachineListError( Exception ): pass
    
class TaskType:
    _child_processes = {}

    def count( cls ):
        """Return the number of uncompleted tasks."""
        return len( cls._child_processes )
    count = classmethod( count )

    def kill_all_tasks( cls ):
        for pid in cls._child_processes:
            try:
                os.kill( pid, signal.SIGTERM )
            except OSError:
                pass            
    kill_all_tasks = classmethod( kill_all_tasks )

    def n_available_machines( cls ):
        """Returns the number of machines currently available for cluster job dispatch."""
        avail = 0
        for am in cls.available_machines():
            avail += 1
        return avail
    n_available_machines = classmethod( n_available_machines )

    def select( cls ):
        """Selecting a completed task."""
        if cls.count() == 0:
            raise EmptyTaskListError()
            
        pid, exit_status = os.wait()

        completed = cls._child_processes.pop( pid )
        completed.free()

        return completed
    select = classmethod( select )

    #
    # Instance methods
    #
    
    def __init__( self, argv ):
        self.argv    = argv
        self.logfile = None

        # Logging management
        if LOGDIR and os.path.isdir( LOGDIR ):
            raise NotImplementedError('Bugs in logging...') 
            
            self.logfile = "_".join( self.argv )            

            for s in [" ", ".", "/", ';']:
                self.logfile = self.logfile.replace(s, "_")

            self.logfile = os.path.join( LOGDIR, self.logfile )
            self.argv.extend([ ">&", self.logfile ])

        
    def launch( self, wait ):
        """Launch process on an available machine"""
        try:
            argv     = self.launch_args( )

            # Always using no wait to get the process id...
            self.pid = os.spawnvp( os.P_NOWAIT, argv[0], argv )        
            self._child_processes[ self.pid ] = self
            
            if wait:
                self.select( )

        except EmptyMachineListError:
            print >>sys.stderr, "Waiting for a machine to be freed..."
            try:
                self.select( )
            except EmptyTaskListError:
                time.sleep(15)      
            self.launch(wait)
            


class SshTask( TaskType ):

    _machines = get_ssh_machines()
    
    def available_machines( cls ):
        for m in cls._machines:
            max_loadavg = MAX_LOADAVG.get(m, 2)
            
            p = os.popen('ssh -x %s cat /proc/loadavg' % m)
            line = p.readline()
            loadavg = float(line.split()[1])

            if loadavg < max_loadavg:
                yield m
    available_machines = classmethod( available_machines )

    #
    # Instance methods
    #

    def launch_args( self ):
        # Get the first available machine
        try:
            self.host = self.available_machines().next()
            self._machines.remove( self.host )
        except StopIteration:
            raise EmptyMachineListError
            
        return ['ssh', '-x', self.host, 'cd', os.getcwd(), ';', 'nice'] + self.argv

    def free( self ):
        self._machines.append( self.host )

class ClusterTask( TaskType ):
    def available_machines( cls ):
        p = os.popen('cluster --charge')
        for line in p:
            if line.find('en panne') == -1:
                # If the machine is not free, the name of the user is found
                # between the first comma and the first colon.
                comma = line.find(',')
                colon = line.find(':')
                # If one of these asserts fail, the format of the cluster --charge
                # output changed. Panic.
                assert comma != -1
                assert colon != -1
                if not line[comma+1:colon].strip():
                    # The name of the machine is found before the first comma
                    yield line[:comma]
    available_machines = classmethod( available_machines )

    #
    # Instance methods
    #

    def launch_args( self ):
        return ['cluster', '--execute', self.argv[0], '--force',
                '--wait', '--duree=120h', '--'] + self.argv[1:]

    def free( self ):
        pass

#######  Assigning Task  ######################################################
#######  Now that TaskType subclasses are all declared, Task can be assigned
    
Task = globals()[ TASK_TYPE_MAP[ DOMAIN_NAME ] ]

class RejectedByPredicate( Exception ): pass

## Under Development
_quoted = lambda L: ['"%s"'%elem for elem in L]
class Dispatch( PyPLearnObject ):
    # The name of the program to invoke
    program                  = PLOption(None)

    # If provided, will always be the first argument
    script                   = PLOption(str)

    # These are args to be provided for each call to program in
    # addition the args returned by the oracle. Must be a string or a
    # list of strings.
    constant_args            = PLOption(list)

    # Either "expkey" or "named_args"
    protocol                 = PLOption("expkey") 

    max_nmachines            = PLOption(6)
    logdir                   = PLOption(None) #"LOGS"

    # Path to the directory where experiments are to be loaded
    expdir_root              = PLOption(None) # Default: os.getcwd()

    allow_unexpected_options = lambda self : False
    def __init__( self, oracles=None,
                  _predicate=inexistence_predicate, **overrides ):
        PyPLearnObject.__init__( self, **overrides )
        self._predicate = _predicate

        # Append current path to PYTHONPATH
        if self.expdir_root is None:
            self.expdir_root = os.getcwd()
        self.__check_constant_args()

        if oracles:                    
            self.start( *oracles )

    def __check_constant_args( self ):
        if isinstance( self.constant_args, str ): 
            self.constant_args = [ self.constant_args ]
        assert isinstance( self.constant_args, list )
        
    def getArguments( self, argument_bindings ):
        """Parses for special keywords and join keys to values.

        Special keywords:
          1) _program_        -> self.program
          2) _script_         -> self.script
          3) _constant_args_  -> self.constant_args

          4) expdir_root      -> Experiments are cached.

        Keys and values are joined using an equal sign ('=').
        """
        self.program       = argument_bindings.pop( "_program_",       self.program )
        self.script        = argument_bindings.pop( "_script_",        self.script )
        self.constant_args = argument_bindings.pop( "_constant_args_", self.constant_args )
        self.__check_constant_args()
        
        expdir_root = os.path.abspath( argument_bindings.get( "expdir_root", self.expdir_root ) )
        if expdir_root != self.expdir_root:
            self.expdir_root = expdir_root
            if os.path.isdir(self.expdir_root):
                Experiment.cache_experiments( self.expdir_root )
        
        if self.protocol=="expkey":            
            expkey = [ "%s=%s"%(k,v) for (k,v) in argument_bindings.iteritems() ]
            if not self._predicate( expkey ):
                raise RejectedByPredicate( ' '.join(_quoted(expkey)) )
            return expkey
        elif self.protocol=="named_args":
            return [ arg%argument_bindings for arg in self.constant_args ]

    def start( self, *oracles ):
        """Frees all tasks if a keyboard interrupt is caught."""
        if self.logdir is not None:
            if not os.path.exists( self.logdir ):
                os.mkdir( self.logdir )

            # Module function set_logdir()
            set_logdir( self.logdir )
            
        try:
            task_sum = 0
            delayed_tasks = 0
            for arguments_oracle in oracles:
                assert isinstance( arguments_oracle, ArgumentsOracle ), TypeError(type(argumentsoracle))
                done, delayed = self.__start( arguments_oracle )
                task_sum += done
                delayed_tasks += delayed

            if task_sum:
                print >>sys.stderr, "\n(%d tasks done)"%task_sum
            if delayed_tasks:
                print >>sys.stderr, "\n(%d tasks delayed)"%delayed_tasks
            print "On", sum([ len(oracle) for oracle in oracles ]), "requested experiments."

        except KeyboardInterrupt:
            sys.stderr.write("\nInterrupted by user.\n")
            self.free( )

    def __start( self, arguments_oracle ):
        """Parallel dispatch; respecting max_nmachines."""
        counter = 0
        delayed = 0
        for argument_bindings in arguments_oracle:            
            try:
                # Parses for special keywords and join keys to values.
                arguments = self.getArguments( argument_bindings )
            except RejectedByPredicate, rejected:
                print 'Already exists:', str(rejected)
                continue

            if self.program is None:
                print 'Delayed:'," ".join(_quoted(arguments))
                delayed += 1
                continue
            
            prepend = [ "echo", "$HOST;", 'nice', self.program ]
            if self.script:
                prepend.append( self.script )
            if self.protocol=="expkey":
                prepend.extend( self.constant_args )

            if self.script.find( '.pyplearn' ) != -1:
                expdir = generateExpdir()
                time.sleep( 1 ) ## Making sure the next expdir will be generated at
                                ## another 'time', i.e. on another second

                arguments.append( "expdir=%s" % expdir )

            # Module function defined above
            ##raw_input( prepend+_quoted(arguments) )
            launch_task( prepend+_quoted(arguments) )

            if Task.count( ) == self.max_nmachines:
                print >>sys.stderr, "Using %d machines or more; waiting..." % self.max_nmachines
                completed = Task.select()
            counter += 1

        ## Wait for all experiments completion
        while Task is not None: # Task is none if no tasks were launched...
            try:
                completed = Task.select()
            except EmptyTaskListError:
                break
        return counter, delayed

    def free( self ):
        try:
            Task.kill_all_tasks()
            sys.stderr.write("\nDone.\n")
        except KeyboardInterrupt:
            sys.stderr.write( "\nCan not interrupt Dispatch.free() please wait...\n" )
            self.free( )    

if __name__ == "__main__":
    dispatch = Dispatch( program = "time", logdir=None )
    dispatch.start({ "_constant_args_" : "ls -lah" }) 
