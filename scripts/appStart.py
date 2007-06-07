__cvs_id__ = "$Id$"

import string, sys, fpformat, os, copy, time
from popen2 import *

### Take care not to override:
undefined = '#undefined'
selected_options = []
targets = []
option_value = {}
current_mode = ""

##################
# defines for writeln
openingTag = 1
nothing = 0
closingTag = -1
opened = 0
##################

##################
# Usefull defines
true = 1
false = 0

#if you need a DEBUG option, there is no need to implement it, use this flag
debug = false  
##################

##################
# INTERNAL
#__possible_options__ = []
##################

## def appS_select_mode__(args, modes):
##     #print(args)
##     #print(modes)
##     #print("selected_options: " + str(selected_options))
##     for m in modes.keys():
##         get_option_value(args, m, undefined)

##     #print("selected_options: " + str(selected_options))

##     if len(selected_options) != 1:
##         print("Exactly one of " + str(modes.keys()) + " must be provided!")
##         sys.exit()

##     global current_mode
##     current_mode = selected_options.pop()

def default_usage():
    print("BAD USAGE! (appStart.default_usage fct called)")

def appStart(_option_value, usage=default_usage, min_targets_len=1, max_targets_len=1000000, _modes={}):
    """
    The following lines are wanted to be totally independant of the
    application. To be used in any app, you'll need to provide an
    option_value dictionnary the following way:
     - keys are the supported options
     - values are
       1) default value for option
       or
       2) undefined if the option needs no value
       or
       3) check function for the option value
    """
    # get the option arguments
    args = sys.argv[:]
    del args[0] # ignore program name

    ##__possible_options__ = _option_value.keys()

    ### Looking for modes before args
    global current_mode
    if len(_modes) > 0:
        if len(args) == 0:
            usage()
            ##options_usage()
            sys.exit()

        current_mode = args.pop(0)
        if not current_mode in _modes.keys():
            print("Mode " + current_mode + " not in " + str(_modes.keys()))
            sys.exit()
    ###

    
    ### DEBUG implementation
    _option_value["debug"] = undefined
    ###

    supported_options = _option_value.keys()
    for key in supported_options:
        _option_value[key] = get_option_value(args, key, _option_value[key])


    ### DEBUG implementation
    if "debug" in selected_options:
        print("+++ Running under debug mode")
        global debug
        debug = true
    ###
        

    ### Syncronizing 'option_value' with '_option_value'
    global option_value
    for k in _option_value.keys():
        option_value[k] = _option_value[k]
    ###
        
    for arg in args:
        if arg[0]=='-':
            print("Asked for '" + arg[1:] + "' option, which is not in \n\t" +
                  str(supported_options) )
            sys.exit()
        else:
            targets.append(arg)

    lTar = len(targets)
    if lTar < min_targets_len or lTar > max_targets_len:
        usage()
        sys.exit()

    if current_mode != "":
        _modes[current_mode]()
#end of: def appStart()

_debug__ = 0
def DEBUG(str, stop=false):
    if not debug:
        return

    global _debug__
    _debug__ = _debug__ + 1
    print("\nDEBUG" + fpformat.fix(_debug__, 0))
    
    if stop:
        raw_input(str)
    else:
        print(str)

def ERROR(msg):
    print("ERROR: " + msg)
    sys.exit()

def command_output(command):
    tmp_file = ".appStart_command_output_tmp_file"
    sort_command = string.join([command, ">>", tmp_file])
    os.system(sort_command)
    file = open(tmp_file, 'r')
    lines = file.readlines()
    file.close()
    os.remove(tmp_file)
    return lines

def get_option_value(args, option, default):
    try:
        vindex = args.index("-" + option)

        # if we got here, it means that the option is in the args
        args.pop(vindex)
        selected_options.append(option)

        # if default is "undefined", the option does not need a value
        if default == undefined:
            return default

        value = "-" # see why later
        if vindex < len(args):
            value = args.pop(vindex)

        # here is why was done value = "-"
        if string.find(value, "-") != -1:
            # means that you have "-opt -other_opt" or vindex >= len(args)
            print("Must provide a value for option -" + option)
            sys.exit()

        # !NEW 27 sept 2002!
        # If 'default' is a function, it's a check function for 'value'
        try:
            if not default( value ):
                sys.exit() #the check function must take care of the message    
        except TypeError:
            pass
        
        return value

    # If the option was not given in the command line
    except ValueError:
        return default

def launch(cmd, logfile=''):
    if logfile == '':
        logfile = time_string()

    print("Launching\n\n" + cmd + "\n\n Outputs will be stored in " + logfile)
    os.system(string.join(['nohup', cmd, ">&", logfile, "&"]))


def int2string(num):
    return fpformat.fix(num, 0) 

## def options_usage():
##     if len(__possible_options__) == 0:
##         return

def time_string():
    t = time.localtime()
    return ( int2string(t[0]) + "_" + int2string(t[1]) + "_" + int2string(t[2])
             + "_" +
             int2string(t[3]) + ":" + int2string(t[4]) + ":" + int2string(t[5]) )

def tostring(num, prec=2):
    return fpformat.fix(num, prec)

def writeln(fd, str = '', occurs = nothing):
    """Formatted line writting"""
    global opened
    if occurs == closingTag:
        opened = opened-1
        
    openedStr = ''
    for i in range(0, opened):
        openedStr = string.join([openedStr, '\t'], "")
    fd.write(openedStr + str  + '\n') ### + "opened: " + fpformat.fix(opened,0)

    if occurs == openingTag:
        opened = opened+1
