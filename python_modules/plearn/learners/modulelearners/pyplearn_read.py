#!/usr/bin/env python

import sys, os, os.path

FID=0

class pyplearnFile:

#      Nbrack
#      Nparen
#      FID
#      cline
#      cline_previous
#      ctype
#      cname
#      islocal

      def __init__(self,fname):
            self.FID = open(fname, 'r')
	    self.cline = ''  # current line
            self.Nbrack  = [] # number of opened brackets [
            self.Nparen  = [] # number of opened parenthesis (
	    self.islocal = 0 # being in a subfunction?
	    self.ctypes = [] # type that i being defined (hierarchical)
	    self.cnames = [] # ...and corresponding names
	    self.isaffect = False # is the previous line affecting something?
      def close(self):
          self.FID.close()

      def read(self):
          self.cline_previous = self.cline
          self.cline = self.FID.readline()
	  if len(self.cline) == 0:
	     return False
	  
	  comment = self.cline.find("#")
	  if comment > -1:
   	     # The '#' may be in a string...
  	     prev = 0
 	     while is_odd(number_occurence(self.cline[prev:comment],"'")) or is_odd(number_occurence(self.cline[prev:comment],'"')):
	        prev = comment
		tp = self.cline[comment+1:].find("#")
		if tp > -1:
		   comment += tp+1 
		else:
		   comment = len(self.cline)
	     self.cline = self.cline[0:comment]+'\n'
	     
	  return True

      def actualize_previous(self):
          if is_def(self.cline):
	     self.islocal = indent_level(self.cline)
	  elif self.islocal > 0:
	     if indent_level(self.cline) <= self.islocal:
	        self.islocal = 0
	  if self.islocal == 0:
  	     closed_paren = number_occurence(self.cline_previous,")") - number_occurence(self.cline_previous,"(") + number_occurence(self.cline_previous,"]") - number_occurence(self.cline_previous,"[")
	     if closed_paren == 0 and self.isaffect:
	        self.isaffect = False
		closed_paren = 1
	     self.cnames = self.cnames[0:len(self.cnames)-closed_paren]
	     self.ctypes = self.ctypes[0:len(self.ctypes)-closed_paren]

	     being_affect = self.name_affect()
	     if len(being_affect[0]) > 0:
	        self.cnames.append(being_affect[0])
		self.ctypes.append(being_affect[1])
	  
          self.Nbrack += number_occurence(self.cline,"[") -  number_occurence(self.cline,"]")
          self.Nparen += number_occurence(self.cline,"(") -  number_occurence(self.cline,")")

      def actualize(self):
          if is_def(self.cline):
	     self.islocal = indent_level(self.cline)
	  elif self.islocal > 0:
	     if indent_level(self.cline) <= self.islocal:
	        self.islocal = 0
	  if self.islocal == 0:
	     assend = 0
	     level = len(self.Nbrack)-1
	     N = 0
	     while N == 0 and level >= 0:
  	        if len(self.Nbrack) > 0:
	           N = self.Nbrack[level]
                else:
	           N = 0
	        if len(self.Nparen) > 0:
	           N += self.Nparen[level]
                if N > 0:
		   break
	        assend += 1
		level  -= 1
		N -= N
	     
             self.cnames = self.cnames[0:len(self.cnames)-assend]
             self.ctypes = self.ctypes[0:len(self.ctypes)-assend]
             self.Nbrack = self.Nbrack[0:len(self.Nbrack)-assend]
             self.Nparen = self.Nparen[0:len(self.Nparen)-assend]

	     being_affect = self.name_affect()
	     if len(being_affect[0]) > 0:
	        self.cnames.append(being_affect[0])
		self.ctypes.append(being_affect[1])
		self.Nbrack.append(number_occurence(self.cline,"[") -  number_occurence(self.cline,"]"))
		self.Nparen.append(number_occurence(self.cline,"(") -  number_occurence(self.cline,")"))
             elif len(self.Nbrack) > 0:
	        self.Nbrack[len(self.Nbrack)-1] += number_occurence(self.cline,"[") -  number_occurence(self.cline,"]")
	        self.Nparen[len(self.Nparen)-1] += number_occurence(self.cline,"(") -  number_occurence(self.cline,")")


       # is there an affectation in the line?
      def name_affect(self):
          tp = self.cline.split('=')
          if len(tp) > 1:
             if '' not in tp: # discard the case of '=='
	        tp[1] = '='.join(tp[1:])
                first_term = tp[0].strip()
                second_term = tp[1].split('(')[0].strip()
		if len(second_term) == 0:
		   while len(second_term) < 1:
		         line = self.cline
		         self.read()
			 self.cline = line + self.cline
		         temp, second_term = self.name_affect()
		   return first_term, second_term
		self.isaffect = True
                if len(second_term) > 3 and second_term[0:3] == 'pl.':
                   return first_term, second_term[3:]
		elif '[' in second_term:
		   return first_term, 'list'
		else:
		   self.isaffect = False
		   return '', type(second_term)
          self.isaffect = False
	  return '',''

# is it a definition line?
def is_def(line):
    tp = line.split('def')
    if len(tp) > 1:
       if len(tp[1]) > 0:
          if len(tp[0]) == 0 and tp[1][0] == ' ':
             return True
    return False



# the level of indentation of the line    
def indent_level(line):
    return line.replace("\t","        ").find(line.lstrip()) + 1

def number_occurence(line,string):
    n = 0
    while line.find(string) > -1  and len(string) > 0:
          n += 1
          line = line[line.find(string)+1:]
    return n
    
def is_odd(number): # nombre impair?
    return int(number/2.0) != number/2.0
    
    
    
    
	  
# fname           : name of the *.pyplearn or *.py file to read
# types_to_discard : list of module names we don't want to construct
#                     (ex: ['ModuleLearner', 'NetworkModule', 'MemoryVMatrix', 'AutoVMatrix'])
#                   but we will get their components
def write_objects_from_pyplearn( fname, types_to_discard, commands_to_discard, fname_out):
    if os.path.isfile(fname) == False:
       print "ERROR: cannot find file "+fname
       sys.exit(0)
       
    module_dict={}
    
    FILE = pyplearnFile(fname)
    
    globals()['FID'] = open(fname_out,'w')
    FID=globals()['FID']
    
    cnames=[]
    
    while FILE.read():
       FILE.actualize()
	  
       if len(FILE.ctypes) == 0 or FILE.ctypes[0] not in types_to_discard:
          
	  do_not_consider = False
	  for command in commands_to_discard:
	      if command in FILE.cline:
	         do_not_consider = True
	  if do_not_consider:
	     continue
	  
	  if cnames != FILE.cnames:
	     if len(cnames) < len(FILE.cnames):
	        cname = '.'.join(FILE.cnames)
		ctype = FILE.ctypes[len(FILE.ctypes)-1]
                if module_dict.has_key(ctype):
		   if cname not in module_dict[ctype]:
		      module_dict[ctype].append( cname )
		else:
		   module_dict[ctype] = [ cname ]
	        FID.write("#"+"/"*10*len(FILE.cnames)+"\n")
                FID.write("# "+' / '.join(FILE.cnames)+"\n# "+' / '.join(FILE.ctypes)+"\n")
	     else: 
                FID.write("# "+' / '.join(FILE.cnames)+"\n")
		if len(cnames) != len(FILE.cnames):
		   FID.write("# "+' / '.join(FILE.ctypes)+"\n")
	           FID.write("#"+"\\"*10*len(FILE.cnames)+"\n")
	     cnames = FILE.cnames
	  FID.write(FILE.cline)
   
    FILE.close()
    FID.close()
    
    return module_dict
    
if __name__ == '__main__':

    object_dict = write_objects_from_pyplearn( sys.argv[1], ['HyperLearner', 'PTester', 'MemoryVMatrix'] , [] , '/tmp/pyplearn_read.py')
    print str(object_dict)
    execfile('/tmp/pyplearn_read.py')
    for i in object_dict['NetworkModule']:
        m = eval(i)
        print m
