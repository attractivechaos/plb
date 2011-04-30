#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int NUM = 1, count;
	if (argc > 1) NUM = atoi(argv[1]);
	char flag[8192+1];
	while (NUM-- > 0) {
		int i, k;
		count = 0;
	    for (i = 2; i <= 8192; i++) flag[i] = 1;
	    for (i = 2; i <= 8192; i++) {
			if (flag[i]) {
		    	for (k = i + i; k <= 8192; k += i) flag[k] = 0;
				++count;
			}
	    }
	}
	printf("%d\n", count);
	return 0;
}

