import sys
import os
import getopt
import re
import string
import time
from subprocess import Popen,PIPE
from utils import *
from configobj import ConfigObj

STATUS_FINISHED = 0
STATUS_RUNNING = 1
STATUS_WAITING = 2
STATUS_ERROR = 3


class DBIBase:

    def __init__(self, commands, **args ):
        self.unique_id = get_new_sid('')
        self.has_short_duration = 0
        self_use_localhost_if_full = 1
        self.time_format = "%Y-%m-%d/%H:%M:%S"

        # Commands to be executed once before the entire batch
        self.pre_batch = []
        # Commands to be executed before every task in tasks
        self.pre_tasks = []
        # The main tasks to be dispatched
        self.tasks = []
        # Commands to be executed before after task in tasks
        self.post_tasks = []
        # Commands to be executed once after the entire batch
        self.post_batch = []

        self.log_dir = 'LOGS'
        self.log_file = os.path.join( self.log_dir, self.unique_id )

        self.file_redirect_stdout = 0
        self.file_redirect_stderr = 0

        # Initialize the namespace
        for key in args.keys():
            self.__dict__[key] = args[key]

        # If some arguments aren't lists, put them in a list
        if not isinstance(commands, list):
            commands = [commands]
        if not isinstance(self.pre_batch, list):
            self.pre_batch = [self.pre_batch]
        if not isinstance(self.pre_tasks, list):
            self.pre_tasks = [self.pre_tasks]
        if not isinstance(self.post_tasks, list):
            self.post_tasks = [self.post_tasks]
        if not isinstance(self.post_batch, list):
            self.post_batch = [self.post_batch]

    def n_avail_machines(self): raise NotImplementedError, "DBIBase.n_avail_machines()"

    def clean(self):
        pass

    def run(self):
        pass

class Task:

    def __init__(self, command, log_dir, time_format, pre_tasks=[], post_tasks=[]):
        self.unique_id = get_new_sid('')
        formatted_command = re.sub( '[^a-zA-Z0-9]', '_', command );
        self.log_file = truncate( os.path.join(log_dir, self.unique_id +'_'+ formatted_command), 200) + ".log"
        # The "python utils.py..." command is not exactly the same for every
        # task in a batch, so it cannot be considered a "pre-command", and
        # has to be actually part of the command.  Since the user-provided
        # pre-command has to be executed afterwards, it also has to be part of
        # the command itself. Therefore, no need for pre- and post-commands in
        # the Task class
        #self.command = command
        #self.before_commands = []
        #self.user_defined_before_commands = []
        #self.user_defined_after_commands = []
        #self.after_commands = []

        self.commands = []

        self.commands.append("python utils.py " + 'set_config_value '+
                string.join([self.log_file,'STATUS',str(STATUS_RUNNING)],' '))
        # set the current date in the field LAUNCH_TIME
        self.commands.append("python utils.py " + 'set_current_date '+
                string.join([self.log_file,'LAUNCH_TIME',time_format],' '))

        self.commands.extend( pre_tasks )
        self.commands.append( command )
        self.commands.extend( post_tasks )

        self.commands.append("python utils.py " + 'set_config_value '+
                string.join([self.log_file,'STATUS',str(STATUS_FINISHED)],' '))
        # set the current date in the field FINISHED_TIME
        self.commands.append("python utils.py " + 'set_current_date ' +
                string.join([self.log_file,'FINISHED_TIME',time_format],' '))

        print "self.commands =", self.commands

    def get_status(self):
        #TODO: catch exception if value not available
        status = get_config_value(self.log_file,'STATUS')
        return int(status)

    def get_waiting_time(self):
        # get the string representation
        str_sched = get_config_value(self.log_file,'SCHEDULED_TIME')
        # transform in seconds from the start of epoch
        sched_time = time.mktime(time.strptime(str_sched,ClusterLauncher.time_format))

        # get the string representation
        str_launch = get_config_value(self.log_file,'LAUNCH_TIME')
        # transform in seconds from the start of epoch
        launch_time = time.mktime(time.strptime(str_launch,ClusterLauncher.time_format))

        return launch_time - sched_time

    def get_running_time(self):
        #TODO: handle if job did not finish
        # get the string representation
        str_launch = get_config_value(self.log_file,'LAUNCH_TIME')
        # transform in seconds from the start of epoch
        launch_time = time.mktime(time.strptime(str_launch,ClusterLauncher.time_format))

        # get the string representation
        str_finished = get_config_value(self.log_file,'FINISHED_TIME')
        # transform in seconds from the start of epoch
        finished_time = time.mktime(time.strptime(str_finished,ClusterLauncher.time_format))

        return finished_time - launch_time

class DBICluster(DBIBase):

    def __init__(self, commands, **args ):
        DBIBase.__init__(self, commands, **args)

        # check if log directory exists, if not create it
        if not os.path.exists(self.log_dir):
            os.mkdir(self.log_dir)

        # create the information about the tasks
        for command in commands:
            self.tasks.append(Task(command, self.log_dir, self.time_format,
                                   self.pre_tasks, self.post_tasks))


    def run_one_job(self, task):
        DBIBase.run(self)

        command = "cluster --execute '" + string.join(task.commands,';') + "'"
        print command

        task.launch_time = time.time()
        set_config_value(task.log_file, 'SCHEDULED_TIME',
                time.strftime(self.time_format, time.localtime(time.time())))
        output = PIPE
        error = PIPE
        if int(self.file_redirect_stdout):
            output = file(task.log_file + '.out','w')
        if int(self.file_redirect_stderr):
            error = file(task.log_file + '.err','w')
        task.p = Popen(command, shell=True,stdout=output,stderr=error)

    def run(self):
        # Execute pre-batch
        pre_batch_command = ';'.join( self.pre_batch )
        output = PIPE
        error = PIPE
        if int(self.file_redirect_stdout):
            output = file(self.log_file + '.pre_batch.out', 'w')
        if int(self.file_redirect_stderr):
            error = file(self.log_file + '.pre_batch.err', 'w')
        self.pre = Popen(pre_batch_command, shell=True, stdout=output, stderr=error)
        print 'pre_batch_command =', pre_batch_command

        # Execute all Tasks (including pre_tasks and post_tasks if any)
        for task in self.tasks:
            self.run_one_job(task)

        # Execute post-batchs
        post_batch_command = ";".join( self.post_batch );
        if int(self.file_redirect_stdout):
            output = file(self.log_file + '.post_batch.out', 'w')
        if int(self.file_redirect_stderr):
            error = file(self.log_file + '.post_batch.err', 'w')
        self.post = Popen(post_batch_command, shell=True, stdout=output, stderr=error)
        print 'post_batch_command =', post_batch_command

    def clean(self):
        #TODO: delete all log files for the current batch
        pass

def DBIbqtools(DBIBase):

    def __init__( self, commands, **args ):
        DBIBase.__init__(self, commands, **args)

        # create directory in which all the temp files will be created
        self.temp_dir = 'batch_' + self.unique_id + '_tmp'
        os.mkdir(self.temp_dir)
        os.chdir(self.temp_dir)

        # create the right symlink for parent in self.temp_dir_name
        self.parent_dir = 'parent'
        os.symlink( '..', parent_dir )

        # check if log directory exists, if not create it
        self.log_dir = os.path.join( parent_dir, self.log_dir )
        if not os.path.exists(self.log_dir):
            os.mkdir(self.log_dir)

        # create the information about the tasks
        for command in commands:
            # pre_tasks and post_tasks commands have to be added to command
            # before the creation of the Task object
            all_commands = pre_tasks + [command] + post_tasks
            self.tasks.append(Task(all_commands, self.log_dir, self.time_format))


    def run(self):
        pre_batch_command = ';'.join( pre_batch );
        if int(self.file_redirect_stdout):
            pre_batch_command += ' >> ' + self.log_file + '.pre_batch.out'
        if int(self.file_redirect_stderr):
            pre_batch_command += ' 2>> ' + self.log_file + '.pre_batch.err'

        post_batch_command = ';'.join( post_batch );
        if int(self.file_redirect_stdout):
            post_batch_command += ' >> ' + self.log_file + '.post_batch.out'
        if int(self.file_redirect_stderr):
            post_batch_command += ' 2>> ' + self.log_file + '.post_batch.err'

        # create one (sh) script that will launch the appropriate ~~command~~
        # in the right environment
        launcher = open( 'launcher', 'w' )
        f.write( dedent('''\
                #!/bin/sh
                HOME=$BQ_CLUSTER_HOME
                export HOME
                ($BQ_SHELL_COMMAND '~~task~~')'''
                ) )

        if int(self.file_redirect_stdout):
            f.write( ' >> ~~logfile~~.out' )
        if int(self.file_redirect_stderr):
            f.write( ' 2>> ~~logfile~~.err' )
        launcher.close()

        # create a file containing the list of commands, one per line
        # and another one containing the log_file name associated
        tasks_file = open( 'tasks', 'w' )
        logfiles_file = open( 'logfiles', 'w' )
        for task in self.tasks:
            tasks_file.write( task.command + '\n' )
            logfiles_file.write( task.log_file + '\n' )
        tasks_file.close()
        logfiles_file.close()

        # create the bqsubmit.dat, with
        bqsubmit_dat = open( 'bqsubmit.dat', 'w' )
        bqsubmit_dat.write( dedent('''\
                batchName = dbi_batch
                command = sh launcher
                templateFiles = launcher
                remoteHost = auto
                param1 = (task, logfile) = load tasks, logfiles
                concurrentJobs = 200
                preBatch = ''' + pre_batch_command + '''
                postBatch = ''' + post_batch_command +'''
                ''') )
        bqsubmit_dat.close()


    def clean(self):
        pass


def main():
    jobs = DBICluster(['ls','sleep 2'])
    jobs.run()
    jobs.clean()
#    config['LOG_DIRECTORY'] = 'LOGS/'
if __name__ == "__main__":
    main()
