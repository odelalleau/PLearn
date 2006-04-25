#  plide.py
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


#####  Python Imports  ######################################################

import fcntl
import os, os.path
import Queue
import re
import select, sys
import threading, time, traceback


#####  GTK Imports  #########################################################

import gobject
import pygtk
pygtk.require('2.0')
import gtk, gtk.gdk
import pango
from plearn.pl_pygtk import GladeAppWindow, GladeDialog, MessageBox


#####  PLearn Imports  ######################################################

from plearn.utilities.metaprog import public_members
from plearn.pyplearn.pyplearn  import *
from plearn.utilities.toolkit  import doc as toolkit_doc


#####  Plide  ###############################################################

from plide_help  import PlideHelp
from plide_tabs  import *
from plide_utils import *


#####  Exports  #############################################################

__all__ = [
    'StartPlide',
    'QuitPlide',
    'GetWork',
    'PostWorkResults',
    'LogAppend',
    'AllocateProgressBar',
    'ReleaseProgressBar',
    'ProgressUpdate'
    ]


#####  Global Configuration  ################################################

def gladeFile():
    import plearn.plide.plide
    return os.path.join(os.path.dirname(plearn.plide.plide.__file__),
                        "resources", "plide.glade")

def helpResourcesPath():
    import plearn.plide.plide
    return os.path.join(os.path.dirname(plearn.plide.plide.__file__),
                        "resources")


#####  Main Window  #########################################################

class PlideMain( GladeAppWindow ):

    PlideVersion = "0.01"

    def __init__( self, *args, **kwargs ):
        GladeAppWindow.__init__(self, gladeFile())

        ## Forward injected to imported Plide modules
        PlideHelp.define_injected(injected)
        PlideTab. define_injected(injected)

        ## Initialize Members
        self.untitled_counter  = 1
        self.work_requests = {}         # Request ids to expdir mapping
        self.all_plearn_classes = injected.getAllClassnames()

        ## Initialize Display
        self.setup_statusbar()
        self.log_filters = [ re.compile("WARNING.*Scintilla.*PosChanged.*deprecated") ]
        self.log_clear()
        self.log_hide()
        welcome_text = kwargs.get("welcome_text",
                                  "<b>Welcome to Plide %s!</b>" % self.PlideVersion)
        self.status_display(welcome_text, has_markup=True)
        self.setup_stdouterr_redirect()
        
        ## Set up help system
        injected.helpResourcesPath(helpResourcesPath())
        self.help_viewer = PlideHelp(self)
        self.help_viewer.display_page("index.html")
        self.help_close()

        ## Prepare the work queue
        self.work_queue = PLearnWorkQueue()

    def quit( self ):
        ## Minor hack: the main-thread loop is terminated by receiving a
        ## 'script' whose contents is Quit().  First close all tabs and
        ## ensure that we stop the process if some tabs won't be closed.
        n = self.w_plide_notebook.get_n_pages()
        for i in range(n-1,-1,-1):
            tab = self.get_nth_tab(i)
            if not tab.close_tab():
                return True        # Stop close process if cannot close tab
        
        print >>raw_stderr, "Quit message received"
        raw_stderr.flush()
        self.work_queue.post_work_request("Quit()","","")
        GladeAppWindow.quit(self)

    def help_close( self ):
        """Close the help pane.
        """
        self.w_help_frame.hide()
        self.w_help_frame.set_no_show_all(True)

    def help_show( self ):
        """Open the help pane.  Bring up context-sensitive help if there is
        a valid context in the current tab.
        """
        self.w_help_frame.set_no_show_all(False)
        self.w_help_frame.show()

        curtab = self.get_current_tab()
        if curtab:
            help_context = curtab.get_help_candidate()
            if help_context:
                self.help_viewer.display_page(help_context)
            
    def setup_stdouterr_redirect( self ):
        """Redirect standard output and error to be sent to the log pane
        instead of the console.
        """
        ## Redirect standard output and standard error to display to the
        ## log pane area.  Keep around old stdout/stderr in order to
        ## display debugging messages.  They are called, respectively,
        ## raw_stdout and raw_stderr (Python file objects)
        global raw_stdout, raw_stderr
        old_stdout_fd = os.dup(sys.stdout.fileno())
        old_stderr_fd = os.dup(sys.stderr.fileno())
        raw_stdout    = os.fdopen(old_stdout_fd, 'w')
        raw_stderr    = os.fdopen(old_stderr_fd, 'w')

        print >>sys.stderr, "Original stderr"
        print >>raw_stderr, "Redirected stderr"
        sys.stderr.flush()
        raw_stderr.flush()
        
        (self.stdout_read, self.stdout_write) = os.pipe()
        (self.stderr_read, self.stderr_write) = os.pipe()
        os.dup2(self.stdout_write, sys.stdout.fileno())
        os.dup2(self.stderr_write, sys.stderr.fileno())
        
        out_flags = fcntl.fcntl(self.stdout_read, fcntl.F_GETFL)
        err_flags = fcntl.fcntl(self.stderr_read, fcntl.F_GETFL)
        fcntl.fcntl(self.stdout_read, fcntl.F_SETFL, out_flags | os.O_NONBLOCK)
        fcntl.fcntl(self.stderr_read, fcntl.F_SETFL, err_flags | os.O_NONBLOCK)

        def callback( fd, cb_condition ):
            # print >>raw_stderr, "log_updater: got stuff on fd", fd, \
            #       "with condition", cb_condition
            raw_stderr.flush()
            data = os.read(fd,65536)

            kind = { self.stdout_read:'stdout', self.stderr_read:'stderr' }.get(fd,'')
            self.log_display(data, kind)
            return True                 # Ensure it's called again!
        
        gobject.io_add_watch(self.stdout_read, gobject.IO_IN, callback)
        gobject.io_add_watch(self.stderr_read, gobject.IO_IN, callback)

    def setup_statusbar( self ):
        """Arrange the status bar area to contain both a status line and a progress bar.
        """
        ## The GTK StatusBar widget is a pain to use.  Our statusbar is an
        ## hbox packed at the bottom of main_vbox, and containing:
        ##
        ## - A frame with a status label
        ## - A progress bar
        self.w_statusframe = gtk.Frame()
        self.w_statusframe.set_shadow_type(gtk.SHADOW_IN)
        self.status_display(' ')        # Establish proper height
        self.w_statusframe.show()

        self.w_progressbar = gtk.ProgressBar()
        self.w_progressbar.set_ellipsize(pango.ELLIPSIZE_END)
        self.w_progressbar.show()

        ## This gives a list of the "order" in which progress bars should
        ## be allocated as well as whether each one has been allocated
        self.all_progressbars   = [ (self.w_progressbar,False) ]

        self.w_statusbar = gtk.Table(rows=1, columns=4)
        self.w_statusbar.attach(self.w_statusframe, left_attach=0, right_attach=3,
                                top_attach=0, bottom_attach=1,
                                xoptions=gtk.EXPAND | gtk.FILL,
                                yoptions=gtk.EXPAND | gtk.FILL,
                                xpadding=0, ypadding=0)
        self.w_statusbar.attach(self.w_progressbar, left_attach=3, right_attach=4,
                                top_attach=0, bottom_attach=1,
                                xoptions=gtk.EXPAND | gtk.FILL,
                                yoptions=gtk.EXPAND | gtk.FILL,
                                xpadding=2, ypadding=2)
        self.w_statusbar.show()
        self.w_main_vbox.pack_start(self.w_statusbar, expand=False, fill=False)

    def log_clear( self ):
        """The log is a TreeView widget that contains 3 columns: (1)
        number, (2) kind, (3) log entry itself.  This sets up a new log or
        clears an existing log.
        """
        self.log_liststore = gtk.ListStore(int, str, str, int)

        ## Create individual columns
        self.log_columns = [ gtk.TreeViewColumn(x) for x in [ 'No.', 'Kind', 'Message' ] ]

        ## Create cell renderers for displaying the contents
        self.log_cells = [ gtk.CellRendererText() for i in xrange(len(self.log_columns)) ]
        for i, (column, cell) in enumerate(zip(self.log_columns, self.log_cells)):
            column.pack_start(cell, True)
            column.add_attribute(cell, 'text', i)
            column.set_sort_column_id(i)

        ## Last column (message) is a bit tricky: we either set the
        ## attribute 'text' or 'markup' depending on the setting of the
        ## fourth model column
        def message_format_func(treeviewcolumn, cell, model, iter):
            message = model.get(iter,2)[0]
            markup  = model.get(iter,3)[0]
            cell.set_property('family-set', True)
            if markup:
                cell.set_property('markup', message)
                cell.set_property('family', 'Helvetica')
            else:
                cell.set_property('text', message)
                cell.set_property('family', 'Monospace')
        self.log_columns[2].set_cell_data_func(self.log_cells[2], message_format_func)
            
        ## Create the TreeView using underlying liststore model and append columns
        ## and make it searchable on the message
        self.log_treeview = gtk.TreeView(self.log_liststore)
        for column in self.log_columns:
            self.log_treeview.append_column(column)
        self.log_treeview.set_search_column(2)
        self.log_treeview.set_rules_hint(True)

        container_remove_children(self.w_plearn_log_scroller)
        self.w_plearn_log_scroller.add(self.log_treeview)
        self.log_treeview.show_all()

        ## Reset the next log entry
        self.log_next_number = 1

    def log_hide( self ):
        self.w_plearn_log_scroller.hide()
        self.w_plearn_log_scroller.set_no_show_all(True)

    def log_show( self ):
        self.w_plearn_log_scroller.set_no_show_all(False)
        self.w_plearn_log_scroller.show()
        
    def log_display( self, message, kind = "", has_markup = False, log_clear = False ):
        """Append the given message to the log area of the main window.
        Thread-safe.
        """
        if log_clear:
            self.log_clear()

        ## If 'data' matches any regular expression in log_filter, skip
        ## this message.  This is mostly a hack to get around displaying
        ## known warnings from the Scintilla editor
        for regex in self.log_filters:
            if regex.search(message):
                return

        row = [ self.log_next_number, kind.rstrip(), message.rstrip(), has_markup ]
        self.log_show()
        self.log_liststore.append(row)
        self.log_next_number += 1

    def status_display( self, message, has_markup = False ):
        """Display the given message in the status bar area of the main window.
        Thread-safe.
        """
        label = gtk.Label(message)
        if has_markup:
            label.set_markup(message)
        label.set_line_wrap(False)
        label.set_alignment(0,0)
        label.set_padding(2,2)
        label.set_single_line_mode(True)
        label.set_ellipsize(pango.ELLIPSIZE_MIDDLE)
        label.show()
        container_remove_children(self.w_statusframe)
        self.w_statusframe.add(label)

    def cursor_hourglass( self, unsensitize = True ):
        """Make the cursor an hourglass for the main window.

        In addition, if the argument unsensitize is True, most entry
        will be disabled for the window.
        """
        self.w_root.window.set_cursor(gtk.gdk.Cursor(gtk.gdk.WATCH))
        if unsensitize:
            self.w_root.set_sensitive(False)

    def cursor_normal( self, sensitize = True ):
        """Take back the cursor to normal form for the main window.
        """
        self.w_root.window.set_cursor(None)  # Set back to parent window cursor
        if sensitize:
            self.w_root.set_sensitive(True)

    def get_nth_tab( self, n ):
        """Return the PlideTab object corresponding to the n-th tab.
        Return None if there is no such PlideTab.
        """
        notebook_page = self.w_plide_notebook.get_nth_page(n)
        if notebook_page:
            return notebook_page.plide_tab_object
        else:
            return None

    def get_current_tab( self ):
        """Return the PlideTab object that's currently selected in the
        notebook.  Return None if none...
        """
        cur_page = self.w_plide_notebook.get_current_page()
        if cur_page >= 0:
            return self.w_plide_notebook.get_nth_page(cur_page).plide_tab_object
        else:
            return None


    #####  Progress Bar Handling  ###########################################

    def reset_progress( self ):
        """Bring back the all progress bars to an 'available' state,
        erase their containing text, and bring the fraction to zero.
        """
        for i,(pb,alloc) in enumerate(self.all_progressbars):
            pb.set_text('')
            pb.set_fraction(0.0)
            self.all_progressbars[i] = (pb,False)

    def allocate_progress( self ):
        """Return the ID of an available progress bar, or -1 if none is
        available.
        """
        for i,(pb,alloc) in enumerate(self.all_progressbars):
            if not alloc:
                self.all_progressbars[i] = (pb,True)
                return i

        return -1

    def release_progress( self, progress_id ):
        """Release an available progress bar and make it available for
        other uses.
        """
        (pb,alloc) = self.all_progressbars[progress_id]
        self.all_progressbars[progress_id] = (pb,False)

    def get_progress_from_id( self, progress_id ):
        """Return the gtk.ProgressBar widget corresponding to its id.
        """
        return self.all_progressbars[progress_id][0]


    #####  Callbacks  #######################################################

    ## General
    def on_plide_top_delete_event(self, widget, event):
        return self.quit()

    def on_quit_activate(self, widget):
        self.quit()

    ## File Menu
    def on_new_activate(self, widget):
        self.add_untitled_tab(".pyplearn")
        
    def on_new_pyplearn_script_activate(self, widget):
        self.add_untitled_tab(".pyplearn")

    def on_new_py_script_activate(self, widget):
        self.add_untitled_tab(".py")

    def on_new_plearn_script_activate(self, widget):
        self.add_untitled_tab(".plearn")

    def on_new_text_file_activate(self, widget):
        self.add_untitled_tab("")

    def on_open_activate(self, widget):
        self.open_file()

    def on_save_activate(self, widget):
        if self.get_current_tab():
            self.get_current_tab().on_save_activate()

    def on_save_as_activate(self, widget):
        if self.get_current_tab():
            self.get_current_tab().save_as_file()

    def on_close_activate(self, widget):
        if self.get_current_tab():
            self.get_current_tab().close_tab(widget)

    def on_browse_expdir_activate(self, widget):
        self.open_file(action=gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER)

    ## Edit menu
    def on_undo_activate(self, widget):
        self.get_current_tab().on_undo_activate()

    def on_redo_activate(self, widget):
        self.get_current_tab().on_redo_activate()

    def on_cut_activate(self, widget):
        self.get_current_tab().on_cut_activate()

    def on_copy_activate(self, widget):
        self.get_current_tab().on_copy_activate()

    def on_paste_activate(self, widget):
        self.get_current_tab().on_paste_activate()

    ## Help menu
    def on_about_activate(self, widget):
        version = injected.versionString().replace("(","\n(")
        MessageBox("PLearn Integrated Development Environment Version " + self.PlideVersion,
                   "Running on " + version + "\n" +\
                   "Copyright (c) 2006 by Nicolas Chapados",
                   title = "About Plide")

    ## Toolbar
    def on_toolbutton_new_pyplearn_clicked(self, widget):
        self.add_untitled_tab(".pyplearn")

    def on_toolbutton_open_clicked(self, widget):
        self.open_file()

    def on_toolbutton_options_clicked(self, widget):
        """Display dialog box for establishing script options.
        """
        tab = self.get_current_tab()
        if tab is not None:
            script = tab.get_text()
            name   = tab.get_basename()

            options_holder = tab.get_options_holder()
            print >>sys.stderr, "Existing options_holder:", options_holder
            if not options_holder:
                ## When executing for the first time, run the script
                if self.pyplearn_parse( name, script ) is None:
                    return              # Syntax errors in script
                options_holder = PyPLearnOptionsHolder(name, script)
                tab.set_options_holder(options_holder)

            # print >>sys.stderr, "Initial option_holder values:", options_holder.option_groups

            options_dialog = PyPLearnOptionsDialog(options_holder)
            result = options_dialog.run()
            if result == gtk.RESPONSE_OK:
                options_dialog.update_options_holder()

            # print "New option_holder values:", options_holder.option_groups
            # options_dialog.destroy()

    def on_toolbutton_execute_clicked(self, widget):
        """Launch the execution of the pyplearn script, only if it's indeed
        such a script.
        """
        tab = self.get_current_tab()
        if tab is not None:
            if type(tab) == PlideTabPyPLearn:
                script_name = tab.get_basename()
                script_code = tab.get_text()
                script_dir  = tab.get_script_directory()
                options_holder = tab.get_options_holder()
                self.pyplearn_executor(script_name, script_code, script_dir,
                                       options_holder)


    ### Help-related
    def on_help_activate(self, widget):
        self.help_show()
        
    def on_help_close_clicked(self, widget):
        self.help_close()
        

    #####  Tab Handling  ####################################################

    def add_untitled_tab(self, extension):
        self.add_intelligent_tab("untitled%d%s" % (self.untitled_counter,
                                                   extension), is_new=True)
        self.untitled_counter += 1

    def add_intelligent_tab(self, filename, is_new = False):
        """Create a new tab 'intelligently' depending on the filename type
        or its extension. (If the file is new, rely on its extension only).
        """
        filename = filename.rstrip(os.path.sep)
        extension = os.path.splitext(filename)[1]
        new_tab   = None
        if extension == ".pyplearn" or extension == ".py":
            new_tab = PlideTabPyPLearn(self.w_plide_notebook, filename, is_new,
                                       self.all_plearn_classes)

        elif extension == ".pmat":
            new_tab = PlideTabPMat(self.w_plide_notebook, filename)

        elif os.path.isdir(filename):
            new_tab = PlideTabExpdir(self.w_plide_notebook, filename)

        elif os.path.exists(filename):
            new_tab = PlideTabFile(self.w_plide_notebook, filename, is_new)

        elif is_new:
            new_tab = PlideTabFile(self.w_plide_notebook, filename, is_new)
            
        else:
            MessageBox("File '%s' is of unrecognized type" % filename,
                       type=gtk.MESSAGE_ERROR)

        self.status_display('')         # Clear the status

    def open_file(self, action=gtk.FILE_CHOOSER_ACTION_OPEN):
        """Display a file chooser dialog and add a new tab based on selected file.
        """
        chooser = gtk.FileChooserDialog(
            title="Open",
            action=action,
            buttons= (gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,
                      gtk.STOCK_OPEN,  gtk.RESPONSE_OK))
        chooser.set_default_response(gtk.RESPONSE_OK)
        response = chooser.run()

        ## Add all files selected by the user
        if response == gtk.RESPONSE_OK:
            filenames = chooser.get_filenames()
            for f in filenames:
                self.add_intelligent_tab(f, is_new=False)
            
        chooser.destroy()


    #####  PLearn execution  ############################################

    def pyplearn_executor( self, script_name, script_code, script_directory,
                           options_holder ):
        """Execute a pyplearn script within an options context.
        
        Operations are as follows:
        
        1. We execute the script one more time with the more
           recent text (may have changed since last time options
           were set)
        2. We set the options corresponding each scoped object
           in their own class
        3. We parse the manual command-line arguments
        4. We transform the script to a .plearn
        5. We grab a hold of the soon-to-be-created expdir
        6. We hand this script off to PLearn for execution
        """
        script_env = self.pyplearn_parse( script_name, script_code )
        if script_env is not None:
            if options_holder:
                options_holder.pyplearn_actualize()
            expdir = plargs.expdir
            print >>sys.stderr, 'Expdir is:', expdir
            plearn_script = eval('str(PyPLearnScript( main() ))', script_env)
            print >>sys.stderr, 'PLearn is:'
            print >>sys.stderr, plearn_script[0:100],"..."
            message = 'Launching script <b>%s</b> in directory <b>%s</b>' % \
                      (script_name, script_directory)
            self.status_display(message, has_markup=True)
            self.log_display   (message, has_markup=True, log_clear=True)
            request_id = self.work_queue.post_work_request(
                plearn_script, script_directory, "pyplearn")
            
            print >>sys.stderr, "Caller executing request_id", request_id
            sys.stderr.flush()
            self.work_requests[request_id] = os.path.join(script_directory,expdir)
            self.add_plearn_results_monitor( script_name, request_id )

    def add_plearn_results_monitor( self, script_name, request_id, interval = 100 ):
        """Add a monitor callback to check for availability of PLearn
        results every 'interval' milliseconds.
        """
        def callback( ):
            completion_result = \
                self.work_queue.work_request_completed(request_id)

            if completion_result is not None:
                gtk.threads_enter()     # This is not a GTK+ callback
                self.reset_progress()
                (result_code, result_details) = completion_result

                ## If result_code is "", it means everything is OK.
                if result_code == "":
                    message = "<b>%s</b> completed successfully" % script_name
                    self.status_display(message, has_markup = True)
                    self.log_display(message, has_markup = True)

                else:
                    status_msg = "<b>%s</b> terminated due to errors" % script_name
                    self.status_display(status_msg, has_markup = True)
                    message = ('A fatal error of kind "%s" was encountered during ' +\
                               'execution of script "%s".') % (result_code, script_name)
                    details = "Details:\n" + result_details
                    self.log_display(message+"\n"+details, has_markup = False)
                    MessageBox(message, details, type=gtk.MESSAGE_ERROR)

                ## Done: don't call again, and add a new tab corresponding
                ## to the expdir.  Check for expdir existence first, since
                ## some experiments don't necessarily leave an expdir around
                if os.path.isdir(self.work_requests[request_id]):
                    self.add_intelligent_tab(self.work_requests[request_id])
                gtk.threads_leave()
                return False
            else:
                ## Call again later
                return True

        ## Add the callback to the GTK list of callback
        callback_id = gobject.timeout_add(interval, callback)
        
        
    #####  PyPLearn Parse  ##############################################
    
    def pyplearn_parse( self, script_name, script_code ):
        """Ensure that a pyplearn script parses without error.

        If an error is encountered, a backtrace is emitted to the log aread
        and a message box is popped to indicate the error.  Return None in
        this case.  Return the script execution environment if no error is
        encountered.
        """
        script_env = { }
        try:
            self.cursor_hourglass()
            exec script_code in script_env
        except:
            (exc_type, exc_value, tb) = sys.exc_info()
            tb_string = ''.join(traceback.format_tb(tb))
            self.log_display(tb_string)
            self.cursor_normal()
            self.status_display("Exception while parsing script <b>%s</b>."
                                % script_name, True)
            MessageBox('Script "%s" raised exception "%s: %s".' \
                       % (script_name, str(exc_type), str(exc_value)),
                       "See the log area for the detailed traceback.",
                       title = "PyPLearn Script Error",
                       type=gtk.MESSAGE_ERROR)
            return None
        else:
            self.cursor_normal()
            self.status_display("Script <b>%s</b> parsed successfully."
                                % script_name, True)
            return script_env


#####  PyPLearnOptionsGroup  ################################################

class PyPLearnOptionsGroup( dict ):
    """Store a group of options (maps either a plargs_namespace or
    plargs_binder).  Provide support for naming the thing, group
    documentation, etc.

    The argument 'group_members' is a dict containing the actual members of
    the group, in the form name:value, where value can be a simple type or
    a fancier documenting property object.

    The argument 'group_object' is any object that should be updated when
    the options are changed by the GUI before executing the script. This is
    typically mapped to the classes inheriting from plargs_namespace in the
    pyplearn script.

    Note that the whole object behaves like a built-in dict, but with
    additional attributes
    """
    def __init__(self, groupname, group_members, group_object,
                 group_description = None, doc = None):
        self.groupname         = groupname
        self.group_description = group_description
        self.group_object      = group_object
        self.doc               = doc
        self.update(group_members)


#####  PyPLearnOptionsHolder  ###############################################

class PyPLearnOptionsHolder( object ):
    """Class which is initialized with a PyPLearnScript code (string) and
    manages to get its hands on the options supported by the script.  Used
    to initialize the PyPLearnOptionsDialog, which provides the view and
    controller for this object.
    """
    def __init__( self, script_name, script_code ):
        self.script_name      = script_name
        self.script_code      = script_code
        self.option_groups    = [ ]
        self.option_overrides = [ ]

        ## Assume that the script has already been executed and is
        ## syntactically valid.  (More formal execution context passing for
        ## subclasses of plargs_namespace will come later)

        ## Look at global options
        self.option_groups.append(PyPLearnOptionsGroup(
            'GlobalOptions', public_members(plarg_defaults),
            plarg_defaults, 'Global Configuration Options'))

        ## Look at all options in plargs_namespace
        for clsname,cls in plargs_namespace._subclasses.iteritems():
            short_doc = toolkit_doc(cls, 0).rstrip('.')
            full_doc  = toolkit_doc(cls, 1, "\n")
            
            group = PyPLearnOptionsGroup(clsname, public_members(cls),
                                         cls, short_doc, full_doc)
            self.option_groups.append(group)

    def pyplearn_actualize( self ):
        """Copy back the current values held by the group members to their
        holding class in the .pyplearn
        """
        for group in self.option_groups:
            obj = group.group_object
            for k,v in group.iteritems():
                setattr(obj, k, v)

        ## Apply the manual overrides
        if self.option_overrides:
            plargs._parse_(self.option_overrides)


#####  PyPLearnOptionsDialog  ###############################################

class PyPLearnOptionsDialog( GladeDialog ):
    """Class which is inialized with a PyPLearnScript code (string) and
    creates a dialog box to query the script options.
    """
    def __init__( self, options_holder ):
        GladeDialog.__init__(self, gladeFile())
        self.options_holder = options_holder

        ## Ensure that we never put an horizontal scrollbar, and put an
        ## automatic vertical one.
        self.w_options_scrolledwindow.set_policy(gtk.POLICY_NEVER,
                                                 gtk.POLICY_AUTOMATIC)

        ## This holds a map from an data-entry widget to the associated
        ## group/field of the OptionsHolder.  Used to update the
        ## OptionsHolder when the user clicks OK on the dialog box.
        self.widget_map = {}

        for group in options_holder.option_groups:

            ## Handle frame creation
            label_text = "<b>%s</b>" % group.groupname
            if group.group_description:
                label_text += " (<i>%s</i>)" % group.group_description
            
            new_frame = gtk.Frame(label = label_text)
            new_frame.get_label_widget().set_use_markup(True)
            new_frame.set_shadow_type(gtk.SHADOW_ETCHED_IN)

            align = gtk.Alignment(xalign=0.0, yalign=0.0, xscale=1.0, yscale=1.0)
            align.set_padding(padding_top=4, padding_bottom=8,
                              padding_left=8, padding_right=8)
            vbox = gtk.VBox(spacing=4)
            align.add(vbox)
            align.show()

            ## Insert documentation in frame if there is any
            if group.doc:
                label = gtk.Label(str=group.doc.replace("\n", ' '))
                label.set_line_wrap(True)
                label.set_alignment(0,0)
                label.set_padding(0,4)
                label.show()
                vbox.pack_start(label)

            ## Insert a table with all the options in the group
            option_names = group.keys()
            option_names.sort()
            table = gtk.Table(rows=len(option_names), columns=3, homogeneous=False)
            for i in xrange(len(option_names)):
                ## Option name
                option_label = gtk.Label(option_names[i])
                option_label.set_alignment(1.0, 0.5)   # Right align and middle-align
                option_label.show()
                table.attach(option_label, left_attach=0, right_attach=1,
                             top_attach=i, bottom_attach=i+1,
                             xoptions=gtk.EXPAND | gtk.FILL, yoptions=gtk.EXPAND | gtk.FILL,
                             xpadding=4, ypadding=0)

                ## Input field for option; 3 types are supported:
                ## - Integers: create a SpinButton with an increment of 1
                ## - Floats  : create a SpinButton with an increment of 0.1
                ## - Other   : create a text entry field
                option_value = group[option_names[i]]
                if type(option_value) == int:
                    option_input = gtk.SpinButton(digits=0)
                    option_input.set_increments(1,10)
                    option_input.set_numeric(True)
                    option_input.set_range(-1e6, 1e6)
                    option_input.set_value(option_value)
                elif type(option_value) == float:
                    option_input = gtk.SpinButton(digits=3)
                    option_input.set_increments(0.1, 1.0)
                    option_input.set_numeric(True)
                    option_input.set_range(-1e6, 1e6)
                    option_input.set_value(option_value)
                else:
                    if type(option_value) == list:
                        text_value = ','.join([str(v) for v in option_value])
                    else:
                        text_value = str(option_value)
                    option_input = gtk.Entry(max=0)
                    option_input.set_text(text_value)
                option_input.show()
                table.attach(option_input, left_attach=1, right_attach=2,
                             top_attach=i, bottom_attach=i+1,
                             xoptions=gtk.EXPAND | gtk.FILL, yoptions=gtk.EXPAND | gtk.FILL,
                             xpadding=4, ypadding=0)

                ## Remember what the new input widget maps to
                self.widget_map[option_input] = (group, option_names[i])

                ## Documentation for option
                option_doc = gtk.Label()
                option_doc.set_markup("<i>Some documentation would come here</i>")
                option_doc.show()
                table.attach(option_doc, left_attach=2, right_attach=3,
                             top_attach=i, bottom_attach=i+1,
                             xoptions=gtk.EXPAND | gtk.FILL, yoptions=gtk.EXPAND | gtk.FILL,
                             xpadding=4, ypadding=0)

            table.set_row_spacings(8)
            table.show()
            vbox.pack_start(table)
            vbox.show()

            new_frame.add(align)
            new_frame.show()

            self.w_options_vbox.pack_start(new_frame)


    def update_options_holder( self ):
        """This function updates the attached options_holder object with
        the latest contents of the dialog box entry widgets.  This is called
        when the user clicks OK or Apply
        """
        for (widget, (group,option_name)) in self.widget_map.iteritems():
            group[option_name] = pyplearn_intelligent_cast(group[option_name],
                                                           widget.get_text())

        ## Update the manual overrides that come from a fixed widget
        overrides_str  = self.w_manual_script_options.get_text()
        overrides_list = overrides_str.split()
        self.options_holder.option_overrides = overrides_list


#####  Utility Classes  #####################################################

class PLearnWorkQueue( object ):
    """Worker thread for PLearn computation.

    Since PLearn is quite highly dependent on the assumption of single
    threading, this object simply passes work requests between the GUI
    thread (which runs Plide) and the main thread (which runs PLearn).
    Work requests are posted from the GUI the post_work_request() method,
    which returns a work_id.  You can then poll the worker for task
    completion status, or sleep until the task is completed.

    Likewise, the main thread calls get_work_request(), which blocks until
    a work request arrives.  The method returns a 4-tuple of strings of the
    form [ request_id, script, root_directory, script_type ].  The main
    thread can post results using the post_work_results() method.
    """
    def __init__( self ):
        self.requests_queue     = Queue.Queue()
        self.results_queue      = Queue.Queue()
        self.next_request_id    = 1
        self.completion_results = { }

    def post_work_request( self, script, script_dir, script_kind ):
        """Post a new work request to PLearn and return immediately.
        """
        request_id = self.next_request_id
        self.completion_results[str(request_id)] = None
        self.requests_queue.put((str(request_id), script, script_dir, script_kind))
        self.next_request_id += 1
        return request_id

    def get_work( self ):
        """Called by the PLearn main thread to get work to do.  Blocks
        until there is work.
        """
        return self.requests_queue.get()

    def post_work_results( self, request_id, code, results ):
        """Called by the PLearn main thread to announce that it has finished
        processing the work request 'request_id'.
        """
        self.results_queue.put((request_id, code, results))

    def work_request_completed( self, request_id ):
        """Return a pair (Code,Results) if the given work request is
        finished processing by PLearn, and None if it's still being
        processed.
        """
        while not self.results_queue.empty():
            (cur_id, code, results) = self.results_queue.get()
            self.completion_results[cur_id] = (code, results)
            print >>raw_stderr, "work_request_completed:", (cur_id, code, results)
            raw_stderr.flush()

        return self.completion_results[str(request_id)]

    def wait_for_work_request( self, request_id ):
        """Wait (block) until the specified work request is finished
        processing by PLearn.
        """
        while self.completion_results[str(request_id)] is None:
            (cur_id, code, results) = self.results_queue.get()
            self.completion_results[cur_id] = (code, results)


#####  C++ Functional Interface  ############################################

# global plide_main_window
plide_main_window = None

def StartPlide(argv = []):
    global plide_main_window
    plide_main_window = PlideMain()
    plide_main_window.run()       # Show and start event loop in other thread

    ## Consider each file passed as command-line argument and create a tab
    ## to view it.
    for arg in argv:
        plide_main_window.add_intelligent_tab(arg, not os.path.exists(arg))

def QuitPlide():
    if not plide_main_window.close_event.isSet():
        gtk.threads_enter()
        plide_main_window.quit()
        gtk.threads_leave()

def GetWork():
    return plide_main_window.work_queue.get_work()

def PostWorkResults( request_id, code, results ):
    plide_main_window.work_queue.post_work_results(request_id, code, results)

def LogAppend( kind, severity, message ):
    gtk.threads_enter()
    plide_main_window.log_display( message, kind )
    gtk.threads_leave()
    

#####  C++ Progress Bar Interface  ##########################################

def AllocateProgressBar( text ):
    progress_id = plide_main_window.allocate_progress()
    # print >>raw_stderr, "Allocating progress bar", progress_id
    raw_stderr.flush()
    if progress_id >= 0:
        gtk.threads_enter()
        pb = plide_main_window.get_progress_from_id(progress_id)
        pb.set_text(text)
        pb.set_fraction(0.0)
        gtk.threads_leave()
    return progress_id

def ReleaseProgressBar( progress_id ):
    # print >>raw_stderr, "Releasing progress bar", progress_id
    raw_stderr.flush()
    if progress_id >= 0:
        gtk.threads_enter()
        pb = plide_main_window.get_progress_from_id(progress_id)
        pb.set_fraction(1.0)            # Make it complete on screen
        gtk.threads_leave()
        plide_main_window.release_progress( progress_id )

def ProgressUpdate( progress_id, fraction ):
    # print >>raw_stderr, "Updating progress bar", progress_id,"to fraction",fraction
    raw_stderr.flush()
    if progress_id >= 0:
        gtk.threads_enter()
        pb = plide_main_window.get_progress_from_id(progress_id)
        pb.set_fraction(fraction)
        gtk.threads_leave()


#####  Standalone Running  ##################################################

if __name__ == "__main__":
    class Poubelle:
        def __getattr__(self, attr): return None

    global plide_main_window
    global injected
    injected = Poubelle()
    
    StartPlide()
    # print >>sys.stderr, "Random stuff to stderr"
    # print >>sys.stdout, "Random stuff to stdout"
    # sys.stderr.flush()
    # sys.stdout.flush()

    # prid = AllocateProgressBar("Simple Progress Text")
    # time.sleep(1)
    # ProgressUpdate(prid,0.33)
    # time.sleep(1)
    # ProgressUpdate(prid,0.66)
    # time.sleep(1)
    # ReleaseProgressBar(prid)
    # time.sleep(1)

    plide_main_window.close_event.wait()  # Wait for window to be closed
    QuitPlide()
    
