# mpl_utilities.py
# Copyright (C) 2008 by Nicolas Chapados
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

# Author: Nicolas Chapados

"""
A set of utility functions for Matplotlib.
"""

import os


def showOrSave(filename=None, use_environment=True, dpi='__AUTO__', **kwargs):
    """Either show a matplotlib figure or save it to disk.

    This function is a replacement for Matplotlib's show() function.  It
    has the behavior or either calling Matplotlib's show function, or
    saving the figure to disk (without showing it).  Here is how it works:

    - If argument 'use_environment' is True, then the environment variable
      PL_SAVEFIG is considered.  If defined, the figure is always saved
      using the given filename (if provided); if not defined, the figure is
      shown to the screen.  Note that if PL_SAVEFIG is defined but the
      filename is NOT provided, then nothing happens (neither saved nor
      shown); this would occur when a valid location (expdir) cannot be
      found for saving the figure.

    - Otherwise, the argument 'filename' alone is considered.  If provided,
      the figure is saved; if not, the figure is shown.

    If the function has been able to succesfully save the figure or show
    it, it returns respectively 'saved' or 'shown'.  Otherwise it returns
    None.
    """
    from pylab import savefig, show
    if dpi == '__AUTO__':
        dpi = int(os.environ.get('PL_SAVEFIG_DPI', 72))
        
    if use_environment:
        if eval(os.environ.get('PL_SAVEFIG', "False")):
            if filename:
                try:
                    savefig(filename, dpi=dpi, **kwargs)
                    return 'saved'
                except Exception:
                    pass
            return None         # Could not save: no filename or exception raised
        else:
            show()
            return 'shown'

    else:
        if filename:
            try:
                savefig(filename, dpi=dpi, **kwargs)
                return 'saved'
            except Exception:
                return None
        else:
            show()
            return 'shown'
