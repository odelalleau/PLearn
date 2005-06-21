import copy, string, time

BUILTIN_IMPORT_FUNCTION = __import__
PROJECTS                = {}

def declare_project( project_name ):
    return PythonProject(project_name)

def official_build( project_name, build_version, fixlevels ):
    PROJECTS[project_name].official_build(build_version, fixlevels)

def declare_module( module_name, version_id ):
    """version_id = \"$Id$\"
    """
    ## print "declare_module: %s" % module_name
    for project in PROJECTS.itervalues():
        project.declare_module(module_name, version_id)

def project_version( project_name, extended=False ):
    return PROJECTS[project_name].version(extended)

def versionned_import(name, globals=None, locals=None, fromlist=None):
    ## raw_input("MY IMPORT: %s" % name)    
    mod = BUILTIN_IMPORT_FUNCTION(name, globals, locals, fromlist)
    
    if hasattr(mod, '__version_id__'):
        version_str = getattr(mod, '__version_id__')
        declare_module(mod.__name__, version_str)
    return mod

## From now on, the import statement will use the above function!
import __builtin__
__builtin__.__import__ = versionned_import
    
__all__ = [ "declare_project", "official_build", "project_version", 
            ]

declare_module( __name__, "$Id$" )

class PythonProject:
    def __init__(self, name):
        if PROJECTS.has_key(name):
            raise ValueError("Duplicate project name %s."%name)

        self.name           = name
        self.modules        = {}
        self.build_versions = [([0, 0], 0)]  
        self.neglected      = []
        self._version       = None
        PROJECTS[name]      = self

##     def declare_module(self, module_name, version_id):
##         bflag = ",v"
##         the_year = time.localtime()[0]

##         begin = string.find(version_id, bflag) + len(bflag)
##         if begin == -1:
##             raise ValueError( "%s - %s: CVS Id string %s is invalid."
##                               % (self.name, module_name, version_id)
##                              )

##         end = string.find(version_id, str(the_year))
##         while end == -1:
##             the_year -= 1
##             end       = string.find(version_id, str(the_year))
##             if the_year == 2003: ## The python_modules were added in PLearn in 2004!
##                 raise ValueError( "The module's version_id does not appear "
##                                   "to contain any date (%s)."
##                                   % version_id
##                                   )

##         version_tuple = [ int(s)
##                           for s in string.split(string.strip( version_id[begin:end] ), '.') ]

##         self.modules[module_name] = version_tuple
    def declare_module(self, module_name, version_id):
        tokens  = version_id.split(' ')
        version = int( tokens[2]
                       )        
        self.modules[module_name] = version
        

    def neglect_module(self, module_name):
        self.neglected.append(module_name)
        
    def official_build(self, build_version, fixlevels):
        assert len(build_version) == 2
        assert len(fixlevels)     == 2
        self.build_versions.append( (build_version, fixlevels) )
        
    def version(self, extended):
        if self._version:
            return self._version

        minv = [0, 1e06]
        maxv = [0, -1]
        sumv = 0

        ## Extended versionning support
        min_name        = None
        max_name        = None 
        formatted_names = []
        def formatter(name, vtup):
            formatted  = string.ljust(name, 40)
            formatted += string.join([str(v) for v in vtup], ".")
            return formatted
        
        for (module_name, v) in self.modules.iteritems():
            if module_name in self.neglected: continue

            sumv += v
            if v < minv:
                minv     = v
                min_name = module_name
            if v > maxv:
                maxv     = v
                max_name = module_name

            if extended:
                formatted_names.append( formatter(module_name,v) )

        version_tuple = self.version_tuple( maxv )        
        self._version = string.join( [str(v) for v in version_tuple], '.' )

        if extended:
            formatted_names.sort()
            formatted_names.extend([ "\nMIN:", formatter(min_name, minv),
                                     "\nMAX:", formatter(max_name, maxv),
                                     
                                     ])
            return ":\n    %s\n\n%s %s"\
                   % ( string.join(formatted_names, "\n    "),
                       self.name, version_str
                       )
        return self._version

    def version_tuple( self, fixlevel ):
        bversion, fix = self.build_versions[-1]
        
        return bversion + [ fixlevel - fix, fixlevel ]

