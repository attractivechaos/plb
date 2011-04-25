// Written by Attractive Chaos; distributed under the MIT license

// translated from matmul_v1.java
import std.stdio, std.string;

double[][] matgen(int n) {
	double[][] a;
	double tmp = 1. / n / n;
	a.length = n;
	for (int i = 0; i < n; ++i) a[i].length = n;
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < n; ++j)
			a[i][j] = tmp * (i - j) * (i + j);
	return a;
}
double[][] matmul(double[][] a, double[][] b) {
	int m = a.length, n = a[0].length, p = b[0].length;
	double[][] x, c;
	x.length = m; c.length = p;
	for (int i = 0; i < m; ++i) x[i].length = p;
	for (int i = 0; i < p; ++i) c[i].length = n;
	for (int i = 0; i < n; ++i) // transpose
		for (int j = 0; j < p; ++j)
			c[j][i] = b[i][j];
	for (int i = 0; i < m; ++i)
		for (int j = 0; j < p; ++j) {
			double s = 0.0;
			for (int k = 0; k < n; ++k)
				s += a[i][k] * c[j][k];
			x[i][j] = s;
		}
	return x;
}
int main(string[] args) {
	int n = 100;
	if (args.length >= 2) n = atoi(args[1]);
	n = n / 2 * 2;
	double[][] a, b, x;
	a = matgen(n);
	b = matgen(n);
	x = matmul(a, b);
	writefln(x[n/2][n/2]);
	return 0;
}
