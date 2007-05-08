#! /usr/bin/env python
import sys
import os
import getopt
import re
import string
import time
import traceback
import shutil
from subprocess import Popen,PIPE
from utils import *
from configobj import ConfigObj
from textwrap import dedent
import pdb
from time import sleep
#from plearn.pymake import pymake

STATUS_FINISHED = 0
STATUS_RUNNING = 1
STATUS_WAITING = 2
STATUS_ERROR = 3


class DBIBase:

    def __init__(self, commands, **args ):
        #generate a new unique id
        self.unique_id = get_new_sid('')

        # option is not used yet
        self.has_short_duration = 0

        # if all machines are full, run the jobs one by one on the localhost
        self_use_localhost_if_full = 1

        # the( human readable) time format used in log file
        self.time_format = "%Y-%m-%d/%H:%M:%S"

        # Commands to be executed once before the entire batch
        self.pre_batch = []
        # Commands to be executed before every task in tasks
        self.pre_tasks = []
        # The main tasks to be dispatched
        self.tasks = []
        # Commands to be executed after each task in tasks
        self.post_tasks = []
        # Commands to be executed once after the entire batch
        self.post_batch = []

        # the default directory where to keep all the log files
        self.log_dir = 'LOGS'
        self.log_file = os.path.join( self.log_dir, self.unique_id )

        #
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

    def __init__(self, command, log_dir, time_format, pre_tasks=[], post_tasks=[], args = {}):
        self.unique_id = get_new_sid('')
        self.add_unique_id = 0
        formatted_command = re.sub( '[^a-zA-Z0-9]', '_', command );
        self.log_file = truncate( os.path.join(log_dir, self.unique_id +'_'+ formatted_command), 200) + ".log"
        # The "python utils.py..." command is not exactly the same for every
        # task in a batch, so it cannot be considered a "pre-command", and
        # has to be actually part of the command.  Since the user-provided
        # pre-command has to be executed afterwards, it also has to be part of
        # the command itself. Therefore, no need for pre- and post-commands in
        # the Task class



        for key in args.keys():
            self.__dict__[key] = args[key]

        if self.add_unique_id:
                command = command + ' unique_id=' + self.unique_id
        #self.before_commands = []
        #self.user_defined_before_commands = []
        #self.user_defined_after_commands = []
        #self.after_commands = []

        self.commands = []
        if len(pre_tasks) > 0:
            self.commands.extend( pre_tasks )
#        self.commands.append("cd parent")
        self.commands.append("./utils.py " + 'set_config_value '+
                string.join([self.log_file,'STATUS',str(STATUS_RUNNING)],' '))
        # set the current date in the field LAUNCH_TIME
        self.commands.append("./utils.py " + 'set_current_date '+
                string.join([self.log_file,'LAUNCH_TIME',time_format],' '))

        #cd to parent diectory, run the command, and then cd back
#	command = 'cd parent;' + command + ';cd ' + self.temp_dir 
        self.commands.append( command )
        self.commands.extend( post_tasks )

        self.commands.append("./utils.py " + 'set_config_value '+
                string.join([self.log_file,'STATUS',str(STATUS_FINISHED)],' '))
        # set the current date in the field FINISHED_TIME
        self.commands.append("./utils.py " + 'set_current_date ' +
                string.join([self.log_file,'FINISHED_TIME',time_format],' '))
#     self.commands.append("cd parent")

        #print "self.commands =", self.commands

    def get_status(self):
        #TODO: catch exception if value not available
        status = get_config_value(self.log_file,'STATUS')
        return int(status)

    def get_stdout(self):
        try:
            if isinstance(self.p.stdout, file):
                return self.p.stdout
            else:
                return open(self.log_file + '.out','r')
        except:
            pass
        return None
        
    def get_stderr(self):
        try:
            if isinstance(self.p.stderr, file):
                return self.p.stderr
            else:
                return open(self.log_file + '.err','r')
        except:
            pass
        return None

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
        #print 'pre_batch_command =', pre_batch_command

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
        #print 'post_batch_command =', post_batch_command

    def clean(self):
        #TODO: delete all log files for the current batch
        pass

class DBIbqtools(DBIBase):

    def __init__( self, commands, **args ):
        DBIBase.__init__(self, commands, **args)

        # create directory in which all the temp files will be created
        self.temp_dir = 'batch_' + self.unique_id + '_tmp'
        os.mkdir(self.temp_dir)
        os.chdir(self.temp_dir)

        # create the right symlink for parent in self.temp_dir_name
        self.parent_dir = 'parent'
        os.symlink( '..', self.parent_dir )

        # check if log directory exists, if not create it
#        self.log_dir = os.path.join( self.parent_dir, self.log_dir )
        if not os.path.exists(self.log_dir):
            os.mkdir(self.log_dir)

        self.log_file = os.path.join( self.parent_dir, self.log_file )

        # create the information about the tasks
        args['temp_dir'] = self.temp_dir
        for command in commands:
            self.tasks.append(Task(command, self.log_dir, self.time_format,
                                   [self.pre_tasks,'cd parent;'], self.post_tasks,args))


    def run(self):
        pre_batch_command = ';'.join( self.pre_batch );
        if int(self.file_redirect_stdout):
            pre_batch_command += ' >> ' + self.log_file + '.pre_batch.out'
        if int(self.file_redirect_stderr):
            pre_batch_command += ' 2>> ' + self.log_file + '.pre_batch.err'

        post_batch_command = ';'.join( self.post_batch );
        if int(self.file_redirect_stdout):
            post_batch_command += ' >> ' + self.log_file + '.post_batch.out'
        if int(self.file_redirect_stderr):
            post_batch_command += ' 2>> ' + self.log_file + '.post_batch.err'

        # create one (sh) script that will launch the appropriate ~~command~~
        # in the right environment


        launcher = open( 'launcher', 'w' )
        bq_cluster_home = os.getenv( 'BQ_CLUSTER_HOME', '$HOME' )
        bq_shell_cmd = os.getenv( 'BQ_SHELL_CMD', '/bin/sh -c' )
        launcher.write( dedent('''\
                #!/bin/sh

                HOME=%s
                export HOME

                (%s '~~task~~')'''
                % (bq_cluster_home, bq_shell_cmd)
                ) )

        if int(self.file_redirect_stdout):
            launcher.write( ' >> ~~logfile~~.out' )
        if int(self.file_redirect_stderr):
            launcher.write( ' 2>> ~~logfile~~.err' )
        launcher.close()

        # create a file containing the list of commands, one per line
        # and another one containing the log_file name associated
        tasks_file = open( 'tasks', 'w' )
        logfiles_file = open( 'logfiles', 'w' )
        for task in self.tasks:
            tasks_file.write( ';'.join(task.commands) + '\n' )
            logfiles_file.write( task.log_file + '\n' )
        tasks_file.close()
        logfiles_file.close()

        # create the bqsubmit.dat, with
        bqsubmit_dat = open( 'bqsubmit.dat', 'w' )
        bqsubmit_dat.write( dedent('''\
                batchName = dbi_batch
                command = sh launcher
                templateFiles = launcher
                linkFiles = parent;parent/utils.py
                remoteHost = ss3
                param1 = (task, logfile) = load tasks, logfiles
                concurrentJobs = 200

                ''') )
#                preBatch = ''' + pre_batch_command + '''
#                postBatch = ''' + post_batch_command +'''
        bqsubmit_dat.close()

        # Launch bqsubmit
        output = PIPE
        error = PIPE
        if int(self.file_redirect_stdout):
            output = file(self.log_file + '.out', 'w')
        if int(self.file_redirect_stderr):
            error = file(self.log_file + '.err', 'w')
        self.p = Popen( 'bqsubmit', shell=True, stdout=output, stderr=error)

        os.chdir('parent')

class DBICondor(DBIBase):

    def __init__( self, commands, **args ):
        DBIBase.__init__(self, commands, **args)

        # check if log directory exists, if not create it
#        self.log_dir = os.path.join( self.parent_dir, self.log_dir )
        if not os.path.exists(self.log_dir):
            os.mkdir(self.log_dir)

#        self.log_file = os.path.join( self.parent_dir, self.log_file )

        # create the information about the tasks
#        args['temp_dir'] = self.temp_dir
        for command in commands:
            pos = string.find(command,' ')
            if pos>=0:
                c = command[0:pos]
                c2 = command[pos:]
            else:
                c=command
                c2=""

                # We will execute the command on the specified architecture
                # if it is specified. If the executable exist for both
                # architecture we execute on both. Otherwise we execute on the
                # same architecture as the architecture of the launch computer
            self.cplat = get_condor_platform()
            if c.endswith('.INTEL'):
                self.targetcondorplatform='INTEL'
                self.targetplatform='linux-i386'
                newcommand=command
            elif c.endswith('.X86_64'):
                self.targetcondorplatform='X86_64'
                self.targetplatform='linux-x86_64'
                newcommand=command
            elif os.path.exists(c+".INTEL") and os.path.exists(c+".X86_64"):
                self.targetcondorplatform='BOTH'
                self.targetplatform='BOTH'
                newcommand=command #TODO:get the good data
                print 'ERROR: executing on 32 and 64 bits is NOT currently implemented'
                sys.exit(100)
            elif self.cplat=="INTEL" and os.path.exists(c+".INTEL"):
                self.targetcondorplatform='INTEL'
                self.targetplatform='linux-i386'
                newcommand=c+".INTEL"+c2
            elif self.cplat=="X86_64" and os.path.exists(c+".X86_64"):
                self.targetcondorplatform='X86_64'
                self.targetplatform='linux-x86_64'
                newcommand=c+".X86_64"+c2
            else:
                self.targetcondorplatform=self.cplat
                if self.cplat=='INTEL':
                    self.targetplatform='linux-i386'
                else:
                    self.targetplatform='linux-x86_64'
                newcommand=command

            self.tasks.append(Task(newcommand, self.log_dir, self.time_format,
                                   self.pre_tasks, self.post_tasks,args))

            #keeps a list of the temporary files created, so that they can be deleted at will            
        self.temp_files = []

    def run_one_job(self, task):
        
        # create the bqsubmit.dat, with

        condor_data = task.unique_id + '.data'
        self.temp_files.append(condor_data)
        param_dat = open(condor_data, 'w')

        param_dat.write( dedent('''\
                #!/bin/bash
                %s''' %(';\n'.join(task.commands))))
        param_dat.close()
        

        condor_file = task.unique_id + ".condor"
        self.temp_files.append(condor_file)
        condor_dat = open( condor_file, 'w' )

        u=get_username()
        tcplat=self.targetcondorplatform
        tplat=self.targetplatform
        condor_dat.write( dedent('''\
                executable     = ./launch.sh
                arguments      = sh %s
                universe       = vanilla
                requirements   = (Arch == "%s")
                output         = main.%s.%s.out
                error          = main.%s.%s.error
                log            = main.%s.log
                environment    = LD_LIBRARY_PATH=/u/lisa/local/byhost/%s/lib:/u/lisa/local/%s/lib:/cluster/diro/home/lisa/local/%s/lib/python2.4/site-packages/numarray:/cluster/diro/home/lisa/local/%s/lib:/cluster/diro/home/lisa/local/%s/lib32:/cluster/diro/home/lisa/lib:/usr/local/lib:/soft/lisa/linux/lib;\
                PATH=/cluster/diro/home/lisa/PLearn/scripts:/soft/lisa/linux/bin:/u/lisa/local/%s/bin:/u/%s/PLearn/commands:/u/%s/PLearn/scripts:/Scripts:/commands:/u/%s/Scripts:/soft/diro/share/moe/bin-i4lx:/u/lamblinp/code/usr/bin:/u/%s/PLearn:/u/%s/PLearn/scripts:/u/%s/PLearn/commands:/usr/kerberos/bin:/usr/GNUstep/System/Tools:/usr/local/bin:/bin:/usr/bin:/usr/X11R6/bin:/opt/diro/bin:/u/lisa/local/linux-i386/bin;\
                PYTHONPATH=/cluster/diro/home/lisa/local/%s/lib/python2.4/site-packages:/cluster/diro/home/lisa/local/%s/lib/python2.3/site-packages:/cluster/diro/home/lisa/local/%s/lib/python2.2/site-packages:/cluster/diro/home/lisa/local/%s/lib/python2.4/site-packages/vtk_python:/cluster/diro/home/lisa/local/%s/lib/python2.3/site-packages/Numeric:/u/%s/PLearn/python_modules:/u/%s/projects/apstatsoft/python_modules:/cluster/diro/home/lisa/local/%s/lib/python2.3/site-packages:/cluster/diro/home/lisa/local/%s/lib/python2.2/site-packages:/cluster/diro/home/lisa/local/%s/lib/python2.4/site-packages/vtk_python:/cluster/diro/home/lisa/local/%s/lib/python2.3/site-packages/Numeric:/u/%s/PLearn/python_modules:
                #getenv         = True
                queue
                ''' % (condor_data, tcplat,tcplat,task.unique_id,tcplat,task.unique_id,tcplat,get_hostname(),tplat,tplat,tplat,tplat,tplat,u,u,u,u,u,u,tplat,tplat,tplat,tplat,tplat,u,u,tplat,tplat,tplat,tplat,u)) )
#                preBatch = ''' + pre_batch_command + '''
#                postBatch = ''' + post_batch_command +'''
        condor_dat.close()
                #environment    = LD_LIBRARY_PATH=/u/lisa/local/byhost/%s/lib:/u/lisa/local/%s/lib:/cluster/diro/home/lisa/local/%s/lib/python2.4/site-packages/numarray:/cluster/diro/home/lisa/local/%s/lib:/usr/local/lib:/cluster/diro/home/lisa/local/%s/lib32:/soft/lisa/linux/lib;PATH=/cluster/diro/home/lisa/PLearn/scripts:/soft/lisa/linux/bin:/u/lisa/local/%s/bin:/u/%s/PLearn/commands:/u/%s/PLearn/scripts:/Scripts:/commands:/u/%s/Scripts:/soft/diro/share/moe/bin-i4lx:/u/lamblinp/code/usr/bin:/u/%s/PLearn:/u/%s/PLearn/scripts:/u/%s/PLearn/commands:/u/%s/projects/apstatsoft:/u/%s/projects/apstatsoft/scripts:/u/%s/projects/apstatsoft/commands:/usr/kerberos/bin:/u/%s/GNUstep/Tools:/usr/GNUstep/Local/Tools:/usr/GNUstep/System/Tools:/usr/local/bin:/bin:/usr/bin:/usr/X11R6/bin:/opt/diro/bin:/u/lisa/local/linux-i386/bin;PYTHONPATH=/cluster/diro/home/lisa/PLearn/python_modules:/cluster/diro/home/lisa/local/%s/lib/python2.4/site-packages:/u/lisa/local/%s/lib/python2.3/site-packages:/u/lisa/local/%s/lib/python2.2/site-packages:/u/lisa/local/%s/lib/python2.4/site-packages/vtk_python:/u/lisa/local/%s/lib/python2.3/site-packages/Numeric:/u/%s/PLearn/python_modules:/u/%s/projects/apstatsoft/python_modules:

        if not os.path.exists('launch.sh'):
            launch_file = open('launch.sh','w')
            launch_file.write(dedent('''\
            #!/bin/sh
            PROGRAM=$1
            shift
            $PROGRAM $@'''))
            launch_file.close()
            os.chmod('launch.sh',0755)

        if not os.path.exists('utils.py'):
            shutil.copy( get_plearndir()+'/python_modules/plearn/parallel/utils.py', '.')
            os.chmod('utils.py',0755)

        if not os.path.exists('configobj.py'):
            shutil.copy( get_plearndir()+'/python_modules/plearn/parallel/configobj.py', '.')
            os.chmod('configobj.py',0755)
                                    
        # Launch bqsubmit
        output = PIPE
        error = PIPE
        if int(self.file_redirect_stdout):
            output = file(self.log_file + '.out', 'w')
        if int(self.file_redirect_stderr):
            error = file(self.log_file + '.err', 'w')
        self.p = Popen( 'condor_submit '+ condor_file, shell=True , stdout=output, stderr=error)


    def clean(self):
                
        sleep(20)
        for file_name in self.temp_files:
            try:
                os.remove(file_name)
            except os.error:
                pass
            pass    


    def run(self):

        for task in self.tasks:
            self.run_one_job(task)




    def clean(self):
        pass



class SshHost:
    def __init__(self, hostname):
        self.hostname= hostname
        self.lastupd= -16
        self.getAvailability()
        
    def getAvailability(self):
        # simple heuristic: mips / load
        t= time.time()
        if t - self.lastupd > 15: # min. 15 sec. before update
            self.bogomips= self.getBogomips()
            self.loadavg= self.getLoadavg()
            self.lastupd= t
            #print  self.hostname, self.bogomips, self.loadavg, (self.bogomips / (self.loadavg + 0.5))
        return self.bogomips / (self.loadavg + 0.5)
        
    def getBogomips(self):
        cmd= ["ssh", self.hostname ,"cat /proc/cpuinfo"]
        p= Popen(cmd, stdout=PIPE)
        bogomips= 0.0
        for l in p.stdout:
            if l.startswith('bogomips'):
                s= l.split(' ')
                bogomips+= float(s[-1])
        return bogomips

    def getLoadavg(self):
        cmd= ["ssh", self.hostname,"cat /proc/loadavg"]
        p= Popen(cmd, stdout=PIPE)
        l= p.stdout.readline().split(' ')
        return float(l[0])
        
    def addToLoadavg(self,n):
        self.loadavg+= n
        self.lastupd= time.time()

    def __str__(self):
        return "SshHost("+self.hostname+" <"+str(self.bogomips) \
               +','+str(self.loadavg) +','+str(self.getAvailability()) \
               +','+str(self.lastupd) + '>)'

    def __repr__(self):
        return str(self)
        
def find_all_ssh_hosts():
    return [SshHost(h) for h in set(pymake.get_distcc_hosts())]

def cmp_ssh_hosts(h1, h2):
    return cmp(h2.getAvailability(), h1.getAvailability())

class DBISsh(DBIBase):

    def __init__(self, commands, **args ):
        print "WARNING: The SSH DBI is not fully implemented!"
        print "Use at your own risk!"
        DBIBase.__init__(self, commands, **args)

        # check if log directory exists, if not create it
        if not os.path.exists(self.log_dir):
            os.mkdir(self.log_dir)

        # create the information about the tasks
        for command in commands:
            self.tasks.append(Task(command, self.log_dir, self.time_format,
                                   self.pre_tasks, self.post_tasks))
        self.hosts= find_all_ssh_hosts()
        

    def getHost(self):
        self.hosts.sort(cmp= cmp_ssh_hosts)
        #print "hosts= "
        #for h in self.hosts: print h
        self.hosts[0].addToLoadavg(1.0)
        return self.hosts[0]
    
    def run_one_job(self, task):
        DBIBase.run(self)

        host= self.getHost()


        cwd= os.getcwd()
        command = "ssh " + host.hostname + " 'cd " + cwd + "; " + string.join(task.commands,';') + "'"
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
        print "tasks= ", self.tasks
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



# creates an object of type ('DBI' + launch_system) if it exists
def DBI(commands, launch_system):
    try:
        str = 'DBI'+launch_system+'(commands)'
        jobs = eval('DBI'+launch_system+'(commands)')
    except NameError:
        print 'The launch system ',launch_system, ' does not exists. Available systems are: Cluster, Ssh, bqtools and Condor'
        traceback.print_exc()
        sys.exit(1)
    return jobs

def main():
    #    jobs = DBICluster(['ls','sleep 2'])
    jobs = DBI([
        'plearn_exp ${PLEARNDIR}/speedtest/sgrad.plearn task=letter_memvmat nout=26 nh1=100 nh2=100 nh3=100 slr=1e-1 dc=0 n=16001 epoch=16000 seed=1 mbs=10',
        'plearn_exp ${PLEARNDIR}/speedtest/sgrad.plearn task=letter_memvmat nout=26 nh1=100 nh2=100 nh3=100 slr=1e-1 dc=0 n=16001 epoch=16000 seed=1 mbs=20'
        #   './plearn dbn.pyplearn n_epochs_grad=250 n_hidden=25 grad_learning_rate=0.05 n_epochs_cd=0 cd_learning_rate=0.00001 unique_id=x25 recons_file=r.txt',
        #   './plearn dbn.pyplearn n_epochs_grad=250 n_hidden=35 grad_learning_rate=0.05 n_epochs_cd=0 cd_learning_rate=0.00001 unique_id=x35 recons_file=r.txt'
        ],'Ssh')
    jobs.run()
#    jobs.clean()

#    config['LOG_DIRECTORY'] = 'LOGS/'
if __name__ == "__main__":
    main()
