import random 
from plearn.pyplearn import pl

def LearnerWithSomeSettingsIOftenUse( learning_rate, dataset_mng ):
    option1 = pl.Option1( val = 10 )
    option2 = pl.Option2( foo = 'bar' )

    return pl.SomeLearner(
        learning_rate = learning_rate,
        dataset_mng   = dataset_mng,
        o1            = option1,
        o2            = option2
        )                           
    
def MyCombiner( underlying_learners ):
    weights = [ random.random()
                for n in range( len(underlying_learners) ) ]

    wsum = sum( weights )
    normalized = [ w/wsum for w in weights ]

    return pl.CombinerLearner(
        underlying_learners = underlying_learners,
        weights             = normalized
        )
