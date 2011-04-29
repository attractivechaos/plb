// Originally written by Attractive Chaos; distributed under the MIT license (D V.2 code)
// D1 code for LDC:
// http://www.dsource.org/projects/ldc
// Compile with: ldc -O3 -release -inline matmul.d

import tango.stdc.stdio, tango.stdc.stdlib;

double[][] matGen(in int n) {
	double tmp = 1.0 / n / n;
	auto a = new double[][](n, n);
	foreach (int i, row; a)
		foreach (int j, ref x; row)
		x = tmp * (i - j) * (i + j);
	return a;
}

double[][] matMul(in double[][] a, in double[][] b) {
	int m = a.length,
		n = a[0].length,
		p = b[0].length;

	// transpose
	auto c = new double[][](p, n);
	foreach (i, brow; b)
		foreach (j, bx; brow)
		c[j][i] = bx;

	auto x = new double[][](m, p);

	foreach (i, arow; a)
		foreach (j, crow; c) {
			// x[i][j] = std.numeric.dotProduct(arow, crow); // right way D2
			double s = 0.0;
			foreach (k, arowk; arow)
				s += arowk * crow[k];
			x[i][j] = s;
		}

	return x;
}

void main(in char[][] args) {
	int n = 100;
	if (args.length >= 2)
		n = atoi((args[1] ~ '\0').ptr) / 2 * 2;
	auto a = matGen(n);
	auto b = matGen(n);
	auto x = matMul(a, b);
	printf("%f\n", x[n / 2][n / 2]);
}
