#!/usr/bin/env python

# show_rows_as_images.py
# Copyright (C) 2008 Pascal Vincent
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

# Author: Pascal Vincent


from plearn.plotting.netplot import showRowsAsImages

def show_rows_as_images(matfile, imgheight, imgwidth, nrows=10, ncols=10, figtitle=""):
    """Will open a .pmat .dmat .amat or .vmat file and consider the beginning of each row a imgheight x imgwidth imagette.
    These images will be interactively displayed in a nrows x ncols grid of imagettes."""
    data = None
    if figtitle=="":
        figtitle = matfile 
    if matfile.endswith(".pmat"):
        # Use pure python implementation of pmat (faster loading)
        from plearn.vmat.PMat import PMat
        data = PMat(matfile)
    else:
        # Use of VMat through the Python-bridge
        from plearn.pyext import AutoVMatrix
        data = AutoVMatrix(filename=matfile)
    showRowsAsImages(data, img_height=imgheight, img_width=imgwidth, nrows=nrows, ncols=ncols, figtitle=figtitle)

####################
### main program ###

if __name__ == '__main__':
    from plearn.utilities.autoscript import autoscript
    autoscript(show_rows_as_images, True)

