#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regexp9.h"

#define BUF_SIZE 0x10000

int main(int argc, char *argv[])
{
	Resub rs[1];
	Reprog *p;
	char *buf, *q;
	int l = 0;
	if (argc == 1) {
		fprintf(stderr, "Usage: %s regexp < in.file\n", argv[0]);
		return 0;
	}
	p = regcomp9(argv[1]);
	buf = calloc(BUF_SIZE, 1);
	while (fgets(buf, BUF_SIZE - 1, stdin)) {
		++l;
		for (q = buf; *q; ++q); if (q > buf) *(q-1) = 0;
		memset(rs, 0, sizeof(Resub));
		if (regexec9(p, buf, rs, 1))
			printf("%d:%s\n", l, buf);
	}
	free(buf);
	return 0;
}
