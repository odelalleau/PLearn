%% Generation of points for the tutorial PLearn 
%% Note that this time the data are noisy

n=100;

t=rand(1,n)*6;
train = [t' sin(t)'+randn(n,1)*.05];
save('reg_train.amat','train','-ascii')

t=rand(1,n)*6;
test = [t' sin(t)'+randn(n,1)*.05];
save('reg_test.amat','test','-ascii')

figure(1);
plot(train(:,1),train(:,2),'.',test(:,1),test(:,2),'.r');
legend('Train set','Test set');

t=(0:6/n:6)';
save('space2.amat','t','-ascii')