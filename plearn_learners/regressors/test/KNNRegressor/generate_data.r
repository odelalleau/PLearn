set.seed(1023)
x = runif(250,0,6)
y = rnorm(250,0,0.5)
z = sin(x-0.5)+y
write.table(data.frame(x,z),"data.txt",row.names=FALSE,col.names=FALSE)
