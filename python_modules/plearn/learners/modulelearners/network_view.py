#!/usr/bin/env python

try:
  from plearn.pyext import *
except:
  from plearn.pymake.pymake import *
  PLEARNDIR = os.environ.get('PLEARNDIR', os.getcwd())
  PLEARNDIRpyext = os.path.join(PLEARNDIR,'python_modules','plearn','pyext')
  PLEARNDIRpyextOBJ =  os.path.join(PLEARNDIRpyext,'OBJS')
  DIRS=os.listdir(PLEARNDIRpyextOBJ)
  l=len(DIRS)
  for dirname in DIRS:
      l -= 1
      if l>0 and 'double' in dirname:
         DIRS.append(dirname)
         continue
      elif l>0 and 'dbg' in dirname:
         DIRS.append(dirname)
         continue
      dirname = os.path.join(PLEARNDIRpyextOBJ, dirname)      
      if 'libplext.so' in os.listdir(dirname):
         linux_command = 'ln -sf '+ os.path.join(dirname,'libplext.so') + ' ' + os.path.join(PLEARNDIRpyext, 'libplext.so') 
         os.system(linux_command)
      try :
           from plearn.pyext import *
	   break
      except: pass
  
  



printAllPorts=False


from pyplearn_read import *

import pydot

# global variables:
modules_dict = {}

    

def countModules(mynetwork):
    n=0
    for module in mynetwork.modules:
        if isModule(module,'Split') == False:
	   n += 1
    return n

def Network2dict(mynetwork):
    mydict={}
    for module in mynetwork.modules:
        mydict[module.name]=module
    return mydict

def Modules2dict(modules):
    mydict={}
    for module in modules:
        mydict[module.name]=module
    return mydict

def getPortDict(ports):
    mydict={}
    if type(ports[0]) == tuple:
        for i in range(0,len(ports)):
	    mydict[ports[i][1].split('.')[0]] = ports[i][0]
    else:
        for i in range(0,len(ports),2):
            mydict[ports[i+1].split('.')[0]] = ports[i]
    return mydict

def getPortLists(ports):
    mylist=[]
    if type(ports[0]) == tuple:
        for i in range(0,len(ports)):
	    mylist.append( [ ports[i][1].split('.')[0] , ports[i][0] ] )
    else:
       for i in range(0,len(ports),2):
           mylist.append( [ ports[i+1].split('.')[0] , ports[i] ] )
    return mylist
    
def formatModulesNames(name,modules_dict):
    if modules_dict.has_key(name):
       return name.upper()
       #return str(type(modules_dict[name])).split("<class '")[1].split("Module")[0].upper()
    print "ERROR: while executing formatModulesNames(name):\n\tCannot find "+name+" in modules_dict"
    return name

def formatPortNames(name,portname,modules_dict,printAllPorts):
    if portname in ['input','target','cost','weight']:
       return '*'+portname.lower()+'*'
    elif portname in ['output']:
       return formatModulesNames(name,modules_dict)
    else:
       if printAllPorts:
          return '-*'+portname.lower()+'*'
       else:
          return formatModulesNames(name,modules_dict)
       

def checkName(ModuleName, ports_dict, modules_dict):
    if ports_dict.has_key(ModuleName):
       return formatPortNames(ModuleName,ports_dict[ModuleName],modules_dict,False)
    return formatModulesNames(ModuleName,modules_dict)


def isModule(module,name):
    return name+'Module' in str(type(module))
     
def module_type(module):
    return str(type(module)).upper()

def getInputOutputSize(mymodule):
    inputSize = ['?']
    outputSize = ['?']
    if 'connection' in mymodule.__dict__:
       inputSize = [mymodule.connection.down_size]
       outputSize = [mymodule.connection.up_size]
    elif 'input_size' in mymodule.__dict__:
       inputSize = [mymodule.input_size]
       outputSize = [mymodule.output_size]
    elif 'up_port_sizes' in mymodule.__dict__:
       inputSize = [mymodule.up_port_sizes]
    elif 'weights' in mymodule.__dict__:
       inputSize = [len(mymodule.weights)]
    return inputSize, outputSize 

        
def getInputOutput(connection):
    nameport = connection.source.split('.')[1]
    inputModuleName = connection.source.split('.')[0]
    outputModuleName = connection.destination.split('.')[0]
    if nameport in ['hidden','hidden.state','output','target']:
       #nameport = connection.destination.split('.')[1]
       #if nameport in ['visible','input','target']:
          nameport = ' '
    #tp, outSize = getInputOutputSize(modules_dict[inputModuleName])
    #inSize, tp  = getInputOutputSize(modules_dict[outputModuleName])
    #nameport = str(outSize) + nameport + str(inSize)
    return inputModuleName, outputModuleName, nameport

def get_graph(modules, connections, ports):
    edges=[]
    edges_toAdd={}
    tmp_dict = Modules2dict(modules)
    for key in tmp_dict:
        if globals()['modules_dict'].has_key(key) == False:
	   globals()['modules_dict'][key] = tmp_dict[key]
    ports_dict = getPortDict(ports)
    for connection in connections:
        inputModuleName , outputModuleName, inputModulePort = getInputOutput(connection)
	inputModuleNameToplot = checkName(inputModuleName,ports_dict,modules_dict)
	outputModuleNameToplot = checkName(outputModuleName,ports_dict,modules_dict)
	
	if isModule(modules_dict[inputModuleName],'Split'):
	   if edges_toAdd.has_key(inputModuleName):
	      edges_toAdd[inputModuleName][1].append(outputModuleNameToplot)
	   else:
	      edges_toAdd[inputModuleName]=[[],[]]
	      edges_toAdd[inputModuleName][1]=[outputModuleNameToplot]
	elif isModule(modules_dict[outputModuleName],'Split'):
	   if edges_toAdd.has_key(outputModuleName):
	      edges_toAdd[outputModuleName][0].append(inputModuleNameToplot)
	   else:
	      edges_toAdd[outputModuleName]=[[],[]]
	      edges_toAdd[outputModuleName][0]=[inputModuleNameToplot]
	else:
	   edges.append( [inputModuleNameToplot, outputModuleNameToplot, inputModulePort] )
	   
    for split_module in edges_toAdd:
        # split from/to a port (input, output)
	if len(edges_toAdd[split_module][0]) == 0: 
	   edges_toAdd[split_module][0]=[checkName(split_module,ports_dict,modules_dict)]
	elif len(edges_toAdd[split_module][1]) == 0:
	   edges_toAdd[split_module][1]=[checkName(split_module,ports_dict,modules_dict)]
	for i in edges_toAdd[split_module][0]:
	    for j in edges_toAdd[split_module][1]:
                edges.append( [ i, j, ' ' ])

    port_list = getPortLists(ports)

    graphSIZE = len(modules_dict)
    graph=pydot.Dot(size=str(graphSIZE)+','+str(graphSIZE))
    for edge in edges:
        myedge=pydot.Edge(edge[0],edge[1])
	myedge.set_label(edge[2])
        graph.add_edge(myedge)
    
    for module in modules:
        if isModule(module,'Network'):
	   for edge in get_graph(module.modules, module.connections, module.ports).get_edge_list():
	       graph.add_edge(edge)


    for port in port_list:
        inputPort = checkName(port[0],ports_dict,modules_dict)
#        outputPort = formatPortNames(port[0],port[1],modules_dict)
	outputPort = formatPortNames(port[0],port[1],modules_dict,printAllPorts)
	if outputPort != inputPort:
	   #graph.add_node(pydot.Node(inputPort+'->'+outputPort))
	   #for edge in edges:
	   #    for i in range(len(edge)):
	   #        if inputPort in edge[i]:
	   #           edge[i] += ' '+outputPort	          
	   myedge=pydot.Edge(outputPort,inputPort)
   	   myedge.set_arrowsize(1)
#  	   myedge.set_label('label')
           graph.add_edge(myedge)

	
    return graph
    

def save_graph( output_name, modules, connections, ports ):
    graph = get_graph( modules, connections, ports )
    graph.write_jpeg(output_name, prog='neato') #'dot', 'twopi' and 'neato'
    #import Image
    #im=Image.open(output_name,"r")
    #im.show()
    print "to see the network: kuickview "+output_name

def show_graph( modules, connections, ports ):
    output_name='temp.jpeg'
    graph = get_graph( modules, connections, ports )
    graph.write_jpeg(output_name, prog='dot')
    im=Image.open(output_name,"r")
    im.show()

def networkview( myObject ):
    if 'NetworkModule' in str(type(myObject)):
       graph = get_graph( myObject.modules, myObject.connections, myObject.ports )
    elif 'ModuleLearner' in str(type(myObject)):
       graph = get_graph( myObject.module.modules, myObject.module.connections, myObject.module.ports )
    else:
        raise TypeError, "Please give a ModuleLearner or NetworkModule"
    output_name = '/tmp/networkview.jpeg'
    graph.write_jpeg(output_name, prog='dot')
    os.system('kuickshow '+output_name+' &')

if __name__ == '__main__':
    import sys
    import os, os.path


    if len(sys.argv) <> 2:
       print "Usage:\n\tpython "+sys.argv[0]+" mylearner.ext"
       print "Purpose:\n\tDraw the graph of a network implemented/saved in a file mylearner.ext\n\twith extension (.ext) .py .pyplearn or .psave"
       sys.exit(0)
    
    inputname  = sys.argv[1]
    output_extension = '.network.jpeg'
    output_name = os.path.splitext(inputname)[0]
    input_extension = os.path.splitext(inputname)[1]
    
    
    if input_extension == '.pyplearn':
       commands_to_discard = []
       types_to_discard = ['HyperLearner', 'PTester', 'MemoryVMatrix', 'AutoVMatrix']
       tmp_file = '/tmp/tp.py'
       object_dict = write_objects_from_pyplearn( inputname, types_to_discard ,commands_to_discard, tmp_file)
#      for debug...
#       os.system('nedit '+tmp_file+' &')
       execfile(tmp_file)
       names=[]
       nets=[]
#      for debug...
#       print object_dict
       for variable_name in object_dict['NetworkModule']:
           names.append(variable_name)
	   nets.append(eval(variable_name))
	   
    elif  input_extension == '.py':  
       commands_to_discard = ['.run()', '.test()', '.train()', '.save(',
                              'choose_initial_lr', 'train_adapting_lr', 'train_with_schedule']
       types_to_discard = []
       tmp_file = '/tmp/tp.py'
       write_objects_from_pyplearn( inputname, types_to_discard ,commands_to_discard, tmp_file)
#      for debug...
#       os.system('nedit '+tmp_file+' &')
       execfile(tmp_file)
       names=[]
       nets=[]
       global_variable=''
       for global_variable in globals():
           if 'ModuleLearner' in str(type(globals()[global_variable])):
              names.append(global_variable+'.module')
              nets.append(eval(global_variable+'.module'))
       
    elif input_extension == '.psave':
       myObject = loadObject(inputname)
       if 'ModuleLearner' in str(type(myObject)):
          net = myObject.module
       else:
          raise TypeError, "Could not recognize the type "+str(type(myObject))
       nets = [net]
       
       
    else:
       raise TypeError, "could not recognize the input_extension ("+input_extension+") of "+inputname
    
    
    
    
    for i in range(len(nets)):
        net = nets[i]
        if len(nets)==1:
	   output_name_i = output_name + output_extension
	else:
	   print "making the graph of " + names[i]
	   output_name_i = output_name+'.'+names[i]+output_extension
        graph = get_graph( net.modules, net.connections, net.ports )
        graph.write_jpeg(output_name_i, prog='dot')
        if os.path.isfile(output_name_i) == False:
           print "ERROR: could not write "+output_name_i
           sys.exit(0)
        print "to see the network: kuickshow "+output_name_i
        os.system('kuickshow '+output_name_i+' &')
    
#    show_graph_network( modules, connections, ports )
