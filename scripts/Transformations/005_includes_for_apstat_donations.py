def transformfunc(fname, text):
    text = re.sub(r'apstatlib/hyper/', 'plearn_learners/hyper/', text)
    text = re.sub(r'apstatlib/vmat/TextFilesVMatrix', 'plearn/vmat/TextFilesVMatrix', text)
    text = re.sub(r'apstatlib/learners/UniformizeLearner', 'plearn_learners/unsupervised/UniformizeLearner', text)    
    text = re.sub(r'commands/ApstatCommands/TxtmatCommand','commands/PLearnCommands/TxtmatCommand',text)
    text = re.sub(r'\"ApstatCommands/TxtmatCommand.h\"','<commands/PLearnCommands/TxtmatCommand.h>',text)
    return text
    
