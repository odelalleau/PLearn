# report/__init__.py
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
    FONTSIZE   = 12
    FONTPROP   = FontProperties(family='sans-serif', weight='normal', size=FONTSIZE)

    global LEGEND_FONTSIZE, LEGEND_FONTPROP
    LEGEND_FONTSIZE = 24
    LEGEND_FONTPROP = FontProperties(family='sans-serif',
                                     weight='normal', size=LEGEND_FONTSIZE)    

    #global TICK_LABEL_FONTSIZE, TICK_LABEL_FONTPROP
    #TICK_LABEL_FONTSIZE = 10
    #TICK_LABEL_FONTPROP = FontProperties(family='sans-serif',
    #                                     weight='normal', size=TICK_LABEL_FONTSIZE)
    matplotlib.rc('text',   usetex=True)
    matplotlib.rc('legend', fontsize=14)
    matplotlib.rc('xtick',  color='k', labelsize=14)
    matplotlib.rc('ytick',  color='k', labelsize=14)
    
    matplotlib.rc('figure',         facecolor=FIGURE_BG)
    matplotlib.rc('figure.subplot', bottom=0.10)
    matplotlib.rc('lines',          linewidth=1.5, linestyle='-')
    matplotlib.rc('grid',           color=GRID_COL, linewidth=0.5, linestyle='-')

    matplotlib.rc('font', family='sans-serif', weight='normal', size=FONTSIZE)
    matplotlib.rc('axes',
        labelsize=FONTSIZE, titlesize=int(1.25*FONTSIZE), facecolor=AXES_BG, grid=False)
        
    
# The default setup is the 'LightGraySetup' which is triggered whenever a
# module from this package is imported
triggerLightGraySetup()
