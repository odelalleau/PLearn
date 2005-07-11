# PLearn conversion instructions.

# Notes:
# 1. Sample number 248 has been removed (duplicated with sample number 102)
#    even if it is in the test set (see note number 3).
# 2. Column 1 has been removed (it is constant).
# 3. It is not really clear what are exactly the training and test sets, as
#    there are some incoherences in the description in the UCI dataset. We
#    chose to take the first 200 instances for training and the remaining 150
#    for testing (after removing the duplicated instance). In the training
#    set, there are 101 positive and 99 negative examples. In the test set,
#    these numbers are respectively 124 and 26.

# Generate dataset from raw data
plearn vmat info UCI_MLDB:ionosphere/ionosphere_generate.vmat

