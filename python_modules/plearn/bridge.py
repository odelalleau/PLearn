
import plearn.bridgemode

# whether we use the plearn server or python extension bridge
# between PLearn and python
#
if plearn.bridgemode.useserver:
    from plearn.pyplearn import *
    from plearn.io.server import *
    import time
    server_command = 'plearn_curses server'
    serv = launch_plearn_server(command = server_command)
    time.sleep(0.5) # give some time to the server to get born and well alive
else:
    from plearn.pyext import *
# from gdb do the following to see PLearn symbols AFTER pyext has been loaded
# add-symbol-file ~/PLearn/python_modules/plearn/pyext/libplextdbg.so

if plearn.bridgemode.interactive:
    from pylab import *
    
