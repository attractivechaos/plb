mat_transpose(a)
{
	int m = a.length, n = a[0].length; // m rows and n cols
	var b = new List(n);
	for (int j = 0; j < n; ++j) b[j] = new List<double>(m);
	for (int i = 0; i < m; ++i)
		for (int j = 0; j < n; ++j)
			b[j][i] = a[i][j];
	return b;
}

mat_mul(a, b)
{
	int m = a.length, n = a[0].length, s = b.length, t = b[0].length;
	if (n != s) return null;
	var x = new List(m), c = mat_transpose(b);
	for (int i = 0; i < m; ++i) {
		x[i] = new List<double>(t);
		for (int j = 0; j < t; ++j) {
			double sum = 0;
			//var ai = a[i], cj = c[j];
			//for (int k = 0; k < n; ++k) sum += ai[k] * cj[k];
			for (int k = 0; k < n; ++k) sum += a[i][k] * c[j][k];
			x[i][j] = sum;
		}
	}
	return x;
}

mat_gen(int n)
{
	var a = new List(n);
	double t = 1.0 / n / n;
	for (int i = 0; i < n; ++i) {
		a[i] = new List<double>(n);
		for (int j = 0; j < n; ++j)
			a[i][j] = t * (i - j) * (i + j);
	}
	return a;
}

main()
{
	int n = 500;
	var a = mat_gen(n);
	var b = mat_gen(n);
	var c = mat_mul(a, b);
	print(c[n~/2][n~/2]);
}
