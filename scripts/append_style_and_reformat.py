#!/usr/bin/env python

import os, sys

style_trailer = \
"""

/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
"""

if len(sys.argv[1:]) == 0:
    print >>sys.stderr, "Usage:", sys.argv[0],"file1 file2 ..."
    sys.exit(1)

for fname in sys.argv[1:]:
    print >> sys.stderr, "Processing",fname
    f = open(fname, "rU")
    lines = f.readlines()
    f.close()

    ## From the end, start by finding the last ^L (page break)
    lastpage = 0
    for i in range(len(lines))[::-1] :
        if lines[i].strip() == "\x0c":
            lastpage = i
            break
    
    ## From lines, find "Local Variables" within the last 15 lines of the
    ## file, and delete from thereon starting from start of comment
    if lastpage == 0:
        lastpage = len(lines)-15
        
    stripto = -1
    for i in range(lastpage, len(lines))[::-1] :
        if lines[i].lower().find("local variables") >= 0 :
            if lines[i].strip()[0:2] == "//" :
                stripto = i
            elif i > 0 and lines[i-1].strip()[0:2] == "/*" :
                stripto = i-1
            break

    if stripto >= 0:
        lines = lines[0:stripto]

    ## Strip all empty lines or page breaks at the end of the file
    for i in range(len(lines))[::-1]:
        curline = lines[i].strip()
        if not (curline == "" or curline == "\x0c") :
            break
    lines = lines[0:i+1]

    ## Append our famed local variables
    newfile = "".join(lines) + style_trailer

    f = open(fname, "w")
    f.write(newfile)
    f.close()

    ## Call emacs to indent the file
    command_line = ('emacs --no-site-file --no-init-file -batch %s ' +
                    '--eval="(indent-region (point-min) (point-max) nil)" ' +
                    '-f save-buffer') % \
                    fname
    os.system(command_line)


### Local Variables:
### mode:python
