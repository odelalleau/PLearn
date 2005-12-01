sigma = 0.05
n = 300

x <- matrix(rnorm(n,sd=10),ncol=1)
y1 <- sin(x) + rnorm(n,sd=sigma)
y2 <- sign(cos(x)) + rnorm(n,sd=sigma)
z <- data.frame(x,y1,y2)
write.table(z, "data.txt", row.names=FALSE, col.names=FALSE)
