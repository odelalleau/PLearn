import fcntl, inspect, logging, operator, os, select, signal, sys, time

from popen2 import Popen4
from datetime import datetime, timedelta

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
    "get_ssh_machines", "launch_task", "launch_server", "set_logdir",

    # Classes
    "ArgumentsOracle", "Dispatch"
    ]

# In Python2.4: logging.basicConfig(level=logging.DEBUG, ...)
__hdlr = logging.StreamHandler()
__hdlr.setFormatter( logging.Formatter("%(message)s") )
logging.root.addHandler(__hdlr)
logging.root.setLevel(logging._levelNames["INFO"])
#logging.root.setLevel(logging._levelNames["DEBUG"])

#######  Module Variables  ####################################################

#
# Definitions for the different known clusters
#  These could be moved in a config file...
#  ( cluster.config in .plearn )
#
TASK_TYPE_MAP    = { 'apstat.com':            'SshTask',
                     'iro.umontreal.ca':      'ClusterTask',
                     '## UNKNOWN DOMAIN ##': 'OnHostTask'
                     }

# Figure out if we are running on a 32bit or 64 bit machine.
if sys.maxint > 2147483647:
    apstat_machines = [ 'nai', 'oni1' ]
else:
    apstat_machines = [ 'lodur',
                        'garm',
                        'mimir',
                        'embla',
                        'valhalla',
                        'inari', 
                        'kamado',
                        'loki',
                        'odin',
                        'midgard',
                        'vili' ]

# Used only for clusters of type 'ssh'. Do not enter the same machine more
# than once: use the MAX_LOADAVG map to allow for higher maximum load
# average than the default of 1.
SSH_MACHINES_MAP = { 'apstat.com': apstat_machines,

                     'iro.umontreal.ca' : [ 'lhmm',    'lknn',    'lmfa',      'lmlp',
                                            'lsom',    'lsvm',    'currie',    'dirac',
                                            'fermi',   'plank',   'einstein'
                                            ],

                     '## UNKNOWN DOMAIN ##': [ 'host' ]
                     }

# To override the default of 1
MAX_LOADAVG = { 'inari'     :  4,
                'kamado'    :  4,
                'lodur'     :  4,
                'mimir'     :  4,
                'garm'      :  4,
                'nai'       :  8,
                'oni1'      : 24
              }

# Do not perform a new query for the loadavg until recently launched
# processes are likely to have started. 
LOADAVG_DELAY = timedelta(seconds=120)
BUFSIZE       = 4096
SLEEP_TIME    = 15
LOGDIR        = None  # May be set by set_logdir()
try:
    DOMAIN_NAME = get_domain_name()
    if DOMAIN_NAME not in SSH_MACHINES_MAP.keys():
        DOMAIN_NAME = "## UNKNOWN DOMAIN ##"
except Exception, e:
    DOMAIN_NAME = "## UNKNOWN DOMAIN ##"

# Configurables
NICE          = 'nice'
ARG_VALUE_FMT = "%s=%s"
QUOTED_ARGS   = lambda L: ['"%s"'%elem for elem in L]


#######  To be assigned to a subclass of TaskType when these will be declared
Task = None

#######  Module Functions  ####################################################

#DBG: import traceback
#DBG: _waitpid = os.waitpid
#DBG: def my_waitpid(*args):
#DBG:     print 'MY WAIT:', args
#DBG:     traceback.print_stack()
#DBG:     return _waitpid(*args)
#DBG: os.waitpid = my_waitpid

def get_ssh_machines():
    from random import shuffle
    machines = list( SSH_MACHINES_MAP[DOMAIN_NAME] ) # copy
    shuffle( machines )
    return machines

def launch_task(argv, wait=False):
    assert Task is not None
    task = Task( argv)
    task.launch( wait )

def launch_server(argv):
    assert Task is not None
    task = Task( argv)
    task.launchServer()

def set_logdir(logdir):
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
    _launched_servers_info= []

    def count( cls ):
        """Return the number of uncompleted tasks."""
        return len( cls._child_processes )
    count = classmethod( count )

    def kill_all_tasks( cls ):
        cls.kill_launched_servers()
        task_list = cls._child_processes.values()
        for task in task_list:
            try:
                os.kill(task.process.pid, signal.SIGTERM)
                exit_status = task.process.wait()
                logging.info(
                    "Killed process with id %d (%d)"%(task.process.pid, exit_status) )
            except OSError:
                assert task.process.poll() != -1, \
                       "Failed to kill task with pid %d"%task.process.pid
            task.free()
        assert not cls._child_processes
    kill_all_tasks = classmethod(kill_all_tasks)

    def kill_launched_servers( cls ):
        for (host, port, pid) in cls._launched_servers_info:
            cmd= "ssh %s kill %d"%(host, pid)
            os.system(cmd)
            logging.info("Killed process %d on %s [%s]"%(pid, host, cmd))
    kill_launched_servers= classmethod(kill_launched_servers)

    def availableMachinesCount( cls ):
        """Returns the number of machines currently available for cluster job dispatch."""
        avail = 0
        for am in cls.listAvailableMachines():
            avail += int(MAX_LOADAVG.get(am, 1.) - cls.getMachineLoad(am))
        return avail
    availableMachinesCount = classmethod(availableMachinesCount)

    def getLaunchedServersInfo( cls ):
        """Returns a list of triples (machine, port, pid) 
        e.g.: [('midgard', 41492, 2677),('odin', 48972, 1830)]
        """
        return cls._launched_servers_info
    getLaunchedServersInfo= classmethod(getLaunchedServersInfo)

    def getLaunchedServerInfo( cls, child_fd ):
        """Reads machine name, port and pid for a launched PLearn server.
        returns a triple (machine, port, pid)
        e.g.: ('midgard', 41492, 2677)
        """
        got_info= False
        while not got_info:
            try:
                s= child_fd.readline()
                if s=='':
                    print "Cannot get info from server!"
                    sys.exit()
                ss= s.split(' ')
                if ss[0] == "PLEARN_SERVER_TCP":
                    info= (ss[1], int(ss[2]), int(ss[3]))
                    got_info= True
            except IOError:
                print '.',
                time.sleep(1)
                
        cls._launched_servers_info+= [info]
        return info
    getLaunchedServerInfo= classmethod(getLaunchedServerInfo)

    def select( cls ):
        """Finds, frees and returns completed tasks."""
        if cls.count() == 0:
            logging.debug("* Raising EmptyTaskListError")
            raise EmptyTaskListError()

        #####  Waiting for a child to end
        completed = []

        # Add the child end pipe to the list of selectable fd
        select_from = cls._child_processes.keys()
        pid_to_child = dict([ (task.process.pid,task)
                              for task in cls._child_processes.values() ])

        while len(completed)==0:
            # Note that a timeout must be provided to select in case tasks
            # don't write anything...
            logging.debug("* select.select(%s, ...)"%pid_to_child.keys())
            iwtd, owtd, ewtd = select.select(select_from, [], [], 1)
            assert not owtd and not ewtd

            # Pipes ready to be read must be to avoid tasks being frozen
            # due to filled buffers...
            logging.debug("* select.select() returned list of len=%d"%len(iwtd))
            for fromchild in iwtd:
                ready = cls._child_processes[fromchild]
                read_str = ready.process.fromchild.read()
                logging.debug("* Read \n%s"% read_str)
                #print("* Read \n%s"% read_str)
                if hasattr(ready, 'logfile'):
                    ready.logfile.write(read_str)

            # Since opening a Popen4 task will call waitpid on already open
            # tasks (_cleanup() in popen2.py), one MUST NOT call os.waitpid
            # on its own when he uses Popen4 tasks. Otherwise, processes
            # whose ending will have been caught by the cleanup will never
            # appear here to have ended and this process will enter an ever
            # lasting loop... Calling poll on each instance will return the
            # exit status caught be the cleanup function if any or try a
            # waitpid call with os.WNOHANG.
            #
            # If poll()'s return value is nonnegative, the task is finished
            # and the return value is the exit code it returned. Otherwise,
            # the task is still running.
            running_tasks = cls._child_processes.values()
            for task in running_tasks:
                poll_value = task.process.poll()
                logging.debug("* Poll returned %s" % poll_value)
                if poll_value >= 0:
                    task.free()
                    completed.append(task)

        return completed
    select = classmethod(select)

    #
    # Instance methods
    #
    
    def __init__(self, argv):
        self.argv = argv
        
    def launchServer(self):
        """Launch a PLearn server"""
        self.launch(wait= False)
        Task.getLaunchedServerInfo(self.process.fromchild)

    def launch(self, wait=False):
        """Launch process on an available machine"""
        try:
            command = self.getLaunchCommand( )

            # Always using no wait to get the process id...
            self.process = Popen4(command)
            flags = fcntl.fcntl(self.process.fromchild, fcntl.F_GETFL)
            fcntl.fcntl(self.process.fromchild, fcntl.F_SETFL, flags | os.O_NONBLOCK)

            task_signature = "[%s]"%command
            if LOGDIR and os.path.isdir(LOGDIR):
                filepath = os.path.join(LOGDIR, self.getLogFileBaseName())
                self.logfile = open(filepath, 'w')
                self.logfile.write( task_signature )

            logging.info(task_signature)            
            self._child_processes[ self.process.fromchild ] = self
            if wait:
                logging.debug(
                    "* TaskType.launch() waits for process (id=%d)"%self.process.pid )
                self.process.wait( )
                self.free()
            else:
                #self._child_processes[ self.process.fromchild ] = self
                logging.debug( "* children %d (%d)"
                               %(self.process.pid,len(self._child_processes)) )
                
        except EmptyMachineListError:
            logging.info("Waiting for a machine to be freed...")
            try:
                TaskType.select( )
            except EmptyTaskListError:
                logging.debug("* time.sleep(%d)"%SLEEP_TIME)
                time.sleep(SLEEP_TIME) 
            self.launch(wait)
            
    def free(self):
        if hasattr(self, 'process'):
            logging.debug("* Freeing task with pid=%d"%self.process.pid)

            Self = self.__class__._child_processes.pop(self.process.fromchild)
            assert Self == self
        
        if hasattr(self, 'logfile'):
            self.logfile.write( self.process.fromchild.read() )
            self.logfile.close()
            

class SshTask( TaskType ):

    Xopt = '-x'
    _machines = get_ssh_machines()
    _loadavg  = {}
    _available_machines = None
    _max_load= 1.0
    
    def getLoadAvg(cls, machine,
                   command = lambda host: 'ssh -x %s cat /proc/loadavg' % host):
        #print "\nQuery to", machine
        if machine in cls._loadavg:
            # For typical PLearn/FinLearn tasks, the process begins by
            # loading the script, creating the expdir, etc., which delays
            # the impact of the process on the load average...
            t, loadavg = cls._loadavg[machine]
            cur_t  = datetime(*time.localtime()[:6])
            #print "Saved %f at %s (now %s)"%(loadavg, t, cur_t)
            if cur_t < t+LOADAVG_DELAY:
                return loadavg
        return cls.getMachineLoad(machine, command)
    getLoadAvg = classmethod(getLoadAvg)

    def getMachineLoad(cls, machine, command = lambda host: 'ssh -x %s cat /proc/loadavg' % host):
        # Query for the load average
        #print "NEW QUERY!"
        p = os.popen( command(machine) )
        line = p.readline()
        return float(line.split()[0]) # Take the last minute average
    getMachineLoad = classmethod(getMachineLoad)
    

    def listAvailableMachines(cls):
        for m in cls._machines:
            try:
                loadavg = cls.getLoadAvg(m)
            except Exception:
                continue # Machine is rebooting, shut down, ...

            max_loadavg = MAX_LOADAVG.get(m, 1.)*cls._max_load
            #print "Load %f / %f"%(loadavg, max_loadavg)
            if loadavg < max_loadavg:
                # Register the load average *plus* one, taking in account
                # the process we are about to launch
                cls._loadavg[m] = datetime(*time.localtime()[:6]), loadavg+1
                #print "At %s Saving %f"%cls._loadavg[m]
                #print
                yield m
    listAvailableMachines = classmethod(listAvailableMachines)

    def nextAvailableMachine(cls):
        # If a StopIteration exception is encountered on an iteration
        # already begun, we simply have queried each machine once and shall
        # start over. If such an exception is raised on a fresh iterator,
        # then no machines are currently available and we have to wait a
        # little before querying again...
        next = None
        fresh_iterator = False
        while next is None:
            try:
                next = cls._available_machines.next()
            except (StopIteration, AttributeError), err:
                if fresh_iterator:
                    time.sleep(SLEEP_TIME)
                cls._available_machines = cls.listAvailableMachines()
                fresh_iterator = True
        return next
    nextAvailableMachine = classmethod(nextAvailableMachine)

    #
    # Instance methods
    #

    def getLaunchCommand(self):
        # Get the first available machine
        self.host = self.nextAvailableMachine()
        actual_command = ' '.join(['cd', os.getcwd(), ';', NICE] + self.argv)
        actual_command = actual_command.replace('"', r'\"')
        return 'ssh %s %s "%s"'%(self.host, self.Xopt, actual_command)

    def getLogFileBaseName(self):
        return "ssh-%s-pid=%d"%(self.host, self.process.pid)

    def free(self):
        #KNOWN ISSUE: self._machines.append( self.host )
        TaskType.free(self)


class OnHostTask( SshTask ):

    def getLoadAvg(cls, machine,
                   command = lambda host: 'cat /proc/loadavg'):
        return SshTask.getLoadAvg(machine, command)
    getLoadAvg = classmethod(getLoadAvg)
    
    def listAvailableMachines(cls):
        for m in cls._machines:
            loadavg = cls.getLoadAvg(m)
            max_loadavg = cls._max_load

            if loadavg < max_loadavg:
                cls._loadavg[m] = datetime(*time.localtime()[:6]), loadavg+1
                yield m
    listAvailableMachines = classmethod(listAvailableMachines)

    #
    # Instance methods
    #

    def getLaunchCommand(self):
        # Get the first available machine
        self.host = self.nextAvailableMachine()
        #assert self.host == 'host', self.host
        return ' '.join(self.argv)

    def getLogFileBaseName(self):
        raise NotImplementedError('OnHostTask::getLogFileBaseName')


class ClusterTask( TaskType ):
    def listAvailableMachines( cls ):
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
    listAvailableMachines = classmethod(listAvailableMachines)

    #
    # Instance methods
    #

    def getLaunchCommand( self ):
        return ' '.join(['cluster', '--execute', self.argv[0], '--force',
                         '--wait', '--duree=120h', '--'] + self.argv[1:])

    def getLogFileBaseName(self):
        return "%s-pid=%d"%(self.argv[0], self.process.pid)
    

#######  Assigning Task  ######################################################
#######  Now that TaskType subclasses are all declared, Task can be assigned    
Task = globals()[ TASK_TYPE_MAP[ DOMAIN_NAME ] ]


class RejectedByPredicate( Exception ): pass

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

    max_nmachines            = PLOption(6,
                                doc="Max number of machines. Use -1 for no limit.")
    logdir                   = PLOption(None) #"LOGS"

    # Path to the directory where experiments are to be loaded
    expdir_root              = PLOption(None) # Default: os.getcwd()

    delay                    = PLOption(False)

    ## Time interval (seconds) to wait before launching a new task
    launch_delay_seconds     = PLOption(1)

    allow_unexpected_options = lambda self : False
    def __init__( self, oracles=None,
                  _predicate=inexistence_predicate, **overrides ):
        PyPLearnObject.__init__( self, **overrides )
        self._predicate = _predicate

        # Append current path to PYTHONPATH
        if self.expdir_root is None:
            self.expdir_root = os.getcwd()
        self.__check_constant_args()

        # If oracles were already provided, the ctor can lauch the tasks...
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

        Keys and values are joined using the ARG_VALUE_FMT string.
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
            expkey = [ ARG_VALUE_FMT%(k,v) for (k,v) in argument_bindings.iteritems() ]
            if not self._predicate( expkey ):
                raise RejectedByPredicate( ' '.join(QUOTED_ARGS(expkey)) )
            return expkey
        elif self.protocol=="named_args":
            return [ arg%argument_bindings for arg in self.constant_args ]

    def start( self, *oracles ):
        """Frees all tasks if a keyboard interrupt is caught."""
        if self.logdir is not None:
            if not os.path.exists(self.logdir):
                os.mkdir(self.logdir)

            # Module function set_logdir()
            set_logdir(self.logdir)
            
        try:
            task_sum = 0
            delayed_tasks = 0
            for arguments_oracle in oracles:
                assert isinstance( arguments_oracle, ArgumentsOracle ), TypeError(type(arguments_oracle))
                done, delayed = self.__start( arguments_oracle )
                task_sum += done
                delayed_tasks += delayed

            if task_sum:
                logging.info("\n(%d tasks done)"%task_sum)
            if delayed_tasks:
                logging.info("\n(%d tasks delayed)"%delayed_tasks)
            logging.info( "[On %d requested experiments.]"
                          % sum([ len(oracle) for oracle in oracles ]) )

        except KeyboardInterrupt:
            logging.error("! Interrupted by user.")
            self.free()

    def __start(self, arguments_oracle):
        """Parallel dispatch; respecting max_nmachines."""
        counter = 0
        delayed = 0
        for argument_bindings in arguments_oracle:            
            try:
                # Parses for special keywords and join keys to values.
                arguments = self.getArguments( argument_bindings )
            except RejectedByPredicate, rejected:
                logging.info('Already exists: %s'%str(rejected))
                continue

            assert self.program
            prepend = [ "echo", "$HOST;", NICE, self.program ]
            if self.script:
                prepend.append( self.script )
            if self.protocol=="expkey":
                prepend.extend( self.constant_args )

            # No expdir is created if there is no script or if the script
            # is not a pyplearn script
            if self.script.find( '.pyplearn' ) != -1:
                expdir = None
                for arg in arguments:
                    if arg.startswith("expdir="):
                        expdir = arg
                        break

                if expdir is None:
                    expdir = generateExpdir()
                    ## Making sure the next expdir will be generated at
                    ## another 'time', i.e. on another second
                    time.sleep(self.launch_delay_seconds) 
                    arguments.append( "expdir=%s" % expdir )

            # # Module function defined above
            # launch_task( prepend+QUOTED_ARGS(arguments) )
            assert Task is not None
            ##TBM?: task = Task(prepend+QUOTED_ARGS(arguments)+[";", "echo", "'Task Done.'"])
            if self.delay:
                ##TBM?: 
                cmd = ' '.join(prepend+QUOTED_ARGS(arguments))
                logging.info('Delayed: %s'%cmd)
                ##TBM?: logging.info('Delayed: %s'%task.getLaunchCommand())
                delayed += 1
                ##TBM?: task.free() # Since it won't be launch, free the resources...
                continue
            ##TBM?: 
            task = Task(prepend+QUOTED_ARGS(arguments)+[";", "echo", "'Task Done.'"])
            
            task.launch()
            if Task.count( ) == self.max_nmachines:
                logging.info( "+++ Using %d machines or more; waiting..."
                              % self.max_nmachines )
                completed = Task.select()
            counter += 1

        ## Wait for all experiments completion
        while Task is not None:
            try:
                completed = Task.select()
            except EmptyTaskListError:
                break
        return counter, delayed

    def free(self):
        try:
            Task.kill_all_tasks()
            logging.error("! Done.")
        except KeyboardInterrupt:
            logging.error("! Dispatch.free() interupted.")
            # logging.error("! Can not interrupt Dispatch.free() please wait...")
            # self.free( )    

if __name__ == "__main__":
    dispatch = Dispatch( program = "time", logdir=None )
    dispatch.start({ "_constant_args_" : "ls -lah" }) 


# vim: filetype=python:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
