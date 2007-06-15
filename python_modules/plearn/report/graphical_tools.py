# graphical_tools.py
# Copyright (C) 2006 Christian Dorion
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

# Author: Christian Dorion
import pylab, os
from matplotlib.font_manager import FontProperties
from plearn.report import GRID_COL, FONTSIZE, LEGEND_FONTPROP, TICK_LABEL_FONTPROP

LEFT,   WIDTH  = 0.125, 0.800
BOTTOM, HEIGHT = 0.100, 0.800
LINE_COLORS = [ '#660033', 'b', 'r', 'k', "#CDBE70",
                "#FF8C69", "#65754D", "#4d6575", "#754d65" ]

STYLELIST = [
    'b-',  'g-',  'r-',  'c-',  'm-',  'k-',  'y-',
    'b--', 'g--', 'r--', 'c--', 'm--', 'k--', 'y--',
    'b:',  'g:',  'r:',  'c:',  'm:',  'k:',  'y:',
    'b-.', 'g-.', 'r-.', 'c-.', 'm-.', 'k-.', 'y-.' ] * 5

_figure_counter = 0
def getNewFigure(figsize=(12,10)):
    global _figure_counter
    _figure_counter += 1
    return pylab.figure(_figure_counter, figsize=figsize)

def getBounds(frame):
    return [ frame.get_x(), frame.get_y(), frame.get_width(), frame.get_height() ]

def getWideRect(bottom, height):
    return [ LEFT, bottom, WIDTH, height ]

def plotZeroLine(axes, color='#666666'):
    axes.axhline(y=0, color=color)

def same_xlim(*ax_list):
    m, M = float('inf'), -float('inf')
    for axes in ax_list:
        xlim = axes.get_xlim()
        m, M = min(m, xlim[0]), max(M, xlim[1]), 

    for axes in ax_list:
        axes.set_xlim(m, M)
    return m, M

def same_ylim(*ax_list, **kwargs):
    m, M = float('inf'), -float('inf')
    for axes in ax_list:
        ylim = axes.get_ylim()
        m, M = min(m, ylim[0]), max(M, ylim[1]), 

    if 'ymin' in kwargs:
        m = max(m, kwargs.pop('ymin'))
    if 'ymax' in kwargs:
        M = min(M, kwargs.pop('ymax'))

    for axes in ax_list:
        axes.set_ylim(m, M)

    assert len(kwargs)==0, "Unexpected keyword arguments: %s"%repr(kwargs.keys())
    return m, M

def setLegend(axes, legend_map, sorted_keys=None, loc=0):
    if not sorted_keys:
        sorted_keys = legend_map.keys(); sorted_keys.sort()
    values = [ legend_map[k] for k in sorted_keys ]
    legend = axes.legend(values, sorted_keys,
                         loc=loc, shadow=False, prop = FontProperties(size=13))
    legend.set_zorder(100)

class Struct(dict):
    def __init__(self, **members):
        dict.__init__(self, members)

    def __getattr__(self, key):
        return self[key]

    def __setattr__(self, key, val):
        self[key] = val

class AxisLimits:
    def __init__(self):
        self.min = +1e09
        self.max = -1e09

    def update(self, *limits):
        self.min = min(limits[0], self.min)
        self.max = max(limits[1], self.max)    

class FigureWrapper(object):
    FIGSIZE = (12,10)
    instances = []
    
    def __init__(self, figsize=None):
        if figsize is None: figsize = self.FIGSIZE
        self.figure = getNewFigure(figsize)
        self.figno  = _figure_counter  
        self.instances.append(self)

    def addAxes(self, rect, *args, **kwargs):
        axes = self.figure.add_axes(rect, *args, **kwargs)
        axes.getRectangle = lambda : rect
        return axes

    def gca(self):
        return self.figure.gca()

    def publish(self, path=""):
        if path:
            if path.endswith('.pdf'):
                eps_path = path.replace('.pdf','.eps')
                self.figure.savefig(eps_path, dpi=600)
                os.system("epstopdf %s; rm -f %s"%(eps_path,eps_path))
            else:
                self.figure.savefig(path, dpi=600)

        fp = TICK_LABEL_FONTPROP
        for axes in self.figure.get_axes():
            for label in axes.get_xticklabels():
                label.set_fontproperties(fp)
            for label in axes.get_yticklabels():
                label.set_fontproperties(fp)            

    def publishAll(FigureWrapper, ext='pdf', fno_start=1):
        for fno, figure in enumerate(FigureWrapper.instances):
            figure.publish('figure%d.%s'%(fno_start+fno,ext))
        FigureWrapper.instances = []
    publishAll = classmethod(publishAll)
    
class TwoFramesFigure(FigureWrapper):
    def __init__(self,
                 urect = [LEFT, 0.525, WIDTH, 0.375],
                 lrect = [LEFT, 0.100, WIDTH, 0.375]):
        super(TwoFramesFigure,self).__init__()

        self.urect = urect
        self.upperAxes = self.figure.add_axes(urect)
        #print self.upperAxes.get_frame()

        self.lrect = lrect
        self.lowerAxes = self.figure.add_axes(lrect)
        #print self.lowerAxes.get_frame()

#3 frames: impact          = self.addAxes(getWideRect(0.075, 0.250))
#3 frames: positive_impact = self.addAxes(getWideRect(0.375, 0.250))
#3 frames: negative_impact = self.addAxes(getWideRect(0.675, 0.250))

class TextWriter:
    """Writes text in the axes.
    
        >> offset_y = 0.025
        >> writer = TextWriter(impact_axes, 0.010, 0.925, offset_y=offset_y)
        >> writer("months to exp: %d" % roll_month,
        >>        "business roll day: %d" % roll_day )
    """
    def __init__(self, axes, x_start=0.0, y_start=0.0,
                 offset_x=0.0, offset_y=0.075, font_size=None):
        self.axes      = axes
        self.cur_x     = x_start
        self.cur_y     = y_start
        self.offset_x  = offset_x
        self.offset_y  = offset_y

        self.font_size = font_size
        if font_size is None:
            self.font_size = FONTSIZE
        
    def __call__(self, *texts, **kwargs):
        """Writes text in axes.
        Keyword arguments:
          - offset_x (default: self.offset_x)
          - offset_y (default: self.offset_y)
          - color    (default: '#660000') 
        """
        offset_x = kwargs.get("offset_x", self.offset_x)
        offset_y = kwargs.get("offset_y", self.offset_y)
        color    = kwargs.get("color", "#660000")

        writer = lambda text: \
            self.axes.text(self.cur_x, self.cur_y, text, zorder=1, color=color, 
                           fontsize=self.font_size, transform=self.axes.transAxes)


        for text in texts[:-1]:
            writer(text)
            self.cur_y -= 0.001*(self.font_size+3)

        text = ""
        if texts:
            text = texts[-1]
        writer(text)
        self.cur_x += offset_x
        self.cur_y -= offset_y

