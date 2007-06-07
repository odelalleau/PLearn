__cvs_id__ = "$Id$"

from Numeric import *
from LinearAlgebra import *
from math import pi
from math import log

def mnd(x, mean, cov):
    cov_inv = inverse(cov)
    cov_det = determinant(cov)
    exponent = -0.5 * matrixmultiply(matrixmultiply(transpose(x - mean),
                                                    cov_inv),
                                     x - mean)
    return (1 / (2 * pi)) * cov_det ** -0.5 * exp(exponent)

def log_mnd(x, mean, cov):
    cov_inv = inverse(cov)
    cov_det = determinant(cov)
    exponent = -0.5 * matrixmultiply(matrixmultiply(transpose(x - mean),
                                                    cov_inv),
                                     x - mean)
    return log(1 / (2 * pi) * cov_det ** -0.5) + exponent

__all__ = [ "mnd", "log_mnd" ]

