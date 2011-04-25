#include <stdio.h>
#include "khash.h"
KHASH_MAP_INIT_STR(str, int)

#define BUF_SIZE 0x10000

int main(int argc, char *argv[])
{
	char *buf;
	int ret, max = 0;
	khint_t k;
	khash_t(str) *h;
	buf = malloc(BUF_SIZE); // buffer size
	h = kh_init(str);
	while (!feof(stdin)) {
		fgets(buf, BUF_SIZE, stdin);
		k = kh_put(str, h, buf, &ret);
		if (ret) { // absent
			kh_key(h, k) = strdup(buf);
			kh_val(h, k) = 0;
		} else ++kh_val(h, k);
		if (kh_val(h, k) > max) max = kh_val(h, k);
	}
	printf("%u\t%d\n", kh_size(h), max);
	for (k = kh_begin(h); k < kh_end(h); ++k)
		if (kh_exist(h, k)) free((char*)kh_key(h, k));
	kh_destroy(str, h);
	free(buf);
	return 0;
}
