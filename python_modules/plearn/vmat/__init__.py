
pyplearn_import_failed = False
try:
    from plearn.pyplearn import pl
except ImportError:
    pyplearn_import_failed = True

if not pyplearn_import_failed:
    __all__ = [ "AutoVMatrix", "vmat" ]
    
    def AutoVMatrix( specification, inputsize=-1, targetsize=-1, weightsize=-1 ):
        return pl.AutoVMatrix( specification = specification,
                               inputsize     = inputsize, 
                               targetsize    = targetsize,
                               weightsize    = weightsize   )
    
    def vmat( vmatrix ):
        ## Consider it as a path.
        if isinstance( vmatrix, str ):
            return AutoVMatrix( vmatrix )
    
        ## For now, consider it as a snippet.
        return vmatrix
