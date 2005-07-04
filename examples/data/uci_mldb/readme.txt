This directory contains scripts to easily load UCI datasets.

PLearn needs the following metaprotocols to be defined in the ppath.config file:
- UCI_MLDB_REP: path to the original UCI_MLDB datafiles
- UCI_MLDB    : path to the PLearn vmat files (default is PLEARNDIR:examples/data/uci_mldb)

In order to enable access to a database named 'data', you must run the
commands given in the file UCI_MLDB:data/readme.txt.

This will typically precompute the database in a PLearn binary format. The
database will then be available through all or some of the following files:
- UCI_MLDB:data_all.vmat                : whole set
- UCI_MLDB:data_train.vmat              : training set
- UCI_MLDB:data_test.vmat               : test set
- UCI_MLDB:data_all_norm.vmat           : whole set, normalized
- UCI_MLDB:data_train_norm.vmat         : training set, normalized
- UCI_MLDB:data_test_norm.vmat          : test set, normalized
- UCI_MLDB:data_all_shuffle.vmat        : whole set, shuffled
- UCI_MLDB:data_train_shuffle.vmat      : training set, shuffled
- UCI_MLDB:data_test_shuffle.vmat       : test set, shuffled
- UCI_MLDB:data_all_norm_shuffle.vmat   : whole set, normalized and shuffled
- UCI_MLDB:data_train_norm_shuffle.vmat : training set, normalized and shuffled
- UCI_MLDB:data_test_norm_shuffle.vmat  : test set, normalized and shuffled

The normalization consists in having mean = 0 and standard deviation = 1 in
all input columns, for the whole set (this means the same normalization is
applied for data_all_norm, data_train_norm and data_test_norm).

Some datasets have a shuffled version, obtained by applying a BootstrapVMatrix
with a shuffle seed of 1000000. This is usually done when the original dataset
exhibits a particular order of samples which could introduce some bias in the
learning algorithm.

It is always true that the concatenation of 'train' and 'test' gives 'all',
even for normalized or shuffled datasets.

Small datasets (< 5 Mb in memory) are automatically precomputed in memory.
Bigger ones are only stored on disk (which can slow down the access to data,
especially if the dataset is shuffled).

