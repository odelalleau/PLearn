import sys
import os
import getopt
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

    def __init__(self):
        self.has_short_duration = 0
        self_use_localhost_if_full = 1
        self.time_format = "%Y-%m-%d/%H:%M:%S"
        self.tasks = []

        self.log_dir = 'LOGS/'
        
        self.file_redirect_stdout = 0
        self.file_redirect_stderr = 0
        
        
    def n_avail_machines(self): raise NotImplementedError, "DBIBase.n_avail_machines()"

    def clean(self):
        pass

    def run(self):
        pass
    
class Task:

    def __init__(self, command, log_dir, time_format):
        self.unique_id = get_new_sid('')         
        formated_command = string_replace(command, ' ./;','_')
        self.log_file = truncate(log_dir + self.unique_id +'_'+ formated_command, 200) + ".log"        
        self.command = command
        
        self.before_commands = []
        self.user_defined_before_commands = []
        self.user_defined_after_commands = []
        self.after_commands = []
        
        self.before_commands.append("python utils.py " + 'set_config_value '+ 
                string.join([self.log_file,'STATUS',str(STATUS_RUNNING)],' '))
        # set the current date in the field LAUNCH_TIME
        self.before_commands.append("python utils.py " + 'set_current_date '+ 
                string.join([self.log_file,'LAUNCH_TIME',time_format],' '))        
        
        self.after_commands.append("python utils.py " + 'set_config_value '+ 
                string.join([self.log_file,'STATUS',str(STATUS_FINISHED)],' '))
        # set the current date in the field FINISHED_TIME
        self.after_commands.append("python utils.py " + 'set_current_date ' + 
                string.join([self.log_file,'FINISHED_TIME',time_format],' '))
        
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
        DBIBase.__init__(self)

        #
        if not isinstance(commands,list):
            commands = [commands]
       
        for key in args.keys():
            self.__dict__[key] = args[key]
       
        # check if log directory exists, if not create it
        if not os.path.exists(self.log_dir):
            os.mkdir(self.log_dir)
       
        # create the information about the tasks
        for command in commands:
            self.tasks.append(Task(command, self.log_dir, self.time_format))
        

    def run_one_job(self, task):

        DBIBase.run(self)
        
        all_commands = task.before_commands
        all_commands.extend(task.user_defined_before_commands)
        all_commands.append(task.command)
        all_commands.extend(task.user_defined_after_commands)
        all_commands.extend(task.after_commands)
        
        command = "cluster --execute '" + string.join(all_commands,';') + "'"
        print command
        
        task.launch_time = time.time()
        set_config_value(task.log_file, 'SCHEDULED_TIME',
                time.strftime(self.time_format, time.localtime(time.time())))
        output = PIPE
        error = PIPE
        if int(self.file_redirect_stdout):
            output = file(self.log_file + '.out','w')
        if int(self.file_redirect_stderr):
            error = file(self.log_file + '.err','w')
        task.p = Popen(command, shell=True,stdout=output,stderr=error)
        
    def run(self):

        for task in self.tasks:
            self.run_one_job(task)
    
    def clean(self):
        #TODO: delete all log files for the current batch
        pass

    
def main():
    jobs = DBICluster(['ls','sleep 2']) 
    jobs.run()
    jobs.clean()
#    config['LOG_DIRECTORY'] = 'LOGS/'
if __name__ == "__main__":
    main()
