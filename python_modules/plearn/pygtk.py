"""This module contains utility functions to be used with pygtk"""

import pygtk
# pygtk.require('2.0')
import gtk
import gtk.glade

import re

# Regular expression to match handler method names patterns
# on_widget__signal and after_widget__signal.  Note that we use two
# underscores between the Glade widget name and the signal name.
handler_re = re.compile(r'^(on|after)_(.*)__(.*)$')

def glade_load_and_autoconnect(myobj, glade_file, widget_name=''):
  """Loads and constructs the specified widget from the specified glade file,
  and connects signal handling methods of myobj to the widgets.
  Methods named like on_widget__signal or after_widget__signal
  are connected to the appropriate widgets and signals.
  The call returns the glade_object, so that you may call glade_object.get_widget(widgetname)
  if you need to get the individual widgets.
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
        print 'Connecting signal',signal,'of widget',widget
        get_widget(widget).connect(signal, method)
      elif when == 'after':
        get_widget(widget).connect_after(signal, method)
      elif attr.startswith('on_') or attr.startswith('after_'):
        # Warn about some possible typos like separating
        # widget and signal name with _ instead of __.
        print ('Warning: attribute %r not connected'
               ' as a signal handler' % (attr,))
  return gladeobj


