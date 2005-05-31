"""Metaprogramming tools.

"""
__cvs_id__ = "$Id: metaprog.py,v 1.5 2005/05/31 14:38:02 dorionc Exp $"

import inspect, string, types
import plearn.utilities.toolkit   as     toolkit


__all__ = [ "classname", "instance_to_string", "members",
            'public_attribute_predicate', 'public_attributes',
            'public_members'
            ]

def classname(obj):
    if inspect.isclass(obj):
        return obj.__name__

    if inspect.isinstance(obj):
        return obj.__class__.__name__

    raise TypeError( type(obj) )

def instance_to_string(obj, obj_members=None, sp=' '):
    if obj_members is None:
        obj_members = public_members(obj)
    
    members = []
    for (member, value) in obj_members.iteritems():
        if isinstance(value, types.StringType):
            members.append( '%s = %s' % (member, toolkit.quote(value)) )
        else:
            members.append( '%s = %s' % (member,value) )

    indent = ''
    if sp == '\n':
        indent = '    '

    return ( "%s(%s%s%s%s)"
             % ( obj.classname(),
                 sp, indent,
                 string.join(members, ',%s%s'%(sp, indent)),
                 sp
                 )
             )

def members( instance, predicate=(lambda x,y: True) ):
    return dict([ (x,y)
                  for (x,y) in inspect.getmembers(instance)
                  if predicate(x, y)
                  ])

def public_attribute_predicate( name, value ):
    """Return whether or not attribute name is considered a public attribute.

    Are consider public attributes whose I{name} does not an underscore and
    I{value} is not a method, a function or a class.
    """
    return not ( name.startswith("_")
                 or inspect.ismethod(value)
                 or inspect.isfunction(value)
                 or inspect.isclass(value)
                 or inspect.isbuiltin(value)
                 )

def public_attributes( instance ):
    return members( instance, public_attribute_predicate )

def public_members( instance ):
    return members( instance, public_attribute_predicate )
