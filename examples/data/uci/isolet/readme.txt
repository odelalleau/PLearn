# PLearn information.

1. Generate datasets from raw data:
plearn_full vmat info isolet_ID=1+2+3+4_generate.vmat
plearn_full vmat info isolet_ID=5_generate.vmat

2. Links for train and test data (to perform in the UCI_MLDB directory):
ln -s isolet_ID=1+2+3+4.vmat isolet_train.vmat
ln -s isolet_ID=5.vmat isolet_test.vmat

