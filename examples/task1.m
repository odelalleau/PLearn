n=300;

t=0:6/n:6;
boundary = [t' sin(t)'];
save('boundary.amat','boundary','-ascii')

%% plot of the boundary
figure(1);
plot(boundary(:,1),boundary(:,2),'.');
hold on;
%% generation des ensembles de train et de test
%% plot du test
for j=1:2

    M= zeros(n,3);

    for i=1:n
        a = rand*6;
        b = rand*3-1.5;


        if b < sin(a)
            if j==1
                plot(a,b,'r+');
            else
                plot(a,b,'ro');
            end
            classe = 1;
        else
            if j==1
                plot(a,b,'g+')
            else
                plot(a,b,'go');
            end
            classe = -1;
        end

        M(i,:) = [a b classe];

    end

    if j==1
        train = M;
    else
        test = M;
    end
end

hold off;

save('train.amat','train','-ascii')  
save('test.amat','test','-ascii')  


%% creation d'un pavage pour afficher la frontière de décision
n=100;
t=0:6/n:6;
t2=-1.5:3/n:1.5;
space = zeros(n^2,2);
for i=1:n
    for j=1:n
        space(n*(i-1)+j,:) = [t(i) t2(j)];
    end
end
save('space.amat','space','-ascii');

