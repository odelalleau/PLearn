"""Metaprogramming tools.

"""
import inspect, string, types
import plearn.utilities.toolkit   as     toolkit


__all__ = [ "classname", "instance_to_string", "members", "public_members" ]

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
    for member in obj_members:
        value = getattr(obj, member)            
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

def public_members( instance ):
    predicate = lambda x,y: not ( x.startswith("_")
                                  or inspect.ismethod(y)
                                  or inspect.isfunction(y)
                                  or inspect.isclass(y)
                                  )

    return members( instance, predicate )
