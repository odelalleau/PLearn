#  pmat_list_model.py
#  Copyright (C) 2006 by Nicolas Chapados
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


import pygtk
pygtk.require('2.0')
import gtk
from plearn.vmat.PMat import PMat


#####  PMatListModel  #######################################################

class PMatListModel( gtk.GenericTreeModel ):
    def __init__(self, pmat):
        gtk.GenericTreeModel.__init__(self)
        self.pmat  = pmat
        self.array = pmat.getRows(0, pmat.length) # Read all data for performance

    def get_column_names( self ):
        return self.pmat.fieldnames

    def on_get_flags(self):
        return gtk.TREE_MODEL_LIST_ONLY     | \
               gtk.TREE_MODEL_ITERS_PERSIST

    def on_get_n_columns(self):
        return self.pmat.width

    def on_get_column_type(self, index):
        return float

    def on_get_iter(self, path):
        ## Row references are simple integers for a PMat
        return path[0]                  # [0] since we're passed a tuple

    def on_get_path(self, rowref):
        return (rowref,)

    def on_get_value(self, rowref, column):
        return self.array[rowref,column]

    def on_iter_next(self, rowref):
        if rowref >= 0 and rowref < self.pmat.length - 1:
            return rowref+1
        else:
            return None

    def on_iter_children(self, parent):
        if parent:
            return None                 # Never any children since it's a flat structure
        else:
            return 0                    # To start iteration with very first row

    def on_iter_has_child(self, rowref):
        return False

    def on_iter_n_children(self, rowref):
        if rowref:
            return 0
        else:
            return self.pmat.length

    def on_iter_nth_child(self, parent, n):
        if parent:
            return None
        elif n >= 0 and n < self.pmat.length:
            return n
        else:
            return None
        
    def on_iter_parent(self, child):
        return None


#####  Standalone Running  ##################################################

if __name__ == "__main__":
    pass
