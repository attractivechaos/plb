pat <- commandArgs(trailingOnly = T)[1]
con <- file("/dev/stdin", "r")
while (length(line <- readLines(con, n = 1, warn = F)) > 0) {
	if (length(grep(pat, line)) > 0) print(line)
}
