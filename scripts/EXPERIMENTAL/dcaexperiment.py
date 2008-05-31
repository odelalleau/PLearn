#!/usr/bin/env python

import os ,sys, time, matplotlib, math, copy
from matplotlib.pylab import *
from matplotlib.colors import *
from plearn.plotting.netplot import plotRowsAsImages, showRowsAsImages
from plearn.vmat.PMat import *

import numpy
from numpy import *
#from numpy.linalg import *
import numpy.numarray
#from plearn.vmat.PMat import *

# from numpy.numarray import *
# from numarray import *
#from numarray.linear_algebra import *

from plearn.io.server import *
from plearn.pyplearn import *

#from plearn.plotting import *
#from copy import *

#from plearn.plotting.netplot import showRowsAsImages

server_command = 'plearn_exp server'
serv = launch_plearn_server(command = server_command)

class DCAExperiment:

    def __init__(self,
                 X,
                 seed=1827,
                 ncomponents=2,
                 constrain_norm_type=-1,
                 cov_transformation_type="cov",
                 diag_weight = -1.0,
                 diag_nonlinearity="square", 
                 diag_premul = 1.0,
                 offdiag_weight=1.0,
                 offdiag_nonlinearity="square", 
                 offdiag_premul = 1.0,
                 lr=0.01, nsteps=1, optimizer_nsteps=1,
                 epsilon=1e-4, nu=0,
                 img_height=None,
                 img_width=None):

        self.nsteps = nsteps

        #some calculations for plotting
        if img_height is None:
            img_size = len(X[0])
            img_width = math.sqrt(img_size)
            img_height = img_size/img_width
        self.img_height = img_height
        self.img_width = img_width

        if isinstance(X, numpy.ndarray):
            self.d = len(X[0])
            self.X = X            
            vmat = pl.MemoryVMatrix(data = X, inputsize=len(X[0]), targetsize=0, weightsize=0)
        else:
            self.d = 100000
            vmat = X

        if self.d==2:
            C = cov(X, rowvar=0, bias=1)
            ei, eiv = eig(C)
            self.principal_components = eiv.T

        dcaspec = pl.DiverseComponentAnalysis(
            seed = seed,
            ncomponents = ncomponents,
            constrain_norm_type=constrain_norm_type,
            cov_transformation_type=cov_transformation_type,
            diag_weight = diag_weight,
            diag_nonlinearity = diag_nonlinearity, 
            diag_premul = diag_premul,
            offdiag_weight = offdiag_weight,
            offdiag_nonlinearity = offdiag_nonlinearity, 
            offdiag_premul = offdiag_premul,
            optimizer = pl.GradientOptimizer(start_learning_rate = lr,
                                             decrease_constant = 0,
                                             nstages = optimizer_nsteps),
            epsilon = epsilon,
            nu = nu
            )

        self.dca = serv.new(dcaspec)
        print "Setting training set..."
        self.dca.setTrainingSet(vmat,1)

        W = self.dca.getVarValue('W')
        # print "W[0]=",W[0]
        print "Squared norm of first row of W: ",sum(W[0]*W[0])
        
        self.dca.nstages = 1
        print "Training to stage 1"
        self.dca.train()
        print "Training to stage 1 DONE."
        print "Squared norm of first row of W: ",sum(W[0]*W[0])
        self.training_curve = []

        self.datafig = 2
        self.traincurvefig = 1
        self.filterfig = 3
        self.Wfig = 4
        self.Cytfig = 5
        self.Cxfig = 6
        self.Cythist = 7

        self.draw()
        
        figure(self.traincurvefig)
        connect('key_press_event', self.keyPressed)

        if self.d==2:
            figure(self.datafig)
            connect('key_press_event', self.keyPressed)
            # connect('button_press_event', self.__clicked)

        figure(self.filterfig)
        connect('key_press_event', self.keyPressed)
        #figure(self.Wfig)
        figure(self.Cytfig)
        #figure(self.Cxfig)
        #figure(self.Cythist)

        # start interactive loop
        show()


    def draw(self):
        figure(self.traincurvefig)
        clf()
        # print 'stages:',arange(2,2+len(self.training_curve))
        # print 'losses:',array(self.training_curve)
        plot(arange(2,2+len(self.training_curve)),array(self.training_curve),'g-')
        title("Training curve")
        xlabel("stage")
        ylabel("optimized cost L")
        draw()

        if(self.d==2):
            figure(self.datafig)
            clf()
            axis('equal')
            plot(self.X[:,0], self.X[:,1], 'b.')
            mu = self.dca.mu
            W = self.dca.getVarValue('W')
            for direction in self.principal_components:
                arrow( mu[0], mu[1], direction[0], direction[1],
                       linewidth=2, edgecolor='black', facecolor='black')
            i = 0
            for w in W:
                arrow( mu[0], mu[1], w[0], w[1],
                       linewidth=2, edgecolor='red', facecolor='red')
                text(mu[0]+w[0], mu[1]+w[1], str(i))
                i = i+1
            title("Data and learnt projection directions")
            draw()
        
    def printState(self): 
        print "----------------------------------"
        print "stage",self.dca.nstages
        print "L =",self.dca.getVarValue('L')[0,0]
        if self.d<=2:
            print "Cyt = "
            print self.dca.getVarValue('Cyt')
            print "gradient Cyt = "
            print self.dca.getVarGradient('Cyt')
            print "W = "
            print self.dca.getVarValue('W')
            print "gradient W = "
            print self.dca.getVarGradient('W')
       
    def trainN(self, n=1):
        lr = self.dca.getOption("optimizer.start_learning_rate")
        print "-------------------------------"
        print "TRAINING for",n,"steps at lr=",lr
        while n>0:
            self.dca.nstages = self.dca.nstages+1
            self.dca.train()
            loss = self.dca.getVarValue('L')[0,0]
            self.training_curve.append(loss)
            n = n-1
        self.printState()
        self.draw()

    def changeSteps(self):
        print "********************************"
        print "new nsteps (",self.nsteps,")?",
        self.nsteps = input()
        lr = self.dca.getOption("optimizer.start_learning_rate")
        print "new lr (",lr,")?",
        lr = input()
        self.dca.changeOptions({"optimizer.start_learning_rate":lr})
        self.dca.changeOptions({"optimizer.learning_rate":lr})
        self.draw()

    def showFilters(self):
        W = self.dca.getVarValue('W')

        #figure(self.Wfig)
        #clf()
        #title('W')
        #imshow(W, interpolation="nearest", cmap = cm.jet)
        #colorbar()
        #draw()

        figure(self.Cytfig)
        Cyt = self.dca.getVarValue('Cyt')
        l = len(Cyt)
        diagvals = []
        offdiagvals = []
        for i in xrange(l):            
            for j in xrange(i):
                offdiagvals.append(Cyt[i,j])
            diagvals.append(Cyt[i,i])

        clf()
        subplot(1,3,1)
        title('Cyt')
        imshow(Cyt, interpolation="nearest", cmap = cm.jet)
        colorbar()

        subplot(1,3,2)
        title('Cyt diagonal values histogram')
        hist(diagvals)
        
        subplot(1,3,3)
        title('Cyt off-diagonal values histogram')
        hist(offdiagvals)

        draw()
        
        #figure(self.Cxfig)
        #clf()
        #title('Cx')
        #imshow(self.dca.getVarValue('Cx'), interpolation="nearest", cmap = cm.jet)
        #colorbar()
        #draw()

        figure(self.filterfig)
        clf()
        plotRowsAsImages(W, 
                         img_height = self.img_height,
                         img_width = self.img_width,
                         nrows=5, ncols=10,
                         figtitle="filters",
                         show_colorbar=False, disable_ticks=True, colormap = cm.gray,
                         luminance_scale_mode = 0, vmin=None, vmax=None)        
        draw()

    def save(self):
        print "*** SAVE TO FILE ***"
        print "normalize output (0 or 1)? ",
        normalize = int(raw_input())
        self.dca.changeOptions({'normalize':normalize})        
        print "filename (.psave)? ",
        filename = raw_input()
        self.dca.save(filename,'plearn_ascii')
        print "*** SAVING DONE ***"

    def keyPressed(self, event):
        char = event.key
        print 'Pressed',char
        if char == ' ':
            self.trainN(self.nsteps)
        elif char == '1':
            self.trainN(10*self.nsteps)
        elif char == '2':
            self.trainN(20*self.nsteps)
        elif char == '3':
            self.trainN(30*self.nsteps)
        elif char == '4':
            self.trainN(40*self.nsteps)
        elif char == '5':
            self.trainN(50*self.nsteps)
        elif char == '6':
            self.trainN(60*self.nsteps)
        elif char == '7':
            self.trainN(70*self.nsteps)
        elif char == '8':
            self.trainN(80*self.nsteps)
        elif char == '9':
            self.trainN(90*self.nsteps)
        elif char == 'o':
            self.changeSteps()
        elif char == 'w':
            self.showFilters()
        elif char == 's':
            self.save()
        elif char == '':
            pass
        else:
            print """
            *******************************************************
            * KEYS
            *   spacebar: does nsteps training steps
            *   '1'     : does 10*nsteps training steps
            *   '2'     : does 20*nsteps training steps
            *   ....           ...
            *   '9'     : does 90*nsteps training steps
            *   'o'     : to change optimizaiton nsteps and lr
            *   'f'     : interactive show filters
            *   's'     : save learner to file
            * Close window to stop.
            *******************************************************
            """


def generate2dNormalAtAngle(npoints=300, angle=None, stddev1=1, stddev2=0.1, mean=[0,0]):    
    if angle is None:
        angle = random.uniform(0., 2*math.pi)
    v1 = array([stddev1*math.cos(angle), stddev1*math.sin(angle)])
    v2 = array([stddev2*math.sin(angle), stddev2*math.cos(angle)])
    V = vstack([v1,v2])
    cov = 0.5*dot(V.T,V)
    X = random.multivariate_normal(mean,cov,npoints)
    return X
    
def generate2dNormal(npoints=200):
    # rnd = random.RandomState(seed)
    mean = random.normal(0, 1, 2)
    #mean = array([0,0])
    cov = random.uniform(-0.2,0.2,(2,2))
    X = random.multivariate_normal(mean,cov,npoints)
    #X = random.normal(0,1,(200,2))
    #W = random.uniform(-1,1,(2,2))
    #b = random.uniform(-0.5, 0.5, (2,1))
    #X = dot(X,W)+b.T
    return X

def loadInputs(datapmatfile, inputsize=None):
    data = load_pmat_as_array(datapmatfile)
    if inputsize is None:
        inputsize = len(data[0])-1
    X = data[:,0:inputsize]
    return X

def getDataSet(dataset):
    """dataset is a string that specifies either a .pmat or is of the form data_seed:ngaussians"""
    data_seed = None
    try:
        data_seed, ngaussians = dataset.split(':')
        data_seed = int(data_seed)
        ngaussians = int(ngaussians)
    except ValueError:
        pass

    if data_seed is None:
        X = PMat(dataset)
        print "** flushing"
        X.flush()
        print "** flushing DONE"
        # X = loadInputs(dataset)
    else:
        random.seed(data_seed)
        if ngaussians==0:
            X = random.multivariate_normal(array([0,0]),eye(2),1000)
        elif ngaussians>0:
            pointgroups = [ generate2dNormal(200) for i in range(ngaussians) ]
            X = vstack(pointgroups)
        else: # ngaussians<0
            ngaussians = -ngaussians
            pointgroups = [ generate2dNormalAtAngle(200) for i in range(ngaussians) ]
            X = vstack(pointgroups)
            
    return X
    


####################
### main program ###

if __name__ == "__main__":

    try:
        dataset, learner_seed, ncomponents, constrain_norm_type, cov_transformation_type, diag_weight, diag_nonlinearity, diag_premul, offdiag_weight, offdiag_nonlinearity, offdiag_premul = sys.argv[1:]        

        learner_seed = int(learner_seed)
        ncomponents = int(ncomponents)
        constrain_norm_type = float(constrain_norm_type)
        diag_weight = float(diag_weight)
        diag_premul = float(diag_premul)
        offdiag_weight = float(offdiag_weight)
        offdiag_premul = float(offdiag_premul)
        



    except:
        print "Usage: "+sys.argv[0]+" dataset learner_seed ncomponents constrain_norm_type cov_transformation_type diag_weight diag_nonlinearity diag_premul offdiag_weight offdiag_nonlinearity offdiag_premul"
        print "  dataset can be either a .pmat or data_seed:ngaussians"
        print """  constrain_norm_type controls how to constrain the norms of rows of W:
        -1:constrained source;
        -2:explicit normalization;
        >0:ordinary weight decay"""
        print """  cov_transformation_type controls the kind of transformation to apply to covariance matrix
        cov: no transformation (keep covariance)
        corr: transform into correlations, but keeping variances on the diagonal.
        squaredist: do a 'squared distance kernel' kind of transformation."""
        print "Ex: "+sys.argv[0]+" 123:1    123 2    -1 cov     -1 square 1       1 square 1"
        print "Ex: "+sys.argv[0]+" 121:-2    123 4    -1 squaredist     0 exp 1       1 exp -1.6"
        raise
    # sys.exit()

    print "Getting data"
    X = getDataSet(dataset)
    print "Data OK."

    DCAExperiment(X,
                  seed=learner_seed, 
                  ncomponents=ncomponents,
                  constrain_norm_type=constrain_norm_type,
                  cov_transformation_type=cov_transformation_type,
                  diag_weight = diag_weight,
                  diag_nonlinearity = diag_nonlinearity, 
                  diag_premul = diag_premul,
                  offdiag_weight = offdiag_weight,
                  offdiag_nonlinearity = offdiag_nonlinearity, 
                  offdiag_premul = offdiag_premul,
                  lr=0.01, nsteps=1, optimizer_nsteps=10)
    

#     try:
#         datapmatfile, inputsize, filtertype, param = sys.argv[1:]
#         inputsize = int(inputsize)
#         param = float(param)
#     except:
#         print "Usage: "+sys.argv[0]+" <datafile.pmat> <inputsize> <filtertype> <param>"
#         print """
#         For filtertype 'PCA', param is the noise added to the diagonal of the covariance matrix
#         For filtertype 'denoising', param is the destruction proportion.
#         """
#         raise
#     # sys.exit()



