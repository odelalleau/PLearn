
import copy
from toolkit import *
from pickle import *

def tmp_vprint(s):
    print s

forbidden_flag = "$#&*forbidden_directories*&#$"
def path_resolve_dir(resolve_dico , dirname, dirlist):
    forbidden_directories = []
    if resolve_dico.has_key(forbidden_flag):
        forbidden_directories = resolve_dico[forbidden_flag]
    remove_forbidden_dirs(dirlist, forbidden_directories)

    for fname in dirlist:
        fname = os.path.join(dirname, fname)
        path_resolve_file(fname, resolve_dico)
                        
def path_resolve_file(fname, resolve_dico):
    if os.path.isfile(fname):
        fcontent = command_output("cat %s"%fname)

        to_resolve = resolve_dico.keys()
        resolve_sort = lambda r, s: len(r)-len(s)
        to_resolve.sort( resolve_sort )
        
        resolved = open(fname, 'w')
        for line in fcontent:
            for to_res in to_resolve:
                if to_res != forbidden_flag:                    
                    line = string.replace(line, to_res, resolve_dico[to_res])
            resolved.write(line)
        resolved.close()
    
    
class IntelligentDiff:

    PREPROCESSING = {'.psave':path_resolve_file}

    def __init__(self, bench, other,
                 forbidden_directories =[], 
                 vprint                = tmp_vprint   ):
        self.__diffs = []
        self.vprint = vprint
        self.forbidden = forbidden_directories

        self.diff(bench, other)

    def __dirlist(self, dirc):
        dirlist = os.listdir(dirc)
        for fb in self.forbidden:
            if fb in dirlist:
                dirlist.remove(fb)
        return dirlist

    def __dirs(self, bench, other):
        if os.path.isdir(other):
            if os.path.isdir(bench):
                self.diff_directories(bench, other)
                return True
            else:
                self.__diffs.append("%s is a directory while %s is not."
                                  % (other, bench) )
                return True
        elif os.path.isdir(bench):
            self.__diffs.append("%s is a directory while %s is not."
                              % (bench, other) )
            return True
        return False        

    def __links(self, bench, other):
        if os.path.islink(other):
            self.vprint("%s is link: diff will be skipped."%other)
            return True
        elif os.path.islink(bench):
            self.__diffs.append( "%s is a link while %s is not."
                               % (bench, other) )
            return True
        return False

    def __files(self, bench, other):
        if os.path.isfile(other):
            if os.path.isfile(bench):
                self.diff_files(bench, other)
                return True
            else:
                self.__diffs.append("%s is a file while %s is not."
                                  % (other, bench) )
                return True
        elif os.path.isfile(bench):
            self.__diffs.append("%s is a file while %s is not."
                              % (bench, other) )
            return True
        return False        
    
    def diff(self, bench, other):
        if not self.__links(bench, other):
            if not self.__dirs(bench, other):
                if not self.__files(bench, other):
                    raise RuntimeError( "%s and %s are not links, nor directories, nor files."
                                        % (bench, other) )

    def diff_directories(self, bench, other):
        other_list = self.__dirlist(other)
        for of_name in other_list:
            bf = os.path.join(bench, of_name)
            of = os.path.join(other, of_name)
            self.diff(bf, of)

    def diff_files(self, bench, other):
        bench_dir = os.path.dirname(bench)
        other_dir = os.path.dirname(other)
        
        ## self.preprocess(other, bench_dir, other_dir)
        some_diff = command_output('diff %s %s' % (bench, other))
        if some_diff:
            self.__diffs.append( "\n+++ While comparing %s and %s:\n\t%s\n"
                               % (bench, other, string.join(some_diff, "\t"))    )

    def get_differences(self):
        return copy.deepcopy(self.__diffs)
    
##     def preprocess(self, fname, bench, other):
##         keys = self.PREPROCESSING.keys()
##         for key in keys:
##             if string.find(fname, key) != -1:
##                 preproc = self.PREPROCESSING[key]
##                 preproc(fname, {other:bench})                
##                 return


