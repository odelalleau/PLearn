# pygtk.py
# Copyright (C) 2005-2006 by Pascal Vincent and Nicolas Chapados
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

"""This module contains utility functions to be used with pygtk

It contains facilities for loading Glade files, as well as running the GTK
main loop as a background thread.
"""

import pygtk
pygtk.require("2.0")
import gtk
import gtk.glade

import re, sys, threading


#####  Load Glade Files  ####################################################

# Regular expression to match handler method names patterns
# on_widget__signal and after_widget__signal.  Note that we use two
# underscores between the Glade widget name and the signal name.
handler_re = re.compile(r'^(on|after)_(.*)__(.*)$')

def glade_load_and_autoconnect(myobj, glade_file, widget_name=''):
    """Loads and constructs the specified widget from the specified glade
    file.  and connects signal handling methods of myobj to the widgets.
    
    Methods named like on_widget__signal or after_widget__signal are
    connected to the appropriate widgets and signals.  The call returns the
    glade_object, so that you may call glade_object.get_widget(widgetname) if
    you need to get the individual widgets.
    """
    gladeobj = None
    if widget_name!='':
        gladeobj = gtk.glade.XML(glade_file, widget_name)
    else:
        gladeobj = gtk.glade.XML(glade_file)
    get_widget = gladeobj.get_widget
    for attr in dir(myobj):
        match = handler_re.match(attr)
        if match:
            when, widget, signal = match.groups()
            method = getattr(myobj, attr)
            assert callable(method)
            if when == 'on':
                # print 'Connecting signal',signal,'of widget',widget
                get_widget(widget).connect(signal, method)
            elif when == 'after':
                get_widget(widget).connect_after(signal, method)
            elif attr.startswith('on_') or attr.startswith('after_'):
                # Warn about some possible typos like separating
                # widget and signal name with _ instead of __.
                print ('Warning: attribute %r not connected'
                       ' as a signal handler' % (attr,))
    return gladeobj


#####  GTK Event Loop Thread  ###############################################

gtk.threads_init()

class GtkMainLoop( threading.Thread ):
    """Encapsulates the GTK Event Loop as a separate thread.

    Call the class method startGTK() in order to start the start the event
    loop (if not already started).  The startGTK() method is a class method
    that can be called several times with no ill effect.
    """

    started_gtk = False
    
    def run(self):
        gtk.threads_enter()
        # print >>sys.stderr, "Starting GtkMainLoop background thread"
        gtk.main()
        gtk.threads_leave()

    def startGTK(cls):
        if not cls.started_gtk:
            main_loop = GtkMainLoop()
            main_loop.setDaemon(True)
            main_loop.start()
            cls.started_gtk = True
    startGTK = classmethod(startGTK)
    

#####  Glade/Pygtk Wrapper  #################################################

class GladeWidget(object):
    """Instantiate a Glade XML file for widget and connects the methods.

    This class loads a GLADE XML file and performs two things:
    
    1. It connects all signals in the Glade file to the methods of the
       same name in the class.

    2. It provides direct access to all widgets under the root widget as
       attributes of the class itself, but addingthe prefix 'w_'. For
       example, if Glade defines a widget 'help_text', the attribute name
       will be 'w_help_text'.  In addition, the root widget is available as
       'w_root'.
    """

    def __init__(self, gladefile, root_widgetname=""):
        if root_widgetname == "":
            root_widgetname = self.__class__.__name__
        gladexml = gtk.glade.XML(gladefile, root=root_widgetname)
        gladexml.signal_autoconnect(self)
        
        self.__gladefile__  = gladefile
        self.w_root         = gladexml.get_widget(root_widgetname)

        ## Make subwidgets available
        for widget in gladexml.get_widget_prefix(''):
            name = "w_" + widget.get_name()
            if hasattr(self, name):
                print "Duplicate attribute",name,"with value",getattr(self,name)
                raise RuntimeError, ("Attempting to add widget '%s' as attribute, but "   +\
                      "it already exists in object '%s' instantiated from gladefile '%s'") \
                      % (name, self.__class__.__name__, gladefile)
            setattr(self, name, widget)


class GladeAppWindow(GladeWidget):
    """Instantiate the Glade window and starts the threaded GtkMainLoop."""

    def run(self):
        self.close_event = threading.Event()
        self.w_root.show_all()
        GtkMainLoop.startGTK()
        # gtk.main()

    def quit(self, *args):
        self.close_event.set()
        gtk.main_quit()


class GladeDialog(GladeWidget):
    """Instantiate the Glade window and starts the dialog box."""

    def run(self):
        result = self.w_root.run()
        self.w_root.hide()
        return result

    def destroy(self):
        self.w_root.destroy()


#####  MessageBox (a la Windows)  ###########################################

def MessageBox(primary_text   = "",   secondary_text   = "",
               primary_markup = "",   secondary_markup = "",
               title=None, type=gtk.MESSAGE_INFO,
               buttons=gtk.BUTTONS_OK,
               custom_buttons = None,
               flags = gtk.DIALOG_MODAL):
    """A MessageBox a la Windows.

    The arguments are passed to GTK MessageDialog.  If custom_buttons is
    specified, it must be a callback function which is called with the
    dialog widget as its sole argument.  This function must perform
    add_buttons as appropriate.
    """
    if not primary_text and not primary_markup:
        raise ValueError, "Either 'primary_text' or 'primary_markup' must be specified"

    if custom_buttons is not None:
        buttons = gtk.BUTTONS_NONE
        
    dialog = gtk.MessageDialog(flags=flags, type=type, buttons=buttons,
                               message_format=primary_text)

    if custom_buttons:
        custom_buttons(dialog)
        
    if title:
        dialog.set_title(title)

    if primary_markup:
        dialog.set_markup(primary_markup)

    if secondary_text:
        dialog.format_secondary_text(secondary_text)

    if secondary_markup:
        dialog.format_secondary_markup(secondary_markup)

    result = dialog.run()
    dialog.hide()
    return result
