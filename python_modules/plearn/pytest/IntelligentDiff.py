__version_id__ = "$Id: IntelligentDiff.py 4080 2005-09-13 13:49:47Z tihocan $"

import copy, os, shutil, string, sys

from   programs                      import PyTestError

import plearn.utilities.ppath        as     ppath
import plearn.utilities.toolkit      as     toolkit
from   plearn.utilities.verbosity    import vprint

class Resources:
    md5_mappings    = {}
    name_resolution = {}

    def memorize(cls, abspath, fname):
        if not cls.name_resolution.has_key(abspath):
            cls.name_resolution[abspath] = fname        
    memorize = classmethod(memorize)                                

    def single_link(cls, path_to, resource, target_dir, must_exist=True):
      
        ## Under Cygwin, links are not appropriate as they are ".lnk" files,
        ## not properly opened by PLearn. Thus we need to copy the files.
        def system_symlink( resource, target ):
            if (sys.platform == "cygwin"):
                if (os.path.isdir(resource)):
                    vprint( "Recursively copying resource: %s <- %s." \
                            % ( target, resource ), 3 )
                    shutil.copytree( resource, target, symlinks = False )
                else:
                    vprint( "Copying resource: %s <- %s." \
                            % ( target, resource ), 3 )
                    shutil.copy( resource, target )

            else:
                vprint( "Linking resource: %s -> %s." % ( target, resource ), 3 )
                os.symlink( resource, target )

        ## Paths to the resource and target files
        resource_path = resource
        target_path   = target_dir

        ## Absolute versions
        if not os.path.isabs( resource_path ):
            resource_path = os.path.join( path_to, resource )
            target_path = os.path.join( path_to, target_dir, resource )
        else:
            target_path = os.path.join(target_dir, os.path.basename(resource))
        assert not os.path.exists( target_path ), target_path
        
        ## Linking
        if os.path.exists( resource_path ):
            ## Linking
            system_symlink( resource_path, target_path )

        elif must_exist:
            raise PyTestError(
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

    def unlink_resources(cls, resources, target_dir):
        for resource in resources:
            path = os.path.join(target_dir, os.path.basename(resource))
            if os.path.islink( path ):
                vprint( "Removing link: %s." % path, 3 ) 
                os.remove( path )
            elif os.path.isfile( path ):
                vprint( "Removing file: %s." % path, 3 )
                os.remove( path )
            elif os.path.isdir( path ):
                vprint( "Removing directory: %s." % path, 3 )
                os.remove( path )

    unlink_resources = classmethod(unlink_resources)

class IntelligentDiff:    
    
    def __init__(self, test): 
        self.differences = []
        self.test        = test

    def are_directories(self, bench, other):
        bench_is = os.path.isdir(bench)
        other_is = os.path.isdir(other)
        
        if bench_is and other_is:                
            return True

        ## At least one is not 
        if bench_is:
            self.differences.append(
                "%s is a directory while %s is not.\n" % (bench, other)
                )
            
        elif other_is:
            if toolkit.is_recursively_empty(other):
                vprint("Empty directory %s was skipped." % other, 2)
            else:
                self.differences.append(
                    "%s is a directory while %s is not.\n" % (other, bench) )
                
        else: ## Both are not
            return False        

        return True

    def are_links(self, bench, other):
        if os.path.islink(other):
            vprint("%s is link: diff will be skipped."%other, 2)
        elif os.path.islink(bench):
            self.differences.append(
                "%s is a link while %s is not.\n" % (bench, other) )
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
                "%s is a file while %s is not.\n" % (bench, other) )

        elif other_is:
            self.differences.append(
                "%s is a file while %s is not.\n" % (other, bench) )

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

        vprint( ''.join(self.differences), priority=3 )
        return self.differences

    def diff_directories(self, bench, other):
        if toolkit.isvmat( bench ):
            ### Ex: a '.dmat' directory represents a PLearn DiskVMatrix
            self.diff_files(bench, other)
            return

        other_list = os.listdir( other )
        toolkit.exempt_list_of( other_list,
                                ppath.special_directories )
        
        for of_name in other_list:
            if of_name.endswith('.metadata'):
                continue
            
            bf = os.path.join(bench, of_name)
            of = os.path.join(other, of_name)
            self.diff(bf, of)

    def diff_files(self, bench, other, diff_template = 'toldiff %s %s'): #'diff -u %s %s'):
        if bench.endswith( 'metainfos.txt' ):
            vprint('Skipping metainfos.txt comparison', 2)
            return
            
        if bench.endswith('.psave'):
            self.diff_psave_files(bench, other)
            return

        if toolkit.isvmat( bench ):
            diff_template = 'plearn_tests --no-version vmat diff %s %s ' \
                            + str(self.test.precision)

        if bench.endswith('_rw'):
            diff_template = 'plearn_tests --no-version diff %s %s ' \
                            + str(self.test.precision)
        
        bench_dir = os.path.dirname(bench)
        other_dir = os.path.dirname(other)
        
        ## self.preprocess(other, bench_dir, other_dir)
        some_diff = toolkit.command_output(diff_template % (bench, other))
        if some_diff:
            self.differences.append("%s and %s differ:\n" % (bench,other) )
            self.differences.extend( some_diff )
            self.differences.append( "" )

    def diff_psave_files(self, bench, other):
        """Special manipulation of psave files.
        
        The psave files are meant to change over time which additions of
        new options to the various class of the library. To avoid a diff to
        fail on the addition of a new option, the psave files are canonized
        through the read_and_write command prior to the diff call.
        """
        directory_when_called = os.getcwd()
        (_b_, basename) = os.path.split(bench)

        ## The read_and_write command must be called within the appropriate
        ## 'plearn' like program
        compare_program = self.test.program.name
        
        ## Creating a temporary directory
        tmp_dir    = "%s.idiff_%s"\
                     % (other, toolkit.date_time_random_string('_', '_', '-'))
        os.mkdir( tmp_dir )
        self.test.link_resources( tmp_dir )

        ## Creating the canonized files
        os.chdir( tmp_dir )
        bench_path = os.path.join(directory_when_called, bench)
        bench_rw   = basename+".expected_rw"

        other_path = os.path.join(directory_when_called, other)
        other_rw   = basename+".run_rw"

        cmd = "%s --no-version read_and_write %s %s"
        os.system( cmd%(compare_program, bench_path, bench_rw) )
        os.system( cmd%(compare_program, other_path, other_rw) )

        ## Actual comparison
        self.diff_files(bench_rw, other_rw)
        os.chdir( directory_when_called )
        
##         else:
##             vprint("Serialization file %s will be skipped."%bench, 3)
