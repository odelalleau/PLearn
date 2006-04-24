#  plide_tabs.py
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

import os.path
import sys

import pygtk
pygtk.require('2.0')
import gtk
import scintilla

from plearn.pl_pygtk  import MessageBox
from plearn.vmat.PMat import PMat
from file_browser     import FileBrowser
from matrix_viewer    import MatrixViewer
from pmat_list_model  import PMatListModel
from plide_utils      import *


#####  Base class for Tabs  #################################################

class PlideTab( object ):
    """Abstract tab that's added to the PlideMain window notebook.

    Derived classes implement viewers/editors for several data types.
    """

    TAB_TYPE_DOCUMENT  = 1              # Normal file (for setting icon type)
    TAB_TYPE_DIRECTORY = 2              # Directory icon

    def __init__(self, notebook):
        self.notebook       = notebook
        self.options_holder = None
        self.page_number    = None

    def set_options_holder(self, options_holder):
        """Provide the tab with an options_holder object.
        """
        self.options_holder = options_holder

    def get_options_holder(self):
        """Return the options_holder object for the tab.
        """
        return self.options_holder

    def get_tab_title_widget( self, title_str, tab_type ):
        """Return the widget that should go in the tab title, including a
        little 'x' button to close the document.  When this button is
        clicked, the tab's close_tab() method is called.
        """
        self.tab_type = tab_type
        tab_icon = gtk.Image()
        tab_icon.set_padding(5,0)
        if (tab_type == self.TAB_TYPE_DOCUMENT):
            tab_icon.set_from_stock(gtk.STOCK_FILE, gtk.ICON_SIZE_MENU)
        elif (tab_type == self.TAB_TYPE_DIRECTORY):
            tab_icon.set_from_stock(gtk.STOCK_DIRECTORY, gtk.ICON_SIZE_MENU)
            
        close_icon = gtk.Image()
        close_icon.set_from_stock(gtk.STOCK_CLOSE, gtk.ICON_SIZE_MENU)
        label = gtk.Label(title_str)
        close_button = gtk.Button("")
        close_button.set_relief(gtk.RELIEF_NONE)
        close_button.set_image(close_icon)
        close_button.connect('clicked', self.close_tab)
        
        hbox = gtk.HBox()
        hbox.pack_start(tab_icon)
        hbox.pack_start(label)
        hbox.pack_start(close_button)
        hbox.show_all()
        return hbox

    def change_tab_title( self, new_title_str ):
        """Change the string associated with the tab title.  Don't touch
        the other graphical elements.
        """
        new_label = self.get_tab_title_widget(new_title_str, self.tab_type)
        self.notebook.set_tab_label(self.notebook.get_nth_page(self.page_number),
                                    new_label)
    
    def append_to_notebook( self, page_widget, title_str, tab_type ):
        page_widget.plide_tab_object = self    # Add backpointer to self
        self.page_number = self.notebook.append_page(
            page_widget, self.get_tab_title_widget(title_str, tab_type))
        self.notebook.set_current_page(self.page_number)

    def close_tab( self, button ):
        """GTK callback that's called when the user wished to close the
        tab.  Should be overridden in derived classes if there must be some
        checking regarding document-modified versus should-save setting.
        """
        ## Renumber all tabs that come after the current one (their page
        ## number change)
        n = self.notebook.get_n_pages()
        for i in range(self.page_number+1, n):
            # print >>sys.stderr, self.notebook.get_nth_page(i).plide_tab_object.page_number, "-->", i-1
            self.notebook.get_nth_page(i).plide_tab_object.page_number = i-1
        
        self.notebook.remove_page(self.page_number)
        self.notebook.set_current_page(self.page_number)
        

######  Scintilla Editor Wrapper  ############################################

class PlideTabScintilla( PlideTab ):
    """Plide tab for scintilla documents.  Note: the textview is appended
    to the notebook, but not shown in this constructor.  This is to allow
    the derived classes to set further styling.
    """
    def __init__(self, notebook, contents, read_only = True):
        PlideTab.__init__(self, notebook)

        self.textview = self.scintilla_widget(contents, read_only)
        self.textview.Colourise(0,-1)
        self.textview.show()
        self.append_to_notebook(self.textview, self.basename, PlideTab.TAB_TYPE_DOCUMENT)

    def on_undo_activate( self ):
        self.textview.Undo()

    def on_redo_activate( self ):
        self.textview.Redo()

    def on_cut_activate( self ):
        self.textview.Cut()

    def on_copy_activate( self ):
        self.textview.Copy()

    def on_paste_activate( self ):
        self.textview.Paste()

    def scintilla_widget( contents, read_only ):
        """Static method that creates a new scintilla widget and sets up
        basic options.  DOES not show the widget, which must be done by the
        caller.
        """
        ## Create new Scintilla widget
        scin = scintilla.Scintilla()
        scin.SetText(contents)
        scin.SetReadOnly(read_only)
        scin.set_size_request(100,100)
        scin.StyleSetFont(scintilla.STYLE_DEFAULT, "!Monospace")
        scin.SetZoom(-2)

        ## Default configuration for Scintilla
        scin.SetUseTabs(False)
        scin.SetTabWidth(4)
        scin.SetIndent(4)
        scin.SetReadOnly(0)

        ## Put line numbers in margin 0.  Determine width in pixels
        ## required for displaying up to 99999 lines
        req_width = scin.TextWidth(scintilla.STYLE_LINENUMBER, "_99999")
        scin.SetMarginTypeN(1, scintilla.SC_MARGIN_NUMBER)
        scin.SetMarginWidthN(1, req_width)
        return scin
    
    scintilla_widget = staticmethod(scintilla_widget)


#####  PlideTabFile  ########################################################

class PlideTabFile( PlideTabScintilla ):
    """Plide tab for editing generic files (manages the is-new and modified
    flags).
    """
    def __init__(self, notebook, filename, is_new = False):
        self.filename = filename
        self.is_new   = is_new
        self.basename = os.path.basename(filename)
        self.dirname  = os.path.dirname(filename)

        try:
            f = open(filename)
            contents = f.read()
            f.close()
        except:
            contents = ""

        PlideTabScintilla.__init__(self, notebook, contents, read_only=False)
        self.textview.SetSavePoint()

    def get_script_directory( self ):
        return self.dirname

    def get_basename( self ):
        return self.basename

    def get_text( self ):
        # return self.buf.get_text(self.buf.get_start_iter(),
        #                          self.buf.get_end_iter())
        return self.textview.GetText()[1] # First element of pair is buffer size

    def on_save_activate( self ):
        """Depending on whether the file is new, save it right away or
        perform a 'save as' operation.
        """
        if self.is_new:
            return self.save_as_file()
        else:
            return self.save_file()

    def save_file( self ):
        f = open(self.filename, 'w')
        f.write(self.get_text())
        f.close()
        self.textview.SetSavePoint()    # Tell Scintilla that we just saved
        return gtk.RESPONSE_OK

    def save_as_file( self ):
        """Prompt the user for a new filename.  If given, make the new
        filename the default, set is_new to false, and update the tab
        label.  Return gtk.RESPONSE_OK if saved, or gtk.RESPONSE_CANCEL if
        not.
        """
        chooser = gtk.FileChooserDialog(
            title   = "Save as",
            action  = gtk.FILE_CHOOSER_ACTION_SAVE,
            buttons = (gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,
                       gtk.STOCK_SAVE,  gtk.RESPONSE_OK))
        chooser.set_default_response(gtk.RESPONSE_OK)
        response = chooser.run()

        ## If user clicked OK, update internal structures
        if response == gtk.RESPONSE_OK:
            filename = chooser.get_filename()

            ## Check if the file already exists and confirm
            confirm = gtk.RESPONSE_YES
            if os.path.exists(filename):
                confirm = MessageBox('File "%s" already exists.  Overwrite?' % filename,
                                     'If you click "yes", existing file will be permanently lost.',
                                     buttons = gtk.BUTTONS_YES_NO)
            if confirm == gtk.RESPONSE_YES:
                self.filename = filename
                self.is_new   = False
                self.basename = os.path.basename(self.filename)
                self.dirname  = os.path.dirname(self.filename)
                self.save_file()
                self.change_tab_title(self.basename)

        chooser.destroy()
        return response

    def close_tab( self, button ):
        """Override base-class version to prompt the user to save his
        changes if the buffer has been modified.
        """
        if self.textview.GetModify():
            def save_buttons(dialog):
                dialog.add_buttons("Close without saving", gtk.RESPONSE_CLOSE,
                                   gtk.STOCK_CANCEL,       gtk.RESPONSE_CANCEL,
                                   gtk.STOCK_SAVE,         gtk.RESPONSE_YES)
            
            response = MessageBox('Save changes to file "%s" before closing?'
                                  % self.get_basename(),
                                  "If you don't save, changes will be permanently lost.",
                                  title = "",
                                  custom_buttons = save_buttons,
                                  type = gtk.MESSAGE_WARNING)

            if response == gtk.RESPONSE_YES:
                ## Save intelligently depending on whether this is a new file.
                ## If we receive a gtk.RESPONSE_CANCEL, it means that the user
                ## freaked out and canceled saving.
                response = self.on_save_activate()

            if response == gtk.RESPONSE_CANCEL:
                return                  # Don't call inherited close

        ## Call inherited version to physically close the tab
        PlideTabScintilla.close_tab(self, button)
        
                
#####  Python Editor  #######################################################

class PlideTabPython( PlideTabFile ):
    """Plide tab for editing Python files (sets up folding and styling on
    Scintilla widget)
    """
    def __init__(self, notebook, filename, is_new = False):
        PlideTabFile.__init__(self, notebook, filename, is_new)

        ## Update the scintilla widget to reflect Python formatting
        self.set_scintilla_style_for_python(self.textview)
        self.textview.show()

    def set_scintilla_style_for_python(scin):
        """Update an existing scintilla widget for Python formatting.
        """
        ## Folding configuration
        scin.SetMarginTypeN(2, scintilla.SC_MARGIN_SYMBOL)
        scin.SetMarginWidthN(2, 12)
        scin.SetProperty("fold",                "1")
        scin.SetProperty("fold.comment",        "1")
        scin.SetProperty("fold.compact",        "1")
        scin.SetProperty("fold.comment.python", "1")
        scin.SetProperty("fold.quotes.python",  "1")
 	scin.SetFoldFlags(16)
        scin.MarkerDefine(scintilla.SC_MARKNUM_FOLDEROPEN, scintilla.SC_MARK_BOXMINUS)
	scin.MarkerSetFore(scintilla.SC_MARKNUM_FOLDEROPEN, 0xffffff)
	scin.MarkerSetBack(scintilla.SC_MARKNUM_FOLDEROPEN, 0x000000)

        scin.MarkerDefine(scintilla.SC_MARKNUM_FOLDER, scintilla.SC_MARK_BOXPLUS)
	scin.MarkerSetFore(scintilla.SC_MARKNUM_FOLDER, 0xffffff)
	scin.MarkerSetBack(scintilla.SC_MARKNUM_FOLDER, 0x000000)

        scin.MarkerDefine(scintilla.SC_MARKNUM_FOLDERSUB, scintilla.SC_MARK_VLINE)
	scin.MarkerSetFore(scintilla.SC_MARKNUM_FOLDERSUB, 0xffffff)
	scin.MarkerSetBack(scintilla.SC_MARKNUM_FOLDERSUB, 0x000000)

        scin.MarkerDefine(scintilla.SC_MARKNUM_FOLDERTAIL, scintilla.SC_MARK_LCORNER)
	scin.MarkerSetFore(scintilla.SC_MARKNUM_FOLDERTAIL, 0xffffff)
	scin.MarkerSetBack(scintilla.SC_MARKNUM_FOLDERTAIL, 0x000000)

        scin.MarkerDefine(scintilla.SC_MARKNUM_FOLDEREND, scintilla.SC_MARK_BOXPLUSCONNECTED)
	scin.MarkerSetFore(scintilla.SC_MARKNUM_FOLDEREND, 0xffffff)
	scin.MarkerSetBack(scintilla.SC_MARKNUM_FOLDEREND, 0x000000)

        scin.MarkerDefine(scintilla.SC_MARKNUM_FOLDEROPENMID, scintilla.SC_MARK_BOXMINUSCONNECTED)
	scin.MarkerSetFore(scintilla.SC_MARKNUM_FOLDEROPENMID, 0xffffff)
	scin.MarkerSetBack(scintilla.SC_MARKNUM_FOLDEROPENMID, 0x000000)

        scin.MarkerDefine(scintilla.SC_MARKNUM_FOLDERMIDTAIL, scintilla.SC_MARK_TCORNER)
	scin.MarkerSetFore(scintilla.SC_MARKNUM_FOLDERMIDTAIL, 0xffffff)
	scin.MarkerSetBack(scintilla.SC_MARKNUM_FOLDERMIDTAIL, 0x000000)

        ## Make the margin sensitive to mouse clicks and connect a callback
        ## to toggle folding
	scin.SetMarginSensitiveN(2, True)
	scin.SetMarginMaskN(2, scintilla.SC_MASK_FOLDERS)

        def on_margin_click(widget, modifiers, position, margin):
            #print >>sys.stderr, "on_margin_click / modifiers =",modifiers,\
            #      " position =",position," margin =",margin
            line = scin.LineFromPosition(position)
            if margin == 2:
                level = scin.GetFoldLevel(line)
                if level & scintilla.SC_FOLDLEVELHEADERFLAG:
                    #print >>sys.stderr, "toggling folding on line",line
                    scin.ToggleFold(line)

        scin.connect("MarginClick", on_margin_click)

        ## Scintilla formatting for Python
        scin.StyleClearAll()
        scin.SetLexer(scintilla.SCLEX_PYTHON)
        python_keywords = "and assert break class continue def del elif else "   +\
                          "except exec finally for from global if import in is " +\
                          "lambda not or pass print raise return try while yield"
        scin.SetKeyWords(0, python_keywords)
        scin.SetIndentationGuides(True)

        ## Python colors for each syntactic element
	scin.StyleSetFore(scintilla.SCE_P_DEFAULT, 0x000000);
	scin.StyleSetBack(scintilla.SCE_P_DEFAULT, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_COMMENTLINE, 0x808080);
	scin.StyleSetBack(scintilla.SCE_P_COMMENTLINE, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_NUMBER, 0x800040);
	scin.StyleSetBack(scintilla.SCE_P_NUMBER, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_STRING, 0x008000);
	scin.StyleSetBack(scintilla.SCE_P_STRING, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_CHARACTER, 0x008000);
	scin.StyleSetBack(scintilla.SCE_P_CHARACTER, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_WORD, 0x800060);
	scin.StyleSetBack(scintilla.SCE_P_WORD, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_TRIPLE, 0x008000);
	scin.StyleSetBack(scintilla.SCE_P_TRIPLE, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_TRIPLEDOUBLE, 0x008000);
	scin.StyleSetBack(scintilla.SCE_P_TRIPLEDOUBLE, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_CLASSNAME, 0x303000);
	scin.StyleSetBack(scintilla.SCE_P_CLASSNAME, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_DEFNAME, 0x800000);
	scin.StyleSetBack(scintilla.SCE_P_DEFNAME, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_OPERATOR, 0x800030);
	scin.StyleSetBack(scintilla.SCE_P_OPERATOR, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_IDENTIFIER, 0x000000);
	scin.StyleSetBack(scintilla.SCE_P_IDENTIFIER, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_COMMENTBLOCK, 0x808080);
	scin.StyleSetBack(scintilla.SCE_P_COMMENTBLOCK, 0xffffff);

	scin.StyleSetFore(scintilla.SCE_P_STRINGEOL, 0x000000);
	scin.StyleSetBack(scintilla.SCE_P_STRINGEOL, 0xffffff);

        scin.Colourise(0,-1)
        
    set_scintilla_style_for_python = staticmethod(set_scintilla_style_for_python)


#####  PyPLearn Editor  #####################################################

class PlideTabPyPLearn( PlideTabPython ):
    """Plide tab for PyPLearn scripts.
    """
    def __init__(self, notebook, filename, is_new = False):
        PlideTabPython.__init__(self, notebook, filename, is_new)


#####  PMat Viewer  #########################################################

class PlideTabPMat( PlideTab ):
    """Plide tab for viewing a PMat.
    """
    def __init__(self, notebook, pmat_filename ):
        PlideTab.__init__(self, notebook)
        pmat = PMatListModel(PMat(pmat_filename, openmode='r'))
        viewer = MatrixViewer(pmat).get_widget()
        title  = os.path.basename(pmat_filename)
        viewer.show_all()
        self.append_to_notebook(viewer,title,self.TAB_TYPE_DOCUMENT)


#####  Expdir Browser  ######################################################

class PlideTabExpdir( PlideTab ):
    """Plide tab for browsing expdirs.
    """
    def __init__(self, notebook, expdir):
        PlideTab.__init__(self, notebook)
        self.expdir = expdir

        self.w_frame_left = gtk.Frame()
        self.w_frame_left.set_property("shadow-type", gtk.SHADOW_IN)
        self.w_frame_right = gtk.Frame()
        self.w_frame_right.set_property("shadow-type", gtk.SHADOW_IN)

        self.browser = FileBrowser(expdir, [ ".metadata" ])
        self.browser.set_double_click_callback(self.double_click_callback)
        self.w_frame_left.add(self.browser.get_widget())
        
        self.w_pane  = gtk.HPaned()
        self.w_pane.pack1(self.w_frame_left)
        self.w_pane.pack2(self.w_frame_right)
        self.w_pane.set_property("position", 350)
        self.w_pane.set_property("position-set", True)

        self.w_pane.show_all()
        self.append_to_notebook(self.w_pane, os.path.basename(self.expdir),
                                PlideTab.TAB_TYPE_DIRECTORY)

    def double_click_callback( self, browser, pathname, isdir ):
        if not isdir:
            ext = os.path.splitext(pathname)[1]
            viewer = None
            if ext == ".pmat":
                pmat = PMatListModel(PMat(pathname, openmode='r'))
                viewer = MatrixViewer(pmat).get_widget()

            else:
                f = open(pathname)
                contents = f.read()
                f.close()
                viewer = PlideTabScintilla.scintilla_widget(contents, True)
                if ext == ".py" or ext == ".pymat" or ext == ".pyplearn" or ext == ".plearn":
                    PlideTabPython.set_scintilla_style_for_python(viewer)

            if viewer:
                container_remove_children(self.w_frame_right)
                viewer.show_all()
                self.w_frame_right.add(viewer)
