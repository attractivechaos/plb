# Writen by Attractive Chaos; distributed under the MIT license

# This version uses the built-in class Matrix. However, this version is
# typically twice as slow as the first version. The reason is "matrix.rb"
# seems not to transpose the matrix.
require 'matrix'

def matgen(n)
	tmp = 1.0 / n / n
  	a = Array.new(n) { Array.new(n) { 0 } }
	for i in 0 .. n-1
		for j in 0 .. n-1
			a[i][j] = tmp * (i - j) * (i + j)
		end
	end
	return a
end

n = 100
if ARGV.length >= 1
	n = ARGV[0].to_i
end
n = n / 2 * 2
tmp = 1.0 / n / n
#a = Matrix.build(n, n) {|i, j| tmp * (i - j) * (i + j)} # why not work?
#b = Matrix.build(n, n) {|i, j| tmp * (i - j) * (i + j)}
a = Matrix.rows(matgen(n))
b = Matrix.rows(matgen(n))
c = a * b
puts c[n/2, n/2]
