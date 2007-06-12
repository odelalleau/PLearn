"""Code to define and manipulate online learning modules and related learners"""

import plearn.bridgemode
from plearn.bridge import *

# THIS SHOULD GO SOMEWHERE ELSE!
def ifthenelse(cond,elsep,condp):
    if cond:
        return elsep
    else:
        return condp

def supervised_classification_mlp(name,input_size,n_hidden,n_classes,
                                  L1wd=0,L2wd=0,lrate=0.01):
    return pl.NetworkModule(name=name,
                            modules=[ pl.GradNNetLayerModule(name='a1',input_size=input_size,
                                                             output_size=n_hidden,
                                                             start_learning_rate=lrate,
                                                             L2_penalty_factor=L2wd,
                                                             L1_penalty_factor=L1wd),
                                      pl.TanhModule(name='tanh',input_size=n_hidden,
                                                    output_size=n_hidden),
                                      pl.GradNNetLayerModule(name='a2',input_size=n_hidden,
                                                             output_size=n_classes,
                                                             start_learning_rate=lrate,
                                                             L2_penalty_factor=L2wd,
                                                             L1_penalty_factor=L1wd),
                                      pl.SoftmaxModule(name='softmax',input_size=n_classes,
                                                       output_size=n_classes),
                                      pl.IdentityModule(name='target'),
                                      pl.NLLCostModule(name='nll',input_size=n_classes),
                                      pl.ClassErrorCostModule(name='clerr',input_size=n_classes)],
                            connections=[  connection('a1.output','tanh.input'),
                                           connection('tanh.output','a2.input'),
                                           connection('a2.output','softmax.input'),
                                           connection('softmax.output','nll.prediction'),
                                           connection('softmax.output','clerr.prediction',False),
                                           connection('target.output','nll.target',False),
                                           connection('target.output','clerr.target',False)],
                            ports = [ ('input', 'a1.input'),
                                      ('target', 'target.input'),
                                      ('output', 'softmax.output'),
                                      ('nll', 'nll.cost'),
                                      ('class_err','clerr.cost') ] )


def supervised_classification_mlp_learner(input_size, n_hidden, n_classes,
        L1wd=0,L2wd=0,lrate=0.01):
    return (pl.ModuleLearner(
            module = supervised_classification_mlp(
                'MLP', input_size, n_hidden, n_classes, L1wd, L2wd, lrate
                ),
            target_ports = [ 'target' ],
            cost_ports = [ 'nll', 'class_err' ]
            ),
            [ 'module.modules[%d].start_learning_rate' % i for i in [0,2] ]
            )


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
