import sys, array

# Writen by Attractive Chaos; distributed under the MIT license
# Modified by brentp: using array instead of list

# reference: http://www.syntagmatic.net/matrix-multiplication-in-python/

def matmul(a, b): # FIXME: no error checking
	ra, rb = list(range(len(a))), list(range(len(b)))
	c = [array.array('d', [b[j][i] for j in rb]) for i in rb]
	d = [array.array('d', [0 for j in range(len(b[0]))]) for i in ra] # transpose
	for i in ra:
		ai = a[i]
		for j in rb:
			d[i][j] = sum(ai[k] * c[j][k] for k in ra)
	return d

def main():
	n = 100
	if (len(sys.argv) > 1): n = int(sys.argv[1])
	n = int(float(n)/2) * 2 # FIXME: I am sure there are better ways to do this...
	tmp = 1. / n / n
	a = [[tmp * (i - j) * (i + j) for j in range(n)] for i in range(n)]
	b = [[tmp * (i - j) * (i + j) for j in range(n)] for i in range(n)]
	d = matmul(a, b)
	print(d[int(n/2)][int(n/2)])

if __name__ == '__main__': main()
