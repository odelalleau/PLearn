#!/usr/bin/env python

import sys
from plearn.vmat.PMat import load_pmat_as_array
from plearn.plotting.netplot import showRowsAsImages

####################
### main program ###

if __name__ == "__main__":

    try:
        datapmatfile, imgheight, imgwidth = sys.argv[1:]
        imgheight = int(imgheight)
        imgwidth = int(imgwidth)
    except:
        print "Usage: "+sys.argv[0]+" <datafile.pmat> <imgheight> <imgwidth>"
        print """
        Will load a pmat in memory and consider the beginning of each row a imgheight x imgwidth imagette.
        It will interactively display those.
        """
        raise
    # sys.exit()

    data = load_pmat_as_array(datapmatfile)
    inputs = data[:,0:imgheight*imgwidth]
    showRowsAsImages(inputs, figtitle=datapmatfile, img_width=imgwidth)



