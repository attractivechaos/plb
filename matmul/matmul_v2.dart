// by Vyacheslav Egorov

import 'dart:scalarlist'; // [!] added import

mat_transpose(a)
{
  int m = a.length, n = a[0].length; // m rows and n cols
  var b = new List(n);
  for (int j = 0; j < n; ++j) b[j] = new Float64List(m); // [!] converted List<double> to Float64List
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
    x[i] = new Float64List(t); // [!] converted List<double> to Float64List
    for (int j = 0; j < t; ++j) {
      double sum = 0.0;
      var ai = a[i], cj = c[j];
      for (int k = 0; k < n; ++k) sum += ai[k] * cj[k];
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
    a[i] = new Float64List(n); // [!] converted List<double> to Float64List
    for (int j = 0; j < n; ++j)
      a[i][j] = t * (i - j) * (i + j);
  }
  return a;
}

domul(n) {
  var a = mat_gen(n);
  var b = mat_gen(n);
  var c = mat_mul(a, b);
  return c[n~/2][n~/2];
}

// [!] added warm up
warmup() {
  domul(10);
}

main() {
  warmup();
  var result = domul(1000);
  print(result);
}
