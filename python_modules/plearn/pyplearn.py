"""Pyplearn is a python preprocessor for .plearn files.

We use Python function definitions, loops and variables to ease the building
of .plearn files with lots of repetition. Because of the (almost) compatible
syntaxes of PLearn and Python, one can write the contents of a Python function
that generates PLearn pretty much as straight PLearn.

Here is a little introductory tutorial.

Syntax' Basics
---------------

First lets say you have a plearn file that looks like:

   $DEFINE{DATASET}{ AutoVMatrix( specification = "somefile.pmat";
                                  inputsize     = 10; 
                                  targetsize    = 1              ) }
   
   PTester( expdir = "constant";
            dataset = ${DATASET};
            statnames = ["E[train.E[mse]]" "V[test.E[mse]]"];
            save_test_outputs = 1;
            provide_learner_expdir = 1;
            
            learner = SomeLearnerClassTakingNoOption();
            ) # end of PTester

Then, the pyplearn file will be:

   dataset = pl.AutoVMatrix( specification = "somefile.pmat",
                             inputsize     = 10, 
                             targetsize    = 1              )

   def main():
       return pl.PTester( expdir = "constant",
                          dataset = dataset,
                          statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                          save_test_outputs = 1, 
                          provide_learner_expdir = 1,
                       
                          learner = pl.SomeLearnerClassTakingNoOption();
                          ) # end of PTester

First, note that the python affectation works just like the $DEFINE
plearn statement. Also note that the python syntax requires the use of
commas (,) instead of semicolons (;) to separate the options
provided. You can also see that python lists [] are mapped to plearn
TVec's and that there elements must be separated by commas rather that by
spaces. Finally, any pyplearn file must contain a single main()
function that returns the representation of a runnable PLearn object. 

Now, one may wonder what does the 'pl' stands for in 'pl.AutoVMatrix',
'pl.PTester' or 'pl.SomeLearnerClassTakingNoOption()'.  To answer that
question, we must first say that the pyplearn mecanism acts as an
additionnal layer over the plearn scripts mecanism, which means that
the pyplearn driver simply reads the pyplearn file and generates the
corresponding plearn script to be read by plearn. The 'pl' object emulates
a module that, for any PLearn class, accepts all options you may wish
to provide. 

Remember that the pyplearn mecanism IS NOT a bridge between the C++
PLearn library and python, it simply is another parsing layer that
provides us with the power of a programming language. Therefore, the 'pl'
instance is not really awared of the PLearn classes, which means that
you could write

   pl.NameOfAClassThatDoesNotExist( any_option_you_like = "bla",
                                    foo                 = ["bar"] )

and the pyplearn driver would generate the following plearn script

   NameOfAClassThatDoesNotExist( any_option_you_like = "bla";
                                 foo                 = ["bar"] )

that would fail when plearn would try to create a
NameOfAClassThatDoesNotExist object. Therefore, we do not have to
maintain any list of 'known classes' and every new class you may write
will be managed by the pyplearn mecanism since 'pl' simply acts as a
parser.

The $INCLUDE statement
-----------------------

If you are used to program complex plearn scripts, you may have
developed a tendency to break down your script in simpler and smaller
plearn files that are to be included in the final plearn script. Lets
say that you have a file, dataset.plearn, that defines your dataset, i.e.

 ## dataset.plearn
   $DEFINE{DATASET}{ AutoVMatrix( specification = "somefile.pmat";
                                  inputsize     = 10; 
                                  targetsize    = 1              ) }
 ## end of dataset.plearn

and that you used to include this file in your final plearn script:

 (final_script.plearn)
   $INCLUDE{dataset.plearn}
 
   PTester( expdir = "constant";
            dataset = ${DATASET};
            statnames = ["E[train.E[mse]]" "V[test.E[mse]]"];
            save_test_outputs = 1;
            provide_learner_expdir = 1;
            
            learner = SomeLearnerClassTakingNoOption();
            ) # end of PTester
 ## end of final_script.plearn

Being now under the python syntax, you have to create a python module

 ## dataset.py
   dataset = pl.AutoVMatrix( specification = "somefile.pmat",
                             inputsize     = 10, 
                             targetsize    = 1              )
 ## end of dataset.py

and include it in the python fashion

 ## final_script.pyplearn
   from dataset import *
   
   def main():
       return pl.PTester( expdir = "constant",
                          dataset = dataset,
                          statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                          save_test_outputs = 1, 
                          provide_learner_expdir = 1,
                       
                          learner = pl.SomeLearnerClassTakingNoOption();
                          ) # end of PTester
 ## end of final_script.pyplearn

Note that the include(filename) function defined below does not act exactly
   
Bindings
---------

bind(name, x)
ref(name)

"""

class plearn_snippet:
    """Objects of this class are used to wrap the parts of the Python code
       that have already be converted to a PLearn string, so they don't
       get the same treatment as Python strings (which will get wrapped in
       double quotes by _plearn_repr).
    """
    def __init__(self, s):
        self.s = s

    def __str__(self):
        return self.s

    def __add__(self, snippet):
        if not isinstance(snippet, plearn_snippet):
            raise TypeError("plearn_snippet.__add__ expects a plearn_snippet instance")
        self.s += snippet.s

class DuplicateBindingError(Exception):
    """This exception is raised when attemping to bind more than one PLearn
    expression to the same variable name."""
    def __init__(self, binding_name):
        self.binding_name = binding_name

    def __str__(self):
        return "Binding '%s' already defined." % self.binding_name

class UnknownArgumentError(Exception):
    """This exception is raised when attempting to use a PLearn argument that
    was not defined, either on the command-line or with plarg_defaults."""
    def __init__(self, arg_name):
        self.arg_name = arg_name

    def __str__(self):
        return "Unknown pyplearn argument: '%s'." % self.arg_name


_name_to_id = {}
_id_to_binding = {}
_last_binding_index = 0

def bind(name, x):
    """Binds name to the PLearn expression contained in value."""
    global _last_binding_index
    if name in _name_to_id:
        raise DuplicateBindingError(name)
    _id_to_binding[_last_binding_index+1] = x
    _name_to_id[name] = _last_binding_index + 1
    binding_id = _last_binding_index + 1
    _last_binding_index +=1

def ref(name):
    """Makes a reference (with "*1;") to the value
       associated with name by a previous bind call."""
    return plearn_snippet('*' + _plearn_repr(_name_to_id[name]) + ';')

def plvar(variable_name):
    return plearn_snippet('${%s}' % variable_name)

def _postprocess_refs(s):
    """Must be called with the *complete* string of the generated .plearn.
       Finds the first instance of each PLearn reference (eg. *1;) and
       replaces it by the correct definition (eg. *1 -> foo(blah...)."""
    for i in range(1, _last_binding_index+1):
        s = s.replace("*%d;" % i,
                      "*%d -> %s" % (i, _plearn_repr(_id_to_binding[i])), 1)
    return s
    

def _plearn_repr(x):
    """Returns a string that is the PLearn representation
    of the corresponding Python object."""
    if isinstance(x, float):
        # Don't use repr, so we don't get 0.20000000000000001 for 0.2
        return str(x)
    elif isinstance(x, int):
        return str(x)
    elif isinstance(x, str):
        return '"' + x + '"'
    elif isinstance(x, list):
        return str(len(x)) + ' [' + ', '.join([_plearn_repr(e) for e in x]) + ']'
    elif isinstance(x, dict):
        dict_items = [_plearn_repr(k) + ': ' + _plearn_repr(v) for k, v in x.iteritems()]
        return '{' + ', '.join(dict_items) + '}'
    elif isinstance(x, tuple) and len(x) == 2:
        return  _plearn_repr(x[0]) + ':' + _plearn_repr(x[1])
    elif isinstance(x, plearn_snippet):
        return x.s
    elif x is None:
        return "*0;"
    elif hasattr(x, 'plearn_repr') and callable(getattr(x, 'plearn_repr')):
        return _plearn_repr(x.plearn_repr())
    else:
        raise TypeError('Does not know how to handle type %s' % type(x))

def TMat(num_rows, num_cols, mat_contents):
    """Instances of this class represent a PLearn TMat.

    num_rows and num_cols are the number of rows and columns of the matrix.
    The contents of the matrix is given to mat_contents as a Python list.

    Example: in Python, TMat(2, 3, [1, 2, 3, 4, 5, 6]) gives
             in PLearn: 2 3 [1 2 3 4 5 6]
    """
    return plearn_snippet(_plearn_repr(num_rows) + ' ' +
                          _plearn_repr(num_cols) + ' [' +
                          ', '.join([_plearn_repr(e) for e in mat_contents]) +
                          ']')

def include(filename):
    """Includes the contents of a .plearn file."""
    f = open(filename, 'U')
    try:
        include_contents = f.read()
    finally:
        f.close()
    return plearn_snippet(include_contents)

class _plargs_storage_fallback:
    """A singleton instance of this class is instanciated by the package
    to store the default values for PLearn command-line variables."""
    pass
plarg_defaults = _plargs_storage_fallback()


class _plargs_storage_readonly:
    def __setattr__(self, k, v):
        raise AttributeError('Cannot modify plargs')

    def __getattr__(self, k):
        try:
            return getattr(plarg_defaults, k)
        except AttributeError:
            raise UnknownArgumentError(k)
plargs = _plargs_storage_readonly()

def _parse_plargs(args):
    """Parses PLearn command-line arguments (which look like foo=1 bar=2)
    and stores the value of the arguments as attributes to plargs."""
    for a in args:
        k, v = a.split('=', 1)
        plargs.__dict__[k] = v


class _pyplearn_magic_module:
    """An instance of this class (instanciated as pl) is used to provide
    the magic behavior whereas bits of Python code like:
    pl.SequentialAdvisorSelector(comparison_type='foo', etc.) become
    a string that can be fed to PLearn instead of a .plearn file."""
    indent = ' ' * 4
    
    def add_indent(self, s):
        """Adds a level of indentation to the (potentially multi-line) strings
        passed as s."""
        s = s.strip()
        if not '\n' in s:
            # Single-line expression, no need to add any tabs
            return s

        lines = s.split('\n')
        # Add a leading indentation to all lines except the first one
        for i in range(1, len(lines)):
            lines[i] = self.indent + lines[i]
        return '\n'.join(lines)

    def __getattr__(self, name):
        if name.startswith('__'):
           raise AttributeError
       
        def printfunc(**kwargs):
            s = [name, '(\n']

            num_args_printed = 0
            num_args = len(kwargs)
            
            for key, value in kwargs.iteritems():
                s.append(self.indent)
                s.append(key)
                s.append(' = ')
                value_repr = _plearn_repr(value)
                s.append(self.add_indent(value_repr))
                if num_args_printed != num_args - 1:
                    # Add a comma after every arg except the last one.
                    s.append(',')
                s.append('\n')
                num_args_printed += 1
            s.append(self.indent + ')\n')
            return plearn_snippet(''.join(s))
        
        return printfunc

pl = _pyplearn_magic_module()

all = ['ref', 'bind', 'TMat', 'plargs', 'plarg_defaults', 'pl', 'include']

