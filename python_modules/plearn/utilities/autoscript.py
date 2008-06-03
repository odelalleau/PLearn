# autoscript.py
# Copyright (C) 2008 Pascal Vincent
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

# Author: Pascal Vincent


import sys, inspect, types

class AutoscriptError(Exception):
    def __init__(self, message):
        self.message = message
    def __str__(self):
        return repr(self.message)


def parse_command_line(argv):
    """argv is expected to be a list as the one in sys.argv.
    The call returns (args, kwargs)
    args is the list of initial non-keyword arguments passed.
    kwargs is a dictionary of keyword arguments.
    """
    args = []
    kwargs = {}
    for arg in argv[1:]:
        if arg.find('=')>=0:
            key, val = arg.split('=',1)
            kwargs[key.strip()] = val.strip()
        else:
            if kwargs:
                raise AutoscriptError('You cannot specify positional arguments after you specified keyword arguments: use key=value syntax for argument '+str(arg))
            args.append(arg)
    return args, kwargs

def prefix_lines(prefix, text):
    return "\n".join([prefix+line for line in text.split('\n')])
    
def autocomplete(args, all_argnames):
    """This function autocompletes the names in args from an official list of all_argnames
    if it can be done unambiguously.
    args can be either a single string or a list of strings or a map whose keys are string.
    In all these cases an object of the same kind is returned with the strings autocompleted.
    """       
    if type(args) is str:
        prefix = args
        matches = [ arg for arg in all_argnames if arg.startswith(prefix) ]
        if len(matches)==0:
            raise AutoscriptError("No matching argument name completion for "+prefix+"... in "+str(all_argnames))  
        elif len(matches)>1:
            raise AutoscriptError("Ambiguity in completion of "+prefix+"... Possible names are: "+str(matches))
        return matches[0]
    
    elif type(args) is list:
        return [autocomplete(arg, all_argnames) for arg in args]

    elif type(args) is dict:
        compldict = {}
        for key, val in args.items():
            compldict[autocomplete(key, all_argnames)] = val
        return compldict

    
def check_args(args, kwargs, all_argnames, default_values):
    """Verify if the arguments provided in args list and kwargs dictionary
    are compatible with calling a function defined with specified all_argnames and default_values.
    Raise explicit AutoscriptError if not.
    Upon success, returns a full_kwargs, corresponding to kwargs completed with the name:value of
    given positional args.
    Ex: check_args(['a','b','c'],[35], [1,2], {'c':3})

    """
    # check if all non default arguments have been provided
    pos_of_first_default = len(all_argnames)-len(default_values)
    for argname in all_argnames[len(args):pos_of_first_default]:
        if argname not in kwargs:
            raise AutoscriptError("Missing mandatory argument: "+argname)

    # check if all specified kwargs are valid argnames
    invalid_names = [ argname for argname in kwargs.keys() if argname not in all_argnames ]
    if len(invalid_names)>0:
        raise AutoscriptError("Unknown arguments: "+str(invalid_names))

    # build a full_kwars
    full_kwargs = kwargs.copy()
    for i in range(len(args)):
        value = args[i]
        try:
            name = all_argnames[i]
        except IndexError:
            raise AutoscriptError("Too many arguments provided.")
        if name in full_kwargs:
            raise AutoscriptError("Argument "+name+" is duplicated: provided both as positional argument at position "+str(i+1)+" and as a keyword argument")
        full_kwargs[name] = value
    return full_kwargs


def print_call_arguments(kwargs, all_argnames, default_values):
    n = len(default_values)
    defaults_dict = dict(zip(all_argnames[-n:], default_values))

    for argname in all_argnames:
        if argname in kwargs:
            print "# "+argname+"="+repr(kwargs[argname])
        else:
            print "# "+argname+"="+repr(defaults_dict[argname])+ "   (default value)"            

def eval_str_argument_values(kwargs, all_argnames, default_values):
    n = len(default_values)
    defaults_dict = dict(zip(all_argnames[-n:], default_values))
    for key, val in kwargs.items():        
        if type(val) is str and val!='':
            if ((key not in defaults_dict) or (type(defaults_dict[key]) is not str) or val[0] in ("'",'"')):
                try:
                    kwargs[key] = eval(val)
                except:
                    pass

def mystr(s):
    if s=="" or s==" ":
        return repr(s)
    else:
        return str(s)

def quote_if_needed(s):
    if ' ' in s:
        s = '"'+s+'"'
    return s

def usage_text(scriptname, all_argnames, default_values):
    defvalpos = len(all_argnames)-len(default_values)
    txt = "Usage: "+scriptname+"  "+\
          "  ".join(all_argnames[0:defvalpos])+"  "+\
          "  ".join(quote_if_needed(name+'='+mystr(value)) for name,value in zip(all_argnames[defvalpos:],default_values))
    return txt

def autoscript(callme, autocomplete_names=False, helptext="", argv=sys.argv):
    """Call autoscript as a one-liner to turn a callable (function, class or object) into a command-line script.
    Typical usage goes as follows:
    if __name__ == '__main__':
       from plearn.utilities.autoscript import autoscript
       autoscript(myFuncionOrClassOrObjectToCall)

    If you choose to set autocomplete_names to True, then the 
    user can abreviate optionnames as long as there is no ambiguity.

    If the script is invoked with no arguments,
    a detailed help will be printed constituted of:
    - *** Help on scriptname ***
    - The optionally provided helptext
    - A usage line listing all arguments and their default values
    - The docstring associated to the callable.
    """
    
    if not callable(callme):
        raise ValueError("First argument to autoscript must be callable: either a function or a class with an __init__ method or a callable instance""")

    doctext=""

    if type(callme) is types.FunctionType:
        all_argnames, varargs, varkw, default_values = inspect.getargspec(callme)
        if callme.__doc__ is not None:
            doctext = callme.__doc__
        
    elif type(callme) is types.ClassType:
        all_argnames, varargs, varkw, default_values = inspect.getargspec(callme.__init__)
        all_argnames = all_argnames[1:] # skip self
        if callme.__doc__ is not None:
            doctext = callme.__doc__
        if callme.__init__.__doc__ is not None:
            doctext = doctext+"\n"+callme.__init__.__doc__
            
    elif type(callme) is types.InstanceType:
        all_argnames, varargs, varkw, default_values = inspect.getargspec(callme.__call__)
        all_argnames = all_argnames[1:] # skip self
        #if callme.__doc__ is not None:
        #    doctext = callme.__doc__
        if callme.__call__.__doc__ is not None:
            doctext = doctext+"\n"+callme.__call__.__doc__

    if default_values is None:
        default_values = []

    scriptname = argv[0]
    if len(argv)<=1:
        print
        print "#"*80
        print "# Help on "+scriptname
        print prefix_lines("# ",helptext)
        print "# "+usage_text(scriptname, all_argnames, default_values)
        print prefix_lines("# ",doctext)
        print "#"*80
        print
        sys.exit()
        
    # Parse arguments
    try:
        args, kwargs = parse_command_line(argv)

        if(autocomplete_names):
            kwargs = autocomplete(kwargs, all_argnames)

        kwargs = check_args(args, kwargs, all_argnames, default_values)
        eval_str_argument_values(kwargs, all_argnames, default_values)
        print
        print "#"*80
        print "# Calling "+scriptname+" with following arguments: "
        print_call_arguments(kwargs, all_argnames, default_values)
        print "#"*80
        print
    except AutoscriptError, inst:
        print
        print "#"*80
        print "# Error when invoking "+scriptname
        print prefix_lines("# ",inst.message)
        print "# "+usage_text(scriptname, all_argnames, default_values)
        print "# Invoke "+scriptname+" with no arguments to display detailed help."
        print "#"*80
        print
        sys.exit()

    # Call the callable
    return callme(**kwargs)
