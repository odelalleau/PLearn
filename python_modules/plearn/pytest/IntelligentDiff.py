
import copy, os, string

from   programs                      import PyTestUsageError

import plearn.utilities.plpath       as     plpath
import plearn.utilities.toolkit      as     toolkit
from   plearn.utilities.verbosity    import *

import plearn.utilities.versionning  as     versionning
versionning.project_module( "PyTest", __name__,
                            "$Id: IntelligentDiff.py,v 1.8 2004/12/20 21:04:57 dorionc Exp $"
                            )

class Resources:
    md5_mappings    = {}
    name_resolution = {}

    def memorize(cls, abspath, fname):
        if not cls.name_resolution.has_key(abspath):
            cls.name_resolution[abspath] = "$RESOURCES{%s}"%fname        
    memorize = classmethod(memorize)                                

    def single_link(cls, path_to, resource, target_dir, must_exist=True):
        ## Paths to the resource and target files
        resource_path = resource
        target_path   = target_dir

        ## Absolute versions
        if not os.path.isabs( resource_path ):
            resource_path = os.path.join( path_to, resource )
            target_path = os.path.join( path_to, target_dir, resource )
            assert not os.path.exists( target_path ), target_path
        
        ## Linking
        if os.path.exists( resource_path ):
            link_cmd = "ln -s %s %s" % ( resource_path, target_path )
            vprint( "Linking resource: %s." % link_cmd, 3 )
            os.system( link_cmd )

        elif must_exist:
            raise PyTestUsageError(
                "In %s: %s used as a resource but path doesn't exist."
                % ( os.getcwd(), resource )
                )

        ## Mapping both to the same variable
        cls.memorize( resource_path, resource )                
        cls.memorize( target_path, resource )                

        return (resource_path, target_path)        

    single_link = classmethod(single_link)
    
    ## Class methods
    def link_resources(cls, path_to, resources, target_dir): 
        for resource in resources:
            cls.single_link( path_to, resource, target_dir )
                        
            if toolkit.isvmat( resource ):
                cls.single_link( path_to, resource+'.metadata',
                                  target_dir,  False  )
                
    link_resources = classmethod(link_resources)

    def md5sum(cls, path_to_ressource):
        if md5_mappings.has_keys(path_to_ressource):
            return md5_mappings[path_to_ressource]
        
        md5 = toolkit.command_output( 'md5sum %s'
                                      % path_to_ressource)
        md5_mappings[path_to_ressource] = md5
        return md5
    md5sum = classmethod(md5sum)

    def unlink_resources(cls, target_dir):
        dirlist = os.listdir( target_dir )
        for f in dirlist:
            path = os.path.join( target_dir, f )
            if os.path.islink( path ):
                vprint( "Removing link: %s." % path, 3 ) 
                os.remove( path )
    unlink_resources = classmethod(unlink_resources)
        

class IntelligentDiff:    
    
    def __init__(self, compare_program, comparable_psaves):
        self.differences       = []
        self.compare_program   = compare_program
        self.comparable_psaves = comparable_psaves

    def are_directories(self, bench, other):
        bench_is = os.path.isdir(bench)
        other_is = os.path.isdir(other)
        
        if bench_is and other_is:                
            return True

        ## At least one is not 
        if bench_is:
            self.differences.append(
                "%s is a directory while %s is not." % (bench, other)
                )
            
        elif other_is:
            if toolkit.is_recursively_empty(other):
                vprint("Empty directory %s was skipped." % other, 2)
            else:
                self.differences.append(
                    "%s is a directory while %s is not." % (other, bench) )
                
        else: ## Both are not
            return False        

        return True

    def are_links(self, bench, other):
        if os.path.islink(other):
            vprint("%s is link: diff will be skipped."%other, 2)
        elif os.path.islink(bench):
            self.differences.append(
                "%s is a link while %s is not." % (bench, other) )
        else:
            return False
        return True

    def are_files(self, bench, other):        
        bench_is = os.path.isfile(bench)
        other_is = os.path.isfile(other)
        
        if bench_is and other_is:                
            return True
        
        ## At least one is not 
        if bench_is:
            self.differences.append(
                "%s is a file while %s is not." % (bench, other) )

        elif other_is:
            self.differences.append(
                "%s is a file while %s is not." % (other, bench) )

        else: ## Both are not
            return False

        return True
    
    def diff(self, bench, other):
        if self.are_links(bench, other):
            vprint("%s is a link: diff will be skipped."%other, 3)
        elif self.are_directories(bench, other):
            self.diff_directories(bench, other)
        elif self.are_files(bench, other):
            self.diff_files(bench, other)
        else:
            raise RuntimeError(
                "%s and %s are not links, nor directories, nor files." % (bench, other)
                )
        
        return self.differences

    def diff_directories(self, bench, other):
        other_list = os.listdir( other )
        toolkit.exempt_list_of( other_list,
                                plpath.special_directories )
        
        for of_name in other_list:
            bf = os.path.join(bench, of_name)
            of = os.path.join(other, of_name)
            self.diff(bf, of)

    def diff_files(self, bench, other):
        if bench.endswith('.psave'):
            self.diff_psave_files(bench, other)
            return
        
        bench_dir = os.path.dirname(bench)
        other_dir = os.path.dirname(other)
        
        ## self.preprocess(other, bench_dir, other_dir)
        some_diff = toolkit.command_output('diff -u %s %s' % (bench, other))
        if some_diff:
            self.differences.extend( some_diff )
            self.differences.append( "" )

    def diff_psave_files(self, bench, other):
        """Special manipulation of psave files.
        
        The psave files are meant to change over time which additions of
        new options to the various class of the library. To avoid a diff to
        failed on the addition of a new option, the psave files are
        canonized through the read_and_write command prior to the diff
        call.
        """
        directory_when_called = os.getcwd()
        (_b_, basename) = os.path.split(bench)

        if basename in self.comparable_psaves:
            ## Creating a temporary directory
            tmp_dir    = "%s.idiff_%s"\
                         % (other, toolkit.date_time_random_string())
            os.mkdir( tmp_dir )
            os.chdir( tmp_dir )

            ## Creating the canonized files
            bench_path = os.path.join(directory_when_called, bench)
            bench_rw   = basename+".expected_rw"

            other_path = os.path.join(directory_when_called, other)
            other_rw   = basename+".run_rw"

            cmd = "%s --no-version read_and_write %s %s"
            os.system( cmd%(self.compare_program, bench_path, bench_rw) )
            os.system( cmd%(self.compare_program, other_path, other_rw) )

            ## Actual comparison
            self.diff_files(bench_rw, other_rw)
            os.chdir( directory_when_called )
        
        else:
            vprint("Serialization file %s will be skipped."%bench, 3)
