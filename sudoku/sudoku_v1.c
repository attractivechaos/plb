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

typedef struct {
	uint64_t a, b;
} sd128_t;

sd128_t *sd_enumerate9()
{
	int8_t pos2box[81], mask_box[9], mask_col[9];
	int i, j, c, p[9];
	sd128_t *e;
	e = calloc(46656, 16);
	for (i = 0; i < 9; ++i)
		for (j = 0; j < 9; ++j)
			pos2box[9*i+j] = i/3*3 + j/3;
	for (i = 0; i < 9; ++i) mask_box[i] = mask_col[i] = 0, p[i] = -1;
	for (i = c = 0;;) {
		while (i >= 0 && i < 9) {
			int i9 = 9 * i;
			if (p[i] >= 0) mask_col[p[i]] = mask_box[pos2box[i9+p[i]]] = 0;
			for (j = p[i] + 1; j < 9; ++j)
				if (!mask_box[pos2box[i9 + j]] && !mask_col[j]) break;
			if (j < 9) mask_col[j] = mask_box[pos2box[9*i+j]] = 1, p[i++] = j;
			else p[i--] = -1;
		}
		if (i-- < 0) break;
		for (j = 0; j < 9; ++j) {
			int x = 9 * j + p[j];
			SET128(e[c], x);
		}
		++c;
	}
	return e;
}

int sd_solve(const sd128_t *e, const char *s)
{
	int i, j, k, p[9], n[9];
	sd128_t x, *z[9];
	for (i = 0; i < 9; ++i) {
		int max = 0;
		sd128_t x, y;
		z[i] = 0; n[i] = 0; p[i] = -1;
		x.a = x.b = y.a = y.b = 0;
		for (j = 0; j < 81; ++j) {
			if (s[j] < '1' || s[j] > '9') continue;
			if ((int)s[j] - '1' == i) SET128(x, j);
			else SET128(y, j);
		}
		for (j = 0; j < 46656; ++j) {
			const sd128_t *ej = e + j;
			if ((ej->a&x.a) == x.a && (ej->b&x.b) == x.b && !(ej->a&y.a) && !(ej->b&y.b)) {
				if (n[i] == max) {
					max = max? max<<1 : 256;
					z[i] = realloc(z[i], max * 16);
				}
				z[i][n[i]++] = *ej;
			}
		}
		n[i] = n[i]<<8 | i;
	}
	for (i = 1; i < 9; ++i)
		for (j = i; j > 0 && n[j] < n[j-1]; --j)
			k = n[j], n[j] = n[j-1], n[j-1] = k;
	x.a = x.b = 0;
	for (i = 0;;) {
		char out[82];
		while (i >= 0 && i < 9) {
			int ni = n[i]>>8;
			sd128_t *zi = z[n[i]&0xff];
			if (p[i] >= 0) x.a &= ~zi[p[i]].a, x.b &= ~zi[p[i]].b;
			for (j = p[i] + 1; j < ni; ++j)
				if (!(x.a&zi[j].a) && !(x.b&zi[j].b)) break;
			if (j < ni) x.a |= zi[j].a, x.b |= zi[j].b, p[i++] = j;
			else p[i--] = -1;
		}
		if (i-- < 0) break;
		for (j = 0; j < 9; ++j) {
			for (k = 0; k < 64; ++k) if (z[n[j]&0xff][p[j]].a>>k&1) out[k] = (n[j]&0xff) + '1';
			for (k = 0; k < 81 - 64; ++k) if (z[n[j]&0xff][p[j]].b>>k&1) out[k+64] = (n[j]&0xff) + '1';
		}
		out[81] = 0;
		puts(out);
	}
	return 0;
}

int main()
{
	sd128_t *e;
	char buf[1024];
	e = sd_enumerate9();
	while (fgets(buf, 1024, stdin) != 0) {
		if (strlen(buf) >= 81) {
			sd_solve(e, buf);
			putchar('\n');
		}
	}
	free(e);
	return 0;
}
