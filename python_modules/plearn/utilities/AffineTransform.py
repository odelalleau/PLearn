from   plearn.pyplearn.PyPLearnObject import *

__all__ = [ "AffineTransform" ]

class AffineTransform( PyPLearnObject ):
    """Emulates an affine transformation of any type of object."""
    
    def __init__(self, obj, a=1, b=0):
        PyPLearnObject.__init__( self, obj = obj, a = a, b = b )

    def as_tuple(self):
        return (self.obj, self.a, self.b)

    def __add__(self, val):
        if not operator.isNumberType(val):
            raise TypeError
        
        return AffineTransform( self.obj,
                                self.a,
                                self.b + val   )

    def __iadd__(self, val):
        if not operator.isNumberType(val):
            raise TypeError

        self.b += val
        return self
    
    def __mul__(self, val):
        if not operator.isNumberType(val):
            raise TypeError
        
        return AffineTransform( self.obj,
                                self.a * val,
                                self.b * val   )

    def __imul__(self, val):
        if not operator.isNumberType(val):
            raise TypeError

        self.a *= val
        self.b *= val
        return self
