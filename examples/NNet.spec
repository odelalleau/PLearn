NNet(
nhidden =  10 ;
noutputs = 1 ;
output_transfer_func = "tanh";
hidden_transfer_func = "tanh"  ;
cost_funcs = 1 [ mse ]  ;
optimizer = GradientOptimizer(
                start_learning_rate = .001;
                decrease_constant = 0;
                )
batch_size = 1  ;
initialization_method = "normal_sqrt"  ;
nstages = 500 ;
verbosity = 3;
);


