import os
import os.path

# static patterns useful for parsing the file
include_regexp = re.compile(r'^\s*#include\s+(?:"|<)([^\s\"\>]+)(?:"|>)',re.M)

homedir = os.environ['HOME']
plearndir = os.environ.get('PLEARNDIR', os.path.join(homedir,'PLearn'))
lisaplearndir = os.environ.get('LISAPLEARNDIR', os.path.join(homedir,'LisaPLearn'))
apstatsoftdir = os.environ.get('APSTATSOFTDIR', os.path.join(homedir,'apstatsoft'))

searchdirs = [ os.path.join(plearndir,'/commands/PLearnCommands'), plearndir, apstatsoftdir, lisaplearndir ]

# Will map raw .h files to more complete paths
includepaths = {}

def visit(arg, dirname, names):
    global includepaths
    basename = dirname[len(arg):]
    #  print 'arg = ', arg
    # print 'dirname = ', dirname
    # print 'basename = ', basename
    if len(basename)>0 and basename[0]=='/':
        basename = basename[1:]
    for filename in names:
        if os.path.splitext(filename)[1]=='.h' and filename not in includepaths.keys():
            includepaths[filename] = os.path.join(basename,filename)

print "Building mapping..."

for searchdir in searchdirs:
    os.path.walk(searchdir, visit, searchdir)

print "Mapping built."

def transformfunc(fname, text):
    res = ''
    chunkstart = 0
    incend = 0
    incstart = text.find('#include', incend)
    while incstart >= 0:
        incstart = incstart+8
        while text[incstart] in [' ', '\t', '<', '"' ]:
            incstart = incstart+1
        incend = incstart
        while text[incend] not in ['>', '"']:
            incend = incend+1
        filename = text[incstart:incend]
        if filename in includepaths.keys():
            res = res + text[chunkstart:incstart] + includepaths[filename] 
            chunkstart = incend
        incstart = text.find('#include', incend)
    res = res + text[chunkstart:]
    return res

