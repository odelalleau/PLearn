#!/usr/bin/env python2.3

import os, shutil, traceback
from toolkit import *
from threading import *

_disp_verbosity = Verbosity(1)
def set_dispatch_verbosity_object(vobj):
    global _disp_verbosity
    _disp_verbosity = vobj

def task_name_from_int(number):
    name = ''
    if number < 10:
        name = '000'
    elif number < 100:
        name = '00'
    elif number < 1000:
        name = '0'
    name += str( int(number) )
    return name

class Task(Thread, WithOptions):

    ## The are options (and there default values) that can
    ##  be modified through the set_option() mechanism. These
    ##  options become (somehow 'protected') class attributes.
    OPTIONS =  { 'log_file':None,
                 'override_dispatch_cmd':None,
                 'parent_name':None,
                 'results_directory':None,
                 'run_in_results_directory':False,
                 'signal_completion':True,
                 'quote_cmd_line':False
                 }
    
    def __init__(self, dispatch, task_id, cmd_line, options=OPTIONS):
        """The constructor.

        The task_id will be helpfull to track tasks. Note that it can be set by
        set_id() method if it is not here, but once it is set, it can't be changed.

        The cmd_line is the exact command line to invoke to get the task done.
        Note that it can be set by set_cmd_line() method if it is not here, but once it
        is set, it can't be changed.

        If provided, the results directory will be created at the beggining of the run.
        No other considerations are currently given to it.
        """
        Thread.__init__(self)
        WithOptions.__init__(self, options)

        self.dispatch = dispatch
        
        self._id = task_id
        self._cmd_line = cmd_line

        ## The name is a 4 digit string representation of the id
        self.setName(task_name_from_int(task_id))

        # If the task fails, this flag must be triggered
        self.failed          = False

    ###############################################
    ## Private methods
    ###############################################

    ###############################################
    ## Protected Methods
    ###############################################

    def _do_not_run(self):
        return False

    def _do_run(self):
        if self._do_not_run():
            self.dispatch.acquire_shared_files_access(self)
            return
        
        ## The directory is not created before to avoid
        ## massive creation of directories at the start
        ## the dispatching.
        if ( self._results_directory is not None
             and not os.path.exists(self._results_directory) ):
            os.makedirs(self._results_directory) 

        ## Making and changing directory to the experiment directory            
        _disp_verbosity('[ LAUNCHED %s %s ]' % (self.classname(),self.getName()), 1)

        real_cmd = self._process_command_line()

        self._launch(real_cmd)
            
        ## For a proper management of last_experiment_finished        
        self._local_postprocessing()
        self.dispatch.acquire_shared_files_access(self)
        self._postprocessing()

        _disp_verbosity('[ FINISHED %s %s ]' % (self.classname(),self.getName()), 1)

    def _launch(self, real_cmd):
##         if self._log_file is None:
##             self._log_file = 'log_file_%s.txt' % self.getName()        
##         final_cmd = string.join([ cluster_command,
##                                   real_cmd,
##                                   '>&', self._log_file ])

        assert(self.dispatch, "The dispatch field of this Task object was not properly set.")

        if self._override_dispatch_cmd is not None:
            final_cmd = string.join([ self._override_dispatch_cmd, real_cmd ])
        else:
            final_cmd = string.join([ self.dispatch.dispatch_command(), real_cmd ])

        _disp_verbosity('%s\n'%final_cmd, 1)
        os.system( final_cmd )

    def _local_postprocessing(self):
        pass
    
    def _failed(self):
        pass
    
    def _postprocessing(self):
        pass
    
    def _process_command_line(self):
        cmd = ''
        if self._run_in_results_directory:
            if self._results_directory is None:
                raise Exception("You must provide a results directory to the %s\n"
                                "if you want to set the 'run_in_results_directory' option\n"
                                "true." % self.classname())
            cmd += 'cd %s; ' % self._results_directory

        cmd += self._cmd_line #+ ' >& ' + self._log_file
        if self._log_file is not None:
            cmd += ' >& ' + self._log_file
            #self._log_file = 'log_file_%s.txt' % self.getName()        

        
        if self._quote_cmd_line:
            cmd = "'%s'" % cmd
            
        return cmd

    def _succeeded(self):
        pass
    
    ###############################################
    ## Public Methods
    ###############################################

    def run(self):
        exception = None
        try:
            self._do_run()
            if self.failed:
                self._failed()
            else:
                self._succeeded()

        except Exception, ex:
            if True:
                #if False:
                exception = ex
            else:
                msg = 'Exception in Task %s:\n %s' % (self.getName(), str(ex))
                header = string.replace(string.center('', len(msg)+4), ' ', '*')
                _disp_verbosity('%s\n* %s *\n%s' % (header,msg,header), 0)
                try:
                    self._failed()
                except Exception, ex2:
                    ## Using print in case the exception pops from _disp_verbosity
                    print 'In %s._failed:'%self.classname(), ex2                

        if self.get_option('signal_completion'):
            self.dispatch.set_last_finished_task( self.getName() )
            self.dispatch.one_task_is_finished.set()    
            if exception is not None:
                #raise exception
                traceback.print_exc(file=sys.stdout)

class AbsPathTask(Task):
    """Task that must be provided absolute paths.

    A program location, for example a c++ executable, may not be in the user's path.
    Nonetheless, one may want to run it in another directory (e.g. a subdirectory).
    For the command to still be valid, the program must be called with absolute paths.
    """
    def __init__(self, dispatch, task_id, abs_prog, abs_args=[], rargs=[], options=Task.OPTIONS):
        """The constructor.

        The abs_prog is the program name, that will be prepended
        its absolute path.

        The same is true for all abs_args in the abs_args array.
        For convenience, the abs_args can be a single argument.

        The 'r' in rargs stands for remaining, i.e. the arguments that
        do not need to be prepended anything. 
        """
        Task.__init__( self, dispatch, task_id,
                       self._build_cmd_line(abs_prog, abs_args, rargs), options )
##         raw_input(("task_id:", task_id))
##         raw_input(("abs_prog:", abs_prog))
##         raw_input(("abs_args:", abs_args))
##         raw_input(("rargs:", rargs))
##        self.set_id(task_id)
        
    ###############################################
    ## Private methods
    ###############################################

    def _build_cmd_line(self, abs_prog, abs_args, rargs):
        ## Only for convenience
        pretty_args = abs_args
        if isinstance(pretty_args, type('')):
            pretty_args = [pretty_args]

        pretty_rargs = rargs
        if isinstance(pretty_rargs, type('')):
            pretty_rargs = [pretty_rargs]

        self.pretty_cmd_line = string.join([abs_prog,
                                            string.join(pretty_args),
                                            string.join(pretty_rargs) ])
        ## Absolute path: if the 
        self.prog_name = os.path.abspath(abs_prog)
        self.abs_args = self.get_abs_args(abs_args)
        self.rargs = rargs

        return string.join( [self.prog_name,
                             string.join( self.abs_args ),
                             string.join( self.rargs )]   )


    ###############################################
    ## Public methods
    ###############################################

    def get_abs_args(self, abs_args):
        if not isinstance(abs_args, type([])):
            return [ os.path.abspath(abs_args) ]
        abs_array = []
        for arg in abs_arg:
            abs_array.append( os.path.abspath(arg) )
        return abs_array



###################################################
## PLTask class
###################################################
class PLTask(AbsPathTask):
    """Task subclass managing plearn script ran under some plearn main.

    ...
    """
    def __init__(self, dispatch, task_id, prog_name, plscript, defines={}, options=Task.OPTIONS):
        AbsPathTask.__init__( self, dispatch, task_id, prog_name, plscript,
                              self.__defines_array(defines), options )
        
    ###############################################
    ## Private Methods
    ###############################################

    def __results_directory(self):
        ## The first if is there to avoid an imperative acquire
        ##  while the second is there to ensure another Experiment
        ##  did not create the directory while waiting.
        self.experiments_path = os.path.join(self.root_expdir, "Experiments")
        if not os.path.exists(self.experiments_path):
            self.dispatch.acquire_shared_files_access(self)
            if not os.path.exists(self.experiments_path):
                os.mkdir(self.experiments_path)    
            self.dispatch.release_shared_files_access(self)

        return os.path.join(self.experiments_path, self.getName())    

    def __defines_array(self, defines_dico):
        self.defines_dico = defines_dico

        self.defines_array = []
        for key in defines_dico.keys():
            self.defines_array.append( string.join([key, "=", str(defines_dico[key])], "") )

        return self.defines_array

    def __write_amat(self, vmat_name, global_stats):
        ### Make the .amat
        amat = open(self.keyed_log_file, 'w')
        amat.write("# Data collected from PLearn experiments, to be visualized through " + vmat_name + "\n")
        amat.write("# Fieldnames:\n")
        amat.write("#: ExpNumber " + string.join(self.defines_dico.keys()) + " ") 
        amat.close()
        
        # Now uses the template matrix to complete the field names.
        if not os.path.exists(global_stats):
            ERROR("The 'global_stats' option must be wrong: " + global_stats + " does not exists in Experiment::writeAmat.");
            
        append_fieldnames = string.join(["plearn vmat fields",global_stats,"name_only transpose >>",self.keyed_log_file]) 
        os.system(append_fieldnames)

    def __write_expdir(self):
        global_stats = os.path.join(self._results_directory, self.global_stats)
        
        if not os.path.exists(self.keyed_log_file):
            vmat_name = self.keyed_log_file
            vmat_name = vmat_name[:-5] + '.vmat'
            self.__write_amat(vmat_name, global_stats)

            vmat_file = os.path.join(self.root_expdir,vmat_name)
            self.writeVmat(vmat_file)

        exp_values = self.getName() + ' ' + string.join(map(str, self.defines_dico.values()))
        append_values_command = string.join(["echo -n", exp_values, ">>",  self.keyed_log_file])
        os.system(append_values_command)

        ### Append a space between values and results
        os.system(string.join(["echo -n", "' '", ">>",  self.keyed_log_file]))
                  
        append_results_command = string.join(["plearn vmat cat", global_stats, ">>",  self.keyed_log_file])
        os.system( append_results_command )

        _disp_verbosity("Results appended.", 1)

        top = int(self.keep_only_n)
        if top > 0:
            self.hasItMadeTheTop(top)
    #END writeExpdir

    ###############################################
    ## Protected Methods
    ###############################################

    def _clean_internal(self):
        ## self.debug()
        internal = os.path.join(self._results_directory,"internal")
        ## self.debug(internal)

        os.system("mv " + internal + "/* " + self._results_directory)
        os.system("rmdir " + internal)
        assert not os.path.exists(internal), "The internal expdir was not removed properly"
        #END if
    #END clean_internal

    def _do_not_run(self):
        if self.already_done():
            _disp_verbosity("%s was already done."%self.pretty_cmd_line, 1)
            return True
        return False

    def _failed(self):
        bugs = os.path.join(self.root_expdir, self.bugs_list)
        if not os.path.exists(bugs):
            cores = open(bugs, 'w')
            cores.write("# Cores \n")
            cores.write("# Fieldnames:\n")
            cores.write("#: ExpNumber " + string.join(self.defines_dico.keys()) + "\n") 
            cores.close()

        exp_values = self.getName() + ' ' + string.join(map(str, self.defines_dico.values()))
        append_values_command = string.join(["echo", exp_values, ">>", bugs])
        os.system(append_values_command)
        _disp_verbosity("Bugged appended.", 1)

    def _postprocessing(self):
        self.check_for_core_dump()
        self.check_for_error()

        ## Adding the informations to the global files
        if self.was_core_dumped or self.issued_an_error:
            self._failed = True
        else:
            self._clean_internal()
            self.__write_expdir()

    def _process_command_line(self):
        self._cmd_line = string.join([self._cmd_line, 'EXPDIR=internal'])
        return AbsPathTask._process_command_line(self)
        
    ###############################################
    ## Public Methods
    ###############################################

    def already_done(self):
        self.dispatch.acquire_shared_files_access(self)

        exists = False
        if os.path.exists(self.keyed_log_file):            
            key = self.defines_dico.values()
            done = command_output("plearn vmat cat " + self.keyed_log_file)
            for line in done:
                line = string.split(line)
                if self.key_compare(key, line):
                    exists = True
                    break

        self.dispatch.release_shared_files_access(self)
        return exists
        
    def check_for_core_dump(self):
        dir_list = os.listdir(self._results_directory)
        for f in dir_list:
            if string.find(f, 'core') != -1:
                self.was_core_dumped = True
                break

    def check_for_error(self):
        log_file = os.path.join( self._results_directory,
                                 self._log_file )
        assert os.path.exists(log_file), "There should be a %s file." % log_file
        
        output = command_output("cat " + log_file)
        for line in output:
            if string.find(line, "ERROR") != -1:
                self.issued_an_error = True
                break

    def debug(self, msg=''):
        ## raw_input( string.join(['\n',self.getName(), os.getcwd(), msg]) )
        pass
        
    def hasItMadeTheTop(self, top):
        sorted_vmat = command_output("plearn vmat cat " + self.sort_file)
        _disp_verbosity("sorted_vmat:\n%s\n\n\n"%str(sorted_vmat), 2)
        
        index = 0
        key = self.defines_dico.values()
        for line in sorted_vmat:
            if index == top:
                break
                
            line = string.split(line)
                
            if self.key_compare(key, line):
            ##if int(line[0]) == int(self.getName()):
                _disp_verbosity("Reached position %d"%index, 1)
                break
            index += 1
        #END for

        # The experiment did not make it to the top
        if index == top:
            _disp_verbosity("SELF: RMDIR(%s)"%self._results_directory, 1)
            shutil.rmtree(self._results_directory)
        elif index < top and len(sorted_vmat) > top: # The experiment is in the top
            define_keys = self.defines_dico.keys()

            ### The rejected row is the row numbered by top
            line = string.split( sorted_vmat[top] )
            rejected_experiment = task_name_from_int( int(line[0]) )
            _disp_verbosity("Experiment %s throws out %s" % (self.getName(),rejected_experiment), 1)

            rejected_experiment = os.path.join(self.experiments_path, rejected_experiment)

            shutil.rmtree( rejected_experiment )

    def key_compare(self, key, line):
        index = 1
        for k in key:
            if k != float(line[index]):
                return False
            index += 1
        return True

    def setOptions(self, root_expdir, keyed_log_file, global_stats, keep_only_n, sort_file, bugs_list):
        self.root_expdir = root_expdir
        self.keyed_log_file = keyed_log_file
        self.global_stats = global_stats
        self.keep_only_n = keep_only_n
        self.sort_file = sort_file
        self.bugs_list = bugs_list

        self.set_option('results_directory', self.__results_directory())
        self.set_option('run_in_results_directory', True)
        self.set_option('quote_cmd_line', True)
        ## raw_input(self._results_directory)

    def setWriteVmat(self, fct):
        self.writeVmat = fct

    def build(self):
##         ## Results directory
##         self.buildExpdir()

        # May be changed after the run
        self.was_core_dumped = False
        self.issued_an_error = False


###############################################################
### Management of KeyboadInterrupt within threads #############
### requires the following                         #############
__dispatch_end = Event()
def wait_for_dispatch_to_end():
    __dispatch_end.wait()

def dispatch_was_killed():
    return __dispatch_end.isSet()

def kill_dispatch():
    __dispatch_end.set()

###############################################################
class Dispatch(Thread, WithOptions):

    OPTIONS = { 'cluster_command':'cluster --wait --duree 240h --execute',
                'localhost':False,
                'nb_hosts':10      }
    
    def __init__(self, options=OPTIONS):
        Thread.__init__(self)
        WithOptions.__init__(self, options)
        
        self.__tasks = {}
            
        self.__shared_files_access = Semaphore()
        self.one_task_is_finished = Event()
        self.__last_finished_task = ''

    def add_task(self, task):
        if not isinstance(task, Task):
            raise TypeError("The add_task() procedure expects a Task (%s provided)."
                            % type(task))
        self.__tasks[task.getName()] = task
        #raw_input("\n\n %s" % str(self.__tasks))

    def acquire_shared_files_access(self, child_thread):
        if dispatch_was_killed():
            return False
        self.__shared_files_access.acquire(True)
        if dispatch_was_killed():
            return False
        return True
        
    def dispatch_command(self):
        if self._localhost:
            return ''
        return self._cluster_command

    def get_last_finished_task(self):
        return self.__last_finished_task

    def localhost_run(self):
        tasks_to_run = self.__tasks.values()
        for task in tasks_to_run:
            task.set_option('quote_cmd_line', False)

        nb_task = len(tasks_to_run)
        task_done = 0
        for task in tasks_to_run:
            task.start()
            
            # Wait for something to finish...
            self.one_task_is_finished.wait()
            self.one_task_is_finished.clear()

            task_done += 1
            _disp_verbosity("*** %d tasks done.    %d to go.\n"%(task_done,nb_task-task_done), 1)
            self.release_shared_files_access(self)

    def release_shared_files_access(self, child_thread):
        self.__shared_files_access.release()
        return True

    def run(self):
        try:
            self.__run_body()
        except KeyboardInterrupt, ex:
            _disp_verbosity("Dispatch killed by KeyboardInterrupt.", 0)
        kill_dispatch()

    def __run_body(self):        
        if self._localhost:
            self.localhost_run()
            return
        
        tasks_to_run = self.__tasks.values()

        hostnum = 0
        nb_hosts = self._nb_hosts
        nb_task = len(tasks_to_run)
        _disp_verbosity("%d tasks to run on at most %d hosts." % (nb_task,nb_hosts), 1)
    
        launched_task = 0
        current_task = {}

        task_done = 0

        while launched_task < nb_task:
            task = tasks_to_run[launched_task]
                    
            if hostnum < nb_hosts:
                hostnum += 1

                task.start()
                current_task[task.getName()] = task
                launched_task += 1

                _disp_verbosity("*** %d tasks done.    %d to go.    %d running."
                            %(task_done,nb_task-task_done,len(current_task)), 1)
                _disp_verbosity("%d hosts used -- %s max.\n" % (hostnum,nb_hosts), 1)
                
            else:
                # Processes are busy, wait for something to finish...
                self.one_task_is_finished.wait()
                self.one_task_is_finished.clear()
            
                # One of the tasks just finished
                del current_task[ self.get_last_finished_task() ]            
                task_done += 1
            
                # Launch a new task on the free host
                task.start()
                current_task[task.getName()] = task
                launched_task += 1
            
                _disp_verbosity("*** %d tasks done.    %d to go.    %d running.\n"
                            %(task_done,nb_task-task_done,len(current_task)), 1)
                self.shared_files_access.release()
            ##END if/else
        ##END while

        # Now wait until everybody is finished
        while current_task:
            # Processes are busy, wait for something to finish...
            self.one_task_is_finished.wait()
            self.one_task_is_finished.clear()

            # One of the tasks just finished
            del current_task[ self.get_last_finished_task() ]            
            task_done += 1

            _disp_verbosity("*** %d tasks done.    %d to go.    %d running.\n"
                        %(task_done,nb_task-task_done,len(current_task)), 1)
            self.shared_files_access.release()
        ##END while
            
        _disp_verbosity("+++ END OF TASKS.", 0)

    def set_last_finished_task(self, task_name):
        self.__last_finished_task = task_name
