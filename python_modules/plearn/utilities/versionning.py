import string, time

BUILTIN_IMPORT_FUNCTION = __import__
PROJECTS                = {}

def declare_project(project_name):
    return PythonProject(project_name)

def official_build(project_name, build_version, fixlevels):
    PROJECTS[project_name].official_build(build_version, fixlevels)

def declare_module(module_name, cvs_id):
    """cvs_id = \"$Id: versionning.py,v 1.2 2005/01/05 19:23:21 dorionc Exp $\"
    """
    ## print "declare_module: %s" % module_name
    for project in PROJECTS.itervalues():
        project.declare_module(module_name, cvs_id)

def project_version(project_name, extended=False):
    return PROJECTS[project_name].version(extended)

def versionned_import(name, globals=None, locals=None, fromlist=None):
    ## raw_input("MY IMPORT: %s" % name)    
    mod = BUILTIN_IMPORT_FUNCTION(name, globals, locals, fromlist)
    
    if hasattr(mod, '__cvs_id__'):
        version_str = getattr(mod, '__cvs_id__')
        declare_module(mod.__name__, version_str)
    return mod

## From now on, the import statement will use the above function!
import __builtin__
__builtin__.__import__ = versionned_import
    
__all__ = [ "declare_project", "official_build", "project_version", 
            ]

declare_module( __name__, "$Id: versionning.py,v 1.2 2005/01/05 19:23:21 dorionc Exp $" )

class PythonProject:
    def __init__(self, name):
        if PROJECTS.has_key(name):
            raise ValueError("Duplicate project name %s."%name)

        self.name           = name
        self.modules        = {}
        self.build_versions = []
        self.neglected      = []
        PROJECTS[name]      = self
        
    def declare_module(self, module_name, cvs_id):
        bflag = ",v"
        the_year = time.localtime()[0]

        begin = string.find(cvs_id, bflag) + len(bflag)
        if begin == -1:
            raise ValueError( "%s - %s: CVS Id string %s is invalid."
                              % (self.name, module_name, cvs_id)
                             )

        end = string.find(cvs_id, str(the_year))
        while end == -1:
            the_year -= 1
            end       = string.find(cvs_id, str(the_year))
            if the_year == 2003: ## The python_modules were added in PLearn in 2004!
                raise ValueError( "The module's cvs_id does not appear "
                                  "to contain any date (%s)."
                                  % cvs_id
                                  )

        version_tuple = [ int(s)
                          for s in string.split(string.strip( cvs_id[begin:end] ), '.') ]

        self.modules[module_name] = version_tuple

    def neglect_module(self, module_name):
        self.neglected.append(module_name)
        
    def official_build(self, build_version, fixlevels):
        assert len(build_version) <= 3
        assert len(fixlevels)     == 3
        self.build_versions.append( (build_version, fixlevels) )
        
    def version(self, extended):
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
        
        for (module_name, vtup) in self.modules.iteritems():
            if module_name in self.neglected: continue
            v = vtup[1]

            sumv += v
            if v < minv[1]:
                minv     = vtup
                min_name = module_name
            if v > maxv[1]:
                maxv     = vtup
                max_name = module_name

            if extended:
                formatted_names.append( formatter(module_name,vtup) )
                
        fixlevels     = [minv[1], maxv[1], sumv]
        version_tuple = self.version_tuple( fixlevels )         
        version_str   = string.join( [str(v) for v in version_tuple], '.' )

        if extended:
            formatted_names.sort()
            formatted_names.extend([ "\nMIN:", formatter(min_name, minv),
                                     "\nMAX:", formatter(max_name, maxv),
                                     
                                     ])
            return ":\n    %s\n\n%s %s"\
                   % ( string.join(formatted_names, "\n    "),
                       self.name, version_str
                       )
        return version_str

    def version_tuple(self, fixlevels):
        last_build = None
        for (bversion, fix) in self.build_versions:
            for (i, v) in enumerate(fixlevels):
                fixlevels[i] = v - fix[i]
            last_build = bversion

        vtuple = [0]        
        if last_build is not None:
            vtuple = last_build
        if fixlevels != [0, 0, 0]: ## This is a little hackish... TBM
            vtuple += fixlevels
            
        return vtuple

