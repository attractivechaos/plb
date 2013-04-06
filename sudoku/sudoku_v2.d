// Works with GDC
// This program segfaults at the end of the process, but I believe this is a bug of GDC

/*
The MIT License

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

import std.stdio: readln, writeln, writef;

struct Sdaux {
	int[9][324] r;
	int[4][729] c;

	void initialize() {
		int r1 = 0;
		for (int i = 0; i < 9; ++i)
			for (int j = 0; j < 9; ++j)
				for (int k = 0; k < 9; ++k)
					c[r1++][] = [9 * i + j, (i/3*3 + j/3) * 9 + k + 81, 9 * i + k + 162, 9 * j + k + 243];
		byte[324] nr;
		for (int r2 = 0; r2 < 729; ++r2)
			for (int c2 = 0; c2 < 4; ++c2) {
				auto k = c[r2][c2];
				r[k][nr[k]++] = r2;
			}
	}
}

int sdUpdate(in Sdaux *aux, byte *sr, ubyte *sc, in int r, in int v)
{
	int min = 10, min_c;
	for (size_t c2 = 0; c2 < 4; ++c2) sc[aux.c[r][c2]] += v << 7;
	for (size_t c2 = 0; c2 < 4; ++c2) {
		int c = aux.c[r][c2], rr;
		if (v > 0) {
			for (size_t r2 = 0; r2 < 9; ++r2) {
				if (sr[rr = aux.r[c][r2]]++ != 0) continue;
				for (size_t cc2 = 0; cc2 < 4; ++cc2) {
					int cc = aux.c[rr][cc2];
					if (--sc[cc] < min)
						min = sc[cc], min_c = cc;
				}
			}
		} else {
			for (size_t r2 = 0; r2 < 9; ++r2) {
				if (--sr[rr = aux.r[c][r2]] != 0) continue;
				auto p = aux.c[rr].ptr;
				++sc[p[0]]; ++sc[p[1]]; ++sc[p[2]]; ++sc[p[3]];
			}
		}
	}
	return min << 16 | min_c;
}

int sdSolve(in Sdaux *aux, in char *_s)
{
	int hints;
	byte[729] sr;
	byte[81] cr = -1;
	ubyte[324] sc = 9;
	short[81] cc = -1;
	char[81] outs;

	for (int i = 0; i < 81; ++i) {
		int a = _s[i] >= '1' && _s[i] <= '9'? _s[i] - '1' : -1;
		if (a >= 0) sdUpdate(aux, sr.ptr, sc.ptr, i * 9 + a, 1);
		if (a >= 0) ++hints;
		outs[i] = _s[i];
	}

	int dir, i, r, cand, n, min;
	for (i = 0, dir = 1, cand = 10 << 16 | 0;;) {
		while (i >= 0 && i < 81 - hints) {
			if (dir == 1) {
				min = cand >> 16, cc[i] = cand & 0xFFFF;
				if (min > 1) {
					for (size_t c = 0; c < sc.length; ++c) {
						if (sc[c] < min) {
							min = sc[c], cc[i] = cast(short)c;
							if (min <= 1) break;
						}
					}
				}
				if (min == 0 || min == 10) dir = cr[i--] = -1;
			}
			int r2, c = cc[i];
			if (dir == -1 && cr[i] >= 0) sdUpdate(aux, sr.ptr, sc.ptr, aux.r[c][cr[i]], -1);
			for (r2 = cr[i] + 1; r2 < 9; ++r2)
				if (sr[aux.r[c][r2]] == 0) break;
			if (r2 < 9) {
				cand = sdUpdate(aux, sr.ptr, sc.ptr, aux.r[c][r2], 1);
				cr[i++] = cast(byte)r2; dir = 1;
			} else dir = cr[i--] = -1;
		}
		if (i < 0) break;
		for (size_t j = 0; j < i; ++j) {
			r = aux.r[cc[j]][cr[j]];
			outs[r / 9] = cast(char)(r % 9 + '1'); // print
		}
		writeln(outs);
		++n; --i; dir = -1;
	}
	return n;
}

void main() {
	Sdaux a;
	a.initialize();
	string buf;
	while ((buf = readln()) != null) {
		if (buf.length < 81) continue;
		sdSolve(&a, buf.ptr);
		writef("\n");
	}
}
