"""PLEASE DO NOT EDIT, THIS MODULE WILL SOON BE DEPRECATED!!!

Contact dorionc@apstat.com for details.
"""
import copy, logging, os, shutil, string, sys

# PyTest Modules
import core

from plearn.utilities import ppath 
from plearn.utilities import moresh 
from plearn.utilities import toolkit

class Resources:
    md5_mappings    = {}
    name_resolution = {}

    def memorize(cls, abspath, fname):
        if not cls.name_resolution.has_key(abspath):
            cls.name_resolution[abspath] = fname        
    memorize = classmethod(memorize)                                

    ## Create a link from target to resource.
    def single_link(cls, path_to, resource, target_dir, must_exist=True):
        ## Under Cygwin, links are not appropriate as they are ".lnk" files,
        ## not properly opened by PLearn. Thus we need to copy the files.
        def system_symlink( resource, target ):
            if (sys.platform == "cygwin"):
                if (os.path.isdir(resource)):
                    logging.debug(
                        "Recursively copying resource: %s <- %s."%(target, resource))
                    shutil.copytree( resource, target, symlinks = False )
                else:
                    logging.debug("Copying resource: %s <- %s."%(target, resource))
                    shutil.copy( resource, target )
            else:
                logging.debug("Linking resource: %s -> %s."%(target,resource))
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
        linked = False
        if os.path.exists( resource_path ):
            system_symlink( resource_path, target_path )
            linked = True

        elif must_exist:
            raise core.PyTestUsageError(
                "In %s: %s used as a resource but path does not exist."
                % ( os.getcwd(), resource )
                )

        ## Mapping both to the same variable
        cls.memorize( resource_path, resource )                
        cls.memorize( target_path, resource )                

        if linked:
            return (resource_path, target_path)
        else:
            return ()

    single_link = classmethod(single_link)
    
    ## Class methods
    def link_resources(cls, path_to, resources, target_dir): 
        resources_to_append = []
        for resource in resources:
            cls.single_link( path_to, resource, target_dir )
                        
            if toolkit.isvmat( resource ):
                metadatadir = resource + '.metadata'
                if metadatadir not in resources:
                    link_result = cls.single_link( path_to, metadatadir,
                                                   target_dir,  False )
                    if link_result:
                        ## Link has been successfully performed: we must add the
                        ## metadata directory to the list of resources, so that it
                        ## is correctly unlinked at a later time.
                        resources_to_append.append(metadatadir)
        resources.extend(resources_to_append)

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
            res = os.path.basename(resource)
            path = os.path.join(target_dir, res)
            if os.path.islink( path ):
                logging.debug("Removing link: %s."%path)
                os.remove( path )
            elif os.path.isfile( path ):
                logging.debug("Removing file: %s."%path)
                os.remove( path )
            elif os.path.isdir( path ):
                logging.debug("Removing directory: %s." % path)
                shutil.rmtree( path )

    unlink_resources = classmethod(unlink_resources)
        

class IntelligentDiff:    
    
    def __init__(self, test): 
        self.differences = []
        self.test        = test

    def pfilecmd(self):
        self.test.pfileprg.compile()
        if self.test.compilationSucceeded():
            return "%s --no-version --verbosity VLEVEL_IMP"%self.test.pfileprg.name

        return "echo COMPILATION ERROR: %s"%self.test.pfileprg.name

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
            if moresh.is_recursively_empty(other):
                logging.debug("Empty directory %s was skipped."%other)
            else:
                self.differences.append(
                    "%s is a directory while %s is not.\n" % (other, bench) )
                
        else: ## Both are not
            return False        

        return True

    def are_links(self, bench, other):
        if os.path.islink(other):
            logging.debug("%s is link: diff will be skipped."%other)
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
            logging.debug("%s is a link: diff will be skipped."%other)
        elif self.are_directories(bench, other):
            self.diff_directories(bench, other)
        elif self.are_files(bench, other):
            self.diff_files(bench, other)
        else:
            raise RuntimeError(
                "%s and %s are not links, nor directories, nor files." % (bench, other)
                )

        logging.debug(''.join(self.differences))
        return self.differences

    def diff_directories(self, bench, other):
        if toolkit.isvmat( bench ):
            ### Ex: a '.dmat' directory represents a PLearn DiskVMatrix.
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

    def diff_files(self, bench, other, diff_template = 'toldiff %s %s %s'):
        if bench.endswith( 'metainfos.txt' ):
            logging.debug('Skipping metainfos.txt comparison')
            return
            
        if bench.endswith('.psave'):
            self.diff_psave_files(bench, other)
            return

        if toolkit.isvmat( bench ):
            diff_template =\
                self.pfilecmd() + ' vmat diff %s %s %s'

        if bench.endswith('_rw'):
            diff_template =\
                self.pfilecmd() + ' diff %s %s %s'
        
        bench_dir = os.path.dirname(bench)
        other_dir = os.path.dirname(other)
        
        ## self.preprocess(other, bench_dir, other_dir)
        some_diff = toolkit.command_output(diff_template % (bench, other, self.test.precision))
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
        self.test.linkResources( tmp_dir )

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

        ## Clean linked resources.
        self.test.unlinkResources( tmp_dir )
        
        ## Move back to original directory.
        os.chdir( directory_when_called )
        
