
class WrappedPLearnObject(object):

    def __init__(self, cptr):
        self._cptr= cptr # ptr to c++ obj
    
    def __setattr__(self, attr, val):
        if attr != '_optionnames' and attr in self._optionnames:
            self.changeOptions({attr: str(val)})
        else:
            object.__setattr__(self, attr, val)

    def __getattr__(self, attr):
        if attr in self._optionnames:
            return self.getOption(attr)
        else:
            raise AttributeError, ("no attribute "
                                   + attr + " in "
                                   + repr(self))
        
    def __del__(self):
        return self.unref()

    def __repr__(self):
        #s= self.__class__.__name__ + '('
        #s+= ', '.join([o+'= '+repr(self.__getattr__(o))
        #               for o in self._optionnames])
        #s+= ')'
        #return s
        return self.asString() #PLearn repr. for now
