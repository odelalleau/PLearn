class plearn_snippet:
    def __init__(self, s):
        self.s = s

    def __str__(self):
        return self.s


class DuplicateBindingError(Exception):
    def __init__(self, binding_name):
        self.binding_name = binding_name

    def __str__(self):
        return "Binding '%s' already defined." % self.binding_name

class UnknownAgrumentError(Exception):
    def __init__(self, arg_name):
        self.arg_name = arg_name

    def __str__(self):
        return "Unknown pyplearn argument: '%s'." % self.arg_name

class plearn_ref(object):

    _bindings = {}
    _bindings_referenced = {}
    _last_binding_index = -1
    
    def bind(klass, name, x):
        if name in klass._bindings:
            raise DuplicateBindingError(name)
        klass._bindings[name] = x
    bind = classmethod(bind)

    def __init__(self, name):
        self.name = name

    def value(self):
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
    elif isinstance(x, plearn_snippet):
        return x.s
    elif isinstance(x, plearn_ref):
        return x.value()
    else:
        raise TypeError('Does not know how to handle type %s' % type(x))

def ref(name):
    return plearn_ref(name)

def bind(name, value):
    plearn_ref.bind(name, value)

def pair(a, b):
    return plearn_snippet(_plearn_repr(a) + ':' + _plearn_repr(b))

def pmap(*pairs):
    s = "{"
    n = len(p)
    for (i, p) in enumerate(pairs):
        if i < n-1:
            s += _plearn_repr( p ) + ","
        else:
            s += _plearn_repr( p )
    s += "}"
    return plearn_snippet(s)
            
def TMat(x, y, mat):
    return plearn_snippet(_plearn_repr(x) + ' ' + _plearn_repr(y) + ' [' +
                          ', '.join([_plearn_repr(e) for e in mat]) +']')

class _plargs_storage_fallback:
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
    for a in args:
        k, v = a.split('=', 1)
        plargs.__dict__[k] = plearn_snippet(v)


class _pyplearn_magic_module(object):
    indent = ' ' * 4
    
    def add_indent(self, s):
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

