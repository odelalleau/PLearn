load -ascii result
load -ascii space.amat
load -ascii boundary.amat
[n,p]=size(result)
C = zeros(n,3);
for i=1:n    
    C(i,:) = [0 (result(i)+1)/2 0];
end
figure(2);
scatter(space(:,1),space(:,2),10,C);
hold on;
plot(boundary(:,1),boundary(:,2),'r.');
