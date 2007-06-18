import sys, os, os.path
import plearn.bridgemode
plearn.bridgemode.interactive = False
plearn.bridgemode.useserver= False
from plearn.bridge import *
#from plearn.pyplearn import *

from plearn.learners.autolr import deepcopy


tmp_file='/tmp/modulelearner.py'

def dirtydeepcopy( myObject ):
    myObject.save(tmp_file,'plearn_ascii')
    return loadObject(tmp_file)

if plearn.bridgemode.useserver:
    learner  = serv.new(learner)
    trainset = serv.new(trainset)
    validset = serv.new(validset)
    testset  = serv.new(testset)


# examples:
#   isModule(module,'RBM')
#   isModule(module,'Split')
#
def isModule(module,name):
    return name+'Module' in str(type(module))


# Handling files...

def loadNetworkModule(filename):
    modulelearner = loadModuleLearner(filename)
    if 'module' not in modulelearner._optionnames:
       raise TypeError, "ModuleLearner has no option module"
    if 'NetworkModule' not in str(type(modulelearner.module)):
       raise TypeError, "ModuleLearner.module is not a NetworkModule"
    return modulelearner.module
    

def loadModuleLearner(filename):
    if os.path.isfile(filename) == False:
       raise TypeError, "could not find the file "+filename
    extension = os.path.splitext(filename)[1]
    
    if extension == '.pyplearn':
       from plearn.learners.modulelearners.pyplearn_read import *
       object_dict = read_objects( filename, ['HyperLearner', 'PTester', 'MemoryVMatrix', 'AutoVMatrix'] , tmp_file)
       execfile(tmp_file)
       modules=[]
       for i in object_dict['ModuleLearner']:
           modules += eval(i)
       if len(modules) == 0:
          raise TypeError, "could not find a NetworkModule in "+filename
       if len(modules) == 1:
          return modules[0]
       else:
          print "WARNING: found several NetworkModules in "+filename+" (a list is returned)"
	  return modules
	  
    elif extension == '.psave':
       myObject = loadObject(filename)
       if 'ModuleLearner' in str(type(myObject)):
          return myObject
       elif 'HyperLearner' in str(type(myObject)):
          if 'ModuleLearner' in str(type(myObject.learner)):
	     return myObject.learner
	  else:
	     raise TypeError, "The learner of HyperLearner in "+filename+" has type "+str(type(myObject.learner))
       else:
          raise TypeError, "Could not recognize the type "+str(type(myObject))

    else:
       raise TypeError, "could not recognize the extension ("+extension+") of "+filename

# String tools...

def port2moduleName( portName ):
    return portName.split('.')[0]


# Getting information about the network...


def getModules( myObject ):
    # Network module
    if 'NetworkModule' in str(type(myObject)):
       return copy.copy( myObject.modules )
    # Module Learner
    elif 'ModuleLearner' in str(type(myObject)):
       return copy.copy( myObject.module.modules )
    # List of modules
    elif type(myObject) == list and 'Module' in str(type(myObject[0])):
         return myObject
    else:
        raise TypeError, "Please give a ModuleLearner or NetworkModule"

def setModules( myObject , new_connections_list):
    if 'NetworkModule' in str(type(myObject)):
       myObject.setOptionFromPython('modules',new_connections_list)
    elif 'ModuleLearner' in str(type(myObject)):
       myObject.module.setOptionFromPython('modules',new_connections_list)
    else:
        raise TypeError, "Please give a ModuleLearner or NetworkModule"


def getModulesNames( myObject ):
    modules_names=[]
    for module in getModules(myObject):
        modules_names.append(module.name)
    return modules_names


def getModule( myObject, modulename ):
    modules_names = getModulesNames( myObject )
    if modulename not in modules_names:
       raise TypeError, "Could not find module "+modulename
    index = modules_names.index( modulename )
    # Network module
    if 'NetworkModule' in str(type(myObject)):
       return deepcopy( myObject.modules[index] )
    # Module Learner
    elif 'ModuleLearner' in str(type(myObject)):
       return deepcopy( myObject.module.modules[index] )
    # List of modules
    elif type(myObject) == list and 'Module' in str(type(myObject[0])):
       return myObject[index]
    else:
       raise TypeError, "Please give a ModuleLearner or NetworkModule"
    
def getConnections( myObject ):
    if 'NetworkModule' in str(type(myObject)):
       return copy.copy( myObject.connections )
    elif 'ModuleLearner' in str(type(myObject)):
       return copy.copy( myObject.module.connections )
    else:
        raise TypeError, "Please give a ModuleLearner or NetworkModule"

def setConnections( myObject , new_connections_list):
    if 'NetworkModule' in str(type(myObject)):
       myObject.setOptionFromPython('connections',new_connections_list)
    elif 'ModuleLearner' in str(type(myObject)):
       myObject.module.setOptionFromPython('connections',new_connections_list)
    else:
        raise TypeError, "Please give a ModuleLearner or NetworkModule"

def getPorts( myObject ):
    if 'NetworkModule' in str(type(myObject)):
       return copy.copy( myObject.ports )
    elif 'ModuleLearner' in str(type(myObject)):
       return copy.copy( myObject.module.ports )
    else:
        raise TypeError, "Please give a ModuleLearner or NetworkModule"

def setPorts( myObject , new_connections_list):
    if 'NetworkModule' in str(type(myObject)):
       myObject.setOptionFromPython('ports',new_connections_list)
    elif 'ModuleLearner' in str(type(myObject)):
       myObject.module.setOptionFromPython('ports',new_connections_list)
    else:
        raise TypeError, "Please give a ModuleLearner or NetworkModule"

def get_last_layer_module_name( learner ):
    last_layer = []
    output_layer = getOutputModuleName(learner)
    connections_list = getConnections(learner)
    for connection in connections_list:
        if output_layer in connection.destination:
	   last_layer.append( port2moduleName( connection.source ) )
    if len(last_layer) != 1:
       raise TypeError, "find several layers before output layer\n(" + str(last_layer) + ")"
    return last_layer[0]

def get_last_layer_module( learner ):
    last_module_name = get_last_layer_module_name(learner)
    modules_list = getModules( learner )
    for module in modules_list:
        if module.name == last_module_name:
	   return module

def getOutputModuleName( learner ):
    outputPort = getOutputPort(learner)
    return port2moduleName(outputPort[1])

def getOutputPort( learner ):
    ports_list = getPorts(learner)
    for port in ports_list:
        if 'output' in port:
           return port
    raise 'cannot find output port of learner'

def getCostModuleName( learner ):
    costPort = getCostPort(learner)
    return port2moduleName(costPort[1])

def getCostPort( learner ):
    cost_port = learner.cost_ports[0]
    ports_list = getPorts(learner)
    for port in ports_list:
        if cost_port in port:
           return port
    raise 'cannot find output port of learner'


def getSourcePorts( learner, modulename ):
    inputports = []
    for connection in getConnections(learner):
        if modulename in connection.destination:
	   inputports.append( connection.source )
    return inputports

def getAllPrevious( myObject, modulename ):
    connections_list = getConnections(myObject)
    previous_connections = []
    previous_modulenames = []
    for connection in connections_list:
        if modulename in connection.destination:
	   previous_connections.append(connection)
	   previous_modulenames.append(port2moduleName(connection.source))
    for sourcename in previous_modulenames:
        connections, sources = getAllPrevious(myObject, sourcename)
        for connection in connections:
	    if connection not in previous_connections:
               previous_connections.append(connection)
	       source = port2moduleName(connection.source)
	       if source not in previous_modulenames:
	          previous_modulenames.append(source)
    return previous_connections, previous_modulenames

def getAllPreviousConnection( myObject, modulename ):
    previous_connections, previous_modulenames = getAllPrevious( myObject, modulename )
    return previous_connections


def getPrevious( myObject, modulename ):
    connections_list = getConnections(myObject)
    previous_connections = []
    previous_modulenames = []
    for connection in connections_list:
        if modulename in connection.destination:
	   previous_connections.append(connection)
	   previous_modulenames.append(port2moduleName(connection.source))
    return previous_connections, previous_modulenames

def getPreviousConnection( myObject, modulename ):
    connections_list = getConnections(myObject)
    previous_connections = []
    for connection in connections_list:
        if modulename in connection.destination:
	   previous_connections.append(connection)
    return previous_connections

def getAllNext( myObject, modulename ):
    connections_list = getConnections(myObject)
    next_connections = []
    next_modulenames = []
    for connection in connections_list:
        if modulename in connection.source:
	   next_connections.append(connection)
	   next_modulenames.append(port2moduleName(connection.destination))
    for sourcename in next_modulenames:
        connections, sources = getAllNext(myObject, sourcename)
        for connection in connections:
	    if connection not in next_connections:
               next_connections.append(connection)
	       source = port2moduleName(connection.destination)
	       if source not in next_modulenames:
	          next_modulenames.append(source)
    return next_connections, next_modulenames

def getNext( myObject, modulename ):
    connections_list = getConnections(myObject)
    next_connections = []
    next_modulenames = []
    for connection in connections_list:
        if modulename in connection.source:
	   next_connections.append(connection)
	   next_modulenames.append(port2moduleName(connection.destination))
    return next_connections, next_modulenames


def removeLastModule( learner , modulename ):
    return removeModule( learner , getOutputModuleName(learner) )

def removeModuleFromNetwork( network , modulename ):
    modules_name_list = getModulesNames(network)
    if modulename not in modules_name_list:
       raise TypeError, "Module's name "+modulename+" not found in ModuleLearner" 

    new_network = deepcopy(network)

    connections_list     = getConnections(network)
    new_connections_list = getConnections(network)
    ports_list     = getPorts(network)
    new_ports_list = getPorts(network)
    modules_list     = getModules(network)
    new_modules_list = getModules(network)


           
    # Creating the list of top modules to remove
    # (looking at the connections)
    #        
    connections_to_reject, modules_to_reject = getAllNext( network, modulename )
    modules_to_reject.append(modulename)
    #connections_to_reject.append(getPreviousConnection(network, modulename))

    # Rejecting modules
    #
    for module in modules_list:
        if module.name in modules_to_reject:
	   new_modules_list.remove(module)


    # Rejecting connections
    #
    for module_being_rejected in modules_to_reject:
          for connection in connections_list:
              if module_being_rejected in connection.source+" "+connection.destination and connection in new_connections_list:
                 new_connections_list.remove(connection)
		 
    # Rejecting ports
    #
    new_output = None
    for port in ports_list:
        if port2moduleName(port[1]) in modules_to_reject:
           new_ports_list.remove(port)
           if port[0] in ['output']:
	      module_being_rejected = port2moduleName(port[1])
	      print module_being_rejected
	      #
	      # Getting the inputs (to connect to ports)
              #
              if new_output == None:
	         new_outputs = getSourcePorts( network, module_being_rejected )
		 test_new_outputs = True
		 while test_new_outputs:
		       test_new_outputs = False
		       for checked_output in new_outputs:
		           checked_module = port2moduleName(checked_output)
		           if checked_module in modules_to_reject:
			      new_outputs.remove(checked_output)
			      new_outputs += getSourcePorts( network, checked_module )
		              test_new_outputs = True
			      break
		 print new_outputs
                 if len(new_outputs)>1:
	            output_sizes = []
		    name_join = '_'+port[0]
	            for i in range(len(new_outputs)):
		        port_tmp = new_outputs[i]
		        new_connections_list.append(pl.NetworkConnection(source = port_tmp,
		   			                                 destination = name_join+'.in'+str(i),
		   				                         propagate_gradient = connections_to_reject[0].propagate_gradient
		   				   ))
		        module = learner.module.modules[modules_name_list.index(port2moduleName(port_tmp))]
		        output_size = module.output_size
		        if output_size == -1:
		           if 'connection' in module._optionnames:
			      output_size = module.connection.output_size
			   else:
			      raise TypeError, "could not find outputsize of module "+module.name
                    new_modules_list.append(pl.SplitModule(
							name = name_join,
							down_port_name = 'output',
							up_port_names = ['in'+str(j) for j in range(len(new_outputs))],
							up_port_sizes = output_sizes
                                        ))
                    new_output = name_join+'.output'
                 else:
                    new_output = new_outputs[0]
	      new_port=(port[0],new_output)
	      new_ports_list.append(new_port)

	   
    setConnections(new_network, new_connections_list)
    setPorts(new_network, new_ports_list)
    setModules(new_network, new_modules_list)
#    new_network.setOptionFromPython('connections',new_connections_list)
#    new_network.setOptionFromPython('ports',new_ports_list)
#    new_network.setOptionFromPython('modules',new_modules_list)
    return dirtydeepcopy( new_network )
    return new_network
    

def removeModuleFromLearner( learner , modulename ):
    new_learner = deepcopy( learner )
    new_network = removeModuleFromNetwork( learner.module , modulename )
    new_ports_list = [ port[0] for port in getPorts(new_network)  ]
    
    print new_ports_list

    # Rejecting ports
    #
    for port_option_name in ['target_ports', 'cost_ports', 'weight_ports']:
        output_ports_list     = copy.copy( learner.getOption(port_option_name) )
	new_output_ports_list = copy.copy( learner.getOption(port_option_name) )
        for port in output_ports_list:
            if port not in new_ports_list:
	       new_output_ports_list.remove(port)
        new_learner.setOptionFromPython(port_option_name, new_output_ports_list)
    new_learner.setOptionFromPython('module',new_network)

    return dirtydeepcopy( new_learner )
    return new_learner


def plug2output(myObject, portslist):
    output_port_tuple = getOutputPort(myObject)
    output_port = output_port_tuple[1]
    if len(portslist) == 1 and portslist[0] == output_port:
       print "WARNING: plug2output return the same myObject"
       return myObject
       
    mynewObject = deepcopy(myObject)
    new_ports_list = getPorts(myObject)
    new_ports_list.remove(output_port_tuple)
    if len(portslist) == 1:
       new_ports_list.append((output_port_tuple[0],portslist[0]))
       setPorts(mynewObject, new_ports_list)
       return dirtydeepcopy( mynewObject )
       return mynewObject
    new_modules_list = getModules(myObject)
    new_connections_list = getConnections(myObject)
    name_join = "_".join(portslist).replace('.','')
    output_sizes = []
    modules_name_list = getModulesNames(myObject)
    for i in range(len(portslist)):
        port = portslist[i]
        new_connections_list.append(pl.NetworkConnection(source = port,
		   			                 destination = name_join+'.in'+str(i),
		   				         propagate_gradient = 0
		   		    ))
        module = myObject.module.modules[modules_name_list.index(port2moduleName(port))]
        output_size = module.output_size
	optionnames = module._optionnames
        if 'connection' in optionnames: # RBM
           output_size = module.connection.output_size
	elif 'up_port_sizes' in optionnames: # splitModule
           output_size = module.up_port_sizes[module.up_port_names.index(port.split('.')[1])]
	else:
	   output_size = module.output_size
	   if output_size == -1:      
              raise TypeError, "could not find outputsize of module "+module.name
	output_sizes.append(output_size)
    new_modules_list.append(pl.SplitModule(
							name = name_join,
							down_port_name = 'output',
							up_port_names = ['in'+str(j) for j in range(len(portslist))],
							up_port_sizes = output_sizes
                           ))
    new_ports_list.append((output_port_tuple[0],name_join+'.output'))
    setConnections(mynewObject, new_connections_list)
    setPorts(mynewObject, new_ports_list)
    setModules(mynewObject, new_modules_list)
    return dirtydeepcopy( mynewObject )
    return mynewObject




def mean_std(data):
    stds=[get_std_cmp(data,i) for i in range(len(data[0]))]
    return sum(stds)/len(stds)
def get_std_cmp(data,i):
    values=[vec[i] for vec in data]
    tot = sum(values)
    avg = tot*1.0/len(values)
    sdsq = sum([(i-avg)**2 for i in values])
    return (sdsq*1.0/(len(values)-1 or 1))**.5
def proposed_gamma(samples):
          dim = len(samples[0])
	  std = mean_std(samples)
	  rho=sqrt(dim)*std
          return 1/(2*rho**2)
	  
def compute_entries_SVM_write_libSVM_file( learner, dataSet, output_filename ):
    if os.path.isfile( output_filename ):
       print "WARNING: file "+output_filename+"already exists"
       pass
    temp_file='/tmp/new_learner.psave'
    learner.save(temp_file,'plearn_ascii')
    learner = loadObject(temp_file)
    connect_last_layer2outputs( learner )
    learner.save(temp_file,'plearn_ascii')
    learner = loadObject(temp_file)
    outputs=make_test_output(learner,dataSet)
    nsamples = dataSet.length
    outputsize = len(outputs[0])
    if nsamples != len(outputs):
       raise "ERROR: conflict in dimensions"
    else:
       print str(nsamples)+" samples, new input size ="+str(outputsize)
    if dataSet.targetsize != 1:
       raise "ERROR: target size not equal to 1 in "+data_filename
    dim = len(dataSet.getRow(0))-1
    data = [ dataSet.getRow(i)[0:dim] for i in range(dataSet.length) ]
    print "SUGGESTED gamma: "+str(proposed_gamma(data))
    FID = open(output_filename, 'w')
    for i in range(nsamples):
        X = dataSet.getRow(i)
        target = X[dataSet.inputsize]
        FID.write(str(int(target))+" ")
	for j in range(outputsize):
            FID.write(str(j+1)+":"+str(output[i][j])+" ")
	FID.write("\n")
    FID.close()



def compute_entries_SVM( learner, dataSet):
    temp_file='/tmp/new_learner.psave'
#    learner.save(temp_file,'plearn_ascii')
#    learner = loadObject(temp_file)   
    learner = deepcopy(learner)
    connect_last_layer2outputs( learner )
    learner.save(temp_file,'plearn_ascii')
    learner = loadObject(temp_file)
    outputs=make_test_output(learner,dataSet)
    nsamples = dataSet.length
    outputsize = len(outputs[0])
    if nsamples != len(outputs):
       raise "ERROR: conflict in dimensions"
    else:
       print str(nsamples)+" samples, new input size ="+str(outputsize)
    if dataSet.targetsize != 1:
       raise "ERROR: target size not equal to 1 in "+data_filename
    return outputs, [ dataSet.getRow(i)[dataSet.inputsize] for i in range(nsamples) ]

def computeOutputsTargets(learner,dataSet):
    ts=pl.VecStatsCollector()
    (stats,outputs,costs)=learner.test(dataSet,ts,True,False)
    targets=pl.SelectColumnsVMatrix(
       source = dataSet.getObject(),
       inputsize=0,
       targetsize = dataSet.targetsize,
       weightsize=0,
       indices = range(dataSet.inputsize, dataSet.inputsize+dataSet.targetsize)
    ).getMat()
    return outputs, [int(target) for target in targets]

    
if __name__ == '__main__':

    from plearn.learners.discr_power_SVM import *
    from plearn.learners.modulelearners.network_view import *

    learner_filename = "/u/louradoj/PRGM/blocksworld/res/textual_v2/BESTdbn/final_learner.psave"
    if os.path.isfile(learner_filename) == False:
       raise TypeError, "ERROR : Learner file cannot be find\n\tCould not find file "+learner_filename
    learner = loadModuleLearner(learner_filename)
        
    #networkview( learner )

    new_learner = removeModuleFromLearner( learner, "rbm_21" )
    networkview( new_learner )

    raise TypeError, "OK"

    data_filename = "/cluster/opter/data/babyAI/textual_v2/BABYAI_gray_10x2obj_32x32.color-size-location-shape.3gram.amat"
    if os.path.isfile(data_filename) == False:
       raise TypeError, "ERROR : Data file cannot be find\n\tCould not find file "+data_filename
    dataSet = pl.AutoVMatrix( filename = data_filename )
    
    new_learner = plug2output( learner, ['split.out1', 'rbm_3.hidden.state', 'rbm_12.hidden.state'])
    getModulesNames(new_learner)  
    outputs, targets = computeOutputsTargets( new_learner, dataSet)
    #networkview( new_learner )
    

#    learner_filename = "/u/louradoj/PRGM/blocksworld/res/textual_v2/BESTdbn/final_learner.psave"
#    learner = loadObject(learner_filename)
#    data_filename = '/cluster/opter/data/babyAI/textual_v2/BABYAI_gray_10x2obj_32x32.color-size-location-shape.3gram.amat'
#    dataSet = pl.AutoVMatrix( filename = data_filename )
    
#    outputs2, targets2 = compute_entries_SVM( learner, dataSet)
#    outputs3, targets3 = compute_entries_SVM( learner, dataSet)    
#    raise TypeError, "douuuu"

    dataPath='/u/lisa/db/babyAI/textual_v2/BACKUP' #'/cluster/opter/data/babyAI/textual_v2/'
    
        
    
    dataTrain_filename = dataPath+'/BABYAI_gray_10000x2obj_32x32.color-size-location-shape.train.3gram.vmat'
    dataValid_filename = dataPath+'/BABYAI_gray_5000x2obj_32x32.color-size-location-shape.valid.3gram.vmat'
    dataTest_filename = dataPath+'/BABYAI_gray_5000x2obj_32x32.color-size-location-shape.test.3gram.vmat'
    result_dir = os.path.dirname(learner_filename)

    if 'ModuleLearner' not in str(type(learner)):
        if learner.hasOption('learner') and 'ModuleLearner' in str(type(learner.learner)):
	   learner=learner.learner
        else:
           raise TypeError,  'Sorry, but this code can only be used with ModuleLearner !!!'
    learner_nickname = 'DBN-2-2-1_'+get_last_layer_module_name( learner ) #os.path.basename(learner_filename)
    
  
    for typeDataSet in ['Train','Valid','Test']:
        data_filename = globals()['data'+typeDataSet+'_filename']
        if os.path.isfile(data_filename) == False:
           raise TypeError, "ERROR : Data file cannot be find\n\tCould not find file "+data_filename
        print "CONVERSION "+data_filename
	dataSet = pl.AutoVMatrix( filename = data_filename )
        globals()[typeDataSet+'_outputs'], globals()[typeDataSet+'_targets'] = compute_entries_SVM( learner, dataSet)

    
    E=discr_power_SVM_eval()
    output_filename = result_dir+'/SVM_results_'+"_"+learner_nickname+os.path.basename(data_filename).replace(".vmat","").replace(".amat","")
    
    print "Writing results in "+output_filename
    if os.path.isfile(output_filename):
       print "WARNING : output "+output_filename+" already exists"
       FID = open(output_filename, 'a')
       abspath = os.path.realpath(learner_filename)
       FID.write('LEARNER.: '+abspath+'\n')
       for i in range(3):
           abspath = os.path.dirname(abspath)
       global_results = abspath+'/global_stats.pmat'
       if os.path.isfile(global_results):
          os.system("echo   baseline test error rate : `plearn vmat cat "+global_results+" | tail -1 | awk '{print $NF}'` \%   >> "+output_filename )
       else:
          print "WARNING : could not find global_stats.pmat\n\t( "+abspath+"/global_stats.pmat )"
       FID.write('Train...: '+os.path.realpath(dataTrain_filename)+'\n')
       FID.write('Valid...: '+os.path.realpath(dataValid_filename)+'\n')
       FID.write('Test....: '+os.path.realpath(dataTest_filename)+'\n')
    else:
       FID = open(output_filename, 'w')
    FID.write('--------\n')

    E.save_filename = output_filename
    E.valid_and_compute_accuracy( 'RBF' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets], [Test_outputs,Test_targets]])
    FID = open(output_filename, 'a')
    FID.write("Tried parameters : "+str(E.tried_parameters)+'\n')
    FID.write('BEST ACCURACY: '+str(E.valid_accuracy)+' (valid) - '+str(E.accuracy)+' (test) for '+str(E.best_parameters)+'\n')
    FID.close()
    E.valid_and_compute_accuracy( 'RBF' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets], [Test_outputs,Test_targets]])
    FID = open(output_filename, 'a')
    FID.write("Tried parameters : "+str(E.tried_parameters)+'\n')
    FID.write('BEST ACCURACY: '+str(E.valid_accuracy)+' (valid) - '+str(E.accuracy)+' (test) for '+str(E.best_parameters)+'\n')
    FID.close()
    E.valid_and_compute_accuracy( 'RBF' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets], [Test_outputs,Test_targets]])
    FID = open(output_filename, 'a')
    FID.write("Tried parameters : "+str(E.tried_parameters)+'\n')
    FID.write('BEST ACCURACY: '+str(E.valid_accuracy)+' (valid) - '+str(E.accuracy)+' (test) for '+str(E.best_parameters)+'\n')
    FID.close()
    print "Results written in "+output_filename
