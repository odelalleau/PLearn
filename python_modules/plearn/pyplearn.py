"""
Pyplearn is a python preprocessor for .plearn files.

We use Python function definitions, loops and variables to ease the building
of .plearn files with lots of repetition. Because of the (almost) compatible
syntaxes of PLearn and Python, one can write the contents of a Python function
that generates PLearn pretty much as straight PLearn.
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


class DuplicateBindingError(Exception):
    """This exception is raised when attemping to bind more than one PLearn
    expression to the same variable name."""
    def __init__(self, binding_name):
        self.binding_name = binding_name

    def __str__(self):
        return "Binding '%s' already defined." % self.binding_name

class UnknownAgrumentError(Exception):
    """This exception is raised when attempting to use a PLearn argument that
    was not defined, either on the command-line or with plarg_defaults."""
    def __init__(self, arg_name):
        self.arg_name = arg_name

    def __str__(self):
        return "Unknown pyplearn argument: '%s'." % self.arg_name

class plearn_ref(object):
    """This class keeps in memory all the bindings done by bind. An
    instance of this class represents a specific binding."""

    _bindings = {}
    _bindings_referenced = {}
    _last_binding_index = 0
    
    def bind(cls, name, x):
        if name in cls._bindings:
            raise DuplicateBindingError(name)
        cls._bindings[name] = x
    bind = classmethod(bind)

    def __init__(self, name):
        self.name = name

    def value(self):
        """Called by _plearn_repr to return the PLearn representation
           of the variable binding. On the first call, it will return something
           like *1 -> PLearnStuff(blah). On subsequent calls it will return
           only a PLearn reference: *1;
        """
        name = self.name
        x = self._bindings[name]
        if name in self._bindings_referenced:
            binding_id = self._bindings_referenced[name]
            return '*' + _plearn_repr(binding_id) + ';'
        else:
            self._bindings_referenced[name] = self._last_binding_index + 1
            self._last_binding_index += 1
            return '*' + _plearn_repr(self._last_binding_index) + ' -> ' + _plearn_repr(x)

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
    elif isinstance(x, plearn_ref):
        return x.value()
    elif x is None:
        return "*0;"
    elif hasattr(x, 'plearn_repr') and callable(getattr(x, 'plearn_repr')):
        return _plearn_repr(x.plearn_repr())
    else:
        raise TypeError('Does not know how to handle type %s' % type(x))

def bind(name, value):
    """Binds name to the PLearn expression contained in value."""
    plearn_ref.bind(name, value)

def ref(name):
    """Makes a reference (either "*1 -> foo" or "*1;") to the value
       associated with name by a previous bind call."""
    return plearn_ref(name)

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
            raise UnknownAgrumentError(k)
plargs = _plargs_storage_readonly()

def _parse_plargs(args):
    """Parses PLearn command-line arguments (which look like foo=1 bar=2)
    and stores the value of the arguments as attributes to plargs."""
    for a in args:
        k, v = a.split('=', 1)
        plargs.__dict__[k] = plearn_snippet(v)


class _pyplearn_magic_module(object):
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

all = ['ref', 'bind', 'pair', 'TMat', 'plargs', 'plarg_defaults', 'pl']

