#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct {
	uint16_t r[324][9]; // r[c][i]: the index of the i-th row matching col c
	uint16_t c[729][4]; // c[r][j]: the index of the j-th col matching row r
} sdaux_t;

sdaux_t *sd_genmat()
{
	sdaux_t *a;
	int i, j, k, r, c, c2;
	int8_t nr[324];
	a = calloc(1, sizeof(sdaux_t));
	for (i = r = 0; i < 9; ++i) // generate c[729][4]
		for (j = 0; j < 9; ++j)
			for (k = 0; k < 9; ++k) // this "9" means each cell has 9 possible numbers
				a->c[r][0] = 9 * i + j,                  // row-column constraint
				a->c[r][1] = (i/3*3 + j/3) * 9 + k + 81, // box-number constraint
				a->c[r][2] = 9 * i + k + 162,            // row-number constraint
				a->c[r][3] = 9 * j + k + 243,            // col-number constraint
				++r;
	for (c = 0; c < 324; ++c) nr[c] = 0;
	for (r = 0; r < 729; ++r) // generate r[][] from c[][]
		for (c2 = 0; c2 < 4; ++c2)
			k = a->c[r][c2], a->r[k][nr[k]++] = r;
	return a;
}

inline void sd_update(const sdaux_t *aux, int16_t sr[729], int16_t sc[324], int r, int v)
{
	int c2;
	for (c2 = 0; c2 < 4; ++c2) {
		int r2, c = aux->c[r][c2];
		sc[c] += v;
		for (r2 = 0; r2 < 9; ++r2) sr[aux->r[c][r2]] += v;
	}
}

int sd_solve(const sdaux_t *aux, const char *_s)
{
	int i, j, r, c, r2, dir, hints = 0;
	int16_t sr[729], sc[324]; // sr[r]/sc[c]: row r/col c - #constraints
	int16_t cr[81], cc[81];  // cr/cc[i]: row/col chosen at step i
	char out[82];
	for (r = 0; r < 729; ++r) sr[r] = 0;
	for (c = 0; c < 324; ++c) sc[c] = 0;
	for (i = 0; i < 81; ++i) {
		int a = _s[i] >= '1' && _s[i] <= '9'? _s[i] - '1' : -1;
		if (a >= 0) sd_update(aux, sr, sc, i * 9 + a, 1);
		if (a >= 0) ++hints;
		cr[i] = cc[i] = -1, out[i] = _s[i];
	}
	for (i = 0, dir = 1, out[81] = 0;;) {
		while (i >= 0 && i < 81 - hints) {
			if (dir == 1) {
				int min = 10, n;
				for (c = 0; c < 324; ++c) {
					const uint16_t *p;
					if (sc[c]) continue;
					for (r2 = n = 0, p = aux->r[c]; r2 < 9; ++r2)
						if (sr[p[r2]] == 0) ++n;
					if (n < min) min = n, cc[i] = c;
					if (min <= 1) break; // not sure if this line is correct
				}
				if (min == 0 || min == 10) cr[i--] = dir = -1;
			}
			c = cc[i];
			if (dir == -1) sd_update(aux, sr, sc, aux->r[c][cr[i]], -1);
			for (r2 = cr[i] + 1; r2 < 9; ++r2)
				if (sr[aux->r[c][r2]] == 0) break;
			if (r2 < 9) {
				sd_update(aux, sr, sc, aux->r[c][r2], 1);
				cr[i++] = r2; dir = 1;
			} else cr[i--] = dir = -1;
		}
		if (i < 0) break;
		for (j = 0; j < i; ++j) r = aux->r[cc[j]][cr[j]], out[r/9] = r%9 + '1';
		puts(out);
		--i; dir = -1;
	}
	return 0;
}

int main()
{
	sdaux_t *a = sd_genmat();
	char buf[1024];
	while (fgets(buf, 1024, stdin) != 0) {
		if (strlen(buf) >= 81) {
			sd_solve(a, buf);
			putchar('\n');
		}
	}
	free(a);
	return 0;
}
