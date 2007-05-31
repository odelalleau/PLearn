"""Code to define and manipulate online learning modules and related learners"""

import plearn.bridgemode
from plearn.bridge import *

# THIS SHOULD GO SOMEWHERE ELSE!
def ifthenelse(cond,elsep,condp):
    if cond:
        return elsep
    else:
        return condp

def rbm(name,
        visible_size,
        hidden_size,
        grad_learning_rate=0, 
        cd_learning_rate=0,
        compute_nll=False,
        ngcd=1,
        have_reconstruction=True,
        compute_contrastive_divergence=True):
    """Construct an RBMModule"""
    # Return a standard binomial RBM.
    conx = pl.RBMMatrixConnection(
                down_size = visible_size,
                up_size = hidden_size)
    return pl.RBMModule(
            name = name,
            cd_learning_rate = cd_learning_rate,
            grad_learning_rate = grad_learning_rate,
            compute_contrastive_divergence = compute_contrastive_divergence,
            compute_log_likelihood = compute_nll,
            n_Gibbs_steps_CD = ngcd,
            visible_layer = pl.RBMBinomialLayer(size = visible_size),
            hidden_layer = pl.RBMBinomialLayer(size = hidden_size),
            connection = conx,
            reconstruction_connection = ifthenelse(have_reconstruction,
                                                   pl.RBMMatrixTransposeConnection(rbm_matrix_connection = conx),
                                                   None))


def connection(src, dst, propagate_gradient = True):
    """Construct a NetworkConnection"""
    return pl.NetworkConnection(
            source = src,
            destination = dst,
            propagate_gradient = propagate_gradient)
