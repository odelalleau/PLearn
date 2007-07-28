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

