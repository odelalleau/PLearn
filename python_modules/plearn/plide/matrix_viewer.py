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
import pango


#####  get_matrix_viewer  ###################################################

class MatrixViewer( object ):
    """Holds a gtk widget that acts as a matrix viewer, given a matrix model
    (such as PMatListModel).  We assume that the model supports a method
    'get_column_names()'.
    """
    def __init__( self, matrix_model ):
        self.matrix_model = matrix_model

        ## Create individual columns (first column is the row number)
        column_names = matrix_model.get_column_names()
        self.columns = [ gtk.TreeViewColumn(x) for x in [''] + column_names ]

        ## Create cell renderers for displaying the contents
        self.cell_rownumber = gtk.CellRendererText()
        self.cell_rownumber.set_property("xalign", 1.0)
        self.cell_rownumber.set_property("xpad",   2)
        self.cell_rownumber.set_property("weight", pango.WEIGHT_BOLD)


        ## Connect the columns to the cell renderers (header)
        def rownumber_func(treeviewcolumn, cell, model, iter):
            cell.set_property('text', str(model.get_path(iter)[0]))
        self.columns[0].pack_start(self.cell_rownumber, True)
        self.columns[0].set_cell_data_func(self.cell_rownumber, rownumber_func)
        self.columns[0].set_property("sizing", gtk.TREE_VIEW_COLUMN_FIXED)
        self.columns[0].set_property("fixed-width", 65)

        ## Connect the remaining columns to their cell renderer
        #def formatting_func(treeviewcolumn, cell, model, iter):
        #    cell.set_property('text', str(model.
        def float_format_func(treeviewcolumn, cell, model, iter, i):
            datum = model.get(iter,i)[0]
            cell.set_property("text", "%6.2f" % datum)
        for i, column in enumerate(self.columns[1:]):
            cell_datum = gtk.CellRendererText()
            cell_datum.set_property("xalign", 1.0)
            cell_datum.set_property("xpad",   0)
            column.pack_start(cell_datum, True)
            column.set_property("sizing", gtk.TREE_VIEW_COLUMN_FIXED)
            column.set_property("fixed-width", 85)
            column.set_property("resizable", True)
            column.set_cell_data_func(cell_datum, float_format_func, i)

        ## Make overall treeview
        self.treeview = gtk.TreeView( matrix_model )
        for column in self.columns:
            self.treeview.append_column(column)
        self.treeview.set_rules_hint(True)
        self.treeview.set_property("enable-search", False)
        self.treeview.set_property("fixed-height-mode", True)

        ## Create the scrolled window
        self.scrolled_window = gtk.ScrolledWindow()
        self.scrolled_window.add(self.treeview)
        self.scrolled_window.show_all()

    def get_widget( self ):
        return self.scrolled_window


#####  Standalone Running  ##################################################

if __name__ == "__main__":
    from pmat_list_model import PMatListModel
    from plearn.vmat.PMat import PMat

    pmat = PMatListModel(PMat("/projects/finance/Desjardins/term_structure/Cotton.pmat",
                              openmode = 'r'))
    viewer = MatrixViewer(pmat)

    window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    window.set_size_request(600,400)
    window.connect("delete_event", lambda w,e: gtk.main_quit())
    window.add(viewer.get_widget())
    window.show()

    gtk.main()
