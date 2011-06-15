/* The MIT License

   Copyright (c) 2011 by Attractive Chaos <attractor@live.co.uk>

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define SET128(z, x) do { if ((x) >= 64) (z).b |= 1ull<<((x)-64); else (z).a |= 1ull<<(x); } while (0)

typedef struct { uint64_t a, b; } sd128_t; // 128-bit integer
// enumerate all valid patterns for one integer
sd128_t *sd_enumerate9()
{
	int8_t pos2box[81], mask_box[9], mask_col[9];
	int i, j, c, p[9];
	sd128_t *e;
	e = calloc(46656, 16);
	for (i = 0; i < 81; ++i) pos2box[i] = i/9/3*3 + i%9/3; // translate position to the box index
	for (i = 0; i < 9; ++i) mask_box[i] = mask_col[i] = 0, p[i] = -1; // initialization
	for (i = c = 0;; ++c) { // each iteration generates a valid pattern
		while (i >= 0 && i < 9) { // loop until found one pattern or found no pattern
			int i9 = 9 * i;
			if (p[i] >= 0) mask_col[p[i]] = mask_box[pos2box[i9+p[i]]] = 0; // reset the mask at row i
			for (j = p[i] + 1; j < 9; ++j) // search for a valid position
				if (!mask_box[pos2box[i9 + j]] && !mask_col[j]) break; // stop if found
			if (j < 9) mask_col[j] = mask_box[pos2box[9*i+j]] = 1, p[i++] = j; // found a valid one
			else p[i--] = -1; // backtrack
		}
		if (i-- < 0) break; // no valid patterns any more
		for (j = 0; j < 9; ++j) { // write the pattern
			int x = 9 * j + p[j];
			SET128(e[c], x); // set the bit of e[c] at x
		}
	}
	return e;
}
// solve Sudoku for s 
int sd_solve(const sd128_t *e, const char *s)
{
	int i, j, k, p[9], n[9];
	sd128_t x, y, *z[9];
	for (i = 0; i < 9; ++i) { // generate all possible patterns for each number
		int max = 0;
		z[i] = 0; n[i] = 0; p[i] = -1;
		x.a = x.b = y.a = y.b = 0; // x keeps the pattern of the current number; y the other numbers
		for (j = 0; j < 81; ++j) { // set x and y
			if (s[j] < '1' || s[j] > '9') continue; // skip if not [1,9]
			if ((int)s[j] - '1' == i) SET128(x, j); // if the current number, set x
			else SET128(y, j); // if other numbers, set y
		}
		for (j = 0; j < 46656; ++j) { // traverse all possible patterns
			const sd128_t *ej = e + j;
			if ((ej->a&x.a) == x.a && (ej->b&x.b) == x.b && !(ej->a&y.a) && !(ej->b&y.b)) {
				if (n[i] == max) { // enlarge
					max = max? max<<1 : 256;
					z[i] = realloc(z[i], max * 16);
				}
				z[i][n[i]++] = *ej; // keep the pattern
			}
		}
		n[i] = n[i]<<8 | i; // this is for sorting
	}
	for (i = 1; i < 9; ++i) // insertion sort; start from numbers with fewer patterns
		for (j = i; j > 0 && n[j] < n[j-1]; --j)
			k = n[j], n[j] = n[j-1], n[j-1] = k;
	x.a = x.b = 0; // x is going to be the mask up to position i
	for (i = 0;;) {
		char out[82];
		while (i >= 0 && i < 9) {
			int ni = n[i]>>8; // this is the number of patterns
			sd128_t *zi = z[n[i]&0xff]; // n[i]&0xff is the number
			if (p[i] >= 0) x.a &= ~zi[p[i]].a, x.b &= ~zi[p[i]].b;
			for (j = p[i] + 1; j < ni; ++j) // search for a compatible pattern
				if (!(x.a&zi[j].a) && !(x.b&zi[j].b)) break;
			if (j < ni) x.a |= zi[j].a, x.b |= zi[j].b, p[i++] = j; // found; move to the next
			else p[i--] = -1; // backtrack
		}
		if (i-- < 0) break; // no further solutions
		for (j = 0; j < 9; ++j) { // write out the solution
			for (k = 0; k < 64; ++k) if (z[n[j]&0xff][p[j]].a>>k&1) out[k] = (n[j]&0xff) + '1';
			for (k = 0; k < 81 - 64; ++k) if (z[n[j]&0xff][p[j]].b>>k&1) out[k+64] = (n[j]&0xff) + '1';
		}
		out[81] = 0;
		puts(out);
	}
	for (i = 0; i < 9; ++i) free(z[i]);
	return 0;
}

int main()
{
	char buf[1024];
	sd128_t *e = sd_enumerate9();
	while (fgets(buf, 1024, stdin) != 0) {
		if (strlen(buf) >= 81) {
			sd_solve(e, buf);
			putchar('\n');
		}
	}
	free(e);
	return 0;
}
