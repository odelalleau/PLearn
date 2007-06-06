
import plearn.bridgemode

# whether we use the plearn server or python extension bridge
# between PLearn and python
#
if plearn.bridgemode.useserver:
    from plearn.pyplearn import *
    from plearn.io.server import *
    import time
    plargs.parse([ arg for arg in sys.argv if arg.find('=') != -1 ]) # Parse command-line.
    server_command = plearn.bridgemode.server_exe + ' server'
    serv = launch_plearn_server(command = server_command)
    time.sleep(1) # give some time to the server to get born and well alive
else:
    from plearn.pyext import *
# from gdb do the following to see PLearn symbols AFTER pyext has been loaded
# add-symbol-file ~/PLearn/python_modules/plearn/pyext/libplextdbg.so

if plearn.bridgemode.interactive:
    from pylab import *
    
