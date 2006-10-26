import matplotlib
from matplotlib.font_manager import FontProperties

FIGURE_BG  = None        # the figure background color
AXES_BG    = None        # the axes background color
GRID_COL   = None        # the axes background color
FONTSIZE   = None        # the default font size (125% for title)
FONTPROP   = None        # the default font properties

LEGEND_FONTSIZE = None
LEGEND_FONTPROP = None

TICK_LABEL_FONTSIZE = None
TICK_LABEL_FONTPROP = None

def triggerLightGraySetup():
    global FIGURE_BG, AXES_BG, GRID_COL, FONTSIZE, FONTPROP
    FIGURE_BG  = 'w'        
    AXES_BG    = '#E9E9E9'  
    GRID_COL   = '#AAAAAA'  
    FONTSIZE   = 16
    FONTPROP   = FontProperties(family='sans-serif', weight='normal', size=FONTSIZE)

    global LEGEND_FONTSIZE, LEGEND_FONTPROP
    LEGEND_FONTSIZE = 14
    LEGEND_FONTPROP = FontProperties(family='sans-serif',
                                     weight='normal', size=LEGEND_FONTSIZE)    

    global TICK_LABEL_FONTSIZE, TICK_LABEL_FONTPROP
    TICK_LABEL_FONTSIZE = 14
    TICK_LABEL_FONTPROP = FontProperties(family='sans-serif',
                                         weight='normal', size=TICK_LABEL_FONTSIZE)        
    matplotlib.rc('text',           usetex=True)
    matplotlib.rc('tick',           color='k')
    matplotlib.rc('figure',         facecolor=FIGURE_BG)
    matplotlib.rc('figure.subplot', bottom=0.15)
    matplotlib.rc('lines',          linewidth=1.5, linestyle='-')
    matplotlib.rc('grid',           color=GRID_COL, linewidth=0.5, linestyle='-')

    matplotlib.rc('font', family='sans-serif', weight='normal', size=FONTSIZE)
    matplotlib.rc('axes',
        labelsize=FONTSIZE, titlesize=int(1.25*FONTSIZE), facecolor=AXES_BG, grid=False)
        
    
# The default setup is the 'LightGraySetup' which is triggered whenever a
# module from this package is imported
triggerLightGraySetup()
