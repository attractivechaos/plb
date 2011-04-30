#include <stdio.h>
#include <fcntl.h>
#include "kseq.h"
KSTREAM_INIT(int, read, 0x10000)

int main()
{
	int c, fd, nc, nl, nw, state = 1, dret;
	kstream_t *ks;
	kstring_t str;
	fd = fileno(stdin);
	ks = ks_init(fd);
	nc = nl = nw = 0;
	str.l = str.m = 0; str.s = 0;
	while (ks_getuntil(ks, '\n', &str, &dret) >= 0) {
		int i;
		for (i = 0; i <= str.l; ++i) {
			int c = str.s[i];
			++nc;
			if (c == ' ' || c == '\t' || c == '\0') state = 1;
			else if (state) state = 0, ++nw;
		}
		++nl;
	}
	printf("%d %d %d\n", nl, nw, nc);
	ks_destroy(ks);
	return 0;
}
