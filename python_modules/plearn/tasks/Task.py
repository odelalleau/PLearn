import os, sys, shutil, string, traceback
from toolkit                    import WithOptions
from threading                  import Thread     
from plearn.utilities.verbosity import vprint

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
        vprint('[ LAUNCHED %s %s ]' % (self.classname(),self.getName()), 1)

        real_cmd = self._process_command_line()

        self._launch(real_cmd)
            
        ## For a proper management of last_experiment_finished        
        self._local_postprocessing()
        self.dispatch.acquire_shared_files_access(self)
        self._postprocessing()

        vprint('[ FINISHED %s %s ]' % (self.classname(),self.getName()), 1)

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

        vprint('%s\n'%final_cmd, 1)
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
                vprint('%s\n* %s *\n%s' % (header,msg,header), 0)
                try:
                    self._failed()
                except Exception, ex2:
                    ## Using print in case the exception pops from vprint
                    print 'In %s._failed:'%self.classname(), ex2                

        if self.get_option('signal_completion'):
            self.dispatch.set_last_finished_task( self.getName() )
            self.dispatch.one_task_is_finished.set()    
            if exception is not None:
                #raise exception
                traceback.print_exc(file=sys.stdout)

