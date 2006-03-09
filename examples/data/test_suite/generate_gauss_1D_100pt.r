set.seed(1023)
x <- rnorm(100, 10, 5)
write.table(x, "data.txt", quote=FALSE, row.names=FALSE, col.names=FALSE)
