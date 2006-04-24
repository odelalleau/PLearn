#  file_browser.py
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

import os, stat, sys, time

import pygtk
pygtk.require('2.0')
import gtk
import pango


#####  FileBrowser  #########################################################

class FileBrowser( object ):
    """Holds a gtk widget that acts as a file browser.  It must be
    initialized with a root directory.
    """
    def __init__( self, root_dir, filter_out_extensions = [] ):
        self.root_dir  = root_dir
        self.filter_out_ext = filter_out_extensions

        ## The store will contain:
        ## 0. filename (str)
        ## 1. whether it's a directory (bool)
        ## 2. file size (int)
        ## 3. last modified (str)
        column_mapping = [ 0, 2, 3 ]    # From user columns to model columns
        self.file_structure = gtk.TreeStore(str, bool, int, str)
        self.populate_tree(root_dir, self.file_structure.get_iter_root())

        ## Create the treeview and link it to the model
        self.w_treeview = gtk.TreeView(self.file_structure)

        ## Create the columns to view the contents
        self.columns = [ gtk.TreeViewColumn(title)
                         for title in [ 'Filename', 'Size', 'Last Modified' ] ]
        self.w_cell = gtk.CellRendererText()
        self.w_cell.set_property("xalign",0)
        for i,column in enumerate(self.columns):
            self.w_treeview.append_column(column)
            column.set_property("resizable", True)
            column.pack_end(self.w_cell, True)
            column.add_attribute(self.w_cell, 'text', column_mapping[i])

        ## Create a cell-renderer that displays a little directory or
        ## document icon depending on the file type
        self.w_cellpix = gtk.CellRendererPixbuf()
        self.w_cellpix.set_property("xpad", 8)
        self.w_cellpix.set_property("xalign", 0)
        def pix_format_func(treeviewcolumn, cell, model, iter):
            if model.get(iter,1)[0]:
                cell.set_property("stock-id", gtk.STOCK_DIRECTORY)
            else:
                cell.set_property("stock-id", gtk.STOCK_FILE)
            cell.set_property("stock-size", gtk.ICON_SIZE_BUTTON)
        self.columns[0].pack_start(self.w_cellpix)
        self.columns[0].set_cell_data_func(self.w_cellpix, pix_format_func)

        ## Create the scrolled window
        self.w_scrolled_window = gtk.ScrolledWindow()
        self.w_scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.w_scrolled_window.add(self.w_treeview)
        self.w_scrolled_window.show_all()

    def get_widget( self ):
        return self.w_scrolled_window

    def get_pathname_from_iter( self, treeiter ):
        """Return a filesystem pathname from a tree in the path.  This
        involves looking up the filenames at each step and joining them.
        """
        m  = self.file_structure
        if treeiter:                    # Not at root
            treepath  = m.get_path(treeiter)
            # print >>sys.stderr, "treepath =",treepath
            filenames = [ ]
            while treeiter is not None:
                filenames.append(m.get_value(treeiter, 0))
                treeiter = m.iter_parent(treeiter)
            filenames.reverse()
        else:
            filenames = [ ]
        return os.path.join(*[self.root_dir]+filenames)

    def populate_tree( self, dir, treeparent, visit_subdirectories = True ):
        """Given a location in the tree (given by treeparent), fill out the
        file information.  Repopulation (i.e. calling a second time to
        update the tree) is not very well handled right now.
        """
        m = self.file_structure
        # print >>sys.stderr, "Populating", dir
        files = [ f for f in os.listdir(dir)
                      if f[0] != '.' and
                         os.path.splitext(f)[1] not in self.filter_out_ext ]
        files.sort()

        ## Stat each file and construct the row
        for f in files:
            row = [ f, False, None, None ]
            fname = os.path.join(dir, f)
            try:
                filestat = os.stat(fname)
                row[1] = stat.S_ISDIR(filestat.st_mode)
                row[2] = filestat.st_size
                row[3] = time.ctime(filestat.st_mtime)
            except OSError:
                pass
            
            # print >>sys.stderr, "Appending",row
            m.append(treeparent, row)

        ## Populate subdirectories if required
        if visit_subdirectories and len(files) > 0:
            n = m.iter_n_children(treeparent)
            for i in range(n):
                child_iter = m.iter_nth_child(treeparent,i)
                if m.get_value(child_iter,1): # It's a subdirectory
                    self.populate_tree(os.path.join(dir,files[i]),
                                       child_iter, visit_subdirectories)

    def set_double_click_callback( self, func ):
        """The callback is called with the following arguments:
        - filebrowser object
        - pathname of the file clicked (not necessarily absolute)
        - whether the file is a directory (bool
        """
        def treeview_callback(treeview, path, view_column):
            file_iter = treeview.get_model().get_iter(path)
            isdir = self.file_structure.get_value(file_iter, 1)
            pathname = self.get_pathname_from_iter(file_iter)
            # print >>sys.stderr, "Mapped treeview path",path,"to pathname",pathname
            # print >>sys.stderr, "isdir =",isdir
            func(self, pathname, isdir)
        
        self.w_treeview.connect("row-activated", treeview_callback)


#####  Standalone Running  ##################################################

if __name__ == "__main__":

    def my_callback(fb,pathname,isdir):
        print >>sys.stderr, "My callback reached:",pathname,isdir
    
    fb = FileBrowser(".")
    fb.set_double_click_callback(my_callback)

    window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    window.set_size_request(600,400)
    window.connect("delete_event", lambda w,e: gtk.main_quit())
    window.add(fb.get_widget())
    window.show()
    
    gtk.main()
