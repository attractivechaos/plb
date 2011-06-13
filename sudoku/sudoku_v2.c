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
// generate the list of neighbors for each cell
uint8_t *sd_genaux()
{
	int i, j, k;
	uint8_t *a;
	a = malloc(81 * 20); // 81 rows and 20 columns (each cell has 20 neighbors)
	for (i = 0; i < 9; ++i) {
		for (j = 0; j < 9; ++j) {
			uint8_t *a_ij = a + (9 * i + j) * 20;
			int x = 0, iN, jN, ki, kj;
			for (k = 0; k < 9; ++k) // add other cell indices from the current row
				if (k != i) a_ij[x++] = 9 * k + j;
			for (k = 0; k < 9; ++k) // other cell indices from the current column
				if (k != j) a_ij[x++] = 9 * i + k;
			iN = i / 3 * 3; jN = j / 3 * 3; // the top-left coordinate of the current box
			for (ki = iN; ki < iN + 3; ++ki) // other cell indices from the current box
				for (kj = jN; kj < jN + 3; ++kj)
					if (ki != i && kj != j) a_ij[x++] = 9 * ki + kj;
		}
	}
	return a;
}
// get the first available number
inline int sd_first(uint16_t z)
{
	int i;
	for (i = 0; i < 9; ++i)
		if (1<<i & z) return i;
	return -1;
}
// set cell p to number x and update its neighbors; distant neighbors may also be updated
int sd_update(const uint8_t *a, uint16_t *z, uint16_t *stack2, int p, int x)
{
	int j = 0;
	stack2[j++] = p<<8 | x;
	while (--j >= 0) { // adjust p's neighbors; distant neighbors may also be updated
		int l, q = stack2[j]>>8, y = 1<<(stack2[j]&0xff);
		const uint8_t *aq = a + 20 * q; // array of q's neighbors
		z[q] = 1<<9 | y; // z[k] is resolved
		for (l = 0; l < 20; ++l) { // update q's neighbors
			uint16_t *zaql = z + aq[l];
			if (*zaql & y) { // neighbor ak[l] should be updated
				if (*zaql>>9 == 1) return -1; // confict!
				*zaql &= ~y; *zaql -= 1<<9; // update
				if (*zaql>>9 == 1) // then prepare to update aq[l]'s neighbors
					stack2[j++] = (int)aq[l]<<8 | sd_first(*zaql); // push to stack2
			} // otherwise, no need to update
		}
	}
	return 0;
}
// print the result
void sd_print(uint16_t z[81])
{
	int i;
	for (i = 0; i < 81; ++i) {
		int x = sd_first(z[i]);
		putchar(z[i]>>9 <= 1? x + '1' : (z[i]>>9) - 1 + 'a');
	}
	putchar('\n');
}
// solve the Sudoku
int sd_solve(const uint8_t *a, const char *_s)
{
	int i, m_stack, top = 0;
	uint16_t **stack, *z, swap[81], stack2[81];
	m_stack = 163; // I think 163 is enough; but I may be wrong. If not enough, use a dynamic array.
	stack = malloc(m_stack * sizeof(void*));
	for (i = 0; i < m_stack; ++i) stack[i] = malloc(162);
	for (i = 0, z = stack[0]; i < 81; ++i) { // change the input string into numbers
		int x = _s[i] >= '0' && _s[1] <= '9'? (int)_s[i] - '1' : -1;
		z[i] = x < 0? 0x1ff | 9<<9 : 1<<x | 1<<9;
	}
	for (i = 0; i < 81; ++i) // eliminate candidates using the basic rule
		if (z[i]>>9 == 1) sd_update(a, z, stack2, i, sd_first(z[i]));
	++top; // push to the stack
	while (top > 0) { // this is a DFS, but a tuned backtracking would be better
		int min, max, min_i;
		z = stack[--top]; // pop up
		for (i = 0, min = 10, min_i = 1, max = 0; i < 81; ++i) { // find the cell to try
			if (z[i]>>9 > 1 && z[i]>>9 < min) min = z[i]>>9, min_i = i;
			if (z[i]>>9 > max) max = z[i]>>9;
		}
		if (max > 1) { // we have not found a solution yet
			memcpy(swap, stack[top], 162);
			for (i = 0; i < 9; ++i) { // traverse all possible numbers
				if (1<<i & swap[min_i]) { // i is a possible number
					memcpy(stack[top], swap, 162);
					if (sd_update(a, stack[top], stack2, min_i, i) >= 0) { // then push to the stack
						int k;printf("%d\t%d\t%d\t", min, min_i, top); for (k = 0; k < 81; ++k) printf("%d", stack[top][k]>>9); printf("\n");
						if (++top >= m_stack) // "++top" to push to the stack
							fprintf(stderr, "[sd_solve] Woops! We need a dynamic stack.\n");
					}
				}
			}
		} else sd_print(z); // found a solution; print it
	}
	for (i = 0; i < m_stack; ++i) free(stack[i]); // free
	free(stack);
	return 0;
}

int main()
{
	uint8_t *a = sd_genaux();
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
