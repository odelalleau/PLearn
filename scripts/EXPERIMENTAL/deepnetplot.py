#!/usr/bin/env python

import sys
from pylab import *
from plearn.io.server import *
from plearn.pyplearn import *
from plearn.plotting.netplot import *
from numarray import *
from numarray.random_array import *
import numpy.random


################
### methods ###
################

def print_usage_and_exit():
    print "Usage: drnPlot <task> <file> [<other arguments>]",    
    "drnPlot plotSingleMatrix x.psave ",
    "drnPlot plotEachRow learner.psave chars.pmat"
    "drnPlot plotRepAndRec learner.psave chars.pmat"
    sys.exit()


def appendMatrixToFile(file, matrix, matrix_name=""):
    file.write("\n\n" + matrix_name + ' ('+ str(len(matrix)) + 'x' + str(len(matrix[0])) + ')\n\n')
    for i, row in enumerate(matrix):
        file.write('[')
        for j, el in enumerate(row):
            file.write(str(el) + ', ')
        file.write(']\n')
        

class HiddenLayer:
    
    def __init__(self,hidden_Layer, groupsize):
        self.hidden_layer = hidden_Layer
        self.groupsize = groupsize

        self.max_height = 150

        #if it's too tall, we do this little tweak
        gs = self.groupsize
        height = self.hidden_layer.size()/gs
        self.nbgroups = 1
        while height > self.max_height:
            self.nbgroups+=1
            while (self.hidden_layer.size()/gs)%self.nbgroups != 0 :
                self.nbgroups+=1                       
            height = self.hidden_layer.size()/gs/self.nbgroups
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



class InteractiveRepRecPlotter:
    
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
                print 'reconstructing...'
                self.learner.setMatValue(i+1, reshape(hl2.hidden_layer, (1,-1)))
                #reconstruct
                row = self.learner.reconstructOneLayer(i+1)[0]
                #HACK
                if len(row) == 28*28*2:
                    print 'hacking...'
                    row = array(toMinusRow(row))
                #print the new layer
                hl1.hidden_layer = row
                self.rep_axes[i].imshow(hl1.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)                 
                draw()
                print '...done'

            # big-reconstruction -- R

            elif char == 'R':
                print 'big-reconstructing...'
                for k in arange(i+1, 0, -1):
                    self.learner.setMatValue(k, reshape(self.hidden_layers[k].hidden_layer, (1,-1)))
                    row = self.learner.reconstructOneLayer(k)[0]
                    #HACK
                    if len(row) == 28*28*2:
                        print 'hacking...'
                        row = array(toMinusRow(row))
                    self.hidden_layers[k-1].hidden_layer = row
                    self.rep_axes[k-1].imshow(self.hidden_layers[k-1].getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                    draw()
                print '...done'                

            # max -- m

            elif char == 'm':
                print 'maximum...'

                for n in arange(hl.hidden_layer.nelements()/hl.groupsize):

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
                for n in arange(hl.hidden_layer.nelements()/hl.groupsize):
                    print 'sum', hl.getRow(n).sum()
                    multi = numpy.random.multinomial(1,hl.getRow(n))                    
                    hl.setRow(n,multi)                            
                self.rep_axes[i].imshow(hl.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()
                print '...done'
                
            # set pixel -- z,x,c,v,b

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
                n = hl.matrixToLayer(x, y)
                
                
                if nameW in listNames and nameM not in listNames:
                    
                    row = learner.getParameterRow(nameW,n)
                    
                    #HACK !!!
                    print 'just hacked...'
                    if i==1 and len(row) == 28*28*2:
                        row = array(toMinusRow(list(row)))
                    #END OF HACK

                    matricesToPlot.append(reshape(row, (-1, self.hidden_layers[i-1].groupsize)))
                    namesToPlot.append(nameW)

                    if nameWr in listNames:

                        row = learner.getParameterRow(nameWr,n)
                    
                        #HACK !!!
                        print 'just hacked...'
                        if i==1 and len(row) == 28*28*2:
                            row = array(toMinusRow(list(row)))
                        #END OF HACK 
                        
                        matricesToPlot.append(reshape(row, (-1, self.hidden_layers[i-1].groupsize)))
                        namesToPlot.append(nameWr)

                    figure(3)
                    clf()
                    plotMatrices(matricesToPlot, namesToPlot)
                    draw()

                if nameW in listNames and nameM in listNames:

                    rowW = learner.getParameterRow(nameW,x)
                    rowM = learner.getParameterRow(nameM,y)
                    
                    #HACK !!!
                    #print 'just hacked...'
                    #if i==1 and len(row) == 28*28*2:
                    #    row = array(toMinusRow(row))
                    #END OF HACK
                                      
                    rowW = reshape(rowW, (-1,self.hidden_layers[i-1].groupsize))
                    rowM = reshape(rowM,(-1,self.hidden_layers[i-1].groupsize))

                    #TODO: rajouter les deux cas  : juste un Wr et lautre : Mr ET Wr

                    produit = rowW*rowM                   

                    figure(3)
                    clf()                  
                    plotMatrices([rowW,rowM,produit], [nameW,nameM, 'term-to-term product'])
                    draw()

                #BIAS

                if nameB in listNames:

                    row = learner.getParameterValue(nameB)
                    print nameB,row.shape

                    print 'i',i
                    matricesToPlot = [reshape(row, (-1, self.hidden_layers[i].groupsize))]
                                           
                    namesToPlot = [nameB]

                    if nameBr in listNames:

                        row = learner.getParameterValue(nameBr)
                        print nameBr,row.shape

                        #HACK !!!
                        print i, row.shape[1]
                        if i==1 and row.shape[1] == 28*28*2:
                            row = array(toMinusRow(list(row[0])))
                            print 'just hacked...'
                        #END OF HACK

                        matricesToPlot.append(reshape(row, (-1, self.hidden_layers[i-1].groupsize)))
                        
                        namesToPlot.append(nameBr)

                    figure(4)
                    clf()
                    plotMatrices(matricesToPlot, namesToPlot)
                    draw()
                

            # back to Original -- o
            
            elif char == 'o':
                print 'getting original layer...'                
                self.hidden_layers[i].hidden_layer = copy.copy(self.originals_hl[i].hidden_layer)
                self.rep_axes[i].imshow(self.hidden_layers[i].getMatrix(), interpolation = self.interpolation, cmap = self.cmap)
                draw()
                print '...done'
                
       

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
        
        print 'current hidden layer is now', self.current_hl

        if event.key == 'control' and self.current_hl != -1:
            
            hidden_layer = self.hidden_layers[self.current_hl]
            
            n = hidden_layer.matrixToLayer(x,y)
           # print 'n', n
            hidden_layer.hidden_layer[n] = 1 - hidden_layer.hidden_layer[n]
            axes.imshow(hidden_layer.getMatrix(), interpolation = self.interpolation, cmap = self.cmap)        
            draw()

        if event.button == 3:
            print 'Layer', self.current_hl, ', position (',x,y,'), value', xself.hidden_layers[self.current_hl].getElement(x,y)

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

        figure(self.fig_rep)
        connect('key_press_event', self.__changeChar)
        #connect('button_press_event', self.__clicked)
        connect('key_press_event', self.__repCommands)

        figure(self.fig_rec)
        connect('key_press_event', self.__changeChar)        
        #connect('button_press_event', self.__clicked)        
        
    ###
    ### plotting
    ###

    def __plotReconstructions(self):
        print 'plotting reconstructions...'
        figure(self.fig_rec)
        clf()
        plotMatrices(self.reconstructions)
        draw()
        print '...done.'

    def __plotRepresentations(self):
        print 'plotting representations...'
        figure(self.fig_rep)
        clf()
        temp = []
        for x in self.hidden_layers:
            temp.append( x.getMatrix() )        
        draw()

        self.rep_axes = plotMatrices(temp)
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

server_command = "slearn server"
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
    
    #doToRow = None
    doToRow = toPlusRow
    #doToRow = toMinusRow

    matrixName = ''
    while matrixName != 'exit':
        print
        print 'Matrix list :'
        print names

        print
        matrixName = raw_input('Choose a matrix to be plotted (or \'exit\')>>>')
        
        if matrixName in names:

            matrix = learner.getParameterValue(matrixName)
            #matrix = rand(500,28*28*2)
            plotter = EachRowPlotter(matrix, 28, .1, .01, doToRow)
            plotter.plot()
            show()          
            
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



else:
    print_usage_and_exit()
