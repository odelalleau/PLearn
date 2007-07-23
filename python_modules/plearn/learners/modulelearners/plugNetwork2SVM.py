import sys, os, os.path
from plearn.learners.modulelearners  import *
from plearn.learners.SVM import *

if __name__ == '__main__':

    if len(sys.argv) <= 5:
       print "Usage:\n\tpython "+sys.argv[0]+" learner train_data valid_data test_data ports_list\n"
       print "Example:\n\tpython "+sys.argv[0]+" 3layer_DBN.psave trainData.dmat validData.dmat testData.dmat rbm3.hidden.state [rbm2.hidden.state ...]\n"
       sys.exit(0)
    
    learner_filename       = sys.argv[1]
    dataTrain_filename = sys.argv[2]
    dataValid_filename = sys.argv[3]
    dataTest_filename  = sys.argv[4]
    ports_list    = sys.argv[5:]

    if os.path.isfile(learner_filename) == False and os.path.isdir(learner_filename) == False:
       raise EOFError, "ERROR : Learner file cannot be find\n\tCould not find file "+learner_filename
    learner = loadModuleLearner(learner_filename)
    learner_nickname = os.path.basename(learner_filename)+"_".join(ports_list).replace(".","")

    result_dir = os.path.dirname(learner_filename)
    output_filename = result_dir+'/SVM_results_'+"_"+learner_nickname+"-"+os.path.basename(dataTrain_filename).replace(".vmat","").replace(".amat","")
    print
    print "Results will be written in "+output_filename

#               #
#   MAIN PART   #
#               #


    new_learner = plug2output( learner, ports_list)

    
  
    for typeDataSet in ['Train','Valid','Test']:
        data_filename = globals()['data'+typeDataSet+'_filename']
        if os.path.isfile(data_filename) == False and os.path.isdir(data_filename) == False:
           raise EOFError, "Could not find "+data_filename
           sys.exit(0)
	dataSet = pl.AutoVMatrix( filename = data_filename )

        print "CONVERSION "+data_filename
        globals()[typeDataSet+'_outputs'], globals()[typeDataSet+'_targets'] = computeOutputsTargets( new_learner, dataSet)
	#
	# Normalizing the data (/!\ compute statistics on the training data and assumes it comes first)
	#
        if typeDataSet == 'Train':
           mean, std = normalize(globals()[typeDataSet+'_outputs'],None,None)
	else:
	   normalize(globals()[typeDataSet+'_outputs'],mean,std)

    my_SVM = SVM()
    
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
       FID.close()

    # A log file where all the intermediate results will be stored
    my_SVM.save_filename = output_filename
    
    # Trying the linear kernel
    # with several values for C (i.e. bias-variance trade-off in SVM)
    #
    my_SVM.train_and_tune( 'LINEAR' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets] ])
    best_valid_error_rate = my_SVM.valid_error_rate
    print
    print "Tried parameters : "+str(my_SVM.tried_parameters)
    print 'BEST ERROR RATE: '+str(best_valid_error_rate)+' (valid) for '+str(my_SVM.best_parameters)
    
    
    # Trying the RBF (Gaussian) kernel
    # with several values for C and 'gamma' (kernel width)
    #
#    my_SVM.train_and_tune( 'RBF' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets] ])
    best_valid_error_rate = my_SVM.valid_error_rate
    print
    print "Tried parameters : "+str(my_SVM.tried_parameters)
    print 'BEST ERROR RATE: '+str(best_valid_error_rate)+' (valid) for '+str(my_SVM.best_parameters)
    
    # Trying the RBF kernel once more
    # i.e. more precise tuning
    # with more values for C and 'gamma' (kernel width)
    #
#    my_SVM.train_and_tune( 'RBF' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets] ])
    best_valid_error_rate = my_SVM.valid_error_rate
    print
    print "Tried parameters : "+str(my_SVM.tried_parameters)
    print 'BEST ERROR RATE: '+str(best_valid_error_rate)+' (valid) for '+str(my_SVM.best_parameters)

    # Trying the polynomial kernel
    # with several values for C and the degree
    #
#    my_SVM.train_and_tune( 'POLY' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets] ])
    best_valid_error_rate = my_SVM.valid_error_rate
    print
    print "Tried parameters : "+str(my_SVM.tried_parameters)
    print 'BEST ERROR RATE: '+str(best_valid_error_rate)+' (valid)  for '+str(my_SVM.best_parameters)

    my_SVM.test( [Test_outputs,Test_targets] )

    test_error_rate = my_SVM.error_rate
    print "Test ERROR RATE with best model : "+str(test_error_rate)

    print
    print "Results written in "+output_filename
