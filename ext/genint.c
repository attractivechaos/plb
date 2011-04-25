#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

void print_data(int n)
{
	int i;
	unsigned x = 1234567890;
	for (i = 0; i < n; ++i) {
		x = (unsigned)(n * ((double)x / UINT_MAX) / 4) * 271828183u;
		printf("%u\n", x);
		x = 1664525L * x + 1013904223L;
	}
}

int main(int argc, char *argv[])
{
	int n = 5000000;
	if (argc > 1) n = atoi(argv[1]);
	print_data(n);
	return 0;
}
