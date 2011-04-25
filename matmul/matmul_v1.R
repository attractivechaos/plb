matgen <- function(n) {
	y0 <- matrix(rep(seq(0, n-1), n), n)
	y1 <- t(y0)
	z <- 1 / n / n
	(y0 - y1) * (y0 + y1) * z
}

n <- as.integer(commandArgs(trailingOnly = T)[1])
if (is.na(n)) n <- 100
x <- matgen(n) %*% matgen(n)
x[n/2+1,n/2+1]
