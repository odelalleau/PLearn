### Three blocks of inputs:
### - 2 columns are original data,
### - 1 column  is an exact linear combination of original data
### - 1 column  is an almost exact linear combination but with small noise
### - 2 targets depend on original data alone

set.seed(1023)
x  <- matrix(rnorm(400),ncol=2)
x1 <-      x %*% c(5,10)
x2 <-      x %*% c(2,1)    +  0.001 * matrix(rnorm(200),ncol=1)
y1 <-  5 + x %*% c(10,100) +  5*rnorm(200)
y2 <- 10 + x %*% c(20,50)  + 20*rnorm(200)

z <- data.frame(x,x1,x2,y1,y2)
write.table(z, "data.txt", row.names=FALSE, col.names=FALSE)
