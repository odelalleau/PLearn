#  plide_options.py
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

import gtk

from plearn.utilities.metaprog import public_members
#from plearn.pyplearn.pyplearn  import *
from plearn.pyplearn           import *
from plearn.utilities.toolkit  import doc as toolkit_doc

from plearn.pl_pygtk import GladeAppWindow, GladeDialog, MessageBox

from plide_utils import *


#####  PyPLearnOptionsGroup  ################################################

class PyPLearnOptionsGroup( object ):
    """Store a group of option names.  Provide support for naming the
    thing, group documentation, etc.

    The argument 'group_members' is a list containing the actual names of
    options contained in the group.

    The argument 'group_object' is any object that should be updated when
    the options are changed by the GUI before executing the script. This is
    typically mapped to the classes inheriting from plargs_namespace in the
    pyplearn script.
    """
    def __init__(self, groupname, group_members, group_object,
                 group_description = None, doc = None):
        self.groupname         = groupname
        self.group_members     = group_members
        self.group_description = group_description
        self.group_object      = group_object
        self.doc               = doc

    def is_plopt( self, option_name ):
        return self.group_object.is_plopt(option_name)

    def get_plopt( self, option_name ):
        return self.group_object.get_plopt(option_name)

    def get( self, option_name ):
        return getattr(self.group_object, option_name)

    def set( self, option_name, option_value ):
        if self.group_object.is_plopt(option_name):
            setattr(self.group_object, option_name, option_value)
        else:
            setattr(self.group_object, option_name,
                    pyplearn_intelligent_cast(self.get(option_name), option_value))


#####  PyPLearnOptionsHolder  ###############################################

class PyPLearnOptionsHolder( object ):
    """Class which is initialized with a PyPLearnScript code (string) and
    manages to get its hands on the options supported by the script.  Used
    to initialize the PyPLearnOptionsDialog, which provides the view and
    controller for this object.
    """
    def __init__( self, script_name, script_code, script_directory ):
        self.script_name      = script_name
        self.script_code      = script_code
        self.launch_directory = script_directory
        self.log_verbosities  = [ "Mandatory", "Important", "Normal", "Debug", "Extreme" ]
        self.log_verbosity    = 2                  # Index in log_verbosities
        self.log_enable       = [ "__NONE__" ]     # List of named logs to enable
        self.option_overrides = [ ]                # Manual overrides
        self.option_groups    = [ ]                # List of groups of script options
        

        ## Assume that the script has already been executed and is
        ## syntactically valid.  (More formal execution context passing for
        ## subclasses of plargs_namespace will come later)

        # ## Look at global options
        # self.option_groups.append(PyPLearnOptionsGroup(
        #     'GlobalOptions', public_members(plarg_defaults),
        #     plarg_defaults, 'Global Configuration Options'))

        ## Look at all options in plargs_namespace
        for clsname,cls in plargs_namespace._subclasses.iteritems():
            short_doc = toolkit_doc(cls, 0).rstrip('.')
            full_doc  = toolkit_doc(cls, 1, "\n")
            group = PyPLearnOptionsGroup(clsname, public_members(cls).keys(),
                                         cls, short_doc, full_doc)
            self.option_groups.append(group)

    def pyplearn_actualize( self ):
        """Since the values of the fields in the options dialog box have
        already been reflected into the .pyplearn script, this function is
        only used to have the manual overrides come into effect.  Also
        handles logging control.
        """
        ## Generate a brand-new expdir
        plargs._parse_(["expdir="+generateExpdir()])
        
        ## Apply the manual overrides
        if self.option_overrides:
            plargs._parse_(self.option_overrides)

        verbosity_map = { 0:0, 1:1, 2:5, 3:10, 4:500 }
        verbosity = verbosity_map.get(self.log_verbosity, 5)
        injected.loggingControl(verbosity, self.log_enable)


#####  PyPLearnOptionsDialog  ###############################################

class PyPLearnOptionsDialog( GladeDialog ):
    """Class which is inialized with a PyPLearnScript code (string) and
    creates a dialog box to query the script options.
    """
    def __init__( self, options_holder ):
        GladeDialog.__init__(self, gladeFile())
        self.options_holder = options_holder

        ## Fill out the first page of notebook
        self.w_launch_directory.set_text(options_holder.launch_directory)
        for v in options_holder.log_verbosities:
            self.w_plearn_log_verbosity.append_text(v)
        self.w_plearn_log_verbosity.set_active(options_holder.log_verbosity)
        self.w_named_logs_activate.set_text(' '.join(options_holder.log_enable))

        ## Fill out the manual overrides page
        buf = gtk.TextBuffer()
        buf.set_text('\n'.join(options_holder.option_overrides))
        self.w_manual_script_options.set_buffer(buf)

        ## This holds a map from a widget-value-getting function to the
        ## associated group/field of the OptionsHolder.  Used to update the
        ## OptionsHolder when the user clicks OK on the dialog box.
        self.widget_map = [ ]

        for group in options_holder.option_groups:

            ## The body of the notebook page is contained in a scrolled window
            scrolled = gtk.ScrolledWindow()
            scrolled.set_policy(gtk.POLICY_NEVER, gtk.POLICY_AUTOMATIC)
            scrolled.set_shadow_type(gtk.SHADOW_NONE)
            align = gtk.Alignment(xalign=0.0, yalign=0.0, xscale=1.0, yscale=1.0)
            align.set_padding(padding_top=8, padding_bottom=6,
                              padding_left=8, padding_right=8)
            vbox = gtk.VBox(spacing=4)
            align.add(vbox)
            scrolled.add_with_viewport(align)
            scrolled.get_child().set_shadow_type(gtk.SHADOW_NONE)
            
            ## Insert group description in vbox, if any
            if group.group_description:
                label = gtk.Label()
                label.set_markup("<b><i>%s</i></b>" %
                                 markup_escape_text(group.group_description))
                label.set_line_wrap(False)
                label.set_alignment(0,0)
                label.set_padding(0,4)
                vbox.pack_start(label, expand=False, fill=False)

            ## Insert documentation in frame, if any
            if group.doc:
                label = gtk.Label(str=group.doc.replace("\n", ' ')+'\n')
                label.set_line_wrap(True)
                label.set_alignment(0,0)
                label.set_padding(0,2)
                vbox.pack_start(label, expand=False, fill=False)

            ## Insert a table with all the options in the group
            option_names = group.group_members
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

                ## Input field for option.  The following types are supported:
                ## bool     Checkbox
                ## int      Spinbutton with increment of 1 and min/max constraints
                ## float    Spinbutton with increment of 0.1 and min/max constraints
                ## str      Combobox if multiple choices
                ## str      Text entry field for unconstrained string
                ## list     Text entry field
                if group.is_plopt(option_names[i]):
                    option_object     = group.get_plopt(option_names[i])
                    option_value      = group.get(option_names[i])
                    option_type       = option_object.get_type()
                    option_docstr     = option_object.__doc__
                    option_bounds     = option_object.get_bounds()
                    option_choices    = option_object.get_choices()
                    option_fr_choices = option_object.get_free_choices()
                else:
                    option_value      = group.get(option_names[i])
                    option_type       = type(option_value)
                    option_docstr     = ""
                    option_bounds     = (None,None)
                    option_choices    = None
                    option_fr_choices = None

                def true_bounds(lower_bound, upper_bound):
                    if lower_bound is None:
                        lower_bound = -1e9
                    if upper_bound is None:
                        upper_bound = +1e9
                    return (lower_bound,upper_bound)

                ## Check box if bool
                if option_type == bool:
                    option_input = gtk.CheckButton()
                    option_input.set_active(bool(option_value))
                    getter = option_input.get_active

                ## Combo box
                elif option_choices is not None:
                    ## Works for both int and string choices
                    option_input = gtk.combo_box_new_text()
                    for choice in option_choices:
                        option_input.append_text(str(choice))
                    if option_value in option_choices:
                        option_input.set_active(option_choices.index(option_value))
                    else:
                        option_input.set_active(-1)

                    def getter(inp=option_input, choices=option_choices):
                        active = inp.get_active()
                        if active == -1:
                            return ""
                        else:
                            return choices[active]

                ## Combo box + entry field
                elif option_fr_choices is not None:
                    option_input = gtk.combo_box_entry_new_text()
                    for choice in option_fr_choices:
                        option_input.append_text(str(choice))
                    if option_value in option_fr_choices:
                        option_input.set_active(option_fr_choices.index(option_value))
                    else:
                        option_input.child.set_text(option_value)
                    getter = option_input.child.get_text

                ## Spin button (integer version)
                elif option_type == int:
                    option_input = gtk.SpinButton(digits=0)
                    option_input.set_increments(1,10)
                    option_input.set_numeric(True)
                    option_input.set_range(*true_bounds(*option_bounds))
                    option_input.set_value(option_value)
                    getter = option_input.get_text

                ## Spin button (float version)
                elif type(option_value) == float:
                    option_input = gtk.SpinButton(digits=3)
                    option_input.set_increments(0.1, 1.0)
                    option_input.set_numeric(True)
                    option_input.set_range(*true_bounds(*option_bounds))
                    option_input.set_value(option_value)
                    getter = option_input.get_text

                ## List or string
                else:
                    if type(option_value) == list:
                        text_value = ','.join([str(v) for v in option_value])
                    else:
                        text_value = str(option_value)
                    option_input = gtk.Entry(max=0)
                    option_input.set_text(text_value)
                    getter = option_input.get_text

                option_input.show()
                table.attach(option_input, left_attach=1, right_attach=2,
                             top_attach=i, bottom_attach=i+1,
                             xoptions=gtk.EXPAND | gtk.FILL, yoptions=gtk.EXPAND | gtk.FILL,
                             xpadding=4, ypadding=0)

                ## Remember how to map the widget's contents into options
                self.widget_map.append((getter, group, option_names[i]))

                ## Documentation for option
                option_doc = gtk.Label()
                option_doc.set_alignment(0.0, 0.5)   # Left align and middle-align
                option_doc.set_markup("<i>%s</i>" % markup_escape_text(option_docstr))
                option_doc.set_line_wrap(True)
                option_doc.show()
                table.attach(option_doc, left_attach=2, right_attach=3,
                             top_attach=i, bottom_attach=i+1,
                             xoptions=gtk.EXPAND | gtk.FILL, yoptions=gtk.EXPAND | gtk.FILL,
                             xpadding=4, ypadding=0)

            table.set_row_spacings(8)
            table.show()
            vbox.pack_start(table, expand=False, fill=False)

            ## Handle notebook page label creation and append to notebook
            notebook_label = gtk.Label()
            notebook_label.set_markup("<b>%s</b>" % group.groupname)
            notebook_label.set_alignment(0,0)
            scrolled.show_all()
            notebook_label.show_all()
            self.w_option_groups.append_page(scrolled, notebook_label)

        self.w_option_groups.set_current_page(0)


    def update_options_holder( self ):
        """This function updates the attached options_holder (and related
        plarg_namespace-derived) object with the latest contents of the
        dialog box entry widgets.  This is called when the user clicks OK
        or Apply
        """
        for (widget_getter, group, option_name) in self.widget_map:
            value = widget_getter()
            group.set(option_name, value)

        ## Updates from the first page of the dialog
        self.options_holder.launch_directory = self.w_launch_directory.get_text()
        self.options_holder.log_verbosity    = self.w_plearn_log_verbosity.get_active()
        self.options_holder.log_enable       = self.w_named_logs_activate.get_text().split()

        ## Updates from manual overrides
        buf  = self.w_manual_script_options.get_buffer()
        text = buf.get_text(buf.get_start_iter(), buf.get_end_iter())
        self.options_holder.option_overrides = text.split()

    def on_pick_directory_clicked( self, button ):
        chooser = gtk.FileChooserDialog(
            title="Open",
            action=gtk.FILE_CHOOSER_ACTION_SELECT_FOLDER,
            buttons= (gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,
                      gtk.STOCK_OPEN,  gtk.RESPONSE_OK))
        chooser.set_current_folder(self.w_launch_directory.get_text())
        chooser.set_default_response(gtk.RESPONSE_OK)
        response = chooser.run()

        ## Update the file in the entry box
        if response == gtk.RESPONSE_OK:
            self.w_launch_directory.set_text(chooser.get_filename())
        chooser.destroy()

    def define_injected( inj, glade_file ):
        """Simply transmits the 'injected' pseudo-module to this module.
        """
        global injected, gladeFile
        injected  = inj
        gladeFile = glade_file
    define_injected = staticmethod(define_injected)

