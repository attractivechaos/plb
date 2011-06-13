/* UNFINISHED port of Guenter Stertenbrink's suexco */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/*
  In the binary matrix M[729][324], the column vector M[.][c] keeps the constraint. In the following
  representation, we are storing the positions of non-zero cells.
*/
typedef struct {
	uint16_t r[324][9]; // r[c][i], i=0..8: the index of the i-th row matching col c
	uint16_t c[729][4]; // c[r][j], j=0..3: the index of the j-th col matching row r
} sdaux_t;

sdaux_t *sd_prep()
{
	sdaux_t *a;
	int i, j, k, r, c;
	int8_t nr[324];
	a = calloc(1, sizeof(sdaux_t));
	for (i = r = 0; i < 9; ++i) // generate c[729][4]
		for (j = 0; j < 9; ++j)
			for (k = 0; k < 9; ++k) // this "9" means each cell has 9 options
				a->c[r][0] = 9 * i + j,                  // row-column constraint
				a->c[r][1] = (i/3*3 + j/3) * 9 + k + 81, // box-number constraint
				a->c[r][2] = 9 * i + k + 162,            // row-number constraint
				a->c[r][3] = 9 * j + k + 243,            // col-number constraint
				++r;
	for (c = 0; c < 324; ++c) nr[c] = 0;
	for (r = 0; r < 729; ++r) // generate r[][] from c[][]
		for (c = 0; c < 4; ++c)
			k = a->c[r][c], a->r[k][nr[k]++] = r;
	return a;
}

inline void sd_update(const sdaux_t *aux, int8_t fr[729], int8_t fc[324], int r)
{
	int c, k, d;
	for (c = 0; c < 4; ++c) {
		d = aux->c[r][c];
		++fc[d];
		for (k = 0; k < 9; ++k) ++fr[aux->r[d][k]];
	}
}

inline void sd_revert(const sdaux_t *aux, int8_t fr[729], int8_t fc[324], int r)
{
	int c, k, d;
	for (c = 0; c < 4; ++c) {
		d = aux->c[r][c];
		--fc[d];
		for (k = 0; k < 9; ++k) --fr[aux->r[d][k]];
	}
}

int sd_solve(const sdaux_t *aux, const char *_s)
{
	int i, j, r, c;
	int clues;       // # known cells
	int8_t fr[729];  // fr[r]: # constraints at row r; 0 iff r has not been marked as forbidden
	int8_t fc[324];  // fc[c]: # constraints at col c; 0 iff c has not been marked as forbidden
	uint16_t ir[81]; // ir[i]: row chosen at step i
	uint16_t ic[81]; // ic[i]: col chosen at step i
	uint16_t Node[81];
	int8_t a[9][9];
	for (i = 0; i < 81; ++i) a[i/9][i%9] = _s[i] >= '1' && _s[i] <= '9'? _s[i] - '1' : -1;
	for (r = 0; r < 729; ++r) fr[r] = 0;
	for (c = 0; c < 324; ++c) fc[c] = 0;
	for (i = clues = 0; i < 9; ++i)
		for (j = 0; j < 9; ++j)
			if (a[i][j] >= 0) {
				++clues;
				sd_update(aux, fr, fc, 81 * i + 9 * j + a[i][j]);
			}
	for (i = 0;; ++i) {
		int min = 729;
		ir[i] = 0;
		for (c = 0; c < 324; ++c) {
			if (!fc[c]) {
				int match = 0;
				for (r = 0; r < 9; ++r)
					if (!fr[aux->r[c][r]]) ++match;
				if (match < min) min = match, ic[i] = c; // choose the smallest as the column
			}
		}
		if (min == 0 || min == 729) {
			if (i == 0) goto end_loop;
			c = ic[--i];
			sd_revert(aux, fr, fc, aux->r[c][ir[i]]);
		}
		do {
			c = ic[i];
			if (++ir[i] >= 9) {
				if (i == 0) goto end_loop;
				c = ic[--i];
				sd_revert(aux, fr, fc, aux->r[c][ir[i]]);
			}
			r = aux->r[c][ir[i]];
		} while (fr[r]);
		sd_update(aux, fr, fc, r);
		++Node[i];
	}
end_loop:
	return 0;
}

int main()
{
	sdaux_t *a = sd_prep();
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
