load -ascii result
load -ascii space2.amat

figure(1);
plot(space2,result,space2,sin(space2));
legend('learned','analytic');
