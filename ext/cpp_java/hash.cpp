// -*- mode: c++ -*-
// $Id: hash.g++,v 1.2 2001/06/20 03:20:02 doug Exp $
// http://www.bagley.org/~doug/shootout/

#include <stdio.h>
#include <iostream>
#include <hash_map.h>

using namespace std;

struct eqstr {
    bool operator()(const char* s1, const char* s2) const {
	return strcmp(s1, s2) == 0;
    }
};

int
main(int argc, char *argv[]) {
    int n = ((argc == 2) ? atoi(argv[1]) : 1);
    char buf[16];
    typedef hash_map<const char*, int, hash<const char*>, eqstr> HM;
    HM X;

    for (int i=1; i<=n; i++) {
	sprintf(buf, "%x", i);
	X[strdup(buf)] = i;
    }

    int c = 0;
    for (int i=n; i>0; i--) {
	sprintf(buf, "%d", i);
	if (X[strdup(buf)]) c++;
    }

    cout << c << endl;
}
