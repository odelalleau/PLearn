# netplot.py
# Copyright (C) 2007-2008 Pascal Vincent
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

# Author: Pascal Vincent


from pylab import *
from math import *
from numpy import *
#from numpy.numarray import *


#################
### constants ###
#################

defaultColorMap = cm.gray#cm.jet
defaultInterpolation = 'nearest'

########################
### plotting methods ###
########################

def setPlotParams(titre='', color_bar=True, disable_ticks=False):
    title(titre)
    if color_bar:
        colorbar()
    if disable_ticks:
        xticks([],[])
        yticks([],[])

def findMinMax(matrix):
    M = matrix
    mi = M[0][0]
    ma = M[0][0]

    for i in arange(M.shape[0]):
        for j in arange(M.shape[1]):
            if M[i,j] > ma:
                ma = M[i,j]
            if M[i,j] < mi:
                mi = M[i,j]
    
    return mi,ma

def customColorBar(min, max, (x,y,width,height) = (0.9,0.1,0.1,0.8), nb_ticks = 50., color_map = defaultColorMap):
    axes((x, y, width,height))
    if(min == max):
        max=min + 1e-6
    cbarh = arange(min, max,  (max-min)/nb_ticks)
    cbar = vecToVerticalMatrix(cbarh)
    cbarh_str = []
    for el in cbarh:
        cbarh_str.append(str(el)[0:5])
    yticks(arange(len(cbarh)),cbarh_str)
    xticks([],[])
    imshow(cbar, cmap = color_map, vmin=min, vmax=max)

def formatFloat(float):
    return "%.2e" % float


def plotLayer1(M, width, plotWidth=.1, start=0, length=-1, space_between_images=.02, apply_to_rows = None, index_to_plot = [], names = [], same_scale = False, colormap = defaultColorMap):

    
    #some calculations for plotting

    #hack
    if length == -1:
        length = len(M)

    mWidth = float(len(M[0]))
    mHeight = float(len(M))
    sbi = space_between_images
    #plotHeight = mHeight/mWidth*plotWidth
    plotHeight = mWidth/width/width*plotWidth
    cbw = .01 # color bar width

    if same_scale:
        mi,ma = findMinMax(M)
        print 'min', mi
        print 'max', ma
    
        ma = max(abs(mi),abs(ma))
        mi = -ma
            

    #THE plotting
    
    x,y = sbi,sbi
    toReturn = -1

    if index_to_plot == []:
        index_to_plot = arange(start,start+length)

    j=0
    for i in index_to_plot:
           
        #normal        
        row = M[i]
        if apply_to_rows != None:
            row = apply_to_rows(row)
        

       # print x,y,plotWidth, plotHeight
        
        axes((x, y, plotWidth, plotHeight))
        if same_scale:
            imshow(rowToMatrix(row, width), interpolation="nearest", cmap = colormap, vmin = mi, vmax = ma)
        else:
            imshow(rowToMatrix(row, width), interpolation="nearest", cmap = colormap)#, vmin = mi, vmax = ma)
            
        if names == []:
            setPlotParams('row_' + str(i), 1-same_scale, True)
        else:
            setPlotParams(names[j], 1-same_scale, True)
        j+=1
        

        x = x + plotWidth + sbi
        if x + plotWidth +cbw > 1:
            x = sbi
            y = y + plotHeight + sbi
        if y + plotHeight > 1:
            # images that follows would be out of the figure...
            toReturn = i
            break

    #custom color bar
   # customColorBar(mi,ma,(1.-cbw-sbi, sbi, sbi, 1.-2.*cbw))
    return toReturn


def plotRowsAsImages(X, 
                     img_height, img_width,
                     nrows=10, ncols=20,
                     figtitle="",
                     show_colorbar=False, disable_ticks=True, colormap = cm.gray,
                     luminance_scale_mode = 0, vmin=None, vmax=None,
                     transpose_img=False):
    """
    Will plot rows of X in a nrows x ncols grid.
    The first img_height x img_width elements of each roe are interpreted as
    greyscale values of a img_height x img_width image.
    If provided, vmin and vmax will be used for luminance scale (see imshow)
    If not povided, they will be set depending on luminance_scale_mode:
       0: vmin and vmax are left None, i.e. luminance
          will be scaled independently for each image
       1: vmin and vmax will be set to min,max of X
       2: vmin and vmax will be set to +-min of X
          or +-max of X (whichever is bigger).
    """

    inputs = array(X[0:min(len(X),nrows*ncols)])
    if len(inputs[0])>img_height*img_width:
        inputs = inputs[:,0:(img_height*img_width)]
        
        print 'luminance_scale_mode = ',luminance_scale_mode
    if vmin is None and luminance_scale_mode!=0:
        vmin = inputs.min()
        vmax = inputs.max()
        print 'filter value range: ',vmin,',',vmax
        if luminance_scale_mode==2:
            vmax = max(abs(vmin),abs(vmax))
            vmin = -vmax
        print 'used luminance scale: ',vmin,',',vmax            

    #THE plotting

    subplots_adjust(left=0.1, right=0.9, bottom=0.1, top=0.9,
                    wspace=0.01, hspace=0.01)
    
    for i in range(min(len(inputs),nrows*ncols)):
        row = inputs[i]
        subplot(nrows,ncols,i+1)
        # print "Reshaping: ",row.shape,"->",img_height,'x',img_width
        img = reshape(row,(img_height,img_width))
        if transpose_img:
            img = transpose(img)        
        imshow(img, interpolation="nearest", cmap = colormap, vmin = vmin, vmax = vmax)
        if show_colorbar and vmin is None:
            colorbar()
            
        # if show_colorbar and vmin is None:
        #    colorbar()
        if disable_ticks:
            xticks([],[])
            yticks([],[])

    if figtitle!="":
        figtext(0.5, 0.95, figtitle,
                horizontalalignment='center',
                verticalalignment='bottom')

    if show_colorbar and vmin is not None:
        cbw = .01 # color bar width
        customColorBar(vmin,vmax,color_map=colormap)

                   


def plotMatrices(matrices, names = None, ticks = False, same_color_bar = False, space_between_matrices = 5, horizontal=True):
    '''plot matrices from left to right
    TODO : same_color_bar does nothing !!
    '''
    
    colorMap = cm.gray
    nbMatrices = len(matrices)
    print 'plotting ' + str(nbMatrices) + ' matrices'

    totalWidth = 0
    maxHeight = 0
    
    for matrix in matrices:
        #print matrix.info()
        if matrix.shape[0] > maxHeight:
            maxHeight = matrix.shape[0]
        totalWidth += matrix.shape[1]
    print maxHeight, totalWidth
    

    #to prevent a little bug   
    space_between_matrices = min(space_between_matrices, maxHeight, totalWidth)

    unit = min(1./((nbMatrices+1)*space_between_matrices + totalWidth), 1./(maxHeight+2.*space_between_matrices))
    sbm = space_between_matrices*unit


    x=sbm
    the_axes = []
    
    if names != None:
        if len(names) != len(matrices):
            raise Exception, "nb of matrices and nb of names must be equals in plotMatrices()"
    else:
        names = ['']*len(matrices)
        
    for matrix,name in zip(matrices,names):
        
        h = matrix.shape[0]*unit
        w = matrix.shape[1]*unit

        if h>1 :
            h = 1.-2*sbm
        bottom = (1.-h)/2.

        #print x,bottom, w, h
        temp = axes(( x,bottom, w,h))
        the_axes.append(temp)
        imshow(matrix, interpolation = 'nearest', cmap = colorMap)
        title(name)       
        if ticks == False:
            xticks([],[])
            yticks([],[])
        colorbar()
        x += w+sbm
    return the_axes



def truncate_imshow(mat, max_height_or_width = 200, width_height_ratio = 1, space_between_submatrices=5):

    matWidth = float(len(mat[0]))
    matHeight = float(len(mat))
    
    mhow = max_height_or_width

    s = float(space_between_submatrices)
    r = float(width_height_ratio)
        
    if (matWidth > mhow and matHeight < mhow) or (matWidth < mhow and matHeight > mhow):
        
        if matWidth > matHeight:
            n = (s + sqrt(s*s + 4.*(matHeight+s)*matWidth/r))/(2.*(matHeight+s))
        else :
            n = (s + sqrt(s*s + 4.*(matWidth+s)*matHeight/r))/(2.*(matWidth_s))        
        newMat = truncateMatrix(mat, n)
    else:
        n=1.
        newMat = [mat]
    
    
    height = len(newMat[0])
    width = len(newMat[0][0])
    
    #on met les submatrices de bas en haut
    if width>height :
        
        #somes plotting constants
        unit = min(1./(n*height + (n+1)*s), 1./(width+2*s))
        plotHeight = height*unit
        plotWidth = width*unit
        sbs = s*unit        
        x,y=sbs,sbs
    
        for i,matrix in enumerate(newMat):
            axes((x,y,plotWidth, plotHeight))
            imshow(matrix, interpolation = defaultInterpolation, cmap = defaultColorMap)
            setPlotParams("",False,True)
            y = y + plotHeight + sbs
    
    #on met les matrices de gauche a droite     
    else :

        
        #somes plotting constants
        unit = min(1./(n*width + (n+1)*s), 1./(height+2*s))
        plotHeight = height*unit
        plotWidth = width*unit
        sbs = s*unit
        x,y=sbs,sbs
    
        for i,matrix in enumerate(newMat):
            axes((x,y,plotWidth, plotHeight))            
            imshow(matrix, interpolation = defaultInterpolation, cmap = defaultColorMap)
            setPlotParams("",False,True)
            x = x + plotWidth + sbs

    #custom colorBar
    mi,ma = findMinMax(mat)
    print 'min', mi
    print 'max', ma
    
    ma = max(abs(mi),abs(ma))
    mi = -ma
    customColorBar(mi,ma)
    


################################################
### somes methods for matrix transformations ###
################################################

def toPlusRow(row):
    '''[1,2,3,4,5,6]->[2,4,6]
    '''
    row2 = []
    for i in arange(1,len(row),2):
        row2.append(row[i])
    return row2

def toMinusRow(row):
    '''[1,2,3,4,5,6]->[1,3,5]
    '''
    row2 = []
        
    for i in arange(0,len(row),2):
        row2.append(row[i])
    return row2

def evenMinusOdd(row):
    return row[0::2]-row[1::2]

def softmaxGroup2(row):
    er = exp(row)
    return er[0::2]/(er[0::2]+er[1::2])
    # return er[1::2]/(er[0::2]+er[1::2])

def doubleSizedWeightVectorToImageMatrix(row):
    # v = evenMinusOdd(row)
    v = softmaxGroup2(row)
    d = sqrt(len(v))
    print "Built %d x %d matrix" % (d,d)
    m = reshape(v, (-1,d))
    return m


def rowToMatrixOld(row, width, validate_size = True, fill_value = 0.):
    '''change a row [1,2,3,4,5,6] into a matrix [[1,2],[3,4],[5,6]] if width = 2
       or [1,2,3,4,5,6] -> [[1,2,3,4],[5,6,fill_value,fill_value]] if width = 4 and validate_size = False
    '''
    if len(row)%width != 0 and validate_size:
        raise Exception, "dimensions does not fit ( " + str(width) + " does not divide " + str(len(row)) + ")"
            
    m = []
    k = 0
    a=-1
    for i,e in enumerate(row):        
        if(i%width == 0):        
            a=a+1
            m.append([])
        m[a].append(e)

    # we finish by filling fill the last elements of the last row
    if len(m)>=2:
        for i in arange(len(m[-1]),len(m[-2])):
            m[-1].append(fill_value)
    
    return m

def rowToMatrix(row, width, validate_size = True, fill_value = 0.):
    '''change a row [1,2,3,4,5,6] into a matrix [[1,2],[3,4],[5,6]] if width = 2
       or [1,2,3,4,5,6] -> [[1,2,3,4],[5,6,fill_value,fill_value]] if width = 4 and validate_size = False
    '''
    copy = list(row)
    if len(copy)%width != 0:
        if validate_size:
            raise Exception, "dimensions does not fit ( " + str(width) + " does not divide " + str(len(copy)) + ")"
        for i in arange(len(copy)%width-1):
            copy.append(fill_value)

    return reshape(copy, (-1,width))


def vecToVerticalMatrix(vec):
    '''ex : [1,2,3,4,5] --> [[1],[2],[3],[4],[5]]
    '''
    return reshape(array(vec), (len(vec),-1))


def truncateMatrix(mat, n=10.):
        
    width = len(mat[0])
    height = len(mat)

    #si notre matrice est trop haute (seulement), on va la couper en morceaux
    if width > height:
        maxWidth = int(width/n)
        truncMat = []
        a = -1
        for indCol in arange(len(mat[0])):
            if indCol % maxWidth == 0:               
                truncMat.append([])
                a=a+1
                for l in arange(height):#on ajoute des lignes
                    truncMat[a].append([])                
            for l in arange(height):#on remplie les lignes
                truncMat[a][l].append(mat[l][indCol])            
            
        return truncMat
    #si notre matrice est trop large   (seulement)    
    else: # height >= width
        maxHeight = int(height/n)
        truncMat = [[]]
        a,l=0,0
        for ligne in mat:
            if(l >= maxHeight):
                truncMat.append([])
                a=a+1
                l=0
            truncMat[a].append(ligne)
            l=l+1
            
    return truncMat


def saveRowsAsImage(imgfile,
                    matrix, img_width, img_height,
                    nrows=10, ncols=20,
                    figtitle="",
                    luminance_scale_mode=0,
                    colormap = cm.gray,
                    show_colorbar = False,
                    vmin = None,
                    vmax = None,
                    transpose_img = False):
    """Saves filters contained in first few rows of matrix as an image file.
    imgfile specifies the name of the image file,
    img_width and img_height specify the sizes of the imagettes (not the size of the big saved image).
    Other parameters are passed directly to plorRowsAsImages
    """
    clf()
    endidx = min(nrows*ncols, len(matrix))        
    plotRowsAsImages(matrix[0:endidx],
                     img_height = img_height,
                     img_width = img_width,
                     nrows = nrows,
                     ncols = ncols,
                     figtitle = figtitle,
                     luminance_scale_mode = luminance_scale_mode,
                     show_colorbar = show_colorbar,
                     disable_ticks = True,
                     colormap = colormap,
                     vmin = vmin,
                     vmax = vmax,
                     transpose_img = transpose_img)
    savefig(imgfile)
      
class showRowsAsImages:

    def __init__(self, X, 
                 img_height,
                 img_width,
                 nrows = 10, ncols = 20,
                 startidx = 0,
                 figtitle="",
                 luminance_scale_mode=0,
                 colormaps = [cm.gray, cm.jet],
                 vmin = None, vmax = None,
                 transpose_img=False):

        self.X = X
        self.img_height = img_height
        self.img_width = img_width
        self.nrows = nrows
        self.ncols = ncols
        self.figtitle = figtitle
        self.startidx = startidx

        # appearance control
        self.luminance_scale_mode = luminance_scale_mode
        self.interpolation = 'nearest'
        self.colormaps = colormaps
        self.cmapchoice = 0
        self.show_colorbar = True
        self.disable_ticks = True
        self.vmin = vmin
        self.vmax = vmax
        self.transpose_img = transpose_img

        # plot it
        self.draw()      
        connect('key_press_event', self.keyPressed)
        # connect('button_press_event', self.__clicked)

        # start interactive loop
        show()

    def draw(self):
        # print "Start plotting..."
        clf()
        endidx = min(self.startidx+self.nrows*self.ncols, len(self.X))        
        title = self.figtitle+" ("+str(self.startidx)+" ... "+str(endidx-1)+")"
        plotRowsAsImages(self.X[self.startidx : endidx],
                         img_height = self.img_height,
                         img_width = self.img_width,
                         nrows = self.nrows,
                         ncols = self.ncols,
                         figtitle = title,
                         luminance_scale_mode = self.luminance_scale_mode,
                         show_colorbar = self.show_colorbar,
                         disable_ticks = self.disable_ticks,
                         colormap = self.colormaps[self.cmapchoice],
                         vmin = self.vmin,
                         vmax = self.vmax,
                         transpose_img = self.transpose_img
                         )
        # print "Plotted,"
        draw()
        # print "Drawn."
        

    def plotNext(self):
        self.startidx += self.nrows*self.ncols
        if self.startidx >= len(self.X):
            self.startidx = 0
        self.draw()

    def plotPrev(self):
        if self.startidx>0:
            self.startidx -= self.nrows*self.ncols
        else:
            self.startidx = len(self.X)-self.nrows*self.ncols
        if self.startidx<0:
            self.startidx = 0            
        self.draw()

    def keyPressed(self, event):
        char = event.key
        print 'Pressed',char
        if char == 'c':
            self.changeColorMap()
        elif char == 'right':
            self.plotNext()
        elif char == 'left':
            self.plotPrev()
        elif char == 'b':
            self.show_colorbar = not self.show_colorbar
            self.draw()
        elif char == 't':
            self.transpose_img = not self.transpose_img
            self.draw()
        elif char == 'i':
            self.disable_ticks = not self.disable_ticks
            self.draw()
        elif char == 's':
            self.luminance_scale_mode = (self.luminance_scale_mode+1)%3
            self.draw()
        elif char == '':
            pass
        else:
            print """
            *******************************************************
            * KEYS
            *  right : show next filters
            *  left  : show previous filters
            *  t     : tranpose images
            *  c     : change colormap
            *  s     : cycle through luminance scale mode
            *          0 independent luminance scaling for each
            *          1 min-max luminance scaling across display
            *          2 +-min or +- max (largest range)
            *  b     : toggle showing colorbar 
            *  i     : toggle showing ticks
            *
            * Close window to stop.
            *******************************************************
            """

    def changeColorMap(self):
        self.cmapchoice = (self.cmapchoice+1)%len(self.colormaps)
        self.draw()
