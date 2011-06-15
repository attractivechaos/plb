/***********************************************************************
*                                                                      *
*                       Copyright (c) 2005-2010                        *
*                         All Rights Reserved                          *
*                                 gsf                                  *
*                                                                      *
***********************************************************************/
/*
 * Glenn Fowler
 * 9x9 sudoku backtrack solver -- no dancing
 * coded for simplicity over speed
 * puzzles on the standard input
 * option -a to list all solutions
 * option -c to count all solutions
 * option -t to list total number of solutions
 * option -u to verify only one solution
 */

static const char id[] = "\n@(#)$Id: sudocoo (AT&T Research) 2007-08-17 $\n";

#include <stdio.h>

typedef struct Grid_s {
	unsigned short	rcb[27], cell[81];
} Grid_t;

typedef struct Try_s {
	int	candidates, cell, free;
} Try_t;

static const char	name[] = ".123456789";

static unsigned char	constraint[81][3];
static unsigned char	ident[1<<9];
static unsigned char	count[1<<9];
static unsigned char	option[1<<8];

static unsigned short	next[1<<9];
static unsigned short	token[1<<8];

static Grid_t		base;

#define MASK(g,i)	(g.rcb[constraint[i][0]]&g.rcb[constraint[i][1]]&g.rcb[constraint[i][2]])
#define PROP(g,i,x)	(g.rcb[constraint[i][0]]^=(x),g.rcb[constraint[i][1]]^=(x),g.rcb[constraint[i][2]]^=(x))
#define MOVE(g,i,x)	(g.cell[i]=(x),PROP(g,i,x))
#define UNDO(g,i)	PROP(g,i,g.cell[i])

int
main(int argc, char** argv) {
	register int	i, j, k, m, n, x, z;
	int		level, depth;
	unsigned long	solutions;
	unsigned long	total;
	char*		s;
	Grid_t		grid;
	Try_t		try[81];

	while ((s = *++argv))
		while ((i = *s++))
			option[i] = 1;
	for (i = 1; i <= 9; i++)
		ident[token[(int)name[i]] = (1<<(i-1))] = i;
	token['.'] = token['0'] = token['_'] = 0x1ff;
	for (i = 0; i < 27; i++)
		base.rcb[i] = 0x1ff;
	for (i = 0; i < 1<<9; i++) {
		k = 0;
		for (j = 1; j < 1<<9; j <<= 1)
			if ((i & j) && !k++)
				next[i] = j;
		count[i] = k;
	}
	for (k = i = 0; i < 9; i++) {
		n = 18 + ((i / 3) * 3);
		for (j = 9; j < 18; j++) {
			constraint[k][0] = i;
			constraint[k][1] = j;
			constraint[k][2] = n + ((j / 3) % 3);
			k++;
		}
	}
	total = 0;
	for (;;) {
		grid = base;
		i = depth = 0;
		for (n = '\n'; (x = getchar()) != EOF; n = x)
			if ((j = token[x])) {
				if (j == 0x1ff)
					try[depth++].free = i;
				else
					MOVE(grid, i, j);
				if (++i >= 81)
					break;
			}
			else if ((x == '"') || ((x == '#' || x == '/') && (x = '\n')) || (x == '[' && (x = ']')) || (x == '<' && (x = '>')) || (n == '\n' && ((x >= 'A' && x <= 'Z') || (x >= 'a' && x <= 'z')) && (x = '\n')))
				while ((j = getchar()) != EOF && j != x);
		if (i < 81) {
			if (x == EOF)
				break;
			continue;
		}
		level = solutions = 0;
		for (;;) {
			m = 10;
			for (j = level; j < depth; j++) {
				i = try[j].free;
				if (!(x = MASK(grid, i))) {
					m = 11;
					break;
				}
				if (m > (n = count[x])) {
					k = j;
					z = x;
					if ((m = n) == 1)
						break;
				}
			}
			if (m < 10) {
				try[level+1].candidates = z;
				try[level+1].cell = try[k].free;
				if (k != level) {
					j = try[level].free;
					try[level].free = try[k].free;
					try[k].free = j;
				}
				level++;
			}
			else {
				if (m == 10) {
					solutions++;
					if (!option['c']) {
						if (solutions > 1 && !option['a']) {
							printf(" multiple solutions");
							break;
						}
						if (!option['t'])
							for (i = 0; i < 81; i++)
								putchar(name[ident[grid.cell[i]]]);
						if (option['a']) {
							if (!option['t'])
								putchar('\n');
						}
						else if (!option['u'])
							break;
					}
				}
				UNDO(grid, try[level].cell);
			}
			while (!try[level].candidates) {
				if (!--level) {
					if (!solutions)
						printf("no solution");
					goto done;
				}
				UNDO(grid, try[level].cell);
			}
			x = next[try[level].candidates];
			try[level].candidates ^= x;
			MOVE(grid, try[level].cell, x);
		}
	done:
		if (option['t'])
			total += solutions;
		else if (option['c'])
			printf("%lu\n", solutions);
		else if (!option['a'])
			putchar('\n');
	}
	if (option['t'])
		printf("%lu\n", total);
	return 0;
}
