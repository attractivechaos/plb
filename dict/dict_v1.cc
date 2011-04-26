#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <unordered_map>

using namespace std;

struct eqstr {
	inline bool operator()(const char *s1, const char *s2) const {
		return strcmp(s1, s2) == 0;
    }
};

namespace std {
	template<>
	struct hash<const char *> : public std::unary_function<const char *, size_t> {
		size_t operator()(const char *s) const
		{ 
			size_t h = *s;
			if (h) for (++s ; *s; ++s) h = (h << 5) - h + *s;
		return h;
		}
	};
}

typedef unordered_map<const char*, int, hash<const char*>, eqstr> strhash;

#define BUF_SIZE 0x10000

int main(int argc, char *argv[])
{
	char *buf;
	int max = 1;
	strhash *h = new strhash;
	buf = (char*)malloc(BUF_SIZE); // buffer size
	while (!feof(stdin)) {
		fgets(buf, BUF_SIZE, stdin);
		strhash::iterator p = h->find(buf);
		if (p == h->end()) h->insert(pair<const char*, int>(strdup(buf), 1));
		else {
			++p->second;
			if (max < p->second) max = p->second;
		}
	}
	printf("%u\t%d\n", h->size(), max);
	strhash::iterator p;
	for (p = h->begin(); p != h->end(); ++p)
		free((char*)p->first);
	delete h;
	free(buf);
	return 0;
}
