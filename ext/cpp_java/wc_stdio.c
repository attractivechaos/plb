#include <stdio.h>

int main()
{
	int c, nc, nl, nw, state = 1;
	nc = nl = nw = 0;
	while (!feof(stdin)) {
		c = fgetc(stdin);
		++nc;
		if (c == '\n') ++nl;
		if (c == ' ' || c == '\t' || c == '\n') state = 1;
		else if (state) state = 0, ++nw;
	}
	printf("%d %d %d\n", nl, nw, nc);
	return 0;
}
