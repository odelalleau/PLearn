from policies import *
from infos import *

# Exclude begin, end. etc. temporarily until we figure out
# how to map them to Python iterators

exclude_list = [ 'begin', 'end', 'top', 'firstElement', 'lastElement',
                 'front', 'back', 'first', 'last', 'data',
                 '_static_initializer_',
                 # Exclude these because they are deprecated.
                 'load', 'save', 'read', 'write', 'oldread',
                 # Exclude these for now because they trigger
                 # a bug in Pyste.
                 'makeDeepCopyFromShallowCopy', 'deepCopy',
                 # Exclude this for now, because otherwise
                 # we get a Python exception when importing pylearn.
                 '_static_initializer_',
                 # Exclude this because there are two overloaded versions,
                 # one which requires a policy and the other which does not.
                 'getStats',
                 # Exclude TMat iterators
                 'compact_begin', 'compact_end', 'rowelements_begin',
                 'rowelements_end', 'rowdata',
                 # Exclude some more stuff to wrap TVec<string> and TMat<VMat>
                 'findIndices', 'find', 'input',
                 # Exclude rmi stuff
                 'getRemoteMethodMap', '_getRemoteMethodMap_',
                 ]

# We exclude the operator<< and operator>> because wrapping them using
# Boost.Python triggers a compiler bug.
exclude_operators = [ '<<', '>>' ]

def exclude_stuff(c, preserve_list=[]):
    for x in exclude_list:
        if x not in preserve_list and hasattr(c, x):
            exclude(getattr(c, x))
            
    if hasattr(c, 'operator'):
        for xop in exclude_operators:
            if xop not in preserve_list:
                exclude(c.operator[xop])

policy_mapping = { 'deepCopy': return_value_policy(manage_new_object),
                   'getOptionList': return_internal_reference(),
                   '_getOptionList_': return_internal_reference(),
                   '_new_instance_for_typemap_': return_value_policy(manage_new_object),
                   'getFieldInfos': return_internal_reference(),
                   'getStats': return_internal_reference()
                   }

def set_our_policy(c):
    for attr, pol in policy_mapping.iteritems():
        set_policy(getattr(c, attr), pol)


