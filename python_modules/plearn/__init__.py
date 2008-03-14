"""The plearn namespace.

It shall englobe all and only python_modules that are relative to the
original PLearn branch (vs LisaPLearn or apstatsoft).
"""

# generically useful things...

def ifthenelse(cond,elsep,condp):
    if cond:
        return elsep
    else:
        return condp

# Mechanism to allow the user to choose which PLearn library should be used.
def getLib():
    try:
        return pl_lib_dir, pl_lib_name
    except:
        # Return default library.
        return 'plearn.pyext', 'plext'

def setLib(dir, name):
    global pl_lib_dir
    global pl_lib_name
    pl_lib_dir = dir
    pl_lib_name = name

