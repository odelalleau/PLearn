
import copy, os, string
import plearn.utilities.plpath     as     plpath
import plearn.utilities.toolkit    as     toolkit
from   plearn.utilities.verbosity  import *

class IntelligentDiff:    
    
    def __init__(self):
        self.differences = []

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
            vprint("%s is link: diff will be skipped."%other)
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
            vprint("%s is a link: diff will be skipped."%other)
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
        bench_dir = os.path.dirname(bench)
        other_dir = os.path.dirname(other)
        
        ## self.preprocess(other, bench_dir, other_dir)
        some_diff = toolkit.command_output('diff %s %s' % (bench, other))
        if some_diff:
            self.differences.append( "\n+++ While comparing %s and %s:\n\t%s\n"
                               % (bench, other, string.join(some_diff, "\t"))    )
