from pylab import *
from numpy.numarray import *


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
    cbarh = arange(min, max,  (max-min)/50.)
    cbar = vecToVerticalMatrix(cbarh)
    cbarh_str = []
    for el in cbarh:
        cbarh_str.append(str(el)[0:5])
    yticks(arange(len(cbarh)),cbarh_str)
    xticks([],[])
    imshow(cbar, cmap = color_map, vmin=min, vmax=max)




def plotLayer1Old(W, M, width):
    '''plots each row of W and M as if they where matrix (width is the width of one of these matrix)'''

    #some calculations for plotting

    nbPlotStyles = 3 # (normal and two times 1 sur 2)
    subPlotHeight = nbPlotStyles*2
    subPlotWidth = max(len(W), len(M))
    axesHeight = 1./float(subPlotHeight)
    axesWidth = 1./float(subPlotWidth)

    colorMap = defaultColorMap
    
    matrices = [W,M]
    names = ["W", "M"]

    #THE plotting

    for i, matrix in enumerate(matrices):
        for j, row in enumerate(matrix):
            
            #normal
        
            #subplot(subPlotHeight, subPlotWidth,(i%2)*subPlotWidth + j + 1)
            axes((j*axesWidth, i*axesHeight, axesWidth, axesHeight))
            imshow(rowToMatrix(row, width), interpolation="nearest", cmap = colorMap)
            setPlotParams(names[i%2] + "_" + str(i) + "_" + str(j), False, True)
            
            #1 sur 2 -
        
            #subplot(subPlotHeight, subPlotWidth, (2+i%2)*subPlotWidth + j + 1)
            axes((j*axesWidth, (i+2)*axesHeight, axesWidth, axesHeight))
            imshow(rowToMatrix(toMinusRow(row),width), interpolation="nearest", cmap = colorMap)
            setPlotParams(names[i%2] + "_" + str(i) + "_" + str(j), False, True)
            
            #1 sur 2 +

            #subplot(subPlotHeight, subPlotWidth, (4+i%2)*subPlotWidth + j + 1)
            axes((j*axesWidth, (i+4)*axesHeight, axesWidth, axesHeight))
            imshow(rowToMatrix(toPlusRow(row),width), interpolation="nearest", cmap = colorMap)
            setPlotParams(names[i%2] + "_" + str(i) + "_" + str(j), False, True)

def plotLayer1(M, width, plotWidth=.1, start=0, length=-1, space_between_images=.01, apply_to_rows = None, index_to_plot = [], names = []):

    
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

    colorMap = defaultColorMap

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
    print index_to_plot

    j=0
    for i in index_to_plot:
           
        #normal
        row = M[i]
        if apply_to_rows != None:
            row = apply_to_rows(row)
        

       # print x,y,plotWidth, plotHeight
        
        axes((x, y, plotWidth, plotHeight))
        imshow(rowToMatrix(row, width), interpolation="nearest", cmap = colorMap, vmin = mi, vmax = ma)
        if names == []:
            setPlotParams('row_' + str(i), False, True)
        else:
            setPlotParams(names[j],False,True)
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
    customColorBar(mi,ma,(1.-cbw-sbi, sbi, sbi, 1.-2.*cbw))
    return toReturn




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

def truncateMatrixOld(mat, maxHeight=10, maxWidth=10):
    
    width = len(mat[0])
    height = len(mat)

    #si notre matrice est trop haute (seulement), on va la couper en morceaux
    if width > maxWidth and height <= maxHeight:        
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
    elif height > maxHeight and width <= maxWidth:
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
    elif height > maxHeight and width > maxWidth:
        print 'matrice trop grosse... rien a faire..'
        truncMat = [mat]
    else:
        truncMat = [mat]

    return truncMat

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

      
        
    
