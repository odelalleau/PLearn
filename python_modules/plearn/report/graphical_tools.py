import pylab, os
from plearn.report import GRID_COL, FONTSIZE, LEGEND_FONTPROP, TICK_LABEL_FONTPROP

LEFT, WIDTH = 0.125, 0.8
LINE_COLORS = [ '#660033', 'b', 'r', 'k', "#CDBE70",
                "#FF8C69", "#65754D", "#4d6575", "#754d65" ]

__figure_counter = 0
def getNewFigure():
    global __figure_counter
    __figure_counter += 1
    return pylab.figure(__figure_counter, figsize=(12,10))

def getBounds(frame):
    return [ frame.get_x(), frame.get_y(), frame.get_width(), frame.get_height() ]

def getWideRect(bottom, height):
    return [ LEFT, bottom, WIDTH, height ]

def setLegend(axes, legend_map, sorted_keys=None, loc=0):
    if not sorted_keys:
        sorted_keys = legend_map.keys(); sorted_keys.sort()
    values = [ legend_map[k] for k in sorted_keys ]
    legend = axes.legend(values, sorted_keys, loc=loc, shadow=True)
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
    def __init__(self):
        self.figure = getNewFigure()

    def addAxes(self, rect, *args, **kwargs):
        axes = self.figure.add_axes(rect, *args, **kwargs)
        axes.getRectangle = lambda : rect
        return axes

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

class TwoFramesFigure(FigureWrapper):
    def __init__(self,
                 urect = [LEFT, 0.525, WIDTH, 0.375],
                 lrect = [LEFT, 0.100, WIDTH, 0.375]):
        super(TwoFramesFigure,self).__init__()

        self.urect = urect
        self.upperAxes = self.figure.add_axes(urect)
        print self.upperAxes.get_frame()

        self.lrect = lrect
        self.lowerAxes = self.figure.add_axes(lrect)
        print self.lowerAxes.get_frame()

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
                 offset_x=0.0, offset_y=0.075, font_size=FONTSIZE):
        self.axes      = axes
        self.cur_x     = x_start
        self.cur_y     = y_start
        self.offset_x  = offset_x
        self.offset_y  = offset_y
        self.font_size = font_size
        
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

