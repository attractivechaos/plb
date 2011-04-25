#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

void print_data(int n)
{
	int i;
	srand48(11);
	for (i = 0; i < n; ++i)
		printf("%u\n", (unsigned)((lrand48() % (n/4))  * 271828183u));
}

int main(int argc, char *argv[])
{
	int n = 5000000;
	if (argc > 1) n = atoi(argv[1]);
	print_data(n);
	return 0;
}
