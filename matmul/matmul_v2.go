// Written by Attractive Chaos; distributed under the MIT license

package main

import "fmt"
import "flag"
import "strconv"

func matgen(n int) [][]float64 {
	a := make([][]float64, n)
	aflat := make([]float64, n*n)
	tmp := float64(1.0) / float64(n) / float64(n) // pretty silly...
	for i := 0; i < n; i++ {
		a[i] = aflat[i*n:(i+1)*n]
		for j := 0; j < n; j++ {
			a[i][j] = tmp * float64(i-j) * float64(i+j)
		}
	}
	return a
}

func matmul(a [][]float64, b [][]float64) [][]float64 {
	m := len(a)
	n := len(a[0])
	p := len(b[0])
	x := make([][]float64, m)
	xflat := make([]float64, m*p)
	c := make([][]float64, p)
	cflat := make([]float64, n*p)
	for i := 0; i < p; i++ {
		c[i] = cflat[i*n:(i+1)*n]
		for j := 0; j < n; j++ {
			c[i][j] = b[j][i]
		}
	}
	for i := 0; i < m; i++ {
		x[i] = xflat[i*p:(i+1)*p]
		for j := 0; j < p; j++ {
			s := float64(0)
			for k := 0; k < n; k++ {
				s += a[i][k] * c[j][k]
			}
			x[i][j] = s
		}
	}
	return x
}

func main() {
	n := int(100)
	flag.Parse()
	if flag.NArg() > 0 { n,_ = strconv.Atoi(flag.Arg(0)) }
	a := matgen(n)
	b := matgen(n)
	x := matmul(a, b)
	fmt.Printf("%f\n", x[n/2][n/2])
}
