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

#"""This module contains utility functions to be used with pygtk
#
#It contains facilities for loading Glade files, as well as running the GTK
#main loop as a background thread.
#"""

import pygtk
pygtk.require("2.0")
import gtk
import gtk.glade

import re


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

class GtkMainLoop(threading.Thread):
    """Encapsulates the GTK Event Loop as a separate thread.

    Call the class method startGTK() in order to start the start the event
    loop (if not already started).  The startGTK() method is a class method
    that can be called several times with no ill effect.
    """

    started_gtk = False
    
    def run(self):
        gtk.threads_enter()
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

class PyGTKWidget(object):
    """Instantiate a Glade XML file for widget and connects the methods."""

    def __init__(self, gladefile, widgetname=""):
        if widgetname == "":
            widgetname = self.__class__.__name__
        gladexml = gtk.glade.XML(gladefile, root=widgetname)
        gladexml.signal_autoconnect(self)
        self.widgetname = widgetname
        self.gladefile  = gladefile
        self.widget     = gladexml.get_widget(widgetname)


class PyGTKAppWindow(PyGTKWidget):
    """Instantiate the Glade window and starts the threaded GtkMainLoop."""

    def run(self):
        self.widget.show_all()
        GtkMainLoop.startGTK()

    def quit(self, *args):
        gtk.main_quit()


class PyGTKDialog(PyGTKWidget):
    """Instantiate the Glade window and starts the dialog box."""

    def run(self):
      result = self.widget.run()
      self.widget.hide()
      return result
