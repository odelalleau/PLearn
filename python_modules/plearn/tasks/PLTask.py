__cvs_id__ = "$Id: PLTask.py,v 1.3 2004/12/21 15:31:50 dorionc Exp $"

import shutil
from Task import *

import plearn.utilities.versionning as     versionning
versionning.declare_module( __name__,
    "$Id: PLTask.py,v 1.3 2004/12/21 15:31:50 dorionc Exp $"
    )


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

        vprint("Results appended.", 1)

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
            vprint("%s was already done."%self.pretty_cmd_line, 1)
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
        vprint("Bugged appended.", 1)

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
        vprint("sorted_vmat:\n%s\n\n\n"%str(sorted_vmat), 3)
        
        index = 0
        key = self.defines_dico.values()
        for line in sorted_vmat:
            if index == top:
                break
                
            line = string.split(line)
                
            if self.key_compare(key, line):
            ##if int(line[0]) == int(self.getName()):
                vprint("Reached position %d"%index, 1)
                break
            index += 1
        #END for

        # The experiment did not make it to the top
        if index == top:
            vprint("SELF: RMDIR(%s)"%self._results_directory, 1)
            shutil.rmtree(self._results_directory)
        elif index < top and len(sorted_vmat) > top: # The experiment is in the top
            define_keys = self.defines_dico.keys()

            ### The rejected row is the row numbered by top
            line = string.split( sorted_vmat[top] )
            rejected_experiment = task_name_from_int( int(line[0]) )
            vprint("Experiment %s throws out %s" % (self.getName(),rejected_experiment), 1)

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
