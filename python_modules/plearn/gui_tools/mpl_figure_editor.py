#!/usr/bin/python

"""Provide a traits-friendly container for Matplotlib figures.

Code from Gael Varoquaux, modifications by Nicolas Chapados
"""

import wx
import matplotlib
matplotlib.use('WXAgg')          # We want matplotlib to use a wxPython backend

from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigureCanvas
from matplotlib.figure import Figure
from matplotlib.backends.backend_wx import NavigationToolbar2Wx

from enthought.traits.api          import Any, HasTraits, Instance
from enthought.traits.ui.api       import Item, View
from enthought.traits.ui.wx.editor import Editor
from enthought.traits.ui.wx.basic_editor_factory import BasicEditorFactory

class _MPLFigureEditor(Editor):
    """Traits editor that contains the actual Matplotlib canvas for drawing.
    """

    scrollable = True

    def init(self, parent):
        self.control = self._create_canvas(parent)
        self.set_tooltip()
        
    def update_editor(self):
        pass

    def _create_canvas(self, parent):
        """ Create the MPL canvas. """
        # The panel lets us add additional controls.
        panel = wx.Panel(parent, -1, style=wx.CLIP_CHILDREN)
        sizer = wx.BoxSizer(wx.VERTICAL)
        panel.SetSizer(sizer)
        # matplotlib commands to create a canvas
        mpl_control = FigureCanvas(panel, -1, self.value)
        sizer.Add(mpl_control, 1, wx.LEFT | wx.TOP | wx.GROW)
        toolbar = NavigationToolbar2Wx(mpl_control)
        sizer.Add(toolbar, 0, wx.EXPAND)
        self.value.canvas.SetMinSize((50,50))
        return panel


class MPLFigureEditor(BasicEditorFactory):

    klass = _MPLFigureEditor


class TraitedFigure( HasTraits ):
    """Traited object acting as a container for a single figure.
    """
    figure  = Instance(Figure, editor=MPLFigureEditor())
    __title = Str

    def __init__(self, title = "Figure", **kwargs):
        """Instantiate a TraitedFigure.

        Apart from the title, the other **kwargs are passed to the
        matplotlib Figure() constructor.
        """
        super(TraitedFigure,self).__init__()
        self.__title = title
        self.figure  = Figure(**kwargs)
    
    traits_view = View(Item('figure', show_label=False))



#####  Test Case  ###########################################################

if __name__ == "__main__":
    # Create a window to demo the editor
    from enthought.traits.api import HasTraits
    from enthought.traits.ui.api import View, Item
    from numpy import sin, cos, linspace, pi

    class Test(HasTraits):

        figure = Instance(Figure, ())

        view = View(Item('figure', editor=MPLFigureEditor(),
                                show_label=False),
                        width=600,
                        height=450,
                        resizable=True)

        def __init__(self):
            super(Test, self).__init__()
            axes = self.figure.add_subplot(111)
            t = linspace(0, 2*pi, 200)
            axes.plot(sin(t)*(1+0.5*cos(11*t)), cos(t)*(1+0.5*cos(11*t)))

    Test().configure_traits()
