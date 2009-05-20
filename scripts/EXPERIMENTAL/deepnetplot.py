#!/usr/bin/env python

import sys
import matplotlib.pyplot as plt

from pylab import *
from plearn.io.server import *
from plearn.pyplearn import *
from plearn.plotting.netplot import *
from numpy.numarray import *
# from numpy.numarray import *
#from numarray.random_array import *
import numpy.random


################
### methods ###
################

def myion():
    pass

def myioff():
    pass

def print_usage_and_exit():
    print "Usage :"  
    print "deepnetplot.py plotSingleMatrix x.psave "
    print "deepnetplot.py plotEachRow learner.psave chars.pmat"
    print "deepnetplot.py plotRepAndRec learner.psave chars.pmat"
    print "deepnetplot.py help"
    print ""
    print "where learner.psave is a .psave of a DeepReconstructorNet object"
    print "and chars.pmat are the training examples"
    print ""
    print "exemple:"
    print ""
    print "deepnetplot.py plotRepAndRec multimax_tied_small_gs1\=4_ng1\=225/Strat0/Trials0/Split0/final_learner.psave /cluster/pauli/data/mnist/mnist_small/mnist_basic2_train.pmat"
    sys.exit()

def print_usage_repAndRec():
    print 'putting you mouse OVER a hidden layer and hitting these keyes does... things :'
    print
    print 'f : fproping from the last layer'
    print 'F : fproping form the last layer and for the next ones'
    print 'r : reconstructing this layer from the next one'
    print 'R : reconstructing this layer from the next one and also the previous ones'
    print 'm : set the max of each group of the current hidden layer to 1 and the other elements of the group to 0'
    print 's : each group of the current layer is sampled (each group has to sum to 1.0)'
    print 'S : samples the current layer, then reconstruct the previous layer, thant sample this reconstructed layer, and continues until the input'
    print 'z : set the current pixel (the one the mouse is over) to 0.0'
    print 'x : set the current pixel to 0.25'
    print 'c : set the current pixel to 0.5'
    print 'v : set the current pixel to 0.75'
    print 'b : set the current pixel to 1.0'
    print ' space-bar : print value and position of the current pixel'
    print 'i : print values of the current layer'
    print 'w : plot the weight matrices associated with the current pixel'
    print 'W : same as w but for all a group'
    print 'C : same as w but for the hidden unit that has the highest value in each group'
    print 'Z : set all the pixels of the current layer to zero'
    print 'B : set all the pixels of the current layer to 1'
    print 'o : set the current hidden layer to its original state'
    print 'O : same as o but for every layer'
    print 't : now we have the same scale for  W, C'
    print 'h : change the function that converts layers with 2 times the number of units of the input to layers with the same number of units of the input'
    print 'right arrow : prints the next character in the dataset and its corresponding hidden layers and reconstructions'
    print 'left arrow :same but for previous character'
    print '0,1,2,3...9 : after having pressed a digit, right and left arrows will only find this digit'
    print '. : cancel the effec of pressing a digit'
    print 'Control + clic : plots the clicked hidden layer in a new figure'


def appendMatrixToFile(file, matrix, matrix_name=""):
    '''output a matrix into a file'''
    file.write("\n\n" + matrix_name + ' ('+ str(len(matrix)) + 'x' + str(len(matrix[0])) + ')\n\n')
    for i, row in enumerate(matrix):
        file.write('[')
        for j, el in enumerate(row):
            file.write(str(el) + ', ')
        file.write(']\n')
        

class HiddenLayer:
    '''represents a hidden layer which elements are divised in groups
       and we have some methods to be able to plot it'''
    
    def __init__(self,hidden_Layer, groupsize):
        self.hidden_layer = hidden_Layer
        self.groupsize = groupsize
        
        self.max_height = 150

        #if it's too tall, we do this little tweak
        gs = self.groupsize
        height = self.hidden_layer.size/gs
        self.nbgroups = 1
        while height > self.max_height:
            self.nbgroups+=1
            while (self.hidden_layer.size/gs)%self.nbgroups != 0 :
                self.nbgroups+=1                       
            height = self.hidden_layer.size/gs/self.nbgroups
        print 'nbgroups', self.nbgroups
        
    def getMatrix(self):        
        return reshape(self.hidden_layer, (-1,self.groupsize*self.nbgroups))
    
    def matrixToLayer(self, x, y):        
        gs = self.groupsize
        x,y = self.correctXY(x,y)
        return x + gs*self.nbgroups*y        

    def correctXY(self,x,y):
        return int(x + .4), int(y + .3)        

    def getElement(self, x, y):
        n = self.matrixToLayer(x,y)
        return self.hidden_layer[n]

    def fill(self,value):
        self.hidden_layer.fill(value)

    def setElement(self, x,y, value):
        n = self.matrixToLayer(x,y)
        self.hidden_layer[n] = value

    def getRow(self,n):
        return self.hidden_layer[n*self.groupsize:(n+1)*self.groupsize]

    def setRow(self,n,row):
        c=0
        for i in arange(n*self.groupsize,(n+1)*self.groupsize):
           self.hidden_layer[i] = row[c]
           c+=1
    def getNGroups(self):
        return self.hidden_layer.size/self.groupsize


class InteractiveRepRecPlotter:
    '''this class is used to plot representations and reconstructions of a DeepReconstructorNet'''
    
    def __init__(self, learner, vmat, image_width=28, char_indice=0):
        '''constructor'''
        self.current = char_indice-1#-1 because it's the first time
        self.learner = learner
        self.vmat = vmat
        self.char = -1#the char we're looking for, -1 for anyone (it can be -1,0,1,2,3,4,5,6,7,8,9)
        self.image_width = image_width

        self.fig_rec = 0
        self.fig_rep = 1
        figure(self.fig_rec)
        figure(self.fig_rep)

        #self.current_fig = None
        #self.current_axes = None
        self.current_hl = None#current hidden layer

        #plotting constants
        self.interpolation = 'nearest'
        self.cmap = cm.gray

        self.plotNext()#starting with a plot...
        self.__linkEvents()

        self.same_scale = True
        self.from1568to784function = 0
        self.from1568to784functions = [softmaxGroup2, toMinusRow, toPlusRow, evenMinusOdd]


    def size(self):
        return len(self.originals_hl)
        

    ###
    ### getting char from vmat
    ###
    
    def __rowToClassInput(self, row):
        '''we put the last element of row in self.classe, the rest in self.input'''
        self.classe = row[-1:][0]
        self.input = row[:-1]

    def __getNextChar(self):
        '''get next input row from the vmat'''
        while True:
            self.current+=1
            raw_input = vmat.getRow(self.current)
            classe = int(raw_input[-1:])
            if classe == self.char or self.char == -1:
                break            
        self.__rowToClassInput(raw_input)

    def __getPrevChar(self):
        '''get next input row from the vmat'''
        while True:
            self.current-=1
            raw_input = vmat.getRow(self.current)
            classe = int(raw_input[-1:])
            if classe == self.char or self.char == -1:
                break            
        self.__rowToClassInput(raw_input)

    ###
    ### computings
    ###

    def __computeRepresentation(self):
        
        # we convert list to tmat
        imagetmat = TMat([self.input])
        
        #representation
        print 'computing representations...'
        raw_rep = learner.computeRepresentations(imagetmat)        
        print '...done.'

        try:
            #groupsizes = self.learner.groupsizes[1:]
            groupsizes = list(self.learner.getOption('group_sizes')[1:])
        except:
            groupsizes = [10,20,40]
        if groupsizes == []:
            groupsizes = [10, 20, 40]
        groupsizes.insert(0, self.image_width)
        groupsizes.append(1)

        print groupsizes

        self.hidden_layers = []

        for gs,el in zip(groupsizes,raw_rep):
            self.hidden_layers.append(HiddenLayer(el[0], gs))

    def __computeReconstructions(self):

        imagetmat = TMat([self.input])
        print 'computing reconstructions...'
        rec = learner.computeReconstructions(imagetmat)
        print '...done.'

        matrices = [rowToMatrix(self.input, self.image_width)]
        for el in rec:
            row = el[0]
            matrices.append(rowToMatrix(row,self.image_width))

        self.reconstructions = matrices
       

    ###
    ### events
    ###

        
    def __changeChar(self, event):
        char = event.key
        if char in ['1', '2', '3', '4', '5', '6', '7', '8', '9', '0']:
            self.char = int(char)
            print 'now plotting only this digit :', char
        elif char == '.':
            self.char = -1
            print 'now plotting any digit'
        elif char == 'right':
            self.plotNext()
        elif char == 'left':
            self.plotPrev()
        elif char == '':
            pass            


    def __repCommands(self,event):
        
        #met a jour self.current_hl
        self.__findCurrentLayer(event)
        
        char = event.key        
        i = self.current_hl
               
        if i != -1:

            #commun

            hl1 = self.hidden_layers[i]
            if i >  0:
                hl0 = self.hidden_layers[i-1]
            if i < self.size()-1:
                hl2 = self.hidden_layers[i+1]

            hl = hl1
                        
            # fprop -- f

            if char == 'f':                
                print 'fproping...'
                #update                
                self.learner.setMatValue(i-1, reshape(hl0.hidden_layer, (1,-1)))
                #fprop
                row = self.learner.fpropOneLayer(i-1)[0]
                #print
                hl1.hidden_layer = row
                self.rep_axes[i].imshow(hl1.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()
                print '...done'

            # big-fprop -- F

            elif char == 'F':
                print 'big-fproping...'                
                for k in arange(i-1, self.size()-1):
                    print 'k',k
                    self.learner.setMatValue(k, reshape(self.hidden_layers[k].hidden_layer, (1,-1)))
                    row = self.learner.fpropOneLayer(k)[0]
                    self.hidden_layers[k+1].hidden_layer = row
                    self.rep_axes[k+1].imshow(self.hidden_layers[k+1].getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                    draw()
                print '...done'
                
            # reconstruction -- r
                
            elif char == 'r':
                if i<self.size()-1:
                    print 'reconstructing...'
                    self.__reconstructLayer(i)
                    self.rep_axes[i].imshow(hl1.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)                 
                    draw()
                    print '...done'
                else:
                    print 'invalid layer for reconstruction'

            # big-reconstruction -- R

            elif char == 'R':
                if i<self.size()-2:
                    print 'big-reconstructing...'
                    for k in arange(i, -1, -1):
                        self.__reconstructLayer(k)
                        self.rep_axes[k].imshow(self.hidden_layers[k].getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                    draw()
                    print '...done'
                else:
                    print 'invalid layer for reconstruction'

            # max -- m

            elif char == 'm':
                print 'maximum...'

                for n in arange(hl.hidden_layer.size/hl.groupsize):

                    row = hl.getRow(n)

                    #finding out the max
                    indmax = 0
                    for el in arange(1,len(row)):
                        if row[el] > row[indmax]:                            
                            indmax = el                    

                    #set max = 1, other = 0
                    for el in arange(len(row)):
                        if el == indmax:
                            row[el] = 1.
                        else:
                           row[el] = 0.

                    hl.setRow(n,row)
                            
                self.rep_axes[i].imshow(hl.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()
                
                print '...done'

            # sampling -- s

            elif char == 's':
                print 'sampling...'
                self.__sampleLayer(i)                
                self.rep_axes[i].imshow(hl.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()
                print '...done'

            # sampling + reconstructin -- S

            elif char == 'S':
                print 'sampling and reconstruction...'
                for n in arange(i,0,-1):
                    self.__sampleLayer(n)
                    self.__reconstructLayer(n-1)
                for n in arange(i+1):
                    self.rep_axes[n].imshow(self.hidden_layers[n].getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()
                print '...done'
                
            # set pixel -- z,x,c,v,b

            elif char=='Z':
                hl.fill(0.)
                self.rep_axes[i].imshow(hl.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()

            elif char=='B':
                hl.fill(1.)
                self.rep_axes[i].imshow(hl.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()

            elif char in ['z', 'x', 'c', 'v', 'b']:
                
                x,y = event.xdata, event.ydata
                
                if char == 'z':
                    hl.setElement(x,y,0.)
                elif char == 'x':
                    hl.setElement(x,y,.25)
                elif char == 'c':
                    hl.setElement(x,y,.5)
                elif char == 'v':
                    hl.setElement(x,y,.75)
                elif char == 'b':
                    hl.setElement(x,y,1.)

                self.rep_axes[i].imshow(hl.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()
                                            
            # infos -- ' '

            elif char == ' ':
                x,y = event.xdata, event.ydata
                print
                print 'Position', hl.matrixToLayer(x,y), '(', x, y, ')'
                print 'Value', hl.getElement(x,y)

            # more infos -- 'i'

            elif char == 'i':
                print hl.getMatrix()                

            # plot W and M -- w

            elif char == 'w':

                prefixe = 'Layer' + str(i)
                nameW = prefixe + '_W'
                nameWr = nameW + 'r'
                nameM = prefixe + '_M'
                nameMr = nameM + 'r'
                nameB = prefixe + '_b'
                nameBr = nameB + 'r'
             
                listNames = learner.listParameterNames()

                matricesToPlot = []
                namesToPlot = []

                x,y = hl.correctXY(event.xdata,event.ydata)
                #n = hl.matrixToLayer(x, y)
                n = hl.matrixToLayer(event.xdata, event.ydata)               
                
                
                if nameW in listNames and nameM not in listNames:
                    
                    row = learner.getParameterRow(nameW,n)
                    
                    #HACK !!!
                    print 'just hacked...'
                    if i==1 and len(row) == 28*28*2:
                        matricesToPlot.append(doubleSizedWeightVectorToImageMatrix(row))
                    #END OF HACK
                    else:
                        matricesToPlot.append(reshape(row, (-1, self.hidden_layers[i-1].groupsize)))
                    namesToPlot.append(nameW)

                    if nameWr in listNames:

                        row = learner.getParameterRow(nameWr,n)
                    
                        #HACK !!!
                        print 'just hacked...'
                        if i==1 and len(row) == 28*28*2:
                            matricesToPlot.append(doubleSizedWeightVectorToImageMatrix(row))
                        #END OF HACK 
                        else:
                            matricesToPlot.append(reshape(row, (-1, self.hidden_layers[i-1].groupsize)))
                        namesToPlot.append(nameWr)

                    figure(3)
                    
                    clf()
                    plotMatrices(matricesToPlot, namesToPlot)
                    draw()

                if nameW in listNames and nameM in listNames:

                    rowW = learner.getParameterRow(nameW,n%hl.groupsize)
                    rowM = learner.getParameterRow(nameM,int(n/hl.groupsize))
                    
                    #HACK !!!
                    #print 'just hacked...'
                    #if i==1 and len(row) == 28*28*2:
                    #    row = array(toMinusRow(row))
                    #END OF HACK
                                      
                    #matW1 = reshape(toMinusRow(rowW), (-1,self.hidden_layers[i-1].groupsize))
                    #matM1 = reshape(toMinusRow(rowM),(-1,self.hidden_layers[i-1].groupsize))
                    #matW2 = reshape(toPlusRow(rowW), (-1,self.hidden_layers[i-1].groupsize))
                    #matM2 = reshape(toPlusRow(rowM),(-1,self.hidden_layers[i-1].groupsize))
                    
                    matW =doubleSizedWeightVectorToImageMatrix(rowW)
                    matM = doubleSizedWeightVectorToImageMatrix(rowM)
                    produit = matW*matM
                   

                    
                    figure(3)
                    myioff()
                    clf()                  
                    plotMatrices([matW,matM,produit], [nameW, nameM,'term-to-term product'])
                    myion()
                    draw()

#                     if nameWr in listNames and nameMr in listNames:

#                         print 'ok'
#                         rowWr = learner.getParameterRow(nameWr, n%hl.groupsize)
#                         rowMr = learner.getParameterRow(nameMr, int(n/hl.groupsize))
#                         print 'ok2'
                        
#                         matWr1 = reshape(toMinusRow(rowWr), (-1, self.hidden_layers[i-1].groupsize))
#                         matMr1 = reshape(toMinusRow(rowMr), (-1, self.hidden_layers[i-1].groupsize))
#                         matWr2 = reshape(toPlusRow(rowWr), (-1, self.hidden_layers[i-1].groupsize))
#                         matMr2 = reshape(toPlusRow(rowMr), (-1, self.hidden_layers[i-1].groupsize))
#                         print 'ok3'

#                         produit = matWr1*matMr1
#                         produit2 = matWr2*matMr2

#                         figure(4)
#                         myioff()
#                         clf()
#                         plotMatrices([matWr1, matMr1, matWr2, matMr2, produit, produit2], [nameWr + "-", nameMr + "-", nameWr + "+", nameMr + "+", 't.-t.-t. product (-)', 't.-t.-t. product (+)'])
#                         myion()
#                         draw()

                #BIAS
                #TODO
                
            
            #like 'w' but for an entire row -- 'W'
            elif char == 'W':

                prefixe = 'Layer' + str(i)
                nameW = prefixe + '_W'
                nameWr = nameW + 'r'
                nameM = prefixe + '_M'
                nameMr = nameM + 'r'
                nameB = prefixe + '_b'
                nameBr = nameB + 'r'

                listNames = learner.listParameterNames()

                matricesToPlot = []
                namesToPlot = []                

                print 'x,y=', event.xdata, event.ydata
                x,y = hl.correctXY(event.xdata, event.ydata)
                n = hl.matrixToLayer(x,y)
                print 'n =',n


                names = []
                for i in arange(n, n+hl.groupsize):
                    names.append( formatFloat(hl.hidden_layer[i]))
            
                if nameW in listNames and nameM not in listNames:                

                    n = n - n%hl.groupsize
                    print 'plotting weigths from',n,'to',n+hl.groupsize-1
                    print learner.getParameterValue(nameW).shape
                   
                    figure(3)
                    myioff()                    
                    clf()
                    plotLayer1(learner.getParameterValue(nameW), 28, .1, n, hl.groupsize,.05, self.from1568to784functions[self.from1568to784function], [], names, self.same_scale)
                    myion()
                    draw()

                if nameW in listNames and nameM in listNames:

                    
                    w = learner.getParameterValue(nameW)
                    m = learner.getParameterValue(nameM)
                    M = zeros((hl.groupsize,w.shape[1]))
                    
                    for a in arange(hl.groupsize):
                        M[a] = m[y]*w[a]                    

                    figure(3)
                    myioff()
                    clf()
                    plotLayer1(M, 28, .056,0,M.shape[0],.05, self.from1568to784functions[self.from1568to784function], [], names, self.same_scale)
                    myion()
                    draw()

            # like 'w' on the max of each row -- 'C'

            elif char == 'C':

                prefixe = 'Layer' + str(i)
                nameW = prefixe + '_W'
                nameWr = nameW + 'r'
                nameM = prefixe + '_M'
                nameMr = nameM + 'r'
                nameB = prefixe + '_b'
                nameBr = nameB + 'r'
                
                listNames = learner.listParameterNames()
                
                matricesToPlot = []
                namesToPlot = []
                
                print event.xdata,event.ydata
                
                indexes = []
                names = []
                for j in arange(hl.getNGroups()):
                    row = hl.getRow(j)
                    index = 0
                    for k,el in enumerate(row):
                        if el > row[index]:
                            index = k

                    indexes.append(j*hl.groupsize + index)
                    names.append(formatFloat(row[index]))

                if nameW in listNames and nameM not in listNames:
                   
                    figure(3)
                    myioff()                    
                    clf()
                    plotLayer1(learner.getParameterValue(nameW), 28, .056, 0,0,.05, self.from1568to784functions[self.from1568to784function], indexes,names, self.same_scale)
                    myion()
                    draw()
                    
                if nameW in listNames and nameM in listNames:
                    
                    figure(3)
                    myioff()
                    clf()

                    w = learner.getParameterValue(nameW)
                    m = learner.getParameterValue(nameM)
                    M = zeros((len(indexes),w.shape[1]))
                    
                    for y,x in enumerate(indexes):
                        print x%hl.groupsize,y
                        M[y] =  m[y]*w[x%hl.groupsize]
                    print M
                    
                    plotLayer1(M, 28, .056,0,M.shape[0],.05,self.from1568to784functions[self.from1568to784function],[],names, self.same_scale)
                    myion()
                    draw()

                    figure(4)
                    myioff()
                    clf()                    
                    plotLayer1(M, 28, .056,0,M.shape[0],.05,self.from1568to784functions[self.from1568to784function],[],names, self.same_scale)
                    myion()
                    draw()

                if nameWr in listNames and nameMr not in listNames:

                    figure(5)
                    myioff()
                    clf()
                    plotLayer1(learner.getParameterValue(nameWr), 28, .056,0,0,.05,self.from1568to784functions[self.from1568to784function],indexes,names, self.same_scale)
                    myion()
                    draw()
                    
                    

            # back to Original -- o
            
            elif char == 'o':
                print 'getting original layer...'                
                self.hidden_layers[i].hidden_layer = copy.copy(self.originals_hl[i].hidden_layer)
                self.rep_axes[i].imshow(self.hidden_layers[i].getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()
                print '...done'

            elif char != 'shift' and char != 'control':
                print char
                print_usage_repAndRec()

        # change the "hack" function -- h

        if char == 'h':
            self.from1568to784function = (self.from1568to784function + 1 ) % len(self.from1568to784functions)
            print '1568 to 784 function is now', self.from1568to784functions[self.from1568to784function]
            
            


        # toggle "same scale" or "individuals scales" for  'W' and 'C' -- t
        
        if char == 't':
            self.same_scale = 1-self.same_scale
            if self.same_scale:
                print 'now we have the same scale for  W, C'
            else:
                print 'now we have individuals scales for  W, C'
                
            
        # big-back to Original -- O
        
        if char == 'O':
            print 'getting all original layers...'
            for k in arange(0,self.size()):
                self.hidden_layers[k].hidden_layer = copy.copy(self.originals_hl[k].hidden_layer)
                self.rep_axes[k].imshow(self.hidden_layers[k].getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()
            print '...done'



    def __clicked(self, event):
            
        self.__findCurrentLayer(event)
        
        if event.key == 'control':
            print 'control-clic performed'

            data = self.hidden_layers[self.current_hl].getMatrix()
            x,y,s,c = [],[],[],[]
            for i in arange(data.shape[0]):
                for j in arange(data.shape[1]):
                    x.append(j)
                    y.append(data.shape[1]-i)
                    s.append(data[i,j]*25.)
                    if data[i,j]<0 :
                        c.append(1)
                    else:
                        c.append(-1)
            f = figure(5)
            
            clf()
            scatter(x,y,s=s, c=c, marker='s',cmap = cm.gray, norm = Normalize())
            f.gca().set_axis_bgcolor("0.75")
            colorbar()
            draw()            
        

    def __findCurrentLayer(self, event):

        fig = event.canvas.figure.number
        axes = event.inaxes

        self.current_hl = -1
        if axes != None:
            figure(fig)
            
            
            #we find to which hidden layer corresponds our axes
            for i,a in enumerate(self.rep_axes):                
                if a == axes:
                    self.current_hl = i
    

    def __linkEvents(self):

        fig = figure(self.fig_rep)
        
        connect('key_press_event', self.__changeChar)
        connect('button_press_event', self.__clicked)
        connect('key_press_event', self.__repCommands)

        fig = figure(self.fig_rec)
        
        # fig.canvas.mpl_connect('key_press_event', self.__changeChar)        
        connect('key_press_event', self.__changeChar)        
        #connect('button_press_event', self.__clicked)        
        
    ###
    ### plotting
    ###

    def __plotReconstructions(self):
        print 'plotting reconstructions...'        
        figure(self.fig_rec)
        myioff()
        clf()
        plotMatrices(self.reconstructions)
        myion()
        draw()
        print '...done.'

    def __plotRepresentations(self):
        print 'plotting representations...'
        myioff()
        figure(self.fig_rep)
        clf()
        temp = []
        for x in self.hidden_layers:
            temp.append( x.getMatrix() )        
        #draw()
        self.rep_axes = plotMatrices(temp)        
        draw()
        myion()
        print '...done.'

    def __computeAndPlot(self):        
        self.__computeRepresentation()
        self.__computeReconstructions()
        self.__plotReconstructions()
        self.__plotRepresentations()

    def plotNext(self):
        self.__getNextChar()
        self.__computeAndPlot()

        #saving "original" matrices
        #print 'copying originals'
        self.originals_hl = copy.deepcopy(self.hidden_layers)
        
        print 'a', self.classe, 'was plotted'
        

    def plotPrev(self):
        self.__getPrevChar()
        self.__computeAndPlot()

        #saving "original" matrices
        #print 'copying originals'
        self.originals_hl = copy.deepcopy(self.hidden_layers)
        
        print 'a', self.classe, 'was plotted'
    
            
    def plotOld(self):

        print 'entered in plot method'
               

        #reconstruction
        print 'computing reconstructions...'
        rec = learner.computeReconstructions(imagetmat)
        
        print 'executing some matrix manipulations...'
        row = raw_rep[0][0]
        
        image = rowToMatrix(row,28)
        
        listDeMatrices = [image]        
        for el in raw_rep[1:]:
            listDeMatrices.append(rowToMatrix(el[0],max(len(el[0])/100.,1), False))

        listDeMatrices2 = [image]
        for el in rec:
            row = el[0]
            listDeMatrices2.append(rowToMatrix(row,28))
        
    
        print 'plotting'
        #plotting
        figure(self.nofig)
        clf()
        plotMatrices(listDeMatrices)        
        draw()

        figure(self.nofig+1)
        clf()
        plotMatrices(listDeMatrices2)
        draw()       

        self.i+=1

        if self.log_file != "":
            file = open(self.log_file,'a')

            file.write("\n\n\n\n\n\n--------------------------------------------------------------------")
            file.write("---------- REP ------------------------------------------------------\n")


            appendMatrixToFile(file, image, "input")
            
            for i,mat in enumerate(raw_rep[1:]):
                appendMatrixToFile(file, mat, 'rep of hidden layer ' + str(i+1))


            file.write("\n\n\n---------- REC ------------------------------------------------------\n")            
            for i, mat in enumerate(rec):
                appendMatrixToFile(file,  rowToMatrix(mat[0],28), 'rec of hidden layer ' + str(i+1))

    ###
    ### utils
    ###

    def __sampleLayer(self, which_layer):
        hl = self.hidden_layers[which_layer]
        for n in arange(hl.hidden_layer.size/hl.groupsize):
            multi = numpy.random.multinomial(1,hl.getRow(n))                    
            hl.setRow(n,multi)

    def __reconstructLayer(self, which_layer):
        hl = self.hidden_layers[which_layer+1]
        
        self.learner.setMatValue(which_layer+1, reshape(hl.hidden_layer, (1,-1)))
        #reconstruct
        row = self.learner.reconstructOneLayer(which_layer+1)[0]
        #HACK
        if len(row) == 28*28*2:
            print 'hacking...'
            row = array(toMinusRow(row))
            #print the new layer
        self.hidden_layers[which_layer].hidden_layer = row
    


class EachRowPlotter:
    
    def __init__(self, matrix, width = 28, plot_width = .1, space_between_images = .01, do_to_rows = None, i=0, nofig=0):
        self.matrix = matrix
        self.i = i
        self.nofig = nofig
        self.do_to_rows = do_to_rows
        self.last_element = -1
        self.plot_width = plot_width
        self.sbi = space_between_images
        self.width = width

    def plotNext(self, event):
        clf()
        if self.last_element > 0:
            self.last_element = plotLayer1(matrix, self.width, self.plot_width, self.last_element+1, -1, self.sbi, doToRow)
        else:
            self.last_element = plotLayer1(matrix, self.width, self.plot_width, 0, -1, self.sbi, doToRow)
        draw()

    def plot(self):        
        print 'Plotting matrix ' +  matrixName + ' (' + str(len(matrix)) + 'x' + str(len(matrix[0])) + ')'        
        self.last_element = plotLayer1(matrix,self.width, self.plot_width, 0, -1, self.sbi, self.do_to_rows)                   
        connect("button_press_event",self.plotNext)
        
    

        

############
### main ###
############

server_command = "myplearn server"
serv = launch_plearn_server(command = server_command)

#print "Press Enter to continue"
#raw_input()

if len(sys.argv)<2:
    print_usage_and_exit()

task = sys.argv[1]

def openVMat(vmatspec):
    if vmatspec.endswith(".amat") or vmatspec.endswith(".pmat"):
        vmat = serv.new('AutoVMatrix(specification ="'+vmatspec+'");')
    else:
        vmat = serv.load(vmatspec)
    return vmat

if task == 'plotEachRow':

    psave = sys.argv[2]
    
    learner = serv.load(psave)
        
    matrices = learner.listParameter() 
    names = learner.listParameterNames()
    
    doToRow = None
    #doToRow = toMinusRow
    #doToRow = toMinusRow

    matrixName = ''
    while matrixName != 'exit':
        print
        print 'Matrix list :'
        print names

        print
        matrixName = raw_input('Choose a matrix to be plotted (or \'exit\') >>> ')
        
        if matrixName in names:

            #matrix = rand(500,28*28*2)
            matrix = learner.getParameterValue(matrixName)
            print "shape: ",matrix.shape

            # guess image dimensions
            n = matrix.shape[1]
            imgheight = int(math.sqrt(n))+1
            while n%imgheight != 0:
                imgheight = imgheight-1
            imgwidth = n/imgheight
            
            showRowsAsImages(matrix, img_height=imgheight, img_width=imgwidth, nrows=5, ncols=7, figtitle=matrixName)
            #plotter = EachRowPlotter(matrix, 28, .1, .01, doToRow)
            #plotter.plot()
            #show()          
            
        elif matrixName != 'exit':
            print
            print 'This matrix does not exist !'


elif task == 'plotRepAndRec':
    
    psave = sys.argv[2]
    datafname = sys.argv[3]
    #test = sys.argv[5]
    #print test
   
    #loading learner
    learner = serv.load(psave)
    
    #taking an input
    vmat = openVMat(datafname)
    
    matrix_plot = InteractiveRepRecPlotter(learner, vmat)

    show()

    
elif task == 'plotSingleMatrix':

    psave = sys.argv[2]

    learner = serv.load(psave)
    
    nameList = learner.listParameterNames()

    matrixName = ''
    while matrixName != 'exit':
        print
        print 'Matrix list :'
        print nameList

        print
        matrixName = raw_input('Choose a matrix to be plotted (or \'exit\')>>>')
        
        if matrixName in nameList:
            matrix = learner.getParameterValue(matrixName)
            figure(0)
            #cadre = .05
            #axes((cadre,cadre,1.-2*cadre,1-2*cadre))
            #imshow(matrix, interpolation = defaultInterpolation, cmap = defaultColorMap)
            #setPlotParams(matrixName, True, True)            
            #figure(1, figsize=(1000,1000), dpi=40)
            truncate_imshow(matrix)
            show()
            
        elif matrixName != 'exit':
            print
            print 'This matrix does not exist !'

elif task == 'test':
    #jutilise cet endroit pour faire des tests
    pass
elif task == 'help':
    print 'plotRepAndRec:'
    print
    print_usage_repAndRec()



else:
    print_usage_and_exit()




    
